#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

// ======================
// PINES ESP32
// ======================

#define SDA_PIN 32
#define SCL_PIN 33

#define PWMA 16
#define AIN2 17
#define AIN1 18

#define PWMB 22
#define BIN1 19
#define BIN2 21

#define STBY 23

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
// PID CON DERIVADA FILTRADA
// ======================

float setPoint = 0.0;

// Valores iniciales.
// Este suele ser el controlador más útil para el robot real.
float Kp = 18.0;
float Ki = 0.0;
float Kd = 0.8;

// Factor del filtro derivativo.
// Valores normales: entre 3 y 10.
float N = 5.0;

// Errores actual y anteriores.
float error0 = 0;   // e(k)
float error1 = 0;   // e(k-1)
float error2 = 0;   // e(k-2)

// Salida del controlador.
float output = 0;

// Coeficientes de la parte PI.
float A0 = 0;
float A1 = 0;

// Coeficientes de la parte derivativa sin filtrar.
float A0d = 0;
float A1d = 0;
float A2d = 0;

// Variables del filtro pasa baja.
float alpha = 0;
float alpha1 = 0;
float alpha2 = 0;

// Derivada sin filtrar y derivada filtrada.
float d0 = 0;
float d1 = 0;
float fd0 = 0;
float fd1 = 0;

int minPWM = 35;
int maxPWM = 255;

// ======================
// SETUP
// ======================

void setup() {

  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);

  mpu.initialize();

  if (mpu.testConnection()) {
    Serial.println("MPU6050 conectado correctamente");
  } else {
    Serial.println("Error al conectar el MPU6050");
    while (1);
  }

  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(STBY, OUTPUT);

  digitalWrite(STBY, HIGH);

  ledcSetup(channelA, freq, resolution);
  ledcSetup(channelB, freq, resolution);

  ledcAttachPin(PWMA, channelA);
  ledcAttachPin(PWMB, channelB);

  stopMotors();

  delay(1000);

  prevTime = millis();

  Serial.println("Sistema listo - PID con derivada filtrada");
}

// ======================
// LOOP PRINCIPAL
// ======================

void loop() {

  int16_t ax, ay, az;
  int16_t gx, gy, gz;

  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  unsigned long currentTime = millis();
  float dt = (currentTime - prevTime) / 1000.0;
  prevTime = currentTime;

  if (dt <= 0) dt = 0.001;

  // ======================
  // CÁLCULO DEL ÁNGULO
  // ======================

  accAngle = atan2(ay, az) * 180 / PI;
  gyroRate = gx / 131.0;

  // Filtro complementario para estimar el ángulo.
  angle = 0.98 * (angle + gyroRate * dt) + 0.02 * accAngle;

  // ======================
  // SEGURIDAD
  // ======================

  if (abs(angle) > 45) {
    stopMotors();
    resetPID();
    return;
  }

  // ======================
  // PID CON FILTRO DERIVATIVO
  // ======================

  actualizarCoeficientes(dt);

  // Actualizamos errores.
  error2 = error1;
  error1 = error0;
  error0 = setPoint - angle;

  // ----------------------
  // Parte PI
  // ----------------------
  // Se usa forma incremental para no tener una integral separada.
  output = output + A0 * error0 + A1 * error1;

  // ----------------------
  // Parte D sin filtrar
  // ----------------------
  // Primero se calcula la derivada discretizada.
  d1 = d0;
  d0 = A0d * error0 + A1d * error1 + A2d * error2;

  // ----------------------
  // Filtro pasa baja
  // ----------------------
  // Aquí se suaviza la parte derivativa.
  // Esto ayuda bastante porque el MPU6050 puede meter ruido.
  fd1 = fd0;
  fd0 = alpha1 * (d0 + d1) - alpha2 * fd1;

  // Salida final del PID.
  output = output + fd0;

  moveMotors(output);

  Serial.print("Angle: ");
  Serial.print(angle);

  Serial.print(" | Error: ");
  Serial.print(error0);

  Serial.print(" | PID filtrado: ");
  Serial.println(output);

  delay(5);
}

// ======================
// CÁLCULO DE COEFICIENTES
// ======================

void actualizarCoeficientes(float dt) {

  // Parte PI.
  A0 = Kp + Ki * dt;
  A1 = -Kp;

  // Parte derivativa discretizada.
  A0d = Kd / dt;
  A1d = -2.0 * Kd / dt;
  A2d = Kd / dt;

  // Cálculo del filtro pasa baja.
  // Se evita división entre cero si Kp vale 0.
  if (Kp != 0) {
    alpha = Kd / (Kp * N * dt);
  } else {
    alpha = 0;
  }

  alpha1 = alpha / (alpha + 1.0);
  alpha2 = (alpha - 1.0) / (alpha + 1.0);
}

// ======================
// REINICIO DEL CONTROLADOR
// ======================

void resetPID() {

  error0 = 0;
  error1 = 0;
  error2 = 0;

  output = 0;

  d0 = 0;
  d1 = 0;
  fd0 = 0;
  fd1 = 0;
}

// ======================
// CONTROL DE MOTORES
// ======================

void moveMotors(float speed) {

  int pwm = abs(speed);

  pwm = constrain(pwm, 0, maxPWM);

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