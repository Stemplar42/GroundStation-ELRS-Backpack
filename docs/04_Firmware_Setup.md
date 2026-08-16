<!-- GroundStation ELRS Backpack documentation -->

# 04 - Instalación del firmware

## Librerías utilizadas por el sketch

El sketch incluye las siguientes librerías:

- `WiFi.h`
- `WiFiUdp.h`
- `TFT_eSPI.h`
- `Preferences.h`
- `math.h`
- `qrcode.h`

## Instalación recomendada

1. Instalar Arduino IDE.
2. Instalar soporte para ESP32 desde el gestor de placas.
3. Instalar la librería TFT_eSPI.
4. Instalar o disponer de la librería `qrcode.h` compatible con ESP32.
5. Copiar `libraries_modified/TFT_eSPI/User_Setup.h` sobre el `User_Setup.h` de la librería TFT_eSPI instalada.
6. Abrir el archivo `.ino` incluido en la carpeta `firmware`.
7. Seleccionar la placa ESP32 correspondiente.
8. Compilar.
9. Cargar el firmware.

## Configuración TFT_eSPI incluida

- Driver: ST7789.
- Resolución: 172 x 320.
- MOSI: GPIO23.
- SCLK: GPIO18.
- CS: GPIO5.
- DC: GPIO2.
- RST: GPIO4.
- Frecuencia SPI: 27 MHz.

## Configuración WiFi/UDP del firmware

- Prefijo SSID: `ExpressLRS TX Backpack`.
- Contraseña: `expresslrs`.
- Puerto UDP local: `14550`.
- Timeout UDP: `2000` ms.
- Timeout telemetría de aeronave: `2000` ms.
