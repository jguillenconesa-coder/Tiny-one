# Drum rev246: note-on completo con reinicios activos y lector

Bloque del 6 de noviembre de 2026. Se ejecuta el note-on original completo
`0x01a84ba0`, con el dispatcher de modulación activo y cuerpos originales de
reset, seguido del reset original del sampler tipo 8. Se comprueba el orden con
instantáneas tomadas en los callbacks. El instrumento conserva **0.54.0**:
este bloque no cambia su DSP ni genera instaladores.

## Cadena contrastada

1. El llamador de ensayo prepara controles, selección, flags, ocupación, lista,
   referencias y estados; ejecuta la factory de audio original `0x01967984`
   con tipo 8. No ejecuta la factory de LFO: los perfiles se enlazan explícitamente.
2. El note-on original valida nota/lista, selecciona voz y consulta actividad
   cuando corresponde (`0x019788b0`). Escribe propietario, pad y palabras de
   velocidad/pitch antes de entrar en `0x0196735e`.
3. El dispatcher aplica sus gates y llama a los resets originales de Value,
   Random o Tremolo, con los ajustes firmados de `this` suministrados.
4. Core despacha el reset del lector, que llega al cuerpo original
   `0x01978926`, usando la selección actual y el mapa de pad ya escrito.
5. Al regresar, note-on inserta el registro retenido y marca la ocupación final.

Se observan las llamadas, no solo el estado final: en los resets de modulador y
en la entrada al reset del lector la ocupación de la voz vale **cero**, las
palabras de evento/mapas ya están escritas, la lista aún tiene tamaño **cero** y
el lector sigue con su estado previo al reset. Al terminar una nota aceptada,
la lista tiene un registro, la ocupación vale uno y el modo del lector vale uno.
La secuencia propia de trazado confirma que el reset del lector llega después
de los callbacks de modulación que hayan superado los gates.

Los casos duplicados y las notas fuera de 53–76 no llegan a ninguno de esos
resets. Sus objetos y lista se comparan también, después de la preparación
explícita y de la limpieza opcional del llamador.

## Selección y referencia de muestra

En 459 casos, el endpoint primario propio cambia deliberadamente el sonido
seleccionado antes de saltar al reset original. El dispatcher vuelve a cargar
la selección para el secundario, como en el [contrato anterior](core246-modulator-reset.md),
y Core vuelve a usar la selección actual para los controles del lector.
La referencia de muestra sigue enlazada al sonido de la factory inicial.
Se comparan ambos datos y la posición resultante: no se asume que el banco de
controles y la referencia enlazada deban coincidir.

Esta mutación es una perturbación del ensayo, **no** un efecto comercial que se
atribuya a Value/Random/Tremolo ni una recuperación de cómo el aparato cambia
bancos. Tampoco identifica qué perfiles coloca la factory LFO en cada ranura.

## Cobertura y observación

**4.096 note-on completos**:

- 3.826 aceptados y 270 ignorados: 118 fuera de rango y 152 duplicados válidos.
- Rejilla aceptada completa de **12 voces × 8 sonidos × 24 pads** (2.304
  combinaciones), con las 256 transposiciones enteras −128…127 suministradas.
- 4.138 resets de modulador: 852 Value synth, 812 Value drum, 828 Random synth,
  832 Random drum y 814 Tremolo. Son perfiles de callback explícitos, no una
  afirmación de selección de tipos por la factory drum.
- 3.826 resets del lector; no son bloques de audio renderizados.
- 216 asignaciones con todos los bytes de ocupación activos y lectores con
  modo cero, para contrastar las consultas que limpian sus contadores. No se
  presentan como robos de voces que estuviesen produciendo audio.
- Fase libre, gates, punteros nulos, this -16/-8/0/8/16, preview, palabras con
  signo/saturación y 459 cambios sintéticos de selección observados.

Cada registro compara **7.624 bytes**: Core/tabla/objetos LFO/display/controles/
RNG (704), globals de voz (812), doce estados de lector (240), sus punteros (48),
lista retenida completa con contador (2.564), ocho bancos de controles de pads
(3.072) y trazas propias (184). Incluye padding, estado ignorado y guardas dentro
de las ventanas. No se declara una traza de todas las escrituras de la máquina.

Los endpoints propios conservan los argumentos R0/R1, registran orden y estado,
y saltan al cuerpo original completo. La vtable de audio de trazado conserva
el ajuste cero y enlaza la consulta y el reset originales. Los observadores
usan un puntero de registro propio; no asumen que los registros callee-saved del
harness conserven su valor dentro del note-on original. Esos endpoints no se
presentan como instrucciones originales.

Las referencias de muestra son entradas opacas suministradas; no se leen como
PCM porque este corpus **no ejecuta render**. No debe interpretarse la factory
de audio como arranque completo, recuperación de LFO, carga de muestras o
calendarización original.

## Modelo y regresión

[DrumVoice246.h](../research/drum-voice/DrumVoice246.h) añade un hook explícito
entre las palabras de evento y el reset del lector. La ruta por defecto mantiene
auxiliares nulos. El nuevo comparador compone el dispatcher ya contrastado y los
modelos de los cinco resets; devuelve al lector la selección actual. La guarda
ante un callback que devolviese una selección inválida es propia y no se ejecuta
en el dominio original comparado. No se atribuye un retorno booleano al callback
original void; el índice de selección del modelo es solo diagnóstico.

Dos extracciones independientes y una reproducción ordinaria coinciden con los
fixtures congelados. Pasan Release y ASan/UBSan del comparador completo. También
pasan las cuatro regresiones que reutilizan la ruta por defecto: VoiceCtl
4.096 registros, bancos 8.192, palabras de evento 65.536 e inicialización Core
256; **78.080 registros previos**, no nuevos oráculos contados otra vez.

[Formato y reproducción](../research/drum-note-modulation/README.md) ·
[Evidencia y hashes](firmware/drum-note-modulation246-validation.json).
CI nativo nuevo y cinco workflows de regresión aprobados en Linux, Windows/MSVC
y macOS Intel. No se activa una nueva compilación del producto.
Firmware, ELF, instrucciones originales y pseudocódigo permanecen privados.

## Límites y siguiente bloque

Se cierra la frontera auxiliar nula para esta composición de note-on en modo
drum 1 / sampler 8. Los perfiles anteriores conservan sus límites y sus
regresiones no se reinterpretan como si hubieran usado LFO activos.

Faltan los callbacks de modulación por bloque compuestos con el render, los
productores de buffers pitch/gain, el reloj/scheduler y su enlace activo con los
tramos L1 protegidos. No se infieren a partir de las palabras del evento ni de
la coincidencia de los resets. La próxima composición usará los callbacks y el
lector contrastados con entradas/cadencia explícitas, manteniendo esa frontera
respecto al aparato. No se enlaza todavía este modelo al plugin.

No hay medición acústica o Ableton real nuevos. Estimación sin cambio:
**≈70 % ±10 de planificación**. Instaladores desactivados hasta nueva petición.
