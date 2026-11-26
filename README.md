# Péndulo Invertido Autocontrolado

<p align="center">
  <img src="assets/robot_ensamblado.jpeg" alt="Robot péndulo invertido ensamblado" width="420">
</p>

<p align="center">
  <b>Robot autoequilibrado de dos ruedas con ESP32 y control PID</b><br>
  Grado en Robótica · Escola Politécnica Superior de Enxeñaría (EPSE) · USC<br>
  Asignatura: <a href="https://www.usc.gal/es/estudios/grados/ingenieria-arquitectura/grado-robotica-2a-edicion/20262027/teoria-control-19946-19042-2-105704">Teoría de control</a>
</p>

<div align="center">

[Web del proyecto](https://liln0e.github.io/Pendulo_Invertido/) · [Documentación técnica](documentacion.md) · [Costes (≈ 20,60 €)](costes.md) · [Fotos](fotos.md) · [Código](software/README.md)

</div>

---

## Descripción

Proyecto de la asignatura **Teoría de control** del Grado en Robótica.

Robot de dos ruedas que **se mantiene en equilibrio por sí solo**. Un sensor **MPU6050** mide la inclinación, un **filtro complementario** junta los datos del acelerómetro y del giroscopio, y un controlador **PID** que corre en una **ESP32** mueve los motores en tiempo real para devolver el robot a la vertical.

- **Electrónica:** PCB propia diseñada en KiCad y fabricada en JLCPCB.
- **Mecánica:** chasis modelado en Fusion 360 e impreso en 3D, con el centro de gravedad bajo.
- **Software:** tres versiones del controlador PID para comparar su estabilidad y cómo responden al ruido.
- **Coste:** unos **20,60 €** en materiales.

## Cómo funciona

```txt
MPU6050 → Estimación del ángulo → Filtro complementario → PID → TB6612FNG → Motores
   ↑                                                                          │
   └──────────────────────────── corrección de la inclinación ←───────────────┘
```

| Controlador | Qué hace |
|---|---|
| [`03_PID_derivada_filtrada_original`](software/03_PID_derivada_filtrada_original/03_PID_derivada_filtrada_original.ino) | **El que usó el robot.** Un PD: la derivada sale del giroscopio, suavizada con un filtro pasa-bajos. `Kp = 10`, `Kd = 2`. |
| [`01_PID_directo`](software/01_PID_directo/01_PID_directo.ino) | PID discreto con las acciones P, I y D calculadas por separado. |
| [`02_PID_IIR`](software/02_PID_IIR/02_PID_IIR.ino) | El mismo PID escrito como una ecuación incremental (filtro IIR). |

El bucle de control se ejecuta cada **5 ms** (200 Hz). Hay vídeos del robot equilibrándose con cada controlador en la [página de fotos y vídeos](fotos.md).

## Problemas y soluciones

- **Salidas del ESP32 mal asignadas en la PCB:** al compararla con el pinout vimos que el SCL y el SDA del sensor iban a IO0 e IO2 en vez de a GPIO22 y GPIO21, los pines I2C. Cortamos las pistas, hicimos puentes con hilo de cobre hasta los pines correctos y los aislamos con celo.
- **Salida del driver quemada:** un cortocircuito quemó la salida del TB6612FNG de una de las ruedas. Como las dos ruedas están en el mismo eje y siempre hacen lo mismo, conectamos los dos motores a la salida que funcionaba, y el robot siguió equilibrándose.

Más detalles en la [documentación técnica](documentacion.md#problemas-encontrados-y-soluciones).

## Hardware

| Componente | Función |
|---|---|
| ESP32 DevKitC | Microcontrolador principal |
| MPU6050 (GY-521) | IMU: acelerómetro y giroscopio |
| TB6612FNG | Driver de los motores |
| LM7805 | Regulador de tensión |
| 2 motores TT con ruedas | Tracción |
| Pila 3R12 de 4,5 V | Alimentación |

## Del diseño al robot

| Esquemático (KiCad) | Diseño de la PCB | PCB fabricada | Montaje |
|---|---|---|---|
| ![Esquemático](assets/esquematico_kicad.jpg) | ![Diseño PCB](assets/pcb_diseno_kicad.jpeg) | ![PCB fabricada](assets/pcb_fabricada.jpeg) | ![Montaje](assets/montaje_componentes.jpeg) |

## Estructura del repositorio

```txt
Pendulo_Invertido/
├── README.md
├── LICENSE
├── documentacion.md   → Explicación técnica completa
├── costes.md          → Desglose de materiales y coste
├── fotos.md           → Fotos y vídeos del proceso
├── _config.yml        → Configuración de la web (Jekyll + GitHub Pages)
├── assets/            → Fotos, pinout del ESP32 y vídeos (assets/videos/)
├── electronica/       → Esquemático y PCB en KiCad
├── mecanica/          → Modelo 3D (.3mf) y G-code del chasis
└── software/          → Los tres controladores y pruebas de hardware (pruebas/)
```

## Cómo usarlo

1. Instala en el Arduino IDE el **core de ESP32 versión 3.x**. Los controladores leen el MPU6050 directamente por I2C, así que no necesitan librerías externas.
2. Abre la carpeta del controlador que quieras de [`software/`](software/README.md), selecciona la placa **ESP32 Dev Module** y súbelo.
3. Para ajustar el controlador, cambia `Kp`, `Ki` y `Kd` al principio del código (por defecto `Kp = 10`, `Kd = 2` y `Ki = 0`). Con el monitor serie puedes ver el ángulo mientras pruebas.
4. Si es la primera vez, usa antes los [programas de prueba](software/README.md#pruebas-de-hardware) para comprobar el sensor y los motores por separado.

## Equipo

| Integrante |
|---|
| **Carla Freire Muíño** · [@carlafreeire](https://github.com/carlafreeire) |
| **Noelia Castro Rodríguez** · [@liln0e](https://github.com/liln0e) |

## Licencia

Proyecto académico publicado bajo la [licencia MIT](./LICENSE).
