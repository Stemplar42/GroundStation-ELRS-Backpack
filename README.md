# GroundStation ELRS Backpack

**GroundStation ELRS Backpack** es una estación de tierra portátil basada en ESP32 para recibir y visualizar telemetría desde un módulo **ExpressLRS TX Backpack** mediante WiFi/UDP.

La versión incluida en este repositorio corresponde a:

- Firmware: **GroundStation V2.5.1 Professional Stable Release**
- Release pública sugerida: **v1.0.0 Stable**
- Plataforma: **ESP32 NodeMCU 38 pines USB-C**
- Pantalla: **TFT ST7789 1,9 pulgadas, 172 x 320 píxeles**
- Autor del proyecto: **Marcelo Grippo**

![GroundStation logo](assets/logo/groundstation_logo.svg)

## Estado del proyecto

Esta versión fue validada en hardware real con alimentación autónoma mediante una batería 18650, TP4056 y Step Down regulado a **3,30 V exactos** hacia el pin **3.3V** de la ESP32.

## Funciones principales

- Conexión automática al primer SSID visible que comience con `ExpressLRS TX Backpack`.
- Recepción UDP local en puerto `14550`.
- Decodificación de tramas MSPv2 y CRSF.
- Visualización de telemetría en pantalla TFT ST7789.
- Interfaz de 5 pantallas: GENERAL, HORIZONTE, ENERGIA, GPS y DEBUG.
- Horizonte artificial con pitch, roll, heading, altitud y velocidad.
- HOME automático al salir del modo `WAIT`.
- Flecha HOME tipo OSD y distancia al HOME.
- GPS con pantalla normal y modo RECOVERY.
- Código QR de recuperación hacia Google Maps cuando hay pérdida de telemetría.
- Plus Code de recuperación.
- Detección automática de cantidad de celdas de batería.
- Color por tensión por celda.
- Control de brillo PWM en GPIO `25`.
- Barra de brillo en pantalla mediante pulsación larga de NEXT.
- Botón RESET físico conectado entre EN y GND.

## Hardware principal

- ESP32 NodeMCU 38 pines USB-C.
- TFT ST7789 1,9 pulgadas 172 x 320.
- Batería Li-Ion 18650.
- Porta batería 18650.
- Módulo TP4056 con protección.
- Step Down ajustable regulado a 3,30 V.
- Interruptor general.
- Pulsadores momentáneos para NEXT, PREV y RESET.
- Gabinete impreso en 3D.


## Archivos STL del gabinete

La Release v1.0.0 documenta el gabinete final **Case V18 SMOOTH_NO_REAR_LIP**.

Los archivos STL oficiales son:

```text
stl/GroundStation_Case_Body_v18_SMOOTH_NO_REAR_LIP.stl
stl/GroundStation_Case_RearCover_v18_SMOOTH_NO_REAR_LIP.stl
```


## Conexión rápida

### Pantalla TFT ST7789

| Señal TFT | GPIO ESP32 |
|---|---:|
| MOSI / SDA | GPIO23 |
| SCLK / SCL | GPIO18 |
| CS | GPIO5 |
| DC | GPIO2 |
| RST | GPIO4 |
| BL / LED | GPIO25 |
| VCC | 3.3V |
| GND | GND |

### Botones

| Función | Conexión |
|---|---|
| PREV / ANT | GPIO32 a GND, usando `INPUT_PULLUP` |
| NEXT | GPIO33 a GND, usando `INPUT_PULLUP` |
| RESET | EN a GND mediante pulsador momentáneo |

## Alimentación recomendada

No conectar una batería 18650 directamente al pin 3.3V de la ESP32.

Conexión validada:

```text
Batería 18650 -> TP4056 -> Step Down 3,30 V -> pin 3.3V ESP32
GND común entre TP4056, Step Down, ESP32 y TFT
```

Antes de conectar la placa, ajustar la salida del Step Down con multímetro a **3,30 V exactos**.

## Instalación del firmware

1. Instalar Arduino IDE.
2. Instalar soporte para placas ESP32.
3. Instalar las librerías indicadas en `docs/04_Firmware_Setup.md`.
4. Copiar `libraries_modified/TFT_eSPI/User_Setup.h` dentro de la librería TFT_eSPI instalada.
5. Abrir `firmware/GroundStation_V2_5_1_Professional_Stable/GroundStation_V2_5_1_Professional_Stable.ino`.
6. Seleccionar la placa ESP32 correspondiente.
7. Compilar y cargar.

## Uso básico

1. Encender la GroundStation.
2. Encender el módulo ExpressLRS TX Backpack.
3. Esperar que la estación encuentre el SSID `ExpressLRS TX Backpack*`.
4. Esperar recepción UDP y luego telemetría del avión.
5. Usar NEXT y PREV para navegar entre pantallas.
6. Mantener NEXT para abrir el control de brillo.
7. Usar RESET si es necesario reiniciar la ESP32.

## Documentación

- [Descripción general](docs/01_Overview.md)
- [Hardware](docs/02_Hardware.md)
- [Conexionado](docs/03_Wiring.md)
- [Instalación del firmware](docs/04_Firmware_Setup.md)
- [Manual de uso](docs/05_User_Manual.md)
- [Pantallas](docs/06_Display_Screens.md)
- [Sistema de alimentación](docs/07_Power_System.md)
- [Gabinete 3D](docs/08_3D_Case.md)
- [Solución de problemas](docs/09_Troubleshooting.md)

## Advertencia

Este proyecto es experimental y está orientado a uso maker, educativo y FPV recreativo. Verificar siempre alimentación, polaridad, cableado y estabilidad antes de utilizarlo en campo.
