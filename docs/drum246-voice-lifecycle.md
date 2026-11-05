# Drum VoiceCtl: asignación, propiedad y ciclo de notas rev246

Investigación posterior al [ruteo Core](drum246-core-routing.md), 5 de noviembre
de 2026. **El instrumento permanece en 0.52.0**: el modelo no se enlaza al plugin,
no modifica su asignador y no genera instaladores. Cierra un consumidor activo
de la consulta de actividad, sin recuperar por ello el scheduler de render ni
la envolvente dinámica original.

## Contrato contrastado

| Ruta / dato | Dirección y comportamiento |
| --- | --- |
| Note-on completo | `0x01a84ba0`, evento de cuatro palabras y transposición suministrada |
| Note-off completo | `0x01a84dac`, busca y elimina la nota retenida |
| Consulta Core | `0x01967308`, vtable `+0x24`, ajuste this firmado `+0x20` |
| Actividad del sampler | `0x019788b0`, devuelve contador; si modo=0 pone contador a cero |
| Modo drum requerido | `0xff801038 = 1` |
| Bytes de ocupación | `0xff801070[12]` |
| Propietario / pad del lector | `0xff8010f4[12]` / `0xff801334[12]` |
| Velocidad por voz | `0xff8012ec[12]`, halfword inferior del evento desplazado ocho bits con saturación firmada |
| Pitch por voz | `0xff801304[12]`, cadena original int→float / 12 → pow(2,x) / 32 → Q31 |
| Preview / último pad y voz | `0x018c2180` / `0xff801044`, `0xff801048` |
| Lista retenida / tamaño | `0x01dfe734`, registros de 20 bytes / `0x01dff134` |

El note-on ignora notas fuera de 53–76 y notas ya presentes en la lista retenida.
Recorre las doce voces hasta el primer byte libre. Si no hay ninguna, consulta
sus objetos de audio y elige el **mayor contador firmado**, con empate favorable
a la primera voz; la consulta puede borrar contadores de lectores con modo cero.
Un caso donde todos los resultados son negativos no proporciona candidato y
lleva a assertion: queda fuera del dominio de prueba.

Después actualiza propiedad, pad, velocidad y pitch, despacha el reset de Core,
inserta el evento con su voz y marca la ocupación. Los objetos de modulación y
FX son explícitamente nulos en este oráculo; sus resets no se infieren aquí.
El campo preview -1 actualiza último pad y voz; un preview fijado actualiza la
voz únicamente cuando coincide con el pad entrante.

Note-off solo libera si la nota retenida sigue siendo propietaria de esa voz y
su byte de ocupación es no nulo. Después compacta la lista conservando el resto
del almacenamiento. **No limpia por sí solo el byte de ocupación** ni el mapa
de propietario. Un release antiguo tras un robo no libera la nota nueva. La
retirada posterior que cambia ocupación pertenece a otro contrato pendiente.
No se compara un retorno de estos callbacks void; el retorno de selección del
modelo es solo un diagnóstico propio.

## Evidencia y alcance

4.096 casos, 64 secuencias de 64 eventos, con las doce voces y ocho bancos:

- 2.176 notas iniciadas (640 redisparos tras retirada), incluidas 2.044 asignaciones sobre voces ocupadas.
- 576 note-ons ignorados y 1.152 notas retiradas de la lista retenida.
- 854 retiradas con propietario antiguo; 878 retiradas no cambian ningún modo
  (incluyen aquellas donde el modo ya tenía el valor de release).
- 2.730 consultas explícitas de actividad, adicionales a las del asignador.
- 3.072 renders originales / 393.216 muestras; 2.402 bloques con audio no nulo.

Se comparan cada byte de los doce estados, 812 bytes de globals y 2.564 bytes de
lista/count, además de consulta, audio, pitch, gain y ganancias derivadas. La
lista parte de padding `0x5a`, para comprobar escrituras parciales y compactación.
Incluye contadores con bit de signo, empates, cambios de propietario, duplicates,
notas fuera de rango, velocidades negativas y saturadas, preview, releases de
Gated/One Shot/Loop bajo controles explícitos y renders omitidos con veneno.
Se comprueban también conversión de los 1.025 coeficientes y PCM intacto.
Dos extracciones independientes dan las mismas entradas y salidas numéricas.
Release y ASan/UBSan completos aprobados en Linux (leaks desactivados por ptrace);
CI nativo (1/1 suite) y regresiones del lector aprobados en Linux, Windows/MSVC
y macOS Intel.

[Modelo, formato y reproducción](../research/drum-voice/README.md).
[Evidencia y hashes](firmware/drum-voice246-validation.json). No se incluyen
firmware, ELF, desensamblado ni pseudocódigo en el repositorio.

La cadena original de pitch solo queda contrastada aquí con -24, -12, 0, 12 y
24 semitonos; el modelo usa sus potencias de dos exactas. No es una reconstrucción
general de pow/soft-float ni se extiende a otras transposiciones. Se mantienen
ASTAT y registros de direccionamiento circular en el dominio normal del harness.
Los renders reciben PCM/padding, pitch/gain y cadencia suministrados: **las
palabras de evento no prueban todavía el productor de esos buffers**.

## Continuación

El [estudio posterior de bancos y registro](drum246-bank-lifecycle.md) ya
contrasta la limpieza de ocupación/lista, el reset retenido y las fases de
factory. Clasifica las referencias candidatas previas y descarta la falsa
coincidencia `0x01976edc` / `0x01cc1070`. La limpieza conserva los lectores,
así que no recupera la retirada normal por bloque. Siguen pendientes el llamador
de render y el consumidor de velocidad/pitch, incluidos accesos indirectos.
Agrupar la integración con esa composición evita confundir un registro vacío
con audio detenido. Dinámica, scheduler y frontend híbridos permanecen propios;
D-box, Live real, acústica y presets completos no quedan validados por estos
estudios. Avance **≈69 % ±10 de planificación**.
