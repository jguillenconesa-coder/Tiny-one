# D-synth 246 — render reconstruido e integración 0.32.0

D-synth es la selección **11 / D-SYNTH**, con dos osciladores, cuatro
configuraciones de forma y ocho rutas de mezcla. El callback mono completo
`0x0197e1d8` se reconstruye en C++ y coincide con **4096 bloques / 524288
muestras** originales ejecutadas en el modelo BF524 corregido descrito debajo.
Se comparan los seis estados de 72 bytes, contexto de 824 bytes, eventos,
históricos, audio, pitch original/derivado, segunda señal, ganancia y guardas.

La comparación usa controles válidos, estados arbitrarios, pitch de 32 bits
variable dentro del bloque y ganancia signed16 positiva/negativa, además de
secuencias retenidas. La reconstrucción calcula todo el estado posterior: **ya
no recibe del oráculo el resultado del render**, como ocurría en la etapa de
[fondación](dsynth246-foundation.md). Dos ejecuciones independientes reproducen
el corpus. No se sustituyen coeficientes: seno, 256 coeficientes de envolvente
y 129 de frecuencia provienen de las rutas legibles contrastadas.

## Controles y uso

Cargar **D-synth 246 → Init / D-synth**, o seleccionar el undécimo motor.
En set 1, **OSC II** cambia el banco visible de cuatro mandos:

| Color | Banco I | Banco II |
| --- | --- | --- |
| Azul | Forma I/II | Pitch II |
| Verde | Feedback I | Feedback II |
| Blanco | Envolvente interna I | Envolvente interna II |
| Naranja | Mezcla | Filtro |

Son etiquetas funcionales propias. Los ocho controles raw son 0–32767; sus
descriptores originales son `{0,32767,10485,3}`. El init
`{12000,2000,16000,5000,32767,2000,16000,16000}` es propio, no un preset original.
Set 2 mantiene los cuatro controles del ADSR común y Poly/Mono/Legato/glide.
Las dos envolventes internas pertenecen al núcleo D-synth y están incluidas
en la comparación completa; no se sustituyen por el ADSR común.

Value Instrument actúa sobre los cuatro colores del banco elegido en set 1;
seleccionar **OSC II** antes de ir a LFO permite apuntar a los controles 5–8.
Value ADSR sigue teniendo cuatro destinos, independientemente del banco.
Random Instrument modifica los ocho controles directamente. Los 4096 Value y
4096 Random callbacks originales ya contrastados siguen coincidiendo al repetir
con el modelo corregido. Tremolo modula pitch y ganancia por bloques.

Se integran seis voces, sustain/bend por canal, modos de interpretación,
automatización de los ocho controles/ADSR/banco, proyectos, slots, clipboard,
mandos contextuales incluso con editor cerrado y animación propia. No se incluyen
activos gráficos, firmware ni presets originales.

## Ganancia y salida

El callback consume pitch y ganancia completos de 128 muestras. El adaptador
prepara el ADSR común y Tremolo **antes del filtrado interno de D-synth**;
no aplica una segunda envolvente después del render. La voz conserva la cola
de sus filtros hasta un bloque de salida por debajo de un LSB. Esta detección
de reposo es propia, y el host añade un margen conservador de 0,25 s a la cola
declarada para permitir que esos filtros terminen.

Son propios el reloj de 88,2 kHz, conversión nota/pitch, cadencia de controles y
eventos, asignación/robo, velocidad, composición del ADSR y filtrado/resampling
hacia el host. El bus de salida y su latencia fija/PDC se conservan. El kernel
no necesita OTP ni una captura arbitraria de RAM externa; sus seis slots y las
memorias de indicadores/histórico son explícitos. Esto no recupera L1 ni establece
el scheduler/ruteo original completo.

## Corrección necesaria del simulador

El modelo anterior aplazaba escrituras de Dregs y cargas halfword, pero
`decode_LDSTiiFP_0` seguía escribiendo directamente los Dregs cargados desde FP.
En un paquete con carga y store del mismo registro, el store tomaba el valor
nuevo. En D-synth afectaba a la primera señal, sus indicadores y el filtrado.

El [Blackfin Processor Programming Reference, rev. 2.2, §4-9, PDF p.143](https://www.analog.com/media/en/dsp-documentation/processor-manuals/Blackfin_pgr_rev2.2.pdf)
sitúa las lecturas del register file en DF2 y las escrituras en WB. El probe propio
`parallel-fp-load.s` aísla dos stores high/low y verifica los operandos anteriores
al paquete: `0x1234` / `0x1357`, frente a `0xaabb` / `0xccdd` del modelo previo.
El valor cargado posterior permanece `0xaabbccdd` y la operación DSP produce 6.

`gnu-13.1-fp-load-writeback.patch` hace que esa codificación use la misma cola
Dreg. Se aplica **después de los dos parches existentes**; el generador D-synth
rechaza simuladores que no superen los tres probes. `scripts/setup-blackfin.py`
admite `--fp-load-writeback`, que implica la opción halfword. No se cambia el
perfil de herramientas ya instalado ni la configuración del entorno en la nube.

Se repitieron, sin cambiar sus fixtures, los oráculos de FM, String, DNA, Voltage,
Filter, Nitro, Delay, Grid, Phone, Spring, CWO y Punch: **los doce conservan sus
resultados bajo sus respectivos perfiles**. Esto no elimina las sustituciones
ya documentadas de FM/Nitro/CWO ni equivale a medir el hardware. Los antiguos
vectores de D-synth pertenecían al modelo previo y se reemplazan por el corpus
corregido; la evidencia histórica identifica su commit de procedencia.

## Validación y límites

[Registro de entrega](releases/v0.32.0-validation.json) ·
[Metadatos, hashes y probes](firmware/dsynth246-render-validation.json).

```sh
cmake -S research/dsynth-render -B build-dsynth -DCMAKE_BUILD_TYPE=Release
cmake --build build-dsynth --config Release --parallel 2
ctest --test-dir build-dsynth -C Release --output-on-failure
```

Ese proyecto independiente comprueba ciclo, render, modulación y adaptador.
La coincidencia numérica está condicionada por controles, eventos, pitch,
ganancia y modelo de CPU suministrados. Quedan calibración acústica con las
referencias disponibles, Ableton Live real, ejecución macOS arm64 y componentes
pendientes de la matriz. Los presets originales completos se dejan al final.
No se afirma equivalencia sonora del instrumento completo.
