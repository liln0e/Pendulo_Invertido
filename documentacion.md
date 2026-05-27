# Documentación técnica del proyecto

Este documento recoge la información técnica general del proyecto de **péndulo invertido (self-balancing robot)**, incluyendo el diseño mecánico, electrónico y software implementado para el control del sistema.

El objetivo del robot es mantener el equilibrio de forma autónoma mediante un sistema de control basado en un **PID**, capaz de corregir la inclinación del robot actuando sobre los motores.

---

# Arquitectura general del sistema

El robot se basa en un sistema de control en lazo cerrado compuesto por:

1. **Captura de inclinación** mediante un sensor IMU (**MPU6050**).
2. **Estimación del ángulo** utilizando acelerómetro y giroscopio.
3. **Filtrado de señal** mediante un **filtro complementario**.
4. **Cálculo de la acción de control PID**.
5. **Control de velocidad y dirección de los motores** para recuperar la posición vertical.

El flujo simplificado del sistema es:

```txt
MPU6050 → Estimación del ángulo → Filtro complementario
→ Control PID → Driver TB6612FNG → Motores
→ Corrección de la inclinación
```

---

# Diseño mecánico

La parte mecánica del robot se centra en conseguir una estructura **estable, compacta y ligera**, favoreciendo un **centro de gravedad bajo** para mejorar el equilibrio.

El chasis fue diseñado mediante modelado 3D e impreso en la universidad, adaptándose a las necesidades específicas del proyecto.

Entre los aspectos principales del diseño se incluyen:

- Integración de la PCB personalizada.
- Soporte para motores y ruedas.
- Distribución compacta de componentes.
- Optimización del peso y estabilidad.

## Archivos incluidos

Dentro de la carpeta `mecanica/` se incluyen:

- Modelos 3D del chasis.
- Archivos preparados para impresión.

---

# Diseño electrónico

La electrónica del robot fue diseñada sobre una **PCB personalizada desarrollada en KiCad**, permitiendo integrar todos los elementos necesarios en una única placa.

Los componentes principales del sistema son:

- **ESP32 DevKitC** → Microcontrolador principal.
- **MPU6050** → Sensor de inclinación.
- **TB6612FNG** → Driver de control de motores.
- **LM7805** → Regulación de tensión.
- Conectores para motores y alimentación.

La PCB fue diseñada buscando:

- Reducir cableado.
- Facilitar el ensamblaje.
- Mejorar la robustez del sistema.
- Integrar todos los módulos principales.

## Archivos incluidos

Dentro de la carpeta `electronica/` se incluyen:

- Esquemático electrónico.
- Diseño de PCB.
- Proyecto de KiCad.

---

# Diseño software

El software del robot se ejecuta sobre una **ESP32**, encargándose de procesar la información del sensor y aplicar la lógica de control necesaria para mantener el equilibrio.

Las principales funciones implementadas son:

- Lectura del **MPU6050**.
- Estimación del ángulo del robot.
- Aplicación de un **filtro complementario**.
- Cálculo del **control PID**.
- Control de motores mediante el **TB6612FNG**.
- Ajuste y depuración mediante **monitor serie**.

La lógica de funcionamiento sigue el siguiente esquema:

```txt
Lectura IMU
    ↓
Estimación de inclinación
    ↓
Filtro complementario
    ↓
Cálculo PID
    ↓
Control de motores
    ↓
Corrección del equilibrio
```

## Archivos incluidos

Dentro de la carpeta `software/` se incluyen:

- Código principal del robot.
- Versiones de prueba y ajuste del PID.

---

# Organización del repositorio

El proyecto se organiza en distintas carpetas para facilitar la documentación y el desarrollo:

```txt
Proyecto_Pendulo_Invertido/
│
├── README.md
├── costes.md
├── documentacion.md
│
├── electronica/
├── mecanica/
├── software/
└── assets/
```

- **electronica/** → Diseño PCB y documentación electrónica.
- **mecanica/** → Modelado e impresión del chasis.
- **software/** → Código del robot y control PID.
- **assets/** → Imágenes y recursos gráficos del proyecto.
