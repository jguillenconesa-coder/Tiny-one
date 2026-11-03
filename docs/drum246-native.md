# Drum sampler 246 — lector reconstruido y adaptación de kits

En 0.34.0, **SAMPLER 246** permite usar el lector reconstruido en **06 / SAMPLE KIT**.
Carga `Sampler 246 → Init / Sampler 246` para probarlo con muestras propias del
proyecto, o activa la opción sobre tu kit. Desactivarla vuelve al reproductor
anterior. Los presets originales completos siguen reservados para la etapa final.

## Manejo

El selector **EDIT PAD** elige el pad que editan los cuatro mandos de set 1.
También puede seleccionarse desde KIT EDITOR. Set 2 ajusta el contorno del kit:

| Set | Azul | Verde | Blanco | Naranja |
| --- | --- | --- | --- | --- |
| 1, por pad | Pitch (semitonos) | Start | End | Gated / One Shot / Loop |
| 2, global | Attack Level | Mid Level | Release Level | Region |

KIT EDITOR conserva la carga por pad, Reverse y Gain. Cuando está activo el lector
246 muestra los tres modos de reproducción en lugar del antiguo botón GATED.
Gated se retira al soltar la nota; One Shot termina su región; Loop repite hasta
la retirada. Sustain se aplica por canal. Hay doce voces con robo de la más antigua
y fundido propio de 3 ms. El mapa MIDI/pad continúa siendo el de los kits del plugin;
no se declara recuperar el allocator ni el layout completo del aparato.

Start y End tienen ahora paso de 10⁻⁷ en su rango normalizado, también en el lector
anterior. Esto permite ajustes finos sobre el cache máximo de 529200 frames.
Modos, dirección, ganancia, afinación, contorno, opción y pad editado se automatizan
y se conservan en proyectos, slots drum y clipboard. Los mandos contextuales siguen
el pad, página y opción con el editor cerrado. Tremolo y los FX del bus están
disponibles; Value Instrument/Random de batería siguen pendientes.

## Qué se reconstruyó y contrastó

La vtable `0x0190fac4 + 0x14` apunta al callback `0x019789ec`. Las referencias de
`DrumSampler.cpp`, la factoría `0x01978df4` de doce estados de 20 bytes y su ciclo
confirman la identificación. `Drum246.h` reconstruye:

- Posición coarse/fine, inicio/end, reproducción directa/inversa y loop.
- Lectura FIR de 16 taps/64 fases con saturación W32 en cada acumulación.
- Composición no lineal del volumen y del stream de ganancia, y escala final.
- Constructor/binding, reset/release, consulta de actividad y contador retenido.

Los **1025 coeficientes** se observan desde el inicializador legible y su conversor
float→fract16 original. Se reutilizan íntegros, sin sustituciones. El C++ coincide
en **4096 bloques / 524288 muestras**, doce estados, audio, pitch, ganancia derivada,
PCM intacto y guardas. Se alternan buffers de salida alineados a palabra y media
palabra y se combinan estados independientes con secuencias conservadas.

El oráculo suministra cuatro buffers PCM sintéticos, guardas de 16 ceros alrededor,
pitch no negativo y ganancia signed16. Ese padding hace explícitos los accesos del
FIR, incluidos pequeños cursores negativos retenidos al acabar en reverse. No
representa una captura de RAM física ni recupera la cadencia del llamador.
Se usa el [modelo BF524 corregido de D-synth](dsynth246-native.md), no hardware.
[Formato reproducible](../research/drum-render/vectors/README.md) ·
[Evidencia](firmware/drum246-render-validation.json).

## Envolvente dinámica: modelo propio

El manual v2.2, §6.4 (PDF pág. 14), describe tres niveles —ataque, zona media y
salida— y un control de región. El callback de audio consume una ganancia ya
preparada; **su productor original no se ha reconstruido**. No se sustituye por
el ADSR común ni se declara numéricamente comprobado este contorno.

El modelo propio interpola linealmente del nivel inicial al central en la primera
región del clip y del central al final en la última. Region cubre 0–50 % de la
longitud por lado; cero deja el nivel central constante. Reverse invierte el
progreso usado por el contorno; Loop lo repite. La raíz del nivel alimenta el
stream porque el callback lo eleva al cuadrado. El init es neutro y no añade
rampas. Estos son criterios propios inspirados en el manual, no un port validado
de la envolvente dinámica del OP-1. Las pruebas de esa curva solo fijan ese contrato.

## Audio importado y demás dependencias propias

El audio original de la sesión se conserva. Al cargar un pad, una conversión propia
fuera del callback prepara PCM16 mono a 44,1 kHz, hasta los primeros 12 segundos.
Usa interpolación lineal para convertir otras tasas; no se atribuye ese conversor
al aparato ni se ha calibrado acústicamente. El camino de audio solo lee el cache,
sin crear ni liberar vectores. El lector anterior conserva el float audio completo.

Desde 0.40.0 puede usar [una fuente compartida de 12 segundos con 24 regiones](shared-drum246.md),
además del banco independiente. La preparación y división inicial son propias;
el layout/segmentación del firmware y la dinámica original siguen pendientes.
Tampoco se replica automáticamente el layout de fábrica ni se incluyen sus kits.

El adaptador aporta reloj de 88,2 kHz, pitch según la tasa de 44,1 kHz y afinación del
pad/canal, conversión de trims a coarse/fine y alineación propia de ocho frames en
el acceso PCM. Aplica la ganancia del pad mediante raíz, mantiene margen para
Tremolo antes del callback y usa filtro/interpolación de salida propios. Estos
niveles y la composición de Tremolo no están calibrados contra un OP-1 físico.
La retirada espera el límite de bloque propio y conserva un drenaje de 512 muestras
internas para la salida; el tail del host usa un margen conservador para one-shots
lentos, según longitud del cache y trims/modos, y es infinito con Tremolo de
pitch, que puede alterar el progreso del lector. La vista de onda usa el cache
convertido cuando está activa la opción, y el audio completo en el lector anterior.

La coincidencia numérica del callback está condicionada a PCM, padding, pitch,
ganancia y eventos suministrados. **La fidelidad acústica del instrumento completo
no está medida**. Quedan la envolvente dinámica original, el productor de pitch,
ruteo/cadencia, algoritmo original de layout/segmentación,
Pulse/L1, validación en Ableton Live real y ejecución macOS arm64. Presets completos
al final. Los materiales disponibles bastan para reproducir esta entrega.

## Modulación en 0.38.0

[Value/Random drum](drum246-modulation.md) integra ocho destinos por golpe,
reglas coarse/fine propias de este helper, dos bancos Value y estado/mandos/UI.
Los callbacks originales rechazan ADSR; la dinámica propia anterior se conserva.
El mapa del host a pitch/modos/dirección y su reloj/asignación siguen propios.
Se reporta tail infinito con modulación Instrument no nula, porque los límites,
dirección y modo pueden variar después del release. No prueba fidelidad acústica.
