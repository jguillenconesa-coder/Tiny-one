# Core rev246: factorías de moduladores y reinicios reales

Se contrasta la selección/construcción completa para IDs **0, 1 y 2**, tanto en
la ruta synth de seis voces como en drum de doce. El instrumento sigue en 0.54.0;
este contrato de investigación no genera instaladores ni cambia todavía el audio.

## Qué queda recuperado

| Entrada original | Función contrastada |
| --- | --- |
| `0x01967a58` | Factory Core de seis voces. |
| `0x01967ac0` | Factory Core de doce voces. |
| `0x019c99ac` | Selección/construcción del objeto secundario compartido. |
| `0x019c9a24` / `0x019c9ac0` | Selección/construcción de cada voz synth/drum. |

La selección no es solo una tabla de punteros: ejecuta constructores reales y
sus helpers, luego enlaza el secundario y los objetos primarios. El ID 0 devuelve
secundario nulo pero construye **Tremolo común** por voz; no equivale a eliminar
el objeto primario. IDs 1 y 2 construyen Value y Random, con un secundario adicional.

| ID | Familia | Tamaño por voz | Vtable synth / drum | Vtable compartida |
| --- | --- | ---: | --- | --- |
| 0 | Tremolo | 28 | `0x019107fc` / `0x019107fc` | nula |
| 1 | Value | 12 | `0x0191097c` / `0x0191099c` | `0x019109bc` |
| 2 | Random | 44 | `0x0191090c` / `0x0191092c` | `0x0191094c` |

Los primarios empiezan en `0xffb00a00` y avanzan según el tamaño seleccionado;
el objeto compartido empieza en `0xffb00c10`. Ambas rutas reutilizan esas mismas
direcciones. Cambiar de familia puede dejar bytes anteriores fuera de los objetos
recién construidos; no son seis y doce pools independientes en esta ruta.

Core enlaza seis punteros desde `0x01aaad6c` o doce desde `0x01aaad84`, y el
secundario en `0x01aaac44` o `0x01aaac48`. Conserva las entradas de la otra ruta,
selección, enables, controles, displays y RNG en las ventanas observadas. Eso no
hace seguras las antiguas referencias de la otra ruta después de reutilizar el pool.
No se presenta esta factory aislada como el cambio completo de modo del aparato.

## Estado construido y estado preservado

- Tremolo fija ambas fases a `0x80000000`, ambas palabras bajas a cero, gate a
  cero y smooth a cero. Conserva tres halfwords de padding.
- Value fija fase a `0x80000000` y palabra baja a cero, conservando el padding.
- Random pone gate, contador y dos arrays de ocho halfwords a cero; conserva
  el halfword entre gate y contador.
- El reset posterior conserva las reglas ya contrastadas: FREE en Value, umbral
  de gate en Random y reinicio de fases/display/gate/smooth en Tremolo. La
  limpieza de punteros no ejecuta estos constructores ni borra los objetos.

El modelo reutiliza CoreModulatorReset246 y los resets existentes de las tres
familias. El caller original recorre sus vtables reales; no se usan spies,
vtables parcheadas ni sustitutos de los constructores o callbacks.

## Ensayo completo y resultado

**3072 escenarios**, con 1584 cargas independientes y **1488 filas retenidas**:

- 2496 factories completas; 22464 construcciones primarias (7488 por familia).
- 1664 construcciones compartidas de Value/Random.
- 135 limpiezas de la ruta elegida.
- 2611 dispatches de reset solicitados, 2144 aceptados; 2144 cuerpos primarios
  y 1460 secundarios ejecutados (3604 cuerpos en total).
- Rejilla aceptada completa de 432 pares ruta/ID/banco/voz: 3×8×6 + 3×8×12.

Las filas retenidas incluyen 12 cambios de familia y cuatro cambios de ruta sin
recargar las ventanas. No se reinicializa el pool antes de esos cambios.

Se comparan todos los 1068 bytes por fila: globals, tablas de punteros, pool con
capacidad no usada/guardas, ambos bancos de controles y displays/meter/RNG. Se
ensayan memoria cero/FF/A5/variable, cambios de familia y ruta, factory omitida,
reset omitido, gate desactivado y enable no canónico. Los controles firmados
suministrados incluyen FREE y ambos lados del umbral de Random/Tremolo.

Dos ejecuciones originales independientes coinciden con los fixtures; la segunda
los reproduce sin `--record`. Release y ASan/UBSan aprobados. Pasan también las
regresiones de dispatcher de resets, note-on activo y modulación/render retenidos.
[Formato/reproducción](../research/core-modulator-factory/README.md) y
[evidencia](firmware/core-modulator-factory246-validation.json).

## Límites y siguiente bloque

Se cierra la asignación de familias para estas dos factories bajo IDs suministrados
0..2, incluida la construcción Tremolo que antes solo se enlazaba explícitamente.
No recupera la decisión física de UI, el llamador completo del cambio de modo,
las rutas de IDs 3..6 o la llamada activa desde el scheduler protegido. El guard
para otros IDs es del modelo, no un comportamiento original contrastado.

Este corpus compone factory→clear opcional→reset original, sin render de audio.
El corpus previo de modulación/render mantiene sus entradas explícitas y no se
reinterpreta como si hubiera ejecutado estas factories. Siguen pendientes los
productores activos de pitch/ganancia y la fidelidad acústica del instrumento.
Siguiente: ampliar el enlace recuperado a las otras familias y al llamador activo,
conservando explícitas las sustituciones híbridas donde L1 siga protegida.
Avance **≈70 % ±10 de planificación**; presets al final, sin instaladores.
