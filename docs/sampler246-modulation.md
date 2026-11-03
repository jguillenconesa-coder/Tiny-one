# Modulación del sampler 246 — 0.37.0

SAMPLER 246 admite **Value Voice, Random y Tremolo** en set 4. Value y Random
pueden modular Instrument o los cuatro controles ADSR. Para escuchar el destino
ADSR hay que activar **246 ENVELOPE** en set 2; la envolvente propia no se convierte
en una envolvente original al seleccionar ese destino.

Value Instrument tiene **ocho destinos**. El banco seleccionado en set 1 decide
qué cuatro colores se editan: Start/In/Out/End, o Reverse/Fine In/Fine Out/Volume
con FINE / GAIN. La página de Value indica START / LOOP o FINE / GAIN. El banco no
cambia los cuatro destinos ADSR. Random recorre los ocho campos en orden; las
señales 5–8 **no** repiten las primeras cuatro divididas por 16 como en otras
familias synth. Tremolo modifica pitch y volumen antes de la adaptación de salida.

Los controles base no se sobrescriben con los resultados del LFO. Los parámetros
existentes se guardan en proyecto, slots y clipboard y los proxies MIDI funcionan
sin editor. No se añaden nuevos IDs de automatización. La prioridad común sigue
siendo Random, Tremolo, Value Voice, Value FX y LFO propio. Seleccionar otra opción
en la interfaz desactiva las anteriores; flags simultáneos del host siguen esa
prioridad. El lector anterior, con SAMPLER 246 apagado, mantiene su LFO propio.

## Qué se ha contrastado

El sampler synth es tipo **6** en las tablas. Value `0x019cfaf8` y Random
`0x019cea70` utilizan el helper de regiones `0x01a1dfec` y los descriptores de
`0x01de5a10`. No basta con copiar los clamps de otros motores: estos descriptores
seleccionan reglas acopladas entre inicio, final, puntos de loop y sus fracciones.
Las operaciones usan saturación entera y Q16 para ciertas relaciones de loop;
no son trims relativos a la longitud del archivo.

Se comparan **4096 llamadas de Value y 4096 de Random** contra instrucciones
originales ejecutadas en GNU BF524, con los tres parches de writeback documentados.
Cada corpus incluye 2048 casos independientes y 2048 con estado retenido, reset
periódico, destinos aceptados/rechazados y controles con signo. Se alternan regiones
correlacionadas —igualdades, span cero, límites y fracciones— y palabras arbitrarias.

Coinciden todos los bytes de estado, displays, ocho destinos y retornos; además
RNG compartido en Random y cálculo de tempo en Value. Los ocho descriptores y los
rangos ADSR se comparan con la inicialización original. Repeticiones en directorios
privados nuevos producen exactamente los mismos fixtures. La igualdad está
condicionada por esas entradas y el modelo de CPU; no prueba la voz física completa.

[Value: funciones, hashes y límites](firmware/value-sampler246-validation.json) ·
[Random: funciones, hashes y límites](firmware/random-sampler246-validation.json).
Los fixtures contienen palabras sintéticas y observaciones numéricas, no firmware,
instrucciones, presets ni audio originales.

## Adaptación del plugin

El host conserva el reloj propio de **44100/64 callbacks por segundo**, el RNG
compartido y la retención de modulación entre callbacks. Cada muestra aplica la
modulación retenida a sus bases actuales, incluyendo todos los cambios acoplados;
no acumula ediciones del LFO sobre las bases guardadas. Otro ensayo compara ese
adaptador con callbacks completos en siete tasas de host, además de audio con
seis voces, las dos envolventes, todos los destinos Value y pánico.

Para poder modular la dirección, el botón Reverse se representa internamente
por **8192 en Forward y 24576 en Reverse**, los extremos del descriptor contrastado.
Esta conversión del botón es propia; ambos valores seleccionan el mismo sentido
que los antiguos 0/16384 en el lector. La modulación puede cruzar su umbral 16384
sin alterar el valor guardado del botón. No se afirma recuperar su interfaz física
ni los valores de init de un preset de fábrica.

Tremolo reutiliza su aritmética mono ya contrastada: prepara el stream de pitch
antes del bloque del lector y aplica amplitud al stream de envolvente. Su unión
con lector/ADSR, fase, eventos, reloj, asignación, velocidad, cache PCM y salida
son políticas propias. No se ha verificado un nuevo dispatcher completo de
Tremolo del sampler. La cola host contempla el release más lento cuando Value o
Random pueden modularlo; no certifica latencia física ni ausencia de dropouts.

## Batería y siguiente bloque

La factoría Random de doce voces `0x019cf3d0` liga la vtable `0x0191092c`: callback
`0x019cedc0`, reset `0x019cef66`. El callback lee la selección de tipo por la ruta
drum y llama a **`0x01a1e57c`**, diferente del helper synth. Se han observado 4096
llamadas originales aisladas con tipo 8 y entradas explícitas (1987 retornos 1,
2109 retornos 0). **Son observaciones de ejecución, no comparación C++**, ni
recuperación del productor de ganancia. No se habilitan Value/Random drum o D-box.
La ruta es legible y ejecutable: no se atribuye este dispatcher a la memoria OTP.

El productor de `0xff801d50` sigue separado de estos callbacks: la referencia L1
parcial de `0xffa00700` no reconstruye su contorno. Se mantiene la dinámica propia
de región. El siguiente bloque debe contrastar el helper/dispatch drum y su Value
correspondiente antes de habilitar destinos. Voz/ruteo y reloj originales,
Pulse/L1, comparación acústica, Ableton real, runtime arm64 y presets completos
siguen pendientes. No hacen falta nuevos materiales para reproducir este bloque.

## Reproducción

Usa el firmware privado y un directorio nuevo fuera del checkout para cada orden:

```sh
python3 scripts/verify-value-voice246.py --sampler \
  --ldr /privado/OP1_vdk.ldr --assembler /tools/bfin-elf-as \
  --linker /tools/bfin-elf-ld --simulator /tools/run-fp \
  --testutils /tools/gdb/sim/testsuite/bfin --output /privado/value-sampler-nuevo
python3 scripts/verify-random-voice246.py --sampler \
  --ldr /privado/OP1_vdk.ldr --assembler /tools/bfin-elf-as \
  --linker /tools/bfin-elf-ld --simulator /tools/run-fp \
  --testutils /tools/gdb/sim/testsuite/bfin --output /privado/random-sampler-nuevo
cmake -S research/sampler-render -B build-sampler -DCMAKE_BUILD_TYPE=Release
cmake --build build-sampler --config Release --parallel 2
ctest --test-dir build-sampler -C Release --output-on-failure
```

Las cinco suites independientes incluyen lector y adaptación anteriores,
interpretación, los dos oráculos LFO y la nueva integración. Las observaciones
drum privadas no se incluyen como un sexto oráculo validado.
