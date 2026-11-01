# Nitro Hybrid — dos filtros y seguimiento bipolar

Desde 0.23, elige este efecto en el **selector FX de set 3**; sustituye los botones anteriores.

Activa **set 3 → NITRO HYBRID**. Los mandos controlan **Frequency 1, Follow,
Resonance y Frequency 2**. Follow admite valores negativos: la entrada desplaza
ambas frecuencias hacia arriba o abajo según su signo. **Value 246 FX**, en set 4,
puede modular cualquiera de los cuatro colores con sus rangos originales de tipo 6.
Los cinco parámetros nuevos admiten automatización, controles MIDI contextuales
del host y guardado en proyectos, slots y LIFT/DROP. No se añaden presets originales.

El efecto ocupa el mismo bus mono de salida que Filter, tras sintetizador, cinta,
entrada, EQ y master. Los botones eligen un efecto; si la automatización activa
varios, la prioridad es **Nitro → Filter → Punch**. El bus añade el mismo retardo
fijo de 0.20, incluso en bypass: 65/71/141 muestras a 44,1/48/96 kHz (~1,5 ms).
No encadena dos retardos ni modifica PDC al cambiar de efecto. Las pistas crudas
exportadas no imprimen ese procesamiento de salida.

## Qué se conserva y qué se sustituye

El callback mono legible `0x019a62a4` contiene dos cascadas de cuatro etapas,
feedback no lineal, Follow cúbico, resonancia dependiente del pico de entrada,
estados suavizados, saturaciones Q15/Q31 y curva de seno. Se conserva su orden de
operaciones, el pico positivo de las 64 entradas antes de procesar, y el uso de
la tabla de frecuencia sin suavizar por muestra aunque se actualicen las palabras
suavizadas. La tabla de frecuencia y el seno ya se contrastaron en Filter/Punch.

El [bloqueo de L1](firmware/nitro246-table-blocker.json) sigue vigente: faltan dos
coeficientes y dos punteros. Tres floats legibles son `1/6, -2/3, 1/6`; **no bastan
para recuperar los otros dos**. Se adopta una mezcla simétrica propia
`[1/6, -2/3, 1, -2/3, 1/6]` (Q31 con saturación) y cuatro direcciones explícitas
de sus coeficientes. Es una hipótesis de diferencias finitas, no una tabla original
descifrada ni una prueba de que el fabricante usara esa fórmula.

Con esa mezcla la primera cascada produce un pasaaltos ponderado y la segunda un
pasabajos; pueden cerrar mucho la banda o sostener oscilación. El volumen puede
variar considerablemente con frecuencia y resonancia. Se conserva la escala del
modelo elegido, sin normalizarla para simular una comparación acústica inexistente.
STOP/pánico o CC120 limpian estados y buffers; la cola se declara potencialmente
infinita. La pantalla muestra una ilustración propia, no una respuesta medida.

El adaptador compartido con Filter —resampling a 44,1 kHz, DC de 15 Hz, guardia de
silencio inicial, crossfade de activación y ruteo— es propio. Los controles se
muestrean al completar bloques internos de 64 muestras. Cambiar Filter/Nitro
reinicia el estado del efecto; no reproduce el relevo de instancias del hardware.

## Alcance de la validación

Se ejecutan las instrucciones originales con **sustituciones explícitas** de esos
coeficientes y punteros. El puerto coincide en **4.096 bloques / 262.144 muestras**,
60 bytes de estado y 64 bytes de display por bloque. Una repetición privada
independiente produce archivos idénticos. Esto valida la aritmética bajo ese
modelo híbrido; **no demuestra coincidencia con Nitro original**, ni recupera L1.
[Metadatos y hashes](firmware/nitro-hybrid246-validation.json).

Las pruebas del adaptador comprueban bypass exacto retrasado a siete tasas,
ausencia de falta de datos, silencio inicial/pánico, 8.192 muestras nativas frente
al núcleo, rechazo de graves/agudos y automatización extrema finita. Las pruebas
del plugin cubren proxies con editor cerrado, rango bipolar, prioridad, proyectos,
slots/clipboard y renders iguales con buffers 64/257. La carga de tiempo real incluye
los cuatro motores nativos; los tiempos del entorno compartido son diagnósticos.

La aceptación en Ableton real, comparación acústica con OP-1 y las tablas originales
siguen pendientes. El CI nativo de esta entrega valida compilación y empaquetado,
no sustituye esas comprobaciones.

## Reproducción privada

```bash
python3 scripts/verify-nitro-hybrid246.py \
  --ldr /privado/OP1_vdk.ldr \
  --assembler /herramientas/bfin-as --linker /herramientas/bfin-ld \
  --simulator /herramientas/run-parallel \
  --testutils /fuentes-gdb/sim/testsuite/bfin \
  --output /privado/nitro-hibrido-nuevo
```

El directorio debe ser nuevo y quedar fuera del repositorio. El harness propio
identifica las sustituciones; no ejecuta el inicializador sobre datos cifrados.
Firmware, ELF, desensamblado y decompilaciones permanecen privados. Los fixtures
contienen únicamente entradas sintéticas y resultados numéricos del modelo.
