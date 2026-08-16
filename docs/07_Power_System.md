<!-- GroundStation ELRS Backpack documentation -->

# 07 - Sistema de alimentación

## Esquema validado

```text
Batería 18650 -> TP4056 -> Step Down -> 3,30 V -> pin 3.3V ESP32
```

## Punto crítico

La salida del Step Down debe ajustarse con multímetro antes de conectar la ESP32. El valor validado es **3,30 V exactos**.

## Advertencias

- No conectar una batería 18650 directamente al pin 3.3V.
- Una batería 18650 completamente cargada puede estar cerca de 4,2 V.
- El pin 3.3V debe recibir una tensión regulada y estable.
- Todos los módulos deben compartir GND.
- Verificar polaridad antes de energizar.

## Recomendación de montaje

Ajustar inicialmente el Step Down sin carga, medir, conectar la ESP32, volver a medir durante funcionamiento y verificar que la tensión se mantenga estable con WiFi y TFT encendidos.
