# Value y Random de batería — 0.38.0

**DRUM 246** y **D-box** admiten Value Voice y Random sobre sus ocho campos de
Instrument en set 4. Value ofrece dos bancos de cuatro colores: botón **I / II**
junto a **FREE**, que conserva la opción de fase libre. Random recorre los ocho
campos directamente. La prioridad sigue Random, Tremolo, Value Voice, Value FX,
LFO propio. Estado, bancos, proyectos, slots de batería, clipboard, automatización
y proxies MIDI se conservan sin editor abierto. Se añade solo `drum246LfoBank`.

El destino **ADSR se rechaza** en estas dos familias. Se muestra OFF; no cambia
los destinos ni avanza estado/RNG del callback. No se conecta el ADSR común a la
curva de batería ni se inventa una modulación de sus cuatro controles de dinámica.
El control naranja de Random sigue siendo la envolvente del LFO, distinta del
destino ADSR elegido por blanco.

| Familia / banco | Azul | Verde | Blanco | Naranja |
| --- | --- | --- | --- | --- |
| Drum I | Pitch | Start | End | Play Mode |
| Drum II | Reverse | Start Fine | End Fine | Gain |
| D-box I | Pitch I | Wave I | Envelope I | Cross Mod |
| D-box II | Pitch II | Wave II | Envelope II | Filter |

Los resultados de modulación no sobrescriben las bases del pad. Cada golpe tiene
su estado y los pads siguen independientes. Las doce voces y el banco propio de
24 muestras/pads mantienen el mapa, asignación, sustain, stealing y salida propios.
No se añaden kits ni presets originales. Tremolo mantiene la adaptación anterior.

## Contrato contrastado

La ruta drum es distinta de la synth. Value `0x019cfdac`, reset `0x019cff16`, y
Random `0x019cedc0`, reset `0x019cef66`, llaman al helper **`0x01a1e57c`** con el tipo
seleccionado por los globals drum. Los tipos **8** (sampler de batería) y **9**
(D-box) tienen descriptores en `0x01de5a90` y `0x01de5ad0`.

Se contrastan **16.384 llamadas originales**: 4096 Value y 4096 Random para cada
tipo. Hay 2048 casos independientes y 2048 retenidos por corpus, reset, fase libre,
regiones/fracciones correlacionadas, span cero, palabras arbitrarias con signo y
selecciones aceptadas/rechazadas. El argumento kind no filtra esta ruta como en
synth; se contrasta también con kinds distintos bajo esas entradas.

Coinciden estado completo (Value 12 bytes, Random 44), display/meter, RNG, los ocho
destinos, retorno y cálculo de tempo. Ambos juegos de descriptores coinciden con
la inicialización original. Value acepta 2048 y rechaza 2048 casos por tipo;
Random acepta 1987 y rechaza 2109 por tipo. Las repeticiones originales en dos
directorios nuevos dan los mismos bytes. El C++ calcula los estados retenidos;
no recibe los estados posteriores del oráculo como entrada.

Drum ajusta Start/End y sus fracciones de forma acoplada y usa un span mínimo de
32 palabras coarse. No comparte las reglas de cuatro límites del sampler synth.
D-box aplica ocho rangos directos, sin repetir los cuatro primeros a 1/16. La
secuencia RNG/suavizado/gate de Random se comparte como aritmética, con dispatch
y selección de destinos separados. Las regresiones synth mantienen sus oráculos.

[Evidencia, tamaños, hashes y límites](firmware/drum-modulation246-validation.json).
Los fixtures son entradas sintéticas y observaciones numéricas; no contienen
firmware, instrucciones, presets ni audio originales. GNU BF524 requiere los tres
parches documentados, comprobados con probes antes del oráculo. No es validación
sobre hardware ni equivalencia acústica de la voz completa.

## Adaptadores y límites

Se conserva el reloj propio de **44100/64 callbacks por segundo**, estado retenido,
eventos y orden de llamadas por voz. Un RNG dedicado del Engine se comparte entre
golpes. Reasignar un golpe conserva estado del LFO y aplica su reset de nota; FREE
mantiene fase Value. Desactivar/cambiar la ruta reinicia su adaptador. Las bases
actuales reciben la modulación retenida, sin acumularla en el estado guardado.

Drum convierte su pitch en semitonos a `256 + 1024*semitonos`; convierte trims
normalizados a coarse/fine según la longitud del cache, modos a centros
4096/12288/20480 y dirección a 8192/24576. Gain mantiene el mapa de raíz anterior.
**Son conversiones del host**, no la recuperación del productor original de pitch
ni del mapa físico de mandos. El lector consume los campos modulados al preparar
su bloque. D-box conserva su afinación logarítmica propia y admite la parte
negativa del rango de pitch que produce la modulación. El camino sin LFO conserva
la adaptación anterior; una selección ADSR rechazada tampoco la altera.

El tail reportado es conservadoramente infinito con Value/Random Instrument y
cantidad no nula: pitch, dirección, regiones, modos o decaimiento pueden continuar
cambiando tras release. No se ofrece una duración fija calculada solo con el pad
inicial. Las selecciones rechazadas mantienen la política anterior de cola. Esto
no demuestra ausencia de dropouts, latencia física o aceptación de Ableton real.

El ensayo adicional compara callbacks completos y modulación retenida en siete
tasas; comprueba rechazo de ENV sin cambios de estado/display/RNG, audio en cinco
tasas, los ocho destinos, doce voces y pánico. La integración del plugin comprueba
persistencia con audio, proxies cerrados, bancos, slots/clipboard, timestamp MIDI,
64/257 frames, cola y vistas de ambos motores. La validación de entrega y el CI se
registran por separado de los oráculos.

## Dinámica original y próximo frente

Estos callbacks modifican palabras de Instrument, **no generan `0xff801d50`**.
La carga de su dirección en `0xffa00700`, dentro de la ruta parcialmente protegida
alrededor de `0xffa00684`, sigue siendo una referencia, sin un productor completo
contrastado. Se revisa su vecindad: las cargas legibles no forman un flujo de
control recuperado a través de los bytes protegidos. No se sustituye el contorno
por una interpretación de esas instrucciones incompletas.

La dinámica de región propia de 0.34 continúa explícita. Quedan el productor de
ganancia/pitch y ruteo/scheduler originales, el archivo compartido original de
12 segundos y sus 24 regiones, Pulse/L1, Element/MIDI/Bend/Crank según la matriz,
comparación acústica, Ableton real, runtime arm64 y todos los presets al final.
El próximo bloque debe delimitar esos productores y los moduladores pendientes,
sin presentar este dispatch como la voz original completa.

## Reproducción

```sh
python3 scripts/verify-drum-modulation246.py \
  --ldr /privado/OP1_vdk.ldr --assembler /tools/bfin-elf-as \
  --linker /tools/bfin-elf-ld --simulator /tools/run-fp \
  --testutils /tools/gdb/sim/testsuite/bfin --output /privado/drum-mod-nuevo
cmake -S research/drum-render -B build-drum -DCMAKE_BUILD_TYPE=Release
cmake --build build-drum --config Release --parallel 2
ctest --test-dir build-drum -C Release --output-on-failure
```

La salida privada debe ser un directorio nuevo fuera del checkout. El script
verifica el hash de firmware, los probes de CPU y los fixtures existentes. Solo
`--record` escribe las observaciones numéricas del repositorio; firmware, imagen
SDRAM, fuentes originales y ELF permanecen fuera del checkout.
