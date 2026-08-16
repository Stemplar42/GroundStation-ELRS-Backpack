<!-- GroundStation ELRS Backpack documentation -->

# 09 - Solución de problemas

## La ESP32 no enciende

- Comprobar batería 18650.
- Comprobar interruptor.
- Medir salida del Step Down.
- Confirmar 3,30 V en el pin 3.3V.
- Confirmar GND común.

## La pantalla no muestra imagen

- Verificar VCC y GND de la TFT.
- Verificar MOSI, SCLK, CS, DC y RST.
- Confirmar que la pantalla sea ST7789 172 x 320.
- Confirmar instalación correcta del `User_Setup.h` incluido.

## El brillo no cambia

- Verificar que BL/LED/BLK de la pantalla vaya a GPIO25.
- Verificar soldadura del pin de backlight.
- Mantener NEXT para abrir modo brillo.

## No conecta al Backpack

- Confirmar que el SSID comience con `ExpressLRS TX Backpack`.
- Confirmar contraseña `expresslrs`.
- Reiniciar Backpack y GroundStation.
- Acercar la estación al transmisor.

## UDP LOST

- Indica que no llegaron paquetes UDP dentro del timeout.
- Verificar WiFi y Backpack.

## TEL LOST

- Indica que hay UDP, pero no telemetría útil de aeronave dentro del timeout.
- Verificar que el avión transmita datos CRSF/MSPv2.

## Lecturas GPS nulas

- Esperar al menos 4 satélites.
- Verificar que el GPS del avión tenga fix.
- Verificar que la telemetría esté llegando correctamente.
