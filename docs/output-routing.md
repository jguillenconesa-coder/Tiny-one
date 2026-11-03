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
