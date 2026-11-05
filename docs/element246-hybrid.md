# Element LFO 246 — 0.43.0

En la página LFO elige **LFO: Element**. Blue selecciona la fuente, Green la
cantidad bipolar, White Instrument/ADSR/FX y Orange el target.
[0.50.0 añade FX Input: Multiply/Subtract](element246-fx-input.md), con contrato
de audio separado, muestras externas firmadas y Self sobre el propio audio. Hay cuatro fuentes:

| Fuente | Uso en el plugin |
|---|---|
| G-force (virtual) | Mueve **VIRTUAL SENSOR**; es automatizable y no necesita acelerómetro. |
| External Input | Alimenta la entrada de audio del plugin/standalone. Usa el promedio estéreo con el Input Gain existente; no hace falta activar la monitorización directa. |
| Voice Envelope | Sigue el máximo de las envolventes activas del motor. |
| Instrument Level | Sigue el pico absoluto de la mezcla del instrumento antes de efectos, cinta, mezcla final y volumen maestro. |

Una entrada externa necesita un bus de entrada conectado por el host o un
dispositivo de audio en standalone; el plugin no crea una radio, un micrófono ni
un acelerómetro. El indicador muestra la señal de modulación; los targets no
aplicables indican OFF. El sensor solo se muestra al elegir G-force.

## Callback original contrastado

El manual §15.2 identifica G-force, entrada externa, envolvente y nivel del motor.
La tabla de nombres y la cuarta entrada de la factoría (**índice 3 desde cero**)
identifican Element: wrapper `0x019c9a88`, factoría synth `0x019ce3a4`, referencias
a `LfoReRoute.cpp` y estas vtables/callbacks:

| Ruta | Vtable | Callback |
|---|---|---|
| Synth | `0x0191089c` | `0x019cda14` |
| Drum | `0x019108bc` | `0x019cde28` |
| FX | `0x019108dc` | `0x019cdf22` |

Los resets observados `0x019cd954`, `0x019cd952` y `0x019cd950` son no-op.
El contrato de parámetros es sin estado interno: recibe controles y palabras o
buffers de entrada explícitos. **18.432 callbacks originales**, 6.144 por familia,
se contrastan completamente y se repiten con salida idéntica.

Se comparan self/tag, ocho controles, ocho targets, retorno, palabras completas
de sensor/envolvente/nivel y todos los buffers: 64 entradas externas, 128 palabras
de ganancia, 128 pitch words y 128 audio halfwords FX. Los buffers y entradas se
mantienen intactos en estas rutas. Los fixtures incluyen positivos/negativos,
silencio, picos en distintas posiciones, extremos Q15 y rangos/tipos/colores/kinds.
La matriz exige despachos aceptados para todos los tipos y las cuatro fuentes.

Las fuentes suministradas se distinguen así:

- Sensor: negación saturada del halfword de `0xff801056` y producto Q15 por cantidad.
- Externa: **máximo positivo con cero** de 64 halfwords en `0xff800cb4`; no valor
  absoluto, RMS ni seguidor con ataque/release.
- Fuente 2 synth: máximo positivo de 128 gains de `0xff801d50`, multiplicado por
  cuatro con saturación **antes** del producto Q15.
- Fuente 2 FX kind 2: halfword `0xff80108c`, producto Q15 y saturación por cuatro
  **después** del producto. El orden no es intercambiable.
- Fuente 2 drum/FX kind 5: halfword superior de `0x01dfc80c`, leído por
  `0x01a66912`, producto Q15 sin esa expansión por cuatro.
- Nivel: halfword `0xff80108e`, producto Q15.

Instrument reutiliza los rangos y las reglas originales legibles de synth,
selectors fine, sampler y drum/D-box. ADSR synth usa los primeros cuatro campos;
drum rechaza ADSR. FX kinds 2/5 usan un **offset directo saturado y limitado al
rango** (`0x01a1de8e`), no la escala de rango de Value/Bend/Crank.

Esto demuestra coincidencia numérica bajo esos controles, buffers y palabras,
en el simulador BF524 con los tres parches de writeback contrastados. **No**
recupera los productores físicos/IPC, el detector de nivel/envolvente original,
la voz completa ni equivalencia acústica. Pitch y FX ENV kind 4 siguen fuera. [Gain de voz tiene un contrato separado en
0.51.0](element246-voice-gain.md). El audio FX tiene un
[contrato e integración separados en 0.50.0](element246-fx-input.md).

## Frontend propio del host

El plugin usa una fuente global compartida entre voces/FX, con ventanas propias
`64/44100` segundos. Reúne el pico positivo de todas las muestras de entrada del
host en la ventana. No hace resampling del bloque original de 64 muestras: la
partición de buffers del host es independiente, pero esta preparación es propia.
Publica los valores al cerrar la ventana; el arranque empieza con palabras cero.

El sensor virtual firmado −32768…32767 se toma en ese tick. Para Envelope se
mapea el máximo ADSR de las voces activas a Q15; los gains synth y la palabra FX
se suministran a un cuarto de escala, y drum a escala completa. Esta normalización,
agregación polifónica y uso de un valor retenido son propios, **no el buffer de
ganancia original por voz**. Las palabras FX/drum conservan su aritmética distinta. El único bus FX del host
usa **kind 2**, también en modo drum; kind 5 está contrastado numéricamente pero
no implementa un bus FX original separado por voz/drum.

Level reúne el máximo absoluto del sum de instrumento previo a FX y al factor de
volumen maestro; incluye la ganancia de voz/velocidad existente. La medida se
alimenta desde la muestra anterior para evitar un lazo algebraico, y se retiene
por ventana. No se recupera el productor de `0xff80108e` ni se afirma su calibración.
Las entradas no finitas del frontend se sustituyen por cero y se limita Q15.

Activación, desactivación y pánico reinician frontend y feedback como política
propia. Las notas no lo reinician. Se guardan controles y sensor, no los picos
pendientes ni la trayectoria viva. Al cambiar de proyecto hay una nueva ventana.

Los cuatro Screen Knob conservan sus IDs y siguen Element con el editor cerrado.
Controles/sensor persisten en proyectos, slots y clipboard. La automatización de
parámetros se toma por bloque; las pruebas de partición sitúan sus cambios en un
límite común explícito, sin atribuir timestamps por muestra al host. Los eventos
MIDI conservan su tratamiento habitual por muestra.

La prioridad al forzar flags simultáneos es Element, Crank, Bend, MIDI, Random,
Tremolo, Value Instrument/ADSR, Value FX y LFO propio. El filtro implícito del LFO
propio también se retira al seleccionar Element/Crank/Bend/MIDI. Se anuncia una
cola conservadora para Release o parámetros drum que puedan prolongar la voz.
No se añade una ganancia maestra ni se modifica el PDC.

## Reproducción y pendientes

`Element246.h`, el frontend y el driver de pruebas son código independiente.
Los fixtures gzip contienen solo entradas sintéticas y observaciones numéricas;
no firmware, volcado privado completo ni código descompilado. El verificador
`scripts/verify-element246.py` requiere el LDR rev246 privado con hash conocido,
herramientas Blackfin y simulador corregido. La salida debe ser un directorio
nuevo fuera del checkout; `--record` registra observaciones y repetir sin él
exige igualdad. [Hashes y matriz](firmware/element246-validation.json).

La suite independiente no necesita firmware ni JUCE:

```sh
cmake -S research/element-modulation -B build-element -DCMAKE_BUILD_TYPE=Release
cmake --build build-element --config Release
ctest --test-dir build-element -C Release --output-on-failure
```

Requiere C++17, CMake 3.22 y Python 3.11. La integración verifica ventanas a siete
frecuencias, **1.008 combinaciones perfil/fuente/destino/frecuencia**, rechazo ADSR
drum, no finitos, prioridades, pánico y audio exacto en particiones de 64/257.
Cluster usa Damping para que una pequeña señal de Level no se pierda en los
umbrales de Oscillators/Feedback; no se cambian esos algoritmos para pasar el test.
[Validación de entrega](releases/v0.43.0-validation.json).

Quedan pendientes productores de las entradas, ruteo por voz y scheduler originales,
rutas especiales, Pulse/L1, calibración acústica, Ableton Live real, ejecución nativa
arm64 y presets originales completos al final. Las sustituciones híbridas de los
motores y efectos no desaparecen con este LFO.
