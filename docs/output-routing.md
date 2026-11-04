# Volumen y bus de salida desde 0.24.1

El volumen maestro escala la salida completa, incluidas las colas de Filter,
Nitro, Delay, Grid, Phone, Spring y CWO. Antes se aplicaba a la entrada de ese bus: una cola
ya almacenada podía seguir sonando al bajar el volumen, y mover el volumen
cambiaba la excitación, cuantización y realimentación del efecto.

La cadena propia queda así:

1. Síntesis y Punch, entrada monitorizada, cinta y mezcla.
2. Metrónomo, EQ y drive/eco maestro.
3. Bus mono Filter/Nitro/Delay/Grid/Phone/Spring/CWO o bypass con su retardo fijo.
4. Volumen maestro suavizado (5 ms), medidor y salida idéntica a ambos canales.

El cero se alcanza exactamente cuando el suavizado cae por debajo de `1e-7`.
Los demás niveles terminan exactamente en el destino cuando el error baja de
`1e-5`, evitando que el redondeo de float detenga la convergencia.
El volumen no vacía las memorias: al volver a subirlo reaparece la cola restante.
Pánico/CC120 siguen borrándolas. El PDC continúa siendo
`ceil(64 × tasa_host / 44100) + 1` muestras, también en bypass.

Esto es una decisión del adaptador propio; no demuestra el ruteo original ni
una cadena estéreo del OP-1. La grabación y exportación de pistas crudas de cinta
siguen tomando la señal antes del procesamiento maestro y del volumen de salida.

La regresión compara salida a ganancia 1 y 0,25 en nueve selecciones de FX y
44,1/48/96 kHz, con síntesis, entrada monitorizada y colas de eco. Comprueba cero
exacto tras el descenso, conservación de la cola al desmutear, canales iguales,
PDC y automatización independiente del buffer del host (64/257).


## MIDI LFO 0.39.0

CC1–4 se retienen por timestamp de muestra y modulan los parámetros del instrumento,
su ADSR nativo o los cuatro controles del FX seleccionado. Punch se aplica en la
síntesis; los otros siete efectos conservan el bus mono anterior. Esto no mueve
el volumen maestro, cambia el PDC ni implementa el ruteo global original.
La modificación de buffers de pitch/gain/audio contrastada en el núcleo todavía
no está conectada a esta cadena. [Contrato](midi246-hybrid.md).


## Bend manual 0.41.0

Un controlador compartido avanza con un reloj de host propio y aplica la entrada
virtual de Orange a Instrument/ADSR o cuatro controles del FX activo. No modifica
la posición del volumen maestro ni el PDC. Las rutas especiales de audio/pitch
originales se mantienen fuera del contrato; no se reconstruyen los productores
de esos buffers. El pitch wheel MIDI por canal sigue independiente.
[Contrato](bend246-hybrid.md).


## Crank 0.42.0

Crank comparte una fuente manual con inercia entre voces y FX; solo se conectan
parámetros Instrument/ADSR/FX. El encoder y el reloj de 44100/64 del host son propios.
Una fase detenida conserva su señal bipolar; no se fuerza su salida a cero. No se
conectan las rutas originales de ganancia/pitch/audio ni se añade ganancia maestra.
Se mantienen el bus de salida y su PDC; Release/drum prolongables anuncian una cola
conservadora. [Alcance](crank246-hybrid.md).


## Element 0.43.0

External Input recibe el promedio estéreo del bus existente tras Input Gain aun
con monitorización directa apagada; no añade el audio de entrada a la mezcla.
Envelope usa el máximo ADSR global; Level observa el sum del instrumento previo
a efectos, cinta y volumen maestro, con una muestra causal y ventana de host
propias. No se recuperan sus productores originales ni su composición por voz.
Solo se conectan parámetros Instrument/ADSR/FX; especiales gain/pitch/audio y FX
kind 4 siguen excluidos. El PDC/volumen maestro se mantienen y los LFO manuales
tienen prioridad sobre el filtro implícito del LFO propio. [Alcance](element246-hybrid.md).
