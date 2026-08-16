# Pinout GroundStation ELRS Backpack

## TFT ST7789

| Función | GPIO |
|---|---:|
| MOSI / SDA | GPIO23 |
| SCLK / SCL | GPIO18 |
| CS | GPIO5 |
| DC | GPIO2 |
| RST | GPIO4 |
| BL / LED / BLK | GPIO25 |

## Botones

| Función | GPIO / conexión |
|---|---|
| PREV / ANT | GPIO32 a GND |
| NEXT | GPIO33 a GND |
| RESET | EN a GND |

## Alimentación

| Señal | Conexión |
|---|---|
| +3,30 V regulados | Pin 3.3V ESP32 |
| GND | GND ESP32 |
| VCC TFT | 3.3V |
| GND TFT | GND común |
