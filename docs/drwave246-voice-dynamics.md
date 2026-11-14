# Dr. Wave: velocidad y ganancia por nota — 0.56.0

Desde 0.57.0, el [robo polifónico](drwave246-voice-restart.md) conserva la
ganancia previa mientras entrega pitch/velocidad pendientes. Esa ruta sustituye
la política de reinicio inmediato descrita aquí para 0.56.0.

Dr. Wave aplica ahora la velocidad y la atenuación por nota dentro de la ruta
ADSR de punto fijo. El multiplicador flotante de velocidad posterior se elimina
para este motor. No hay un mando nuevo: cambia la respuesta de las notas al tocar,
y las notas agudas reciben la atenuación del controlador original contrastado.
Los demás motores conservan su respuesta anterior.

## Procedencia y frontera

La [activación VoiceCtl](drwave246-note-activation.md) escribe velocidad signed16
saturando el halfword bajo desplazado ocho bits. Para MIDI 0–127 produce
`code × 256`. Las [ramas de activación](drwave246-poly-first-free.md) y
[selectores 1–3](drwave246-note-modes.md) escriben la ganancia baja ADSR:
`32767` para nota 0, `32768 − note × 128` para notas 1–127, preservando la mitad
alta. La [repreparación de pitch](drwave246-pitch-refresh.md) también actualiza
esa ganancia. Los modelos originales y sus fixtures permanecen sin cambios.

El [kernel ADSR](envelope246-research.md) combina volumen y ganancia por producto Q15
con redondeo al par; después multiplica el nivel Q31, añade dither, satura y
cuantiza a signed16. `Phase246Voice` usa ahora esa combinación en su adaptación
por muestra. Con volumen/ganancia predeterminados, el coeficiente sigue siendo
`0x7ffe0000`, conservando el camino anterior de los otros motores.

`DrWave246Voice::setDynamics` introduce dos decisiones **propias del host**:
convertir su velocidad float finita a MIDI 0–127 por redondeo al más cercano y
actualizar nota/velocidad sobre el reloj interno híbrido. Volumen, ganancia y
parámetros se capturan al comenzar cada bloque interno de 128 muestras; cambios
legato pueden esperar al bloque siguiente. No se recupera por ello el productor
L1, el servicio de reinicios pendientes, su clock ni el scheduler original.

Poly/Mono/Legato, prioridad por canal, sustain, glide, robo y cola para suavizar el cambio de voz siguen
siendo la política híbrida documentada. Al robar, el adaptador reinicia la voz y
recalcula la ganancia para la nueva nota; la rama original ocupada puede conservar
la ganancia anterior y dejar un reinicio pendiente. Esa composición no se atribuye
al firmware. El nivel puede ser menor que en 0.55.0; no se compensa con otra
ganancia supuesta. En Legato se conserva la envolvente y se
actualizan las palabras de dinámica al cambiar la nota prioritaria. El filtro de
salida, normalización global y mezcla continúan siendo propios. No se extrapola
la ganancia a Cluster ni a otros motores sin contrastar su composición.

## Verificación

- La prueba ADSR existente compara además el camino por muestra del producto con
  los 4.096 bloques originales: 524.288 posiciones, incluyendo las inactivas,
  y estado final/dither. Filtros de salida identidad y moduladores desactivados
  permiten observar la ganancia sin introducir una referencia flotante nueva.
- 16.384 combinaciones de nota y código MIDI comparan el puente con los registros
  del helper VoiceCtl ya contrastado; se verifica la conservación de bits altos.
  No son 16.384 ejecuciones nuevas del firmware original.
- Nueve escenarios Engine (44,1/48/96 kHz × Poly/Mono/Legato) incluyen sustain,
  fallback, robo y retirada. Dos velocidades float distintas del mismo código
  MIDI producen audio idéntico, detectando un segundo multiplicador de velocidad.
  La energía a velocidad 32 frente a 64 se comprueba cerca de 1/4.
- Un cambio de volumen/ganancia dentro del bloque no entra hasta el siguiente.

[Evidencia de entrega](releases/v0.56.0-validation.json) registra regresión,
sanitizadores y builds. No hay nueva UI ni instaladores. La coincidencia del
kernel no mide fidelidad acústica del instrumento completo; Ableton real y
hardware físico continúan pendientes.
