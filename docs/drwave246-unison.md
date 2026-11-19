# Dr. Wave: Unison de seis voces — 0.59.0

En **Set 2 / ADSR**, el botón **UNISON** activa seis voces sobre la última nota
retenida. Está desactivado por defecto y solo actúa en Dr. Wave. Mientras está
activo sustituye Poly/Mono/Legato y usa Glide; al desactivarlo vuelve al modo
seleccionado. Cambiarlo retira las voces y las notas retenidas. Vuelve a tocar
para iniciar el modo nuevo.

El parámetro automatizable **Dr. Wave: Unison** (`drWave246Unison`) funciona con
el editor cerrado. Se guarda en sesión, slots y clipboard. Los estados anteriores
sin él lo cargan apagado; los init lo restablecen apagado. Las tres opciones del
selector de voz existente conservan sus valores. El parámetro se añade al final
del layout Phase246; no se promete conservar los índices numéricos de proxies
registrados después de ese layout. No se altera la salida MIDI del host: sigue
enviando una nota generada/tecla, sin duplicarla seis veces.

## Composición del grupo

Las seis voces tienen oscilador, envolvente, filtros, SRC y workspace propios.
Reciben la prioridad de última nota, velocidad y canal, incluidos sustain, bend y
el canal interno 17. Soltar una nota antigua no retriggera el grupo; al soltar la
prioritaria se vuelve a la anterior retenida. Una nota nueva solicita el reinicio
con rampa de cada voz activa; las libres arrancan por su ruta habitual.

La dinámica y el servicio son los de [Mono/retirada](drwave246-mono-retirement.md).
El pitch nuevo se entrega al acabar la rampa, conservando el pitch anterior hasta
ese punto. Cada voz termina su cola antes de retirarse y una petición nueva puede
revocar la retirada. Panic y cambios de modo/motor vacían grupo y prioridad.
Unison queda inactivo en otros motores aunque su parámetro siga almacenado.

La mezcla usa la media de las seis contribuciones (`1/6`), una normalización
propia para evitar multiplicar el nivel por seis. No se afirma igualdad de RMS
con Mono ni recuperación de la ganancia de mezcla original.

## Desviación de pitch contrastada

La [ruta de seis voces del selector 3](drwave246-note-modes.md) ya estaba
contrastada en un contexto Core Dr. Wave acotado. Se extraen sus operaciones a
`Voice246UnisonPitch.h` y se reutilizan tanto en ese modelo como en el producto:

- RNG u32: `seed = seed × 1664525 + 1013904223`, con wrap.
- Halfword alto signed16 dividido con floor por 256 (voz libre) o 512 (activa).
- Producto por el índice de voz 0–5 y desplazamiento 16 bits con wrap u32.
- `pitch = saturate32(base + multiplyQ31(base, deviation))`.

La voz cero también avanza el RNG, aunque su desviación sea cero. La desviación
se aplica en punto fijo al pitch base del host y se incluye en la palabra
pendiente, antes de los moduladores de pitch del render. No se sustituye por
un detune flotante en cents.

El host usa una semilla propia y reproducible `0x554e4953`, reiniciada por Panic
u operación de activar/desactivar el modo, con estado separado de otros LFO.
No recupera el seed global físico, su cadencia ni su relación con otros módulos.
La distribución cambia según cada voz siga activa o haya terminado su cola.
El pitch base A440, la prioridad, Glide, servicio, mezcla y reloj siguen siendo
híbridos. El nombre comercial original del selector 3 **no se deduce** de este
ensayo; Unison es el modo funcional de nuestro producto.

## Validación

[Evidencia](releases/v0.59.0-validation.json): los 768 registros originales de
selectores siguen pasando sin regenerar fixtures, incluyendo RNG, registros,
ADSR y guardas. No son ejecuciones originales nuevas.
La prueba propia comprueba entrega del pitch desviado después de la rampa y seis
voces, fallback, sustain, canal 17, Panic, retirada y cambios de motor a seis tasas.
El procesador compara nueve pares de audio exacto entre callbacks 64/257
a tres tasas y con las tres opciones subyacentes del selector; prueba
estado, migración, slots, clipboard y el botón real del editor. Seis cargas
preparadas recorren 1.536 callbacks con automatización y ráfagas de notas y
rastrean llamadas al heap. El resto de regresión y CI se registra por separado.

La coincidencia numérica de las operaciones no mide fidelidad acústica del
instrumento completo ni valida Ableton real o MIDI físico. No se generan
instaladores y los presets originales siguen reservados para la etapa final.
