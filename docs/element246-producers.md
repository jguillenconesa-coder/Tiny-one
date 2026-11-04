# Productores de entrada de Element — publicador contrastado

Se reconstruye un escritor legible de la palabra `0x01dfc80c`, cuyo halfword
superior consume la fuente Envelope de drum y FX kind 5. El núcleo queda en
[research/element-producers](../research/element-producers/EnvelopePublisher246.h),
con pruebas reproducibles independientes del plugin. **La aplicación permanece en
0.43.0 y conserva su frontend de Element documentado como propio.**

## Hallazgo y contrato

| Rutina | Observación |
|---|---|
| `0x01a66922` | Procesa 128 halfwords de audio in-place, actualiza dos palabras de estado del llamador y escribe `0x01dfc80c`. |
| `0x01a66912` | Copia el halfword superior de la palabra publicada al destino; conserva la mitad superior del word que contiene ese destino. |
| `0x01a66bd8` | Pone a cero la palabra publicada; no reinicia el estado independiente del llamador suministrado en el ensayo. |

Se ejecutan las rutinas originales completas **sin parchear su código** en el
simulador BF524 corregido. El driver es propio; suministra estado, controles y
buffer, igual que en los oráculos anteriores. Se contrastan **4.096 bloques /
524.288 muestras**. Cuatro estados artificiales del llamador se intercalan con
**una misma palabra publicada**; no se infiere que el aparato tenga cuatro voces.

Se suministran buffers alineados y separados; no se ensayan alias de estado,
controles y audio. Los cuatro estados del pool son entradas artificiales, y los
valores cero de la API no identifican un constructor original.

La comparación incluye las ocho palabras del pool de prueba, publicación completa,
getter con su mitad superior preservada, cuatro controles sin cambios, los 128
halfwords de audio resultantes, acción e integridad del guard del pool. Se ejecutan
111 resets y 4.096 getters. Hay inyecciones de estados, controles signed completos,
extremos, impulsos, ruido, cambios y secuencias continuas con silencio. Una segunda
ejecución independiente exige igualdad byte a byte.

El port independiente conserva:

- Aritmética Q31 truncada, sumas/restas/desplazamientos saturados y ABS16 saturado.
- Coeficientes derivados de la cuarta palabra, incluido el término de quinta
  potencia; no usa una curva de sexto grado ni coeficientes de coma flotante.
- Dos seguimientos por llamador, diferencia escalada y respuesta por ramas según
  el signo, usando las tres primeras palabras de control.
- Publicación compartida suavizada y transformación del audio, con saturación final.
- Estado del llamador separado de la publicación global: reiniciar uno no se
  presenta como política original de reinicio del otro.

## Por qué no se sustituye aún el frontend

En **1.684 bloques originales con audio silencioso**, el getter publicado es
positivo. Con controles sintéticos que anulan las pendientes y dejan una base
positiva, el valor converge a una constante aunque no exista señal de audio.
Se comporta, por tanto, como una **publicación de ganancia calculada**, no como
una lectura directa de amplitud ni una ADSR que necesariamente sea cero en silencio.
Esto no identifica por sí solo su nombre comercial o sus controles de interfaz.

Falta localizar y contrastar la llamada activa: qué audio recibe, en qué momento
se ejecuta respecto a Element, quién inicializa/posee los dos estados, cuáles son
los controles/rangos predeterminados y cuál es su cadencia y ruteo. No se atribuye
esa ausencia automáticamente a una rutina L1 cifrada. Tampoco se presenta esta
función como el compresor maestro sin demostrar esa conexión.

Sustituir el máximo ADSR del host por este valor calculado con un audio o unos
controles elegidos por nosotros añadiría otra hipótesis. Por eso este bloque entrega
el núcleo contrastado, su API con publicación compartida explícita y sus pruebas;
la integración audible espera ese contrato de llamada. Los adaptadores actuales
continúan identificados como propios en [Element 0.43.0](element246-hybrid.md).

## Resto de entradas

| Palabra | Resultado de este bloque |
|---|---|
| `0xff801056` | Continúa pendiente el productor. `0x0198d5aa` consume el par `0xff801054/56` para otros cálculos; no es un escritor del sensor. |
| `0xff80108c` | Productor sin localizar. La coincidencia de mitad baja en `0x01978246` no es un acceso a este word: la siguiente carga de mitad alta forma `0x01cc108c`. |
| `0xff80108e` | Productor sin localizar; no se sustituye por un nuevo detector bajo un nombre original. |
| `0x01dfc80c` | Escritor, reset y getter contrastados bajo sus entradas explícitas; contrato de llamada activa pendiente. |

Estas son conclusiones acotadas de las rutas inspeccionadas, no una prueba de
que no existan otros escritores o alias. Las anotaciones de registros del
objdump pueden conservar mitades anteriores: se comprueban ambas cargas y el
acceso efectivo antes de identificar una dirección.

## Reproducción y límites

```sh
cmake -S research/element-producers -B build-publisher -DCMAKE_BUILD_TYPE=Release
cmake --build build-publisher --config Release
ctest --test-dir build-publisher -C Release --output-on-failure
```

C++17, CMake 3.22 y Python 3.11 bastan para el ensayo nativo, sin JUCE ni firmware.
[Resultados y CI](element246-producers-validation.json) · [hashes del oráculo](firmware/envelope-publisher246-validation.json).
Los fixtures gzip contienen entradas sintéticas y observaciones numéricas; no
firmware, código descompilado ni un volcado completo de RAM original.

`scripts/verify-envelope-publisher246.py` requiere el LDR privado rev246 con hash
conocido, binutils/testutils Blackfin y el simulador con los tres parches de
writeback contrastados. Exige un directorio nuevo fuera del checkout; `--record`
registra observaciones y repetir sin él exige igualdad. Los binarios de firmware
solo se reconstruyen en ese directorio privado y no se añaden al repositorio.

La coincidencia está condicionada por estados, controles, buffers y secuencia
suministrados. No demuestra scheduler/voz/ruteo del instrumento completo, tasa de
muestreo del llamador, formato mono/estéreo, detección física ni equivalencia acústica.
No se cambia la versión del producto ni se generan instaladores por este bloque.
