# Plan activo: reconstrucción híbrida del instrumento

**Prioridad acordada: motores, filtros, envolventes y moduladores originales.**
Los presets, bancos, recetas del manual y la comparación con vídeos pasan a una
etapa posterior. Se conserva la investigación previa como referencia, sin usarla
para dirigir ajustes de sonido durante esta etapa.

## Coordinación principal y entregas agrupadas

Desde el 1 de noviembre de 2026 esta cuenta coordina el proyecto. El propietario
autoriza en esta sesión pushes directos verificados a main, sin releases ni
cambios automáticos de configuración del entorno. Se agrupan dependencias,
composición y regresión en entregas amplias; no se pide intervención por cada
oráculo. [Estado, estimación y próximos lotes](project-status.md). Las reservas
históricas de otra cuenta deben contrastarse con trabajo remoto actual.

## Decisión vigente: método híbrido

El propietario autoriza una versión funcional lo más parecida posible al
original sin esperar L1 descifrada. Usar núcleos contrastados donde existen y
adaptadores propios explícitos en lo que falta. No se priorizan migraciones de
sesiones históricas. La memoria protegida conserva sus bloqueos documentados,
pero no bloquea la integración tocable. [Dr. Wave integrado](drwave246-hybrid.md).
Esta decisión sustituye el requisito anterior de verificar todas las dependencias
antes de integrar un motor; no reduce la exigencia de describir su procedencia.

Desde 0.17, Dr. Wave/Cluster disponen de [Mono/Legato y glide propios](hybrid-performance.md),
con integración de controles, estado y MIDI. El scheduler y los selectores
originales siguen pendientes; no se amplía por ello la evidencia del firmware.

Desde 0.18 se integra [Tremolo reconstruido](tremolo246-native.md) en los cuatro
motores nativos. Callback completo, reset, cinco formas y tabla contrastados;
clock, buffers iniciales y ruteo del instrumento mantienen sus límites explícitos.

## Un componente activo y criterios de cierre

Se trabaja un bloque DSP cada vez. Un bloqueo se documenta con su dependencia
concreta; no se rellena con constantes o algoritmos supuestamente originales.

1. **Contrato:** localizar entradas, controles, memoria, tablas, salidas y callbacks
   en el firmware. Separar datos observados de hipótesis.
2. **Datos:** reproducir las tablas/coeficientes usados por ese bloque, incluyendo
   inicializadores y conversiones. Un buffer rellenado por el harness es una
   entrada de prueba, no un dato recuperado.
3. **Oráculo:** ejecutar las instrucciones originales aisladas con límites,
   barridos, estados independientes y secuencias continuas. Comparar muestras y
   estado completo, incluidos RNG y efectos laterales compartidos.
4. **Reconstrucción:** implementar el bloque C++ independiente y comprobar su
   coincidencia. Repetir el oráculo en otro directorio privado y ejecutar pruebas
   Linux/Windows y sanitizadores aplicables.
5. **Integración:** introducir el bloque en el motor de audio con sus
   dependencias verificadas o adaptadores propios documentados. Comprobar asignación de voces, orden/cadencia,
   cambios de controles, tasas, note-off/pánico y ausencia de asignaciones en el
   callback. Probar controles MIDI y persistencia de sesiones actuales.
6. **Distribución:** generar instaladores cuando cambie el instrumento ejecutable.
   Una prueba de núcleo aislado no se presenta como un motor completo instalado.

## Integraciones híbridas entregadas y siguiente bloque

Dr. Wave y [Cluster](cluster246-hybrid.md) ya disponen de adaptadores tocables.
En 0.16 se integra [Value por voz y ADSR](value-hybrid246-native.md) en ambos,
con otros 4.096 casos y rangos contrastados. El siguiente bloque debe priorizar
un motor/FX legible o la calibración acústica
con capturas conocidas; no repetir el bloqueo L1 sin evidencia nueva. El CI añade
[paquetes macOS universales](macos-packaging.md).

## Evidencia acumulada: Dr. Wave rev246

Se avanza con Dr. Wave por el bloqueo concreto de las dependencias restantes de
Cluster. Su ruta de lectura es SDRAM legible y permite contrastar otro núcleo sin
inventar coeficientes ni ejecutar instrucciones cifradas.

| Parte | Estado |
| --- | --- |
| Lectura, mezcla de cinco osciladores y desviación de pitch | 241.920 muestras contrastadas con ondas sintéticas explícitas. Fases de 24 bits y refinamiento entero del recíproco conservados. |
| Objetos y contexto compartido | 2.048 pools de seis slots; 293 construcciones, 121 resets y 187 releases; cinco incrementos, semillas y puntero compartidos contrastados. |
| Generación de tablas base | Seis bancos / 320 ondas generados desde el seno contrastado; 18 secuencias de arranque y 1.474.560 muestras de tabla coinciden con las rutinas originales. Valores predeterminados y preservación de memoria compartida contrastados. |
| Transformación de ondas | Tres callbacks completos contrastados: retención por grupos, extremo fijo y repetición; 131.072 palabras / 512 llamadas, seis ondas y cachés compartidas. La palabra Q16 y la elección de callback son entradas explícitas. |
| Mezcla espectral | Seis callbacks: 768 llamadas / 196.608 palabras, bancos generados y preservados, reemplazo/adición y seis slots contrastados. |
| Render completo, actualización y controles | 61.568 muestras / 512 registros: 240 actualizaciones, 241 lecturas activas y 31 retornos inactivos. Preparación de cuatro controles, dos familias, inversión de 25 bits, blanco opcional, retención común y cinco osciladores contrastados juntos. |
| Consumo del contador externo | Los 65.536 valores raw contrastados dos veces con vecinos distintos; el resto es sin signo. 61.568 muestras adicionales de render completo cruzan los límites 32767/32768 y 65535/0. El adaptador admite toda la palabra; no establece el reloj original. |
| Eventos Core y seis voces | Factoría tipo interno 2, note-on condicionado al sonido seleccionado y release sin esa condición; 61.568 muestras adicionales junto al render completo. 194 disparos aceptados, 86 rechazados, 122 releases y pools previos al render contrastados. Release de motor no equivale a release ADSR. |
| Activación VoiceCtl | Tabla de 128 notas, pitch, velocidad y actividad contrastados con afinación explícita. 427 activaciones aceptadas / 85 bloqueadas y 61.568 muestras posteriores con pitch mantenido por el harness; no es el proveedor de pitch original. |
| Actualización de pitch VoiceCtl `0x01a85cbe` | 512 llamadas con listas de 0–128 entradas: destino selectivo, inactivo y broadcast contrastados, más 61.568 muestras posteriores. Afinación y copia del registro al buffer siguen explícitas del harness; otras rutas de pitch y el relevo original al render permanecen pendientes. |
| Entrada de pitch bend VoiceCtl `0x01a84efa` | 65.536 parejas de bytes y 256 llamadas continuas contrastadas, incluida la preservación de seis registros de pitch. Control signed16 y tres flags; consumidor candidato en L1 parcialmente cifrada. No establece bend → frecuencia ni añade audio/integración. |
| Índice de afinación VoiceCtl `0x01a842f0` | Setter contrastado con estrés signed32, 4.577 parejas de controles y 256 llamadas continuas. Otras 61.568 muestras verifican activación/render con coeficientes explícitos en índices calculados; no recupera los valores originales de la tabla ni el escritor del buffer. |
| Productor de la tabla común de afinación | Cola SDRAM `0x01a85454` contrastada: 2400 coeficientes, tres precargas y otras 61.568 muestras de activación/render sin overrides de coeficiente. Se omite el setup previo; escritor del buffer y cadencia siguen pendientes. |
| Refresh de pitch VoiceCtl `0x01a854cc` | Rutina completa contrastada con tabla generada, seis ADSR, lista/una/seis voces, gates Core y RNG; 61.568 muestras posteriores. Módulos auxiliares Core explícitamente ausentes; render ADSR y escritor del buffer siguen pendientes. |
| Modificador de pitch de cuatro entradas `0x019cce68` | Ramas pitch/desactivadas contrastadas: 65.536 palabras y 61.568 muestras posteriores, 16 máscaras de canales y saturación serial. Consume un buffer ya lleno; fuentes, orden y contador explícitos. No completa su productor, otros destinos ni la identidad del módulo. |
| Entrada de cuatro canales a parámetros ADSR | Callback `0x019cce68`, destino ENV 0–3, seguido del render ADSR: 1.024 llamadas / 131.072 muestras, estado y RNG contrastados. Parámetros iniciales/cadencia explícitos; auxiliar 4 y relevo por voz pendientes. |
| Selector auxiliar ADSR en contexto Dr. Wave | Color 4 del mismo callback: rango original generado, liberación rápida de seis estados al cambiar nibble y 131.072 muestras ADSR contrastadas; pool completo y scratch dither compartido. Tipo 2 y pool inicializado explícitos; callbacks de tipo 4 y relevo por voz excluidos. |
| Modificador de volumen/pitch en serie | 81 combinaciones, 65.536 muestras de volumen y 65.536 palabras de pitch; 61.568 muestras Dr. Wave posteriores. Cuatro suavizados compartidos contrastados con segundo parche GNU de cargas halfword, verificado con probes propios. Ambos buffers iniciales explícitos; no completa mezcla/salida. |
| Entrada de control a cuatro fuentes del modificador | Ruta válida `0x01a84f8c`: 32.768 parejas + 256 llamadas continuas, tabla completa contrastada. Otros 2.048 decodes alimentan volumen/pitch y 61.568 muestras Dr. Wave; status/canal, inicializador y scheduler no recuperados. |
| Destino instrumento en contexto Dr. Wave | Rangos originales generados, ocho selectores hacia cuatro controles, wrap/clamp y 61.568 muestras posteriores contrastados. Tipo 2, parámetros iniciales y contador explícitos; no completa dispatcher ni integración. |
| Destinos instrumento/volumen/pitch intercalados | Las 256 combinaciones en una llamada tipo 2, 512 registros, suavizado/estado compartidos y otras 61.568 muestras contrastados. El ensayo se limita a kind 0; imágenes, contador y selección explícitos. |
| Dispatch tipo 2 con ENV y kind 0/1 | Las 625 combinaciones por kind, 1.506 llamadas: 91.648 muestras activas Dr. Wave y 95.104 posiciones ADSR (86.656 escritas). Pools completos, RNG/dither y guardas contrastados; imagen de trabajo, renderer, contador y selección explícitos. No recupera el llamador original. |
| Inicializador Core de workspaces y límite del llamador | Rutina completa `0x01968774`: 82 bytes a cero en imagen de 228, 4.096 casos + 256 llamadas continuas y preservación de pitch/ADSR/RNG. Cadena de 1.506 llamadas: 91.648 muestras Dr. Wave / 95.104 posiciones ADSR (85.120 escritas). La recarga por llamada continúa sin recuperar, con candidatos L1 protegidos inventariados; no se sustituye por ceros. |
| Cadencia externa, voz completa e integración | Referencia candidata al contador en L1 junto a franjas cifradas; proveedor/cadencia aún sin verificar. Asignación/pitch/ADSR y salida originales siguen pendientes. El núcleo ya se integra mediante los adaptadores híbridos propios documentados desde 0.14; estos oráculos no validan el planificador L1. |

[Lectura y mezcla de Dr. Wave](drwave246-reader.md) ·
[Generación de los seis bancos base](drwave246-wavebank.md) ·
[Tres transformaciones de la onda de voz](drwave246-wave-transform.md) ·
[Mezcla espectral y render aislado completo](drwave246-kernel.md) ·
[Contador de 16 bits y dependencia del proveedor](drwave246-counter.md) ·
[Factoría, disparo y release de seis voces](drwave246-core-routing.md) ·
[Preparación y activación VoiceCtl](drwave246-note-activation.md).

La ruta candidata del proveedor del contador continúa bloqueada en L1
parcialmente cifrada. El siguiente paso verificable del mismo componente es
contrastar las demás rutas de actualización del pitch de VoiceCtl y resolver su
relevo al buffer del render. La [actualización selectiva/broadcast](drwave246-pitch-update.md)
ya está contrastada; la referencia candidata al relevo está en L1 parcialmente cifrada.
La activación inicial está contrastada con afinación explícita;
esa entrada no recupera la calibración original ni suple el relevo ADSR pendiente.

La [entrada de pitch bend](drwave246-pitch-bend-input.md) está contrastada como
decodificador independiente. Su control no se conecta a los registros de pitch
ni al render con una fórmula supuesta; su consumidor original sigue pendiente.

El [setter del índice](drwave246-tuning-index.md) y el
[productor de las 2400 entradas](drwave246-tuning-table.md) ya están contrastados
junto a su consumo en la activación de Dr. Wave. La tabla deja de ser una entrada
de coeficientes explícitos en el nuevo ensayo. Setup completo, calibración física,
relevo L1 y demás rutas de pitch permanecen pendientes.

La [repreparación de pitch por lista/modo](drwave246-pitch-refresh.md) ya está
contrastada en el contexto acotado de módulos Core auxiliares ausentes. Incluye
callbacks ADSR y RNG compartido, sin completar render/relevo ADSR, asignador,
cadencia, salida ni el escritor original del buffer. Se mantiene Dr. Wave activo.

El [modificador de cuatro entradas](drwave246-pitch-buffer-modifier.md) ya está
contrastado sobre el buffer y seguido del render. No sustituye el escritor
inicial bloqueado en L1. Sin hardware ni volcado L1 descifrado disponible, el
siguiente avance verificable debe limitarse a otra dependencia legible del
buffer o del relevo ADSR, conservando explícitas las entradas y la cadencia.


La [entrada ENV de cuatro canales](drwave246-envelope-control-input.md) está contrastada
junto al render ADSR común. El [color auxiliar 4 en contexto Dr. Wave](drwave246-envelope-mode-control.md)
ya está contrastado con seis estados ADSR y scratch compartido; no se completa el relevo
por voz, el llenado inicial de pitch ni la salida de Dr. Wave. El [destino de volumen y las combinaciones con pitch](drwave246-volume-buffer-modifier.md)
ya están contrastados sobre buffers preparados, con suavizado compartido y un
segundo parche GNU acotado para cargas halfword. Se mantienen pendientes los
productores iniciales, relevo por voz y salida. La [entrada de control hacia las cuatro fuentes](drwave246-control-source-input.md)
ya está contrastada junto a su consumo por el modificador. Dispatcher MIDI,
inicializador de fuentes y reloj siguen pendientes. El [destino instrumento para el tipo interno 2](drwave246-instrument-control-routing.md)
ya está contrastado con sus rangos originales y el render posterior. Las [256 combinaciones de instrumento/volumen/pitch](drwave246-mixed-control-routing.md)
ya están contrastadas en llamadas conjuntas, con suavizado y render posteriores.
La [convivencia con ENV y kind 0/1](drwave246-control-env-dispatch.md) ya está
contrastada en el contexto de tipo 2 y pool inicializado, con ambos renderers
elegidos por el harness. El [inicializador Core de workspaces](drwave246-control-workspace-init.md) está
contrastado completo y junto al callback/render. La búsqueda de recarga de la
imagen por llamada deja dos pares de dirección junto a franjas L1 protegidas
y un candidato parcial: no se recupera el productor ni se le asignan ceros.
Se conserva el inventario para no repetir el bloqueo. La [retirada polifónica
VoiceCtl y el release ADSR desde la lista](drwave246-voicectl-poly-release.md)
ya están contrastados con lista completa, seis voces y render ADSR posterior
explícito. No se completa el scheduler ni el relevo de campos pendientes. El
[avance de selectores 1–4](drwave246-release-selectors.md) contrasta también
retirada de la última coincidencia, recuperación de pitch con afinación explícita,
RNG y release de una/seis voces. Los modos externos y la integración siguen fuera.
El [constructor de argumentos de nota `0x01a85014`](voice246-note-argument.md)
ya está contrastado por separado, y la [composición con activación/retirada y
ADSR](drwave246-note-event-chain.md) compara el argumento y sus efectos sin
sobrescribirlo entre rutinas. El orden y las listas siguen siendo del harness,
no el dispatcher/asignador original. El [setter `0x01a8633c`](voice246-note-adjustment.md)
ya está contrastado por separado y la [cadena de ajustes/eventos](drwave246-adjusted-note-chain.md)
suministra la tabla mediante setters originales, sin overrides tras ellos.
Los valores iniciales y el orden siguen explícitos. El [llamador polifónico con
primera voz libre](drwave246-poly-first-free.md) ya contrasta selección, inicio
ADSR/ganancia, activación y adición de lista dentro del original, incluidos flag,
duplicados y aceptación parcial con Core bloqueado. La actividad libre sigue
siendo entrada del ensayo; no completa asignador/cadencia. La [rama de voces
ocupadas y solicitud de reinicio](drwave246-poly-steal.md) ya contrasta máximo
signed32, primer empate, producción de pitch/velocidad pendientes y primera
reasignación de lista, con rampa ADSR posterior. No recupera el consumidor de
esos campos. La [consulta `0x01a863a8`](voice246-last-voice-query.md) también está
contrastada por separado. El [dispatcher de retirada](voice246-release-dispatch.md)
ya está contrastado para discriminador cero y mapa de notificación vacío (PR #25).
El [consumidor gráfico de última voz](voice246-last-voice-consumer.md) quedó
clasificado y cerrado (PR #27): no aporta un productor DSP.
Los [selectores de activación 1–3 con lista vacía](drwave246-note-modes.md)
ya contrastan inicio frente a reinicio pendiente por actividad, diferencia de
escala RNG, aceptación parcial Core y entrada de lista con voz -2. El contador
cero y la actividad se suministran cada vez; no se recupera el scheduler.
Los [selectores con lista no vacía](drwave246-held-note-modes.md) ya están
contrastados en PR #29: el 2 escribe pitch directamente, el 1/3 producen
reinicio pendiente y conservan actividad. La [composición de eventos con estado
retenido](drwave246-note-lifecycle.md) contrasta activación/duplicados/retirada y
ADSR sin reponer lista ni actividad entre eventos. El orden sigue siendo del
harness y retirar la última nota no se sustituye por una limpieza de actividad.
El [pitch explícito no cero con lista vacía](drwave246-empty-explicit-pitch.md)
ya está contrastado en PR #31 para selectores 1–3. El [despacho exterior de
activación](voice246-note-dispatch.md) contrasta discriminador cero con metadata
cero: indicador byte, normalización de velocidad -1 y copia local antes de gate/
duplicados interiores. La metadata 1 del constructor habitual y su notificación
no se sustituyen por ese contexto acotado.
El [pitch explícito del selector polifónico 0](drwave246-poly-explicit-pitch.md)
ya está contrastado en PR #32, tanto libre como ocupado, sin desviación RNG.
La [composición con metadata 1 y salida explícitamente cero](voice246-quiet-note-lifecycle.md)
ya ejecuta constructor, ambos dispatchers, notificaciones y ADSR con mapa/estado
retenidos, sin borrar metadata ni inventar transporte. Se contrasta actualización
del mapa incluso con on bloqueado/duplicado o retirada ausente; la palabra cero
no se presenta como configuración de fábrica.
El [accessor e inicialización diferida del pool ADSR](adsr246-lazy-pool.md),
`0x0191264c..0x019126d8`, ya está contrastado: 1.024 accesos y una cadena
de 1.536 eventos con ready retenido, pool frío/listo y gates sin inicialización.
El [controlador compartido de eventos de instrumento](voice246-instrument-lifecycle.md)
ya compone selectores 0–4, lookup/pitch explícito, pools fríos/listos y cambios
de selector entre eventos: 3.072 eventos contrastados, sin reponer actividad ni
inventar el commit de pendientes. Se centraliza el despacho de las interfaces
anteriores y se conservan sus regresiones.
Siguen pendientes buffers iniciales, consumidor de pendientes, reloj, mezcla y salida.

## Cluster rev246: conservado con dependencias bloqueadas

| Parte | Estado |
| --- | --- |
| Render de ocho osciladores, realimentación y mezcla | Reconstrucción aislada contrastada con coeficientes explícitos del llamador. |
| Estado de 108 bytes, reset/release, RNG y contexto compartido | 2.048 bloques / 262.144 muestras; estado completo contrastado. |
| Rangos de los cuatro controles | Contrastados con el inicializador original. |
| 24 coeficientes de amortiguación | Conversión original float → Q31 contrastada. |
| Normalización | Cinco entradas legibles convertidas por el helper original; faltan las de 2, 3 y 8 osciladores. Otras 81.920 muestras contrastadas con las entradas conocidas. No se extrapolan las desconocidas. |
| Factoría de objetos | Seis slots de 108 bytes: 60 casos contrastados de construcción/reset, vecinos, relleno y RNG. No completa el asignador/cadencia. |
| Inicialización compartida | Rutina completa contrastada en 256 casos con ocho palabras fuente explícitas; pesos/display a cero y campos/RNG/amortiguación no escritos preservados. No completa la tabla original. |
| Conversión de tablas | Helpers Q15/Q31 contrastados en 65.536 patrones binary32: 131.072 resultados. Coinciden también con GNU sin el parche local para estos casos. |
| Cadencia, pitch por muestra, envolvente/ruteo y salida completos | Pendientes. |
| Disparo y liberación desde Core | Note-on condicionado a coincidencia entre sonido cargado y seleccionado; release sin esa condición. Factoría Core tipo 4 y seis punteros contrastados. |
| Estado compartido entre voces | 131.072 muestras adicionales y 1.024 pools completos, con seis slots intercalados y un RNG/contexto compartidos. Orden externo explícito; no completa el scheduler original. |
| Dependencia ADSR: inicialización y reinicio | Callbacks y rampa de etapa 6 → 7 → ataque contrastados: 327.680 muestras / 2.560 registros. Conservación de nivel/ganancia y almacenamiento de palabras pendientes. No completa su consumo ni el ruteo con Cluster. |
| Preparación/activación desde VoiceCtl | Tabla de 128 notas y preparación de pitch/velocidad/actividad contrastadas, con afinación explícita. 131.072 muestras posteriores con incremento mantenido por el harness. Calibración/afinación original y proveedor de pitch por muestra pendientes. |
| Consumidor de campos ADSR pendientes | Referencias candidatas localizadas en L1 parcialmente cifrada; faltan instrucciones verificadas del relevo. No se ejecuta ese tramo ni se supone su flujo. |
| Integración en el selector del plugin | Pendiente; el núcleo aún no se expone como motor original completo. |

El siguiente paso de **este bloque** es resolver la procedencia/inicialización de
las tres entradas restantes de normalización, la calibración de afinación y el
relevo L1 pendiente, y contrastar el contrato completo
antes de conectarlo al plugin. [Avance de normalización/inicialización](cluster246-normalization.md) ·
[Inicialización compartida y apoyo de REA](cluster246-shared-init.md) ·
[Disparo, liberación y seis voces intercaladas](cluster246-core-routing.md) ·
[Transiciones del ADSR común](cluster246-envelope-transitions.md) ·
[Preparación de nota y bloqueo del relevo L1](cluster246-note-activation.md) ·
[Contrato y reproducción del núcleo](cluster246-kernel.md).

La búsqueda de afinación ha identificado un inicializador en `0x01a866e4` que
construye **2.400 palabras de cuatro bytes** desde `0x027fc720`, usando el callback
`0x01a84388`: este solo escribe cero. El bloque LDR que cubre esa memoria también
es un fill a cero. Ese callback no proporciona una afinación válida. La
investigación posterior de Dr. Wave ha localizado y contrastado el productor en
la cola `0x01a85454`: la tabla común ya se genera, sin completar el setup previo
ni una nueva validación de su consumo por Cluster. Las tres normalizaciones y
el relevo L1 cifrado siguen pendientes; Dr. Wave se mantiene como bloque activo.

## Herramienta para investigar L1

La [descompilación con Ghidra Blackfin](boot246-decompiler.md) localiza el
callback del bootloader y la lectura ROM/OTP de la clave en índice 0xd0.
Se contrastan ambas primitivas XTEA y su pseudocódigo con claves sintéticas,
pero no se recuperan los 16 bytes OTP ni se descifra L1. No se repiten las
auditorías de referencias como sustituto de ese dato. La integración de
Dr. Wave conserva los bloqueos de productor de pitch/relevo/clock.

## Cola de componentes

La [matriz completa](original-op1-coverage.md) conserva el estado individual. El
orden puede ajustarse si una dependencia cifrada bloquea un componente; se registra
el motivo y el siguiente bloque activo, manteniendo un solo frente.

| Área | Trabajo pendiente que debe contrastarse |
| --- | --- |
| Motores | Completar Dr. Wave; retomar Cluster cuando se resuelvan sus dependencias; después FM y demás motores. Resolver las tablas L1 necesarias; no sustituir un motor por otra síntesis con el mismo nombre. |
| Filtros | Filtros internos de cada motor, ruta Filter/Nitro y etapas de salida/decimación. Cada ruta requiere coeficientes, estado e impulsos/barridos contrastados. |
| Envolventes | Completar cuatro campos ADSR auxiliares, ruteo y modos de ejecución; envolventes específicas de D-synth/D-box/drum. El ADSR común ya contrastado no las completa. |
| Moduladores | Extender Value a rutas pendientes; ampliar Random synth (ya contrastado e integrado en cuatro motores) a FX/drum; después Element/MIDI/Bend/Crank, con sus destinos, reinicios y fase libre originales. |
| Integración de voz | Preparación de pitch, poly/mono/legato/unison/portamento, asignación y reloj originales, además de mezcla/salida. |

## Aplazado

Identificación adicional y afinado de presets, bancos de samples, recetas de
sonidos, comparación con el vídeo y cambios visuales ajenos a una integración DSP
necesaria. No se añaden presets para presentar avances de esta etapa.

La coincidencia con el modelo GNU documentado prueba los casos aislados bajo ese
modelo. No certifica por sí sola la máquina completa ni temporización/ASIO reales.

## Entrega 0.19.0: Random synth

[Random 246](random246-native.md) queda contrastado con 4096 llamadas e integrado
en las cuatro familias nativas, con las dos pasadas Instrument y ADSR. El próximo
bloque propuesto es Filter/Nitro: identificar el callback legible y contrastar
coeficientes, estado e impulsos antes de integrarlo. El bloqueo OTP/L1 persiste;
no se declara resuelto por avanzar en este modulador.

## Entrega 0.20.0: Filter mono y bloqueo Nitro

[Filter 246](filter246-native.md) coincide en 4096 bloques de 64 muestras, estado,
display y tablas generadas. El bus de salida propio conserva la dependencia del
pico de bloque completo y declara su latencia fija, incluso en bypass. Value FX,
MIDI y guardado integrados; no se reconstruye la ruta estéreo original.

Nitro consume palabras protegidas de L1; [registro de offsets](firmware/nitro246-table-blocker.json).
El siguiente bloque debe evaluar el adaptador híbrido sin atribuirle coeficientes
originales y continuar las rutas restantes. Sin claves OTP ni captura descifrada,
la fidelidad de esas dependencias sigue pendiente. No se añaden presets.

## Entrega 0.21.0: Nitro híbrido condicionado

[Nitro Hybrid](nitro246-hybrid.md) conserva la aritmética mono y usa una mezcla
simétrica propia para dos coeficientes protegidos, con punteros explícitos.
4.096 bloques y estados/displays coinciden bajo esas sustituciones. Se integra
con Value FX, controles y guardado en el bus existente; no se recupera L1.
Siguiente bloque: otros efectos y calibración independiente del modelo híbrido.

## Entrega 0.22.0: Delay mono

[Delay 246](delay246-native.md) conserva los siete taps, acumulaciones Q15/Q31,
tres cursores y suma seca/envío. Se reproducen la tabla legible de 225 valores y
el reset de cursores. El núcleo coincide en 4096 bloques, estados/displays y ocho
memorias circulares completas bajo una vinculación explícita del workspace externo.
Se integra con Value FX, controles y guardado en el bus compartido. No se reconstruyen
el allocator original, ruta estéreo ni reloj completo. Siguiente bloque: Phone/Grid.

## Entrega 0.23.0: Grid mono y selector

[Grid 246](grid246-native.md) contrasta 4096 bloques, estados/displays, reset sobre
memoria retenida y ocho snapshots de dos planos 260×260. El dominio acotado usa
slot 2 y cursores válidos; el adaptador normaliza los cursores al reducir ejes de
forma incompatible. Esa política de resize y el ruteo siguen siendo propios.
Selector común, Value FX, automatización y persistencia integrados. Phone tiene
callback legible 0x01a1bb74; siguiente paso: buffers segmentados y filtros compartidos.

## Entrega 0.24.0: Phone mono

[Phone 246](phone246-native.md) contrasta 4096 bloques, estados/displays y ocho
workspaces completos. Se conservan los tres buffers, cruces por cero, repeticiones,
filtros y resampling. Las tablas vienen de conversiones legibles; el slot 0 y dos
guardas hacen explícito el acceso de centinela al final del stride lógico. Soft
reset contrastado y pánico completo propio se documentan por separado. Selector,
Value FX y persistencia integrados. Siguiente bloque: Spring/CWO y ruteo de salida.

## Entrega 0.24.1: salida y ciclo de Spring

El volumen maestro sigue al bus de FX y controla sus colas sin vaciar memorias;
[contrato de salida](output-routing.md). [Spring: ciclo de memoria](spring246-lifecycle.md)
contrasta 8 resets, 16 callbacks pendientes y 24 capturas de dos slots. Conserva
los historiales y las líneas intermedias; ambos callbacks borran el mismo tramo
final. No se integra todavía un render de Spring. Se localizan también los
auxiliares y tablas candidatos de CWO. Próximo cierre: inicializador y render
activo de Spring, controles/Value/persistencia; después productores y render CWO.

## Entrega 0.25.0: Spring mono integrado

[Spring 246](spring246-native.md) ejecuta su inicializador estático y contrasta
4096 bloques, con 4088 renders activos y ocho callbacks pendientes. Coinciden
las tablas, cuatro constantes, estados/displays/contador y ocho workspaces
completos. El dominio usa slot 0, no el allocator. La activación/pánico del host
limpian todo mediante política propia distinta al soft reset original. Selector,
Value FX, automatización, proxies y persistencia integrados; volumen tras el bus
y PDC mantenidos. Siguiente componente: productores/auxiliares y render de CWO.

## Fundamentos de CWO (instrumento 0.25.0)

[Ocho auxiliares y tablas](cwo246-foundation.md) contrastados en 65.536 casos.
Se identifican cuatro coeficientes de fase protegidos; los cuatro legibles se
conservan y se diseña un modelo propio explícito para los otros. El oráculo de la
red de fase queda condicionado a esas sustituciones. Verificación independiente
de JUCE y de los instaladores. Quedan retardo y buffers, fases por bloque y
composición original; después integración/Value/controles/persistencia. El diseño
de cuadratura no sustituye una comparación acústica ni recupera L1.

## Entrega 0.26.0: CWO Hybrid completo e integrado

[Composición mono](cwo246-hybrid.md) contrastada en 4096 bloques con todos los
estados/RNG/display y ocho buffers; repetición independiente idéntica. Retardo
y oscilador se contrastan además en 8192 casos y dieciséis memorias completas.
Se conservan los cuatro coeficientes de fase legibles y se sustituyen los otros
cuatro por el modelo propio ya documentado. PDC, volumen posterior y reset propio
del host diferenciados del original. Selector/Value/persistencia integrados.
Los ocho FX tienen opción mono tocable; siguiente prioridad: voz/ruteo y motores
restantes, empezando por las dependencias del FM de cuatro operadores. No dar por
resuelta la calibración acústica ni adelantar la réplica completa de presets.

## Base interna del FM, instrumento 0.26.0

[Contrato y ensayos](fm246-foundation.md): se cierran conversión/copia de las 30
palabras, constructor/reset distintos, factoría de seis slots y actualización de
seis envolventes internas. Las 20 palabras legibles se bloquean contra cambios y
10 protegidas se sustituyen por un modelo propio explícito. La comparación del
arranque es condicionada; el test de envolventes usa tablas sintéticas explícitas.

Siguiente cierre del mismo frente: reconstruir los nueve destinos de render de
`0x019a23d4` y contrastar audio, estados y scratch con la tabla definida; después
integrar voz/ADSR/reloj propios y validar el host en un único lote. No reinterpretar
el FM propio de dos osciladores como ese render ni añadir presets para aparentarlo.

## FM Hybrid 0.27.0

Se cierra el render mono `0x019a23d4` de cuatro operadores y nueve rutas, contrastado
en 4096 bloques con estado completo, tablas y scratch. [Integración y límites](fm246-hybrid.md).
Se mantienen las veinte constantes legibles y diez sustituciones explícitas.
La voz tocable usa ADSR reconstruida y adaptadores propios de pitch/reloj/salida.
Quedan los moduladores Instrument/ENV del FM, dependencias protegidas, scheduler
original y comparación acústica/Live; presets completos al final.


## FM 0.28.0: afinación y modulación

[Pitch fraccionario propio y moduladores](fm246-performance.md): conserva el
callback original separado y exacto con pitch entero; conecta Value Instrument/ENV,
Random y Tremolo al tipo FM 0 y sus descriptores contrastados. No recupera el
scheduler ni modifica las diez constantes híbridas. El siguiente frente será
comparar carácter con las referencias de vídeo ya aportadas y priorizar los
motores restantes según la matriz, sin pretender calibrar constantes a partir de
patches con FX/LFO activos ni bloquear el proyecto por falta de hardware.


## String 246 (0.29.0)

[Render mono y ciclo](string246-native.md) contrastados con tablas legibles sin
sustitución: 4096 bloques, estados/RNG, constructor/binding slot 0/reset/release y
16 workspaces completos. Cuatro controles raw, ADSR, seis voces y moduladores
integrados; Value/Random contrastados para tipo 3. Reloj, cursor/pitch y asignación
siguen siendo adaptaciones. El siguiente bloque prioriza los motores restantes
(DNA/Voltage/D-synth según dependencias), manteniendo Pulse pendiente de L1 y
calibración orientativa separada. Presets originales completos al final.


## DNA Hybrid 246 (0.30.0)

[Render y perfil virtual](dna246-hybrid.md): 4096 bloques, seis estados vivos,
RNG y dieciséis capturas de onda/RAM coinciden bajo entradas explícitas. La onda
original deriva de 32 bytes de identidad obtenidos por OTP; se contrasta el tramo
posterior al lector con cuatro identidades. El render lee además RAM DSP viva:
una prueba pareada aísla su efecto audible. El host usa identidad propia fija,
pool original de seis estados y ceros externos; no se afirma recuperar memoria
cifrada ni el carácter acústico de un aparato determinado.

Cuatro controles, ADSR, Value/Random/Tremolo, interpretación y estado integrados.
Voltage se completa en 0.31.0 en el alcance descrito debajo; sigue D-synth
según dependencias. No renombrar aproximaciones como constantes recuperadas
ni adelantar los presets originales completos.


## Voltage 246 (0.31.0)

[Render y ciclo contrastados](voltage246-native.md): la candidata `0x01a49bf4`
queda validada en 4096 bloques / 524288 muestras, pool de seis estados antes/después,
factoría/reset/release, pitch variable por muestra, display y guardas. Seno y 129
coeficientes legibles, sin sustituciones; esta ruta no lee OTP/RAM externa ni RNG.
Value/Random tipo13 añaden 8192 callbacks. Integración completa de controles/ADSR,
modulación, modos de voz, interfaz, automatización y estado. Reloj/cadencia/pitch,
política de voz, composición ADSR y salida propios: no equivalencia acústica.

Próximo bloque: estudiar D-synth y sus envolventes/dependencias antes de delimitar
la integración. Pulse sigue condicionado por L1, y los presets completos al final.


Prelectura del siguiente bloque: el hook del op1-sdk ya acreditado identifica
D-synth en `0x0197e1d8`; la palabra de render de la vtable `0x0190fbbc + 0x14`
coincide con esa dirección en el LDR disponible. Esto confirma la referencia,
**no valida su implementación ni sus dependencias**. Revisar el grafo de llamadas,
ocho controles y envolventes específicas antes de delimitar el próximo oráculo.


## D-synth: ciclo y modulación de ocho destinos

[Base implementada y límites](dsynth246-foundation.md): factoría, binding, reset,
release, inicialización compartida, decaimiento de indicadores y captura de
histórico contrastados. Tipo12 kind3: Value selecciona ocho controles y Random
los trata como ocho destinos directos. 8192 callbacks coincidentes.

El harness ejecuta y registra el render original completo en4096 bloques; sus
524288 muestras son observaciones, no comparaciones con un renderer C++ ya
implementado. Las tablas legibles coinciden con ADSR/Filter existentes. Ghidra
omite ramas ejecutadas: continuar desde simulación/disassembly privado para
portar ambos osciladores, envolventes, todas las mezclas y filtros antes de añadir
D-synth al plugin. No hacen falta materiales privados adicionales ahora.
