# Random sobre los efectos — 0.46.0

En la página LFO selecciona **Random 246** y destino **FX** con el mando blanco.
Azul controla cantidad bipolar, verde velocidad y naranja la envolvente del LFO.
Se modulan simultáneamente los cuatro controles del efecto seleccionado: Punch,
Filter, Nitro, Delay, Grid, Phone, Spring o CWO. No hay selector de color ni banco
extra: las cuatro señales tienen evoluciones distintas. Cantidad cero conserva
las bases válidas, pero se aplican los clamps originales a bases fuera de rango.
La velocidad no sigue tempo; la pantalla muestra la tasa de nuevos objetivos.
Naranja por debajo de 16384 hace crecer la profundidad; desde ese umbral decae,
por lo que conviene tocar una nota para iniciarla.

FX está disponible con motores nativos y heredados. Elegir Random en un motor
heredado selecciona FX automáticamente. Instrument/ADSR conservan
sus restricciones de motor; ADSR sigue rechazado en batería/D-box. OFF / FX indica
que no hay un efecto seleccionado. Activar Random excluye los otros LFO de menor
prioridad; Element, Crank, Bend y MIDI conservan su prioridad existente. El destino
FX retira la modulación Random por voz en vez de aplicarla también a ADSR.

Se guardan cantidad, velocidad, destino y envolvente en proyectos, slots y
clipboard, con automatización y mandos contextuales sin editor. Los valores vivos
del generador aleatorio y su historial no forman parte del preset.

## Parte reconstruida y contrastada

El callback FX Random `0x019cefb0`, reset `0x019cf2f4`, acepta destino 2
(`floor(white/2048)`) y kinds 2/5. Se reutilizan la secuencia LCG, contador uint32,
ocho objetivos, suavizado Q15 y envolvente contrastados para Random. Cada una de
las ocho señales aplica su propio rango FX, **sin** la segunda pasada dividida
por 16 del callback synth. El reset solo cambia la profundidad; contador,
objetivos, suavizado y RNG sobreviven. El display de 32 bits conserva su mitad alta.

**4.096 llamadas originales** incluyen 2.048 estados independientes y 2.048 pasos
retenidos, los ocho tipos FX, ambos kinds y rechazos. Coinciden los 44 bytes de
estado, RNG, display, ocho destinos y retorno: 3.799 llamadas aceptadas y 297
rechazadas. Una repetición en otro directorio privado produce resultados idénticos.
El adaptador de cuatro controles coincide con los mismos casos aceptados y se
comprueba además con secuencias retenidas y bases cambiantes. No se reutilizan
estas llamadas como evidencia del scheduler ni del aparato completo.
[Evidencia original](firmware/random-fx246-validation.json).

La otra rama del callback (kind 4, destino 1) queda fuera de este bloque. No se
atribuye reconstrucción de su consumidor ni se conecta por semejanza con ADSR.

## Adaptador propio del host

Hay un estado global del bus, distinto del estado Random de cada voz. El reloj
propio produce 689,0625 callbacks/s y retiene ocho modulaciones entre callbacks;
cada muestra vuelve a aplicar las primeras cuatro sobre las bases actuales, sin
acumular resultados. Las otras cuatro palabras se contrastan en el núcleo pero
no se convierten en controles nuevos del efecto.

Una nota válida reinicia únicamente la profundidad según naranja, incluso en
Mono/Legato; el siguiente tick conserva objetivos y contador. La secuencia utiliza
el RNG compartido del Engine, sembrado con `0x52414e44` en prepare/pánico. Con FX
seleccionado como destino el reloj avanza también sin notas o con el efecto en
bypass, para no depender del tamaño de bloque o de la presencia de una voz.
Cambiar entre tipos FX conserva la secuencia y aplica el rango del efecto nuevo.
Salir del destino FX o desactivar Random descarta este estado; pánico lo reinicia.

Ownership global, seed, cadencia, captura de parámetros, inicialización y ciclo
de vida son decisiones propias. No se recuperan el caller/allocator originales,
L1 ni OTP. La coincidencia numérica condicionada no demuestra equivalencia acústica.
Punch conserva su posición previa a la mezcla; los otros siete FX conservan su
bus mono posterior. No se modulan volumen maestro ni PDC, no se borran colas por
retirar Random. La posición del camino seco se conserva. Al compartir RNG con
String/DNA, avanzar Random puede cambiar su secuencia de excitación/ruido incluso
sin un FX activo; no se promete identidad del audio seco al activar esta ruta.
Los controles propios del
FX siguen automatizables; Random modifica sus valores de render, no los parámetros
guardados ni sus notificaciones al host.

## Comprobación

[Resultados del bloque](releases/v0.46.0-validation.json). Se comprueban estado,
reloj a siete tasas, bypass, nota/reset, exclusividad y señales visuales; el plugin
comprueba los ocho FX a dos tasas, igualdad exacta de audio estéreo 64/257,
automatización temporizada de cantidad/destino, release/pánico, PDC y persistencia.
Se añaden 48 cargas de tiempo real. Ableton real, calibración acústica,
firma/notarización y presets originales completos siguen pendientes.

Para reproducir el oráculo, usa `scripts/verify-random-fx246.py` con los mismos
argumentos privados de firmware/herramientas que el verificador Random synth.
La ruta de salida debe ser nueva y estar fuera del checkout. `--record` escribe
solo los dos fixtures numéricos; firmware, ELF y desensamblado permanecen privados.
Las cuatro suites portables se construyen desde `research/random-modulation`.
