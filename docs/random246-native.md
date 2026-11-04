# Random 246: modulación aleatoria por voz

En **Digital 246 CORE, Phase 246 CORE, Dr. Wave o Cluster**, abre **set 4 / T4 →
RANDOM 246**. Azul controla cantidad bipolar (−32768…32767), verde velocidad
(0…32767), blanco selecciona Instrument o ADSR, y naranja la envolvente del LFO
(0…32767). Para escuchar ADSR en Digital/Phase hay que activar su envolvente 246.
El modo Instrument actúa sobre los cuatro controles del motor. La pantalla muestra
las cuatro señales principales de la última voz y la tasa nominal de nuevos
objetivos, no una forma periódica inventada. No se añaden presets.

Una velocidad alta genera nuevos objetivos más a menudo. El reloj no sigue tempo.
La profundidad comienza en cero si naranja <14745 y plena en otro caso; crece
por debajo de 16384 y decae desde ese umbral. Cantidad cero conserva los controles
válidos; los clamps originales siguen corrigiendo bases fuera de rango.
Sesión, slots, clipboard, automatización y mandos MIDI conservan estos controles
con el editor cerrado. Se añaden cinco parámetros después de todos los existentes.
Los botones cambian entre Random, Tremolo, Value Voice y Value FX. Si el host
activa varios flags, la prioridad es **Random → Tremolo → Value Voice → Value FX
→ LFO propio**, limitada a los cuatro motores nativos para las tres primeras rutas.

## Contrato reconstruido

La fábrica synth `0x019cf340` selecciona el callback `0x019cea70` y reset
`0x019ced74` de Random. Se mantiene un estado de 44 bytes: cabecera preservada,
profundidad, contador uint32 y dos grupos de ocho int16 (objetivos y suavizado).
El generador compartido usa `rng = rng * 1664525 + 1013904223` módulo 2^32.
Cada regeneración consume ocho valores y toma sus mitades altas con signo.
El periodo es `1 + (Q15(sat16(32767-speed)^2) >> 6)` llamadas; se compara el
contador incrementado como int32 y se conserva el wrap. El suavizado combina
Q15(target,4096) y Q15(previous,28672), redondeados por separado y saturados.

Instrument admite kind 0 y blanco/2048 = 0; ADSR admite kind 1 y blanco/2048 = 1.
Las otras combinaciones probadas devuelven cero sin modificar estado, RNG,
display ni destinos. El reset solo cambia la profundidad: conserva contador,
objetivos, suavizado y RNG. El display compartido escribe sus 16 bits bajos y
preserva los altos. Los coeficientes de caída son los 24 ya contrastados en Tremolo.

Instrument recorre ocho señales: las primeras cuatro modulan los cuatro controles;
las siguientes vuelven a esos mismos controles, dividiendo aritméticamente su
modulación por 16. Se aplican los rangos/clamps/wrap originales en cada pasada.
**No son ocho controles independientes**. Los cuatro words auxiliares del buffer
se preservan en estas familias. ADSR solo aplica las primeras cuatro señales.
La integración conserva ambas pasadas, incluidos su orden y redondeo.

Se comparan **4096 llamadas originales** (3799 aceptadas y 297 rechazadas),
2048 casos independientes y 2048 con estado retenido, sobre Digital, Phase,
Dr. Wave y Cluster. Coinciden todos los bytes de self, RNG, display de 32 bits,
buffer de ocho words y retorno. La repetición en otro directorio privado produce
fixtures idénticos. Estos contienen datos sintéticos y resultados numéricos;
firmware, instrucciones, desensamblado y ELF permanecen fuera del repositorio.
[Evidencia del oráculo](firmware/random-voice246-validation.json).

## Adaptadores y límites

El plugin usa un reloj propio de **689,0625 callbacks/s** por voz (44100/64),
independiente del tamaño de buffer host, y retiene la modulación entre callbacks.
Cada muestra parte de los controles actuales: no acumula modulación sobre el
resultado anterior. El estado por voz sobrevive a una reasignación de nota;
Mono retrigger modifica la profundidad y Legato continuo conserva el estado.
Un RNG dedicado, compartido por las voces del Engine, comienza en `0x52414e44`
tras prepare/reset. No comparte entropía con el ruido de otros motores.
Desactivar la ruta retira su estado; pánico retira las voces.

Ese seed, orden de llamadas entre voces, asignación y reloj son **adaptadores
propios**: no se ha recuperado el scheduler L1 ni la inicialización completa del
instrumento. Guardar un sonido conserva parámetros, no una captura de la secuencia
aleatoria en ejecución. Este bloque histórico no implementaba Random FX/drum ni otros motores. Las
entregas posteriores amplían los motores y [drum/D-box](drum246-modulation.md);
[0.46.0 integra Random FX](random246-fx.md) con su propia evidencia.
La coincidencia bajo GNU BF524 con ambos parches documentados no demuestra
fidelidad acústica completa, temporización física ni validación en Ableton real.

## Reproducción

```sh
python3 scripts/verify-random-voice246.py \
  --ldr /privado/OP1_vdk.ldr \
  --assembler /herramientas/bfin-elf-as --linker /herramientas/bfin-elf-ld \
  --simulator /herramientas/run-parallel --testutils /herramientas/testsuite/bfin \
  --output /privado/random-repetition
ctest --test-dir build -R random246 --output-on-failure
```

La ruta de salida debe ser nueva y estar fuera del checkout. Se comprueba el hash
del LDR y un microtest de escrituras paralelas antes del oráculo. `--record`
actualiza únicamente los fixtures numéricos. Los dos parches están documentados
en [la guía del simulador](../research/blackfin/README.md).
