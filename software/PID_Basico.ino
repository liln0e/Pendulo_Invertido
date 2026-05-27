#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

// ======================
// PINES ESP32
// ======================

// Pines I2C del MPU6050
#define SDA_PIN 32
#define SCL_PIN 33

// Pines del driver TB6612FNG
#define PWMA 16
#define AIN2 17
#define AIN1 18

#define PWMB 22
#define BIN1 19
#define BIN2 21

#define STBY 23

// Configuración PWM del ESP32
const int freq = 20000;
const int resolution = 8;

const int channelA = 0;
const int channelB = 1;

// ======================
// VARIABLES MPU6050
// ======================

float accAngle = 0;
float gyroRate = 0;
float angle = 0;

unsigned long prevTime = 0;

// ======================
// PID BÁSICO
// ======================

// El setPoint es el ángulo deseado.
// Para el péndulo invertido, el equilibrio está en 0 grados.
float setPoint = 0.0;

// Valores iniciales del PID.
// Estos valores se tienen que ajustar probando el robot real.
float Kp = 18.0;
float Ki = 0.0;
float Kd = 0.8;

float error = 0;
float previousError = 0;
float integral = 0;
float derivative = 0;

float output = 0;

// Límites de PWM
int minPWM = 35;
int maxPWM = 255;

// ======================
// SETUP
// ======================

void setup() {

  Serial.begin(115200);

  // Iniciamos comunicación I2C con el MPU6050.
  Wire.begin(SDA_PIN, SCL_PIN);

  // Inicializamos el sensor.
  mpu.initialize();

  if (mpu.testConnection()) {
    Serial.println("MPU6050 conectado correctamente");
  } else {
    Serial.println("Error al conectar el MPU6050");
    while (1);
  }

  // Configuración de pines del driver de motores.
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(STBY, OUTPUT);

  // Activamos el driver.
  digitalWrite(STBY, HIGH);

  // Configuración PWM del ESP32.
  ledcSetup(channelA, freq, resolution);
  ledcSetup(channelB, freq, resolution);

  ledcAttachPin(PWMA, channelA);
  ledcAttachPin(PWMB, channelB);

  stopMotors();

  delay(1000);

  prevTime = millis();

  Serial.println("Sistema listo - PID basico");
}

// ======================
// LOOP PRINCIPAL
// ======================

void loop() {

  int16_t ax, ay, az;
  int16_t gx, gy, gz;

  // Leemos acelerómetro y giroscopio.
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Calculamos el tiempo de muestreo.
  unsigned long currentTime = millis();
  float dt = (currentTime - prevTime) / 1000.0;
  prevTime = currentTime;

  // Protección por si dt sale cero.
  if (dt <= 0) dt = 0.001;

  // ======================
  // CÁLCULO DEL ÁNGULO
  // ======================

  // Ángulo obtenido con el acelerómetro.
  accAngle = atan2(ay, az) * 180 / PI;

  // Velocidad angular del giroscopio.
  // El factor 131 depende de la escala usada por el MPU6050.
  gyroRate = gx / 131.0;

  // Filtro complementario.
  // Se usa principalmente el giroscopio, pero el acelerómetro corrige la deriva.
  angle = 0.98 * (angle + gyroRate * dt) + 0.02 * accAngle;

  // ======================
  // SEGURIDAD
  // ======================

  // Si el robot se inclina demasiado, se paran los motores.
  if (abs(angle) > 45) {
    stopMotors();

    // Reinicio del PID para que no se acumule integral al estar caído.
    integral = 0;
    previousError = 0;

    return;
  }

  // ======================
  // PID DISCRETO BÁSICO
  // ======================

  // Error entre el ángulo deseado y el ángulo medido.
  error = setPoint - angle;

  // Parte integral.
  // Se suma el error multiplicado por el tiempo de muestreo.
  integral += error * dt;

  // Limitamos la integral para evitar wind-up.
  integral = constrain(integral, -100, 100);

  // Parte derivativa.
  // Se calcula la variación del error entre dos muestras.
  derivative = (error - previousError) / dt;

  // Salida total del PID.
  output = Kp * error + Ki * integral + Kd * derivative;

  // Guardamos el error actual para el siguiente ciclo.
  previousError = error;

  // Aplicamos la salida del PID a los motores.
  moveMotors(output);

  // Datos por serial para observar el comportamiento.
  Serial.print("Angle: ");
  Serial.print(angle);

  Serial.print(" | Error: ");
  Serial.print(error);

  Serial.print(" | PID basico: ");
  Serial.println(output);

  delay(5);
}

// ======================
// CONTROL DE MOTORES
// ======================

void moveMotors(float speed) {

  int pwm = abs(speed);

  pwm = constrain(pwm, 0, maxPWM);

  // Los motores no se mueven bien con valores muy bajos,
  // por eso se fuerza un PWM mínimo.
  if (pwm > 0 && pwm < minPWM) {
    pwm = minPWM;
  }

  if (speed > 0) {

    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);

    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);

    ledcWrite(channelA, pwm);
    ledcWrite(channelB, pwm);
  }

  else if (speed < 0) {

    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);

    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);

    ledcWrite(channelA, pwm);
    ledcWrite(channelB, pwm);
  }

  else {
    stopMotors();
  }
}

// ======================
// PARAR MOTORES
// ======================

void stopMotors() {

  ledcWrite(channelA, 0);
  ledcWrite(channelB, 0);

  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, LOW);

  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, LOW);
}