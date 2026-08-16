<!-- GroundStation ELRS Backpack documentation -->

# 02 - Hardware

## Componentes principales

- ESP32 NodeMCU 38 pines USB-C.
- Pantalla TFT ST7789 1,9 pulgadas, 172 x 320 píxeles.
- Batería Li-Ion 18650.
- Porta batería 18650.
- Módulo TP4056 con protección.
- Step Down ajustable.
- Interruptor general.
- Pulsadores momentáneos.
- Gabinete impreso en 3D.

## Pines principales extraídos del firmware

| Elemento | GPIO |
|---|---:|
| BTN_PREV | GPIO32 |
| BTN_NEXT | GPIO33 |
| Backlight PWM | GPIO25 |
| TFT MOSI | GPIO23 |
| TFT SCLK | GPIO18 |
| TFT CS | GPIO5 |
| TFT DC | GPIO2 |
| TFT RST | GPIO4 |

## Observaciones

Los botones usan `INPUT_PULLUP`, por lo tanto cada pulsador debe cerrar contra GND.

El botón RESET no está programado en el sketch porque actúa físicamente entre EN y GND.
