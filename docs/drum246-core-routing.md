# Core: creación y disparo del drum sampler rev246

Bloque de investigación del 5 de noviembre de 2026. **El instrumento sigue en
0.52.0**: no cambia el ejecutable ni necesita instaladores nuevos. El CI completo
de esa versión terminó aprobado en Linux, Windows/MSVC y macOS universal
(runtime Intel). [Evidencia de producto](releases/v0.52.0-validation.json).

Se reconstruye y contrasta el enlace entre Core y los doce objetos de audio del
sampler, tipo 8. El nombre `s_drum_producers` identifica esos objetos; **no es el
productor original de ganancia/envolvente**. La dinámica propia documentada en
[el lector integrado](drum246-native.md) permanece en el instrumento.

## Contrato observado

| Ruta/dato original | Dirección y comportamiento |
| --- | --- |
| Factory de Core | `0x01967984`, llama `0x01a1b9c0` con tipo, tabla de objetos y sonido seleccionado |
| Factory tipo 8 | Doce constructores `0x01978df4`, getter `0x01978da8`, bind `0x019788ec` |
| Objetos / tabla de punteros | `0xffb00000 + voz × 20` / `0x01aaad30[12]` |
| Referencias de muestras | `0x01cc10cc[8]`, handles suministrados como puntero a puntero PCM |
| Disparo / release Core | `0x01967ccc` / `0x01967d64` |
| Sound / mapa voz→pad | `0x01aaac10` / `0xff801334[12]` |
| Argumento de controles | `0x01aaadb4 + sound × 384 + pad × 16` |
| Despacho de disparo | Función de vtable `+0x1c`, ajuste firmado de this en `+0x18` |
| Despacho de release | Función de vtable `+0x2c`, ajuste firmado de this en `+0x28` |
| Vtable sampler | `0x0190fac4`: reset `0x01978926`, release `0x01978898`, render `0x019789ec`; ajustes reales cero |

El constructor conserva el contador de bloques mientras inicializa tag, handle,
posición y modo. La factory enlaza el mismo handle seleccionado a las doce
voces. Disparo/release eligen controles por el sonido actual y el mapa de pads;
no exigen que ese sonido coincida con el del handle enlazado. Esto contrasta con
la guarda de igualdad de ciertos despachos Core de seis voces, y no autoriza
inferir cómo la interfaz original programa los cambios de banco.

Los endpoints de trazado son código propio: permiten comprobar los argumentos y
los ajustes firmados del despacho original sin dereferenciar un this artificial.
Después, la misma ruta original ejecuta reset/release reales del sampler con
su vtable original. No se atribuye el cuerpo de los spies al firmware. No se
compara un supuesto retorno booleano de estos callbacks void.

## Oráculo y comparación

4.608 casos cubren dos veces la rejilla de 12 voces × 8 sonidos × 24 pads: una
pasada de disparos y otra de releases, con acciones combinadas y secuencias que
conservan estado. Incluyen 159 factories, 2.513 resets, 2.514 releases y 3.456
bloques originales renderizados (442.368 muestras; 1.614 bloques no nulos).
3.088 renders usan un handle de banco distinto al seleccionado para controles.

El verificador compara todos los argumentos trazados, los doce punteros, todos
los estados antes/después, audio, pitch, gain, ganancias derivadas y el banco
completo de controles. Comprueba la conversión original de los 1.025 coeficientes
FIR y que el PCM permanezca intacto. Los renders omitidos conservan el veneno de
audio y el buffer derivado previo. Dos extracciones independientes producen las
mismas entradas y salidas numéricas. No se suman los bloques silenciosos a los
no nulos ni se presentan llamadas trazadas como audio adicional.

[Modelo, formato y reproducción](../research/drum-core/README.md).
[Evidencia y hashes](firmware/drum-core246-validation.json). Suite independiente
Release y ASan/UBSan completa aprobadas en Linux; leak detection desactivado por
ptrace del entorno. CI nativo separado aprobado: 1/1 suite en Linux, Windows/MSVC y macOS Intel.
El modelo y los fixtures no se enlazan al plugin; tampoco contienen firmware,
pseudocódigo ni desensamblado original.

## Límites y siguiente bloque

El harness suministra índices válidos, PCM/padding, referencias, mapa de pads,
controles, pitch, gain y orden de eventos. Los guards de índices del modelo son
propios; no reconstruyen la recuperación tras assertions originales. Se cubre
el tipo 8, sin extender la prueba a la factory alternativa de D-box (tipo 9).

No se recuperan el productor de dinámica, el productor de pitch, la selección
MIDI, el orden/cadencia del scheduler, la decisión de consultar actividad o
retirar una voz, ni el mezclador completo. Tampoco se mide equivalencia acústica.
La siguiente dependencia concreta es el llamador activo de render/actividad de
estos doce objetos y los escritores de sus entradas pitch/gain. Esa composición
se contrastará antes de sustituir la dinámica o el allocator del instrumento.
Se mantiene **≈69 % ±10 puntos como estimación de planificación**; presets
originales completos, Live real y calibración acústica siguen pendientes.
