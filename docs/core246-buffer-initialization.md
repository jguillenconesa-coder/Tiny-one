# Core rev246: inicialización de buffers compartidos y frontera de dinámica

Se contrasta `0x01a3b34c..0x01a3b3dc`, referenciada por la palabra de arranque
`0x0100033c`. Es un inicializador separado del Core amplio y del workspace ya
estudiados. El instrumento permanece en 0.54.0; no hay DSP nuevo ni instaladores.

## Contrato observado

| Ventana/dato | Resultado del inicializador |
| --- | --- |
| `0x01df4df4` | Escribe int32 `0x00a3d700`. |
| `0x01df4df8` | Escribe int16 `0x4000`, conserva los dos bytes siguientes. |
| `0xff801950` | Cero en 128 palabras int32 de pitch. |
| `0xff801b50` | Cero en 128 palabras int32 de ganancia derivada. |
| `0xff801d50` | Cero en 128 halfwords de ganancia. |
| `0xff801e50` | Cero en 128 halfwords de entrada compartida. |
| Historia Core `0x01aaac14` y coeficiente `0x01bab53c` | Conservados en las ventanas observadas. |
| Salida `0xff800bb4` y guardas adyacentes | Conservadas por el inicializador. |

La rutina original llama al constructor de vectores `0x01aa3862` con callbacks
originales que ponen a cero una palabra (`0x01a3b308`) o un halfword
(`0x01a3b310`). El ensayo ejecuta todo ese código sin sustituciones. Su orden
respecto al kernel posterior es explícito del harness; el puntero de arranque no
demuestra que el aparato llame a esta rutina durante notas, pánico o cambios.
Los seis bytes constantes no se etiquetan con un control comercial no probado.

## Composición y consecuencia práctica

**1024 escenarios**: 528 cargas independientes y 496 filas con las cinco ventanas
retenidas. Incluyen 512 inicializaciones y 512 callbacks del kernel Core de
64 muestras, 256 de ellos inmediatamente después del inicializador. Son
**32768 muestras de salida del kernel**, no muestras de voces de batería.
También se prueban solo init, solo kernel y ninguna llamada; los buffers/estados
retenidos deben permanecer exactamente iguales cuando corresponde.

En **64 filas compuestas**, el kernel aún produce salida no nula después de
poner a cero su entrada. Su historia previa se conserva y puede responder al
silencio. Esto impide usar el borrado de buffers como una prueba de pánico o
reset completo del instrumento. No se propone cambiar el pánico actual del host.

El comparador verifica 1760 bytes por fila: defaults/guardas, cuatro buffers y
sus guardas externas, historia/guardas, coeficiente/vecinos y salida/guardas.
El alcance es de ventanas observadas; no se afirma un trazado de todas las
escrituras de la máquina. Tres ejecuciones originales independientes coinciden
con los fixtures, dos de ellas sin `--record`; Release y ASan/UBSan aprobados.
También pasan Core signal/auditor, inicialización Core amplia y modulación/render
retenidos de batería. [Formato](../research/core-buffer-init/README.md) y
[evidencia](firmware/core-buffer-init246-validation.json).

CI nativo nuevo y regresión Core/auditor aprobados en Linux, Windows/MSVC y
macOS Intel sobre `8c4d588`: [inicializador](https://github.com/jguillenconesa-coder/Tiny-one/actions/runs/42213365129)
y [Core/auditor](https://github.com/jguillenconesa-coder/Tiny-one/actions/runs/42213365053).
No se activa una compilación nueva del producto.

## Auditoría ampliada y productores pendientes

El [inventario ampliado](firmware/core-buffer246-reference-audit.json) cubre 22
direcciones, frente a las 13 del inventario anterior. Añade los tres buffers de
pitch/ganancia, workspace, inicializador, defaults y helpers. Registra llamadas
relativas inmediatamente posteriores a pares low/high y high/low; no confunde
la dirección cargada con el destino de esa llamada. Dieciséis pruebas sintéticas
comprueban signo, alineación, alias de mitad baja, límites y lecturas protegidas.
Dos inventarios independientes coinciden en sus hallazgos.

| Hallazgo | Alcance |
| --- | --- |
| Pares SDRAM hacia pitch/gain/derivada: 19/9/9 | Candidatos estáticos; no se clasifican todos como productores. |
| `0x01a3b380`, `0x01a3b394`, `0x01a3b3b0` | Pares seguidos de llamadas a `0x01aa3862`; quedan contextualizados por el inicializador completo contrastado. |
| `0xffa00700` | Par visible hacia gain `0xff801d50`; el tramo siguiente `0xffa00708..0xffa00710` está protegido. No hay productor completo. |
| `0xffa043c2` | Mitad baja compatible con derivada `0x1b50`; la alta cruza protección. Dirección completa desconocida. |

El auditor nunca decodifica bytes protegidos. Mantiene la suposición comunitaria
de ocho bytes protegidos por grupo de 24 para el único bloque L1 inventariado.
No ejecuta L1 ni prueba alcanzabilidad de sus candidatos; no encontrar una pareja
completa de pitch en este alcance no demuestra ausencia de consumidores.

El siguiente frente continúa siendo los productores activos de pitch/ganancia y
el ruteo de voz protegido. Los ceros de arranque no se usan para rellenar esas
dependencias. Esta entrega amplía contratos contrastados, sin nueva integración
sonora, medición acústica o validación de Live real. Avance **≈70 % ±10** de
planificación, presets al final y sin instaladores hasta nueva petición.
