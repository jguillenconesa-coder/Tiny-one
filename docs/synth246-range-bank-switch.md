# Notas fuera de rango y cola MIDI original — rev246

Se compone el cambiador synth completo con eliminación de notas que, tras el
delta × 12, salen de 0…127. El grafo sigue siendo Dr. Wave (tipo 2), con seis
ADSR inicializados y auxiliares ausentes. Las dos respuestas L1 de selección
siguen siendo propias. El resto del recorrido ejecutado permanece original:
cambiador, dispatcher de liberación, compactación, refresco, productores MIDI
y encolado. No se modifica Source ni el plugin.

## Eliminación y reactivación

El registro suministrado tiene hasta 128 filas y notas iniciales válidas. Cuando
una fila de metadata no nula queda fuera de rango, el cambiador llama a
`0x01a85840`, que en contexto synth dirige a `0x01a85058`. Se usan los
[helpers ya contrastados](drwave246-release-selectors.md) de liberación. La búsqueda
se hace por nota, no por identidad de la fila. Con duplicados, el selector y
la coincidencia de la última nota determinan qué entrada se quita.

La compactación conserva los bytes de padding 9–11 del destino y no borra el
slot final antiguo. El cambiador vuelve a evaluar su posición con el contador
actualizado. Los selectores del banco antiguo son 0–3 en cambios efectivos;
el selector 4 no se usa para eliminación. El refresco posterior consulta los
controles del destino, cuyos selectores son 0–4. Se incluyen cambios del RNG y
pitch al volver a la nota anterior durante una liberación, antes del refresco.

El modelo anterior de notas retenidas conserva su perfil público: rechaza la
eliminación y exige salida MIDI deshabilitada. Un adaptador genérico propio
permite componer las callbacks nuevas. Su bool y guardas no son retornos ni
semántica de error originales. Una comprobación propia exige que cada eliminación
reduzca el contador; no se atribuye al bucle original como timeout/protección.

## Productores y cola, sin envío físico

Los helpers `0x019e2dc0` y `0x019e2e64` actualizan el bitmap de notas. Con
`0x01dfe03c` no nulo preparan eventos de cuatro bytes en `0x01aadb20 + 24/28`
y escriben el byte pendiente `0x018b600d` a 1:

| Evento | Bytes del evento interno |
| --- | --- |
| Off | `08`, `80 OR canal`, nota, `40` |
| On | `09`, `90 OR canal`, nota, byte bajo de velocidad |

El canal suministrado recorre 0–15. La velocidad se decide como palabra u32
completa: exactamente cero se dirige a Off. Los words de estrés pueden producir
un byte de velocidad fuera de 0…127; no se afirma validación general del protocolo. Los encendidos duplicados conservan
el byte activo sin producir otro evento; no se normalizan los bytes no canónicos.

El encolador original `0x019e0d18` usa un gate byte en `0x01ab2b48`. Con gate no
nulo, intenta añadir un word en el ring de 1.024 words de `0x01ab1b44`. Lee el
contador de lectura `0x01ab1b3c` y el de escritura `0x01ab2b44`. Acepta cuando
`1 < lectura + 1024 − escritura` con aritmética u32: conserva un hueco, así que
el máximo ocupado ensayado es 1.023. Si acepta, escribe en `escritura & 1023`
y aumenta el contador con wrap; si rechaza, conserva ring/contador. El bitmap,
evento preparado y pendiente no se revierten por falta de espacio.

**El contador en vuelo `0x01ab1b40` se suministra siempre positivo (1/2/165).**
Así el código original no inicia la bomba DMA ni llama a `0x01a6a5fc`. No hay
consumidor, dispositivo, paquetes enviados, transporte de Ableton ni validación
de latencia. CLI/STI se ejecutan en el simulador; el modelo C++ aislado no es
una implementación concurrente de la cola del host.

## Evidencia

2.048 entradas independientes: 1.792 cambios, 256 retornos por mismo banco,
las 64 parejas, listas 0–128 e índices de voz −2…5. Todas las ventanas se recargan.

- 8.183 eliminaciones y 10.828 pares Off/On de traducción válida.
- 3.297 eventos Off y 1.744 On preparados con salida habilitada.
- 418 encolados, 620 rechazados por capacidad y cuatro wraps del contador u32.
  Los restantes eventos no entran en la cola por gate deshabilitado.
- 560 entradas a refresco, 6.063 resets de Dr. Wave, 10.752 liberaciones rápidas
  y 4.480 respuestas L1 propias.

Cada salida compara 8.324 bytes de ventanas, 96 de trace propio y contador
(8.424 bytes). El ring/guardas alterna cero, FF, A5 y patrones variables. Se
incluyen contadores próximos a UINT32_MAX, posiciones que cruzan el final del
ring, ocupaciones 0/1/1020/1022/1023 y gates 0/1/165. Los contadores de operaciones
siguen el recorrido del modelo contrastado; no se captura una historia externa
de paquetes físicos. Se comparan toda la memoria de cola y los buffers de evento.

Dos extracciones originales independientes coinciden. Release y ASan/UBSan
aprobados, junto con diez suites de regresión; el perfil anterior también se
recompila y pasa con sanitizadores. [CI nativo aprobado](https://github.com/jguillenconesa-coder/Tiny-one/actions/runs/43504787812) en Linux x64, Windows/MSVC x64 y macOS ARM64 sobre `cc79266`.
[Formato/reproducción](../research/synth-range-bank-switch/README.md),
[evidencia](firmware/synth-range-bank-switch246-validation.json).

Límites pendientes: bomba/consumidor MIDI y transporte físico, modo de módulo 4,
auxiliares, inicialización perezosa, servicio L1 real, scheduler y productores
activos pitch/gain. No se renderiza audio ni cambia el plugin 0.54.0.
Sin instaladores. Avance ≈70 % ±10 de planificación; fidelidad acústica sin medir.


Actualización: [bomba y callback de finalización con endpoint ocupado](midi246-pump.md)
contrastados por separado; el driver físico y el ciclo de propiedad siguen pendientes.
