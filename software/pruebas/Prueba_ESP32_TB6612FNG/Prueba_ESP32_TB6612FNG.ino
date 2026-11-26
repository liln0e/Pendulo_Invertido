// Prueba ESP32 + TB6612FNG
// Compatible con ESP32 Arduino Core 3.x

// Pines corregidos para coincidir con el montaje final del péndulo
// (los mismos que usan los controladores 01, 02 y 03).

// Pines Motor A
const int PWMA = 32;
const int AIN1 = 25;
const int AIN2 = 33;

// Pines Motor B
const int PWMB = 12;
const int BIN1 = 27;
const int BIN2 = 14;

// Standby
const int STBY = 26;

// PWM
const int freq = 1000;
const int resolution = 8; // 8 bits = 0 a 255

void setup() {
  Serial.begin(115200);

  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(STBY, OUTPUT);

  // Nueva API LEDC para ESP32 Arduino Core 3.x
  ledcAttach(PWMA, freq, resolution);
  ledcAttach(PWMB, freq, resolution);

  digitalWrite(STBY, HIGH);

  Serial.println("Prueba ESP32 + TB6612FNG iniciada");
}

void loop() {
  Serial.println("Adelante");
  motorA(180);
  motorB(180);
  delay(2000);

  Serial.println("Freno");
  brake();
  delay(1000);

  Serial.println("Atras");
  motorA(-180);
  motorB(-180);
  delay(2000);

  Serial.println("Freno");
  brake();
  delay(1000);

  Serial.println("Giro izquierda");
  motorA(-160);
  motorB(160);
  delay(1500);

  Serial.println("Freno");
  brake();
  delay(1000);

  Serial.println("Giro derecha");
  motorA(160);
  motorB(-160);
  delay(1500);

  Serial.println("Freno");
  brake();
  delay(2000);
}

// velocidad: -255 a 255
void motorA(int velocidad) {
  velocidad = constrain(velocidad, -255, 255);

  if (velocidad > 0) {
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
    ledcWrite(PWMA, velocidad);
  } else if (velocidad < 0) {
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
    ledcWrite(PWMA, -velocidad);
  } else {
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, LOW);
    ledcWrite(PWMA, 0);
  }
}

// velocidad: -255 a 255
void motorB(int velocidad) {
  velocidad = constrain(velocidad, -255, 255);

  if (velocidad > 0) {
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
    ledcWrite(PWMB, velocidad);
  } else if (velocidad < 0) {
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);
    ledcWrite(PWMB, -velocidad);
  } else {
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, LOW);
    ledcWrite(PWMB, 0);
  }
}

void brake() {
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, HIGH);
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, HIGH);

  ledcWrite(PWMA, 0);
  ledcWrite(PWMB, 0);
}