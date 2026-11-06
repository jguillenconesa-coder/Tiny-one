# Core rev246: sincronizadores de selección y frontera de reactivación L1

Se contrastan completas las rutinas exteriores `0x01967e90..0x01967ed0` (drum)
y `0x01967f68..0x01967fcc` (synth). **Esperan igualdad de bancos; no reconstruyen
por sí mismas los objetos.** El servicio invocado en L1 sigue sin recuperarse.
El plugin permanece en 0.54.0, sin cambios audibles ni instaladores nuevos.

## Protocolo exterior recuperado

| Dato | Synth | Drum |
| --- | --- | --- |
| Banco activo Core | `0x01aaac0c` | `0x01aaac10` |
| Banco seleccionado | `0xff8016d4` | `0xff8016d8` |
| Si ambos son iguales | Retorna sin llamar a L1. | Retorna sin llamar a L1. |
| Mientras difieren | Relee flag `0xff801040` en cada vuelta y llama a una de dos entradas. | Llama a la entrada común en cada vuelta. |

Para synth, el valor **exactamente 1** del flag escoge `0xffa017ec`, con
R0/R1/R2 explícitamente puestos a cero. Los demás valores escogen `0xffa0bd2c`,
con R0=1 y R2=0. Para drum se usa siempre `0xffa0bd2c`, con R0=1; el bucle
exterior no pone a cero R2. En la entrada común, R1 contiene la selección recién
leída. Se documentan valores de registros, **no el significado de parámetros
de las funciones L1**, cuya ABI completa sigue desconocida.

Después de cada llamada vuelve a leer banco activo y seleccionado. Si el destino
cambia durante el servicio, espera al destino nuevo; no conserva una copia del
banco inicial. Synth también puede cambiar de entrada L1 si cambia el flag durante
los reintentos. Si ya está listo al entrar, no ejecuta el borrado de R2 de esa
rama ni solicita servicio, incluso cuando el flag vale 1.

No hay timeout en estas rutinas legibles. El ensayo suministra respuestas finitas;
no mide cuánto tarda el aparato ni ensaya un servicio que nunca converja.
El límite de callbacks del harness/modelo es propio y no se atribuye al firmware.
No se integra una espera bloqueante en el callback de audio del host.

## Qué ejecuta el oráculo y qué sustituye

Se ejecuta todo el cuerpo original de ambos sincronizadores, sin parchearlo.
En las dos direcciones L1 se colocan **trampolines escritos para el ensayo**, que
registran argumentos/estado y aplican una respuesta programada: bancos activos,
selecciones, flag y R2. Clobber de R0/R1 permite comprobar que se recargan los
valores de banco tras la llamada. No se carga ni ejecuta la imagen L1 original.

Estas respuestas no construyen audio, FX o moduladores; no equivalen a los
servicios originales. Los resultados validan el protocolo exterior bajo esas
respuestas explícitas. No prueban que el aparato pueda reactivar una parte con
solo escribir sus índices. La [construcción global previa](core246-refresh.md)
y sus pools compartidos conservan sus límites; este ensayo no la reinterpreta
como activación de la parte que va a sonar.

## Cobertura y resultados

**4096 escenarios / 8192 entradas originales completas**, invocando cada rutina
dos veces por escenario:

- Rejilla de ambas rutas y 0..6 respuestas, con todas las combinaciones presentes.
- 586 escenarios ya listos al inicio y 4096 repeticiones ya listas: **4682 entradas
  sin callbacks**.
- 12285 respuestas programadas: 6141 drum, 6144 synth; 11058 en la entrada común
  y 1227 en la especial.
- 8190 cambios de selección durante respuestas y 4095 respuestas con selección
  conservada; 6143 cambios de flag y 6142 conservaciones.
- Valores del flag 0, 1, 2, −1 e INT32_MIN: valores no canónicos no se confunden
  con el caso especial exactamente 1.

Cada fila compara 392 bytes: las tres ventanas/guardas, seis slots de trazado
con guardas y veneno en los slots no usados, contador y observaciones R0/R2 al
volver de ambas llamadas. Los traces son estructuras propias del harness.
No se presenta R0 como una API pública original de retorno de banco ni el bool
del modelo como retorno original; R2 se observa como registro temporal del caller.

Dos ejecuciones originales independientes coinciden con los fixtures; la segunda
los reproduce sin `--record`. Release y ASan/UBSan aprobados, junto con regresiones
de refresco Core, lifecycle de bancos drum y las dos suites de factories.
[Formato/reproducción](../research/core-selection-sync/README.md) y
[evidencia](firmware/core-selection-sync246-validation.json).

[CI nativo aprobado](https://github.com/jguillenconesa-coder/Tiny-one/actions/runs/42238923574)
en Linux, Windows/MSVC y macOS Intel sobre `a59e95e`. Sin build del producto.

## Relación con el cambio de banco y siguiente bloque

En el tramo legible de cambio de banco drum se observa una llamada en
`0x01a865b0` a este sincronizador, antes del recorrido que reinicia las notas
retenidas (dispatcher LFO y reset de lector). Esa vecindad es evidencia estática;
no se ha ejecutado aquí el cambiador exterior completo. Su punto L1 pendiente
no se reemplaza por una secuencia deducida del refresco global.

Queda por recuperar el trabajo de `0xffa0bd2c` / `0xffa017ec`, la reconstrucción
de la parte activa y el consumidor normal previo a render. El siguiente bloque
puede componer el cambiador legible y la reactivación de notas retenidas con esta
frontera explícita, manteniendo propio el servicio que no se haya recuperado.
Scheduler, productores activos pitch/gain, acústica y Ableton real siguen
pendientes. Avance **≈70 % ±10 de planificación**, presets al final y sin
instaladores hasta nueva petición.


Actualización: el [bloque posterior drum](drum246-bank-switch.md) ejecuta ya
el cambiador exterior en contextos 1/2 con servicio L1 propio explícito.
