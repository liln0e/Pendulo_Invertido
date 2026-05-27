# Diseño mecánico del péndulo invertido

En esta carpeta se incluye la documentación mecánica necesaria para el desarrollo del **péndulo invertido autocontrolado**, incluyendo el diseño del chasis y las piezas fabricadas para integrar el sistema.

## Contenido

Los siguientes archivos corresponden al diseño e impresión de la estructura mecánica del robot:

- `teoria_cotrol_3d.3mf` → Modelo 3D original del chasis del péndulo invertido.
- `nombre_pieza.gcode.3mf` → Archivos preparados para impresión 3D.

## Diseño mecánico

La parte mecánica del proyecto se centra en el diseño de un chasis capaz de mantener un **centro de gravedad bajo** y una **distribución equilibrada del peso**, factores fundamentales para mejorar la estabilidad del péndulo invertido.

El diseño fue realizado en **Fusion 360** y fabricado mediante **impresión 3D en la universidad**, permitiendo adaptar la estructura a las necesidades del sistema y reducir costes de fabricación.

Durante el desarrollo se realizaron distintas iteraciones del chasis para mejorar el comportamiento del robot, ajustando la posición de los componentes electrónicos y la distribución del peso hasta conseguir una configuración más estable.

El diseño mecánico se planteó para integrar de forma compacta los principales elementos del sistema:

- **ESP32 DevKitC**
- **MPU6050**
- **TB6612FNG**
- Motores DC con reductora y ruedas
- Sistema de alimentación

El objetivo del diseño fue obtener una estructura funcional, ligera y fácilmente ensamblable, permitiendo un acceso sencillo a la electrónica y facilitando futuras modificaciones del sistema.
