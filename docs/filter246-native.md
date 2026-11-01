# Filter 246 — núcleo contrastado y bus de salida

Desde 0.23, elige este efecto en el **selector FX de set 3**; sustituye los botones anteriores.

En **set 3**, activa **FILTER 246**. Los mandos existentes controlan **Cutoff,
Resonance/Follow, Mode y Drive**. Los cinco parámetros nuevos —activación y cuatro
controles— son automatizables y se guardan con el proyecto, slots y LIFT/DROP.
El editor puede cerrarse sin perder los cuatro mapeos MIDI. Filter tiene prioridad
sobre Punch si una automatización activa ambos; los botones seleccionan uno.

La inserción actual es **el bus de salida del plugin**, después de cinta, entrada,
EQ y master. Afecta también al audio monitorizado y a la reproducción de cinta;
no imprime el efecto en los archivos de pistas crudas exportados. Es ruteo propio,
no una reconstrucción de la cadena completa del OP-1. No se añaden presets.

## Núcleo y tablas

La ruta mono `0x019e7934` coincide en **4.096 bloques / 262.144 muestras**, sus
36 bytes de estado y 64 bytes de display compartido, incluyendo espacios no
modificados de otros slots. Se ejecutó dos veces en directorios privados con
resultado idéntico. Los 65 coeficientes de mezcla se generan por el inicializador
legible `0x019e7f1c`. Los 129 coeficientes de frecuencia se convierten con el helper
original desde sus floats legibles, reproduciendo el fragmento exacto de arranque;
esto no ejecuta ni valida todo el boot. [Hashes y límites](firmware/filter246-validation.json).

Se conservan la mezcla cuadrática entre modos, cuatro etapas, no linealidad por
seno interpolado, acoplamiento de feedback/drive/pico y el orden de saturación
Q15/Q31. El medidor recoge el máximo positivo y su decay, no un pico absoluto.
La palabra de frecuencia suavizada se actualiza aunque esta ruta usa la entrada
de frecuencia de tabla en el cálculo por muestra; no se cambia por una versión
supuestamente más correcta. Los buffers de prueba son entradas explícitas, no
una memoria L1 recuperada.

## Por qué cambia la latencia declarada

El callback consulta **las 64 muestras de entrada antes de renderizar**. Su pico
interviene en todas ellas, por lo que procesar una muestra suelta como Punch
cambiaría el resultado. El adaptador reúne bloques a 44,1 kHz y los procesa enteros.

El bus mantiene una latencia fija de `ceil(64 × tasa_host / 44100) + 1` muestras,
también con Filter apagado. Son 65 muestras a 44,1 kHz, 71 a 48 kHz y 141 a 96 kHz,
alrededor de 1,5 ms. `getLatencySamples()` comunica esa espera al host para su
compensación. Al apagar/encender el efecto no se cambia la latencia ni se envían
notificaciones desde el callback. El bypass es una copia exacta retrasada; cinta,
entrada, metrónomo y sintetizador comparten el mismo retardo de salida.

La conversión causal de tasas, anti-alias de entrada, interpolación de salida,
bloqueo DC de 15 Hz, guardia de silencio inicial y crossfade de 12 ms son propios.
La prueba a 44,1 kHz desactiva esas guardias para contrastar 8.192 muestras con
llamadas directas al núcleo. No se usa contenido futuro de otro callback ni se
asigna/libera memoria al procesar. Cambios de controles se toman al completar el
bloque interno; su cadencia/alineación completa en el hardware sigue pendiente.

El efecto puede sostener oscilación; se declara cola potencialmente infinita,
como Punch. STOP/pánico o CC120 limpian su estado y los buffers de salida.

## Value y comprobaciones

**set 4 → VALUE 246 FX** puede modular el color seleccionado de Filter. El destino
blanco pasa a llamarse **Active FX**. Se reutilizan la ruta FX y rangos ya
contrastados para el tipo 5; reloj, inserción y suavizado siguen siendo propios.
La selección de Tremolo, Random o Value Voice conserva su prioridad actual.

Las pruebas incluyen:

- Oráculo completo, tablas generadas, estado/display y extremos/continuidad.
- Bypass exacto y sin falta de datos a siete tasas, latencia, silencio y pánico.
- Identidad entre bloques y adaptador a 44,1 kHz, sin DC/guardias propios.
- Proxies/GUI, estado, slots/clipboard y cambios de buffer 64/257 con Value FX.
- 228 cargas del callback, con primer disparo y cambios de activación/controles
  sin heap rastreado. Los tiempos compartidos de CI no certifican cero dropouts.

La comparación acústica con hardware y scanner/audio/automatización en Ableton
real siguen pendientes. El CI 0.19 anterior sí pasó Linux, Windows y macOS,
incluido el paquete universal; el nuevo código necesita su propio CI.

## Nitro

Nitro tiene callback legible, pero el inicializador convierte cinco palabras
procedentes de L1 y dos caen en las franjas protegidas. Además el render consume
otra zona protegida. Se registra el [bloqueo y offsets](firmware/nitro246-table-blocker.json)
sin rellenarlos con ceros ni convertirlos en coeficientes supuestamente originales.
La entrega 0.21 añade un [Nitro híbrido](nitro246-hybrid.md) con sustituciones
propias explícitas; las tablas originales continúan protegidas.
Esta entrega retoma Filter desde main, como se autorizó al recuperar el chat.

## Reproducir

```bash
python3 scripts/verify-filter246.py \
  --ldr /privado/OP1_vdk.ldr \
  --assembler /herramientas/bfin-as --linker /herramientas/bfin-ld \
  --simulator /herramientas/run-parallel \
  --testutils /fuentes-gdb/sim/testsuite/bfin \
  --output /privado/filter-nuevo
```

La salida debe ser un directorio nuevo fuera del repositorio. El firmware, ELF,
desensamblado y pseudocódigo se conservan privados. Los fixtures publicados son
entradas/resultados numéricos del ensayo, no instrucciones del firmware.
