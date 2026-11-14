# Dr. Wave: reinicio polifónico y entrega de registros — 0.57.0

Al agotar las seis voces polifónicas, Dr. Wave conserva ahora el estado de la
víctima mientras ejecuta la rampa ADSR de reinicio contrastada. Después entrega
pitch y velocidad pendientes al render. Sustituye el reinicio inmediato y el
relevo de 3 ms de versiones anteriores **en el robo polifónico de Dr. Wave**.
Mono/Legato y los demás motores conservan sus políticas anteriores.

## Parte contrastada y servicio propio

La [rama original ocupada](drwave246-poly-steal.md) elige el mayor contador
ADSR interpretado como signed32, partiendo de −1 y conservando la primera voz
en los empates. `Voice246PolySelection.h` extrae ese mismo escaneo para compartirlo
entre los modelos con fixtures originales y el producto. Un resultado −1
identifica el dominio de assert excluido; el host usa en ese caso su fallback
seguro de voz más antigua, sin emular el assert original.

La [solicitud ADSR](envelope246-research.md) escribe etapa 6, contador cero,
pitch pendiente en `field12` y velocidad en el halfword alto de `field16`,
conservando nivel y ganancia. El kernel contrastado desvanece durante 128 muestras
internas y deja etapa 7. La siguiente ejecución vuelve al ataque. La pendiente
entera puede dejar un residuo de nivel entre 0 y 127; no se afirma cero exacto.

El consumidor L1 y su temporización **siguen sin recuperarse**. Este servicio es
una adaptación explícita del host:

1. La asignación cambia el propietario nota/canal y pone a cero su edad de
   selección. Conserva el DSP y espera al siguiente bloque interno de Dr. Wave.
2. En ese límite aplica la solicitud original. El bloque de rampa conserva el
   pitch, volumen y ganancia anteriores. Los eventos pendientes se almacenan
   sin reservar memoria.
3. Al encontrar etapa 7 en el límite siguiente, copia `field12` al proveedor de
   pitch y el halfword alto de `field16` al volumen ADSR. Conserva la ganancia
   anterior, como el helper ocupado contrastado. Reinicia Core/ondas/runtime y
   su contador de refresco propio, conservando filtros, bloqueador DC y SRC.
4. El render aplica sus moduladores después del proveedor de pitch; continúa
   usando el reloj híbrido de 88,2 kHz. La palabra de pitch procede de la
   conversión A440 del host: no es recuperación del productor L1 original.

La espera puede incluir el resto del bloque actual y otro bloque de rampa,
aproximadamente hasta 256 muestras internas, además del adelanto del SRC.
No se atribuye esa temporización al hardware ni se mide latencia física.
El control de pitch normal vuelve a actualizarse por bloque tras la entrega.
Value/Random se notifican al reasignar la voz; la cadencia de modulación es propia.

## Eventos que llegan durante la espera

- Varios destinos antes de un límite conservan el último. Una petición nueva
  antes de entregar la anterior vuelve a solicitar la rampa en el límite: no
  dispara primero el destino sustituido.
- Note-off del propietario anterior no libera al nuevo. Canales y sustain se
  resuelven con la política actual del host.
- Si se libera el destino nuevo antes de entregarlo, se cancela el ataque y se
  libera el pitch anterior. No se lo reemplaza por el pitch cancelado.
- Panic, restauración, cambio de motor o modo vacían las voces y las peticiones.
- Las asignaciones sin audio previo arrancan directamente: no hay sonido anterior
  que desvanecer. La inicialización diferida original completa no se reproduce.

No se introduce un parámetro ni se guarda estado transitorio nuevo. No se amplía
esta política a Mono/Legato ni a Cluster. La composición de lista, duplicados,
retirada, servicio, reloj y salida permanece híbrida; usar el escaneo original
sobre nuestros contadores no demuestra equivalencia de toda la asignación.

## Validación

[Evidencia](releases/v0.57.0-validation.json): oráculos originales de robo y pitch
explícito conservados, con 768 y 4.552 registros. No hay extracciones originales
nuevas. La rampa del producto se compara con el modelo ADSR contrastado; se
comprueban entrega de palabras, conservación de ganancia, reset Core al entregar,
reemplazo, cancelación, primer empate y contadores signed.

Seis streams Engine ejercitan tres tasas con cancelación temprana y robo denso.
El procesador real compara nueve escenarios a 44,1/48/96 kHz en callbacks 64/257:
el audio debe coincidir bit a bit. Otros nueve A/B retiran el note-off del dueño
anterior y exigen audio idéntico. All Sound Off durante una petición pendiente
exige silencio exacto posterior y ninguna voz resucitada.

Sanitizadores, regresión completa y CI se registran por separado. No hay cambios
de UI ni instaladores. Ableton real, transporte físico y fidelidad acústica del
instrumento completo continúan pendientes.
