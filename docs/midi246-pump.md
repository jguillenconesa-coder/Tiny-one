# Bomba y finalización MIDI 246 con endpoint ocupado

Se contrastan las entradas originales de encolado `0x019e0d18`, bomba
`0x01a6a5fc`, submit `0x01a6c134` y dispatcher `0x019e10fc` en perfiles de
evento 9 (finalización) y 16 (reinicio). El ensayo no sustituye código ni instala
trampolines L1. El grafo de objetos y los eventos de finalización son entradas
explícitas; el endpoint está ocupado para impedir activación del driver físico.

## Programación y bomba

El encolador conserva el contrato del [bloque previo](synth246-range-bank-switch.md).
Cuando el contador en vuelo es menor que 1 y quedan words, toma un tramo contiguo
hasta el final del ring, con máximo 32 words. Incrementa el contador en vuelo y
llama a la bomba con la dirección de ese tramo y tamaño en bytes. No revierte
el incremento si la bomba falla. La cola se observa completa antes/después.

La bomba consulta un objeto suministrado mediante `0x018c1e94`. Solo el tipo
exactamente 6 y la habilitación MIDI no nula continúan. El owner suministrado
por `0x01dfe038` tiene dos slots y un selector en offsets 176/180/184. El selector
avanza con `(selector + 1) & 1` antes de comprobar si el slot elegido es nulo.
Un slot nulo retorna −1, conservando ese cambio del selector.

Con un request no nulo, copia los bytes al buffer apuntado por el campo +24,
escribe 0 en +36 y la longitud en +28/+32. Si su endpoint +8 es nulo, el submit
retorna −1: la copia y esas escrituras ya han sucedido. Con endpoint válido se
pone +40 a 0 y se enlaza el request en su lista. La lista/centinela y los requests
se suministran separados; no se ejecuta el asignador original.

El campo +12 del endpoint está explícitamente no nulo: `0x01a6bef8` retorna
antes de activar un request. Por eso este perfil acepta el submit en la cola,
pero no ejecuta driver, DMA o USB físico. La bomba retorna 0 en ese caso y −1
en los retornos anteriores. Solo se comparan sus retornos directos; no se
atribuye una API de resultado a encolado o al dispatcher.

## Consumidor de finalización y reinicio

El evento 9 entra por el dispatcher completo, sin saltar al bloque interior:

1. Incrementa la palabra de estadísticas `0x01ab1b38`.
2. Avanza lectura `0x01ab1b3c` en `bytesCompletados >> 2`, con aritmética u32.
3. Decrementa el contador en vuelo `0x01ab1b40`.
4. Si queda menor que 1 y hay datos, programa el próximo tramo contiguo,
   de nuevo con máximo 32 words, incrementando antes de llamar a la bomba.

Este recorrido no vuelve a consultar el gate del encolador. El resto de
1–3 bytes de una longitud no múltiplo de cuatro se descarta al convertir a words;
son estrés explícito del argumento, no longitudes observadas de hardware.

El evento 16 pone a cero estadística, lectura, escritura y contador en vuelo,
y deja el gate byte a 1. Conserva el contenido del ring y los bytes vecinos
del gate. Los dos words de metadata adicionales del callback se observan en
`0x018b6068/6c`; su significado físico no se infiere.

## Corpus y comprobaciones

4.096 casos independientes con todas las ventanas recargadas. Acciones:
encolar, bomba directa, finalización, encolar+finalización y reinicio. Los
primeros 32 casos fuerzan cada tamaño contiguo 1–32. Las copias directas cubren
cada longitud 0–128 bytes y los dos buffers alternos.

- 2.063 llamadas a bomba; 42.618 bytes copiados y 387 submits aceptados en lista.
- 189 rechazos por endpoint nulo, 1.024 por tipo, 252 por deshabilitación y
  211 por slot nulo.
- 29.007 words encolados y 166 lotes rechazados por capacidad.
- 1.626 finalizaciones, 813 reinicios y 1.251 programaciones.
- 72 wraps de lectura y 31 de escritura u32, además de cruce del final del ring.

Se comparan 5.088 bytes de memoria y una observación propia de retorno directo
(5.092 por salida): ring/contadores/gate, contexto, owner, requests, endpoint,
buffers completos, payload y metadata del callback. Guardas y colas alternan
cero/FF/A5/patrones variables. Las operaciones contadas siguen el modelo
contrastado; no equivalen a bytes transmitidos o finalizaciones de hardware.

Dos extracciones originales independientes coinciden. Release y ASan/UBSan
aprobados, además de las regresiones del cambiador range, perfil retenido y
sincronizador. Se calibran las watches PC con un programa propio: no se activan
en la entrada al driver `0x01a6bf26..0x01a6bf53`, asserts ni rango L1. [CI nativo](https://github.com/jguillenconesa-coder/Tiny-one/actions/runs/43508701953): Linux x64 y Windows/MSVC x64 aprobados; macOS en cola.
[Formato/reproducción](../research/midi-pump/README.md) y
[evidencia](firmware/midi-pump246-validation.json).

## Límite de la composición

Los requests están separados de la lista al entrar y no se reutilizan después
de encolarlos. El endpoint ocupado evita la ejecución de driver. Las
finalizaciones son argumentos suministrados; no recuperan la ISR, el ritmo de
USB, concurrencia ni el ciclo completo de propiedad/liberación de requests.
CLI/STI se ejecuta en el simulador, no constituye sincronización del host.

Este bloque es investigación aislada. El plugin 0.54.0 sigue declarando
`producesMidi=false` y `NEEDS_MIDI_OUTPUT FALSE`: no hay nueva salida MIDI del
host ni render DSP. Siguiente frente: adaptar eventos y consumidor al host,
manteniendo explícito su reloj, y continuar con el servicio activo de voz.
Sin instaladores. Avance ≈70 % ±10 de planificación; fidelidad acústica pendiente.
