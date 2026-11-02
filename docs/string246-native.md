# String 246 — waveguide mono reconstruido e integrado

**03 / STRING**, Set 1: Damping, Excitation Decay, Pickup Position y Noise Colour.
Set 2: ADSR reconstruida, Poly de seis voces, Mono/Legato y glide. Set 3/LFO:
Value Instrument/ADSR, Random y Tremolo. Los parámetros se automatizan y guardan
en proyectos, slots y LIFT/DROP. Se sustituye el Karplus–Strong propio anterior;
las recetas propias cambian su sonido y no se convierten en presets originales.

## Núcleo contrastado

El callback mono **`0x01a4f098`** usa una línea de 24.576 celdas int16, excitación
por ruido LCG y dos filtros, realimentación filtrada e interpolación de siete
coeficientes sobre dos posiciones de lectura. Las sumas/MAC saturan en el orden
original; no es una versión del Karplus–Strong anterior con otro nombre.

Se comparan **4096 bloques / 524.288 muestras**, los 32 bytes de estado, RNG,
resultado de la división de pitch y **16 memorias completas de 49.152 bytes**.
Los estímulos combinan estados independientes y secuencias retenidas, controles
extremos y posiciones fraccionarias. La repetición en un segundo directorio
privado reproduce las fixtures byte a byte. Se contrastan también constructor,
binding de slot 0, reset y liberación, incluyendo el estado previo al render.

Las tablas son **legibles, sin sustituciones**:

| Datos | Fuente float32 | Destino y conversión | Entradas |
| --- | --- | --- | ---: |
| Filtro | `0x018be948` | `0x0106a5ec`, Q15 | 129 |
| Interpolación | `0x018bebcc` | `0x0106a730`, Q15 | 225 |
| Caída de excitación | `0x018befb0` | `0x0106a954`, Q31 | 24 |

Se reutilizan los resultados numéricos compartidos con Spring, Delay y Cluster,
y se verifican de nuevo contra las conversiones originales. No se necesitan las
tres normalizaciones desconocidas de Cluster ni los modelos de FM/Nitro/CWO.
El oráculo no distribuye instrucciones, firmware, ELF ni pseudocódigo.

## Controles y ciclo de vida

Los descriptores originales de `0x01de5950` quedan contrastados, incluidos sus
campos de escala para modulación:

| Mando | Mínimo | Máximo | Escala de modulación |
| --- | ---: | ---: | ---: |
| Azul: amortiguación | 64 | 8256 | 8192 |
| Verde: caída de excitación | 512 | 24064 | 23552 |
| Blanco: posición de lectura | 0 | 16384 | 16384 |
| Naranja: color del ruido | 8256 | 16448 | 8192 |

El constructor `0x01a4ef38` limpia los campos definidos, conservando padding.
El binding `0x01a4ef64` asigna el workspace; solo el slot 0 se ejecuta en este
oráculo. El reset `0x01a4efc8` consume dos pasos del RNG, actualiza los dos filtros,
reinicia cursor/contador y establece la excitación a 32767; no borra la línea ni
el historial de realimentación. La liberación `0x01a4efae` cambia la excitación a
256 y activa la amortiguación de release sin borrar los demás campos.

El callback limpia hasta 1536 celdas por bloque durante el arranque y protege
además una ventana de 140 celdas cerca de la segunda lectura. No se reemplaza esa
limpieza progresiva por un vaciado en cada callback. El test conserva y compara
la memoria entre bloques y durante reset/release.

## Adaptaciones del host y límites

El reloj de **88.2 kHz**, la entrega de pitch por bloque de 128 muestras, el
suavizado de controles, asignación de seis voces, último sostenido, glide y rampa
de relevo de 3 ms son propios. No se ha recuperado el scheduler original L1.
La voz usa la ADSR ya reconstruida, DC de 15 Hz, filtro causal de cuatro polos y
conversión de tasa compartidos con otros motores; no son la salida del hardware.

El pitch se convierte a la palabra de fase completa y el callback calcula el
periodo con su división entera original. Se conserva el truncamiento a palabra
baja y el clamp firmado observados, incluidos los casos de frontera del oráculo.
El host limita el divisor positivo a `0x40000..0x7fffffff` y envuelve el cursor
al nuevo periodo al acortarse la línea: política propia para pitch bend/glide y
Tremolo, distinta de afirmar reconstruido su llamador original. No se aplica la
interpolación de pitch desarrollada para FM ni se calibra la afinación a oído.

La voz nueva usa los valores iniciales definidos y el reset original. El panic
retira las voces; en la siguiente nota el callback vuelve a limpiar progresivamente
su workspace. El disparo/reuso de voces no reproduce el allocator original.

Value y Random usan el tipo original **3**. Sus **4096 + 4096 callbacks** se
comparan con estados, display/RNG, ocho destinos y retornos completos; repetición
independiente idéntica. Tremolo conserva su aritmética contrastada y toma la
primera palabra de pitch por bloque, alineada con sus 128 muestras de amplitud.
Cadencia y orden entre voz/moduladores siguen siendo adaptaciones del host.

**Random comparte el RNG con la excitación de String.** Activarlo puede cambiar
el ruido aun con profundidad cero. La prueba de modulación compara profundidad
cero y no cero con Random activo en ambos casos para aislar el efecto de sus
controles; también comprueba que la prioridad sobre otros LFO no altere el render.
Value y Tremolo sí conservan identidad con profundidad cero en las pruebas.

## Validación y siguiente trabajo

Se comprueban siete tasas de host (8–192 kHz), coincidencia de 32 bloques directos
con el adaptador, memoria/RNG, seis voces, extremos de controles/MIDI, release y
panic. Value cubre los cuatro colores hacia Instrument y ADSR; Random ambas rutas
y Tremolo cinco formas. Mono/Legato/glide cubren notas, pedal y bend por canal,
retorno, robo y cambios de modo/motor. La política musical se distingue del núcleo.

El plugin prueba proxies con editor cerrado, cambio FM/String, estado, slots,
clipboard y renders 64/257. Se amplían a **420** las cargas de callback y se
mantiene el volumen maestro después del bus FX y su PDC fijo.
[Evidencia](releases/v0.29.0-validation.json).

Esto cierra otra reconstrucción mono; no demuestra equivalencia acústica con el
OP-1 físico, aceptación en Ableton real ni presets originales. Solo se dispone de
los vídeos y presets ya aportados. La calibración orientativa seguirá separada
de las constantes recuperadas. Después se priorizan los motores restantes según
su código y dependencias legibles; Pulse sigue bloqueado en L1. Los presets
originales completos permanecen para la etapa final.
