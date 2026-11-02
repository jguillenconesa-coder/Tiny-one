# DNA Hybrid 246 — identidad y memoria explícitas

**09 / DNA HYBRID**, o Sound Library → DNA 246 Hybrid → Init / DNA. Set 1:
Feedback/Colour, Clock, Scan/Detune y Noise; Set 2: ADSR, seis voces, Mono/Legato y
glide; Set 3/LFO: Value Instrument/ADSR, Random y Tremolo. Controles automatizables,
proyectos, slots y LIFT/DROP integrados. El init es propio, no uno de los dieciséis
presets originales inventariados.

## Qué se reconstruye

El callback mono `0x019b48d8` usa dos cursores sobre una onda compartida,
interpolación de siete coeficientes, mezclas/XOR, saturación no lineal, ruido LCG
y filtrado. También usa bytes de la memoria de trabajo del DSP como señal.

La reconstrucción coincide en **4096 bloques / 524.288 muestras**, con los seis
estados de 32 bytes, feedback compartido, RNG y **16 capturas completas** de la
onda de 24.576 bytes y una imagen de RAM de 196.608 bytes. El lector de bytes
observa el pool vivo: los cambios del contador/filtro durante el callback pueden
modificar datos leídos en ese mismo callback. No se congela el pool por bloque.

La coincidencia está **condicionada a la identidad y a la RAM suministradas**.
No implica recuperar la identidad física del OP-1 del vídeo ni su RAM. Las tablas
Q15 legibles de filtro e interpolación se reutilizan sin sustituir coeficientes,
pero esas tablas por sí solas no determinan el timbre de DNA.

## Identidad original y perfil virtual

El inicializador `0x019b4cc4` llama a `0x019f16a2`, que lee dos páginas a través
de la ROM `0xef00001a` y forma dos cadenas con `%016llx`. La transformación que
sigue consume 32 bytes: acumula `(carácter − '0') << (4 × posición)`, con wrap de
32 bits, y escribe cuatro grupos de 64 muestras reflejando desplazamientos de
los cuatro resultados. **No se sustituye por un parser hexadecimal convencional.**

Se ejecuta el tramo original **`0x019b4cd4..0x019b4d40`** con un frame explícito
que contiene esos bytes. Se comparan 40 inicializaciones con cuatro identidades,
además del contenido completo de la onda. El lector OTP/ROM y el arranque físico
no se ejecutan ni se presentan como reconstruidos.

El plugin usa la identidad virtual fija:

```
706f636b65746f6e652d646e612d7630
```

Es una identidad propia del instrumento software, no un número de serie
recuperado. Es constante entre sesiones/instancias de esta versión. Las demás
entradas de la onda se inicializan a cero mediante el ciclo de construcción
contrastado; no hay audio muestreado de los vídeos.

## La RAM es una dependencia sonora

El callback lee `0xffb00000 + (contador >> desplazamiento)`. Los primeros
**192 bytes** corresponden al pool de seis estados de la factoría `0x019b4c64`.
Los ensayos proporcionan explícitamente los demás bytes, con perfiles cero y LCG,
y comparan también sus capturas completas para detectar escrituras imprevistas.
La región de simulación ampliada es un modelo de ensayo, no el mapa RAM físico
validado del aparato.

Una prueba original de dos callbacks mantiene idénticos identidad, pool,
controles, pitch y RNG y cambia **solo la RAM ajena al pool**. La suma de cuadrados
de las 128 muestras pasa de **0** a **25.146.443.216**. Es evidencia causal de
esa dependencia, no una medida de calidad o semejanza acústica. La reconstrucción
C++ reproduce también ambos callbacks y sus estados.

El perfil del plugin preserva el **pool vivo** y devuelve **cero para todos los
bytes externos al pool**. Esa parte es una aproximación explícita. La memoria de
otros subsistemas y su historial no se han recuperado. No se deben atribuir al
núcleo, a los presets o a coeficientes una diferencia que provenga de este perfil.

## Controles y ciclo

Los descriptores de `0x01de5b50`, tipo original **11**, y los callbacks específicos
de Value y Random se contrastan con el firmware:

| Control | Mínimo | Máximo | Escala de modulación |
| --- | ---: | ---: | ---: |
| Azul: feedback/color | −29491 | 32767 | 32767 |
| Verde: reloj | 4608 | 12800 | 12800 |
| Blanco: scan/detune | 0 | 32767 | 32767 |
| Naranja: ruido | 0 | 32767 | 32767 |

La factoría conserva los campos que no inicializa; reset `0x019b4888` y release
`0x019b4844` tienen efectos distintos, incluyendo el feedback publicado al liberar.
Se comparan esos efectos antes del render y los seis estados completos después.
El wrapper crea una voz nueva con política propia de limpieza antes de componer
esas operaciones; no se atribuye esa política al allocator original.

Value y Random añaden **4096 + 4096 callbacks** con estados/display/RNG, ocho
destinos y retornos completos. Random comparte RNG con DNA, por lo que activarlo
puede cambiar el ruido aunque la profundidad sea cero. La prueba de modulación
compara cero/no cero con Random activo para aislar sus controles. Tremolo conserva
su aritmética contrastada y alinea pitch/amplitud por bloques de 128 muestras.

## Adaptador y validación

Son propios el reloj de **88.2 kHz**, scheduler, asignación de seis voces,
Mono/Legato/glide, rampa de relevo de 3 ms, ADSR compuesta, DC de 15 Hz, filtro
causal y conversión de tasa. El original toma una palabra de pitch por bloque;
el host suministra esa palabra y envuelve sus dos cursores al cambiar el periodo.
Se limita el divisor positivo a `0x50000..0x7fffffff`, con guarda de contador.
Esas decisiones no se presentan como afinación/calibración física del OP-1.

El oráculo usa pitch positivo desde `0x40000`, fases válidas, contadores positivos
y lecturas dentro de su imagen explícita de RAM. Mantiene los clamps y la palabra
baja de la división entera original; no aplica la extensión fraccionaria de FM.
Los coeficientes Q15 proceden de las mismas conversiones originales ya verificadas
para otros componentes. La repetición privada independiente coincide byte a byte.

Las pruebas cubren siete tasas (8–192 kHz), 32 bloques del adaptador frente al
callback directo, pool/RNG/onda, cuatro colores Value hacia Instrument/ADSR,
Random y cinco formas Tremolo, seis voces, notas/controles extremos, sustain,
release/panic y modos de interpretación. El plugin comprueba nueve selecciones de
motor, proxies con editor cerrado, guardado/slots/clipboard y buffers 64/257.
Las cargas de callback se amplían a **444**; volumen maestro y PDC del bus FX se
mantienen. [Evidencia de entrega](releases/v0.30.0-validation.json).

Quedan la identidad/RAM de una referencia física, comparación acústica, scheduler
original y Ableton real. Solo disponemos de los vídeos/presets ya aportados; no se
requiere reenviarlos ni se presupone acceso al hardware. Voltage y D-synth siguen
pendientes según sus dependencias. Los presets originales completos quedan al final.
