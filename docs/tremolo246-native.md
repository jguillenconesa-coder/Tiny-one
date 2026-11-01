# Tremolo 246: pitch y amplitud por voz

En Phase 246 CORE, Digital 246 CORE, Dr. Wave o Cluster, abre **set 4 / T4** y
activa **TREMOLO 246**. Los cuatro mandos y sus identidades MIDI habituales siguen
Speed, Pitch Depth, Amplitude y LFO Envelope. El selector inferior elige **Sine,
Ramp, Curve, Square o Pulse**. La pantalla muestra una representación de la forma,
la fase de la última voz y la tasa nominal del adaptador.

El init conserva las profundidades a cero; sube blanco para oír la modulación de
amplitud o verde para pitch. Para empezar, una profundidad moderada permite
oír mejor Ramp/Curve: con profundidad máxima pueden comenzar casi mudas
hasta que avanza la fase. No se añaden presets de fábrica ni variantes de
estudio. Los controles se guardan en sesión, slots y clipboard. Funcionan con el
editor cerrado. Los parámetros nuevos se añaden después de los anteriores.

| Color | Control | Comportamiento raw |
| --- | --- | --- |
| Azul | Speed | 0–16383 sigue tempo; al subirlo se reduce la tasa por divisiones binarias. 16384 detiene el reloj; por encima crece una curva de sexta potencia independiente del tempo. |
| Verde | Pitch Depth | −32768…32767; cantidad e inversión del pitch. Incluye una expansión no lineal en la zona positiva alta. No es un selector de semitonos. |
| Blanco | Amplitude | 0–32767; profundidad de amplitud. La zona alta también modifica la forma. Cero conserva el volumen previo. |
| Naranja | LFO Envelope | 0–32767; evolución de la profundidad desde el disparo. Por debajo de 14745 comienza desde cero y crece; de 14745 a 16383 comienza plena; desde 16384 decae usando la tabla reconstruida. |

Tremolo tiene prioridad sobre Value Voice, Value FX y el LFO propio cuando sus
flags se activan simultáneamente mediante automatización. Los botones permiten
cambiar de LFO; al apagar Tremolo se recupera el mapeo de la ruta que quede activa.
Este lote no amplía Tremolo al FM/String propios, al sampler ni a drum.

## Algoritmo contrastado

Se identifica el índice 0 de la fábrica synth de LFO como `LfoDual`, asociado a
Tremolo por la tabla de nombres. Callback `0x019cbe48`, reset `0x019cbdca`,
constructor synth `0x019cc664`. El estado tiene 28 bytes y el display compartido,
32. Hay dos fases, suavizado para las formas no sinusoidales y una profundidad
con evolución por bloque. Pitch y amplitud se procesan en 128 posiciones; los
kinds 0 y 3 son activos y las demás llamadas probadas se rechazan sin cambios.

`Tremolo246.h` reconstruye esa aritmética. Se contrastan **2048 llamadas**, con
**262144 palabras de pitch y 262144 muestras de amplitud**, estados completos,
bytes preservados, display, retorno, reset y secuencias retenidas. Incluye
profundidades de pitch negativas y extremas, tempo sintético de 32 bits y las
cinco formas. De las 2048 llamadas, 1664 procesan audio/pitch (212992 posiciones
activas por buffer); las otras 384 comprueban el rechazo y la preservación.
Se convierten las 24 entradas float32 de la tabla de envolvente
mediante el helper original y se comprueba el resultado contra la tabla C++.

Se repite el oráculo en otro directorio privado, con salida idéntica. Los
adaptadores que intercalan pitch/amplitud por muestra o preparan primero el
buffer de pitch también coinciden con las llamadas originales en estos casos.
Las instrucciones, ELF, firmware y su desensamblado permanecen fuera del repo.
Los fixtures contienen entradas sintéticas y resultados numéricos, no firmware.

## Ruteo del plugin y límites

`Tremolo246Voice.h` organiza los mismos cálculos a **88,2 kHz internos**, la tasa
que ya usan los adaptadores de voz; esa elección no establece el reloj físico
original. Phase/Digital procesan pitch y amplitud por muestra interna. Dr. Wave y
Cluster preparan el pitch antes de cada bloque de su núcleo y consumen amplitud
por muestra. Se aplica Tremolo a la ganancia ADSR cuantizada antes del filtro de
salida. Si se usa la envolvente propia de Phase/Digital, la modulación se aplica
con una entrada de ganancia fija y esa envolvente queda fuera del núcleo 246.

El display y el estado se mantienen por voz; la máquina comparte parte del
display. Se conservan los asignadores propios y la mezcla/salida existentes.
Dr. Wave mantiene su contrato de lectura de pitch y el cambio por bloque, sin
convertirse en un oscilador con un nuevo lector por muestra. No se recuperan el
scheduler, los buffers iniciales ni el relevo ADSR L1 originales.

Los controles y tempo se fijan al inicio de cada bloque interno de 128 muestras.
En los motores con buffer, activar el LFO espera al siguiente bloque para
arrancar juntos sus relojes de pitch y amplitud. Apagarlo evita nuevas modificaciones de
amplitud; el pitch ya preparado se consume hasta el siguiente bloque.
Disparar/reasignar una voz reinicia Tremolo; Legato conserva su fase cuando
conserva la voz. No hay una nueva opción FREE en este lote. Pánico retira voces y
modulación; el LFO no genera audio independiente después de un reset.

La comparación demuestra los casos aislados bajo GNU BF524 con los dos parches
documentados de escrituras paralelas. No demuestra semejanza acústica completa
con un OP-1, temporización física ni aceptación del VST3 en Ableton real.

## Reproducir

```sh
python3 scripts/verify-tremolo246.py \
  --ldr /privado/OP1_vdk.ldr \
  --assembler /herramientas/bfin-elf-as --linker /herramientas/bfin-elf-ld \
  --simulator /herramientas/run-parallel \
  --testutils /fuentes-gdb/sim/testsuite/bfin \
  --output /privado/tremolo-nuevo
```

Requiere los dos parches de [la preparación Blackfin](new-account-dsp-setup.md).
Sin `--record` comprueba los fixtures existentes. La salida debe ser nueva y
estar fuera del checkout. [Contrato y hashes](firmware/tremolo246-validation.json).

`tremolo246` verifica núcleo, coeficientes y ambos adaptadores de orden/cadencia.
`tremolo246_integration` cubre siete tasas entre 8 y 192 kHz, cuatro motores,
ambas profundidades y cinco formas, identidad con profundidad cero, sustain,
release, pánico, extremos de controles/MIDI y activación durante un bloque.
El plugin comprueba interfaz, MIDI contextual, prioridad, guardado y renders
iguales con buffers host de 64 y 257 muestras. El benchmark añade 24 cargas a las
156 previas. Los sanitizadores se aplican al oráculo completo y a la integración
con 48/96 kHz y las esquinas de controles. [Validación de la entrega](releases/v0.18.0-validation.json).
