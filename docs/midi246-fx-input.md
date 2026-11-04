# MIDI LFO: audio de entrada al bus FX — 0.44.0

Se añaden dos destinos para cada CC 1–4: **FX Input Bipolar** y
**FX Input Bipolar Steep**. Actúan sobre la entrada de Filter, Nitro, Delay, Grid,
Phone, Spring o CWO. No requieren un motor nativo: también admiten entrada externa
monitorizada, cinta y mezcla. Punch se procesa en otra etapa y estos destinos
aparecen como OFF cuando solo está seleccionado Punch o el bus está en bypass.

Ambas curvas invierten la polaridad por debajo de CC64, cierran la entrada en
CC64 y vuelven a abrirla por encima de ese valor. La segunda alcanza antes sus
extremos. **CC64 no es neutro para estos dos destinos**; sí lo sigue siendo para
los destinos de parámetros. El silencio de entrada no borra las colas ya guardadas
por el FX. No es un control de volumen maestro.

Los cuatro streams actúan en orden y pueden coincidir en la entrada del FX.
Los destinos se automatizan con los mandos contextuales aun sin editor abierto,
y se guardan en proyectos, slots y clipboard. Los CC vivos mantienen la política
anterior: último evento CC1–4 entre canales, valor inicial 64, reset a 64 con
prepare/pánico y conservación con All Notes Off.

## Procedencia y límites

Se reutiliza la rama de audio del callback FX MIDI original `0x019cd104`, kinds
2/5, destino interno 3, colores 2/3. Sus curvas conservan el wrap o saturación
antes de la multiplicación Q15 y el orden secuencial de los cuatro streams.
No se añade un nuevo detector ni se recupera un productor de gain/pitch.

Las **6.144 llamadas del oráculo MIDI existente** siguen coincidiendo byte a byte.
Además, el puente por muestra se compara con **21.568 muestras de audio observadas
en esos mismos fixtures originales**. Esa cifra no es un nuevo conjunto de
llamadas al firmware. El callback original opera sobre los primeros 64 halfwords
de un buffer suministrado de 128; el verificador completo también conserva el
control de los 64 restantes, estados, CC, destinos y retornos.

La integración coloca los multiplicadores **después de cuantizar la entrada del
adaptador y antes del núcleo FX mono**, a 44,1 kHz internos. Lee los CC retenidos
para cada muestra interna usando el reloj/remuestreo causal existente. Las
ventanas, posición respecto a mezcla/cinta/EQ/FX y lectura de automatización son
**decisiones propias del host**, no una recuperación del scheduler o ruteo global
del OP-1. A CC constante, conserva la aritmética del callback; no se presenta el
comportamiento automatizado del instrumento completo como numéricamente idéntico.

El bypass y la señal seca de su transición conservan su ruta anterior. El volumen
maestro continúa después del bus; no se modifica su PDC, reinicio ni remuestreo.
No se conectan las rutas especiales de gain/pitch synth o drum, ni FX kind 4.
El publicador Envelope de Element permanece en investigación hasta identificar
su contrato activo. No hay una nueva medición de fidelidad acústica ni validación
en Ableton Live por este bloque.

## Validación

La suite independiente compara el puente con las observaciones MIDI originales,
los destinos de parámetros y 112 combinaciones de tasa/curva/FX/bypass, incluyendo
CC y rutas cambiantes, dos streams, PDC, reset y partición exacta 64/257. Las
inversiones de entrada de los siete FX a 44,1 kHz conservan exactamente el historial
Q15 del núcleo. La salida float puede diferir un ULP frente a invertir el audio
antes del adaptador, porque la señal seca de comparación es diferente.

La suite del plugin cubre las siete opciones FX, ambas curvas, dos tasas, eventos
con timestamp, bloqueo anterior al evento, PDC, canales, pánico, disponibilidad,
mandos sin editor y persistencia. Los resultados completos, sanitizadores y CI
figuran en [la evidencia de 0.44.0](releases/v0.44.0-validation.json).
