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


## D-synth 246 integrado (0.32.0)

[Render completo](dsynth246-native.md): el C++ ya calcula el estado posterior y
coincide en4096 bloques/524288 muestras, incluyendo las dos envolventes internas,
cuatro configuraciones, ocho mezclas, pitch/ganancia y contexto de seis voces.
Tercer parche GNU BF524 de cargas Dreg relativas a FP; probes y §4-9 del manual
sustentan operandos anteriores al paquete. Se repiten doce oráculos sin cambios
de sus resultados. Tablas originales sin sustituciones. Ocho controles, bancos,
ganancia ADSR antes del filtro, interpretación, modulación, interfaz y estado
integrados. Cadencias, asignación/velocidad y salida siguen siendo propios.

El siguiente bloque prioriza estudiar D-box/drum y sus envolventes/dependencias,
además del ruteo de voz original pendiente. Pulse/L1 y calibración siguen
explícitamente abiertos; presets originales completos al final.


## D-box 246 integrado (0.33.0)

[Render completo y pads](dbox246-native.md): `0x01970ee8`, vtable verificada,
doce estados de72 bytes y contexto de 1640 bytes. 4096 bloques/524288 muestras
coincidentes bajo streams explícitos de pitch/ganancia y el modelo BF524 corregido.
Ocho mezclas, ambas alineaciones, envolventes internas, ciclo, consulta de actividad,
indicadores/históricos y todos los buffers contrastados. Sin coeficientes sustituidos.
24 pads/192 controles, doce voces, Tremolo, host/estado/slots/UI integrados;
afinación, mapa/cadencias, allocator/eventos/velocidad y salida son propios.

Siguiente frente: estudiar drum sampler/envolvente dinámica y sus dependencias,
incluido Value/Random de batería antes de habilitar esos destinos. El dispatch
synth no demuestra el de drum. Ruteo de voz, Pulse/L1 y comparación acústica siguen
pendientes. Mantener los presets completos al final; no hacen falta materiales
privados adicionales para reproducir la evidencia de D-box.


## Sampler de batería 246 integrado (0.34.0)

[Callback completo y adaptación](drum246-native.md): vtable verificada apunta a
`0x019789ec`; doce estados de 20 bytes, ciclo/actividad, posiciones coarse/fine y
FIR de 16 taps. 1025 coeficientes convertidos por el helper original sin sustitución.
4096 bloques/524288 muestras coincidentes con PCM sintético/padding explícito,
pitch no negativo, ganancia signed16 y modelo BF524 corregido. Pools, audio,
pitch/ganancia derivada, PCM intacto y guardas contrastados; repetición idéntica.

Integración opt-in sobre el kit: doce voces, Gated/One Shot/Loop, reverse, trims
finos, automatización/estado/slots/clipboard, editor y Tremolo. Conversión/cache,
reloj/afinación/asignación, alineación/salida y contorno dinámico son propios.
La curva sigue la descripción funcional del manual, no una rutina original
recuperada. El banco propio de 24 muestras tampoco completa la fuente única de 12 s.

Próximo bloque: delimitar el productor original de la ganancia dinámica y sus
dependencias y continuar la familia sampler, incluido el sampler synth. Value y
Random de batería requieren su propio dispatch/oráculo antes de habilitarlos.
El gain stream `0xff801d50` es entrada del lector: no dar por recuperado su
productor por coincidir el consumidor. No atribuir a L1 un bloqueo sin seguir sus
dependencias. Voz/ruteo original, Pulse/L1, acústica/Live/arm64 y presets completos
mantienen sus pendientes y límites anteriores.

## Sampler melódico 246 integrado (0.35.0)

[Callback y límites](sampler246-hybrid.md): `0x01a51a7c` es el renderer completo;
`0x01a51a66` era un helper y no debe citarse como callback. Seis estados de 16
bytes, bind/reset/release, FIR original, cursores Q12, loop independiente/reverse
contrastados en 4096 bloques/524288 muestras con pitch signed32 y padding explícito.
Las dos ejecuciones originales son idénticas; no hay sustituciones de coeficientes.

Integración opt-in con Start/Length/Root/Loop y Reverse, seis voces y estado/slots/UI.
El mapa de región, cache/clock/tuning, ADSR/velocity/allocation y salida son propios;
la integración no expone aún todos los límites/fine de loop que contrasta el núcleo.
Siguiente entrega: completar controles secundarios y composición de voz del sampler,
y seguir el productor de dinámica/dispatch drum sin inventar la parte protegida.
La referencia de preparación de ganancia en `0xffa00700` delimita una candidatura
L1 parcialmente protegida; no es una función completa recuperada ni prueba de
ruteo de los cuatro controles. La curva drum propia continúa identificada como tal.

## Controles y composición del sampler (0.36.0)

Se exponen los ocho campos del lector contrastado, sin normalizar sus posiciones
a un trim relativo: Start, Loop In/Out, End, Reverse, fine In/Out y volumen.
LOOP conserva un gate propio. ADSR 246 opcional reutiliza el núcleo compartido;
32768 muestras de composición independiente comparan lector, ADSR y dither.
Mono/Legato/glide usa el adaptador propio, con canales, sustain y vuelta de nota.
No se ha recuperado el compositor de voz original ni el reloj/allocator físico.

Los campos secundarios ya no bloquean el sampler tocable. Próximo frente:
delimitar el dispatch de moduladores del sampler antes de habilitar destinos,
y continuar dinámica/dispatch drum. Conservar el rastro L1 concreto y evitar
presentar la referencia aislada al gain buffer como su productor recuperado.
Calibración, voz/ruteo original, Pulse/L1, Live/arm64 y presets siguen pendientes.

## Modulación del sampler (0.37.0)

[Contrato y límites](sampler246-modulation.md): tipo 6 y ocho descriptores flag 2;
Value/Random usan reglas acopladas de región, no el mapa de cuatro synth repetido.
4096+4096 callbacks coincidentes, estados/displays/destinos/RNG/tempo/retornos y
repetición idéntica. Integración de ocho destinos Value por banco, Random directo,
ADSR y Tremolo con reloj/composición de voz propios. Sin nuevos IDs/presets.

Siguiente bloque: helper drum `0x01a1e57c`, Random `0x019cedc0` y Value drum,
con sus propias selecciones/tipos y oráculos antes de habilitar destinos. La
vtable de Random drum y 4096 observaciones originales delimitan una ruta ejecutable,
no un port contrastado ni un productor de dinámica. El gain stream y su referencia
L1 siguen abiertos por separado; no etiquetar todo el dispatcher como cifrado.
Voz/ruteo/clock original, Pulse/L1, acústica/Live/arm64 y presets completos pendientes.

## Value / Random de batería (0.38.0)

[Contrato y adaptación](drum246-modulation.md): Value `0x019cfdac`, Random
`0x019cedc0` y helper `0x01a1e57c` contrastados para tipos 8/9 en 16384 llamadas,
con estado/display/RNG/destinos/retornos/tempo y descriptores completos; repetición
idéntica. Se integran ocho destinos por golpe, bancos de Value, Random, proxies,
estado y UI de drum y D-box. El destino ADSR es rechazado por el callback; no se
conecta el ADSR común a la dinámica. La secuencia Random comparte aritmética,
con dispatch de synth y drum separados.

El contorno de ganancia drum sigue propio. La referencia L1 a `0xff801d50` no es
su productor recuperado; estas rutas solo ajustan palabras de Instrument. Próximo
frente: productores originales de pitch/ganancia, ruteo/eventos y moduladores
Element/MIDI/Bend/Crank según la matriz, sin inferir ejecución de bytes protegidos.
Fuente drum compartida, calibración, Live/arm64, Pulse/L1 y presets finales pendientes.


## Bloque MIDI LFO — 0.39.0

Se resolvió el dispatch tipo 5 de synth, drum y FX: **6.144 callbacks originales**
con globals/entradas explícitas, comparación completa y repetición independiente.
La [integración](midi246-hybrid.md) añade cuatro CC a Instrument/ADSR/FX; el receptor,
combinación de canales y hold a timestamp son propios. El núcleo contrasta también
modificaciones de buffers gain/pitch/audio, sin exponerlas aún como rutas de producto.
El proceso no recupera sus productores. Drum rechaza ADSR. FX kind 4 queda fuera.

Antes de ampliar las rutas de audio/pitch hay que integrar sus buffers y documentar
su composición con envolvente, velocidad y bend; no llamar productor a una rutina
que solo modifica datos suministrados. Element, Bend y Crank requieren estudiar
primero sus entradas físicas. Bend es el accesorio manual del §15.8, no el MIDI
pitch wheel. Continúan fuente drum compartida, ruteo original, Pulse/L1 y calibración;
los presets originales completos siguen al final.


## Fuente compartida drum — 0.40.0

Se integra el layout funcional de una fuente de 12 segundos y 24 regiones, con
carga/captura/edición/estado/slots/clipboard y doce lectores vinculados al mismo PCM.
105 combinaciones comparan esta composición con copias explícitas del PCM completo;
no son ejecuciones nuevas del firmware. Preparación/división inicial y ruteo siguen
propios. [Contrato](shared-drum246.md). Se mantiene pendiente el algoritmo original
de layout/segmentación, además de productores, voz/ruteo/clock, LFO físicos, Pulse/L1,
acústica/Live/arm64 y presets finales.


## Bend manual — 0.41.0

Se contrastan 6.144 callbacks originales intercalados de Bend (sexta entrada LFO, índice 5): procesador
compartido de entrada, historial/flags/contador, wrap, dirección, reposo y dispatch
Instrument/primeros cuatro ADSR/FX kinds 2/5, con buffers suministrados intactos.
Matriz aceptada completa para los tipos soportados y repetición exacta.
[Contrato](bend246-hybrid.md). Se integra un encoder naranja virtual y Reverse,
con reloj/activación propios, estado, proxies y selector LFO. Los auxiliares FX
kind 4 y rutas especiales de audio/pitch no forman parte de este port.

Continúan Element/Crank, productores de voz y ruteo/clock, layout original, Pulse/L1,
calibración/Live/arm64 y presets finales. La entrada manual no es el pitch wheel
MIDI, y no se atribuye su adaptación de host a un lector físico recuperado.


## Crank manual — 0.42.0

Se contrastan 18.432 callbacks de la séptima entrada LFO (índice 6) synth/drum/FX, incluida su fuente compartida
`0x019cae26`: contador/flags, historial de diez deltas, ángulo, velocidad, target,
pasos fraccionarios, fase/triángulo y dispatch de Instrument/primeros cuatro ADSR/FX
kinds 2/5. Comparación completa y repetición independiente bajo entradas acotadas.
[Contrato](crank246-hybrid.md). El encoder azul y reloj/lifecycle son propios; se
integran estado, proxies, UI y destinos. No se atribuye recuperación de scheduler,
lectura del accesorio, productor gain/pitch ni secuenciadores Crank.

Siguiente frente: estudiar Element y sus fuentes de entrada antes de delimitar el
oráculo. Siguen productores/voz/ruteo/clock, layout, Pulse/L1, calibración/Live/arm64
y presets completos al final. Las rutas especiales de los LFO no se convierten en
productores de buffers por el hecho de ajustar datos suministrados.

Para Element, el manual §15.2 distingue G-force, entrada externa (radio/line/mic),
envolvente del motor y nivel del motor. El siguiente estudio debe delimitar esas
cuatro entradas y sus callbacks antes de elegir el adaptador de host. Un slider
virtual o un seguidor de audio propio no deben presentarse como recuperación del
sensor, del detector original ni de sus productores de envolvente/nivel.


## Element — 0.43.0

La tabla de nombres/factoría identifica Element como LfoReRoute, índice 3; no
confundir con LfoDual/Tremolo. Se contrastan 18.432 callbacks de synth, drum y FX,
cuatro fuentes suministradas, helpers/rangos y buffers completos intactos; repetición
idéntica. [Contrato](element246-hybrid.md). El plugin integra sensor virtual, input
externo, máximo ADSR global y nivel pre-FX con frontend/ventana propios, estado,
proxies y UI. Se conservan las diferencias de escala/orden y offset directo de FX.

Siguiente prioridad: trazar los productores de `0xff801056`, `0xff80108c`,
`0xff80108e` y `0x01dfc80c`, y la composición original de voz/ruteo antes de
sustituir sus adaptadores. `0x01a66912` solo lee una palabra suministrada; no
es su productor validado. Siguen voz/allocator/clock, gain/pitch drum, rutas
especiales de LFO, layout, Pulse/L1, acústica/Live/arm64 y presets completos.


## Publicador de Envelope — investigación posterior a 0.43.0

Se reconstruye y contrasta el escritor `0x01a66922` de `0x01dfc80c`, su getter
`0x01a66912` y reset `0x01a66bd8`: 4.096 bloques/524.288 halfwords de audio,
cuatro estados de prueba intercalados y palabra compartida, repetición idéntica.
El getter es positivo en 1.684 bloques silenciosos: no equivale sin más a una
ADSR o un detector de amplitud. [Contrato y pendientes](element246-producers.md).

El núcleo queda en research, sin enlace al plugin ni nueva versión/build de
producto. Antes de integrarlo hay que localizar la llamada activa, entrada de
audio, controles/defaults, propiedad de los dos estados y orden/cadencia respecto
a Element. No atribuirle compresión maestra ni bloqueos L1 sin demostrar su ruteo.
Los otros tres productores siguen pendientes.


## Composición de publicador y Element — investigación posterior a 0.43.0

[Composición original contrastada](element246-composition.md): 4.096 bloques /
524.288 muestras y 8.192 consumidores, con la publicación generada por el escritor,
sin reemplazo entre ambas rutinas. Dos puntos de lectura sintéticos producen
destinos diferentes en 250 bloques; el orden del hardware sigue sin validarse.
2.048 despachos aceptados cubren drum 8/9, kinds 0–3 y los ocho FX kind 5.
Estado/audio completos, repetición idéntica y sanitizadores aprobados. La búsqueda
acotada de CALL/literales/inmediatos no localiza el llamador; no atribuir a L1 ni
a compresión maestra. Sigue pendiente el contrato activo antes de integrar.


## Validación nativa ARM64 y estado del rastreo

Se verifica el producto 0.43.0 en macOS 26 ARM64: 126/126 suites, más las dos de
publicador/composición Envelope, carga real del VST3 y arranque del standalone.
[Alcance y evidencia](native-macos-arm64.md). No equivale a validación en Ableton,
dispositivos de audio, instalador o todos los macOS anteriores.

La revisión adicional de alias con base/desplazamiento no localiza un nuevo
productor activo verificable de Element. El llamador del publicador, controles,
audio y cadencia siguen sin resolverse; no se atribuyen automáticamente a L1.
Mantener el núcleo fuera del plugin hasta tener ese contrato. Evitar repetir los
mismos barridos o la composición sintética sin una nueva candidata; continuar
con dependencias legibles y rutas de voz/modulación de la matriz que permitan un
bloque verificable independiente. No hacen falta nuevas grabaciones para ese
trabajo.


## MIDI FX Input — integración híbrida 0.44.0

Las curvas de audio del callback MIDI FX `0x019cd104` (kinds 2/5, destino 3,
colores 2/3) se conectan como destinos FX Input Bipolar/Steep de CC1–4. El
puente coincide con 21.568 muestras de los fixtures originales existentes y se
conservan los 6.144 callbacks completos. No son nuevas ejecuciones del firmware.
La posición antes del núcleo FX, la lectura por muestra interna, mezcla y reloj
son propios del host. [Alcance](midi246-fx-input.md). Punch/bypass no se modifican.

Siguiente prioridad verificable: adaptadores de gain/pitch de voz ya contrastados,
con definición explícita de sus buffers y ciclo de vida antes de conectarlos.
No reutilizar esta entrada como Envelope original ni afirmar scheduler recuperado.


## MIDI voice gain/pitch — integración híbrida 0.45.0

Se conectan las operaciones especiales del callback synth MIDI `0x019cce68`,
kind 0, destino 3, colores 0/1, a diez motores melódicos. Ambos puentes, stream y
bloque, coinciden con 11.392 pares observados en los fixtures originales; se
conserva el oráculo completo de 6.144 llamadas. No se ejecutó un nuevo oráculo del firmware.
[Alcance](midi246-voice.md). Ownership del display, inicialización, callbacks de
128 muestras internas, integración con ADSR y límites son adaptaciones propias.
No se recuperan el productor de buffers, allocator ni scheduler originales.

Siguiente prioridad: estudiar las otras rutas especiales de modulación y el
contrato de productores/ruteo legibles; mantener Envelope de Element fuera del
plugin hasta localizar una llamada activa validable. Calibración acústica,
Live, firma/notarización, Pulse/L1 y presets completos siguen pendientes.
Antes de otro cambio del producto, consultar los trabajos aún pendientes en la
[evidencia 0.45.0](releases/v0.45.0-validation.json) y resolver cualquier fallo
real; no repetir compilaciones que ya hayan concluido correctamente.


## Random FX — integración 0.46.0

El callback `0x019cefb0` y reset `0x019cf2f4` se contrastan en 4096 llamadas,
2048 independientes y 2048 retenidas, con los ocho FX, kinds 2/5, estado de
44 bytes, RNG, display, ocho targets y retorno completos. La repetición privada
es idéntica. Se integra el destino FX, cuatro controles simultáneos, UI,
automatización, proxies y estado. Ownership global del bus, reloj, seed y
lifecycle son propios. No se integra la rama kind 4 ni se infiere el caller.
[Contrato](random246-fx.md) · [Resultados](releases/v0.46.0-validation.json).

Siguiente frente: delimitar el consumidor original de las rutas especiales que
faltan y revisar la matriz para priorizar voz/ruteo frente a más adaptadores de
parámetros. Mantener Envelope de Element fuera del plugin mientras no exista
una llamada activa validable. Pulse/L1, calibración acústica, Live real,
firma/notarización y presets completos siguen pendientes; no repetir búsquedas
acotadas agotadas ni presentar más fixtures como mayor fidelidad acústica.

Revisión del 5 de noviembre: el CI completo 0.46.0 de `25a26bc` termina aprobado
en Linux, Windows/MSVC y macOS universal (runtime Intel). ARM64 termina sin
adquirir runner ni ejecutar pasos por falta de capacidad; no es evidencia de
fallo del producto ni validación de esa revisión. No repetir builds aprobados.
La corrección `25a26bc` añade `<string>` al verificador MSVC sin cambiar DSP;
usar sus trabajos nuevos, no los cancelados de `34cd351`. El CI completo 0.45.0
también terminó aprobado en los tres sistemas.


## Bend Voice — 0.47.0

[Level/Pitch](bend246-voice.md) amplía el callback synth `0x019c9d44`, kind 0,
destino 3, colores 0/1. 6.144 llamadas originales comparan estado/historial/display,
controles/targets/tag/retorno y buffers pitch/gain/FX completos, con retención y
repetición idéntica. 696 aceptadas producen 89.088 pares comparados también con
el puente por bloques; no se confunden las 5.448 rechazadas con audio renderizado.

Se integran diez motores melódicos y ambos destinos, con coeficiente compartido,
UI, estado y automatización. El clock de 44100/64, las capturas de 128 muestras,
lifecycle y ubicación de gain/pitch en la voz siguen siendo adaptaciones propias.
El oráculo no proporciona el contrato activo del scheduler ni sus productores.
Mantener Envelope de Element fuera del plugin, como exige su estudio.

Siguiente frente: contrastar los especiales restantes de Crank/Element y sus
consumidores, sin reinterpretar buffers explícitos como productores recuperados.
El productor de ganancia drum, ruteo/voz completos, Pulse/L1, calibración acústica,
Live real, firma/notarización y presets completos permanecen separados.


## Crank Voice — 0.48.0

[Level/Pitch](crank246-voice.md): callback synth 0x019cb0d4, kind 0, destino 3,
colores 0/1. 18.432 llamadas originales, 2.060 aceptadas, 16.372 rechazadas;
263.680 pares para el puente y repetición independiente idéntica. Ganancia
suavizada compartida y pitch asimétrico según mitad de fase triangular.
Diez motores con UI/estado/automatización; owner Bend/Crank separado en captura.
Reloj, inicialización, lifecycle y composición siguen siendo propios.
Siguiente: especiales FX/Element y consumidores activos; mantener Envelope de
Element fuera del plugin sin llamada activa verificable. Productores de voz,
dinámica drum, Pulse/L1, acústica/Live y presets finales siguen pendientes.


## Bend/Crank FX Input — 0.49.0

[Gain/Shape](manual246-fx-input.md): callbacks 0x019ca328 / 0x019cb3d4,
kinds 2/5, destino 3, colores 2/3. 8192 llamadas por fuente, 3493 aceptadas cada
una, 447104 muestras del puente total; repetición independiente idéntica.
Siete FX, controles, automatización y persistencia. Clock/fuente global,
ubicación por muestra cuantizada y composición del host propios. No se
reinterpretan como scheduler o productores recuperados. Siguiente frente:
especiales de Element y consumidores activos, dinámica drum y productores/ruteo.
Mantener Envelope de Element fuera del plugin sin llamada activa verificable.
Pulse/L1, acústica, Live y presets finales siguen pendientes.


## Element FX Input — 0.50.0

[Multiply/Subtract](element246-fx-input.md): callback 0x019cdf22, kinds 2/5,
destino 3, colores 2/3, cuatro fuentes. 8192 llamadas; 3494 procesadas, incluidas
874 Self con retorno cero; 223616 muestras originales y 111936 del puente kind 2.
La fuente externa usa muestras firmadas, no el pico de parámetros; Self usa el
propio audio, no la palabra Level. Frontend, remuestreo alineado y ubicación del
bus son propios. No recupera productores ni scheduler. Siguiente: especiales
Gain/Pitch de Element y consumidores, dinámica drum y ruteo/productores de voz.
Mantener Envelope auxiliar fuera del plugin sin contrato activo validable.
Pulse/L1, acústica/Live y presets originales completos siguen pendientes.


## Element Voice Gain — 0.51.0

[Gain](element246-voice-gain.md): synth 0x019cda14, kind 0, destino 3, color 0.
8192 llamadas, cuatro fuentes, 1540 procesadas y 197120 gains del puente.
385 Level retornan cero; core lo conserva y host usa predicado separado.
Diez motores y fuentes Sensor/External/Gain Self/Level; externa causal con
captura histórica de 64 muestras por voz hacia 128 gains. SRC, frontend,
captura/lifecycle/clock propios. No sustituir esas pruebas por fidelidad acústica.
Siguiente: Pitch de Element y helpers/consumidores, dinámica drum y productores
originales. Envelope auxiliar sigue fuera sin contrato activo. Presets al final.


## Element Voice Pitch — 0.52.0

`0x019cda14`, kind 0/destino 3/color 1: 8192 llamadas, 5780 procesadas, 2412
rechazadas y 1444 Level procesadas con retorno cero. Repetición idéntica y 5175
casos del puente stream/bloque, con gain constante explícito para Env→Pitch.
Núcleo conserva cadena Q31 extrema de Audio. [Contrato](element246-voice-pitch.md).

Conserva la ganancia de 0.51.0 y su frontend SRC; contorno previo por voz, captura,
clock, ownership y lifecycle propios. Añade cola conservadora D-synth para la
ganancia aditiva que puede persistir tras ADSR. No se recuperan productores ni
scheduler. Siguiente: dinámica drum y contratos legibles de voz/ruteo sobre más
rutas de parámetros. Mantener Envelope auxiliar fuera sin llamada activa
validable. Pulse/L1, acústica/Live, ARM64 actual y presets finales siguen pendientes.

El CI completo 0.52.0 terminó aprobado en Linux, Windows/MSVC y macOS
universal (runtime Intel), igual que 0.51.0. No repetir builds del producto
sin cambiarlo.

## Core del drum sampler — investigación posterior a 0.52.0

[Contrato](drum246-core-routing.md): factory tipo 8, reset y release con 4.608
casos originales y rejilla 12 × 8 × 24. 3.456 renders / 442.368 muestras, estados
y buffers completos contrastados; dos extracciones idénticas. Suite standalone
Release y ASan/UBSan aprobadas; CI nativo aprobado en Linux, Windows/MSVC y
macOS Intel. No integra código en el
instrumento, no cambia versión ni genera instaladores. `s_drum_producers` son
objetos de audio, no el productor de dinámica. Siguiente: llamador activo de
render/actividad y escritores pitch/gain de esos doce objetos; conservar dinámica
y asignación híbridas mientras falta esa composición. No extrapolar a D-box,
scheduler MIDI o equivalencia acústica. Mantener Envelope auxiliar fuera sin
contrato activo y presets originales completos al final.

## Drum VoiceCtl — investigación posterior al ruteo Core

[Note-on/off y consulta de actividad](drum246-voice-lifecycle.md): 4.096 casos
originales, 2.176 notas iniciadas / 2.044 ocupadas, 854 retiradas con propietario
antiguo y 393.216 muestras. Modelo independiente, dos extracciones idénticas,
Release y ASan/UBSan aprobados. Compara lista retenida, globals, doce estados y
buffers completos. Pitch de evento solo cinco transposiciones por octavas. No integra
código en el instrumento ni incrementa versión; CI nativo y regresiones aprobados
en Linux, Windows/MSVC y macOS Intel. Siguiente:
retirada de ocupación, llamador activo de render y productores pitch/gain. No
reemplazar la dinámica ni el scheduler híbridos sin contrastar esa composición.

## Drum bank / registro retenido — investigación posterior a VoiceCtl

[Contratos](drum246-bank-lifecycle.md): 8.192 casos originales, 4.096 máscaras de
ocupación, 128 pares begin/factory finish, 256 resets retenidos sobre 2.176 filas,
128 limpiezas aisladas y 786.432 muestras con entradas/cadencia explícitas.
Dos extracciones idénticas, Release y ASan/UBSan completos aprobados; CI nativo
separado y regresiones del lector aprobados en Linux, Windows/MSVC y macOS Intel.
Clear registry conserva lectores y filas residuales, no reemplaza
pánico ni retirada de audio. MIDI output=0: solo flags externos, sin transmisor.
No integra código ni aumenta versión. Se clasifican las referencias candidatas
anteriores, descartando coincidencias de inmediato bajo. Siguiente: llamador de
render y consumidor activo de velocidad/pitch mediante accesos indirectos;
productores de buffers y dinámica aún pendientes, sin atribuir automáticamente
su ausencia a L1. Mantener el instrumento híbrido y presets completos al final.

## Asignador drum híbrido — 0.53.0

[Integración](drum246-voice-allocation.md): selector original compartido entre
Source y los dos verificadores de investigación (12.288 registros, 4.416
selecciones aceptadas). Doce hits con primera libre/mayor contador firmado,
consultas con efectos laterales y empate por índice. Registro nota/canal y
liberación por propietario; frontera de ocupación/retirada, sustain, pánico,
canal interno y fallback negativo propios. No integra los resets de banco o
clear registry como retirada normal. Aplica la decisión híbrida del propietario:
se integra un contrato probado sin atribuir sus fronteras al scheduler original.
84 composiciones de lector y ASan/UBSan aprobados; Linux 143/143 suites,
1.446 cargas sin heap rastreado y cuatro casos de plugin 48/96 kHz / 64/257
aprobados. CI nativo y regresiones aprobados en Linux, Windows/MSVC y macOS
Intel; plugins e instaladores completos de 0.53.0 también aprobados.
Siguiente: llamador de render y productores de buffers; mantener curva dinámica,
frontend y cadencia del host identificados como propios. No repetir búsquedas de
inmediatos bajos como prueba de L1; presets completos permanecen al final.

## Palabras de evento drum — 0.54.0

[Velocidad y transposición](drum246-event-words.md): 65.536 note-on originales
completos, todos los halfwords de velocidad y 256 transposiciones enteras,
con cruce independiente del byte bajo. Dos extracciones idénticas; tabla de
salidas derivadas, no ROM recuperada. Helpers compartidos con VoiceCtl/bancos.
El host usa códigos MIDI, anclas de pitch entero e interpolación geométrica
propia, con bend y factor dos propios. No se recuperan productores por muestra,
curva dinámica ni scheduler. No cambia parámetros, proyectos, D-box o kit legacy.
Siguiente: consumidor activo de estas palabras y composición con pitch/gain;
seguir el llamador de render. Mantener avance ≈70 % ±10, sin medición acústica.

Validación local 0.54.0: Linux 145/145 suites, doce casos de plugin, 1.446
cargas sin heap rastreado, ASan/UBSan del corpus y puente completos aprobados.
CI nativo aprobado. El empaquetado anterior se canceló por petición del propietario;
el CI completo de reemplazo pasó en las tres plataformas sin generar instaladores.

## Inicialización Core y cadena de eventos — investigación tras 0.54.0

[Contrato](core246-initial-state.md): inicializador amplio 0x01967fcc, workspace
0x01968774, factory tipo 8 y note-on completos en 256 cadenas originales.
Cuatro ventanas observadas, 4.641 bytes fijos y 1.215 preservados; los objetos y
ocupación no se vacían. Tabla derivada de defaults, no port general de soft-float.
Todos los pares banco/pad y transposiciones, Release y ASan/UBSan aprobados;
CI separado sin compilación del producto. No cambia versión, DSP ni instaladores.
Siguiente: consumidor activo de velocidad/pitch y llamador de render; no usar
esta inicialización como productor de buffers o pánico. Avance sin cambio.
El propietario pide bloques más largos y empaquetado solo cuando vuelva a pedirlo.

## Kernel Core de 64 muestras y referencias — investigación tras inicialización

[Contrato y auditoría](core246-signal-kernel.md): rutina 0x019674f8 completa,
4.864 llamadas / 311.296 muestras, 512 bloques retenidos y 256 compuestos con
inicializador. Historia, truncado, saturaciones y guardas coinciden; Release y
ASan/UBSan aprobados, dos extracciones idénticas. Nueve casos sintéticos del
auditor protegen la separación de bytes conocidos/protegidos y alias parciales.
Carga de pitch drum en 0xffa00718 y secuencia visible de llamada al kernel en
0xffa0288c/0xffa02894 localizadas; contexto vecino protegido, sin llamador completo
ni calendario contrastados. No se clasifica el kernel como productor de dinámica.
Queda fuera del plugin hasta recuperar su ruteo. Siguiente: consumidor del pitch
y contexto de esas llamadas; mantener contrato híbrido, sin rellenar dependencia
con ceros o deducir L1 por ausencia de CALL directos. Avance y versión sin cambios.

Cierre del bloque: CI nativo aprobado, 2/2 suites por plataforma en Linux,
Windows/MSVC y macOS Intel. Sin cambios en el instrumento ni instaladores.

## Despacho Core de reinicios — investigación tras referencias L1

[Contrato](core246-modulator-reset.md): dos dispatchers completos y dos limpiezas,
8.192 casos / 8.830 llamadas a cinco resets originales Value/Random/Tremolo.
Gate primario, bytes no canónicos, this firmado y recarga de selección para el
secundario contrastados. La limpieza borra punteros, conserva objetos; no pánico.
Dos extracciones iguales y Release/ASan/UBSan aprobados. No ejecuta la factory ni
el note-on exterior, no cambia audio o versión. El contexto visible de pitch L1
sigue interrumpido: se avanza este contrato SDRAM sin inventar el productor.
Siguiente bloque agrupado: note-on exterior con reset activo y lector, orden de
palabras/ocupación/lista y cambios de selección; después consumidor/render/gain.
Sin instaladores hasta nueva petición; planificación ≈70 % ±10 sin medir acústica.

Cierre del reinicio Core: CI nativo aprobado, 1/1 suite en Linux, Windows/MSVC
y macOS Intel. No se reconstruye el producto ni se generan instaladores.

## Note-on con auxiliares activos y lector — composición agrupada

[Contrato](drum246-active-note-chain.md): 4.096 note-on completos con dispatcher
y resets originales activos, 3.826 notas aceptadas, 4.138 resets LFO y 3.826 del
sampler. Orden de palabras, ocupación/lista y estado del lector observado en
cada callback; 459 cambios sintéticos de selección alcanzan los controles del
lector sin cambiar su referencia enlazada. Rejilla 12×8×24 y 256 transposiciones.
Hook explícito en el modelo; la ruta por defecto mantiene 78.080 registros de
regresión. Release/ASan/UBSan y reproducción congelada aprobados. No ejecuta la
factory LFO ni render, no cambia plugin/versión o instaladores. Siguiente bloque:
callbacks de modulación por bloque + lector bajo entradas/cadencia explícitas,
sin confundir esa composición con productores pitch/gain o scheduler del aparato.

Cierre de note-on activo: suite nueva y cinco regresiones CI aprobadas en Linux,
Windows/MSVC y macOS Intel; sin build del producto ni instaladores nuevos.


## Modulación retenida y render de batería — composición agrupada

[Contrato](drum246-modulated-render.md): 288 sesiones con 16 bloques retenidos,
4608 callbacks Value/Random drum y Tremolo común, 288 note-on/off completos y
4464 renders / 571392 muestras; 144 omisiones conservan los buffers esperados.
Pool, globals, estados/displays/RNG, controles, punteros, lista retenida,
pitch/gain/derivada y audio coinciden. Dos ejecuciones originales independientes,
Release y ASan/UBSan aprobados, sin alterar modelos previos. No demuestra el
reloj del aparato, productores de pitch/gain o factory Tremolo: todo eso se
suministra explícitamente. Próximo frente: productores/consumidores activos y
ruteo protegido; usar esta composición como regresión de la ruta híbrida.
Sin cambios en producto 0.54.0, estimación ≈70 % ±10 o instaladores.

Cierre de modulación/render retenidos: CI nativo aprobado en Linux, Windows/MSVC
y macOS Intel sobre `70ac42a`; sin build del producto ni instaladores.


## Arranque de buffers compartidos — investigación tras render retenido

[Contrato](core246-buffer-initialization.md): inicializador completo 0x01a3b34c,
con constructor y callbacks originales, 1024 escenarios, 512 inicializaciones,
512 kernels / 32768 muestras y 496 filas retenidas. Cuatro buffers a cero y seis
bytes de defaults; historia/coeficiente/salida del kernel se conservan por init.
64 filas compuestas producen salida pese a entrada borrada. No usar como pánico
ni productor de dinámica. Tres ejecuciones originales coinciden; Release y
ASan/UBSan aprobados. Auditor de 22 direcciones, con destinos de llamadas
posteriores separados de los valores cargados y 16 pruebas sintéticas.
Siguen pendientes productores activos, scheduler y ruteo protegido; el borrado
inicial no rellena esas dependencias. Producto 0.54.0 y ≈70 % ±10 sin cambios;
sin instaladores. Regresiones Core y render retenido conservadas.
