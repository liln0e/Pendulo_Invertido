# Software del péndulo invertido

En esta carpeta se incluye el código fuente y la documentación relacionada con el funcionamiento del **péndulo invertido autocontrolado**, incluyendo las distintas implementaciones del controlador PID en tiempo discreto.

## Contenido

Los siguientes archivos corresponden a las diferentes implementaciones del controlador empleadas durante el desarrollo del proyecto:

- `PID_Basico.ino` → Implementación del controlador PID discreto básico.
- `PID_IIR.ino` → Implementación del controlador PID en forma de filtro IIR.
- `PID_Filtrado.ino` → Implementación del controlador PID con filtro pasa baja en el término derivativo.

## Descripción

El software del sistema se encarga de controlar la lógica general de funcionamiento del péndulo invertido, permitiendo la integración entre sensores, electrónica de control y motores para mantener el equilibrio del robot.

Entre sus funciones principales se incluyen:

- Lectura del sensor **MPU6050**.
- Estimación del ángulo de inclinación mediante procesamiento de datos.
- Implementación del controlador PID en tiempo discreto.
- Generación de señales de control para los motores DC.
- Ajuste y pruebas de estabilidad del sistema.

Durante el desarrollo se implementaron varias versiones del controlador PID para comparar su comportamiento y evaluar mejoras en estabilidad, suavidad de respuesta y sensibilidad al ruido del sensor.

El código ha sido desarrollado para ejecutarse sobre un **ESP32 DevKitC**, integrando el control del sistema en tiempo real.
