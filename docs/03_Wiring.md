<!-- GroundStation ELRS Backpack documentation -->

# 03 - Conexionado

## Alimentación

```text
Batería 18650 positivo -> B+ TP4056
Batería 18650 negativo -> B- TP4056
OUT+ TP4056 -> entrada positiva Step Down
OUT- TP4056 -> entrada negativa Step Down
Salida Step Down +3,30 V -> pin 3.3V ESP32
Salida Step Down GND -> GND ESP32
GND común -> TFT, botones y módulos
```

## Pantalla TFT ST7789

| Pin pantalla | Conectar a ESP32 |
|---|---:|
| VCC | 3.3V |
| GND | GND |
| MOSI / SDA | GPIO23 |
| SCK / SCL | GPIO18 |
| CS | GPIO5 |
| DC | GPIO2 |
| RST | GPIO4 |
| BL / LED / BLK | GPIO25 |

## Botones

| Botón | Conexión |
|---|---|
| PREV / ANT | GPIO32 a GND |
| NEXT | GPIO33 a GND |
| RESET | EN a GND |

## Plano lógico ASCII

```text
                 +-------------------------+
                 |       ESP32 NodeMCU     |
                 |                         |
TFT MOSI --------| GPIO23                  |
TFT SCLK --------| GPIO18                  |
TFT CS ----------| GPIO5                   |
TFT DC ----------| GPIO2                   |
TFT RST ---------| GPIO4                   |
TFT BL ----------| GPIO25 PWM              |
PREV ------------| GPIO32 -> pulsador -> GND |
NEXT ------------| GPIO33 -> pulsador -> GND |
RESET -----------| EN -> pulsador -> GND   |
3.30 V ----------| 3.3V                    |
GND -------------| GND                     |
                 +-------------------------+
```
