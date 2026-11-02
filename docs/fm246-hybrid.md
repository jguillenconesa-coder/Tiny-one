# FM Hybrid 246 — cuatro operadores (0.27.0)

El FM de dos osciladores se sustituye por un núcleo de cuatro operadores
reconstruido y contrastado con el callback mono de rev246. No basta con duplicar
osciladores: se reproducen nueve rutas, realimentación por operador, aritmética
Q15, fases con wrap y seis evoluciones internas de frecuencia/nivel.

La comparación aislada pasa **4096 bloques / 524288 muestras**, incluyendo los
60 bytes de voz, las 30 palabras de tablas, 512 bytes de scratch y dos guardas
por llamada. Se prueban estados independientes y secuencias conservadas, cambios
de ruta, límites y selectores inválidos. Dos ejecuciones privadas producen
vectores idénticos. Los originales solo se ejecutan en un simulador GNU BF524 con
ambos parches de writeback ya contrastados; no se afirma validación en hardware.

**Límite de procedencia:** se conservan veinte constantes legibles y se usan
diez sustituciones propias declaradas en [el modelo](../research/blackfin/fm246-table-model.json).
La coincidencia del render está condicionada a esa tabla; no recupera las diez
constantes protegidas ni demuestra equivalencia acústica. [Base y procedencia](fm246-foundation.md).

## Uso y controles

Seleccionar **01 / FM**, con Digital 246 desactivado. La pantalla muestra cuatro
operadores A–D y el grafo de la ruta seleccionada. La inicialización es propia;
no se presenta como un preset de fábrica original.

| Set 1 | Parámetro estable del host | Rango crudo | Función |
| --- | --- | --- | --- |
| Azul | `fm246Blue` | 0..32767 | Niveles de modulación y realimentación cúbica por encima del umbral 8192 |
| Verde | `fm246Green` | 0..32767 | Tres índices de ratios, bits 12–14, 9–11 y 6–8; los seis bits bajos no intervienen |
| Blanco | `fm246White` | 1024..17408 | Nueve rutas, selector `word >> 11` |
| Naranja | `fm246Orange` | 0..32767 | Evolución interna y desplazamiento de frecuencias; seis canales gain/angle |

Set 2 usa `fm246Attack/Decay/Sustain/Release`, con la ADSR reconstruida que ya
comparten los demás núcleos. Su curva no es la ADSR anterior del FM propio.
Poly usa seis voces; Mono y Legato tienen prioridad de última nota y glide.
La asignación, el relevo con rampa de 3 ms y la política de notas son adaptadores
propios. Cambiar entre FM y Digital limpia las voces incompatibles.

Los ocho parámetros crudos se automatizan y persisten en el estado, slots y
clipboard mediante el árbol común. Los cuatro proxies de pantalla siguen FM o
Digital sin necesitar que el editor esté abierto. Las recetas FM propias se
adaptan a los nuevos controles; **su sonido cambia y no son presets originales**.
La compatibilidad sonora con el FM de dos operadores no se conserva.

## Rutas del núcleo

A es el operador de salida principal; B/C/D aplican sus niveles Q15 antes de
modular o sumarse. Todas las rutas conservan además realimentación propia en
cada operador. Las sumas respetan saturación y orden originales.

| Ruta | Conexiones entre operadores | Salida |
| --- | --- | --- |
| 1 | D→C→B→A | A |
| 2 | (D+C)→B→A | A |
| 3 | (D→C)+B→A | A |
| 4 | D→(C+B)→A | A |
| 5 | D→C→(B+A) | Media de A y B ponderado |
| 6 | (D+C+B)→A | A |
| 7 | D→B y C→A | Media de A y B ponderado |
| 8 | D→C, D→B, D→A | Media de A y suma ponderada B+C |
| 9 | Cuatro operadores en paralelo | Media de A y suma ponderada B+C+D |

No se intercambian simplemente aristas: las rutas 6, 7 y 9 cambian también la
asignación de bancos/contornos a operadores. Los índices y orden exactos están
cubiertos por los vectores; no se infieren por la apariencia del gráfico.

## Qué conserva el adaptador y qué sigue pendiente

- El kernel procesa 128 muestras por bloque con estados y scratch por voz.
  El reloj fijo de **88.2 kHz**, la llamada por bloque y el suministro de pitch
  son propios; no se ha recuperado el scheduler completo L1.
- El núcleo consume la mitad alta del pitch. El adaptador redondea esa palabra:
  la resolución es `88200/65536 ≈ 1.346 Hz`. A4 queda aproximadamente en 440.093 Hz;
  el error relativo puede crecer en graves. Glide/pitch bend se actualizan por
  bloque. No se oculta esta limitación como afinación física exacta del OP-1.
- Se usa el filtro causal propio de cuatro polos y la conversión de tasa
  compartida con Phase/Digital. El Tone anterior deja de colorear implícitamente
  el FM. Drive, eco y FX seleccionados siguen siendo etapas explícitas del host;
  volumen maestro y PDC del bus permanecen según [output-routing.md](output-routing.md).
- Los seis canales internos y la ADSR están activos. Random, Tremolo y Value
  Instrument/ENV todavía no se conectan al FM; la UI los deshabilita. El LFO común
  y los efectos de salida conservan su ruta propia. No se promete el ruteo original.
- Falta recuperar/calibrar las diez constantes, el pitch/reloj/allocator completo,
  comparación acústica con el aparato y validación actual en Ableton macOS/Windows.
  Más oversampling o menos aliasing por sí solos no demuestran mayor fidelidad.

## Reproducción

Sin firmware ni JUCE:

```sh
cmake -S research/fm-render -B /tmp/pocket-fm-render -DCMAKE_BUILD_TYPE=Release
cmake --build /tmp/pocket-fm-render --config Release --parallel 2
ctest --test-dir /tmp/pocket-fm-render -C Release --output-on-failure
```

Para regenerar los vectores, usar `scripts/verify-fm246-render.py` con los mismos
argumentos de herramientas/LDR que el verificador de la base FM y una salida
privada nueva fuera del checkout. El verificador ejecuta primero las guardas de
procedencia y microtests del simulador de esa base. `--record` escribe únicamente
entradas sintéticas y observaciones numéricas, nunca firmware o desensamblado.
[Formato](../research/fm-render/README.md) y [evidencia](firmware/fm246-render-validation.json).
