# Dr. Wave y Cluster: Mono, Legato y Glide

En **set 2 / T2**, junto a la ADSR, selecciona **POLY**, **MONO** o **LEGATO**.
**GLIDE** establece un portamento de 0–2000 ms entre notas solapadas. Los dos
controles también están disponibles como parámetros del host: `Hybrid: Voice
Mode` y `Hybrid: Glide Time (ms)`. Funcionan con el editor cerrado y se guardan
en sesiones, slots de sonido y clipboard. El init de cada motor empieza en Poly,
sin glide. Los cuatro mandos de color siguen controlando ADSR en esta página.

## Comportamiento

- **Poly** conserva las seis voces y el robo por antigüedad existentes. Glide no
  actúa en este modo.
- **Mono** usa una voz y reinicia osciladores, ADSR y Value en cada nueva nota o
  retorno a otra nota retenida. FREE de Value conserva su política de fase.
  El relevo de 3 ms evita sustituir de golpe la última contribución de la voz.
- **Legato** mantiene los osciladores, ADSR y fase Value mientras haya notas
  solapadas y la voz siga activa. Una nota separada vuelve a iniciar la voz.
- Prima la última nota retenida. Al soltarla se vuelve a la anterior con su
  velocidad y canal. Soltar una nota anterior no reinicia la voz actual.
- Sustain retiene notas solo en su canal; pedal-up elimina las soltadas de ese
  canal y vuelve a evaluar la prioridad. Bend ±2 semitonos sigue al canal de la
  nota que suena. El canal interno 17 del secuenciador es independiente de los
  16 canales MIDI. Las 128 notas de cada canal tienen almacenamiento fijo.
- Glide recorre intervalos a velocidad constante en semitonos. Su duración se
  fija al comenzar cada transición; una nueva transición parte del pitch actual.
  Sin solapamiento, la nota comienza directamente afinada. Bend actúa después
  de glide. Cambiar el tiempo durante una transición afecta a la siguiente.
- Pánico y cambio de motor borran notas retenidas. Cambiar Poly/Mono/Legato
  también retira las voces y su lista; vuelve a tocar para iniciar nuevas notas.
  All-notes-off libera la envolvente y limpia las notas del canal indicado;
  all-sound-off aplica el pánico inmediato existente.

## Procedencia y límites

Esta política es **un adaptador propio** de interpretación. No es un port de los
selectores originales 0–4 de VoiceCtl ni resuelve el consumidor ADSR L1. Los
oráculos del controlador original siguen comprobándose por separado. Tampoco
implementa unison original ni extiende estos modos al resto de motores.

Los núcleos Dr. Wave, Cluster y ADSR contrastados permanecen intactos. Se conservan
el reloj interno, normalización incompleta de Cluster, resampling, salida y demás
aproximaciones documentadas en sus guías. El pitch se calcula a la tasa del host,
pero cada motor sigue recibiéndolo en sus bloques de 128 muestras a 88,2 kHz;
el cambio audible puede retrasarse un bloque interno (~1,45 ms), más el filtro
causal. No se afirma una equivalencia acústica o temporal con el OP-1 físico.

## Comprobación reproducible

`hybrid_performance` ejercita ambos motores y ambos modos a 8/44,1/48/88,2/96/192
kHz: señal, afinación audible, trayectoria/fin del glide, reinicio frente a legato,
prioridad/fallback, notas iguales en canales distintos, bend, sustain por canal,
velocidad cero, las 2176 posiciones nota/canal, pánico, cambios de modo/motor y
retorno a seis voces Poly. AddressSanitizer/UBSan se aplican al mismo ensayo.

`plugin_smoke` comprueba controles de interfaz y host Configure, sesiones,
slots/clipboard y MIDI sin editor. Compara renders con buffers de 64 y 257
muestras, incluidos timestamps, glide interrumpido por fallback, bend, pedal y
pánico. `realtime` añade 24 cargas Mono/Legato con glide y Value por voz a las
132 anteriores, rastreando heap tanto en la primera nota como en callbacks
continuos. Los tiempos en una máquina compartida son diagnósticos; no certifican
un plazo físico de audio. Windows/macOS y sus paquetes se validan en el CI del
commit correspondiente. Ableton real y hardware OP-1 siguen pendientes.
