# Fuente drum compartida — 0.40.0

En **Sampler 246 → KIT EDITOR**, **LOAD SOURCE** carga los primeros 12 segundos
mono de un WAV/AIFF y los presenta como 24 regiones de una sola grabación.
Start/End, ganancia, pitch, reverse y Gated/One Shot/Loop siguen siendo independientes
por región. Los límites se pueden mover o solapar en toda la fuente, sin recortar
24 archivos. RESET PAD devuelve únicamente esa región a su intervalo inicial.

El manual, §6.2–6.3 (PDF 13–14), describe una grabación drum de 12 segundos con
puntos de entrada/salida asignados a teclas. La división inicial en **24 partes
iguales** es una elección propia, editable: no se ha recuperado el detector o
layout original. Se conserva el banco anterior de muestras independientes al
cargar un kit de fábrica o de usuario.

## Audio y memoria

El kit compartido contiene audio float y PCM16 de la fuente únicamente en su
primer slot; los otros 23 están vacíos. Las doce voces seleccionan ese mismo PCM
y usan los límites del pad disparado. Los lectores conservan sus caches, estados
FIR y modos individuales. No se copian ni recortan las muestras dentro del callback.
La vista muestra toda la grabación y los límites actuales de las 24 regiones.

La importación, reducción estéreo a mono, recorte de los primeros 12 segundos,
resampling lineal a 44,1 kHz, saneamiento de valores y preparación de PCM son
adaptadores propios fuera del callback. Se reutiliza el lector FIR ya contrastado;
este bloque **no aporta un nuevo contraste de ejecución del firmware** ni recupera
sus productores de pitch/ganancia, allocator, filesystem o reloj.

El modo compartido también mantiene la selección de regiones al desactivar el
lector nativo y usar el reproductor anterior. Los moduladores Value, Random,
Tremolo y MIDI existentes siguen disponibles con las mismas limitaciones de 0.39.
La dinámica de región continúa siendo propia; ADSR synth no sustituye al drum.

## Cargar, grabar y guardar

- LOAD SOURCE reemplaza el kit completo y crea los 24 intervalos iniciales.
  Los archivos originales no se modifican. Las muestras independientes se cargan
  con LOAD PAD en un kit no compartido; el API rechaza esa operación sobre una
  fuente compartida para no dejar 23 regiones sin audio.
- Shift + IN, con el sampler drum nativo activo, captura hasta 12 segundos y
  conserva el destino drum. El sampler melódico mantiene su captura de 16 segundos.
  El destino queda fijado al empezar la captura.
- Los proyectos guardan el indicador de layout y un solo payload `kitAudio0`,
  además de todos los límites y controles. La restauración prepara el PCM fuera
  del callback y vuelve a vincular los 24 pads a la fuente.
- Los slots guardan una instantánea del audio y sus controles: pueden recuperar
  una fuente sustituida, incluso después de cerrar/restaurar un proyecto o borrar
  el archivo importado. Cada slot es una instantánea independiente, por lo que un
  proyecto puede contener copias deliberadas de fuentes en sus distintos slots.
- Clipboard conserva tanto audio como layout. Pegar un kit independiente borra
  el indicador compartido. La entrada “Current source” del browser se refiere a
  la fuente actualmente cargada; los slots conservan las versiones anteriores.

## Validación y límites

`SharedDrum246Tests.cpp` compara la composición de una sola fuente con una
referencia de 24 copias del mismo PCM completo y los mismos límites absolutos:
**105 combinaciones** de siete tasas, tres modos y cinco opciones de modulación.
Compara muestras con doce voces, robo de voz, eventos, release y pánico; comprueba
además los 24 pads, límites iniciales y recorte/preparación a seis tasas de entrada.
Es una prueba de composición/propiedad de datos del adaptador, no del sonido del
OP-1 físico. No equivale a comparar regiones recortadas, cuyos bordes FIR cambian.

Las pruebas del plugin cubren importación real de WAV y borrado del original,
proyectos sin 24 payloads duplicados, edición, slots portables, clipboard, captura
y partición exacta de audio/MIDI en bloques de 64 y 257 muestras. Las cargas de
tiempo real incluyen fuente compartida, doce voces, reverse/modos y moduladores.

[Pruebas y CI de entrega](releases/v0.40.0-validation.json). Continúan pendientes
segmentación/layout originales, productores y ruteo de voz, calibración acústica,
Ableton Live real, ejecución arm64 y presets originales completos.
