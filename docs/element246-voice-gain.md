# Element: ganancia de voz — 0.51.0

En LFO: Element el destino **Voice / FX Input** añade **Gain** (Orange 0).
Funciona en los diez motores melódicos nativos; batería/D-box y legacy muestran
OFF. Pitch (1) sigue pendiente. FX Mult/Sub (2/3) conservan sus rutas de 0.50.
Las cuatro fuentes, Amount bipolar, sensor, automatización, mandos sin editor y
proyectos/slots/clipboard están integrados. No es un volumen maestro.

| Fuente | Operación original sobre cada gain word `g` |
| --- | --- |
| Sensor | `sat16(g + Q15(sat16(-sensor), Amount))` |
| External Input | `sat16(g + Q15(external[i/2], Amount))`: cada muestra externa alimenta una pareja de gains. No usa pico positivo ni valor absoluto. |
| Voice Envelope / Gain Self | `sat16(g + Q15(g, Amount))`, sobre el gain actual de cada voz, sin agregado de envolventes. La pantalla identifica GAIN SELF. |
| Instrument Level | `sat16(g + Q15(level, Amount))`, usando la palabra Level explícita. No es el Self del bus FX. |

Amount cero conserva los gains. Los offsets Sensor/External/Level pueden elevar
una ganancia cero, reducirla o invertir su signo. La voz debe estar activa: no se
crean notas por conectar entrada externa. El lifecycle/gate y Release del host
siguen determinando cuándo termina la voz. Gain Self conserva cero y puede
saturar al aumentar Amount. Se conserva el orden Q15/suma, sin suavizado añadido.

## Contraste original

Callback synth `0x019cda14`, kind 0, destino 3, color 0: **8192 llamadas**,
cuatro fuentes y diez tipos synth, más rechazo drum/kind/destino/color.
Se comparan tag, ocho controles/targets, retorno, palabras de fuente y todos los
buffers de externa, gain, pitch y audio. **1540 llamadas procesadas**, 6652
rechazadas; **197120 gains** también coinciden con el puente por voz.
Dos extracciones independientes tienen entradas/salidas numéricas idénticas.

**385 llamadas Level procesadas retornan cero**. Ese retorno se reproduce en el
modelo y se separa del predicado de disponibilidad. Su consumidor original no
está recuperado: aplicar el buffer modificado en el host es una política híbrida,
no una afirmación sobre el instrumento completo. Pitch/color 1 se excluye del
oráculo; sus helpers y contrato se estudiarán aparte.

## Captura y productores propios

Sensor y Level usan el frontend global ya integrado y sus ventanas 44100/64.
Level sigue siendo el pico absoluto del instrumento de la muestra anterior,
retenido por ventana; no se recupera su productor físico/IPC. Gain Self opera
sobre el gain que proporciona cada backend, sin usar el máximo ADSR del frontend.
El indicador Element sigue siendo nominal, no un medidor individual de ganancias.

External tiene un remuestreador causal propio a 44,1 kHz, con antialias,
interpolación, cuantización Q15 firmada y un historial circular de 64 muestras.
Cada voz captura ese historial al comenzar su bloque de 128 gains, y usa cada
muestra dos veces. La captura queda estable durante el bloque y se renueva en
el siguiente. La historia es pasada: puede introducir desfase/retardo de
modulación respecto a un productor original desconocido. No se añade latencia
al audio ni cambia el PDC existente. No necesita monitorización directa.

Activación/pánico/prepare limpian filtros/historia; notas y polifonía comparten
la fuente pero capturan bloques independientes. Se conserva la prioridad de
Element sobre Crank/Bend/MIDI y el resto de LFO. Cambiar de propietario de captura
reinicia el bloque, sin reutilizar gains/audio de otro modulador. La historia y
los gains vivos no se guardan como controles de proyecto.

Clock, SRC, captura por voz, inicialización, ubicación respecto a ADSR/velocidad,
composición y lifecycle son adaptaciones del host. No se recuperan allocator,
scheduler, productor original de gains, dinámica drum, lectura física o
calibración acústica. Envelope auxiliar permanece fuera del plugin sin contrato
activo. Ableton real, ARM64 nativo, firma/notarización y presets completos no se
validan por este bloque. [Resultados y CI](releases/v0.51.0-validation.json).

## Reproducción

```
cmake -S research/element-gain -B build-element-gain -DCMAKE_BUILD_TYPE=Release
cmake --build build-element-gain --config Release --parallel 2
ctest --test-dir build-element-gain -C Release --output-on-failure
```

`verify-element-gain246.py` requiere LDR privado y las herramientas con los tres
parches de writeback verificados. Reutiliza las entradas de `verify-element246.py`;
salidas fuera del checkout, salvo fixtures numéricos con `--record`. No se
redistribuyen firmware, ELF, volcados completos ni pseudocódigo descompilado.
