# Cambio de banco drum 246 con servicio L1 propio

El cambiador original `0x01a86420..0x01a86660` se ejecuta completo para el
contexto drum (1) y el contexto sin acción (2). La rama synth (0) queda pendiente.
El harness conserva el código original del cambiador, sincronizador, setter,
factorías y resets. Solo sustituye la entrada L1 `0xffa0bd2c` por un servicio
propio: tras 1–4 llamadas cambia el banco activo e invoca las factorías originales
de audio drum y modulador. Ni ese orden ni ese plazo son comportamiento L1 recuperado.

## Comportamiento observado

En contexto drum, pedir el banco ya seleccionado retorna sin llamar al servicio
ni reiniciar voces. Un cambio escribe el marcador byte `0x01ab314c` a 1,
actualiza la selección `0xff8016d8` y espera con el sincronizador original.
Después recorre el registro de notas retenidas: desocupa cada voz, llama al
dispatcher de reset LFO y al reset del lector, y vuelve a ocuparla. El lector
consulta el mapa voz/pad actual, no el número de nota del registro. Al terminar
pone el marcador a 0. El contexto 2 conserva el marcador, incluso no nulo.

## Contraste

- 2.048 entradas originales: 1.617 cambios, 231 solicitudes del mismo banco y
  200 entradas de contexto 2. Las primeras 448 cubren 8 bancos antiguos ×
  8 destinos × 7 familias LFO.
- 4.305 llamadas al servicio propio y 19.398 resets originales del lector.
  Hay 12.961 callbacks LFO primarios y 11.127 secundarios; cuatro familias
  tienen cuerpos de reset originales vacíos. Estos contadores no son muestras de audio.
- Registros con 0–24 filas: 63 cambios sin filas y 768 con voces repetidas.
  Controles independientes para los 24 pads, gates 0/1/165, modos de lector,
  estados antiguos activos, contadores no nulos y guardas.
- 1.056 recargas completas y 992 casos con retención parcial. El caller siempre
  suministra selecciones/controles, referencias drum, registro, globals de voz,
  gates y estados antiguos del lector; no se atribuye retención a esas regiones.

Cada salida compara 8.796 bytes de ventanas más 128 de trace propio y contador
(8.928 bytes). Dos extracciones independientes coinciden con los fixtures.
Release y ASan/UBSan aprobados; regresiones de sincronización, refresco Core,
factorías LFO y lifecycle drum aprobadas. CI nativo pendiente al registrar el bloque.

[Reproducción y formato](../research/drum-bank-switch/README.md).
[Evidencia](firmware/drum-bank-switch246-validation.json).

## Límites y siguiente frente

Solo entradas válidas, hasta 24 filas; no se exploran las 128 filas de capacidad,
asserts, concurrencia ni cambios físicos durante la espera. Las referencias son
opacas y no se renderiza PCM. El guard de llamadas del harness es propio; el bucle
original no aporta ese timeout. No se ha descifrado ni ejecutado la imagen L1.
Quedan la rama synth, el servicio activo real, scheduler y productores pitch/gain.
Este bloque es investigación aislada: plugin 0.54.0, sin instaladores ni integración
nueva en el producto. Avance ≈70 % ±10 de planificación; acústica sin medir.
