# Estado y entregas del proyecto — 4 de noviembre de 2026

Objetivo: instrumento inspirado en OP-1 original, standalone macOS/Windows y
VST3 para Ableton Live; avanzar hacia la reconstrucción verificable de motores,
efectos y flujo. La réplica de todos los presets originales queda al final.
Esta cuenta es la coordinadora principal según la instrucción del propietario.
Las notas de coordinación entre cuentas anteriores se conservan como historial,
no como reservas indefinidas de tareas ya entregadas o sin PR abierto.

## Avance estimado: aproximadamente 68 % del objetivo completo

Es una estimación de planificación, **no una medición de equivalencia sonora**,
ni un cociente de muestras, pruebas o commits. La incertidumbre es de unos
±10 puntos: las dependencias L1 protegidas impiden estimar con precisión el
trabajo de fidelidad que falta. Se usa esta ponderación para no confundir un
prototipo tocable con una réplica completa:

| Área | Peso del objetivo | Avance estimado del área | Aportación |
| --- | ---: | ---: | ---: |
| Motores originales y sus filtros | 30 % | 86 % | 25,8 puntos |
| Efectos, envolventes y moduladores | 20 % | 95 % | 19 puntos |
| Voz, modos de ejecución y ruteo original | 15 % | 37 % | 5,55 puntos |
| Interfaz, cinta, mezcla y flujo creativo | 15 % | 75 % | 11,25 puntos |
| Distribución macOS/Windows y validación Live | 10 % | 60 % | 6 puntos |
| Todos los presets originales fieles | 10 % | 0 % | 0 puntos |
| Total | 100 % | | **≈68 %** (67,6 antes de redondear) |

La base funcional de software está bastante más adelantada (orientativamente
70–75 %): VST3/standalone, MIDI, estados, interfaz, cinta y motores propios ya
funcionan. Ese porcentaje no se suma al anterior. Phase/Digital/Punch/Filter/Tremolo/Random y partes
de Value/ADSR tienen reconstrucciones contrastadas; Dr. Wave y Cluster incorporan motores híbridos tocables con
dependencias propias explícitas. FM Hybrid incorpora cuatro operadores y nueve
rutas contrastadas bajo diez sustituciones; siguen pendientes la calibración
acústica y partes del ruteo/arranque original. La [matriz técnica](original-op1-coverage.md) es la
fuente del estado por componente y prevalece sobre este resumen orientativo.

Hay instalador Windows y builds multiplataforma en CI. El CI 0.19 pasó también macOS universal y su paquete; cada cambio nuevo necesita su propio CI.
Falta validación actual en Ableton real de ambos sistemas; el
CI nativo o Wine no demuestra por sí solo aceptación del scanner de Live,
latencia física o ausencia de dropouts. No se afirma que el 100 % esté cerca.

## Entrega actual: fuente drum compartida (0.40.0)

[Una fuente de 12 segundos y 24 regiones](shared-drum246.md): importación WAV/AIFF,
captura, límites editables, doce voces, proyectos, slots portables, clipboard y
vista de regiones. Un PCM compartido; conversiones y división inicial igual son
propias. Reutiliza el lector contrastado, sin nuevo oracle de firmware ni nueva
medición acústica. **120/120 suites Linux**, ASan/UBSan y **702 cargas sin heap
rastreado**, importación/captura/estado/slots/clipboard y vista final aprobados.
[Pruebas y CI](releases/v0.40.0-validation.json). [CI nativo](https://github.com/jguillenconesa-coder/Tiny-one/actions/runs/41738336295):
**6/6 suites en Linux, Windows/MSVC y macOS Intel**. MIDI/D-box/sampler también verdes;
plugins/instaladores completos en curso sobre `ddb76e2`.

La estimación sube a **≈68 % ±10 de planificación** por completar el layout y el
flujo de datos/captura/edición/guardado compartidos. La tabla refleja ese cambio;
no asigna un porcentaje de fidelidad sonora. El CI completo de 0.39.0 terminó bien
en Linux, Windows/MSVC y macOS universal, runtime Intel, y sus regresiones también.

Siguen pendientes productores de pitch/ganancia y ruteo original, layout/segmentación
originales, Element/Bend/Crank, Pulse/L1, calibración, Live/arm64 y presets completos.

## Entrega anterior: MIDI LFO (0.39.0)

[Contrato y límites](midi246-hybrid.md): cuatro CC entrantes y destinos independientes
Instrument/ADSR/FX, automatización contextual, estado y OLED. **6.144 callbacks
originales** contrastados, 2.048 por synth/drum/FX, con entradas y buffers explícitos.
El receptor, asignación CC1–4, combinación de canales y scheduling por timestamp
son propios. Las rutas de modificación gain/pitch/audio se contrastan en el núcleo,
pero no se exponen todavía en la integración ni recuperan sus productores.

**119/119 suites Linux**, ASan/UBSan, 678 cargas sin heap rastreado, estado,
proxies sin editor, timestamps y los ocho FX aprobados; vista MIDI revisada.
[CI MIDI](https://github.com/jguillenconesa-coder/Tiny-one/actions/runs/41705154163):
**2/2 suites en Linux, Windows/MSVC y macOS Intel**. Plugins/instaladores completos
y regresiones aprobados sobre `2f5657f`, sin release publicada.

La estimación se mantiene en **≈67 % ±10**: se cierra una parte acotada de modulación,
sin nueva medición acústica, recuperación de L1 ni validación Live. Véase la
[evidencia de entrega](releases/v0.39.0-validation.json) para pruebas y CI actuales.
El CI completo de 0.38.0 pasó en Linux, Windows/MSVC y macOS universal (runtime Intel),
y también terminaron correctamente sus regresiones FM y D-synth.

Sigue el estudio de productores/ruteo original y las entradas Element/Bend/Crank,
fuente drum compartida, Pulse/L1, calibración con el material disponible, Ableton
real, ejecución arm64 y presets originales al final.

## Entrega anterior: Value y Random de batería (0.38.0)

[Modulación drum/D-box](drum246-modulation.md): ocho destinos de Instrument por
voz, dos bancos de Value, Random directo, estado/proyectos/slots/clipboard y proxies
sin editor. **16384 callbacks originales** coinciden con todos los estados,
displays, destinos, RNG, retornos y tempo; se repiten con fixtures idénticos.

El destino ADSR de esta ruta **se rechaza** y se indica OFF. No se usa el ADSR
común como sustituto de la dinámica drum. Helper propio contrastado de Start/End y
fracciones, descriptores de ambos tipos y secuencia Random compartida como aritmética.
Reloj, mapa de pitch/modos/dirección, asignación, cache, dinámica y salida propios.

Linux: **117/117 suites**, cinco suites independientes, ASan/UBSan y **630 cargas
sin heap rastreado** (48 nuevas de LFO drum/D-box). Estado/audio/proxies, 64/257
frames y vistas revisados. Las cinco suites nativas pasan en Linux, Windows/MSVC
y macOS Intel sobre `38f01a6` (include explícito del verificador, sin cambios DSP).
Plugins/instaladores completos y todas las regresiones aprobados en las tres
plataformas: [evidencia](releases/v0.38.0-validation.json), macOS universal con
runtime Intel. La carga de
`0xff801d50` en L1 sigue sin un productor recuperado; el dispatch nuevo no lo genera.
Continúan productores de pitch/ganancia, voz/ruteo/scheduler originales, fuente única
drum de 12 s/24 regiones, moduladores pendientes, Pulse/L1, acústica/Live/arm64 y
presets completos al final. Estimación **≈67 % ±10 de planificación**.

## Entrega anterior: moduladores del sampler (0.37.0)

[Value/Random/Tremolo en sampler](sampler246-modulation.md): ocho destinos de
Instrument con reglas coarse/fine acopladas, ADSR, automatización, estado y proxies
sin editor. Value selecciona sus ocho campos por el banco de set 1. Random recorre
los ocho directamente. Dirección usa un mapa propio del botón al rango nativo.

**8192 callbacks originales** de Value/Random coinciden con estado, displays,
destinos, RNG/tempo y retornos; corpus independientes/retenidos, regiones válidas,
igualdades y palabras con signo. Repetición idéntica. Tremolo reutiliza aritmética
ya contrastada; su composición con esta voz y el reloj/allocator siguen propios.

Linux: **114/114 suites**, cinco suites independientes, ASan/UBSan, **582 cargas
sin heap rastreado** (60 sampler), estado/audio y cinco vistas revisados. Las cinco
suites del sampler pasan en Linux, Windows/MSVC y macOS Intel; plugins/instaladores
completos aprobados: [evidencia](releases/v0.37.0-validation.json). No se reclama
fidelidad acústica. Random drum tiene callback/helper legibles y 4096 observaciones
originales aisladas, aún sin port C++ contrastado: no se habilita ni se confunde
con la dinámica. Siguen dispatch Value/Random drum/D-box, productor de ganancia,
ruteo/voz/clock originales, Pulse/L1, Live/arm64 y presets completos al final.
Estimación **≈66 % ±10 puntos de planificación**. Materiales disponibles suficientes.

## Entrega anterior: controles y voz del sampler (0.36.0)

[Sampler 246](sampler246-hybrid.md) expone ocho campos, incluidos límites de loop
independientes y fine, dirección y volumen. Dos bancos de mandos, unidades ms/us/%,
cuatro marcadores, automatización, estado/slots/clipboard y proxies sin editor.
Añade ADSR 246 opcional y Poly/Mono/Legato/glide con sustain/bend por canal.

El lector y ADSR conservan sus verificaciones originales; la unión, clock,
allocator, eventos, dither/seed, LOOP gate y salida son propios. Se compara la
composición explícita en 32768 muestras, estados/dither, y se prueba interpretación
a seis tasas con ambas envolventes. No es prueba de la voz completa original.

El CI completo de 0.35.0 pasó Linux, Windows y macOS universal (runtime Intel).
Validación local: **112/112 suites**, tres suites independientes, ASan/UBSan y
**546 cargas sin heap rastreado** (24 del sampler). Interfaz revisada y estado/audio
verificados. El verificador del sampler y las regresiones drum/D-box pasan en
Linux, Windows/MSVC y macOS Intel. Plugins/instaladores completos aprobados:
[evidencia y CI](releases/v0.36.0-validation.json).
Estimación **≈65 % ±10 puntos de planificación**. Pendientes: ruteo/voz original,
moduladores del sampler y drum, dinámica de batería, Pulse/L1, acústica/Live/arm64
y presets completos al final. No hacen falta grabaciones nuevas para este bloque.

## Entrega anterior: sampler melódico 246 (0.35.0)

[Sampler melódico](sampler246-hybrid.md): callback original `0x01a51a7c`, seis
estados de 16 bytes, lifecycle, FIR, pitch escalado y regiones de loop/reverse.
4096 bloques / 524288 muestras coincidentes bajo PCM/padding/pitch/factor y eventos
suministrados; coeficientes legibles sin sustitución. Repetición independiente
idéntica; núcleo/adaptador ASan/UBSan y dos suites independientes aprobados.

Opción sobre 04 / SAMPLER con seis voces, reverse, Root/Loop/Start/Length, proyectos,
slots y clipboard. Mapa de trims/loop, cache 44,1 kHz (primeros ~6 s), reloj, pitch,
ADSR/velocidad/allocator y salida son propios. El núcleo contrasta ocho campos;
no se han expuesto todos sus controles secundarios. La voz original completa y
la equivalencia acústica siguen pendientes. [Validación](releases/v0.35.0-validation.json).
Release Standalone/VST3 y 111/111 suites Linux aprobadas; 534 cargas sin heap
rastreado, estado/slots/clipboard, 64/257 frames y revisión visual aprobados.
CI lector/adaptador aprobado en Linux, Windows/MSVC y macOS Intel sobre
`16d5738`; plugins/instaladores completos también aprobados.

El CI completo de 0.34.0 pasó las tres plataformas (macOS runtime Intel). La
preparación drum tiene una referencia candidata en L1 parcialmente protegida;
no recupera por sí misma el productor de ganancia ni cambia su curva propia.

Estimación **≈64 % ±10 puntos de planificación**, no de fidelidad sonora. Siguen
los controles secundarios del sampler/voz, dinámica y dispatch drum, ruteo original,
Pulse/L1, calibración, Ableton real, runtime arm64 y presets completos al final.

## Entrega anterior: sampler de batería 246 (0.34.0)

[Sampler 246](drum246-native.md) añade el lector reconstruido al kit existente:
doce voces, FIR original, trims finos y modos Gated/One Shot/Loop. Callback completo
contrastado en 4096 bloques / 524288 muestras, doce estados y todos los buffers,
bajo PCM/padding/pitch/ganancia explícitos. 1025 coeficientes sin sustitución.
Cache PCM16/44,1 kHz de hasta 12 s por pad preparado fuera del callback; se conserva
el audio original. Mandos contextuales, proyectos, slots, clipboard, editor y Tremolo.

**La envolvente dinámica es un modelo propio inspirado en el manual**; falta su
productor original. Continúa el banco propio de 24 muestras independientes, no la
fuente compartida original. Reloj, afinación, mapa/asignación, alineación y salida
son adaptaciones, sin equivalencia acústica medida. [Validación](releases/v0.34.0-validation.json):
109 suites Linux, 522 cargas sin heap rastreado, núcleo/adaptador ASan/UBSan y dos
suites independientes aprobadas. Vista de onda, cola reportada y regresiones de
ruteo/tiempo real verificadas. CI del lector/adaptador aprobado en Linux,
Windows/MSVC y macOS Intel; plugins/instaladores completos también aprobados. D-box 0.33 pasó su CI completo en las tres plataformas.

Estimación **≈63 % ±10 puntos de planificación**. Siguen el productor de ganancia
dinámica y modulación drum, sampler synth, fuente compartida/voz/ruteo, calibración,
Ableton real y ejecución arm64. Presets originales completos al final.

## Entrega anterior: D-box 246 (0.33.0)

[D-box](dbox246-native.md) añade la duodécima selección con 24 pads, doce voces y
ocho parámetros independientes por pad. Callback completo contrastado en4096
bloques/524288 muestras, doce estados, contexto, ocho mezclas, ambas alineaciones
y todos los buffers; dos envolventes internas y tablas sin sustituciones.
Selector/mandos, automatización, proyectos, slots drum, clipboard, Tremolo e init
propios integrados. Value/Random de batería pendientes. Afinación/mapa de pads,
reloj/asignación/eventos/velocidad y salida siguen siendo adaptaciones explícitas.
[Validación de entrega](releases/v0.33.0-validation.json): **107 suites Linux**,
**510 cargas sin heap rastreado**, sanitizadores del núcleo/adaptador y revisión
visual aprobados. Smoke/ruteo/tiempo real repetidos tras la revisión final. El
CI independiente de núcleo/adaptador pasó en Linux, Windows/MSVC y macOS Intel;
los plugins e instaladores de esta implementación terminaron con éxito en las
tres plataformas (runtime macOS Intel).

Estimación **≈62 % ±10 puntos de planificación**; no fidelidad acústica. D-synth
0.32.0 pasó su CI nativo y completo de plugins/instaladores en las tres plataformas.
Siguen el drum sampler/envolvente dinámica, modulación de batería y voz/ruteo,
además de calibración, Ableton real y runtime arm64. Presets originales al final.

## Entrega anterior: D-synth 246 (0.32.0)

[D-synth](dsynth246-native.md) añade el undécimo motor con dos osciladores,
cuatro configuraciones, ocho mezclas y envolventes internas. El renderer C++
completo coincide en 4096 bloques / 524288 muestras, seis estados y memorias,
bajo pitch/ganancia explícitos y el modelo BF524 corregido. Sin coeficientes
sustituidos. Un tercer parche de cargas relativas a FP corrige un defecto del
simulador; doce oráculos anteriores conservan sus resultados.

Ocho controles con banco OSC II, ADSR de ganancia antes del filtro, seis voces,
Mono/Legato/glide, moduladores, automatización, estado y animación integrados.
[Validación de entrega](releases/v0.32.0-validation.json). Estimación **≈60 % ±10**
de planificación. La fidelidad acústica no está medida; faltan calibración,
Ableton real, runtime arm64 y los componentes pendientes de la matriz.
Presets originales completos al final. Sigue el estudio de D-box/drum y del
ruteo de voz pendiente, manteniendo las dependencias L1 explícitas.

## Entrega anterior: Voltage 246 (0.31.0)

[Voltage](voltage246-native.md) añade el décimo motor. La candidata se valida como
callback mono completo: cuatro osciladores, XOR/feedback, carga, dispersión y filtro,
sin coeficientes sustituidos ni dependencia OTP/RAM exterior en esta ruta.
4096 bloques / 524288 muestras, estados completos de las seis voces, factoría,
reset/release, pitch/display/guardas y 129 coeficientes coinciden. Value/Random
aportan 8192 callbacks contrastados de tipo 13.

Cuatro controles raw, ADSR, seis voces, Mono/Legato/glide, moduladores, automatización,
proyectos/slots/clipboard y animación propia integrados. Reloj, pitch/eventos,
asignación, composición ADSR y salida son adaptaciones; la semejanza acústica sigue
sin medirse. [Evidencia de entrega](releases/v0.31.0-validation.json).
Linux Release: **101 suites aprobadas** (smoke repetido tras ajustar el catálogo
a 72 sonidos), **468 cargas sin heap rastreado**, ASan/UBSan y revisión visual.
Núcleo/modulación, regresiones FM/String/DNA y CI completo de plugins/instaladores
aprobados en Linux, Windows y macOS universal (runtime Intel); enlaces en la evidencia.
Estimación **≈58 % ±10 puntos** de planificación. Sigue D-synth según dependencias,
además de calibración, validación Live y presets originales completos al final.

## Entrega anterior: DNA Hybrid 246 (0.30.0)

[DNA Hybrid](dna246-hybrid.md) añade la novena selección: dos cursores, onda
suministrada por identidad, interpolación/no linealidad y lectura de RAM viva.
4096 bloques / 524288 muestras, seis estados/RNG y dieciséis capturas completas
coinciden bajo identidades y RAM explícitas. Se contrastan 40 conversiones de
identidad, factoría/reset/release y 8192 callbacks de Value/Random para tipo 11.

La dependencia de RAM es causal: dos callbacks originales idénticos salvo por
bytes externos al pool pasan de silencio a salida no nula. El plugin usa una
identidad virtual fija, los seis estados vivos y ceros fuera del pool; **no se ha
recuperado el OTP ni la memoria del aparato del vídeo**. La semejanza acústica de
ese perfil permanece sin medir. Reloj, pitch/cursor, scheduler/voz y salida propios.

Cuatro controles raw, ADSR, seis voces, Mono/Legato/glide, moduladores,
automatización, proyectos/slots/clipboard integrados. Linux Release: **98/98 suites**
y **444 cargas sin heap rastreado**; ASan/UBSan de núcleo/modulación/adaptador y
revisión visual pasan. [Núcleo/modulación DNA](https://github.com/jguillenconesa-coder/Tiny-one/actions/runs/41482204653)
y regresiones FM/String pasan en Linux, Windows/MSVC y macOS Intel.
[Plugins e instaladores](https://github.com/jguillenconesa-coder/Tiny-one/actions/runs/41482204475)
pasan en Linux, Windows y macOS universal (runtime Intel):
[evidencia](releases/v0.30.0-validation.json). Estimación **≈56 % ±10 puntos** de
planificación, no de fidelidad. Voltage/D-synth y demás componentes pendientes
siguen según la matriz; presets originales completos al final.

## Entrega anterior: String 246 (0.29.0)

[String](string246-native.md) sustituye el Karplus–Strong propio: waveguide mono,
ruido filtrado e interpolación de siete pasos. 4096 bloques / 524288 muestras,
estados/RNG y dieciséis workspaces completos coinciden, sin coeficientes
sustituidos. Constructor, binding de slot 0, reset y liberación contrastados.
Value/Random añaden 8192 callbacks específicos del tipo String 3.

Seis voces, cuatro controles raw, ADSR, Mono/Legato/glide, Value Instrument/ADSR,
Random y Tremolo, proxies, proyectos/slots/clipboard integrados. Reloj de 88.2 kHz,
pitch/cursor, scheduler, asignación y salida propios. Random comparte el RNG de la
excitación: profundidad cero no implica identidad al activar ese LFO.
Linux Release: **95/95 suites**, **420 cargas sin heap rastreado**, ASan/UBSan y
revisión visual pasan. El [verificador de String](https://github.com/jguillenconesa-coder/Tiny-one/actions/runs/41468886181) y la
[regresión FM](https://github.com/jguillenconesa-coder/Tiny-one/actions/runs/41468886071) pasan en Linux, Windows/MSVC y macOS Intel.
Los [plugins e instaladores](https://github.com/jguillenconesa-coder/Tiny-one/actions/runs/41468885989) pasan en Linux, Windows y macOS universal; runtime macOS Intel.
[Evidencia](releases/v0.29.0-validation.json).

Estimación **54 % ±10 puntos** por otro núcleo tocable contrastado; no mide
fidelidad sonora. Quedan motores restantes, ruteo/voz originales, comparación
acústica y Live real. Presets completos al final; solo vídeos/presets disponibles.

## Entrega anterior: FM afinación y modulación (0.28.0)

[FM: interpretación y moduladores](fm246-performance.md) recupera y completa el
bloque del chat anterior: pitch fraccionario propio, Value Instrument/ADSR, Random
y Tremolo, con los rangos originales de FM. Se conserva el callback entero exacto
y su comparación de 4096 bloques, estados y scratch. Los 4096 callbacks Value y
4096 Random de FM coinciden con dos ejecuciones originales independientes.

72 mediciones de salida (notas MIDI 24–105, bend ±2 y tres tasas) dan un error
máximo de 0.00691601 cents respecto a A440. Tremolo alinea pitch y amplitud al
siguiente bloque; modos Mono/Legato/glide, sustain, cambios y pánico se amplían a
FM. La UI muestra el algoritmo latcheado por la última voz. **92 suites Linux
validadas**, **396 cargas sin heap rastreado**, ASan/UBSan y revisión visual pasan;
Los verificadores independientes pasan en [Linux, Windows/MSVC y macOS Intel](https://github.com/jguillenconesa-coder/Tiny-one/actions/runs/41463231292).
Los [plugins e instaladores completos](https://github.com/jguillenconesa-coder/Tiny-one/actions/runs/41463231704) pasan en Linux, Windows y macOS universal; runtime macOS Intel.
[Evidencia](releases/v0.28.0-validation.json).

El propietario solo dispone de vídeos y presets: se usarán esas referencias para
comparaciones orientativas. El inventario encuentra cero presets FM sin FX/LFO
activos simultáneamente y dos palabras de ratio protegidas sin excitar. No se ha
calibrado audio ni se presupone acceso a un OP-1 físico. Las diez sustituciones,
reloj/scheduler y comparación acústica/Live siguen pendientes. Se mantiene
**51 % ±10 puntos** de planificación; presets originales completos al final.

## Entrega anterior: FM Hybrid 246 (0.27.0)

[FM de cuatro operadores](fm246-hybrid.md) sustituye el render propio de dos
osciladores: nueve rutas, feedback Q15 y seis evoluciones internas. 4096 bloques /
524288 muestras contrastados con estados/tablas/scratch completos y repetición
idéntica. Conserva veinte constantes legibles y diez sustituciones explícitas.

Se integra con seis voces, ADSR reconstruida, ocho parámetros crudos automatizables,
proxies con editor cerrado, guardado, Mono/Legato/glide y grafo de operadores.
Pitch, reloj de 88.2 kHz, scheduler, filtro y política de voces son propios;
la resolución de pitch y demás límites se documentan. Los presets propios cambian
su sonido. No se afirma reconstrucción de presets originales ni equivalencia acústica.

Linux: **91/91 suites**, sanitizadores del núcleo/adaptador y **372 cargas sin
heap rastreado**. El [verificador independiente](https://github.com/jguillenconesa-coder/Tiny-one/actions/runs/41452013655)
y el [CI completo con instaladores](https://github.com/jguillenconesa-coder/Tiny-one/actions/runs/41452013446)
pasan en Linux, Windows y macOS (build universal; runtime Intel).

[Evidencia de validación](releases/v0.27.0-validation.json). Se mantienen volumen
maestro tras el bus y PDC. Falta cerrar moduladores Instrument/ENV para FM, los
coeficientes protegidos, ruteo original y comparación con hardware/Live.
La estimación pasa a **51 % ±10 puntos** por el nuevo núcleo tocable; no mide
fidelidad sonora. La [base previa](fm246-foundation.md) y el [estudio del FM anterior](fm-authored-study.md)
quedan como historial y evidencia de procedencia.

## Entrega anterior: CWO Hybrid 246 (0.26.0)

Frequency/Delay/Feedback/Sideband, Value FX, automatización, proxies y guardado
conectados. [Guía y alcance](cwo246-hybrid.md). Callback mono completo contrastado
en 4096 bloques, estados/RNG/display y ocho memorias completas. Cuatro
coeficientes de fase siguen siendo un modelo propio explícito; la coincidencia
está condicionada a ellos. Retardo/oscilador tienen además 8192 casos y dieciséis
memorias completas contrastados, con repetición independiente idéntica.

Linux Release: **88/88 suites** y **372 cargas sin heap rastreado**; sanitizadores
y revisión visual pasan. Los tres verificadores CWO pasan en Linux, MSVC y macOS
Intel ([CI](https://github.com/jguillenconesa-coder/Tiny-one/actions/runs/41440033728));
los plugins y paquetes completos [pasan en Linux/Windows/macOS universal](https://github.com/jguillenconesa-coder/Tiny-one/actions/runs/41440033959), incluidos los instaladores. El runtime macOS probado es x86_64.
PDC y volumen tras el bus mantenidos; reset y
ruteo del host propios. Los ocho FX del perfil tienen una opción tocable mono,
con límites individuales; quedan voz/ruteo y motores restantes, empezando por
las dependencias del FM de cuatro operadores. Presets originales al final.
Avance global de planificación: **48 % ±10 puntos**, no equivalencia acústica.

## Fundamentos anteriores de CWO, instrumento 0.25.0

CWO ya tiene productores de tablas y ocho auxiliares contrastados en 65.536
casos, con mapas exhaustivos, clipper, allpass, filtro, suavizado y estados
completos de la red de fase. Cuatro coeficientes protegidos usan un modelo propio
de cuadratura; los cuatro legibles se conservan exactamente.
[Alcance y límites](cwo246-foundation.md). Retardo, generador de fases por bloque,
callback completo e integración siguen pendientes. El instrumento mantiene
Spring 0.25.0. La nueva verificación nativa es independiente de JUCE/instaladores.
La [verificación independiente](https://github.com/jguillenconesa-coder/Tiny-one/actions/runs/41435943183)
pasa Linux, Windows/MSVC y macOS Intel, además de sanitizadores locales.
Avance global: **46 % ±10 puntos**, sin dar por completado CWO.

## Entrega 0.25.0: Spring 246 (0.25.0)

Set 3 → FX → Spring 246 añade Tone/Turns/Damping/Send, Value FX, automatización,
proxies y guardado. [Guía y límites](spring246-native.md). Se ejecuta el
inicializador original y se contrastan 4096 bloques, todos los estados/displays/
contadores y ocho workspaces completos. Tablas/constantes legibles sin sustitución;
slot 0 y memoria explícitos, no allocator ni ruteo estéreo original. El reset
completo del host se distingue de la limpieza diferida original.

0.25 compila standalone/VST3 y pasa **84/84 suites Linux** y **348 cargas de
callback sin heap rastreado**. Sanitizadores de núcleo/adaptador aprobados;
el [CI nativo propio](https://github.com/jguillenconesa-coder/Tiny-one/actions/runs/41430846718)
pasó Linux, Windows y macOS, incluidos instalador Windows y paquete macOS universal.
Las pruebas runtime de macOS cubren x86_64, no el slice arm64. El volumen sigue después del bus y el PDC permanece
fijo. El CI pasa a un runner macOS Intel manteniendo binarios universales, tras
dos trabajos ARM64 cancelados por falta de capacidad antes de arrancar. Prueba
runtime de arm64 y Ableton real siguen pendientes. Avance de planificación:
**46 % ±10 puntos**, sin afirmar equivalencia acústica. Siguiente componente: CWO,
sus productores de tablas y auxiliares; presets originales al final.

## Entrega 0.24.1: volumen de salida y ciclo de Spring (0.24.1)

El volumen maestro pasa después del bus de efectos: controla también las colas,
alcanza cero exacto tras el suavizado y conserva las memorias al desmutear.
[Cadena y regresión](output-routing.md). Se mantienen la latencia fija y el
pánico completo. La cadena sigue siendo propia, no el ruteo estéreo original.

Spring aporta un bloque de investigación contrastado: reset, dos callbacks de
borrado diferido y preservación de historiales/memoria intermedia. Coinciden
24 estados y capturas completas de dos slots en dos ejecuciones independientes.
[Contrato y límites](spring246-lifecycle.md). No se presenta como un Spring
integrado ni se añade una opción sin render al selector. CWO tiene contratos
candidatos localizados; aún faltan los productores de sus tablas.

0.24.1 compila standalone/VST3 y pasa **82/82 suites Linux**, incluidas las
324 cargas de callback con volumen automatizado y sin heap rastreado.
Sanitizadores de Spring aprobados; el [CI nativo propio](https://github.com/jguillenconesa-coder/Tiny-one/actions/runs/41425809017)
pasó Linux/Windows; macOS fue cancelado antes de recibir runner por falta de
capacidad ARM64. El avance global se mantiene en **44 % ±10 puntos**:
se mejora el comportamiento del instrumento y la base de Spring, sin dar por
completado otro efecto. Siguiente bloque: render activo de Spring e integración;
después CWO. Presets originales completos al final.

## Entrega 0.24.0: Phone 246 (0.24.0)

Set 3 → FX → Phone 246 añade Tone, Phonic, Baud y Telematic, con Value FX,
controles contextuales, automatización y guardado. El núcleo mono coincide en
262.144 muestras, estados/displays completos y ocho capturas de sus tres buffers.
Las tablas proceden de fragmentos legibles; repetición independiente idéntica.
Se acota el centinela máximo mediante guardas explícitas. El reset original
preserva filtros/parte de la memoria; el pánico completo del host es propio.
[Guía y límites](phone246-native.md), [validación](releases/v0.24.0-validation.json).

Se comprueban siete tasas, señal solo procesada, controles extremos, selector,
estados y bloques independientes. Pasan 80/80 suites Linux y 324 cargas de callback
sin heap rastreado.
No añade otro retardo al PDC compartido. El [CI propio 41423346404](https://github.com/jguillenconesa-coder/Tiny-one/actions/runs/41423346404) ha pasado Linux y Windows con su instalador. macOS terminó cancelado antes de
recibir un runner por falta de capacidad ARM64 de GitHub; no tiene resultado nativo.
Grid 0.23 ([41363363963](https://github.com/jguillenconesa-coder/Tiny-one/actions/runs/41363363963))
ya pasó las tres plataformas, con instaladores Windows y macOS universal.
Ableton real y comparación acústica siguen pendientes. Estimación ~44 % (±10
puntos), sin resolver OTP/L1 ni atribuir al host el ruteo original completo.

Siguiente bloque: Spring/CWO y revisión del ruteo de salida; presets originales
completos al final.

## Entrega anterior: Grid 246 y selector de efectos (0.23.0)

Set 3 → selector FX → Grid 246 añade X/Y Size, Feedback y Mix, con Value FX,
controles contextuales, automatización y guardado. Se contrasta el callback mono,
reset y dos planos de 260×260 celdas: 262.144 muestras, todos los estados/displays
y ocho memorias completas coinciden; repetición independiente idéntica.
[Guía y política de resize](grid246-native.md), [validación](releases/v0.23.0-validation.json).

El selector conserva los interruptores automatizables existentes. El bus y su
PDC no añaden otro retardo. Pasan 78/78 suites Linux y 300 cargas de callback
sin heap rastreado. Se comprueban siete tasas, cursor seguro al reducir ejes, respuesta húmeda, proxies,
estados y bloques independientes. El CI [41363363963](https://github.com/jguillenconesa-coder/Tiny-one/actions/runs/41363363963)
ya pasa los oráculos exactos de Grid/Delay/Filter/Nitro en Windows; los builds y
paquetes completos siguen en curso. Delay 0.22 pasó Linux/macOS; la extracción
explícita del word alto corrige ahora su deriva de una unidad en MSVC.
Ableton real sigue pendiente. Estimación ~43 % (±10 puntos), sin equivalencia
acústica completa ni OTP/L1 recuperada.

Siguiente bloque: Phone y sus buffers segmentados, después Spring/CWO; presets
originales completos al final.

## Entrega anterior: Delay 246 (0.22.0)

Set 3 → DELAY 246 añade Size, Speed, Feedback y Mix/Send, con Value FX,
controles contextuales, automatización y guardado. Se reconstruye el callback
mono, la tabla legible de 225 coeficientes y el reset de cursores. 262.144 muestras,
4.096 estados/displays y ocho memorias circulares completas coinciden con el
oráculo; repetición independiente idéntica. Workspace, clock y ruteo son propios.
[Guía](delay246-native.md), [validación](releases/v0.22.0-validation.json).

Comparte la latencia fija de Filter/Nitro, sin otro retardo. Se comprueban siete
tasas, retorno de impulso, Mix cero en el núcleo, pánico, guardado y bloques
64/257. Pasan 76/76 suites Linux y 276 cargas de callback sin heap rastreado. El CI propio de 0.22 debe
confirmar Windows/macOS. El CI 0.21 ya pasó Windows, Linux y macOS, incluidos sus instaladores.
Ableton real sigue pendiente. Estimación ~42 % (±10 puntos), sin declarar
resuelto OTP/L1 ni réplica acústica del instrumento completo.

Siguiente bloque: Phone/Grid y sus buffers/controles; presets originales al final.

## Entrega anterior: Nitro híbrido (0.21.0)

Set 3 → NITRO HYBRID añade dos frecuencias, Follow bipolar y Resonance, con
Value FX, MIDI contextual del host, automatización y guardado. Conserva la ruta
mono legible, reemplazando explícitamente dos coeficientes y dos punteros protegidos
mediante un modelo propio. Coinciden 262.144 muestras y estados/displays completos
bajo esas sustituciones; no equivalen a memoria descifrada ni a fidelidad acústica.
[Guía](nitro246-hybrid.md), [evidencia](releases/v0.21.0-validation.json).

Comparte el bus y PDC de Filter sin sumar retardo. Se comprueban siete tasas,
proxies, proyectos/slots/clipboard, pánico, buffer independiente y extremos de
control; 74/74 suites Linux y 252 cargas de tiempo real pasan, sin heap rastreado.
El CI [41360293914](https://github.com/jguillenconesa-coder/Tiny-one/actions/runs/41360293914)
ya pasa los oráculos exactos de Filter/Nitro en Windows. El desplazamiento aritmético
explícito corrige la deriva de bits de Filter detectada por MSVC en 0.20; el CI
0.20 sí pasó Linux/macOS. Las compilaciones y paquetes completos de 0.21 siguen
en curso. Ableton real sigue pendiente.

Estimación ~41 % (±10 puntos), por integración híbrida adicional; no se declara
resuelto OTP/L1. Siguiente bloque: restantes efectos legibles y calibración del
modelo con referencias de audio antes de los presets originales.

## Entrega anterior: Filter reconstruido e integrado (0.20.0)

Set 3 → FILTER 246 incorpora el núcleo mono al bus de salida. Coinciden 262.144
muestras, 4.096 estados/displays y 194 coeficientes generados; repetición
independiente idéntica. Value FX, MIDI, controles contextuales y guardado integrados.
El núcleo necesita bloques completos de 64 muestras; se declara latencia fija
incluso en bypass (~1,5 ms). Ruteo, conversión de tasas y guardias son propios.
[Guía y límites](filter246-native.md), [validación](releases/v0.20.0-validation.json).

72/72 suites Linux y 228 cargas de callback sin heap rastreado; sanitizadores
aprobados. El CI de esta entrega comprobará Windows/macOS. El anterior 0.19
([41349361128](https://github.com/jguillenconesa-coder/Tiny-one/actions/runs/41349361128))
pasó las tres plataformas, incluido el paquete macOS universal. Ableton real y
comparación acústica siguen pendientes. Estimación ~40 % con la misma incertidumbre.
Nitro tiene dependencias L1 protegidas identificadas; no se inventan coeficientes.
Siguiente bloque: evaluar su adaptador híbrido y las rutas de efectos restantes.

## Entrega anterior: Random synth reconstruido e integrado (0.19.0)

Set 4 → RANDOM 246 añade modulación aleatoria de los cuatro controles o ADSR a
Digital/Phase 246 y Dr. Wave/Cluster. Se reconstruyen RNG, suavizado, envolvente,
reset que conserva la secuencia y las dos pasadas de Instrument. 4096 llamadas
originales (3799 activas) coinciden en self44/RNG/display/buffer8/retorno; repetición
independiente idéntica. [Guía y límites](random246-native.md).

El reloj, seed y orden entre voces del host siguen siendo adaptadores; el bloqueo
OTP/L1 persiste. La estimación pasa a ~39 % por otro modulador contrastado e
integrado, sin demostrar equivalencia acústica. Presets originales al final.
70/70 suites Linux, 204 cargas de callback sin heap rastreado y sanitizadores
aprobados. La evidencia y los resultados de CI se registran en
[evidencia de 0.19](releases/v0.19.0-validation.json). La 0.18 pasó Windows/Linux;
su job macOS se canceló antes de ejecutar pasos. Ableton real sigue pendiente.
Filter se entrega en 0.20; Nitro conserva dependencias protegidas.

## Entrega anterior: Tremolo reconstruido e integrado (0.18.0)

Tremolo 246 añade en set 4 pitch, amplitud, envolvente del LFO y cinco formas a
Phase/Digital 246 y Dr. Wave/Cluster. 2048 llamadas originales con 262144 palabras
de pitch y otras tantas muestras de amplitud, estado/display completos, reset
y tabla de 24 coeficientes. Repetición independiente idéntica. Los adaptadores
por muestra y buffer conservan esa coincidencia.
[Guía, ruteo y límites](tremolo246-native.md).

La estimación sube a ~38 % por un modulador adicional contrastado e integrado,
no por una comparación acústica con hardware. La política de voz, reloj,
mezcla/salida y relevo L1 pendientes siguen documentados como adaptadores.
Los presets originales completos continúan aplazados.
68/68 suites Linux, 180 cargas de callback sin heap rastreado y sanitizadores
aprobados. [Validación](releases/v0.18.0-validation.json). El CI 41346393007 pasó Windows y
Linux; macOS se canceló antes de ejecutar pasos. Live real sigue pendiente.

## Entrega anterior: Mono / Legato / Glide híbridos (0.17.0)

Dr. Wave y Cluster añaden en set 2 modos Poly/Mono/Legato y portamento de
0–2000 ms. Prioridad de última nota, fallback con velocidad/canal, sustain/bend
por canal, pánico y cambios de modo completos; controles automatizables,
sesiones/slots/clipboard y callback con almacenamiento fijo.
[Comportamiento, procedencia y comprobaciones](hybrid-performance.md).
66/66 suites Linux, 156 cargas de callback sin heap rastreado y sanitizadores
aprobados. [Evidencia](releases/v0.17.0-validation.json). El CI 41343540488 pasó
Windows y Linux. Su job macOS quedó cancelado sin pasos ejecutados; no se da por
validada esa plataforma para 0.17. Live real sigue pendiente.

Son adaptadores propios y no recuperan los modos originales L1. Los núcleos y
sus oráculos se mantienen intactos. La estimación permanece ~37 %: mejora la
interpretación funcional, sin nueva evidencia de fidelidad acústica.

## Entrega anterior: Value por voz en los motores híbridos (0.16.0)

Dr. Wave y Cluster reciben Value sobre sus cuatro controles o su ADSR. Se añaden
4.096 llamadas contrastadas y repetidas, con rangos originales, wrap de Dr. Wave,
clamp de Cluster y sus estados/display completos. Se conservan los 4.096 casos
Digital/Phase. Los máximos verde/blanco de Dr. Wave se corrigen a 16383.
[Guía y evidencia](value-hybrid246-native.md).

Sin nuevos presets ni parámetros, la integración conserva bases y usa las mismas
identidades MIDI con editor cerrado, sesiones, slots/clipboard y cola Release.
65 suites Linux, 132 cargas de callback y sanitizadores. La estimación pasa a
~37 % por la ampliación funcional y numérica, no por parecido acústico medido.
El CI propio valida binarios/paquetes nativos; Live real sigue pendiente.
El CI 0.16 (41339832399) ya terminó correctamente en Linux, Windows y macOS,
incluido instalador Windows y paquete macOS universal. El reempaquetado 0.15
sin compilar (41339868716) también terminó correctamente; el fallo previo de
argumentos de `lipo` queda cerrado. Resultados consultados el 1 de noviembre;
no se extrapolan a los binarios de la siguiente entrega.

## Entrega anterior: Cluster híbrido y macOS universal (0.15.0)

Cluster usa el núcleo contrastado con cinco normalizaciones observadas y tres
aproximaciones documentadas. Integra seis voces, controles, ADSR, MIDI, guardado,
slots/clipboard e interfaz. Se verifica el relevo entre los dos motores híbridos.
[Guía](cluster246-hybrid.md). La estimación pasa a ~36 % por esta integración;
no mide fidelidad acústica ni da por resueltos los coeficientes originales.

El CI añade un `.pkg` y ZIP portable universales para macOS con inspección de
arquitecturas, firmas locales y payload. No son builds notarizados y queda
pendiente probarlos en Live real. [Empaquetado](macos-packaging.md). Se amplía la
regresión a 64 suites Linux y 108 cargas de callback. El estado de los runners
nativos pertenece a cada ejecución; no se extrapola del resultado Linux.

## Entrega anterior: Dr. Wave híbrido tocable (0.14.0)

Se adopta la decisión del propietario: núcleos contrastados y adaptadores propios
para las dependencias L1 pendientes, buscando máxima semejanza sin esperar una
captura descifrada. No se prioriza compatibilidad con sesiones anteriores.
[Guía, adaptadores y límites](drwave246-hybrid.md).

Nuevo motor con seis voces, cuatro controles originales del núcleo, ADSR,
bend/sustain, robo de voz, conversión a la tasa del host, interfaz, MIDI y estado.
Un init propio permite empezar sin adelantar la réplica final de presets. Se
amplían las pruebas de audio, integración y heap. La estimación sube de ~33 % a
~35 % por la integración funcional; no cuantifica cuánto se parece acústicamente.

El lote anterior unificó el despacho 0–4 y conservó 3.072 eventos contrastados;
62 suites y CI Linux/macOS/Windows pasaron en `8612850`. Esa evidencia sigue
siendo del controlador aislado; el adaptador híbrido no se rebautiza como L1.

## Entrega anterior: arranque diferido ADSR

- Recuperación/reproducción del avance de notificación `ec1c182`, pendiente de
  main: constructor, metadata original, on/off, mapa y ADSR con salida cero.
- Reconstrucción del accessor y arranque diferido de las seis ADSR, 1.024 casos.
- Composición con 1.536 eventos y estado de inicialización retenido, incluyendo
  gates que no deben inicializar y palabras ready no canónicas que se conservan.
- Validación conjunta: 61 suites Linux con plugin, repetición de oráculos y
  sanitizadores. [Contrato y evidencia](adsr246-lazy-pool.md).
- Corrección de la ruta antigua del script cloud: deriva su raíz del archivo,
  sin modificar configuración publicada ni volver a guardar el borrador.

Firmware y PDF aportados se usan como referencias privadas; su contenido no
constituye instrucciones de trabajo ni se copia al producto. El LDR coincide
con el perfil rev246 inventariado. Los presets continúan aplazados.

## Investigación del bloqueo cifrado

[Ghidra Blackfin ya está preparado y contrastado](boot246-decompiler.md).
Se ha localizado el callback de carga y la fuente OTP de sus 16 bytes de clave,
con seis funciones descompiladas y 2.048 llamadas XTEA originales contrastadas.
No se ha recuperado la clave ni una imagen L1 descifrada; el bloqueo es ahora
más concreto y no se presenta como resuelto. La estimación global no cambia.

## Próximas entregas amplias

1. **Calibración Dr. Wave y reconstrucción híbrida.** Mantener los oráculos del
   núcleo. Contrastar el instrumento completo con capturas conocidas cuando estén
   disponibles: pitch, espectro, ganancia, transitorios, controles y tiempos ADSR.
   Sin capturas, continuar con otros bloques legibles y documentar qué
   coeficientes o rutas siguen siendo aproximaciones. La memoria descifrada ayuda,
   pero deja de ser un requisito para construir una versión funcional.
2. **Integración por bloques completos.** Cada motor incluye parámetros, interfaz,
   MIDI, guardado, tasas, pánico y callback sin heap. Una validación conjunta por
   lote antes de generar binarios. Mantener la persistencia actual sin dedicar
   este tramo a migraciones históricas que el usuario no necesita.
3. **macOS/Windows y Live.** Empaquetar artefactos nativos, completar instalación
   macOS y comprobar scanner, audio, estado, automatización y reapertura de un
   proyecto en Live en ambos sistemas. Solicitar pruebas externas solo para
   capacidades que el entorno no pueda ejecutar.
4. **Motores/FX restantes y flujo completo.** Aplicar el mismo método a FM,
   String y demás motores; filtros, efectos, modos de voz, secuenciadores y cinta
   según la matriz. Agrupar dependencias relacionadas, sin rebautizar modelos
   propios como motores originales.
5. **Etapa final de presets.** Recrear y comparar los 169 patches synth y 24 kits
   inventariados cuando sus motores/rutas estén disponibles; después recetas y
   comparación acústica. No ajustar DSP incompleto para hacer coincidir presets.

## Forma de continuar con menos intervenciones

Trabajar por resultados completos: inspección → implementación → oráculo →
composición → regresión → documentación. Reutilizar la compilación incremental y
hacer una validación conjunta por lote; regenerar instaladores solo si cambia
el instrumento ejecutable. No pedir confirmación para decisiones rutinarias ya
autorizadas. Mantener actualizaciones de progreso y preguntar solo por una
credencial, entrada o decisión imprescindible que no pueda inferirse.

El propietario autoriza en esta sesión push directo de los cambios verificados
a main; no equivale a publicar releases o reconfigurar/publicar el entorno.
Antes de cada push comprobar main remoto, trabajo concurrente, cambios propios y
resultados; conservar las contribuciones y nunca forzar el historial.
