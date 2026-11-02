# Objetivo: OP-1 original completo

La meta es cubrir motores, filtros, efectos, envolventes, modulación, modos de
ejecución y sonidos documentados. La versión 0.30.0 **todavía no es una réplica
completa**. Esta matriz distingue un núcleo contrastado de una aproximación
propia o una función pendiente; cambiar el nombre de un modelo no completa un port.

**Prioridad activa: reconstrucción híbrida del instrumento.** Presets, bancos y vídeo
quedan aplazados. El [plan de trabajo por componentes](firmware-dsp-workplan.md)
registra las integraciones tocables de Dr. Wave y Cluster con adaptadores propios.
Ambos conservan sus bloqueos originales de L1; los presets completos quedan al final.

Fuentes: manual v2.2 suministrado por el usuario, 56 páginas, y firmware rev246
inventariado. Las páginas 53–54 del PDF muestran motores y efectos; 47–50, LFO;
51–52, los ejercicios de sonidos. Se revisaron las páginas como imágenes cuando
la extracción de texto omitía los diagramas. No se distribuyen el PDF, firmware,
DB ni instrucciones originales. REA verifica el inventario SQLite; GNU BF524
ejecuta rutinas aisladas para los oráculos de DSP.

[Estado de proyecto y estimación ponderada](project-status.md): aproximadamente
56 % del objetivo completo, con incertidumbre explícita; no es una medición
de equivalencia sonora.

## Motores

Los IDs son observaciones/referencias del perfil rev246, no el orden del selector
del plugin. El plugin añade Dr. Wave, Cluster y DNA Hybrid como séptima, octava y novena opciones; no se prioriza compatibilidad histórica.

| ID | Motor objetivo | Estado actual |
| --- | --- | --- |
| 0 | FM de cuatro operadores | **[FM Hybrid tocable](fm246-hybrid.md)**: cuatro operadores, nueve rutas, seis voces y ADSR reconstruida. Callback completo contrastado en 4096 bloques / 524288 muestras con estado/scratch; veinte constantes legibles y diez sustituidas explícitamente. [0.28](fm246-performance.md): pitch fraccionario propio, Value Instrument/ENV, Random y Tremolo integrados; rangos y 8192 callbacks de modulación contrastados. Pitch/reloj/allocator/salida propios; calibración acústica y presets originales pendientes. |
| 1 | Digital | Núcleo reconstruido, 262.144 muestras / 2.048 estados; integración, mezcla y voz propias. 16 presets CORE. |
| 2 | Dr. Wave | **Motor híbrido tocable de seis voces**, con núcleo contrastado, controles y ADSR; pitch/reloj/allocator/salida propios ([guía](drwave246-hybrid.md)). Render aislado completo: 983.680 muestras en quince oráculos, con actualización, controles y cinco osciladores; [eventos Core de seis voces](drwave246-core-routing.md), [activación VoiceCtl con afinación explícita](drwave246-note-activation.md), [actualización selectiva/broadcast de pitch](drwave246-pitch-update.md), [índice de afinación](drwave246-tuning-index.md), [tabla de afinación generada](drwave246-tuning-table.md), [refresh con ADSR/RNG](drwave246-pitch-refresh.md), [modificación del buffer por cuatro entradas](drwave246-pitch-buffer-modifier.md), [volumen/pitch en serie y suavizado compartido](drwave246-volume-buffer-modifier.md), [decodificación hacia sus cuatro fuentes](drwave246-control-source-input.md), [destino instrumento y wrap/clamp de controles](drwave246-instrument-control-routing.md), [256 combinaciones de destinos en una llamada](drwave246-mixed-control-routing.md), [dispatch con ENV/kind 0–1 y ambos renderers](drwave246-control-env-dispatch.md), [inicializador de la imagen común y bloqueo del llamador](drwave246-control-workspace-init.md), [retirada polifónica VoiceCtl con release ADSR](drwave246-voicectl-poly-release.md), [selectores 1–4 con recuperación de pitch/RNG](drwave246-release-selectors.md), [constructor de argumentos](voice246-note-argument.md), [composición constructor/eventos/ADSR con orden explícito](drwave246-note-event-chain.md), [setter de ajustes](voice246-note-adjustment.md), [suministro por setters antes de eventos/ADSR](drwave246-adjusted-note-chain.md), [llamador polifónico con primera voz libre e inserción original](drwave246-poly-first-free.md), [voces ocupadas y productor de reinicio ADSR](drwave246-poly-steal.md), [consulta de última voz válida](voice246-last-voice-query.md), [dispatcher de retirada con mapa vacío](voice246-release-dispatch.md), [selectores 1–3 con lista vacía, actividad explícita y reinicio pendiente](drwave246-note-modes.md), [selectores con notas retenidas](drwave246-held-note-modes.md), [secuencias on/off con lista y estado conservados](drwave246-note-lifecycle.md), [pitch explícito con lista vacía en selectores 1–3](drwave246-empty-explicit-pitch.md), [despacho exterior de activación con metadata cero](voice246-note-dispatch.md), [pitch explícito polifónico](drwave246-poly-explicit-pitch.md), [eventos con metadata original y salida cero](voice246-quiet-note-lifecycle.md) y consumidor de contador de 16 bits contrastado exhaustivamente ([detalle](drwave246-counter.md)); seis mezcladores / 196.608 palabras contrastados aparte. Se conservan los oráculos de lector, 320 tablas y tres transformaciones. Llenado inicial del buffer de pitch explícito del harness; cadencia externa, asignación, demás rutas de pitch/ADSR, salida originales pendientes; integración híbrida disponible; [núcleo completo](drwave246-kernel.md). |
| 3 | String | **[String 246 tocable](string246-native.md)**: waveguide mono, excitación/ruido filtrados, dos lecturas de siete pasos, constructor/reset/release y tablas legibles sin sustitución. 4096 bloques / 524288 muestras, estados/RNG y 16 memorias completas contrastados. Cuatro controles raw, ADSR, seis voces, Mono/Legato/glide y Value/Random/Tremolo integrados; reloj, pitch/cursor, scheduler, asignación/salida propios. Calibración acústica/Live y presets originales pendientes. |
| 4 | Cluster | **Motor híbrido tocable de seis voces**, ocho osciladores por voz y tres normalizaciones aproximadas explícitamente; [guía](cluster246-hybrid.md). Núcleo aislado: 262.144 muestras con coeficientes de prueba y 81.920 con cinco normalizaciones legibles convertidas por el helper original. 24 coeficientes de amortiguación y 60 casos de factoría/reset de seis slots contrastados. Faltan tres normalizaciones originales y la ruta completa del hardware; [avance](cluster246-normalization.md). |
| 5 | Pulse | Render localizado en L1 cifrada (`0xffa04b88`), sin imagen descifrada validada. Port pendiente; [bloqueo](firmware/pulse246-l1-blocker.json). |
| 6 | Synth sampler | Reproductor propio con CLEAN, raíz/loop/trim; banco de audio importable. No es el render original completo con sus cuatro controles secundarios. |
| 7 | Phase | Núcleo Pd y ADSR contrastados; NCO, salida, mezcla y asignación propios. 16 presets CORE. |
| 8 | Drum sampler | Kit propio de 24 pads con edición y bancos; falta el render/envolvente dinámica original. |
| 9 | D-box | Drum synth propio disponible; pendiente el original con cross-mod y controles secundarios. |
| 10 | LS, identificador interno | Referencia candidata; sin equivalencia comercial confirmada, port ni presets de fábrica encontrados en el DB. |
| 11 | DNA | **[DNA Hybrid tocable](dna246-hybrid.md)**: callback contrastado en 4096 bloques / 524288 muestras con identidades/RAM explícitas, seis estados vivos, RNG y 16 capturas completas. Cuatro controles y ADSR, seis voces, modos y moduladores integrados. Identidad virtual propia y RAM externa al pool modelada como cero; no OTP/RAM físicos recuperados ni calibración acústica. Scheduler/pitch/salida propios; 16 presets originales pendientes. No aparece en el diagrama de referencia del manual suministrado. |
| 12 | D-synth | [Base contrastada](dsynth246-foundation.md): ciclo de seis estados, memoria de captura y 8192 callbacks Value/Random con ocho destinos directos. Oráculo del render original: 4096 bloques/524288 muestras observadas, sin renderer C++ equivalente todavía. Pendientes audio completo, envolventes específicas, integración y calibración. |
| 13 | Voltage | [Núcleo mono y ciclo contrastados](voltage246-native.md): 4096 bloques/524288 muestras, seis estados y 129 coeficientes sin sustitución. Value/Random: 8192 callbacks tipo 13. Integración seis voces/ADSR/modulación/estado; reloj, pitch, asignación y salida propios. Cadencia/ruteo original y equivalencia acústica pendientes; 16 presets inventariados, réplica final pendiente. |
| 14 | Iter, identificador interno | Referencia candidata; sin presets en este DB, pendiente. |

Las candidatas proceden del análisis estático y de la referencia pública op1-sdk
citada en créditos. Su localización no es prueba de equivalencia sonora. Los
núcleos Digital, Pd y ADSR tienen evidencia numérica específica; las referencias
a otras funciones no extienden esa evidencia al resto del aparato.

Cluster dispone además de la [inicialización compartida contrastada](cluster246-shared-init.md)
en 256 casos con fuentes explícitas y de 131.072 resultados de los helpers Q15/Q31.
La tabla original sigue incompleta; las entradas de prueba no sustituyen los tres
valores cifrados. REA apoya consultas de lectura/procedencia; la verificación de
instrucciones Blackfin de esta etapa usa GNU y Binutils.

La [ruta Core de disparo/liberación](cluster246-core-routing.md) añade 131.072
muestras y 1.024 pools completos con seis slots intercalados y RNG compartido.
Note-on exige coincidencia del sonido cargado/seleccionado; release no la exige.
El orden de eventos, pitch y renders es explícito del harness, sin completar
asignación, cadencia, ruteo ADSR ni mezcla de salida originales.

Como dependencia de Cluster, se contrastan las [transiciones del ADSR común](cluster246-envelope-transitions.md):
inicialización y solicitud de reinicio, rampa de etapa 6 y nuevo ataque,
con 327.680 muestras y 2.560 pares de estados. Las palabras pendientes del objeto
no completan los cuatro controles auxiliares de preset ni el ruteo con el motor.

La [preparación de nota de VoiceCtl para Cluster](cluster246-note-activation.md)
añade una tabla contrastada de 128 palabras y la escritura de pitch, velocidad y
actividad, con afinación explícita y 131.072 muestras con incremento mantenido.
La calibración original, pitch por muestra y consumidor ADSR siguen pendientes;
las referencias candidatas del relevo están en L1 parcialmente cifrada.

## Efectos y filtros

| Objetivo | Estado |
| --- | --- |
| Delay | Callback mono, siete taps/interpolación, 225 coeficientes y reset contrastados: 262.144 muestras, estado/display y ocho memorias completas. Integrado en bus con Value FX; workspace/tasa/ruteo propios. [Guía](delay246-native.md). |
| Grid | Núcleo mono, dos planos y reset contrastados: 262.144 muestras, estado/display y ocho memorias completas. Integrado con Value FX y selector; transición de resize/ruteo propios. [Guía](grid246-native.md). |
| Punch | Núcleo mono contrastado con 131.072 muestras, 2.048 estados y display. Integración opcional con conversión de tasa/DC propios. Cola potencialmente sostenida. |
| Phone | Núcleo mono, segmentación, filtros/remuestreo y reset contrastados en 262.144 muestras y ocho memorias completas. Integrado con Value FX; slot/workspace, guardas, pánico y ruteo propios. [Guía](phone246-native.md). |
| Spring | [Mono e integración](spring246-native.md) contrastados: inicializador, tablas/constantes, 4096 bloques, estados y workspaces completos. Allocator, estéreo y equivalencia acústica pendientes. |
| Nitro | Híbrido integrado: ruta mono legible contrastada en 262.144 muestras bajo coeficientes/punteros sustituidos. Los originales siguen protegidos; no hay equivalencia acústica demostrada. [Guía y límites](nitro246-hybrid.md). |
| CWO | [CWO Hybrid integrado](cwo246-hybrid.md): callback mono completo, 4096 bloques y ocho buffers contrastados bajo cuatro coeficientes propios. Retardo/oscilador con evidencia separada. Allocator, estéreo y equivalencia acústica pendientes. |
| Filter, ruta interna | Núcleo mono contrastado: 262.144 muestras, estado/display y 194 coeficientes. Integrado en bus de salida con PDC fijo; tasa/ruteo propios. [Guía](filter246-native.md). |
| Filtros internos de cada motor y decimación/salida | Pendientes individualmente. Los filtros de salida/adaptación actuales son propios. |
| EQ y master | Implementación propia; EQ, dinámica y ruteo originales pendientes. |

## Envolventes, modulación y ejecución

| Objetivo | Estado |
| --- | --- |
| ADSR común | Núcleo, tablas y callbacks contrastados; 2.621.440 muestras / 20.480 estados. |
| Arranque del pool ADSR | [Accessor e inicialización diferida](adsr246-lazy-pool.md): seis estados, ready de palabra completa, 1.024 accesos y 1.536 eventos compuestos; no completa allocator ni scheduler. |
| Cuatro campos ADSR adicionales y ruteo | [Entrada de cuatro fuentes a ADSR 0–3](drwave246-envelope-control-input.md), seguida del render: 131.072 muestras / 1.024 estados adicionales. [Selector auxiliar 4 en contexto Dr. Wave](drwave246-envelope-mode-control.md): otras 131.072 muestras, seis estados y scratch dither compartido. Demás campos auxiliares, relevo por voz y salida siguen pendientes. |
| Envolventes D-synth / D-box / drum dinámica | Pendientes. No quedan completadas por portar el ADSR común. |
| Value LFO y destino parámetro/color | FX: 4.096 llamadas contrastadas. Voice: 8.192 llamadas instrument/primeros cuatro ADSR para Digital/Phase/Dr. Wave/Cluster; estado independiente por voz y wrap/clamp contrastados. [Integración híbrida](value-hybrid246-native.md). Otras familias, drum y auxiliares pendientes; reloj/allocator propios. |
| Tremolo | [Callback completo contrastado e integrado en Phase/Digital/Dr. Wave/Cluster](tremolo246-native.md): 2048 llamadas, 262144 muestras de amplitud y palabras de pitch, 28 bytes de estado / 32 de display, cinco formas, reset y 24 coeficientes. Clock, display por voz y ruteo propios; resto de motores y comparación con hardware pendientes. |
| Random | [Callback synth reconstruido e integrado](random246-native.md) en Digital/Phase/Dr. Wave/Cluster: 4096 llamadas, self44/RNG/display/destinos completos, Instrument en dos pasadas y ADSR. Reloj, seed y scheduling propios; FX/drum y comparación con hardware pendientes. |
| Element / MIDI / Bend / Crank | Pendientes algoritmos y ruteo originales. Las fuentes físicas necesitarán una integración definida para un plugin. Recibir notas/CC normales no implementa estos LFO. |
| Retrigger / free | Callbacks Value FX y synth/ENV contrastados; integración por voz conserva fase al reutilizar, pausa voces inactivas. Resto de LFO/rutas y allocator originales pendientes. |
| Poly / mono / legato / unison / portamento | [Mono/Legato y glide propios para Dr. Wave/Cluster](hybrid-performance.md), con prioridad de última nota y sustain/bend por canal. No implementan los modos originales ni unison. Polifonía propia: seis voces Dr. Wave/Cluster, ocho en los demás motores. [Cadena original acotada de selectores 0–4](voice246-instrument-lifecycle.md): 3.072 eventos con voces libres/ocupadas, pitch explícito/lookup y arranque frío; consumidor L1, reloj e integración de esos modos pendientes. |
| Octavas del teclado | Interfaz ampliada a −4…+4, guardada en sesión/slot/clipboard, limitada a notas MIDI 0–127. No altera MIDI externo ni implementa aún octave de todos los patches originales. |
| Secuenciadores | Secuenciación/arpegios propios disponibles; Endless, Pattern, Finger, Tombola, Sketch y Crank originales requieren revisión individual. Tombola bloquea completar Singing Birds. |
| Cinta y mezcla | Cuatro pistas y edición propias; no se afirma equivalencia completa de longitud, varispeed, tape tricks, pan y dinámica originales. |

## Presets y sonidos del manual

El DB rev246 contiene **169 patches de synth**: diez categorías con 16 entradas
cada una y 9 de sampler. También contiene **24 kits**. El inventario guarda
nombres y campos numéricos necesarios, no audio ni el DB original:
[factory-preset-inventory.json](../research/blackfin/factory-preset-inventory.json).
La [comprobación REA](firmware/rea-factory-presets-summary.json) devolvió las 169
filas synth sin truncar y con el hash de la fuente inventariada.

Las recetas del manual son objetivos adicionales; no son una lista de 169 nombres
con parámetros raw. Sus posiciones descriptivas necesitan calibración. Se conserva
esa distinción en [manual-sound-targets.json](../research/blackfin/manual-sound-targets.json).

| Receta | Dependencias completadas | Pendiente para darla por recreada |
| --- | --- | --- |
| Helicopter, §16.3, PDF 51–52 | Digital, ADSR común, Punch y Value → azul; teclado +4. Preset de estudio adicional. | Calibración display/raw, cadencia/alineación original, ruteo/salida y comparación acústica. |
| Singing birds, §16.4, PDF 52 | Infraestructura MIDI/ADSR | FM Hybrid, Spring y Tremolo a FM disponibles con límites documentados; scheduler/ruteo originales, Tombola y calibración pendientes. |

Actualmente se ofrecen 25 sonidos Factory propios, 16 Phase CORE, 16 Digital CORE
y 3 variantes **246 CORE + Punch**, más 3 **246 Value FX Studies** y 5 **246 Value Voice Studies**. Los bancos importados son adicionales. Las
entradas anteriores conservan su comportamiento anterior; Punch no se
activa al abrir un proyecto previo; Value y Value Voice también se cargan apagados cuando faltan en el estado. Los registros pendientes no aparecen como
presets originales supuestamente reproducidos en el navegador.

## Orden de trabajo verificable

1. Calibrar Dr. Wave y Cluster híbridos frente a capturas con ajustes conocidos,
   cuando estén disponibles. Mantener explícitos reloj, normalizaciones, afinación
   y relevo L1 pendientes; la decisión vigente permite adaptadores propios para
   integrar sin esperar a descifrar esas dependencias.
2. Continuar los motores y sus filtros/rutas internas por dependencias, con tablas,
   muestras y estado completo contrastados antes de cada integración.
3. Completar envolventes auxiliares/específicas, moduladores y destinos pendientes,
   además de preparación/asignación de voz, modos de ejecución y salida originales.
4. Validar cada integración en el motor de audio: tasas, cambios de controles,
   note-off/pánico, sesiones/MIDI y trabajo del callback, junto con Windows/ASIO.
5. Presets, recetas de sonidos y comparación con vídeo en una etapa posterior.

El [plan DSP](firmware-dsp-workplan.md) define los criterios de cierre y registra
los bloqueos sin sustituir datos desconocidos por valores supuestamente originales.

Una coincidencia de oráculo bajo el modelo GNU documentado no certifica la máquina
completa, el reloj físico, su latencia ni ausencia de dropouts. El proyecto conserva
ambos tipos de evidencia por separado y mantiene sonidos/versiones anteriores.

La [demo de presets aportada](preset-listening-reference.md) ya dispone de un
[índice de 169 selecciones revisadas en 11 categorías](preset-video-index.md),
obtenido del lote definitivo de 21 segmentos. Se ha decodificado su audio y
alineado el vídeo mediante paquetes compartidos. Sigue pendiente la comparación
acústica con notas/FX/LFO documentados; identificar un nombre no completa un port.
Los lotes descartados a petición del usuario permanecen excluidos.

El [piloto espectral Sugar/Pure](preset-audio-pilot.md) mide dos ventanas bajo
hipótesis de nota explícitas y rechaza otra por polifonía. Se conservan diferencias
de hasta decenas de dB en determinados armónicos relativos; la grabación y las
rutas incompletas impiden atribuirlas a un algoritmo concreto. Se añade una
[tabla de dependencias de los 32 presets CORE](../research/blackfin/preset-audio-readiness.json).
No se han retocado motores, filtros ni envolventes a partir de este piloto.
