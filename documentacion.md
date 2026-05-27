# Documentación técnica del proyecto

Este documento recoge la información técnica general del proyecto de **péndulo invertido autocontrolado (self-balancing robot)**, incluyendo la arquitectura del sistema, el diseño mecánico, la electrónica y el software implementado para el control del robot.

El objetivo principal del sistema es mantener el equilibrio de forma autónoma mediante un controlador **PID**, capaz de corregir la inclinación del robot actuando sobre los motores en tiempo real.

---

# Arquitectura general del sistema

El robot implementa un sistema de control en lazo cerrado compuesto por las siguientes etapas:

1. **Captura de inclinación** mediante una unidad IMU (**MPU6050**).
2. **Estimación del ángulo** utilizando acelerómetro y giroscopio.
3. **Filtrado de señal** mediante un **filtro complementario**.
4. **Cálculo de la acción de control PID**.
5. **Actuación sobre los motores** para recuperar la posición vertical.

El flujo simplificado del sistema es el siguiente:

```txt
MPU6050
    ↓
Estimación del ángulo
    ↓
Filtro complementario
    ↓
Control PID
    ↓
Driver TB6612FNG
    ↓
Motores
    ↓
Corrección de la inclinación
```

El sistema realiza un procesamiento continuo de los datos de inclinación y ajusta dinámicamente la velocidad de los motores para compensar perturbaciones y mantener la estabilidad.

---

# Diseño mecánico

La parte mecánica del robot se diseñó con el objetivo de obtener una estructura **compacta, ligera y estable**, favoreciendo un **centro de gravedad bajo** para mejorar el comportamiento dinámico del sistema.

El chasis fue modelado en **Fusion 360** y fabricado mediante **impresión 3D**, permitiendo adaptar la geometría a las necesidades específicas del proyecto y facilitar la integración de los componentes electrónicos.

Durante el desarrollo se realizaron distintas iteraciones del diseño para optimizar:

- Distribución del peso.
- Posicionamiento de la electrónica.
- Integración de motores y ruedas.
- Robustez estructural.
- Facilidad de ensamblaje.

El diseño final permite alojar de forma compacta todos los elementos del sistema, manteniendo un acceso sencillo a la electrónica y facilitando futuras modificaciones.

---

# Diseño electrónico

La electrónica del robot fue diseñada sobre una **PCB personalizada desarrollada en KiCad**, permitiendo integrar los distintos módulos del sistema en una única placa.

Los componentes principales utilizados son:

- **ESP32 DevKitC** → Unidad principal de control.
- **MPU6050** → Sensor IMU para estimación de inclinación.
- **TB6612FNG** → Driver de control de motores.
- **LM7805** → Regulación de tensión.
- Conectores de alimentación y motores.

El diseño electrónico se planteó con varios objetivos:

- Reducir el cableado del sistema.
- Mejorar la integración entre componentes.
- Facilitar el ensamblaje del robot.
- Incrementar la robustez del conjunto.

La PCB integra alimentación, control y conexiones de potencia necesarias para el funcionamiento autónomo del sistema.

---

# Diseño software

El software del robot se ejecuta sobre una **ESP32**, encargándose de procesar la información procedente del sensor y aplicar la lógica de control necesaria para mantener el equilibrio.

Las funcionalidades principales implementadas son:

- Lectura del sensor **MPU6050**.
- Estimación del ángulo de inclinación.
- Aplicación de un **filtro complementario**.
- Implementación del **control PID discreto**.
- Generación de señales de control para los motores.
- Ajuste y depuración mediante monitor serie.

Durante el desarrollo se implementaron distintas versiones del controlador PID con el objetivo de comparar comportamiento, estabilidad y sensibilidad al ruido del sistema.

La lógica simplificada del software es:

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

---

# Organización del repositorio

El proyecto se organiza en distintas carpetas para separar la documentación y facilitar el desarrollo:

```txt
Proyecto_Pendulo_Invertido/
│
├── README.md
├── costes.md
├── documentacion.md
│
├── assets/
├── electronica/
├── mecanica/
└── software/
```

Cada carpeta contiene información específica del proyecto:

- `electronica/` → Diseño PCB, esquemáticos y archivos de KiCad.
- `mecanica/` → Modelado 3D y archivos de fabricación.
- `software/` → Código fuente y pruebas del controlador.
- `assets/` → Recursos visuales del proyecto.
