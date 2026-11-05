# Bend y Crank: audio de entrada al bus FX — 0.49.0

El destino **Voice / FX Input** reúne cuatro targets: **Level**, **Pitch**,
**FX Gain** y **FX Shape**. Los dos primeros conservan las rutas de voz de
0.47/0.48. Los nuevos targets actúan sobre la entrada mono de Filter, Nitro,
Delay, Grid, Phone, Spring o CWO. Admiten mezcla, cinta y audio externo
monitorizado. Punch ocupa otra etapa: estos targets aparecen como OFF si solo
está seleccionado Punch o si el bus está en bypass. Los targets 4–7 se rechazan.

Bend se mueve con Orange, conserva Reverse e inercia; Crank se mueve con Blue,
conserva su fase triangular e inercia. Amount admite ambos signos y cero.
Automatización, mandos sin editor, proyectos, slots y clipboard incluidos.

**FX Gain** añade al input su producto Q15 por la señal moduladora y Amount,
con suma saturada. **FX Shape** hace XOR de la palabra de audio con la señal
moduladora, multiplica por el input y después por Amount, y suma al input con
saturación. No es un control de volumen maestro ni una multiplicación bipolar
como las dos curvas MIDI. Amount cero conserva exactamente el input; las curvas
no crean audio a partir de cero. Shape puede alterar audio aunque Bend esté en
reposo: XOR con fuente cero sigue multiplicando el input por sí mismo. Crank
puede conservar modulación estática al detenerse, según la fase alcanzada.

## Contrato contrastado

Se reconstruyen independientemente los callbacks FX de Bend `0x019ca328` y
Crank `0x019cb3d4`, kinds 2/5, destino interno 3, colores 2/3. Bend obtiene su
fuente mediante saturación de `level*256`; Crank mediante `(triangle-64)*512`.
En Shape el XOR usa esa fuente **antes** de Amount. Se conserva el orden de las
dos multiplicaciones redondeadas Q15 y la suma saturada final.

Cada fuente aporta **8.192 llamadas originales**: **3.493 aceptadas** y **4.699
rechazadas**. Se comparan fuente/flags/historial/display, ocho controles, targets,
tag, retorno y los buffers completos de pitch, ganancia y audio. Cada matriz
cubre ocho tipos FX, ambos kinds válidos y ambas curvas; hay rechazo de kinds,
destinos y colores incorrectos. Los 64 halfwords afectados por cada llamada
aceptada también contrastan el puente por muestra: **223.552 muestras por fuente,
447.104 en total**. Los otros 64 halfwords de audio se comprueban intactos.
Dos extracciones independientes producen entradas/salidas numéricas idénticas.
Los estados y buffers son entradas explícitas; no son capturas de hardware.

## Integración y límites

Se reutiliza un snapshot del controlador global ya integrado. La fuente avanza
una vez por tick propio de 44100/64; la entrada FX lee su valor retenido por
muestra interna, después de cuantizar y antes del núcleo mono de 44,1 kHz.
Bend y Crank respetan la prioridad existente Element > Crank > Bend > MIDI;
las voces no avanzan otra copia de la fuente. Bypass, transición seca, PDC,
remuestreo y reset del bus conservan sus políticas anteriores. La curva cambia
la entrada del efecto, sin borrar su memoria ni sus colas.

Ese clock, ubicación, ownership, captura de automatización, inicialización y
composición son decisiones del host. No se recupera el scheduler/allocator ni el
productor original de buffers, protocolo físico o dinámica drum. Tampoco se
integran auxiliares FX kind 4, modos de secuenciador Crank ni Envelope de Element.
No se afirma equivalencia acústica, aceptación del scanner de Ableton, ejecución
ARM64 nativa, firma/notarización o réplica de todos los presets originales.

## Reproducción

```
cmake -S research/manual-fx -B build-manual-fx -DCMAKE_BUILD_TYPE=Release
cmake --build build-manual-fx --config Release --parallel 2
ctest --test-dir build-manual-fx -C Release --output-on-failure
```

`verify-manual-fx246.py --source bend|crank` reproduce el contraste con LDR y
herramientas privadas suministradas por el operador; exige las tres correcciones
verificadas de writeback del simulador. Reutiliza los generadores de estado
`verify-bend-voice246.py` / `verify-crank-voice246.py`. Salidas fuera del checkout;
`--record` guarda solo fixtures numéricos. No se distribuye firmware, ELF ni
pseudocódigo del descompilador. [Resultados y CI](releases/v0.49.0-validation.json).
