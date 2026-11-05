# Crank: ganancia y pitch de voz — 0.48.0

Selecciona **Crank** en LFO y **Voice** en White. Orange elige **Level** o
**Pitch**, Green conserva Amount bipolar y Blue mueve el controlador con inercia.
Se integra en los diez motores melódicos nativos; drum/D-box y motores heredados
muestran OFF. El movimiento manual, frenado y fase detenida siguen siendo los de
Crank. No se añade una ganancia maestra ni se modifica el pitch wheel MIDI.

Crank detenido conserva su fase: puede mantener atenuación o cambio de pitch.
Amount cero conduce hacia nivel unidad y pitch sin desplazamiento. Se guardan los
controles y destinos en proyectos, slots y clipboard; trayectoria, coeficiente y
captura viva se reinician con el lifecycle propio del host.

## Contrato contrastado

Callback synth `0x019cb0d4`, helper compartido `0x019cae26`, kind 0, destino 3,
colores 0/1. **18.432 llamadas originales adicionales**: 12.288 synth y 6.144 de
rechazo drum. Hay **2.060 aceptadas** (1.040 Level y 1.020 Pitch) y 16.372 rechazadas.
Dos ejecuciones independientes producen entradas y salidas idénticas.

Se comparan tag, siete palabras de estado, diez entradas de historial, ángulo
anterior, palabra de ganancia, ocho controles, ocho targets, retorno, 128 pitch
words, 128 gain halfwords y 128 FX audio halfwords. Se inicializan los buffers con
patrones explícitos por llamada y se cargan o retienen los estados. Los fixtures
cubren bordes de fase, ambos sentidos y las cuatro combinaciones de mitad del
triángulo/signo de Amount. No son una captura de los productores originales.

El puente de voz coincide también con los **263.680 pares de pitch/ganancia** de
todos los bloques aceptados. El oráculo anterior de parámetros de Crank se conserva
como una comprobación separada. Se utiliza GNU BF524 con los tres parches de
writeback y sus probes; no es una medición sobre hardware.

## Curvas de Crank

El nivel triangular es `phase` hasta 127 y `abs(phase − 255)` después. Ambos ápices
se conservan; el centro de la modulación es nivel 64.

**Level** usa `delta = Q15(sat16((level − 64) × 256), Amount)`. Se suma 16384 con
saturación y se filtra con pesos Q15 4096/28672. El coeficiente vive en el halfword
bajo de **`0x01dd59bc`**, separado del historial y del estado de Bend; el halfword
alto se conserva. Cada gain se multiplica por él y por dos con saturación.
Puede saturar una envolvente que ya esté a fondo de escala.

**Pitch** escala `(level − 64)` por **64 en la mitad inferior** y por **512 en la
superior**, antes de aplicar Amount Q15. La selección depende de la mitad del
triángulo, no del signo de Amount. A cada palabra de pitch se suma siete veces el
producto fraccionario, respetando el orden de saturación. Esta operación de
palabras coincide con la primitiva de Bend; sus coeficientes y fuentes se
contrastan aparte.

Con cantidades negativas, la aritmética observada puede producir palabras de
pitch negativas bajo estas entradas. El plugin conserva la operación y las
restricciones ya documentadas de cada motor: FM/String/DNA limitan su divisor
positivo y los demás mantienen sus adaptadores. No se presenta esa composición
como afinación física equivalente del OP-1 ni se sustituye por una curva propia.

## Qué aporta el host

Crank conserva una fuente y un coeficiente globales, actualizados por el tick
propio `44100/64`. Level avanza su suavizado una vez por tick; Pitch conserva el
último coeficiente de Level. Cada voz captura target y coeficientes en bloques de
128 muestras internas a 88,2 kHz; las voces no avanzan el suavizado compartido.

La captura común de Bend/Crank descarta el bloque anterior al cambiar de dueño.
Ambos controladores conservan fuentes y coeficientes distintos. Los motores con
caché siguen su calendario de render en bloques. Phase/Digital consumen pitch por
muestra; FM/String/DNA conservan una palabra por callback; D-synth recibe gain en
su bloque anterior al núcleo. El resto conserva sus puentes de pitch/gain.

Inicialización a cero, clock, orden de callbacks, captura, límites, composición
con ADSR/velocidad/salida y resets son políticas del host. No recuperan el
scheduler, allocator ni productor original de buffers. Los flags y el tick de la
fuente se contrastan sobre estados suministrados; no se recupera el lector del
accesorio físico. Cambiar de notas no reinicia la fuente manual global.

## Validación y pendientes

[Resultados y CI](releases/v0.48.0-validation.json). La integración cubre siete
frecuencias, diez motores y dos destinos, rechazo drum/legacy, ambos signos y cero
en los diez motores, Mono/Legato/glide, captura/cambio de dueño, retención del
coeficiente, pánico y audio exacto 64/257. Los ensayos dejan tiempo explícito para
inicialización e inercia, en vez de confundir una fase aún detenida con un fallo.
El plugin comprueba rutas, automatización, proxies sin editor y persistencia.

```
cmake -S research/crank-voice -B build-crank-voice -DCMAKE_BUILD_TYPE=Release
cmake --build build-crank-voice --config Release --parallel 2
ctest --test-dir build-crank-voice -C Release --output-on-failure
```

La reproducción original usa `scripts/verify-crank-voice246.py`, LDR privado y
las herramientas ya documentadas. Salidas nuevas fuera del checkout; `--record`
registra únicamente entradas y observaciones numéricas. Los modos Crank de los
secuenciadores y los especiales de FX audio/ENV no forman parte de esta entrega.
Siguen productores/ruteo/voz originales, Element especial, Pulse/L1, acústica,
Ableton real, firma/notarización y presets originales completos al final.
