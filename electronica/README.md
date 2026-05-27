# Diseño electrónico del péndulo invertido

En esta carpeta se incluye la documentación electrónica necesaria para el desarrollo del **péndulo invertido autocontrolado**, basado en un sistema de equilibrio mediante control PID.

## Contenido

Los siguientes archivos corresponden al diseño electrónico realizado en **KiCad**:

- `nombre_proyecto.kicad_sch` → Esquemático electrónico del circuito.
- `nombre_proyecto.kicad_pcb` → Diseño de la PCB del sistema.
- `nombre_proyecto.kicad_pro` → Archivo del proyecto de KiCad.

## Diseño electrónico

La PCB ha sido diseñada para integrar los componentes electrónicos necesarios para el funcionamiento del sistema, incluyendo:

- Alimentación del sistema.
- Integración del **ESP32 DevKitC** como unidad principal de control.
- Conexión del sensor **MPU6050** para la medición de aceleración e inclinación.
- Integración del controlador de motores **TB6612FNG**.
- Conexiones para motores DC con reductora.
- Regulación de tensión mediante **LM7805** y componentes auxiliares.

El diseño electrónico se ha realizado utilizando **KiCad**, permitiendo organizar el esquema eléctrico, la distribución de componentes y la fabricación de una PCB personalizada para el proyecto.

Durante el desarrollo se realizaron distintas iteraciones del circuito para mejorar la integración de componentes y adaptar el sistema a las necesidades del control del péndulo invertido.

Las imágenes del esquemático, diseño PCB y placa fabricada se encuentran disponibles dentro de la documentación del proyecto.
