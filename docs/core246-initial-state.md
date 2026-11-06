# Core rev246: estado inicial y composición con eventos de batería

Bloque del 6 de noviembre de 2026, posterior a las palabras de evento de 0.54.0.
El instrumento conserva esa versión y su DSP. Se contrasta la inicialización
Core amplia y se compone con el workspace común, factory y note-on originales.
No se genera ningún instalador ni se integra esta inicialización como reset del
host. El productor de dinámica por muestra sigue pendiente.

## Qué cambia en el conocimiento del contrato

`0x01967fcc..0x01968766` no es una limpieza completa de Core. En cuatro ventanas
observadas fija **4.641 bytes** y conserva **1.215**. En particular:

| Dato | Resultado del inicializador amplio |
| --- | --- |
| Velocidad `0xff8012ec[12]` | Cero; son palabras de evento, no un stream de gain recuperado. |
| Pitch `0xff801304[12]` | Cero; no asigna el neutral del note-on `0x04000000`. |
| Controles drum `0x01aaadb4`, 8 × 24 × 8 int16 | Los 3.072 bytes a cero. |
| Modo `0xff801038`, ocupación `0xff801070[12]`, propietarios `0xff801334[12]` | Conservados; no equivalen a un banco vacío. |
| Doce objetos en `0xffb00000` y guarda posterior | Conservados, incluidos contadores. |
| Workspace común `0xff801724` | Conservado por este inicializador; lo pone a cero el inicializador separado ya contrastado. |
| Coeficientes/defaults Core y configuración `0x01bab53c` | Escrituras constantes bajo las entradas ROM fijadas; no se extrapolan a otras calibraciones. |

La observación abarca 5.856 bytes repartidos en las ventanas siguientes; no es
un trazado de todas las escrituras de la máquina ni de los scratch internos de
las rutinas soft-float.

| Inicio | Bytes observados |
| --- | ---: |
| `0xff800ff8` | 2.024 |
| `0x01aaabd4` | 3.560 |
| `0x01bab538` | 16 |
| `0xffb00000` | 256 |

Se comprueban los punteros al inicializador amplio y al pequeño en
`0x0100010c` y `0x01000110`. Esas referencias no prueban que el aparato ejecute
este orden en cada bloque ni recuperan la secuencia de arranque completa.

## Una cadena completa por caso

El harness ejecuta las instrucciones originales sin sustituir sus helpers:

1. Precarga las cuatro ventanas con cero, A5, FF o patrones variables.
2. Ejecuta el inicializador amplio y conserva una primera instantánea completa.
3. Ejecuta el [inicializador de workspaces](drwave246-control-workspace-init.md)
   `0x01968774`, con sus 82 bytes de escritura ya contrastados.
4. Suministra explícitamente bancos de controles y sonido, ejecuta factory tipo
   8 y prepara modo drum 1, ocupación/lista vacías, preview −1 y FX/LFO nulos.
5. Ejecuta note-on completo y captura otra instantánea, fila retenida y contador.

El paso 4 es parte del llamador de ensayo: el inicializador original **no**
recupera esos controles activos ni vacía ocupación u objetos por deducción. La
factory conserva los contadores previos de los objetos, como el estudio Core
anterior; el note-on usa la primera voz libre suministrada. No hay audio renderizado,
mezcla, buffers producidos de pitch/gain o calendario del instrumento físico.

## Reproducción acotada y evidencia

**256 cadenas**, con todos los 192 pares de ocho sonidos × 24 pads, las 256
transposiciones enteras −128…127 y velocidades firmadas. Cada caso compara los
5.856 bytes inicializados y, después de workspace/factory/evento, otros 5.856
bytes más 20 bytes de fila retenida y cuatro de contador. Los huecos, campos
parciales, propietarios, selección y guardas también participan en la comparación.

Dos extracciones independientes dan entradas y salidas numéricas idénticas. El
simulador pasa previamente los dos probes de los tres arreglos de writeback; se
comprueban el hash del firmware y el de la rutina. Firmware, ELF, desensamblado y
pseudocódigo permanecen privados.

[CoreInitialState246.h](../research/core-initial-state/CoreInitialState246.h)
contiene **185 spans de escrituras derivadas del oráculo** para las cuatro
ventanas. Es una reproducción de defaults bajo un dominio fijado, no una tabla
ROM recuperada ni una implementación independiente general del cálculo
soft-float. El comparador aplica esos spans y después reutiliza el helper de
workspace y los modelos de factory/note-on. La equivalencia no se amplía por ello
a calibraciones modificadas, estados fuera de las ventanas o ejecución de audio.

[Modelo y formato](../research/core-initial-state/README.md) ·
[Evidencia y hashes](firmware/core-initial-state246-validation.json).
Release y ASan/UBSan del comparador completos aprobados; CI nativo separado
comprueba Linux, Windows/MSVC y macOS Intel. No aumenta la cuenta de 145 suites
del producto: es una suite de investigación independiente.

## Consecuencia para la integración

No usar esta rutina como pánico, retirada de voces, recarga de parámetros por
bloque o generador de dinámica. Tampoco usar sus ceros como sustituto de las
entradas activas que faltan. El estado inicial ya puede reproducirse en los
siguientes oráculos con sus límites explícitos; la siguiente dependencia sigue
siendo el consumidor activo de velocidad/pitch y el llamador de render.

La estimación se mantiene **≈70 % ±10 puntos de planificación**. No hay medición
acústica nueva, validación de Live real o réplica de todos los presets originales.
Los instaladores quedan desactivados hasta una nueva petición del propietario.
