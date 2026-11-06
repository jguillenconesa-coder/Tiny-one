# Batería 0.54.0: palabras originales de velocidad y transposición

Entrega del 6 de noviembre de 2026. El lector de batería incorpora las palabras
que produce el note-on original `0x01a84ba0`. Se mantiene el método híbrido:
las conversiones del evento están contrastadas; los productores por muestra,
la dinámica de región y el reloj del host siguen siendo adaptaciones propias.

## Contrato y procedencia

La palabra de velocidad `0xff8012ec` interpreta los 16 bits bajos del evento
como entero firmado, multiplica por 256 y satura a int16. Los bits altos no
participan en la conversión, aunque el evento completo se copia al registro
retenido. Se contrastan los 65.536 patrones posibles del halfword, incluidos
negativos y saturación; el frontend MIDI del producto solo usa 1–127.

La palabra de pitch `0xff801304` procede de convertir la transposición entera
a float (`0x01aa5244`), dividir entre 12 (`0x01aa54c8`), calcular potencia de dos
(`0x01aa5db4`), dividir entre 32 y convertir a Q31 (`0x01aa6fd0`). La tabla de
[Drum246EventTables.h](../Source/Drum246EventTables.h) contiene **salidas derivadas**
para −128…127 semitonos; **no es una tabla ROM recuperada** ni un port general
de esas rutinas soft-float. Tiene 189 valores distintos; 60…127 saturan a
`0x7fffffff`. Cero produce `0x04000000` y las octavas conservan las potencias de dos.
Fuera de ese dominio el helper de referencia rechaza la consulta sin alterar
la salida. No se extrapola evidencia.

## Integración y fronteras propias

[Drum246Event.h](../Source/Drum246Event.h) comparte ambas conversiones entre
el modelo de investigación y el instrumento. El host cuantiza la velocidad
float al código MIDI más próximo y conserva la palabra en cada hit. La amplitud
propia divide por 32512: los 127 códigos MIDI mantienen exactamente su amplitud
anterior; las velocidades arbitrarias del teclado gráfico se cuantizan.
Una nota ya retenida conserva su velocidad, conforme al asignador de 0.53.

El pitch del pad y de los controles modulados conserva el rango del host
±24 semitonos. Los enteros usan la palabra contrastada; las fracciones usan
interpolación geométrica entre palabras adyacentes. Esa interpolación, el mapeo
del control del pad, el bend y la normalización por dos son políticas propias,
no el productor original de buffers. Se conserva la composición posterior con
Tremolo, el lector/FIR y SRC. No se modifica D-box, el kit legacy, los parámetros
ni el formato de proyecto. La curva dinámica original y su composición con
velocidad permanecen pendientes; no se afirma equivalencia acústica.

## Evidencia y reproducción

[Verificador](../scripts/verify-drum-event246.py): dos ejecuciones independientes
de instrucciones originales, usando firmware privado cuyo SHA-256 se comprueba,
y simulador con los tres arreglos de writeback y probes previos. Solo se
versionan el harness propio y datos numéricos, sin firmware o descompilación.

El corpus de 65.536 note-on completos cubre todos los halfwords de velocidad y
las 256 transposiciones. La permutación latina cruza cada byte bajo de velocidad
con cada transposición una vez, con bits altos, notas, bancos y metadatos
variados. Se suministran ocho bancos de controles, modo drum tipo 1, sampler tipo
8, preview −1, registro/flags vacíos por llamada y LFO/FX nulos. Factory se
inicializa con referencias del sonido 0. El pool conserva el orden de llamadas.
Se comparan los doce objetos (240 bytes), 812 bytes globales, la primera fila
retenida (20 bytes) y el contador (4 bytes) de **cada** registro. Hay 32.768
palabras negativas de velocidad y 17.408 pitches saturados. No hay render de
audio en este corpus; los registros no se cuentan como muestras de audio.

La [suite de referencia](../research/drum-event) compara todos los bytes y
comprueba cobertura, límites y anclas. Las 12.288 filas anteriores de VoiceCtl
y bancos siguen pasando al compartir las nuevas conversiones.

El [puente del host](../Tests/Drum246EventBridgeTests.cpp) comprueba todos los
códigos MIDI, 49 anclas enteras, continuidad y 245.765 pasos de pitch/bend,
bounds y 56 composiciones directas del lector a siete tasas. El plugin contrasta
12 casos A/B con duplicados, note-off y cola: 48/96 kHz, buffers 64/257 y tres
pitches (0, −7,75, +12,25). Se varían las velocidades de las ráfagas dentro de las
36 cargas de batería ya reforzadas; no se aumenta el total por reutilizarlas.

[Evidencia original](firmware/drum-event246-validation.json) y
[validación de entrega](releases/v0.54.0-validation.json). Las pruebas numéricas
no miden semejanza sonora, dropouts ni aceptación real en Ableton. Estimación
sin cambio: **≈70 % ±10 puntos de planificación**. Próximo frente: composición
activa de esas palabras con los productores pitch/gain y el llamador de render.
