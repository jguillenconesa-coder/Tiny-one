# Core rev246: despacho de reinicios de moduladores

Bloque del 6 de noviembre de 2026. Se contrastan los dispatchers completos Core
de seis y doce voces, sus funciones de limpieza y cinco resets originales de
Value, Random y Tremolo. Se cierra la frontera de punteros auxiliares nulos en
este contrato aislado; no se amplía por ello la evidencia de los note-on completos
anteriores. El plugin conserva **0.54.0**, sin cambios DSP ni instaladores nuevos.

El contexto visible de la referencia L1 al pitch no permite completar su
consumidor a través de las franjas protegidas. Este frente ejecutable en SDRAM
avanza la composición de eventos/moduladores sin inventar ese productor.

## Contrato de las dos rutas

| Dato | Ruta de seis voces | Ruta de doce voces |
| --- | --- | --- |
| Dispatcher completo | `0x019673dc..0x0196745a` | `0x0196735e..0x019673dc` |
| Limpieza completa | `0x0196745a..0x0196747c` | `0x01967330..0x0196735e` |
| Tabla de punteros primarios | `0x01aaad6c[6]` | `0x01aaad84[12]` |
| Sonido seleccionado | `0x01aaac0c` | `0x01aaac10` |
| Bytes de activación | `0x01aaac28[8]` | `0x01aaac38[8]` |
| Puntero secundario | `0x01aaac44` | `0x01aaac48` |
| Ocho bancos de controles | `0xff8014c4 + sonido × 16` | `0xff801644 + sonido × 16` |

Todos los rangos terminan en la dirección indicada, excluida. El índice de voz
elige el puntero primario. Si es nulo, no se llama tampoco al secundario. Si no
es nulo pero el byte de activación del sonido seleccionado vale cero, la ruta
se conserva igualmente sin llamadas. Se prueban activaciones no canónicas
1/127/255 además de cero.

Al activarse, Core invoca el callback de vtable `+0x1c`, ajustando `this` con el
int16 firmado de `+0x18`. Los controles son ocho int16 del banco seleccionado.
Después lee el puntero secundario y, si no es nulo, vuelve a leer la selección
para calcular sus controles. **No vuelve a comprobar la activación.** El segundo
reset puede recibir un banco diferente si el primer callback modifica la
selección. La mutación se provoca deliberadamente mediante un endpoint propio;
no se atribuye a los resets Value/Random/Tremolo ese comportamiento comercial.

Las limpiezas ponen a cero solamente su tabla de seis/doce punteros y su
puntero secundario. Conservan selección, activaciones, la otra tabla, los objetos
de modulación y los demás bytes observados. No ejecutan destructores, no reinician
esos objetos y no deben usarse como pánico o retirada de audio del host.

## Cuerpos originales ejecutados

Los endpoints de trazado propios conservan R0/R1, registran receptor/controles/
selección y saltan al cuerpo original completo, sin reemplazar su aritmética:

| Perfil suministrado | Reset original |
| --- | --- |
| Value synth | `0x019cfd5a` |
| Value drum | `0x019cff16` |
| Random synth | `0x019ced74` |
| Random drum | `0x019cef66` |
| Tremolo | `0x019cbdca` |

Se comprueban fase libre de Value, umbral 14744/14745 de profundidad, padding,
estado previo y display compartido de Tremolo. RNG y vecino se conservan.
Las funciones de callback se enlazan explícitamente: no se ejecuta la factory
original ni se demuestra qué tipo asigna a cada ranura primaria o secundaria.
Los ajustes de `this` -16/-8/0/8/16 usan proxies de ensayo acotados; incluyen
superposición del vptr del proxy con datos del receptor. No se infiere por ello
un layout original de herencia múltiple para esas clases.

## Oráculo y modelo

**8.192 casos**, 4.096 por ruta. Cubren todas las combinaciones válidas de
voz, sonido y perfil primario, presencia/ausencia de punteros y activación,
482 limpiezas y 957 mutaciones sintéticas de selección que alcanzan un callback.

- 3.379 casos sin llamadas.
- 796 con una llamada.
- 4.017 con dos llamadas.
- **8.830 resets originales** en total: 1.764 Value synth, 1.769 Value drum,
  1.774 Random synth, 1.768 Random drum y 1.755 Tremolo.

Se comparan **732 bytes por caso**: Core/vecinos (68), tablas/vecinos (84),
dos objetos proxy completos (256), display Tremolo (32), ambos bancos completos
de controles (256), RNG/vecino (8) y trazas propias (28). Las trazas no son
instrucciones o resultados atribuidos al firmware: observan los argumentos que
el dispatcher original entrega a los endpoints. No se compara un supuesto
retorno booleano de estos callbacks void.

[CoreModulatorReset246.h](../research/core-modulator-reset/CoreModulatorReset246.h)
reconstruye el despacho/limpieza, con invocación suministrada. El comparador usa
los modelos aritméticos Value/Random/Tremolo ya contrastados para los cuerpos.
El estado posterior se calcula desde las entradas; no se importa del oráculo.
Guardas de índices inválidos son propias y quedan fuera del dominio original.

Dos extracciones independientes coinciden, con hash del firmware y de las
herramientas y probes de los tres arreglos writeback previos. Release y
ASan/UBSan completos aprobados; leaks desactivados por ptrace del entorno.
CI nativo: 1/1 suite aprobada en Linux, Windows/MSVC y macOS Intel.
[Formato y reproducción](../research/core-modulator-reset/README.md) ·
[Evidencia y hashes](firmware/core-modulator-reset246-validation.json).
No se distribuyen firmware, ELF, desensamblado o pseudocódigo original.

## Límites y siguiente composición

No hay render de audio, reloj de modulación, entrada física, productor de buffers
pitch/gain o medición acústica nuevos. No se ejecuta aquí el note-on exterior:
los estudios anteriores que suministraban auxiliares nulos conservan ese límite.
Tampoco se enlaza este modelo al plugin ni se cambia su política de prioridad,
FREE, canales, persistencia o captura manual.

La siguiente composición es note-on completo → dispatcher activo → reset real
→ reset del lector, conservando las palabras de evento y comprobando el orden
respecto a selección, lista retenida y ocupación. El consumidor de pitch visible
en L1 y la dinámica original siguen pendientes. No se sustituyen por este reset.
Avance **≈70 % ±10 de planificación**, sin aumentar por número de llamadas.

## Composición posterior

La [cadena activa de note-on y lector](drum246-active-note-chain.md) ya ejecuta
el note-on exterior con estos resets. Añade observación de orden y selección;
no amplía este corpus aislado ni recupera render, reloj o productores de buffers.
