// Prueba ESP32 + MPU6050
// Muestra posicion angular: Roll y Pitch en grados

#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <math.h>

Adafruit_MPU6050 mpu;

// Pines I2C ESP32
const int SDA_PIN = 21;
const int SCL_PIN = 22;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Prueba ESP32 + MPU6050 - Posicion angular");

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!mpu.begin()) {
    Serial.println("No se detecta el MPU6050");
    Serial.println("Revisa VCC, GND, SDA y SCL");
    while (1) {
      delay(10);
    }
  }

  Serial.println("MPU6050 detectado correctamente");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  delay(1000);
}

void loop() {
  sensors_event_t aceleracion, giroscopio, temperatura;
  mpu.getEvent(&aceleracion, &giroscopio, &temperatura);

  float ax = aceleracion.acceleration.x;
  float ay = aceleracion.acceleration.y;
  float az = aceleracion.acceleration.z;

  // Calculo de inclinacion usando acelerometro
  float roll = atan2(ay, az) * 180.0 / PI;
  float pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0 / PI;

  Serial.println("----- Posicion angular MPU6050 -----");

  Serial.print("Roll izquierda/derecha: ");
  Serial.print(roll);
  Serial.println(" grados");

  Serial.print("Pitch adelante/atras: ");
  Serial.print(pitch);
  Serial.println(" grados");

  Serial.print("Temperatura: ");
  Serial.print(temperatura.temperature);
  Serial.println(" C");

  Serial.println();

  delay(300);
}