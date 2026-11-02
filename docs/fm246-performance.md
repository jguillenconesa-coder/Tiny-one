# FM 0.28.0 — afinación, moduladores e interpretación

FM Hybrid incorpora Value Instrument/ADSR, Random y Tremolo, además de una
extensión propia de pitch fraccionario que elimina el escalón de afinación del
adaptador 0.27.0. Se mantienen las nueve rutas y las diez constantes sustituidas
explícitamente. No se afirma equivalencia acústica con el OP-1.

## Núcleo original y extensión de afinación

El callback original lee `0xff801950` y usa su mitad alta: la fracción baja no
interviene en los incrementos. El adaptador 0.27.0 redondeaba esa palabra, dando
pasos de unos 1.346 Hz al reloj propio de 88.2 kHz. Esto podía desafinar los graves.
No hemos recuperado el scheduler L1 que suministra ese pitch en el aparato.

Se conserva `reference::fm246Block` como reconstrucción exacta bajo el modelo de
tabla publicado. `fm246HybridBlock` es una **extensión nuestra**: conserva el
incremento completo de la portadora e interpola los incrementos con signo de los
otros operadores entre dos evaluaciones adyacentes del cálculo original. No
interpola audio ni cambia el reloj por nota. Feedback, niveles, evolución interna,
orden de los operadores y scratch conservan sus operaciones.

Cuando la fracción es cero, el adaptador usa el callback exacto. Ese camino se
compara en los 4096 registros originales: 524288 muestras, estados y scratch
completos coinciden. El camino fraccionario **no es aritmética recuperada** ni se
presenta como coincidencia con el firmware. Corrige la afinación musical del
adaptador; sus posibles diferencias tímbricas necesitan referencia acústica.

La medición de periodos de la salida real de Engine, con Blue=0, cubre MIDI
24/36/48/60/69/81/93/105, bend −2/0/+2 semitonos y 44.1/48/96 kHz: **72 casos**,
error máximo observado **0.00691601 cents**. Es una comprobación de afinación del
software frente a A440, no de la afinación física del OP-1 ni de todos los timbres.

## Rangos y rutas originales de modulación

El inicializador original proporciona los ocho descriptores FM en `0x01de5890`.
Los cuatro primeros son `(mínimo, máximo, escala, wrap)`:

| Control | Descriptor |
| --- | --- |
| Azul | 0, 32767, 32767, 0 |
| Verde | 0, 32767, **10485**, 0 |
| Blanco | 1024, 17408, 16384, 0 |
| Naranja | 0, 32767, 32767, 0 |

No se sustituye la escala verde por todo el rango. Los otros cuatro descriptores
tienen escala cero. La comparación específica de FM cubre **4096 llamadas Value**
y **4096 Random**, con estados completos, display/RNG, ocho destinos, retornos,
reinicios, secuencias y destinos inválidos. Las dos repeticiones privadas coinciden
byte a byte. Estos ensayos no necesitan las diez constantes del render FM.

Value y Random usan el tipo original FM `0`, tanto hacia Instrument como hacia
la ADSR. Se conservan prioridad Random → Tremolo → Value Voice → Value FX, fase
FREE/retrigger, RNG compartido y desactivación/pánico. Los relojes y el orden de
composición con las voces siguen siendo adaptadores del host.

Tremolo calcula sus 128 palabras de pitch con la aritmética ya contrastada y el
FM toma la primera para ese bloque; la amplitud procesa las 128 muestras en la
misma época. Activarlo a mitad de un bloque almacenado espera al siguiente bloque
para empezar ambas ramas juntas. La interfaz muestra la ruta efectiva de la
última voz, incluyendo la modulación del control blanco.

Set 3/LFO permite activar los moduladores sobre FM con los mismos parámetros y
proxies estables que los demás núcleos. El estado se guarda y restaura con el
editor cerrado. No se reconstruyen por ello Element, Bend, Crank ni el ruteo
completo original. Los FX de salida conservan su cadena propia.

## Interpretación y validación

Las pruebas de Mono/Legato/glide incluyen ahora FM en seis tasas: prioridad de
última nota, canales independientes, pedal, retorno de notas retenidas, relevo,
reinicio, pánico y cambios de modo/motor. La trayectoria de glide sigue siendo
lineal en semitonos; la política no se atribuye al scheduler original.
Se comprueban neutralidad de profundidad cero, cuatro colores Value hacia ambas
rutas, cinco formas Tremolo, Random Instrument/ADSR y cambios durante el audio.
Los renders del plugin con buffers 64/257 deben coincidir, también con moduladores.

[Evidencia de entrega](releases/v0.28.0-validation.json) ·
[Oráculos y alcance](firmware/fm246-performance-validation.json).

Para reproducir sin JUCE se mantiene el proyecto `research/fm-render`, que incluye
ahora las comparaciones FM Value/Random. Para regenerar los oráculos privados:
`scripts/verify-value-voice246.py --fm` y `scripts/verify-random-voice246.py --fm`,
con LDR/herramientas y salida nueva fuera del checkout, como los otros verificadores.
Los vectores publicados contienen entradas sintéticas y observaciones numéricas;
no se distribuyen instrucciones, firmware, medios privados ni pseudocódigo.

## Referencias reales y siguiente calibración

El chat anterior identificó los segmentos definitivos **14 y 15** del vídeo
aportado; sus 16 selecciones FM se conservan en el índice. Esta entrega comprueba
el inventario numérico, sin afirmar una nueva verificación o escucha de los medios.
El [triaje reproducible](firmware/fm246-listening-readiness.json) encuentra **cero**
presets con FX y LFO simultáneamente apagados. Además, los registros almacenados
no seleccionan las palabras de ratio protegidas **12 y 25**. No basta con comparar
los 16 nombres para identificar las diez constantes desconocidas.

El vídeo sirve para orientar el carácter; sigue faltando controlar notas,
velocidad, pulsaciones, FX/LFO y cadena de grabación. No se ha calibrado ningún
coeficiente ni ajustado un preset a ese audio en esta entrega. Si en el futuro hay una referencia seca, un primer piloto sería Big Bottom, Sub Bass y Yoshonka, con FX/LFO apagados,
MIDI 36/48/60/69, velocidad 100, pulsaciones de dos segundos y un segundo de pausa.
Sería un piloto parcial: después harán falta configuraciones específicas para
las constantes no excitadas por esos patches. La réplica completa de presets
originales sigue reservada para el final. Estimación global: **51 % ±10 puntos**
de planificación; no un porcentaje de fidelidad sonora.

El propietario confirma que solo dispone de los vídeos y presets aportados.
Se continuará con ellos para comparación orientativa, sin requerir grabaciones
nuevas ni atribuir a las constantes diferencias causadas por FX, LFO o interpretación.
