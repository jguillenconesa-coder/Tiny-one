# Cambiador de banco synth 246 con registro vacío

El cambiador original completo `0x01a86420..0x01a86660` se ejecuta en contexto
synth (0), registro de notas retenidas vacío y tipos de módulo suministrados 0–3.
El código legible del cambiador y sus helpers queda intacto. Las entradas L1
`0xffa0bd2c` / `0xffa017ec` son trampolines propios: tras 1–4 llamadas copian el
banco seleccionado al activo, sin reconstruir voces ni llamar a factorías.

## Comportamiento

Pedir el banco ya seleccionado retorna conservando marcador no nulo, envolventes
y transposición. Un cambio activa el marcador byte `0x01ab314c`. Si el word de
inicialización `0x01dfe4d8` es cero, escribe 1 y copia el byte de estado synth
`0x01ab3141` al flag diferido `0x01dfe4dc`.

Reinicia las seis envolventes de 24 bytes en `0xff8000d4`: escribe estado 8 y
word 0 en los primeros ocho bytes; conserva los otros 16. Calcula el delta entre
los words de destino y banco antiguo en `0x01dfe4e0`. Con delta no nulo y flag
diferido no nulo lo suma al acumulador `0x018b6208`, módulo 32 bits.

Escribe la selección synth `0xff8016d4` y llama al sincronizador original.
Solo el flag exactamente 1 en `0xff801040` usa la entrada especial L1. Si el
flag diferido es cero, entra después en el helper de reactivación. El registro
vacío impide eventos de notas. Al volver borra el marcador.
La traducción inmediata también se recorre con registro vacío: no se invoca
note-off/on ni se modifica una nota; no se extrapola al caso con notas presentes.

## Contraste

- 2.048 entradas: 1.792 cambios y 256 retornos por mismo banco; las 64 parejas.
- 4.480 llamadas L1 propias, 896 inicializaciones de transposición y 893
  actualizaciones del acumulador diferido; 10.752 resets de envolvente.
- Cada salida compara 476 bytes de ventanas, 96 de trace propio y contador.
  Todas las filas recargan ventanas. Acumuladores 0/UINT32_MAX/INT32_MIN/INT32_MAX,
  deltas positivos/negativos/cero, flags no canónicos y guardas.
  La tabla se precarga directamente con words de prueba (incluidos valores fuera
  de −4…4); este bloque no ejecuta ni amplía el dominio del setter original.

Dos extracciones originales independientes coinciden. Release y ASan/UBSan
aprobados, junto con regresiones drum y del sincronizador Core. [CI nativo](https://github.com/jguillenconesa-coder/Tiny-one/actions/runs/43464759166): Linux, Windows/MSVC y macOS Intel aprobados.
[Formato y reproducción](../research/synth-bank-switch/README.md) y
[evidencia](firmware/synth-bank-switch246-validation.json).

## Límites

Servicio L1, orden y plazo propios; no se ejecuta ni recupera su payload protegido.
Registro vacío y flag de envolventes inicializadas: no cubre inicialización
perezosa, tipo de módulo 4, notas retenidas, scheduler, PCM ni productores
activos pitch/gain. Siguiente frente: traducción/reactivación con notas y frontera
MIDI, antes de integrar un flujo equivalente en el plugin.
Plugin 0.54.0 sin cambios ni instaladores. Avance ≈70 % ±10 de planificación;
fidelidad acústica y Ableton real pendientes.


Actualización: [registro de notas retenidas con Dr. Wave](synth246-held-bank-switch.md)
contrastado en el bloque posterior; la eliminación fuera de rango sigue pendiente.
