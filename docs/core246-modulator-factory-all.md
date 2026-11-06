# Core rev246: siete familias de modulación, construcción y reset

Se amplía la reconstrucción de las factories synth/drum a **todos los IDs válidos
0..6**. La entrega anterior cubría 0..2; sus 3072 registros permanecen congelados
y siguen pasando como regresión separada. El instrumento conserva 0.54.0: no se
cambia el audio del plugin ni se generan instaladores.

## Cuatro familias añadidas

| ID | Familia | Bytes por voz | Vtable synth / drum / compartida |
| --- | --- | ---: | --- |
| 3 | Element | 4 | `0x0191089c` / `0x019108bc` / `0x019108dc` |
| 4 | MIDI | 4 | `0x0191082c` / `0x0191084c` / `0x0191086c` |
| 5 | Bend | 4 | `0x0191071c` / `0x0191073c` / `0x0191075c` |
| 6 | Crank | 4 | `0x0191078c` / `0x019107ac` / `0x019107cc` |

Sus constructores escriben solo el tag/vtable. Los primarios empiezan en
`0xffb00a00` con stride de cuatro bytes; el secundario en `0xffb00c10`. No borran
el resto del pool de una familia anterior. Los nueve callbacks de reset de
Element/MIDI/Bend y los tres de Crank son no-op; una nota no reinicia por sí sola
las trayectorias compartidas. Se ejecutan los callbacks originales mediante sus
vtables reales, no sustitutos que simplemente devuelvan sin hacer nada.

La factory Core sí ejecuta una preparación previa para Bend y Crank:

| Familia | Helper original | Escritura observada |
| --- | --- | --- |
| Bend | `0x019c9cf4` | Pone a cero el byte `0xff907ab0`. |
| Crank | `0x019cae08` | Pone a cero el byte `0xff907acd`. |

En la ventana `0xff907aa8..0xff907ad8`, los demás bytes permanecen iguales.
Cada helper se ejecuta 832 veces en este corpus. Es una invalidación puntual;
no se presenta como borrado completo de controlador, historial o movimiento.
Factory se ejecuta con independencia del gate que después permite/rechaza reset.

Para Tremolo/Value/Random se conservan los tamaños, constructores, padding y
reglas de reset del [contrato anterior](core246-modulator-factory.md). Las dos
rutas siguen reutilizando el mismo pool físico y objeto compartido. Preservar
las referencias de la otra ruta no hace seguro usarlas tras cambiar de familia.

## Corpus completo y memoria retenida

**7168 escenarios adicionales** comparan 1116 bytes por fila:

- 3696 cargas independientes y **3472 filas retenidas**.
- 5824 factories completas y 52416 construcciones primarias (7488 por familia).
- 4992 construcciones de objeto compartido (todos salvo Tremolo).
- 316 limpiezas; 6093 dispatches de reset solicitados, 5004 aceptados.
- Rejilla aceptada completa de **1008** combinaciones: 7 IDs × 8 bancos ×
  (6 voces synth + 12 drum).
- 28 cambios de familia y cuatro de ruta sin recargar ventanas, con capacidad
  sobrante, padding, guardas y flags previos retenidos.

Los dispatches aceptados conducen a 5004 cuerpos primarios y 4289 secundarios.
Se separa lo que cuenta cada familia para no llamar procesamiento de señal a un
reset vacío:

| Familia | Cuerpos de reset del corpus |
| --- | ---: |
| Tremolo | 715 |
| Value | 1434 |
| Random | 1428 |
| Element | 1422 (no-op) |
| MIDI | 1432 (no-op) |
| Bend | 1430 (no-op) |
| Crank | 1432 (no-op) |

Son 3577 llamadas a las tres familias con reset no trivial y 5716 a resets
vacíos. No se afirma que toda llamada a Value cambie su fase: FREE la conserva.
Los recuentos siguen el dispatcher contrastado; no son una medición de ciclos ni
prueban la cadencia del aparato físico.

La comparación abarca globals Core, tablas de punteros, pool completo con guardas,
ambos bancos de controles, displays, meter, RNG y la nueva ventana de backend.
El caller suministra explícitamente ID, ruta, banco, enables y orden de llamadas.
Incluye factory/reset omitidos, clear y gates desactivados/no canónicos.

## Validación y siguiente frente

Dos ejecuciones originales independientes coinciden; la segunda exige igualdad
con los fixtures sin `--record`. Las dos suites (corpus previo y nuevo) pasan
Release y ASan/UBSan. También pasan dispatcher de resets, note-on activo y
modulación/render retenidos de batería. [Formato y reproducción](../research/core-modulator-factory/README.md)
y [evidencia](firmware/core-modulator-factory-all246-validation.json).

[CI nativo aprobado](https://github.com/jguillenconesa-coder/Tiny-one/actions/runs/42219816546):
2/2 suites en Linux, Windows/MSVC y macOS Intel sobre `0e0b9c4`, sin build del
producto ni instaladores.

No se ha recuperado aquí el llamador completo del cambio de modo, la selección
física de controles, el scheduler de render o los productores activos de pitch y
ganancia. Los índices inválidos se rechazan en el modelo con una guarda propia;
no se ejecutan las ramas originales de log/error. El ensayo no incluye nuevos
renders de audio o mediciones acústicas.

Queda como siguiente bloque el refresco Core que enlaza audio, FX y moduladores,
y su composición con eventos bajo un estado explícito. Mantener separadas las
rutas legibles del scheduler protegido, sin deducir que un callback no-op o una
factory pequeña resuelve esa frontera. Avance **≈70 % ±10 de planificación**;
presets al final, sin instaladores hasta nueva petición.

La [composición posterior con refresco Core](core246-refresh.md) ejecuta el
llamador legible que enlaza audio, FX y ambas rutas de modulación dentro de un
perfil acotado. El llamador físico y la reactivación previa a render siguen pendientes.
