# Voltage 246 — núcleo original y adaptación al host

## Qué está comprobado

La candidata `0x01a49bf4` se ha seguido hasta sus dependencias y ejecutado como
callback mono completo en GNU BF524. No requiere OTP, identidad física, RNG ni
lector de RAM exterior al estado, tablas y buffers declarados. Esto describe esta
ruta concreta, no la memoria cifrada del instrumento: **no se ha descifrado L1**.

La reconstrucción independiente conserva cuatro osciladores con realimentación,
rectificación/XOR, dispersión, carga, filtro y media entre muestras. Usa el seno
legible convertido por `0x01a49b1c` y los **129 coeficientes originales** de
`0x018be948`, convertidos por la rutina original; coinciden con la tabla compartida
`spring246Low`. No se sustituyen coeficientes. El buffer de pitch contiene **128
palabras completas** por bloque, todas consumidas; no usa la extensión
fraccionaria propia del FM.

Se contrastan **4096 bloques / 524288 muestras**, con controles extremos y
aleatorios válidos, fases y estados arbitrarios, pitch de 32 bits que cambia
dentro del bloque, secuencias retenidas y las seis voces. Se comparan los 312
bytes del pool antes y después de los eventos/render, audio, 32 valores de display,
preservación del pitch y cuatro guardas, incluido el RNG que esta ruta no modifica.

| Operación | Dirección | Efecto contrastado |
| --- | --- | --- |
| Factoría | `0x01a49ef8` | Seis slots de 52 bytes, tag y cuatro osciladores; conserva padding |
| Reset | `0x01a49b94` | Fases a `0x80000000`, carga/previo a 16384, filtro/detune/release a cero |
| Release | `0x01a49b4e` | Activa el avance de detune; no sustituye al release ADSR |
| Render | `0x01a49bf4` | Callback mono completo, salida y display |

Los fixtures incluyen eventos en distintos slots y guardas, no una emulación
completa de la asignación original. El signo del cruce de fase, la saturación
serial, el octavo de la tercera señal y el incremento de detune al liberar forman
parte de la aritmética contrastada. Dos ejecuciones independientes reproducen los
fixtures completos byte por byte.

## Controles, modulación e integración

Seleccionar **10 / VOLTAGE** o **Voltage 246 → Init / Voltage**. Los cuatro controles
se presentan con etiquetas funcionales propias: azul XOR/Mix, verde Feedback,
blanco Tone y naranja Detune. Todos tienen rango raw **0–32767**, escala original
de modulación **16384** y auxiliar cero, descriptor de tipo **13** en `0x01de5bd0`.
Los valores de Init `{12000,4000,16000,8000}` son propios, no un preset de fábrica.

Value Instrument/ADSR y Random Instrument/ADSR se contrastan en **4096 + 4096
callbacks** con estado, RNG, display, ocho destinos y retorno completos. Random
consume su RNG compartido; el núcleo Voltage no lo consume. Por eso la profundidad
cero conserva el audio de Voltage en estos casos. Tremolo usa su aritmética ya
contrastada y entrega el pitch/amplitud por bloques de 128 muestras. La prioridad
en el host sigue Random → Tremolo → Value por voz.

Se integran ADSR común contrastada, seis voces, Mono/Legato/glide, sustain y bend
por canal, cuatro controles y cuatro parámetros ADSR automatizables, Value/Random/
Tremolo, proyectos, slots, clipboard y mandos contextuales con editor cerrado.
La pantalla de cuatro trazas es una animación propia, no un gráfico del firmware.

## Qué continúa siendo propio o pendiente

Son adaptaciones el reloj de **88,2 kHz**, conversión de nota a pitch, cadencia de
controles/eventos, asignación/robo de voces y composición con ADSR, DC blocker de
15 Hz, filtro de salida y resampling. Conserva el bus/latencia común de salida y
el volumen maestro. El núcleo no atraviesa implícitamente el antiguo filtro/tanh
propio; los FX seleccionados explícitamente sí siguen disponibles.

La igualdad numérica depende de las entradas y eventos suministrados. No establece
la cadencia original, la respuesta del aparato completo ni equivalencia acústica.
Faltan calibración con referencias disponibles, Ableton Live real, ejecución
macOS arm64 y los presets originales completos, reservados para la etapa final.
D-synth es el próximo motor, sujeto a revisar sus dependencias.

## Reproducción y procedencia

Prueba pública sin firmware:

```sh
cmake -S research/voltage-render -B build-voltage -DCMAKE_BUILD_TYPE=Release
cmake --build build-voltage --config Release --parallel 2
ctest --test-dir build-voltage -C Release --output-on-failure
```

`scripts/verify-voltage246.py` y los verificadores Value/Random con `--voltage`
reconstruyen los oráculos desde el LDR privado de hash documentado. Requieren GNU
BF524 con ambos parches ya documentados y un directorio privado nuevo fuera del
checkout. El ejecutable, SDRAM y log permanecen allí. Solo se versionan harness
propio y observaciones numéricas sintéticas; nunca instrucciones originales,
firmware, volcados privados ni pseudocódigo descompilado.

[Metadatos y hashes](firmware/voltage246-validation.json) ·
[Formato de vectores](../research/voltage-render/vectors/README.md) ·
[Validación de entrega](releases/v0.31.0-validation.json).
