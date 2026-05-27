# Costes y materiales

En este documento se recoge el coste aproximado de los materiales empleados para la construcción del robot tipo **péndulo invertido (self-balancing robot)**.

El objetivo del proyecto fue desarrollar un sistema funcional utilizando **componentes económicos y fácilmente accesibles**, complementados con recursos proporcionados por la universidad, como la impresión 3D y parte del material electrónico.

---

# Desglose de materiales

| Componente | Cantidad | Precio (€) | Comentarios |
|------------|-----------|-------------|--------------|
| **ESP32 DevKitC** | 1 | 4,49 | Microcontrolador principal |
| **MPU6050 (GY-521)** | 1 | 1,81 | Sensor IMU (acelerómetro + giroscopio) |
| **Motores TT + ruedas** | 2 motores + 2 ruedas | 3,99 | Sistema de tracción |
| **Driver TB6612FNG** | 1 | Incluido | Control de motores |
| **LM7805** | 1 | 1,45 | Regulador de tensión |
| **PCB personalizada (JLCPCB)** | 1 | ≈ 2,87 | Coste real aproximado por unidad |
| **Pila plana 3R12 4,5 V** | 1 | 5,99 | Alimentación del sistema |
| **Condensador 0.1 µF** | 1 | 0,00 | Material de laboratorio |
| **Condensador equivalente 0.33 µF** | 1 conjunto | 0,00 | Implementado con 0.22 µF + 0.11 µF |
| **Borneras de 2 pines** | 3 | 0,00 | Material de laboratorio |
| **Chasis impreso 3D** | 1 | 0,00 | Fabricado en la universidad |

---

# Coste total aproximado

Considerando únicamente los materiales realmente utilizados en el robot:

**Coste estimado total del proyecto:** **≈ 20,60 €**

> Este valor no incluye material proporcionado gratuitamente por la universidad ni costes indirectos de fabricación.

---

# Notas

- La **PCB fue diseñada específicamente para el proyecto** y fabricada mediante **JLCPCB**. El coste reflejado corresponde a una unidad individual del pedido realizado.
- El condensador de **0.33 µF no estaba disponible**, por lo que se implementó correctamente conectando **0.22 µF y 0.11 µF en paralelo**.
- El **chasis fue diseñado e impreso en 3D en la universidad**, reduciendo significativamente el coste total del proyecto.
- Se priorizó el uso de **componentes económicos y reutilizables**, manteniendo un equilibrio entre funcionalidad y bajo coste.
