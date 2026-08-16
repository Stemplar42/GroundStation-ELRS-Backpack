# Changelog

## v1.0.0 Stable - GroundStation V2.5.1 Professional Stable Release

Primera release pública estable del proyecto GroundStation ELRS Backpack.

### Añadido

- Recepción de telemetría ExpressLRS Backpack por WiFi/UDP.
- Conexión automática al primer SSID compatible `ExpressLRS TX Backpack*`.
- Decodificación de tramas MSPv2 y CRSF.
- Pantallas GENERAL, HORIZONTE, ENERGIA, GPS y DEBUG.
- Horizonte artificial con pitch, roll, escala de alabeo y referencia de aeronave.
- HOME automático al detectar modo de vuelo distinto de `WAIT` con GPS válido.
- Flecha HOME tipo OSD.
- Mini brújula.
- Pantalla GPS estilo instrumento con coordenadas, satélites, altitud, velocidad y rumbo.
- Modo RECOVERY con última posición GPS válida.
- QR de recuperación para Google Maps.
- Plus Code de recuperación.
- Detección automática de celdas 2S a 8S.
- Color por tensión por celda.
- Control de brillo PWM en backlight.
- Persistencia de brillo en memoria Preferences.
- Botón RESET físico por EN a GND.

### Corregido

- Brillo PWM corregido y estable.
- Barra de brillo con actualización en vivo.
- Reset físico sin necesidad de lógica adicional en firmware.
- Conservación de última posición GPS válida durante pérdida de telemetría.

### Validado

- ESP32 NodeMCU 38 pines USB-C.
- TFT ST7789 1,9 pulgadas 172 x 320.
- Alimentación por batería 18650 con Step Down ajustado a 3,30 V.
- Funcionamiento de navegación por botones y control de brillo.
