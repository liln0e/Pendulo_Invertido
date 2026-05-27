# Diseño electrónico del péndulo invertido

Esta carpeta contiene los archivos del diseño electrónico del **péndulo invertido autocontrolado**, desarrollados en **KiCad**.

## Contenido

- `Pendulo_Inverso.kicad_sch` → Esquemático electrónico del sistema.
- `Pendulo_Inverso.kicad_pcb` → Diseño de la PCB.
- `Pendulo_Inverso.kicad_pro` → Proyecto completo de KiCad.

## Descripción

La electrónica integra los principales componentes del robot:

- **ESP32 DevKitC** como unidad de control.
- **MPU6050** para la medición de inclinación.
- **TB6612FNG** para el control de motores.
- **LM7805** para regulación de tensión.
- Conectores de alimentación y motores.

El diseño fue realizado sobre una **PCB personalizada** con el objetivo de reducir cableado, facilitar el ensamblaje y mejorar la integración del sistema.

La explicación técnica detallada puede consultarse en [`../documentacion.md`](../documentacion.md).
