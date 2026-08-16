<!-- GroundStation ELRS Backpack documentation -->

# 05 - Manual de uso

## Encendido

1. Colocar una batería 18650 cargada.
2. Encender el interruptor principal.
3. Confirmar que aparece la pantalla de arranque `GROUND STATION`.
4. Encender el sistema ExpressLRS TX Backpack.
5. Esperar conexión WiFi.
6. Esperar recepción UDP.
7. Esperar telemetría de aeronave.

## Navegación

| Acción | Resultado |
|---|---|
| Pulsar NEXT | Avanza a la pantalla siguiente |
| Pulsar PREV | Retrocede a la pantalla anterior |
| Mantener NEXT | Abre barra de brillo |
| NEXT en modo brillo | Sube brillo |
| PREV en modo brillo | Baja brillo |
| RESET físico | Reinicia la ESP32 |

## Niveles de brillo

El firmware define cuatro niveles PWM:

- 25 por ciento
- 50 por ciento
- 75 por ciento
- 100 por ciento

El nivel se guarda usando `Preferences`, por lo que se conserva entre reinicios.

## Estados de enlace

| Estado | Significado |
|---|---|
| LIVE | UDP y telemetría de aeronave activos |
| UDP LOST | No se reciben paquetes UDP dentro del timeout |
| TEL LOST | Hay UDP, pero no telemetría útil de la aeronave |

## Modo RECOVERY

La pantalla GPS pasa a modo RECOVERY cuando se detecta pérdida UDP o pérdida de telemetría. Si existe una última posición GPS válida, muestra coordenadas, altitud, Plus Code y QR hacia Google Maps.
