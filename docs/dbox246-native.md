# D-box 246 — núcleo y pads

En 0.33.0, **12 / D-BOX** añade el callback mono reconstruido a un banco tocable
de **24 pads y doce voces**. Selecciona `D-box 246 → Init / D-box` en la biblioteca.
Es un init propio; los diez kits originales se dejan para la etapa final.
El antiguo `05 / DRUM` conserva su síntesis propia y no se presenta como D-box.

## Controles

El selector **EDIT PAD 1–24** elige el pad que editan los cuatro mandos. No cambia
el sonido de los demás pads. MIDI 60–83 dispara pads 1–24; fuera de esa ventana se
repite el mapa cada 24 notas. El selector de edición es automatizable y se guarda.
No sigue automáticamente a cada nota de una secuencia, para poder editar un pad
mientras suenan otros. Set 1 y set 2 exponen los ocho valores raw, 0–32767:

| Set | Azul | Verde | Blanco | Naranja |
| --- | --- | --- | --- | --- |
| 1 | Pitch I | Wave I | Envelope I | Cross Mod |
| 2 | Pitch II | Wave II | Envelope II | Filter |

Son etiquetas funcionales propias. Set 2 controla el segundo oscilador/filtro;
las dos envolventes están dentro del núcleo y no se sustituyen por el ADSR común.
Los 192 parámetros tienen identidades de host independientes. Los cuatro mandos
contextuales siguen el pad y el set incluso con el editor cerrado. Proyectos,
slots de batería y clipboard conservan todos los pads, el motor y el pad editado.

Tremolo utiliza el componente mono ya contrastado, con pitch y ganancia antes del
núcleo. La ganancia nominal propia de 16384 deja margen para su modulación
aditiva, evitando que una entrada a escala completa la recorte. Su reloj y
composición aquí son adaptaciones. Value Instrument y Random
**de batería no están implementados**: el dispatch de sintetizador no prueba esas
rutas. Value FX y el bus común conservan su alcance documentado. Los ocho FX del
perfil siguen disponibles en el bus.

## Evidencia original

La vtable `0x0190f8a4 + 0x14` apunta a `0x01970ee8`. La identificación también
concuerda con las referencias de `dbox.cpp` y las factorías de doce posiciones.
El callback contiene dos osciladores con realimentación, dos envolventes internas,
mezclas saturadas/bitwise, cross-mod y filtros en serie. Su primer control no se
lee en este callback: el pitch llega desde el llamador. No se atribuye a ese
callback la conversión del control azul a frecuencia.

El C++ independiente coincide en **4096 bloques / 524288 muestras**, los doce
estados de 72 bytes, el contexto compartido de 1640 bytes y todos los buffers.
Se contrastan las ocho selecciones de mezcla, salida alineada a palabra y a media
palabra, estados independientes y secuencias conservadas, constructor/binding,
reset/release, inicializador, consulta de actividad, indicadores e históricos.
Las 128 entradas de ataque, 128 de caída y 129 de filtro coinciden con tablas
legibles ya inventariadas; no se sustituyen coeficientes. Pitch completo por
muestra y ganancia con signo son suministrados explícitamente.

El oráculo requiere el modelo GNU BF524 con los tres parches de writeback
[documentados con D-synth](dsynth246-native.md). No es ejecución medida en hardware.
Ghidra se usa como ayuda privada; sus ramas omitidas no sustituyen el disassembly
ni la comparación completa. No se publican instrucciones ni pseudocódigo original.
[Formato y método](../research/dbox-render/vectors/README.md),
[evidencia](firmware/dbox246-render-validation.json).

## Adaptación al host y límites

`Dbox246Voice.h` aporta reloj de 88,2 kHz, suavizado, interpolación/filtro de salida,
mapa de pads, afinación, disparo/retirada, sustain por canal, velocidad y robo de la
voz más antigua con fundido propio de 3 ms. Doce voces no demuestra que el
allocator completo del aparato esté recuperado. La retirada de notas invoca el
release reconstruido; no se afirma recuperar todos los modos de disparo originales.

La afinación propia usa `40 × 2^(azul/4096)` Hz para el primer oscilador, con bend
del canal. El stream de pitch multiplica esa frecuencia por 16 porque el callback
divide ese stream por 16 en el primer oscilador. El segundo conserva su razón
cuadrática reconstruida. El stream se limita a 0,49 del reloj para evitar desborde;
esto comprime el extremo alto del control. La tabla/cadencia de afinación original
queda pendiente; no se calibra a oído ni se declara equivalencia de afinado.

La actividad original deja de estar presente cuando la suma saturada de niveles
es inferior a 16. El adaptador espera además un bloque completo con salida entre
−4 y +4 LSB: la ganancia final del núcleo amplifica a cuatro LSB su residuo de
cuantización. Esta detección y el margen conservador de 0,25 s del filtro de
salida son propios. El tail reportado considera la caída más lenta de ambos
osciladores en los 24 pads, los efectos comunes y la latencia del bus.

La coincidencia numérica es condicionada al pitch, ganancia, eventos, memoria y
modelo de CPU suministrados. **La fidelidad acústica del instrumento completo no
está medida**. Siguen pendientes el llamador/ruteo original, modulación específica
de batería, sampler/envolvente dinámica original, Ableton Live real, ejecución
macOS arm64 y los presets completos. No se necesita otra captura privada para
reproducir este bloque con los materiales ya disponibles.
