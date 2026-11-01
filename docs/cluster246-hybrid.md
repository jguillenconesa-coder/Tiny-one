# Cluster 246: motor híbrido tocable

Selecciona **08 / CLUSTER** o **BROWSE → Cluster 246 Hybrid → Init / Cluster**.
El init es un sonido propio; la réplica completa de presets originales sigue
reservada para la etapa final.

Set 1 expone los cuatro controles del núcleo: **Oscillators, Feedback, Damping y
Drift** (azul, verde, blanco y naranja). Azul cruza de uno a ocho osciladores por
voz; el número se muestra en pantalla. No debe confundirse con las seis voces
polifónicas. Set 2 expone la ADSR reconstruida. Los rangos raw siguen los cuatro
rangos observados del firmware: 3072–17408, 0–32767, 512–24064 y 3–1638.
Release raw alto acorta la cola, como en los otros motores 246.

## Frontera entre evidencia y aproximación

`Cluster246.h` permanece sin cambios: fase, feedback, ocho osciladores, pesos,
damping, detune, RNG y release conservan los oráculos existentes. Se llama a ese
núcleo en bloques de 128 muestras a 88,2 kHz. La ADSR conserva su propio núcleo
contrastado. Esto no recupera el sistema completo del aparato.

La normalización conocida se conserva exactamente para los conteos 1, 4, 5, 6 y
7. **Solo para 2, 3 y 8**, el adaptador `Cluster246Config` usa `floor(16384/n)`:
8192, 5461 y 2048. Son hipótesis del modelo híbrido basadas en la pauta conocida;
no se presentan como bytes recuperados. La API de referencia sigue devolviendo
`nullopt` para esos tres casos. Los 24 coeficientes de damping son los contrastados.

La afinación A4=440 Hz genera un buffer constante de incrementos por bloque.
El detune del núcleo sigue activo y puede separar osciladores de esa referencia.
Cada voz tiene contexto/RNG independiente; su semilla se deriva del orden de
note-on. La cadencia compartida y el RNG global originales no están recuperados.

También son propios la asignación de seis voces, el robo por antigüedad y relevo
lineal de 3 ms, el suavizado de controles en 12 ms, la conexión de ADSR a la salida,
el bloqueo DC de 15 Hz, filtro/resampling causal y mezcla/ganancia/velocidad del
plugin. La liberación envía el release del núcleo una vez; la ADSR decide la
retirada de la voz. Ambos motores híbridos retiran voces al cambiar de motor.
Bend de ±2 semitonos y sustain funcionan por canal. Los bloques internos no se
reinician al cambiar el tamaño de buffer del host.

## Comprobaciones y límites

La suite `cluster246_hybrid` comprueba afinación en una configuración de drift
mínimo a cuatro tasas, bend, sustain/release, seis voces, robo, pánico, 16 esquinas
de controles y extremos MIDI. Verifica los cinco coeficientes observados y las
tres aproximaciones. Además compara 24 bloques completos y sus estados con la
llamada directa al núcleo, incluyendo release y los ocho conteos.

La integración comprueba interfaz/proxies con editor cerrado, estado actual,
slots/clipboard, cola declarada y audio idéntico con bloques host de 64 y 257
muestras. El benchmark añade seis cargas Cluster a las 102 anteriores: primer
disparo y callback continuo sin heap rastreado. Los sanitizadores se aplican al
adaptador. La sintaxis del empaquetado macOS se comprueba localmente; arquitecturas,
firmas locales y contenido del paquete se verifican en el runner macOS.

La comparación acústica completa con hardware, los coeficientes originales que
faltan, los modos mono/legato originales y la validación en Ableton real siguen
pendientes. Un build universal no demuestra por sí solo que Live acepte o ejecute
correctamente el VST3. Esta entrega no necesita una memoria descifrada para ser
tocable, pero tampoco resuelve el cifrado.
