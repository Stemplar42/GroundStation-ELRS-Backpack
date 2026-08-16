<!-- GroundStation ELRS Backpack documentation -->

# 01 - Descripción general

GroundStation ELRS Backpack es una estación de tierra portátil basada en ESP32, pensada para visualizar telemetría recibida desde un módulo ExpressLRS TX Backpack.

El firmware incluido corresponde a GroundStation V2.5.1 Professional Stable Release. El sketch declara conexión WiFi contra SSID que comienzan con `ExpressLRS TX Backpack`, contraseña `expresslrs`, recepción UDP local en puerto 14550 y parser de datos MSPv2/CRSF.

## Objetivos del proyecto

- Construir una estación compacta y autónoma.
- Visualizar datos críticos de telemetría en campo.
- Mantener última posición GPS válida para recuperación.
- Ofrecer navegación por pantallas con botones físicos.
- Permitir brillo regulable para uso en exterior.
- Facilitar replicación por otros usuarios mediante documentación y archivos de firmware.

## Arquitectura general

```text
ExpressLRS TX Backpack -> WiFi -> ESP32 -> UDP 14550 -> Parser MSPv2/CRSF -> TFT ST7789
```

## Pantallas disponibles

1. GENERAL
2. HORIZONTE
3. ENERGIA
4. GPS / RECOVERY
5. DEBUG
