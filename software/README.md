# Software del péndulo invertido

Código del **péndulo invertido autocontrolado** para un **ESP32 DevKitC**. Cada programa está en su propia carpeta, como pide el Arduino IDE.

## Controladores

Los tres usan el mismo sensor, los mismos motores, los mismos pines y el mismo periodo de control. **Solo cambia cómo se calcula la señal de salida.**

| Código | Qué hace |
|---|---|
| [`03_PID_derivada_filtrada_original`](03_PID_derivada_filtrada_original/03_PID_derivada_filtrada_original.ino) | **El controlador original del robot.** En realidad es un **PD**: la parte proporcional usa el error del ángulo, y la derivativa usa directamente la velocidad angular del giroscopio, suavizada con un filtro pasa-bajos para reducir el ruido. No tiene acción integral. |
| [`01_PID_directo`](01_PID_directo/01_PID_directo.ino) | **PID discreto en forma directa.** Calcula por separado la parte proporcional (error actual), la integral (suma de errores) y la derivativa (diferencia entre el error actual y el anterior), y luego las suma. Es la versión más fácil de entender. |
| [`02_PID_IIR`](02_PID_IIR/02_PID_IIR.ino) | **PID en forma incremental, como un filtro IIR.** Usa el error actual, los dos anteriores y la salida anterior con tres coeficientes (`A0`, `A1`, `A2`) calculados a partir de `Kp`, `Ki`, `Kd` y el periodo de muestreo. Necesita pocas operaciones, por eso es habitual en sistemas digitales. |

Las ganancias se ajustaron probando: **`Kp = 10`** y **`Kd = 2`**. En los controladores 01 y 02, `Ki` empieza en `0`, así que funcionan como un PD hasta que se le da otro valor. No se comportan exactamente igual que el original, porque calculan la derivada a partir del error y no con el giroscopio.

### Parámetros comunes

- Periodo de control: **5 ms** (200 Hz).
- Ángulo estimado con un **filtro complementario** (acelerómetro y giroscopio), con calibración del giroscopio al arrancar (500 lecturas).
- PWM de **20 kHz** y 8 bits, con un límite al cambio de PWM entre ciclos para evitar tirones.
- Los motores se paran si el robot se inclina más de **50°**.
- Las dos ruedas reciben siempre la misma velocidad: `moverMotores(velocidad)`.

## Pruebas de hardware

En [`pruebas/`](pruebas/) hay dos programas para comprobar cada parte por separado antes de cerrar el lazo de control:

- [`Prueba_ESP32_MPU6050`](pruebas/Prueba_ESP32_MPU6050/Prueba_ESP32_MPU6050.ino): comprueba la comunicación I2C con el sensor y muestra el *roll* y el *pitch* por el monitor serie. Sirve para ver hacia qué lado crece el ángulo al inclinar el robot con la mano. Usa la librería **Adafruit MPU6050**.
- [`Prueba_ESP32_TB6612FNG`](pruebas/Prueba_ESP32_TB6612FNG/Prueba_ESP32_TB6612FNG.ino): mueve los motores hacia delante y hacia atrás, y gira a cada lado, para comprobar el driver, el PWM y el sentido de giro de cada rueda.

## Pines (montaje final)

| Señal | GPIO | | Señal | GPIO |
|---|---|---|---|---|
| SDA (MPU6050) | 21 | | STBY | 26 |
| SCL (MPU6050) | 22 | | BIN1 | 27 |
| PWMA | 32 | | BIN2 | 14 |
| AIN1 | 25 | | PWMB | 12 |
| AIN2 | 33 | | | |

## Requisitos

- Arduino IDE con el **core de ESP32 versión 3.x** (el código usa `ledcAttach`, de la API nueva).
- Los controladores leen el MPU6050 directamente por I2C con `Wire`, así que no necesitan librerías externas.
- Solo la prueba del sensor necesita **Adafruit MPU6050** y **Adafruit Unified Sensor**.

La arquitectura completa del sistema se explica en [`../documentacion.md`](../documentacion.md).
