# MIDI LFO 246 — reconstrucción e integración 0.39.0

El MIDI LFO descrito en §15.6 del manual acepta cuatro fuentes CC y permite elegir
destinos. Pocket One incorpora cuatro CC entrantes, Instrument, ADSR y los cuatro
mandos del FX seleccionado. La opción **MIDI LFO** se encuentra en la página LFO;
los cuatro encoders eligen el destino de CC 1–4. El OLED muestra sus valores y
marca **OFF** los destinos no disponibles en el motor o efecto actual.

## Evidencia original acotada

Las entradas de fábrica de tipo LFO 5 conducen a los callbacks legibles:

| Familia | Callback | Autorizado en el verificador |
|---|---|---|
| Synth | `0x019cce68` | Instrument, primeros cuatro ADSR, modificación de gain/pitch suministrados |
| Drum | `0x019cd048` | Instrument, ocho palabras; otros destinos rechazados |
| FX | `0x019cd104` | kinds 2/5, parámetros y modificación del buffer suministrado |

Se contrastan **2.048 llamadas por familia: 6.144 en total** en el simulador BF524
con los tres parches de writeback comprobados. Se suministran controles, CC,
selección de motor/FX, targets, display y buffers, y se ejecuta el inicializador
original de rangos. Cada llamada recarga sus entradas: **son casos independientes,
no una captura del receptor MIDI ni un stream original del scheduler**.

La comparación C++ incluye self/tag, las cuatro palabras CC, cuatro halfwords de
display, ocho targets, 128 palabras pitch, 128 halfwords gain, 128 halfwords FX y
retorno. La ruta FX de audio modifica solamente los primeros **64** halfwords;
se comprueba que los 64 restantes permanecen intactos. Los fixtures comprimidos
contienen entradas sintéticas y observaciones numéricas, nunca firmware ejecutable.

El valor CC se convierte en modulación bipolar restando 64 y desplazando diez
bits con saturación de 16 bits; 64 aporta desplazamiento cero. Se procesan los
cuatro streams en orden, con saturación secuencial si comparten destino. Las
reglas de rangos, wrapping y regiones acopladas se reutilizan de las rutas ya
contrastadas. En Instrument synth de cuatro mandos, los selectores auxiliares
4–7 vuelven a 0–3 con modulación dividida aritméticamente por 16. El producto
expone los cuatro mandos primarios; los ocho destinos directos quedan disponibles
para Sampler, D-synth, drum y D-box.

La evidencia no cubre colores negativos/fuera del rango válido, colores ADSR
auxiliares ni el dispatch FX kind 4. El callback no necesita estado privado aparte
del display usado en la ruta de ganancia suministrada.

## Integración propia del host

- Cuatro destinos automatizables, con Off, Instrument 1–8, ADSR 1–4 y FX 1–4.
- CC **números 1, 2, 3 y 4** fijos, combinando canales: prevalece el último evento
  recibido para cada número. Esta asignación y combinación son políticas propias;
  no se ha reconstruido el receptor/IPC físico del OP-1.
- Los eventos actualizan el latch en su timestamp de muestra y el valor se retiene
  hasta otro evento. No se atribuye este reloj al scheduler original.
- Prepare/pánico All Sound Off inicializan los cuatro valores a 64. Los CC vivos
  no forman parte del preset: se guardan los destinos y la activación, en proyectos,
  slots y clipboard. All Notes Off conserva los CC, como los controladores normales.
- Instrument se aplica a los controles suavizados y ADSR al contorno nativo cuando
  está habilitado. El sampler conserva sus límites de región acoplados. Drum/D-box
  rechazan ADSR; su dinámica propia no se sustituye por una envolvente synth.
- Las ocho opciones de FX admiten los cuatro destinos expuestos. Con motor propio
  heredado se puede usar MIDI FX; Instrument/ADSR requieren un motor nativo.
- Prioridad con flags simultáneos del host: MIDI, Random, Tremolo, Value Voice,
  Value FX y LFO propio. Los botones desactivan las otras opciones. Proxies de
  automatización cambian de destino también con el editor cerrado.
- La cola anunciada al host es conservadora para Release y para controles drum
  que pueden prolongar una voz. No se hacen reservas en el callback.

## Límites

Las rutas numéricamente contrastadas de ganancia, pitch y audio FX **no se exponen
en esta integración**. Modifican buffers suministrados; no son los productores
originales de esos buffers y no desbloquean el contorno de ganancia drum, L1 u OTP.
Tampoco recuperan el receptor físico, el ruteo global, Element, Bend ni Crank.
Bend (§15.8) es el accesorio físico manual: el pitch wheel MIDI existente no lo
implementa. No se afirma equivalencia acústica ni validación en Ableton Live real.

## Reproducción

El script `scripts/verify-midi246.py` exige el LDR rev246 privado con hash conocido,
assembler/linker Blackfin, testutils y simulador parcheado; el directorio de salida
debe ser nuevo y quedar fuera del checkout. `--record` solo guarda observaciones.
Una segunda ejecución sin esa opción debe coincidir exactamente. Las herramientas
y hashes figuran en [evidencia](firmware/midi246-validation.json).

La suite independiente requiere CMake 3.22, C++17 y Python 3.11 o posterior;
no necesita firmware ni JUCE:

```sh
cmake -S research/midi-modulation -B build-midi -DCMAKE_BUILD_TYPE=Release
cmake --build build-midi --config Release
ctest --test-dir build-midi -C Release --output-on-failure
```

El wrapper Python descomprime los fixtures en un directorio temporal; la comparación
y la integración se ejecutan como C++ nativo. El CI lo hace en Linux, MSVC y macOS
Intel. La ejecución nativa arm64 y la escucha comparada siguen pendientes.
