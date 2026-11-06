# Core rev246: reconstrucción conjunta de audio, FX y moduladores

Se contrasta completa la rutina `0x01967b28..0x01967ccc`, denominada aquí
**refresco Core** por las tablas/objetos que reconstruye. El nombre no prueba
su llamador, momento de ejecución o función comercial. El perfil ensayado usa
sampler synth tipo 6, sampler drum tipo 8, FX ID 0 (Delay) y los siete IDs de
modulación en cada ruta. Todos sus helpers originales son legibles y se ejecutan
sin sustituciones. El instrumento sigue en 0.54.0, sin instaladores nuevos.

## Cadena original ejecutada

1. Lee el tipo de audio de la selección drum y reconstruye sus doce productores;
   enlaza las referencias del banco activo Core suministrado.
2. Construye e inicializa el FX drum para contexto 1.
3. Lee el modulador drum, ejecuta su preparación, crea el secundario y doce
   primarios, y enlaza las tablas Core.
4. Reconstruye los seis productores de audio synth y enlaza las referencias del
   banco activo Core synth suministrado.
5. Construye e inicializa el FX synth para contexto 0.
6. Construye secundario y seis primarios de modulación synth y los enlaza.

El orden se reconstruye del cuerpo legible y se ejecuta entero en el oráculo;
las instantáneas comparadas son posteriores a cada llamada completa. No se
inventa una cadencia de render a partir de este orden de construcción.

| Dato | Fuente del ensayo/resultado |
| --- | --- |
| Selecciones de edición | `0xff8016d4` synth y `0xff8016d8` drum, suministradas. |
| Bancos activos Core | `0x01aaac0c` synth y `0x01aaac10` drum, suministrados. |
| IDs de modulador | Banco de edición en `0xff801384` synth / `0xff8013a4` drum. |
| Referencias de audio | Tablas explícitas `0x01df6f78` synth / `0x01cc10cc` drum, indexadas por banco activo Core. |
| Productores de audio | Seis punteros desde `0x01aaad18`, doce desde `0x01aaad30`. |
| FX y moduladores | Ventana completa de 21 punteros desde `0x01aaad60`, más secundarios en globals. |

Se prueban bancos de edición y Core iguales y distintos: 989 llamadas completas
coincidentes y 2967 distintas. No se presupone que ambos selectores sean aliases.
Las referencias son handles opacos no nulos del harness; no son audio o muestras
recuperadas del aparato. Aquí se enlazan, sin dereferenciarlas para render.

## Solapamientos que importan para la integración

Las dos rutas usan los mismos pools físicos. El resultado tras esta llamada no
representa dos instrumentos reconstruidos en memoria independiente:

- **Audio:** drum construye doce estados de 20 bytes desde `0xffb00000`. Después,
  synth construye seis de 16 bytes sobre los primeros 96 bytes. Las dos tablas
  conservan sus respectivos strides y referencias al mismo espacio; algunos
  objetos que aún señala la tabla drum han sido sobrescritos.
- **FX:** ambos punteros apuntan a `0xffb00600`. En el perfil Delay, el objeto
  final queda con tag `0x0190fb6c`, cursores 9/8/1 muestras y slot/contexto 0,
  después de construirlo/inicializarlo también para contexto drum 1.
- **Modulación:** se reutilizan `0xffb00a00` y el secundario `0xffb00c10`.
  La construcción synth ocurre al final y puede cambiar la familia de bytes
  que aún referencian los punteros drum. Hay 3391 llamadas con familias distintas
  solicitadas en ambos lados; la base de primarios sigue siendo común.

No se deduce un bug del aparato ni se traslada ese solapamiento al host. Falta
el contexto de llamada y cómo se vuelve a construir/seleccionar la parte activa
antes de eventos y render. Estos resultados impiden usar el refresco aislado
como prueba de que ambas rutas están listas para ejecutarse concurrentemente.

## Validación

**4160 escenarios**: 3956 refrescos originales completos y 204 omisiones,
3168 cargas de ventanas y 992 filas retenidas. La rejilla inicial cubre las
3136 combinaciones de dos IDs de modulación y dos bancos activos: 7×7×8×8.
Los casos retenidos prueban reconstrucciones sucesivas y memoria conservada al
omitir la llamada; los writes explícitos del caller se modelan por separado.

Cada fila compara **5628 bytes**: globals Core, las 39 entradas conjuntas de
audio/FX/modulación, pools de audio/FX/modulación y guardas, flags de backend,
selecciones/controles, referencias, lista retenida/count y globals de voz. En las
ventanas observadas, la rutina conserva la lista de notas, ocupación y palabras
de evento; no se presenta como pánico o reset de todo el instrumento. No es un
trazado de todas las escrituras de la máquina.

El modelo compone los estados reconstruidos de Drum, Sampler y Delay y la factory
de las siete familias. Preserva contadores/padding según sus contratos al escribir
sobre el pool compartido. Dos ejecuciones originales independientes coinciden;
Release y ASan/UBSan aprobados. También pasan las dos suites de factories, Core
drum y modulación/render retenidos de batería.
[Formato/reproducción](../research/core-refresh/README.md) y
[evidencia](firmware/core-refresh246-validation.json).

[CI nativo aprobado](https://github.com/jguillenconesa-coder/Tiny-one/actions/runs/42227862984)
en Linux, Windows/MSVC y macOS Intel sobre `729e7dd`, sin build del producto.

### Referencia estática de llamada

Aplicando el auditor existente a `0x01967b28` se encuentra una llamada relativa
candidata en SDRAM, `0x019f04a6`. Su vecindad visible lee CYCLES/CYCLES2 antes
(`0x019f0494/496`) y después (`0x019f04b2/4`), y opera sobre las diferencias.
Es un contexto estático de medición; no se ha ejecutado ese llamador ni se ha
establecido que sea el camino normal al tocar. No se deduce ausencia de otros
llamadores indirectos o protegidos. El inventario publica direcciones, no código.

## Frontera pendiente

Este perfil no valida otras combinaciones de motores/FX, el llamador físico,
la reactivación de la parte elegida, el scheduler protegido o productores activos
de pitch/ganancia. No se han renderizado muestras nuevas ni cambiado el plugin.
El guard de entradas fuera del perfil es propio del modelo; no se ejecutan las
ramas originales de error/log. Las referencias y selecciones son explícitas.

Siguiente: localizar el llamador y la reconstrucción de la ruta activa antes de
notas/render, y componer esa frontera con los eventos ya contrastados. Mantener
el método híbrido donde L1 siga protegida, sin atribuirle comportamiento deducido
de estos pools. Acústica y Ableton real siguen pendientes. Estimación **≈70 % ±10**
de planificación, presets al final y sin instaladores hasta nueva petición.

La [investigación posterior de sincronización](core246-selection-sync.md)
contrasta dos bucles exteriores bajo respuestas L1 explícitas. La igualdad de
selecciones no recupera por sí misma el servicio que reconstruye los pools.
