#include <Arduino.h>
#include <Wire.h>
#include <math.h>

// Dirección I2C del sensor MPU6050 y pines utilizados para la comunicación I2C.
#define MPU_ADDR 0x68
#define SDA_PIN 21
#define SCL_PIN 22

// Pines del canal A del controlador de motores TB6612FNG.
#define PWMA 32
#define AIN1 25
#define AIN2 33

// Pin que activa o desactiva el controlador de motores.
#define STBY 26

// Pines del canal B del controlador de motores TB6612FNG.
#define BIN1 27
#define BIN2 14
#define PWMB 12

// Permiten invertir individualmente el sentido de giro de cada motor
// para compensar que normalmente están montados en orientaciones opuestas.
bool invertirMotorA = false;
bool invertirMotorB = true;

// Define el sentido global de la corrección de los motores.
// Se puede cambiar a -1 si el robot se mueve hacia el lado contrario.
const int SIGNO_CONTROL = 1;

// Define el signo de las mediciones de inclinación del MPU6050.
// Se puede cambiar a -1 si el ángulo medido aparece invertido.
const float SIGNO_IMU = 1.0f;

// Ganancia proporcional: determina la fuerza con la que se corrige
// el error de inclinación respecto a la posición vertical.
float Kp = 10.0f;

// Ganancia derivativa: amortigua el movimiento usando la velocidad angular
// para reducir oscilaciones y frenar la caída del robot.
float Kd = 2.0f;

// Ángulo en grados que se considera como la posición vertical deseada.
float anguloObjetivo = 0.0f;

// Zona alrededor de la vertical en la que no se aplica corrección proporcional.
// La corrección derivativa continúa activa para amortiguar el movimiento.
const float MARGEN_VERTICAL = 0.1f;

// Inclinación máxima permitida antes de considerar que el robot se ha caído.
const float ANGULO_MAXIMO = 50.0f;

// PWM mínimo aplicado cuando existe una orden de movimiento.
// Ayuda a vencer la fricción inicial de los motores.
const int PWM_MINIMO = 2;

// Cambio máximo de PWM permitido entre dos ciclos consecutivos.
// Evita aceleraciones bruscas y golpes mecánicos en los motores.
const int CAMBIO_PWM_MAXIMO = 10;

// Tiempo entre ejecuciones del control, expresado en microsegundos.
// 5000 microsegundos equivalen a una frecuencia de control de 200 Hz.
const unsigned long PERIODO_US = 5000;

// Coeficiente del filtro pasa-bajos aplicado a la velocidad del giroscopio.
// Cuanto menor sea, más suave será la medición, pero reaccionará más lentamente.
const float FILTRO_GYRO = 0.25f;

// Ángulo estimado actual del robot alrededor del eje X.
float anguloX = 0.0f;

// Desviación del giroscopio medida durante la calibración.
// Se resta de las lecturas para reducir el error del sensor en reposo.
float offsetGyroX = 0.0f;

// Velocidad angular actual obtenida directamente del giroscopio.
float velocidadGyroX = 0.0f;

// Velocidad angular después de aplicar el filtro pasa-bajos.
float velocidadGyroFiltrada = 0.0f;

// Último PWM aplicado a los motores.
// Se utiliza para limitar cuánto puede variar la potencia en cada ciclo.
int pwmAnterior = 0;

// Momento en microsegundos en el que se ejecutó el ciclo de control anterior.
unsigned long tiempoAnterior = 0;

// Configura como salidas los pines del controlador de motores
// e inicializa las señales PWM de los dos canales.
void configurarMotores() {
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);

  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  pinMode(STBY, OUTPUT);

  // Mantiene el controlador desactivado durante su configuración.
  digitalWrite(STBY, LOW);

  // Asocia cada pin PWM a una señal de 20 kHz con una resolución de 8 bits.
  // Esta forma de configuración corresponde a ESP32 Arduino Core 3.x.
  ledcAttach(PWMA, 20000, 8);
  ledcAttach(PWMB, 20000, 8);
}

// Controla un motor indicando su velocidad, sus pines de dirección,
// el pin PWM correspondiente y si debe invertirse su sentido de giro.
void controlarMotor(
  int velocidad,
  int pin1,
  int pin2,
  int pinPWM,
  bool invertir
) {
  // Invierte el signo de la velocidad cuando el montaje del motor lo requiere.
  if (invertir) {
    velocidad = -velocidad;
  }

  // Limita la velocidad al intervalo válido para un PWM de 8 bits.
  velocidad = constrain(velocidad, -255, 255);

  // Selecciona el sentido de giro según el signo de la velocidad.
  if (velocidad > 0) {
    digitalWrite(pin1, HIGH);
    digitalWrite(pin2, LOW);
  }
  else if (velocidad < 0) {
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, HIGH);
  }
  else {
    // Si la velocidad es cero, deja las dos entradas de dirección a nivel bajo.
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, LOW);
  }

  // Aplica como PWM el valor absoluto de la velocidad.
  // El sentido de giro ya se ha establecido mediante los pines de dirección.
  ledcWrite(pinPWM, abs(velocidad));
}

// Aplica la misma orden de velocidad a los dos motores,
// respetando la inversión configurada para cada uno.
void moverMotores(int velocidad) {
  controlarMotor(
    velocidad,
    AIN1,
    AIN2,
    PWMA,
    invertirMotorA
  );

  controlarMotor(
    velocidad,
    BIN1,
    BIN2,
    PWMB,
    invertirMotorB
  );
}

// Detiene ambos motores enviándoles una velocidad igual a cero.
void detenerMotores() {
  moverMotores(0);
}

// Reinicia las variables relacionadas con el control y detiene los motores.
// Se utiliza cuando ocurre una caída, un error de lectura o un tiempo anómalo,
// evitando que se reutilicen valores anteriores de PWM o velocidad angular.
void reiniciarControl() {
  pwmAnterior = 0;
  velocidadGyroFiltrada = 0.0f;

  detenerMotores();
}

// Escribe un byte en un registro del MPU6050 mediante el bus I2C.
// Devuelve true si la transmisión se completa correctamente.
bool escribirMPU(byte registro, byte valor) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(registro);
  Wire.write(valor);

  return Wire.endTransmission() == 0;
}

// Lee dos bytes consecutivos del bus I2C y los combina
// para formar un valor entero de 16 bits con signo.
int16_t leer16Bits() {
  byte alto = Wire.read();
  byte bajo = Wire.read();

  return (int16_t)((alto << 8) | bajo);
}

// Inicializa la comunicación I2C, comprueba que el MPU6050 responde
// y configura sus rangos de medida, filtro interno y frecuencia de muestreo.
bool iniciarMPU() {
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);

  // Espera a que el sensor esté disponible después de iniciar el bus I2C.
  delay(100);

  // Comprueba si existe un dispositivo que responde en la dirección del MPU6050.
  Wire.beginTransmission(MPU_ADDR);

  if (Wire.endTransmission() != 0) {
    return false;
  }

  // Desactiva el modo de reposo del MPU6050.
  if (!escribirMPU(0x6B, 0x00)) {
    return false;
  }

  // Configura el giroscopio con un rango de ±250 grados por segundo.
  if (!escribirMPU(0x1B, 0x00)) {
    return false;
  }

  // Configura el acelerómetro con un rango de ±2 g.
  if (!escribirMPU(0x1C, 0x00)) {
    return false;
  }

  // Configura el filtro digital interno del sensor.
  if (!escribirMPU(0x1A, 0x03)) {
    return false;
  }

  // Configura el divisor utilizado para determinar la frecuencia de muestreo.
  if (!escribirMPU(0x19, 0x04)) {
    return false;
  }

  // Espera a que la configuración del sensor se estabilice.
  delay(100);

  return true;
}

// Lee del MPU6050 la aceleración de los ejes Y y Z y la velocidad
// angular del eje X. La temperatura se encuentra entre esos registros,
// por lo que también se lee, pero su valor se descarta.
bool leerDatosEjeX(
  int16_t &ay,
  int16_t &az,
  int16_t &gx
) {
  // Sitúa el puntero de lectura en el registro correspondiente a AY.
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3D);

  // Mantiene activa la comunicación I2C para realizar una lectura consecutiva.
  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  // Solicita los ocho bytes correspondientes a AY, AZ, temperatura y GX.
  if (Wire.requestFrom(MPU_ADDR, 8, true) != 8) {
    return false;
  }

  ay = leer16Bits();
  az = leer16Bits();

  // Lee los dos bytes de temperatura para avanzar hasta el registro de GX.
  leer16Bits();

  gx = leer16Bits();

  return true;
}

// Calcula el ángulo de inclinación alrededor del eje X utilizando
// las componentes Y y Z del acelerómetro y lo convierte a grados.
float calcularAnguloAcelerometro(
  int16_t ay,
  int16_t az
) {
  return SIGNO_IMU *
         atan2f((float)ay, (float)az) *
         180.0f / PI;
}

// Obtiene una nueva lectura del sensor y actualiza el ángulo estimado.
// Combina el acelerómetro y el giroscopio mediante un filtro complementario.
bool actualizarAngulo(float dt) {
  int16_t ay;
  int16_t az;
  int16_t gx;

  // Cancela la actualización si no se pueden leer los datos del sensor.
  if (!leerDatosEjeX(ay, az, gx)) {
    return false;
  }

  // Calcula una referencia absoluta de inclinación usando el acelerómetro.
  float anguloAcelerometro =
    calcularAnguloAcelerometro(ay, az);

  // Resta el offset de calibración y convierte la lectura del giroscopio
  // a grados por segundo usando la sensibilidad del rango de ±250 grados/s.
  velocidadGyroX =
    SIGNO_IMU *
    ((float)gx - offsetGyroX) /
    131.0f;

  // Aplica un filtro pasa-bajos a la velocidad angular.
  // Esto reduce el ruido utilizado por el término derivativo del control.
  velocidadGyroFiltrada +=
    FILTRO_GYRO *
    (velocidadGyroX - velocidadGyroFiltrada);

  // El filtro complementario combina la respuesta rápida del giroscopio
  // con la referencia estable a largo plazo proporcionada por el acelerómetro.
  anguloX =
    0.98f *
    (anguloX + velocidadGyroX * dt) +
    0.02f *
    anguloAcelerometro;

  return true;
}

// Calcula el offset del giroscopio promediando 500 lecturas en reposo.
// Durante este proceso el robot no debe moverse.
void calibrarGyroX() {
  const int muestras = 500;

  // Acumula las lecturas del giroscopio para calcular posteriormente la media.
  long suma = 0;
  int muestrasValidas = 0;

  Serial.println("Calibrando GX. No mover el robot.");

  // Continúa leyendo hasta obtener el número requerido de muestras válidas.
  while (muestrasValidas < muestras) {
    int16_t ay;
    int16_t az;
    int16_t gx;

    if (leerDatosEjeX(ay, az, gx)) {
      suma += gx;
      muestrasValidas++;
    }

    // Introduce una pequeña pausa entre muestras.
    delay(2);
  }

  // La media obtenida representa la lectura del giroscopio cuando está inmóvil.
  offsetGyroX =
    suma / (float)muestras;

  Serial.print("Offset GX: ");
  Serial.println(offsetGyroX);
}

// Inicializa la comunicación serie, los motores, el MPU6050,
// la calibración del giroscopio y las variables iniciales del control.
void setup() {
  Serial.begin(115200);
  delay(1000);

  // Configura el controlador y garantiza que los motores comiencen detenidos.
  configurarMotores();
  detenerMotores();

  Serial.println("Iniciando MPU6050...");

  // Si el sensor no responde, mantiene permanentemente los motores detenidos.
  if (!iniciarMPU()) {
    Serial.println("No se detecta el MPU6050.");

    while (true) {
      detenerMotores();
      delay(1000);
    }
  }

  // Calcula el error en reposo del giroscopio antes de iniciar el control.
  calibrarGyroX();

  int16_t ay;
  int16_t az;
  int16_t gx;

  // Inicializa el ángulo con una lectura del acelerómetro
  // para evitar que el filtro complementario comience desde un valor incorrecto.
  if (leerDatosEjeX(ay, az, gx)) {
    anguloX =
      calcularAnguloAcelerometro(ay, az);
  }

  // Reinicia las variables dinámicas antes de comenzar el control.
  velocidadGyroX = 0.0f;
  velocidadGyroFiltrada = 0.0f;
  pwmAnterior = 0;

  // Guarda el tiempo inicial que se utilizará para calcular el primer intervalo.
  tiempoAnterior = micros();

  // Saca al TB6612FNG del modo de espera y habilita sus salidas.
  digitalWrite(STBY, HIGH);

  Serial.println("Robot listo.");
}

// Ejecuta periódicamente la lectura del sensor, el cálculo del control PD
// y la aplicación del PWM necesario para mantener el robot en equilibrio.
void loop() {
  unsigned long ahora = micros();

  // No ejecuta un nuevo ciclo hasta que haya transcurrido el periodo configurado.
  if (ahora - tiempoAnterior < PERIODO_US) {
    return;
  }

  // Convierte a segundos el tiempo transcurrido desde el ciclo anterior.
  float dt =
    (ahora - tiempoAnterior) /
    1000000.0f;

  // Guarda el instante actual para calcular el siguiente intervalo.
  tiempoAnterior = ahora;

  // Detiene y reinicia el control si el intervalo de tiempo no es válido
  // o es demasiado grande para realizar un cálculo fiable.
  if (dt <= 0.0f || dt > 0.05f) {
    reiniciarControl();
    return;
  }

  // Actualiza la inclinación y detiene el control si falla la lectura del sensor.
  if (!actualizarAngulo(dt)) {
    reiniciarControl();
    return;
  }

  // Detiene los motores si el robot supera la inclinación máxima permitida.
  if (fabsf(anguloX) > ANGULO_MAXIMO) {
    reiniciarControl();
    return;
  }

  // Calcula la diferencia entre el ángulo deseado y el ángulo medido.
  float errorReal =
    anguloObjetivo - anguloX;

  // Error utilizado por la parte proporcional del controlador.
  // Inicialmente es cero para crear una zona sin corrección alrededor de la vertical.
  float errorControl = 0.0f;

  // Cuando el error supera el margen positivo, se elimina del cálculo
  // el tamaño de la zona muerta para que la respuesta comience progresivamente.
  if (errorReal > MARGEN_VERTICAL) {
    errorControl =
      errorReal - MARGEN_VERTICAL;
  }
  // Se aplica el mismo procedimiento cuando el error supera el margen negativo.
  else if (errorReal < -MARGEN_VERTICAL) {
    errorControl =
      errorReal + MARGEN_VERTICAL;
  }

  // La parte proporcional genera una corrección según la inclinación actual.
  float terminoProporcional =
    Kp * errorControl;

  // La parte derivativa se opone a la velocidad angular del robot,
  // ayudando a frenar la caída y reducir las oscilaciones.
  float terminoDerivativo =
    -Kd * velocidadGyroFiltrada;

  // Combina las dos partes del controlador y aplica el sentido global configurado.
  float salida =
    SIGNO_CONTROL *
    (terminoProporcional + terminoDerivativo);

  // Limita la salida al intervalo que puede representar el PWM de 8 bits.
  salida = constrain(
    salida,
    -255.0f,
    255.0f
  );

  // Redondea la salida decimal para obtener un valor entero de PWM.
  int pwmSolicitado =
    roundf(salida);

  // Garantiza una potencia mínima cuando se solicita movimiento positivo,
  // evitando valores demasiado pequeños para vencer la fricción del motor.
  if (
    pwmSolicitado > 0 &&
    pwmSolicitado < PWM_MINIMO
  ) {
    pwmSolicitado = PWM_MINIMO;
  }
  // Aplica el mismo mínimo cuando se solicita movimiento negativo.
  else if (
    pwmSolicitado < 0 &&
    pwmSolicitado > -PWM_MINIMO
  ) {
    pwmSolicitado = -PWM_MINIMO;
  }

  // Calcula cuánto tendría que variar el PWM respecto al ciclo anterior.
  int cambioPWM =
    pwmSolicitado - pwmAnterior;

  // Limita esa variación para evitar cambios instantáneos de velocidad.
  cambioPWM = constrain(
    cambioPWM,
    -CAMBIO_PWM_MAXIMO,
    CAMBIO_PWM_MAXIMO
  );

  // Obtiene el PWM que realmente se aplicará después de limitar su variación.
  int pwm =
    pwmAnterior + cambioPWM;

  // Garantiza nuevamente que el resultado permanezca dentro del rango válido.
  pwm = constrain(pwm, -255, 255);

  // Guarda el PWM actual para utilizarlo como referencia en el siguiente ciclo.
  pwmAnterior = pwm;

  // Envía la orden calculada a los dos motores.
  moverMotores(pwm);

  // Conserva entre ciclos el momento de la última impresión por el puerto serie.
  static unsigned long ultimaImpresion = 0;

  // Muestra información de diagnóstico cada 50 milisegundos
  // sin detener la ejecución del control.
  if (
    millis() - ultimaImpresion >= 50
  ) {
    ultimaImpresion = millis();

    Serial.print("Angulo: ");
    Serial.print(anguloX, 2);

    Serial.print("  Error: ");
    Serial.print(errorReal, 2);

    Serial.print("  Gyro: ");
    Serial.print(velocidadGyroFiltrada, 2);

    Serial.print("  PWM solicitado: ");
    Serial.print(pwmSolicitado);

    Serial.print("  PWM aplicado: ");
    Serial.println(pwm);
  }
}