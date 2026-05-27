# Software del péndulo invertido

Esta carpeta contiene el código fuente del **péndulo invertido autocontrolado**, desarrollado para ejecutarse sobre un **ESP32 DevKitC**.

## Contenido

- `PID_Basico.ino` → Implementación de un controlador PID discreto básico.
- `PID_IIR.ino` → Implementación del PID en forma de filtro IIR.
- `PID_Filtrado.ino` → Implementación del PID con filtrado en el término derivativo.

## Funcionalidades implementadas

- Lectura del sensor **MPU6050**.
- Estimación del ángulo de inclinación.
- Filtrado de señal mediante filtro complementario.
- Control PID discreto.
- Generación de señales para motores DC.
- Ajuste y depuración mediante monitor serie.

Durante el desarrollo se implementaron distintas versiones del controlador para comparar estabilidad, suavidad de respuesta y sensibilidad al ruido.

La arquitectura completa del sistema se describe en [`../documentacion.md`](../documentacion.md).
