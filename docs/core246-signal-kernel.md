# Core rev246: kernel de 64 muestras y referencias del render

Bloque del 6 de noviembre de 2026. Se reconstruye el callback completo
`0x019674f8..0x01967614` y se auditan referencias a trece direcciones Core/drum.
El instrumento permanece en **0.54.0**, sin cambios DSP ni instaladores nuevos.
La función no se presenta como productor de ganancia/pitch de batería.

## Contrato numérico del kernel

| Memoria original | Contrato observado |
| --- | --- |
| `0x01bab53c` | Coeficiente int32 Q31 suministrado; el inicializador Core escribe `0x00a3d700`. |
| `0x01aaac14` | Tres int32 de historia: entrada anterior, salida anterior cuantizada y salida anterior completa. |
| `0xff801e50` | Lee 64 int16. No modifica este buffer. |
| `0xff800bb4` | Escribe 64 int16. |

[CoreSignal246.h](../research/core-signal/CoreSignal246.h) reproduce por muestra,
con `sat` como saturación firmada de 32 bits:

```
x = entrada_int16 × 65536
s = sat(sat(salida_anterior − entrada_anterior) + x)
y = sat(s − mulQ31(coeficiente, salida_anterior_cuantizada))
salida_int16 = floor(y / 65536)
historia = {x, salida_int16 × 65536, y}
```

La primera palabra cuantizada de realimentación es un estado separado; no se
recalcula a partir de la salida completa inicial. Las saturaciones intermedias,
el truncado hacia menos infinito y la cuantización forman parte del contrato.
Es una diferencia con realimentación cuantizada. No se atribuye a esos buffers
un papel comercial, una frecuencia de corte, una tasa de muestreo o un enlace
con las voces que no se haya recuperado. La rutina no lee las palabras de evento
de velocidad/pitch, y el llamador completo sigue sin contrastarse.

## Oráculo compuesto

**4.864 callbacks originales completos / 311.296 muestras numéricas**:

- 4.096 estados independientes, con coeficientes e historias extremos y variables.
- 512 llamadas que conservan historia y guardas entre bloques.
- 256 llamadas tras ejecutar el inicializador amplio Core original, reutilizando
  su [contrato de defaults ya contrastado](core246-initial-state.md).

Se suministran silencio, impulsos positivos/negativos, alternancia extrema,
constante, rampa y señales variables. Los coeficientes firmados extremos son
entradas de ensayo, no un rango original de controles recuperado. En 1.095
registros la historia final completa vale INT32_MAX y en 550 vale INT32_MIN;
son registros con último estado saturado, no el total de muestras saturadas.

Cada fila compara los **328 bytes** de coeficiente/vecinos, historia/guardas,
entrada/guardas y salida/guardas. En la composición, el inicializador original
pone la historia a cero y escribe el coeficiente y la palabra vecina baja; el
modelo reutiliza los spans derivados del estudio anterior antes del kernel
aritmético. El orden de llamadas y las entradas son del harness. La configuración
circular y ASTAT son las normales del ensayo; no se cubren todos los estados
posibles de registros del procesador.

Dos extracciones independientes producen los mismos bytes. El firmware y las
herramientas se identifican por hash; los probes de los tres arreglos de
writeback pasan antes de ejecutar el original. La implementación aritmética y
las guardas coinciden en Release y ASan/UBSan. La segunda suite comprueba nueve
casos sintéticos del auditor; no ejecuta firmware. [Formato y reproducción](../research/core-signal/README.md).
[Evidencia numérica](firmware/core-signal246-validation.json).

## Auditoría de referencias: hallazgos positivos y límites

[Auditor reproducible](../scripts/audit-drum246-render-references.py) sobre SDRAM
con bytes conocidos y el bloque L1 ya inventariado de 49.076 bytes, cabecera
`0x206214`, destino `0xffa00000`. En L1 aplica la suposición comunitaria existente:
primeros ocho bytes protegidos de cada grupo de 24. **Nunca decodifica esos bytes**.
No publica instrucciones ni firmware; solo direcciones y clasificación.

| Candidato L1 | Qué se observa |
| --- | --- |
| `0xffa006f8` | Par completo visible que forma `0xff801334`, mapa voz→pad. No demuestra su lectura ni un dispatcher completo. |
| `0xffa00718` | Par completo visible que forma `0xff801304`, palabra de pitch del evento. Tramos protegidos vecinos `0xffa00708..710` y `0xffa00720..728` impiden completar aquí su consumidor. |
| `0xffa0074a` | Solo mitad baja compatible con ocupación `0x1070`; la alta cruza `0xffa00750..758`. La dirección completa queda desconocida. |
| `0xffa01eb8` | Par visible hacia el kernel `0x019674f8`; la instrucción posterior cruza una franja protegida. No confirma una llamada. |
| `0xffa0245a` | Mitad baja `0x74f8`; alta no visible completa. No es una referencia confirmada al kernel. |
| `0xffa0288c` y `0xffa02894` | Par completo hacia `0x019674f8` seguido inmediatamente de llamada indirecta visible al mismo registro. Es una secuencia lineal estática; no valida alcanzabilidad, llamador completo o cadencia. |

En SDRAM, los pares completos a velocidad/pitch aparecen en el inicializador y
el note-on ya estudiados. La dirección del renderer drum figura como palabra en
su vtable (`0x0190fad8`). El inventario no encuentra CALL relativo directo al
renderer ni al kernel dentro del alcance escaneado. Eso **no prueba ausencia de
llamadores**: no resuelve llamadas indirectas o direcciones calculadas. Los
pares con misma mitad baja pero alta diferente se clasifican como otra dirección;
no se repite la falsa equivalencia anterior de `0x01cc1070` y `0xff801070`.

Se cubren pares adyacentes en ambos órdenes, literales alineados a cuatro bytes,
CALL relativos SDRAM y secuencias lineales inmediatas de llamada indirecta tras
pares low/high. Se excluyen pares separados, aritmética de punteros, aliases,
flujo de datos y prueba de ejecución. Nueve casos sintéticos comprueban además
que el decoder no lee franjas protegidas o huecos desconocidos, signo del CALL,
alineación y distinción entre coincidencia parcial y dirección confirmada.
[Inventario y límites](firmware/drum246-render-reference-audit.json).

## Consecuencia para el instrumento

El kernel permanece en investigación hasta recuperar el papel de entrada/salida
y su ruteo. No se añade al bus del plugin por su parecido con un filtro, ni se
usa como envolvente de batería. Los hallazgos L1 acotan dónde seguir; no recuperan
el productor por muestra ni justifican fingir ceros o pedir un volcado nuevo.
Los siguientes ensayos pueden reutilizar la inicialización y el kernel probados,
con llamadas propias explícitas mientras no exista el contrato activo.

El CI previo de 0.54.0 y de inicialización Core ha terminado aprobado en las tres
plataformas. El CI completo de reemplazo omitió los cuatro pasos de instaladores
en todas ellas. Este bloque usa dos suites nativas separadas: no aumenta la
cuenta de 145 del producto ni requiere una nueva compilación del instrumento.
Avance **≈70 % ±10 de planificación**, sin medición acústica, Live real o presets
originales completos nuevos.
