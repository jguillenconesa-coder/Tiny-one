# Batería rev246: bancos, registro retenido y ocupación

Investigación del 5 de noviembre de 2026, continuación de
[VoiceCtl note-on/off](drum246-voice-lifecycle.md). **El instrumento continúa en
0.52.0**, sin cambios ejecutables ni nuevos instaladores. El bloque distingue
limpieza del registro, reinicio de lectores y retirada normal durante el render;
esta última sigue sin contrato recuperado.

## Contratos originales contrastados

| Rutina | Contrato bajo las entradas suministradas |
| --- | --- |
| `0x01a27e2c` | Devuelve 1 si cualquiera de los doce bytes de ocupación es no nulo, 0 si todos son cero; no los modifica. |
| `0x01a85df8` | Requiere fase cero en `0x01dfe730`, escribe fase 1 y limpia los doce bytes de ocupación. Conserva lista y objetos de audio. |
| `0x01a85e40` | Llama a la factory Core del tipo recibido, requiere fase no nula y escribe fase cero. Se prueba tipo 8 y ocho referencias de muestra explícitas. |
| `0x01a856bc` | Recorre la lista retenida, pone ocupación de cada voz a cero, despacha resets Core y vuelve a escribir 1. No reasigna ni depura propietarios antiguos. |
| `0x01a8624c` | Limpia ocupación, recorre metadata de las notas retenidas, llama al auxiliar externo cuando el byte es no nulo y pone count a cero. Conserva las filas residuales y los estados del lector. |
| `0x019e2dc0` | Bajo palabra de salida MIDI `0x01dfe03c = 0`, limpia el flag externo de esa nota en `0x01aafd60[128]`, sin emitir ni recorrer el transmisor. |

El reset retenido usa el **pad actual de la voz**, vía Core; no reconstruye el
pad de cada fila antigua. Dos filas pueden reiniciar el mismo lector más de una
vez después de robos de voz. Se conserva ese orden, junto con contador, posición,
modo, handle y metadata. La factory cambia la referencia de las doce voces y
conserva los contadores que el constructor no inicializa.

La limpieza de registro **no equivale a detener el audio**: no despacha release
ni pone a cero los objetos del lector. Las 128 limpiezas aisladas del oráculo
conservan byte a byte el pool anterior. El modo puede intervenir en un consumidor
posterior que este bloque no identifica. Por eso no se sustituye el pánico del
host por esta rutina ni se presenta la limpieza como una retirada por bloque.

Hay llamadas estáticas desde el cargador a begin/factory finish en
`0x01a006ca` / `0x01a007b4`, y un consumidor de held reset en `0x019c2e0a`.
Se ejecutan los callbacks completos, no esos cargadores o llamadores completos;
el harness aporta el orden de la composición. No se infiere la cadencia del
instrumento físico ni se ejecutan transporte, archivos o IPC del original.

## Oráculo

**8.192 casos**, todos comparados byte a byte con el modelo C++ independiente:

- Las 4.096 máscaras posibles de ocupación de doce voces, con bytes no canónicos
  1/127/128/255, además de cero.
- 64 secuencias retenidas de 64 eventos: 128 comienzos y 128 finales de factory,
  256 resets retenidos sobre 2.176 filas y 128 limpiezas aisladas que retiran
  1.728 filas sin borrar su almacenamiento.
- 1.515 flags externos limpiados con salida MIDI desactivada; no son mensajes
  MIDI transmitidos ni validan el receptor del host.
- 2.240 notas iniciadas (704 redisparos, 1.404 asignaciones ocupadas), 384
  note-ons ignorados y 128 retiradas por note-off, 57 con propietario antiguo.
- 5.374 consultas explícitas de actividad; las adicionales del asignador no se
  suman a esa cifra. 8.192 consultas de ocupación, 7.871 con resultado 1.
- 6.144 renders originales / **786.432 muestras**; 5.306 bloques no nulos.

Se comparan query, doce estados, 812 bytes de globals, 2.564 bytes de backing
storage/count, audio, pitch, gain, ganancias derivadas, fase y guardas adyacentes,
y 128 flags externos. Se comprueba la conversión de los 1.025 coeficientes FIR,
PCM intacto, todos los slots seleccionados y las 128 conservaciones del pool
en clear puro. Dos extracciones independientes dan los mismos bytes numéricos.

[Modelo, formato y reproducción](../research/drum-bank/README.md).
[Evidencia y hashes](firmware/drum-bank246-validation.json). Suite Release y
ASan/UBSan completas en Linux; leaks desactivados por ptrace del entorno. CI
nativo (1/1 suite) y regresiones del lector aprobados en Linux, Windows/MSVC y
macOS Intel. No se versionan firmware,
ELF, desensamblado ni pseudocódigo.

## Límites y continuación

Tipo 8, modo drum 1, LFO/FX nulos, salida MIDI cero, índices y transiciones de
fase válidos (fase 0/1). Las inicializaciones, preview, PCM/padding, pitch/gain y tiempos son
entradas del harness; no se atribuyen al arranque original. Las cinco
transposiciones por octavas y la selección de banco no recuperan los productores
de buffers. Los guards del modelo ante fase/índice inválidos son propios; no
reconstruyen la recuperación tras assertions originales.

Se revisaron las candidatas anteriores: `0x01976edc` construye `0x01cc1070`, no
la ocupación `0xff801070`; compartir inmediato bajo no basta para identificar un
objeto. `0x01a856ee` pertenece a held reset, `0x01a85e16` a begin bank,
`0x01a86254` a clear registry y `0x01a865cc` a una reactivación dentro de cambio de
motor. Tampoco deben presentarse como el consumidor normal de final de audio.

Siguiente frente: identificar el llamador de render y el consumidor activo de
las palabras de velocidad/pitch, incluyendo accesos indirectos. No se deduce un
bloqueo L1 únicamente de no encontrar referencias absolutas. La dinámica y la
retirada del host mantienen su composición propia hasta contrastar ese enlace.
Se mantiene **≈69 % ±10 puntos de planificación**; acústica, Live real y presets
originales completos siguen pendientes.
