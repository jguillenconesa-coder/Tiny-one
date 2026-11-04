# Envelope → Element: composición contrastada y orden pendiente

Se compone el [publicador recuperado](element246-producers.md) con los consumidores
originales de Element **sin inyectar una palabra publicada entre ambos**. El
publicador procesa el audio y actualiza `0x01dfc80c`; los consumidores leen su
halfword superior mediante el getter original. El plugin continúa en **0.43.0**
y conserva su adaptación propia: esta composición es investigación, no una nueva
ruta audible ni una recuperación de la llamada activa.

## Qué se ejecuta y compara

El driver propio ejecuta los cuerpos originales completos, sin parches:

1. Carga o conserva uno de cuatro estados artificiales del llamador y la palabra
   compartida; aplica el reset publicado cuando lo exige la entrada sintética.
2. Ejecuta Element con la publicación **anterior** al bloque de audio.
3. Ejecuta el publicador `0x01a66922` sobre 128 halfwords de audio.
4. Ejecuta el mismo consumidor Element con la publicación **posterior**, recargando
   los mismos controles y destinos iniciales para hacer comparable el resultado.

Ese orden es una elección explícita del ensayo. No demuestra el orden del OP-1.
Los controles del publicador y los controles de Element son entradas separadas;
no se ha recuperado su relación con la interfaz del instrumento.

Se contrastan **4.096 bloques / 524.288 muestras y 8.192 callbacks Element**:

| Consumidor | Alcance |
| --- | --- |
| Drum `0x019cde28` | Fuente 2, tipos 8/9, kinds 0–3 y los ocho índices de color; destinos aceptados y rechazados. |
| FX `0x019cdf22` | Fuente 2, kind 5, los ocho tipos FX e índices de color; destinos aceptados y rechazados. |

Hay **2.048 despachos aceptados**; se exige cobertura de cada combinación drum
(tipo/kind) y cada FX. Se comparan los controles completos, tag, ocho destinos y
retorno de ambos consumidores, además del pool completo, publicación, getter,
controles de productor, audio, guard y acción antes/después del consumidor final.
Los consumidores ensayados conservan ese estado y audio del publicador. La
publicación compartida sobrevive entre bloques y llamadores de prueba; no se
supone que estos sean cuatro voces originales.

Se mantienen los extremos signed, impulsos, ruido, silencio, cambios de controles,
111 resets y secuencias continuas del ensayo del publicador. El driver invoca
4.096 getters explícitos y los 2.048 despachos aceptados pasan por el getter de
los consumidores. Una segunda ejecución original independiente es idéntica byte
a byte. El port C++ coincide con los 704 bytes observados por bloque; ASan/UBSan
aprueban la composición. [Evidencia](element246-composition-validation.json).

## Resultado que condiciona la integración

En **250 bloques**, los destinos calculados antes y después del publicador son
diferentes, aun con los mismos controles y destinos de partida. Es un resultado
del conjunto sintético, no una proporción de presets ni una medición de latencia
del aparato. Demuestra que elegir libremente el punto de lectura puede modificar
la modulación. La coincidencia de cada núcleo por separado no valida su cadencia.

No se sustituye el máximo ADSR del host por esta composición hasta determinar qué
audio recibe el productor, quién posee e inicializa su estado, qué controles lo
alimentan y cuándo lo lee el scheduler original. No hace falta escuchar presets
para resolver este contrato; la comparación acústica sigue siendo otro trabajo.

## Búsqueda acotada de la llamada activa

En las imágenes privadas SDRAM y L1 disponibles se inspeccionaron literales de
32 bits, candidatos de carga inmediata de la mitad baja y codificaciones de CALL
relativo alineadas a dos bytes hacia escritor/reset/getter. Los cinco CALL de
SDRAM encontrados apuntan al getter (`0x019cdf00`, `0x019ce030`, `0x019ce118`,
`0x019ce252`, `0x019ce398`), no al escritor.

El literal `0x01a66bd8` de `0x010003e0` pertenece a la tabla de arranque. La carga
con mitad baja `0x6bd8` en `0x01a2e202` forma `0x018f6bd8` con su mitad alta:
no identifica otra llamada al reset. Tampoco la coincidencia baja `0x108a` en
`0xffa04932` demuestra un acceso al vecino de los escalares de Element: la carga
siguiente completa `0x92b1108a` en la imagen disponible.

Una revisión posterior de accesos con base/desplazamiento en SDRAM y L1 tampoco
ha localizado un escritor activo verificable de las otras tres palabras. El
análisis de constantes fue conservador: invalida información en ramas, llamadas,
loops e instrucciones no interpretadas; no prueba ausencia de escritores.

Esta búsqueda **no localiza la llamada activa** y no demuestra que no exista.
No cubre todas las construcciones por offsets, tablas o alias ni ejecución que
no pueda interpretarse en estas imágenes. No se atribuye por ello el llamador a
L1 cifrada ni se identifica el productor como compresor maestro. Los otros tres
productores conservan sus límites en [el estudio anterior](element246-producers.md).

## Reproducción

El mismo proyecto nativo de investigación ejecuta ahora dos suites: publicador
individual y composición. No necesita JUCE ni firmware:

```sh
cmake -S research/element-producers -B build-publisher -DCMAKE_BUILD_TYPE=Release
cmake --build build-publisher --config Release
ctest --test-dir build-publisher -C Release --output-on-failure
```

Para regenerar el oráculo privado se añade `--composition` a
`scripts/verify-envelope-publisher246.py`, con el LDR rev246, binutils/testutils y
simulador corregido ya documentados. `--record` registra únicamente entradas
sintéticas y observaciones numéricas; repetir sin él exige igualdad. Los
[hashes](firmware/envelope-publisher246-element-validation.json) identifican los
fixtures. No se distribuyen firmware, volcados originales ni código descompilado.
