# Sampler melódico 246 — lector reconstruido y adaptador propio

En 0.35.0, carga un sonido SAMPLE o tu WAV/AIFF en **04 / SAMPLER** y activa
**SAMPLER 246**. Se conservan los controles Start/Length del set 1, ADSR del set 2,
Root Note del host y LOOP. **REVERSE** invierte la lectura. La opción desactivada
mantiene el reproductor anterior. Cambiar la opción reinicia las voces activas. El material de fábrica del OP-1 no se distribuye.

Hay seis voces, sustain/bend por canal, robo de la más antigua con fundido propio,
FX del bus, automatización y conservación de opción/dirección/audio en proyectos,
slots synth y clipboard. Los mandos contextuales mantienen sus destinos actuales.
El LFO propio de pitch/filter sigue disponible. Value/Random/Tremolo nativos de
este motor requieren su dispatch antes de habilitarlos.

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
de 512 muestras internas. La ADSR, velocidad, allocator y mapa MIDI siguen siendo
propios; esta entrega no reconstruye la voz completa original. La cola reportada
incluye la envolvente propia y los efectos del bus.

**El mapa de controles es propio**: Start/Length define una región, y LOOP repite
esa misma región hasta note-off. El núcleo sí contrasta límites de loop
independientes y sus fracciones, pero todavía no se exponen todos esos ocho campos
en el editor/host. La compatibilidad con antiguos proyectos no implica identidad
sonora entre ambos reproductores. No se ha medido equivalencia acústica ni se ha
probado esta entrega en Ableton Live real o ejecutado nativamente en macOS arm64.

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
controles secundarios del sampler, Pulse/L1 y calibración. Presets completos al final.
