# Spring 246 — reverb mono contrastado e integrado

**Set 3 → FX → Spring 246** añade Tone, Turns, Damping y Send al instrumento.
Los cuatro controles admiten automatización, Value FX, encoders contextuales
con el editor cerrado y guardado en proyectos, slots y LIFT/DROP. Los valores
iniciales son una elección del adaptador; no se añade un banco de presets original.

| Control | Rango raw | Consumo del callback |
| --- | --- | --- |
| Tone | 1344–16448 | Índice de frecuencia `Tone/128`, limitado para que Tone/128 + Turns/128 no supere 220. |
| Turns | 7744–16448 | Coeficiente de paso bajo de la tabla compartida, índice `Turns/128`. |
| Damping | 0–16384 | Cubo Q15 redondeado; su complemento controla la salida de las líneas filtradas. |
| Send | 0–32767 | Cuadrado Q15 para la excitación. Se conserva la señal seca y se suma la reverberación. |

## Evidencia del núcleo

Se ejecutan el inicializador estático `0x01a33964`, el callback mono
`0x01a326e8` y el reset `0x01a32e9a` con firmware rev246 aportado por el operador.
El inicializador produce las cuatro constantes consumidas por el render:
**29491, 3276, 8192 y 26214**. No se sustituyen por aproximaciones. Las tablas
compartidas se generan con sus conversiones legibles: 129 entradas de frecuencia
Q31 y 129 de paso bajo Q15. Se contrastan sus 258 entradas y el prefijo de 16 bytes
del inicializador; dos campos auxiliares del prefijo no se usan como controles.

La reconstrucción conserva trece historiales, nueve cursores y cuatro valores
de feedback. Las nueve líneas tienen **382, 1272, 5330, 3418, 1848, 5902, 596,
3490 y 6898 muestras**. Se respetan los taps, las sumas saturadas por etapa,
productos Q15 con redondeo par, fracciones Q31 y orden de realimentación.
Los high words se extraen mediante bits explícitos para evitar las derivas de
compilador encontradas en entregas anteriores.

El oráculo contiene **4.096 bloques / 262.144 posiciones de audio**: 4.088 bloques
activos (261.632 muestras procesadas) y ocho callbacks pendientes que conservan
la entrada. Incluye cuatro resets originales sobre memoria retenida, cuatro
cargas de estados/workspaces sintéticos, silencios, impulsos, DC negativo,
alternancia extrema y controles mínimos/máximos. Coinciden todas las muestras,
estados de 104 bytes, display de 64 bytes, contador de borrado y ocho capturas
completas de las 29.136 celdas. Una segunda ejecución privada independiente
reproduce byte a byte las fixtures.

El dominio acotado usa **slot 0**, workspace explícito de 58.272 bytes y cursores
válidos. No ejecuta el allocator, scheduler ni arranque completo del instrumento.
El [ensayo previo de ciclo de memoria](spring246-lifecycle.md) verifica por
separado ambos slots y el borrado diferido. El código original y los artefactos
ejecutables/decompilados permanecen privados; se comparten harness propio,
entradas sintéticas, resultados y [evidencia](firmware/spring246-validation.json).

## Adaptación al host

Spring usa el bus mono compartido de salida, después de cinta/entrada/EQ y drive/
eco maestro y **antes del volumen maestro**. Prioridad si el host activa varios
flags: Spring > Phone > Grid > Delay > Nitro > Filter > Punch. No se reconstruye
por ello el ruteo estéreo original. [Cadena](output-routing.md).

Las tasas del host se adaptan al núcleo de 44,1 kHz y bloques de 64 muestras. El
PDC permanece en `ceil(64 × tasa_host / 44100) + 1`; el retardo de las líneas es
parte musical del reverb. Resampling, DC de 15 Hz, guarda inicial de silencio y
transición de bypass de 12 ms siguen siendo propios del adaptador compartido.

Activación, cambio de efecto y pánico usan un **reset completo propio**: cero en
historiales y todas las líneas, sin borrado pendiente. Se distingue del reset
original, que conserva historiales y memoria intermedia y devuelve la entrada
durante dos callbacks de limpieza. El oráculo contrasta ambas ramas originales;
la política del host evita reutilizar memorias de otro efecto.

La integración comprueba siete tasas (8–192 kHz), bypass retrasado exacto,
8.192 muestras frente al núcleo directo a tasa nativa, cola tras cesar la entrada,
controles extremos, ausencia de underruns, CC120, selector/Configure/proxies y
renders 64/257. La regresión de volumen incluye Spring y las cargas de tiempo
real incluyen sus cuatro motores nativos con Value FX y automatización de todos
los controles/enable. [Validación de entrega](releases/v0.25.0-validation.json).

La equivalencia acústica con un OP-1 físico y Ableton Live real en ambos sistemas
siguen pendientes. Siguiente componente: productores, auxiliares y render de CWO;
presets originales completos al final.
