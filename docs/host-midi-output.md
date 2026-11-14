# Salida MIDI al host — 0.55.0

En **COM → Send keys and sequencer to host MIDI** se activa una salida opcional
para las teclas locales y las notas generadas por el secuenciador/arpegiador.
El parámetro automatizable **Output: Host MIDI** está desactivado por defecto.
VST3 y standalone anuncian capacidad de salida MIDI; el host debe enrutarla a
su destino. El destino físico y su selección pertenecen al host/standalone.
La aceptación y el ruteo en Ableton Live real siguen pendientes de prueba.

## Contrato funcional

- Teclas locales: canal original 1–16, timestamp cero del siguiente callback
  con muestras. Con arpegiador activo se envía su salida, no la tecla fuente.
- Secuenciador/arpegiador: canal 1 y el mismo offset de disparo/cierre usado por
  el motor. Gate, transporte y reloj son los de la implementación del host.
- La entrada del host no se reenvía: ni notas, ni CC, ni SysEx. Puede alimentar
  al arpegiador, cuya salida sí se genera. No se envían clock, bend ni sustain.
- Una nota compartida por tecla y secuencia en el mismo canal se mantiene hasta
  soltar ambos propietarios. Los duplicados de una misma fuente se suprimen.
- Al activar no se reconstruyen notas ya retenidas. Al desactivar, cambiar modo,
  usar Panic, restaurar estado o desbordar colas se liberan las notas publicadas.
  Detener la secuencia libera su nota; las teclas locales siguen disponibles.
- Reprepare/releaseResources solicitan limpieza para el siguiente callback con
  muestras. No existe envío fuera del callback; si el host deja de llamarlo,
  la limpieza no puede entregarse hasta que reanude el procesamiento.
- Bloques de cero muestras no consumen eventos locales ni limpieza pendiente.

El parámetro se añade después de los existentes, conservando sus índices. Se
guarda como propiedad global `hostMidiOutput` en el estado completo. Los estados
anteriores sin ella lo cargan apagado; los slots de sonido no lo modifican.

## Frontera con el original y tiempo real

El estudio de [productores MIDI](midi246-pump.md) y de la
[bomba original](midi246-pump.md) aporta contexto de mensajes y transporte.
Esta integración usa ownership, cadencia, colisiones y política de overflow
**propios**. No ejecuta la bomba USB original, sus requests ni su reloj físico;
no demuestra equivalencia temporal/acústica del instrumento original.

`HostMidiOutput` reserva estáticamente 4.096 eventos y 2.048 propietarios:
presupuesto normal de 1.024 eventos por callback más cierres de emergencia.
Al agotarlo bloquea nuevos eventos normales y libera todas las notas retenidas.
La cota cubre notas iniciales y note-ons emitidos, incluso con varios flushes.
El router no reserva memoria dinámica. El `MidiBuffer` suministrado por JUCE
puede crecer si está frío o el host lo sustituye: **no se promete ausencia
universal de heap en el callback**. Las cargas medidas usan buffers preparados.

## Validación

[Evidencia estructurada](releases/v0.55.0-validation.json): 147/147 suites Release
Linux, router con ASan/UBSan y seis cargas nuevas a 44,1/48/96 kHz y bloques
64/257, 1.536 callbacks sin llamadas de heap rastreadas. Se comprueban offsets,
partición de bloques, transporte, audio idéntico con salida activada/desactivada,
no eco, limpieza, migración y cero muestras. La prueba gráfica abre COM y activa
el parámetro con su menú real, usando Xvfb y xfwm4; captura revisada localmente.
La prueba de capacidad recorre las 2.048 notas y llena los 4.096 eventos sin
notas colgadas. También pasa la regresión previa de 1.446 cargas de tiempo real.
Los sanitizadores de este bloque cubren el router, no el plugin completo.

Se han compilado VST3 y standalone Linux; CI del producto registrado por separado.
No se han generado instaladores. Hardware MIDI físico y Ableton real pendientes.
