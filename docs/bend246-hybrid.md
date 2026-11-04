# Bend LFO 246 — 0.41.0

Bend ofrece control manual de la modulación. En la página LFO, elige **LFO: Bend**:
Blue selecciona el target, Green Instrument/ADSR/FX, White la cantidad bipolar y
Orange la entrada manual. **REVERSE BEND** invierte la dirección del movimiento.
Mueve Orange para excitar el controlador; mantenerlo inmóvil termina provocando
el retorno gradual. La pantalla muestra el nivel y marca OFF targets no disponibles.

El manual §15.8 (PDF 49) describe el encoder naranja, cantidad en White, destinos
en Blue/Green y cambio de dirección con Shift + Orange. El plugin ofrece un
encoder virtual automatizable y un botón de dirección. **No es el pitch wheel**:
el bend MIDI por canal existente conserva su función independiente.

## Reconstrucción contrastada

La sexta entrada de las factorías LFO (índice 5 contando desde cero) conduce a
estos callbacks legibles; las referencias
de error de sus factorías identifican `LfoBend.cpp`:

| Ruta | Dirección | Contrato contrastado |
|---|---|---|
| Synth | `0x019c9d44` | Instrument y primeros cuatro ADSR; kinds explícitos |
| Drum | `0x019ca0ec` | Instrument de ocho campos; otros destinos rechazados |
| FX | `0x019ca328` | Parámetros, kinds 2/5; auxiliares ENV excluidos |
| Preparar pasada | `0x019c9d04` | Limpia el flag de procesamiento compartido |
| Reiniciar entrada | `0x019c9cf4` | Limpia el byte de inicialización, conservando las demás palabras |

Se ejecutan **6.144 llamadas originales intercaladas**, 2.048 por familia, con
synth/drum/FX compartiendo el estado de entrada, historial y flag. Se inyectan
estados acotados y acciones explícitas de prepare/reset; otra secuencia larga
conserva el estado y cubre reposo y retorno. La cobertura exige casos aceptados
para todos los tipos synth soportados, ambos drum, los ocho FX y ambos kinds FX.

Se comparan las ocho palabras del estado, dos entradas de historial, palabra de
display/flag completa, ocho controles (incluido el display modificado), ocho
targets, self/tag y retorno. Además se comprueba que permanecen intactos los
128 pitch words, 128 gain halfwords y 128 FX audio halfwords inicializados con
sentinelas sintéticas. Las rutas de audio/pitch **no se ejecutan en este contrato**.
El resultado se repite exactamente en otro directorio privado.

El estado compartido ocupa 32 bytes en `0xff907aac`; los dos históricos están en
`0x01dd59b4` y el flag en la palabra `0x01dd591c`. El receptor físico del encoder
no forma parte del test: se suministra su palabra de control.

El procesamiento recuperado incluye:

- Una actualización por flag preparado, con contador uint32 y wrap contrastado.
- Lectura periódica cada diez actualizaciones; ángulo cuantizado en 256 posiciones
  y diferencia angular corregida al cruzar el límite.
- Dirección, reinicialización, historial de dos diferencias y acumulación asimétrica.
- Target acotado a 0–127, rampa de una unidad y retorno tras inactividad.
- Conversión Q15 de nivel/cantidad, dispatch original de rangos, selectors fine de
  synth y reglas de región acopladas de sampler/drum.

Esto prueba la aritmética y el dispatch **bajo las entradas/estados/acciones
suministrados** en el simulador BF524 corregido, no el reloj físico, su lector de
encoder, el accesorio ni equivalencia acústica. Los reset de voz son no-op en las
vtable observadas; no se interpreta la entrada manual como un reinicio por nota.

## Adaptador propio

El plugin comparte un solo controlador entre voces y FX y lo mantiene con el
reloj propio `44100/64`, independiente del tamaño del buffer y de las notas activas.
La fuente avanza también sin voces mientras Bend está habilitado. El flag de
pasada se prepara en esos ticks; no se afirma recuperar el scheduler del OP-1.

Los cuatro parámetros y Reverse son automatizables y se guardan en proyectos,
slots y clipboard. Los cuatro Screen Knob conservan sus IDs y siguen Bend con
el editor cerrado. Los parámetros del host se toman por bloque; las pruebas de
partición usan cambios en un límite explícito común, **no timestamps de muestra
para automatización de parámetros**. Los eventos MIDI mantienen su tratamiento
habitual por muestra.

La activación, desactivación, prepare y pánico limpian el estado completo como
política propia. Las notas no lo reinician. Se guardan controles y dirección,
no la trayectoria viva del historial/contador. Al cargar o habilitar Bend hay que
volver a mover Orange después de su inicialización.

Instrument admite cuatro targets en los motores de cuatro controles y ocho en
Sampler/D-synth/drum/D-box; los selectors fine adicionales contrastados del núcleo
no se exponen como targets 5–8 de los motores de cuatro controles. ADSR necesita
la envolvente nativa habilitada; drum/D-box lo rechazan. Los ocho FX tienen sus
cuatro controles expuestos, también con un motor heredado sin Instrument nativo.

El selector LFO sustituye la fila de botones y conserva los parámetros booleanos
individuales existentes. Sus cambios eligen un modo y desactivan los demás. Si el
host fuerza flags simultáneos, la prioridad es Bend, MIDI, Random, Tremolo,
Value Voice, Value FX y LFO propio. Se anuncia una cola conservadora para Release
y controles drum que puedan prolongar la voz.

## Límites y reproducción

Audio/pitch especiales, FX kind 4, otros ADSR auxiliares, lectura física y
scheduler/voz/ruteo originales siguen pendientes. Los motores/FX híbridos conservan
sus sustituciones documentadas; este bloque no las elimina. Tampoco recupera L1,
OTP ni el productor original de ganancia drum. Element y Crank no se habilitan.

Los fixtures gzip contienen entradas sintéticas y observaciones numéricas, nunca
firmware o código descompilado. `scripts/verify-bend246.py` requiere el LDR privado
rev246 con su hash conocido, assembler/linker/testutils Blackfin y simulador con
los tres parches de writeback comprobados. La salida debe ser un directorio nuevo
fuera del checkout. `--record` graba observaciones; repetir sin él exige igualdad.
[Hashes, herramientas y matriz aceptada](firmware/bend246-validation.json).

La suite independiente no necesita JUCE ni firmware:

```sh
cmake -S research/bend-modulation -B build-bend -DCMAKE_BUILD_TYPE=Release
cmake --build build-bend --config Release
ctest --test-dir build-bend -C Release --output-on-failure
```

Requiere C++17, CMake 3.22 y Python 3.11. El wrapper descomprime los datos en un
directorio temporal y ejecuta la comparación C++ nativa. El adaptador se contrasta
en siete tasas y 252 casos motor/destino/tasa, con audio exacto en particiones de
64/257 muestras, rechazo de ADSR drum, prioridad y pánico. [Entrega](releases/v0.41.0-validation.json).
Ableton Live real, ejecución arm64 y calibración acústica siguen pendientes.
