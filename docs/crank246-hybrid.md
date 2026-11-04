# Crank LFO 246 — 0.42.0

Elige **LFO: Crank** en la página LFO. Blue mueve el encoder virtual; Green
controla la cantidad bipolar, White elige Instrument/ADSR/FX y Orange el target.
Mueve Blue en ambas direcciones: el controlador estima una velocidad con inercia,
y esa velocidad hace avanzar una fase triangular. Al dejar de moverlo se frena
hasta detenerse; conserva la fase alcanzada. **Una entrada inmóvil puede mantener
una modulación distinta de cero**. No es un oscilador de velocidad automática.

El manual §15.7 (PDF 49) identifica Blue para el movimiento/velocidad, Green para
la cantidad y White para el destino. El accesorio físico no se emula como un
protocolo USB. El encoder virtual es automatizable: su rango firmado −8192…8191
abarca una vuelta de 256 posiciones cuantizadas. Tampoco habilita los modos Crank
de los secuenciadores Endless/Tombola, que tienen otro contrato pendiente.

## Evidencia original acotada

Se localizó la séptima entrada de las factorías LFO (**índice 6 contando desde
cero**): wrapper `0x019c9ab2`, factoría synth `0x019cb62c` y referencias a
`LfoCrank.cpp`. Las tablas de vtable confirman los tres callbacks.

| Componente | Dirección |
|---|---|
| Procesador de entrada compartida | `0x019cae26` |
| Preparación de pasada | `0x019cae18` |
| Reinicio de inicialización de entrada | `0x019cae08` |
| Callback synth | `0x019cb0d4` |
| Callback drum | `0x019cb30c` |
| Callback FX | `0x019cb3d4` |

Se contrastan **18.432 llamadas originales**, 6.144 por familia, intercaladas con
una misma fuente compartida. Se comparan las siete palabras completas de estado,
diez entradas de historial, ángulo previo, ocho controles, ocho targets, self/tag,
retorno y los buffers íntegros de pitch/ganancia/audio FX inicializados con datos
sintéticos. Las rutas especiales que modificarían esos buffers están excluidas:
se comprueba que las rutas de parámetros los dejan intactos.

Los casos incluyen estados suministrados acotados, flags parcialmente ocupados,
prepare/reset, contador uint32 con wrap, ambos sentidos, cruce angular, arranque,
velocidad/target positivos y negativos, acumulación y reposo. La secuencia final
mantiene el estado sin inyecciones y termina con velocidad y target cero, conservando
la fase. Se exige un despacho aceptado para cada tipo synth/drum soportado y para
los ocho FX con kinds 2/5. Una repetición independiente exige salida idéntica.

El estado compartido ocupa 28 bytes en `0xff907acc`; el historial de 40 bytes
comienza en `0xff907ae8`, y el ángulo anterior está en `0x01dd5a54`. El núcleo
recuperado incluye lectura cada 50 ticks, historial de diez deltas, estimación de
velocidad limitada a ±120, frenado asimétrico en los cambios de dirección,
acercamiento al target cada cuatro ticks, pasos fraccionarios en 32 fases y
triángulo de salida centrado en 64. Los callbacks usan esa señal con cantidad Q15
para los rangos y los destinos originales legibles.

Esto demuestra coincidencia **bajo controles, RAM y acciones de preparación
explícitos**, en el simulador BF524 con los tres parches de writeback contrastados.
No recupera el lector físico, su scheduler, toda la voz ni equivalencia acústica.
Los resets de voz observados son no-op; no se reinicia por cada nota.

## Adaptación del host

Un controlador global comparte entrada y fase entre voces y FX. Avanza incluso
sin notas, con reloj propio `44100/64`, independiente del tamaño de buffer. La
activación, desactivación y pánico limpian todo el estado como política propia;
los eventos de nota no lo reinician. Guardar/restaurar conserva parámetros, no la
trayectoria de velocidad/historial. Tras habilitarlo o cargar un proyecto hay que
volver a mover Blue después de la primera inicialización.

Los cuatro mandos y la activación se automatizan y persisten en proyectos, slots y
clipboard. Los Screen Knob conservan sus IDs y siguen Crank con el editor cerrado.
Los parámetros se capturan por bloque; se contrasta audio exacto en particiones
64/257 con cambios en un límite común explícito. No se atribuyen timestamps por
muestra a la automatización del host; los eventos MIDI conservan su tratamiento.

Instrument expone cuatro targets en los motores de cuatro controles y ocho en
Sampler/D-synth/drum/D-box. El núcleo contrasta también los selectors fine de synth,
que no se presentan como targets 5–8 en motores de cuatro mandos. ADSR ofrece los
primeros cuatro campos con envolvente nativa; drum/D-box lo rechazan. Los ocho FX
exponen cuatro parámetros, incluso con un motor heredado sin Instrument nativo.
Targets no disponibles muestran OFF.

El selector elige un único LFO. Si el host fuerza varios flags, la prioridad es
Crank, Bend, MIDI, Random, Tremolo, Value Instrument/ADSR, Value FX y LFO propio.
El botón Reverse de Bend se oculta en Crank: el sentido depende del movimiento.
La cola anunciada es conservadora si Release o un parámetro drum puede prolongar
la voz; no cambia el PDC ni añade un volumen maestro adicional.

## Reproducción y límites

`Crank246.h` y su adaptador son código independiente. Los fixtures gzip contienen
únicamente entradas sintéticas y observaciones numéricas; no firmware, volcados
completos ni código descompilado. `scripts/verify-crank246.py` necesita el LDR privado
rev246 con el hash conocido, herramientas Blackfin y el simulador corregido.
La salida debe ser un directorio nuevo fuera del checkout; `--record` registra
observaciones, y repetir sin él exige igualdad. [Hashes y matriz](firmware/crank246-validation.json).

La validación nativa no necesita firmware ni JUCE:

```sh
cmake -S research/crank-modulation -B build-crank -DCMAKE_BUILD_TYPE=Release
cmake --build build-crank --config Release
ctest --test-dir build-crank -C Release --output-on-failure
```

Requiere C++17, CMake 3.22 y Python 3.11. El adaptador se prueba a siete frecuencias,
252 combinaciones perfil/destino/frecuencia, ambas direcciones, inercia y parada,
prioridad/pánico y audio exacto con distintos buffers. [Evidencia de entrega](releases/v0.42.0-validation.json).

Quedan fuera los especiales gain/pitch/audio, ENV auxiliar FX kind 4, lector físico,
scheduler/voz/ruteo originales y secuenciadores Crank. El pitch wheel MIDI sigue
independiente. Este bloque no elimina las sustituciones de los motores/FX híbridos
ni recupera L1/OTP. Siguen pendientes Element, calibración acústica, Ableton Live
real, ejecución arm64 y los presets originales completos al final.
