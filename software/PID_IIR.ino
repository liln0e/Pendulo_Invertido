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
// PID COMO FILTRO IIR
// ======================

float setPoint = 0.0;

// Ganancias iniciales.
// Se pueden cambiar según las pruebas del robot.
float Kp = 18.0;
float Ki = 0.0;
float Kd = 0.8;

// Errores actual y anteriores.
float error0 = 0;   // e(k)
float error1 = 0;   // e(k-1)
float error2 = 0;   // e(k-2)

// Coeficientes del PID discreto en forma IIR.
float A0 = 0;
float A1 = 0;
float A2 = 0;

// Salida del controlador.
float output = 0;

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

  Serial.println("Sistema listo - PID IIR");
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
  // PID EN FORMA IIR
  // ======================

  // Actualizamos los coeficientes con el dt real.
  actualizarCoeficientes(dt);

  // Desplazamos los errores.
  // Lo que antes era e(k-1) pasa a ser e(k-2), etc.
  error2 = error1;
  error1 = error0;

  // Error actual.
  error0 = setPoint - angle;

  // Ecuación recursiva:
  // u(k) = u(k-1) + A0 e(k) + A1 e(k-1) + A2 e(k-2)
  output = output + A0 * error0 + A1 * error1 + A2 * error2;

  moveMotors(output);

  Serial.print("Angle: ");
  Serial.print(angle);

  Serial.print(" | Error: ");
  Serial.print(error0);

  Serial.print(" | PID IIR: ");
  Serial.println(output);

  delay(5);
}

// ======================
// CÁLCULO DE COEFICIENTES
// ======================

void actualizarCoeficientes(float dt) {

  // Estos coeficientes salen de discretizar el PID continuo.
  A0 = Kp + Ki * dt + Kd / dt;
  A1 = -Kp - 2.0 * Kd / dt;
  A2 = Kd / dt;
}

// ======================
// REINICIO DEL CONTROLADOR
// ======================

void resetPID() {

  error0 = 0;
  error1 = 0;
  error2 = 0;

  output = 0;
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