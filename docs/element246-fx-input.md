# Element: Multiply/Subtract en la entrada FX — 0.50.0

En LFO: Element, White ofrece **FX Input** y Orange los targets **FX Mult**
(índice 2) y **FX Sub** (3). Se aplican antes de Filter, Nitro, Delay, Grid,
Phone, Spring o CWO. Punch está en otra etapa: estos targets aparecen como OFF
si solo hay Punch o el bus está en bypass. Los targets 0/1 y 4–7 se rechazan.
Instrument, ADSR y FX parámetros conservan sus rutas; los mandos contextuales,
automatización, proyectos, slots y clipboard incluyen el nuevo destino.

| Fuente Blue | Entrada a las curvas FX |
| --- | --- |
| Sensor | Palabra firmada del sensor virtual, sin negación; el signo difiere de la ruta de parámetros. |
| External Input | Audio externo firmado por muestra, con Input Gain; no necesita monitorización directa. No usa pico positivo, RMS ni valor absoluto. |
| Voice Envelope | Escalar de envolvente. El host mantiene su mapa propio a un cuarto de escala y el bus kind 2. |
| Level / Self | Producto del audio de entrada consigo mismo; el callback no lee la palabra publicada de Level. La pantalla muestra SELF. |

**Multiply** reemplaza la excitación del FX por el producto Q15 correspondiente.
Para Sensor/External: `Q15(Q15(audio, fuente), Amount)`; para Self:
`Q15(Q15(audio, audio), Amount)`. Envelope kind 2 calcula primero
`factor = sat16(4 * Q15(envelope, Amount))` y luego `Q15(audio, factor)`.
El kind 5 contrastado usa la palabra drum sin expansión por cuatro, pero no se
crea un segundo bus drum en el host. El orden de redondeo/saturación se conserva.

**Subtract** resta ese mismo producto al audio con saturación. No hay XOR y no
son las curvas de Bend/Crank. Amount admite ambos signos. **Amount cero silencia
la excitación en Multiply y conserva el audio en Subtract**, antes del efecto.
No silencia automáticamente memorias/colas ya guardadas ni es un volumen maestro.
Ambas curvas conservan el silencio si el audio que entra al bus es cero.

## Contrato original y retorno

Se reconstruye el callback FX `0x019cdf22`, kinds 2/5, destino 3, colores 2/3.
**8.192 llamadas originales** comparan tag, ocho controles/targets, retorno,
palabras completas de sensor/envolvente/nivel y todos los buffers de entrada,
ganancia, pitch y audio. La matriz de procesamiento cubre 128 celdas:
cuatro fuentes × ocho tipos FX × dos kinds × dos curvas, más rechazos.
Los buffers de audio incluyen extremos firmados exactos y los dos órdenes de
polaridad. Dos extracciones independientes tienen entradas/salidas idénticas.

**3.494 llamadas procesan audio** (223.616 muestras); 4.698 se rechazan.
De las procesadas, **874 de Self devuelven cero** pese a transformar el buffer.
Se conserva ese retorno exacto y se separa del predicado de disponibilidad;
contar solo retornos no nulos infravaloraría las rutas procesadas. El consumidor
original de ese retorno no está recuperado: aplicar Self en el bus del host es
una política híbrida explícita, no una afirmación sobre el instrumento completo. Los otros
64 halfwords del buffer FX y los buffers de fuente/pitch/gain quedan intactos.
El puente del host, kind 2, se compara también con **111.936 muestras originales**.
Kind 5 está contrastado en el modelo, sin presentarlo como integración de otro bus.

## Integración propia y límites

La entrada externa tiene un camino de remuestreo independiente, alineado con la
entrada mono del bus: los mismos coeficientes de antialias y reloj causal del
adaptador, interpolación y cuantización Q15 a 44,1 kHz. Conserva muestras
negativas y funciona sin monitorización directa. Reset/pánico reinician sus
filtros y muestra anterior. Las entradas externas no finitas se sustituyen por
cero. No se reutiliza el máximo positivo de la ruta antigua de parámetros.

Sensor y Envelope conservan el frontend global del host y sus ventanas 44100/64.
La agregación de envolventes, escala propia, fuente externa del host, clock,
ubicación tras mezcla/cinta/EQ y antes del núcleo mono, automatización por bloque
y lifecycle son adaptaciones. El indicador de fuente mantiene el frontend de
Element; no es una medición del audio transformado del bus.

Se conserva Element > Crank > Bend > MIDI y el resto de prioridades, bypass,
latencia/PDC y transición seca. No se recuperan productores físicos/IPC, allocator,
scheduler, dinámica drum, voz completa o equivalencia acústica. Pitch de Element y auxiliares FX kind 4 siguen pendientes.
[Gain se integra por separado en 0.51.0](element246-voice-gain.md). El publicador Envelope original
permanece fuera del plugin sin contrato activo verificable. Ableton real, ARM64
nativo, firma/notarización y todos los presets originales no se validan aquí.

## Reproducción

```
cmake -S research/element-fx -B build-element-fx -DCMAKE_BUILD_TYPE=Release
cmake --build build-element-fx --config Release --parallel 2
ctest --test-dir build-element-fx -C Release --output-on-failure
```

`verify-element-fx246.py` usa LDR privado con hash conocido y los tres parches de
writeback verificados del simulador BF524. Importa el generador de entradas de
`verify-element246.py`; escribe fuera del checkout, salvo fixtures numéricos con
`--record`. No se distribuyen firmware, ELF ni pseudocódigo descompilado.
[Resultados y CI](releases/v0.50.0-validation.json).
