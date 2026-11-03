# Sampler melódico 246 — lector reconstruido y adaptador propio

En 0.37.0, carga un sonido SAMPLE o tu WAV/AIFF en **04 / SAMPLER** y activa
**SAMPLER 246**. El lector y sus ocho campos proceden del callback contrastado;
la composición de voz mantiene adaptaciones propias explícitas. La opción apagada
vuelve al reproductor anterior. Cambiarla reinicia las voces activas.

## Controles e interpretación

| Página / banco | Azul | Verde | Blanco | Naranja |
| --- | --- | --- | --- | --- |
| Set 1 | Start | Loop In | Loop Out | End |
| Set 1, FINE / GAIN | Reverse | Loop In Fine | Loop Out Fine | Volume |
| Set 2, 246 ENVELOPE | Attack | Decay | Sustain | Release |

Start/In/Out/End muestran milisegundos; los ajustes finos, microsegundos; Volume,
porcentaje de ganancia cuadrática (100 % en el init propio). El host conserva las
palabras de control, con texto/unidades para editarlas. Reverse también tiene un
botón directo. La pantalla representa los cuatro límites sobre el audio convertido;
los marcadores de loop se atenúan cuando LOOP está apagado.

Los límites de loop y sus fracciones son independientes de Start/End. Sitúa In y
Out dentro de la región para repetir; un End no posterior a Start deja una región
vacía. El botón LOOP aporta un permiso propio del host: apagado deshabilita el
loop sin borrar sus puntos. La detección final de repetición y la retirada de nota
siguen la rutina contrastada. El importador inicializa los puntos con la longitud
cuantizada del clip; no los reajusta automáticamente al editar otro punto.

**246 ENVELOPE** activa el núcleo ADSR reconstruido compartido. Apagado conserva la
envolvente propia anterior y sus mandos. En set 2, Poly/Mono/Legato y Glide de
0–2000 ms usan el adaptador de interpretación del proyecto: prioridad de última
nota, sustain/bend por canal, vuelta a la tecla anterior y seis voces en Poly.
Mono reinicia lector/envolvente; Legato conserva la voz mientras las notas solapan.
Ese allocator, mapa de eventos y portamento no se presentan como el original.

Parámetros, banco de mandos, ADSR y modo de voz se automatizan y guardan en
proyectos, slots synth y clipboard. Los proxies MIDI cambian de banco y de
envolvente con el editor cerrado. Root Note y los FX del bus siguen disponibles;
LFO propio de pitch/filter también. Desde 0.37 se integran
[Value/Random/Tremolo del sampler](sampler246-modulation.md), con reglas acopladas
contrastadas para los ocho destinos Instrument y ADSR; reloj/composición propios. No se incluyen presets/audio originales.

## Contrato reconstruido

La factoría `0x01a51e34` crea seis estados de 16 bytes con vtable `0x019119f4`.
Su entrada +0x14 apunta al callback completo **`0x01a51a7c`**. La dirección
`0x01a51a66`, apuntada en la investigación anterior, es un helper diferente:
no era el renderer validado. Bind `0x01a519a0`, reset `0x01a518f4` y release
`0x01a51964` quedan contrastados con el pool entero, incluidos bytes retenidos.

El lector tiene ocho controles: inicio, inicio/end de loop, final, reverse,
dos fracciones finas de loop y volumen. El cursor usa Q12, FIR saturado de 16 taps
y 64 fases, pitch por muestra multiplicado por un factor de fuente, y volumen al
cuadrado. El restart latch aplica un límite inicial; release deshabilita el loop.
Al sobrepasar el límite se escriben ceros y el cursor queda conservado. No se
inventa una marca original de voz terminada donde el callback no la produce.

**4096 bloques / 524288 muestras**, seis estados completos, salida, pitch intacto,
PCM fuente y guardas coinciden bajo entradas explícitas. Se conservan los 1025
coeficientes FIR legibles, convertidos por el helper original. No hay sustituciones.
La segunda mitad usa pitch signed32 y estados retenidos; los accesos fuera del
PCM se definen como ceros del harness, no como una captura de RAM. Dos ejecuciones
originales en directorios privados dan los mismos bytes. Se usa el modelo GNU
BF524 con tres correcciones de writeback; no hardware físico.

[Formato y límites del corpus](../research/sampler-render/vectors/README.md) ·
[Evidencia numérica](firmware/sampler246-render-validation.json).

## Adaptaciones del plugin y límites

El plugin conserva el float audio original para sesiones, exportación y el lector
anterior. El cache PCM16 mono a 44,1 kHz se prepara fuera del callback con nuestro
conversor lineal. La vista y lectura nativas usan hasta los primeros **264591
frames, aproximadamente seis segundos**, por el rango del cursor/control. El
cache común puede contener más audio para otros usos; no se borran los originales.

El adaptador aporta reloj de 88,2 kHz, afinación relativa a Root Note, cuantización
de pitch, alineación PCM de ocho frames, filtros/interpolación de salida y drenaje
de 512 muestras internas. Desde 0.36 se pasan los ocho campos al lector, con el
permiso LOOP propio descrito arriba. Las palabras coarse/fine mantienen el cálculo
y sus límites originales; no se convierten en un trim relativo a la longitud del
archivo. El máximo coarse representa aproximadamente seis segundos, aunque el
clip importado sea más corto. La zona fuera del audio suministrado contiene ceros.

La ADSR 246 reutiliza el [núcleo y tablas contrastados](envelope246-research.md).
**Su composición con este lector, cadencia, dither/seed por voz y mapa de eventos
son decisiones del adaptador**; no se ha recuperado el mezclador original del
sampler. La envolvente se aplica a la señal del lector antes de los filtros de
salida. Volumen del lector, velocidad y mezcla del host conservan sus funciones.
La cola reportada utiliza el polo de release elegido más el drenaje de salida.

Un nuevo ensayo compone independientemente ambos núcleos en **32768 muestras**:
forward/reverse, loop/no loop, fracciones, volumen variable y release, comparando
audio, los 16 bytes del lector, estado ADSR y dither. Es una prueba de esta
composición explícita; no una ejecución de la voz completa original. La prueba de
interpretación cubre seis tasas, las dos envolventes, Mono/Legato/glide, afinación
audible, canales, sustain, cambios de modo y pánico. Los 4096 bloques originales
del lector de 0.35 se mantienen como regresión sin regenerar sus fixtures.

No se priorizan migraciones de sesiones antiguas. No se ha medido equivalencia
acústica ni probado esta entrega en Ableton Live real o ejecutado en macOS arm64.
Quedan el pitch/clock/allocator/ruteo originales, dispatch LFO de
batería, dinámica drum, Pulse/L1 y presets completos al final.

## Dinámica de batería: investigación de este bloque

El lector drum consume `0xff801d50`; los moduladores también pueden transformarlo.
Una referencia legible de preparación aparece en L1 alrededor de `0xffa00700`,
dentro de la zona parcialmente protegida de la ruta candidata `0xffa00684`.
Eso delimita una ruta de estudio, pero no recupera un productor completo ni
valida el vínculo con los cuatro controles de dinámica. Los callbacks Core de
reset/release drum y VoiceCtl son rutas de eventos, no prueba de ese productor.

Se mantiene el contorno propio de batería de 0.34.0. No se adjudica a L1 todo el
problema por una referencia, ni se sustituye la curva por una hipótesis presentada
como original. Siguen el productor de dinámica, voz/ruteo, dispatch drum,
Pulse/L1 y calibración. Presets completos al final.
