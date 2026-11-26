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

// Los motores están montados en sentidos opuestos.
// Estas variables permiten corregir el sentido de cada uno sin cambiar el cableado.
bool invertirMotorA = false;
bool invertirMotorB = true;

// Cambia este valor a -1 solamente si la corrección completa actúa al revés.
const int SIGNO_CONTROL = 1;

// Cambia este valor a -1 solamente si el ángulo del MPU6050 aparece invertido.
const float SIGNO_IMU = 1.0f;

// Se conservan exactamente las ganancias ajustadas en el programa original.
float Kp = 10.0f;
float Kd = 2.0f;

// La estructura de los dos programas incluye la acción integral.
// Se deja desactivada para no modificar la respuesta que ya fue ajustada con Kp y Kd.
float Ki = 0.0f;

// Ángulo que representa la posición vertical deseada.
float anguloObjetivo = 0.0f;

// El controlador no corrige errores proporcionales menores que este margen.
// La finalidad es evitar pequeñas oscilaciones debidas al ruido cerca de la vertical.
const float MARGEN_VERTICAL = 0.1f;

// Si se supera este ángulo, se considera que el robot ha caído y se detienen los motores.
const float ANGULO_MAXIMO = 50.0f;

// PWM mínimo necesario para ayudar a vencer la fricción inicial de los motores.
const int PWM_MINIMO = 2;

// Limita el cambio de PWM entre dos ciclos para evitar golpes mecánicos.
const int CAMBIO_PWM_MAXIMO = 10;

// El control se ejecuta cada 5000 microsegundos, es decir, a 200 Hz.
const unsigned long PERIODO_US = 5000;

// La acción integral del controlador recursivo usa un periodo fijo de 5 ms.
const float DT_CONTROL = PERIODO_US / 1000000.0f;

// Suaviza la velocidad angular medida por el giroscopio.
// Se conserva para la estimación y para mostrar datos de diagnóstico.
const float FILTRO_GYRO = 0.25f;

// Variables utilizadas para estimar el ángulo del robot.
float anguloX = 0.0f;
float offsetGyroX = 0.0f;
float velocidadGyroX = 0.0f;
float velocidadGyroFiltrada = 0.0f;

// Guarda el PWM aplicado en el ciclo anterior para limitar cambios bruscos.
int pwmAnterior = 0;

// Guarda el instante del ciclo anterior para mantener el periodo de control.
unsigned long tiempoAnterior = 0;

// Estas funciones se implementan de forma diferente en cada uno de los dos programas.
float calcularControl(float error);
void reiniciarControlador();

// Configura los pines de dirección, espera y PWM del TB6612FNG.
void configurarMotores() {
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(STBY, OUTPUT);

  // El controlador de motores permanece desactivado durante la inicialización.
  digitalWrite(STBY, LOW);

  // Configura un PWM de 20 kHz y 8 bits en los mismos pines del programa original.
  ledcAttach(PWMA, 20000, 8);
  ledcAttach(PWMB, 20000, 8);
}

// Aplica velocidad y sentido a uno de los motores.
void controlarMotor(
  int velocidad,
  int pin1,
  int pin2,
  int pinPWM,
  bool invertir
) {
  // Corrige el sentido de giro cuando el motor está montado de forma opuesta.
  if (invertir) {
    velocidad = -velocidad;
  }

  // El PWM de 8 bits admite valores entre 0 y 255.
  velocidad = constrain(velocidad, -255, 255);

  if (velocidad > 0) {
    digitalWrite(pin1, HIGH);
    digitalWrite(pin2, LOW);
  } else if (velocidad < 0) {
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, HIGH);
  } else {
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, LOW);
  }

  // El signo selecciona el sentido y el valor absoluto determina la potencia.
  ledcWrite(pinPWM, abs(velocidad));
}

// Envía la misma orden de control a los dos motores.
void moverMotores(int velocidad) {
  controlarMotor(velocidad, AIN1, AIN2, PWMA, invertirMotorA);
  controlarMotor(velocidad, BIN1, BIN2, PWMB, invertirMotorB);
}

// Detiene ambos motores.
void detenerMotores() {
  moverMotores(0);
}

// Escribe un valor en un registro del MPU6050.
bool escribirMPU(byte registro, byte valor) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(registro);
  Wire.write(valor);

  return Wire.endTransmission() == 0;
}

// Une dos bytes consecutivos del MPU6050 en un entero de 16 bits con signo.
int16_t leer16Bits() {
  byte alto = Wire.read();
  byte bajo = Wire.read();

  return (int16_t)((alto << 8) | bajo);
}

// Inicializa el bus I2C y configura el MPU6050.
bool iniciarMPU() {
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);
  delay(100);

  // Comprueba que el sensor responde en la dirección configurada.
  Wire.beginTransmission(MPU_ADDR);

  if (Wire.endTransmission() != 0) {
    return false;
  }

  // Desactiva el modo de reposo.
  if (!escribirMPU(0x6B, 0x00)) {
    return false;
  }

  // Selecciona el rango de ±250 grados por segundo para el giroscopio.
  if (!escribirMPU(0x1B, 0x00)) {
    return false;
  }

  // Selecciona el rango de ±2 g para el acelerómetro.
  if (!escribirMPU(0x1C, 0x00)) {
    return false;
  }

  // Activa el filtro digital interno del MPU6050.
  if (!escribirMPU(0x1A, 0x03)) {
    return false;
  }

  // Configura el divisor de la frecuencia de muestreo interna.
  if (!escribirMPU(0x19, 0x04)) {
    return false;
  }

  delay(100);
  return true;
}

// Lee aceleración Y, aceleración Z y velocidad angular X.
bool leerDatosEjeX(
  int16_t &ay,
  int16_t &az,
  int16_t &gx
) {
  // El registro 0x3D corresponde al byte alto de la aceleración Y.
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3D);

  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  // Se leen AY, AZ, temperatura y GX en una única operación.
  if (Wire.requestFrom((uint8_t)MPU_ADDR, (uint8_t)8, true) != 8) {
    return false;
  }

  ay = leer16Bits();
  az = leer16Bits();

  // La temperatura no se utiliza, pero debe leerse para llegar al registro GX.
  leer16Bits();

  gx = leer16Bits();

  return true;
}

// Calcula el ángulo alrededor del eje X usando el acelerómetro.
float calcularAnguloAcelerometro(
  int16_t ay,
  int16_t az
) {
  return SIGNO_IMU *
         atan2f((float)ay, (float)az) *
         180.0f / PI;
}

// Actualiza el ángulo estimado combinando acelerómetro y giroscopio.
bool actualizarAngulo(float dtSensor) {
  int16_t ay;
  int16_t az;
  int16_t gx;

  if (!leerDatosEjeX(ay, az, gx)) {
    return false;
  }

  float anguloAcelerometro =
    calcularAnguloAcelerometro(ay, az);

  // Convierte la lectura del giroscopio a grados por segundo y resta su offset.
  velocidadGyroX =
    SIGNO_IMU *
    ((float)gx - offsetGyroX) /
    131.0f;

  // Reduce el ruido de la velocidad angular.
  velocidadGyroFiltrada +=
    FILTRO_GYRO *
    (velocidadGyroX - velocidadGyroFiltrada);

  // El giroscopio aporta respuesta rápida y el acelerómetro corrige la deriva.
  anguloX =
    0.98f *
    (anguloX + velocidadGyroX * dtSensor) +
    0.02f *
    anguloAcelerometro;

  return true;
}

// Calcula el offset del giroscopio con el robot inmóvil.
void calibrarGyroX() {
  const int muestras = 500;
  long suma = 0;
  int muestrasValidas = 0;

  Serial.println("Calibrando GX. No mover el robot.");

  while (muestrasValidas < muestras) {
    int16_t ay;
    int16_t az;
    int16_t gx;

    if (leerDatosEjeX(ay, az, gx)) {
      suma += gx;
      muestrasValidas++;
    }

    delay(2);
  }

  offsetGyroX = suma / (float)muestras;

  Serial.print("Offset GX: ");
  Serial.println(offsetGyroX);
}

// Aplica la misma zona muerta que utiliza el programa original.
float aplicarZonaMuerta(float errorReal) {
  if (errorReal > MARGEN_VERTICAL) {
    return errorReal - MARGEN_VERTICAL;
  }

  if (errorReal < -MARGEN_VERTICAL) {
    return errorReal + MARGEN_VERTICAL;
  }

  return 0.0f;
}

// Limpia los estados internos cuando el robot cae o se produce un error.
void reiniciarControl() {
  pwmAnterior = 0;
  velocidadGyroFiltrada = 0.0f;

  reiniciarControlador();
  detenerMotores();
}

// Convierte la salida decimal del controlador en una orden PWM segura.
int convertirSalidaAPwm(
  float salidaControl,
  int &pwmSolicitado
) {
  float salida =
    SIGNO_CONTROL *
    salidaControl;

  // Evita pedir al actuador un valor fuera de su intervalo físico.
  salida = constrain(
    salida,
    -255.0f,
    255.0f
  );

  pwmSolicitado =
    roundf(salida);

  // Aplica un mínimo cuando existe una orden distinta de cero.
  if (
    pwmSolicitado > 0 &&
    pwmSolicitado < PWM_MINIMO
  ) {
    pwmSolicitado = PWM_MINIMO;
  } else if (
    pwmSolicitado < 0 &&
    pwmSolicitado > -PWM_MINIMO
  ) {
    pwmSolicitado = -PWM_MINIMO;
  }

  // Limita cuánto puede cambiar la orden respecto al ciclo anterior.
  int cambioPWM =
    constrain(
      pwmSolicitado - pwmAnterior,
      -CAMBIO_PWM_MAXIMO,
      CAMBIO_PWM_MAXIMO
    );

  int pwm =
    pwmAnterior +
    cambioPWM;

  pwm = constrain(
    pwm,
    -255,
    255
  );

  pwmAnterior = pwm;

  return pwm;
}


// Guarda el error utilizado en el ciclo anterior.
float errorAnteriorIIR = 0.0f;

// Guarda la velocidad angular filtrada del ciclo anterior.
float gyroAnteriorIIR = 0.0f;

// Guarda la salida anterior porque el controlador se calcula de forma recursiva.
float salidaAnteriorIIR = 0.0f;

// Evita utilizar estados anteriores que todavía no existen en el primer ciclo.
bool primerCicloIIR = true;

// Implementa el PID en forma incremental y recursiva.
// La ecuación es equivalente a calcular:
// u = Kp * error + Ki * integral - Kd * velocidadAngular
//
// Su forma incremental es:
// u[k] = u[k-1]
//        + Kp * (e[k] - e[k-1])
//        + Ki * DT * e[k]
//        - Kd * (gyro[k] - gyro[k-1])
//
// Se usa la velocidad del giroscopio para la acción derivativa,
// igual que en el código original y en el PID directo corregido.
float calcularControl(float error) {
  if (primerCicloIIR) {
    // La primera salida se calcula directamente porque todavía no hay
    // valores anteriores con los que formar la ecuación incremental.
    salidaAnteriorIIR =
      Kp * error -
      Kd * velocidadGyroFiltrada;

    errorAnteriorIIR =
      error;

    gyroAnteriorIIR =
      velocidadGyroFiltrada;

    primerCicloIIR =
      false;

    return salidaAnteriorIIR;
  }

  // Cambio de la acción proporcional respecto al ciclo anterior.
  float incrementoProporcional =
    Kp *
    (error - errorAnteriorIIR);

  // Incremento producido por la acción integral.
  // Ki se mantiene en cero para conservar el ajuste original.
  float incrementoIntegral =
    Ki *
    DT_CONTROL *
    error;

  // Cambio de la acción derivativa calculada con la velocidad angular.
  // El signo negativo hace que la derivada se oponga al movimiento de caída.
  float incrementoDerivativo =
    -Kd *
    (
      velocidadGyroFiltrada -
      gyroAnteriorIIR
    );

  // La nueva salida se obtiene sumando los incrementos a la salida anterior.
  float salida =
    salidaAnteriorIIR +
    incrementoProporcional +
    incrementoIntegral +
    incrementoDerivativo;

  // Guarda los valores actuales para calcular el siguiente ciclo.
  errorAnteriorIIR =
    error;

  gyroAnteriorIIR =
    velocidadGyroFiltrada;

  salidaAnteriorIIR =
    salida;

  return salida;
}

// Borra todos los estados del controlador recursivo cuando el robot cae
// o cuando se produce un error de lectura o de temporización.
void reiniciarControlador() {
  errorAnteriorIIR = 0.0f;
  gyroAnteriorIIR = 0.0f;
  salidaAnteriorIIR = 0.0f;
  primerCicloIIR = true;
}


// Inicializa el puerto serie, los motores, el sensor y los estados del controlador.
void setup() {
  Serial.begin(115200);
  delay(1000);

  configurarMotores();
  detenerMotores();

  Serial.println("Iniciando MPU6050...");

  if (!iniciarMPU()) {
    Serial.println("No se detecta el MPU6050.");

    // Mantiene los motores detenidos si el sensor no está disponible.
    while (true) {
      detenerMotores();
      delay(1000);
    }
  }

  calibrarGyroX();

  int16_t ay;
  int16_t az;
  int16_t gx;

  // Inicializa el filtro complementario con un ángulo real del acelerómetro.
  if (leerDatosEjeX(ay, az, gx)) {
    anguloX =
      calcularAnguloAcelerometro(
        ay,
        az
      );
  }

  velocidadGyroX = 0.0f;
  velocidadGyroFiltrada = 0.0f;
  pwmAnterior = 0;

  reiniciarControlador();

  tiempoAnterior = micros();

  // Habilita el TB6612FNG después de terminar toda la inicialización.
  digitalWrite(STBY, HIGH);

  Serial.println("Robot listo.");
}

// Ejecuta la lectura del sensor, el controlador y la actuación cada 5 ms.
void loop() {
  unsigned long ahora =
    micros();

  // Sale inmediatamente hasta que se complete el periodo de muestreo.
  if (
    ahora - tiempoAnterior <
    PERIODO_US
  ) {
    return;
  }

  // Este tiempo real se usa únicamente para integrar correctamente el giroscopio.
  float dtSensor =
    (ahora - tiempoAnterior) /
    1000000.0f;

  tiempoAnterior = ahora;

  // Un intervalo anómalo hace que los estados anteriores dejen de ser fiables.
  if (
    dtSensor <= 0.0f ||
    dtSensor > 0.05f
  ) {
    reiniciarControl();
    return;
  }

  if (!actualizarAngulo(dtSensor)) {
    reiniciarControl();
    return;
  }

  // Detiene el sistema cuando el robot está demasiado inclinado.
  if (
    fabsf(anguloX) >
    ANGULO_MAXIMO
  ) {
    reiniciarControl();
    return;
  }

  // El error es positivo o negativo según el lado hacia el que cae el robot.
  float errorReal =
    anguloObjetivo -
    anguloX;

  // Mantiene la zona muerta del programa original alrededor de la vertical.
  float errorControl =
    aplicarZonaMuerta(
      errorReal
    );

  // Calcula la salida mediante la ecuación PID incremental y recursiva.
  float salidaControl =
    calcularControl(
      errorControl
    );

  int pwmSolicitado = 0;

  int pwm =
    convertirSalidaAPwm(
      salidaControl,
      pwmSolicitado
    );

  moverMotores(pwm);

  // Imprime información cada 50 ms sin bloquear el lazo de control.
  static unsigned long ultimaImpresion = 0;

  if (
    millis() - ultimaImpresion >=
    50
  ) {
    ultimaImpresion =
      millis();

    Serial.print("Angulo: ");
    Serial.print(anguloX, 2);

    Serial.print("  Error: ");
    Serial.print(errorReal, 2);

    Serial.print("  Gyro: ");
    Serial.print(velocidadGyroFiltrada, 2);

    Serial.print("  Control: ");
    Serial.print(salidaControl, 2);

    Serial.print("  PWM solicitado: ");
    Serial.print(pwmSolicitado);

    Serial.print("  PWM aplicado: ");
    Serial.println(pwm);
  }
}