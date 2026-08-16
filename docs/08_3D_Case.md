<!-- GroundStation ELRS Backpack documentation -->

# 08 - Gabinete 3D

## Archivos STL oficiales

La versión final del gabinete documentada para la Release v1.0.0 es:

```text
stl/
├── GroundStation_Case_Body_v18_SMOOTH_NO_REAR_LIP.stl
└── GroundStation_Case_RearCover_v18_SMOOTH_NO_REAR_LIP.stl
```

## Estado del diseño 3D

- Versión del gabinete: **V18**.
- Variante: **SMOOTH_NO_REAR_LIP**.
- Release del producto: **GroundStation ELRS Backpack v1.0.0 Stable**.
- Firmware asociado: **GroundStation V2.5.1 Professional Stable Release**.

## Filosofía de diseño

- Gabinete compacto para uso de campo.
- Diseño pensado para montaje sin tornillos.
- Fijación interna de módulos con silicona caliente.
- Tapa trasera independiente.
- Versión sin reborde posterior, indicada por `NO_REAR_LIP`.
- Acabado suavizado, indicado por `SMOOTH`.
- Pantalla frontal protegida por el propio cuerpo del gabinete.
- Espacio interno para electrónica, batería y módulos de alimentación.

## Módulos internos previstos

- ESP32 NodeMCU 38 pines USB-C.
- Pantalla TFT ST7789 de 1,9 pulgadas.
- Porta batería 18650.
- TP4056.
- Step Down regulado a 3,30 V.
- Interruptor rectangular.
- Pulsadores externos NEXT y PREV.
- Pulsador RESET físico.

## Recomendación de carpeta GitHub

Los STL finales deben ubicarse en la carpeta `stl/` del repositorio con los nombres exactos indicados arriba.

## Parámetros de impresión sugeridos

Estos valores son una base recomendada para PLA/PLA+. Pueden ajustarse según impresora, filamento y resultado buscado.

| Parámetro | Recomendación |
|---|---|
| Material | PLA o PLA+ |
| Boquilla | 0,4 mm |
| Altura de capa | 0,20 mm |
| Perímetros | 3 |
| Relleno | 15 % a 25 % |
| Patrón de relleno | Gyroid o Grid |
| Adhesión | Brim si hay riesgo de despegue |
| Soportes | Revisar en slicer según orientación elegida |
| Impresora validada por el proyecto | Artillery Sidewinder X2 |

## Orientación de impresión

La orientación debe verificarse visualmente en Cura o en el slicer utilizado. Para el cuerpo principal conviene priorizar:

- buena adherencia a la cama,
- mínima cantidad de soportes,
- buen acabado del frente de pantalla,
- resistencia adecuada en paredes y esquinas.

## Montaje recomendado

1. Imprimir el cuerpo principal y la tapa trasera.
2. Presentar la pantalla TFT en el frente antes de pegar.
3. Presentar ESP32, TP4056, Step Down y porta batería 18650.
4. Verificar acceso a conectores USB necesarios.
5. Verificar que los pulsadores accionen correctamente.
6. Fijar módulos con silicona caliente.
7. Verificar alimentación a 3,30 V antes de cerrar.
8. Cerrar la tapa trasera con silicona caliente si se desea mantener el diseño sin tornillos.

