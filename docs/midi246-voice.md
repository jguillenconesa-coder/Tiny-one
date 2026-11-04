# MIDI LFO: ganancia y pitch de voz — 0.45.0

Cada CC1–4 añade **Voice Level** y **Voice Pitch** para FM, Phase, Digital, String,
Sampler melódico, Dr. Wave, Cluster, DNA, Voltage y D-synth. Drum, D-box y motores
heredados sin núcleo nativo muestran OFF para estos destinos. No se cambia el
pitch wheel por canal ni se añade una ganancia maestra.

Voice Level lleva la voz hacia silencio con CC0, toma CC64 como referencia nominal
y permite subir por encima de ella dentro de los límites de ganancia del motor.
Conserva el suavizado original por bloques; una voz nueva empieza con una breve
rampa. La cuantización y saturación forman parte de la respuesta, por lo que CC64
no se presenta como una identidad bit a bit con la ruta desactivada.

Voice Pitch aplica la ley cúbica original alrededor de CC64. El incremento sigue
sujeto a los límites del motor: String y DNA conservan sus límites positivos para
no utilizar un divisor cero. Las rutas de varios CC se aplican en orden; pueden
coincidir en ganancia o pitch. Los encoders contextuales permiten asignarlas sin
editor abierto y los destinos se guardan en proyectos, slots y clipboard.

## Núcleo contrastado

Se separan las operaciones especiales del callback synth MIDI `0x019cce68`,
kind 0, destino 3, colores 0/1:

- Ganancia: actualización de una palabra de display por stream con coeficientes
  Q15 4096/28672, multiplicación del buffer y duplicación saturada por stream.
- Pitch: CC convertido a Q15 saturado, tercera potencia con truncamiento Q15,
  multiplicación Q31 sobre la palabra signed suministrada y escalado x8 saturado.

Las **6.144 comparaciones originales MIDI** siguen coincidiendo en estado completo,
CC, display, destinos, pitch, gain, audio FX y retornos. Los puentes por muestra y
por bloque se comparan con **11.392 pares de pitch/ganancia** de los mismos fixtures
originales, incluido display. También se conserva la comprobación de 21.568
muestras FX de 0.44.0. No son nuevas ejecuciones del firmware ni un nuevo oráculo
del instrumento completo. [Resultados](releases/v0.45.0-validation.json).

## Composición propia de la voz

El adaptador captura destinos y CC al iniciar bloques de 128 muestras internas a
88,2 kHz. Prepara una única actualización de ganancia por bloque y conserva el
orden secuencial de los cuatro streams. Los cambios recibidos durante un bloque
quedan pendientes para el siguiente. La asignación y combinación de CC entre
canales es la del host ya documentada para MIDI LFO.

Phase y Digital consumen pitch por muestra. Dr. Wave, Cluster, Voltage, D-synth y
Sampler reciben sus buffers de pitch preparados; FM, String y DNA conservan la
lectura del primer word del bloque que ya usaban sus adaptadores. La ganancia se
aplica al contorno Q15 antes de los filtros de salida. En D-synth se modifica el
buffer de gain suministrado al núcleo, antes de sus filtros reconstruidos. La
posición de estas operaciones es una composición propia alrededor de interfaces
conocidas, no una recuperación del productor ni del ruteo original de buffers.

Cada voz del host posee cuatro palabras de suavizado que empiezan a cero. Cambiar
hacia rutas de parámetros conserva esos coeficientes mientras MIDI siga activado;
desactivar MIDI los limpia. Note-off mantiene la evolución mientras la voz siga
activa. Legato conserva el estado de la voz reutilizada; retrigger o reemplazo
crean estado nuevo conforme al allocator propio. Prepare y pánico reinician voces
y CC a 64. Los valores CC vivos y coeficientes no se incluyen en un preset.

Estas decisiones de inicialización, ownership, capturas, reloj, modos de voz,
glide, límites y remuestreo son **adaptaciones del host**. No se ha recuperado la
inicialización/propiedad física del display, el scheduler L1, OTP, ni la voz/ruta
completa del aparato. La concordancia del cálculo bajo entradas suministradas no
demuestra equivalencia acústica. El PDC anunciado, volumen maestro, bus FX y sus
rutas de entrada de 0.44.0 permanecen en sus posiciones documentadas.

## Validación

La suite independiente añade 182 combinaciones motor/tasa/destino, con siete tasas,
referencia nominal frente a CC con timestamp, comparación exacta 64/257, seis estados
independientes, rechazo de drum/legacy, modos Mono/Legato/glide, mantenimiento del
coeficiente al cambiar rutas y reset. La suite del plugin comprueba ambos destinos
en los diez motores a dos tasas, PDC, estéreo, release/pánico, mandos sin editor,
proyectos, slots y clipboard. Las pruebas de callback incluyen otras 120 cargas de
tiempo real. Los resultados finales y CI figuran en la evidencia enlazada.

Siguen pendientes Ableton Live real, calibración acústica y presets originales
completos. Para este bloque no se necesitan nuevas grabaciones.
