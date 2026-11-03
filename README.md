# Pocket One 0.35.0

**Sampler melódico 246**: lector FIR reconstruido de seis voces, reverse y loop.
Mapa de trims/reloj/ADSR propios; [alcance](docs/sampler246-hybrid.md) · [validación](docs/releases/v0.35.0-validation.json).

**Sampler de batería 246**: lector FIR reconstruido, doce voces y modos Gated/One Shot/Loop.
La dinámica es un modelo propio; [controles y límites](docs/drum246-native.md) · [validación](docs/releases/v0.34.0-validation.json).

**D-box 246**: 24 pads, doce voces, ocho controles por pad y envolventes internas reconstruidas.
[Controles y límites](docs/dbox246-native.md) · [Validación](docs/releases/v0.33.0-validation.json).

**D-synth 246**: ocho controles, dos osciladores, envolventes internas y ocho mezclas reconstruidos.
[Alcance y controles](docs/dsynth246-native.md) · [Validación](docs/releases/v0.32.0-validation.json).

**Voltage 246**: décimo motor, cuatro osciladores, XOR/feedback y filtro sin coeficientes sustituidos.
[Alcance y límites](docs/voltage246-native.md) · [Validación](docs/releases/v0.31.0-validation.json).

**DNA Hybrid**: noveno motor, identidad virtual explícita, seis voces y moduladores.
[Alcance y límites](docs/dna246-hybrid.md) · [Validación](docs/releases/v0.30.0-validation.json).

**String 246**: waveguide reconstruido, cuatro controles, ADSR, seis voces y moduladores.
[Alcance](docs/string246-native.md) · [Validación](docs/releases/v0.29.0-validation.json).

FM **0.28.0**: afinación fraccionaria propia y Value/Random/Tremolo integrados.
[Alcance y límites](docs/fm246-performance.md) · [Validación](docs/releases/v0.28.0-validation.json).

**Sintetizador VST3 y aplicación independiente inspirados en el Teenage Engineering OP-1.** Interfaz gris, pantalla animada, cuatro mandos azul/verde/blanco/naranja y teclado de dos octavas. Implementación independiente; no contiene firmware, muestras de fábrica ni gráficos originales.

[English manual](README.en.md) · [Créditos](CREDITS.md) · [Licencia](LICENSE) · [Notas de versión](docs/releases/v0.35.0.md)

![Interfaz y pantalla animada de Pocket One](docs/screen-animation.gif)

[Navegador de presets y muestras](docs/sound-browser.png) · [Guía de botones](docs/buttons-and-browser.md) · [Captura estática](docs/interface.png) · [Distribución y animaciones inspiradas en OTTO](docs/hardware-interface.md)

**Nuevo en 0.27.0:** [FM Hybrid de cuatro operadores](docs/fm246-hybrid.md), nueve rutas, seis voces y ADSR reconstruida, controles independientes, Mono/Legato/glide y guardado. Render contrastado en 524.288 muestras bajo diez constantes propias explícitas; no se afirma equivalencia acústica.

**Nuevo en 0.26.0:** [CWO Hybrid](docs/cwo246-hybrid.md), con Frequency, Delay, Feedback y Sideband, Value FX, automatización y guardado. Callback mono completo contrastado bajo cuatro coeficientes de fase propios explícitos; los cuatro legibles se conservan.

**Nuevo en 0.25.0:** [Spring 246](docs/spring246-native.md), reverb mono con Tone, Turns, Damping y Send, Value FX, automatización y guardado. Inicializador, coeficientes y nueve líneas de memoria contrastados con las instrucciones originales.

**Nuevo en 0.24.1:** el [volumen maestro](docs/output-routing.md) controla también las colas de los efectos, con mute completo y memorias conservadas. Reset y borrado diferido de [Spring](docs/spring246-lifecycle.md) contrastados como base de la integración de 0.25.

**Nuevo en 0.24.0:** [Phone 246](docs/phone246-native.md) en **Set 3 → FX → Phone 246**: Tone, Phonic, Baud y Telematic, con Value FX, automatización y guardado. Núcleo mono, filtros y buffers segmentados contrastados; workspace acotado con guardas explícitas. Comparte la latencia fija del bus (~1,5 ms).

**Nuevo en 0.17.0:** [Mono, Legato y Glide para Dr. Wave / Cluster](docs/hybrid-performance.md). En **set 2**, selecciona el modo de voz y el portamento de 0–2000 ms. Prioridad de última nota, sustain/bend por canal, automatización y guardado; adaptadores propios, con los núcleos contrastados intactos.

**Nuevo en 0.16.0:** [Value por voz para Dr. Wave y Cluster](docs/value-hybrid246-native.md), sobre sus cuatro controles o ADSR, con rangos/ruteo contrastados. Actívalo en **set 4 → VALUE 246 VOICE**.

**Nuevo en 0.15.0:** [Cluster híbrido](docs/cluster246-hybrid.md), seis voces y hasta ocho osciladores por voz, con sus cuatro controles y ADSR. **08 / CLUSTER** o **Cluster 246 Hybrid → Init / Cluster**. El CI añade un [instalador macOS universal y ZIP portable](docs/macos-packaging.md).

**Nuevo en 0.14.0:** [Dr. Wave híbrido](docs/drwave246-hybrid.md), motor tocable de seis voces con cuatro controles y ADSR. Selecciona **07 / DR. WAVE** o carga **Dr. Wave 246 Hybrid → Init / Dr. Wave**. Núcleo contrastado y adaptadores propios para las dependencias pendientes. Los paquetes de descarga publicados debajo siguen siendo 0.13.0.

**Nuevo en 0.13.0:** [Value 246 Voice](docs/value-voice246-native.md), modulación por voz de los cuatro controles Phase/Digital o de su ADSR. Cinco variantes adicionales, con los mismos mandos MIDI del set 4 y estados antiguos conservados. [Cobertura completa y pendientes](docs/original-op1-coverage.md).

**Investigación FM:** [base interna contrastada](docs/fm246-foundation.md) y [render híbrido integrado](docs/fm246-hybrid.md). Veinte constantes legibles se conservan; diez siguen siendo sustituciones propias. Pitch/reloj/ruteo completos y comparación acústica pendientes.

[Estudio histórico del FM propio de dos operadores](docs/fm-authored-study.md), retirado de la ruta tocable en 0.27.0.

**Trabajo activo: reconstrucción híbrida del instrumento.** [Dr. Wave](docs/drwave246-hybrid.md) y [Cluster](docs/cluster246-hybrid.md) ya son tocables con seis voces, ADSR, Value y modos de interpretación. Se conservan sus núcleos contrastados; reloj, voz/salida y las dependencias L1 sin recuperar usan adaptadores propios documentados. [Plan por componentes](docs/firmware-dsp-workplan.md) y [estado actual](docs/project-status.md). Los presets originales completos siguen aplazados.

**Voces de Dr. Wave:** [factoría y eventos Core contrastados](docs/drwave246-core-routing.md) con otras 61.568 muestras y pools previos al render. La [activación VoiceCtl](docs/drwave246-note-activation.md) añade pitch, velocidad y actividad con afinación explícita y otras 61.568 muestras. El release de motor no sustituye al ADSR común pendiente.

**Contador de Dr. Wave:** [consumo completo de 16 bits contrastado](docs/drwave246-counter.md), con 131.072 ejecuciones y otras 61.568 muestras de render al cruzar sus límites. El proveedor del reloj sigue pendiente en L1 parcialmente cifrada.

**Tablas base de Dr. Wave:** [seis bancos / 320 ondas generados](docs/drwave246-wavebank.md) desde el seno contrastado. Coinciden 1.474.560 muestras de tabla en 18 inicializaciones y se usan en el oráculo del render completo.

**Transformaciones de Dr. Wave:** [tres callbacks reconstruidos](docs/drwave246-wave-transform.md) coinciden con 131.072 palabras y sus cachés. El render completo contrasta su orden real; seis mezcladores espectrales añaden 196.608 palabras verificadas separadamente.

**Avance de Cluster:** [disparo, liberación y seis slots intercalados](docs/cluster246-core-routing.md), contrastados con 131.072 muestras adicionales y un RNG compartido. Se conservan las [cinco normalizaciones observadas](docs/cluster246-normalization.md) y la [inicialización compartida con apoyo de REA](docs/cluster246-shared-init.md). Tres normalizaciones y la integración completa siguen pendientes.

Como dependencia de esa ruta, las [transiciones del ADSR común](docs/cluster246-envelope-transitions.md) añaden 327.680 muestras contrastadas: inicialización y rampa previa al nuevo ataque. No completan su conexión con Cluster ni cambian el plugin instalado.

La [preparación de nota desde VoiceCtl](docs/cluster246-note-activation.md) contrasta una tabla de 128 valores de pitch y la preparación de velocidad y activación, con un coeficiente de afinación explícito. El relevo ADSR sigue pendiente en L1 parcialmente cifrada.

**Banco adicional:** [16 estudios del núcleo Phase / 13 WAV únicos](docs/phase-core-studies.md), compatibles con 0.7.0. Controles extraídos del firmware y renders contrastados con el núcleo original simulado; siguen siendo estudios experimentales sin el resto del preset original.

[Estado del proyecto, avance aproximado y próximos lotes](docs/project-status.md).
El arranque diferido ADSR y su cadena de eventos ya están
[contrastados](docs/adsr246-lazy-pool.md); la integración completa de Dr. Wave sigue pendiente.

El [controlador común de eventos de voz](docs/voice246-instrument-lifecycle.md)
ya contrasta 3.072 eventos entre selectores 0–4, con arranque frío y cambios de
modo; todavía no completa el relevo de pitch/ADSR ni el audio de Dr. Wave.

## Descargar

| Paquete | Descarga |
| --- | --- |
| Instalador Windows 10/11 x64 | [Pocket-One-0.13.0-Windows-x64-Setup.exe](https://github.com/jguillenconesa-coder/Tiny-one/raw/refs/heads/main/downloads/v0.13.0/Pocket-One-0.13.0-Windows-x64-Setup.exe) |
| Windows portable: aplicación y VST3 | [ZIP](downloads/v0.13.0/Pocket-One-0.13.0-Windows-x64-portable.zip) |
| Linux x86_64: aplicación y VST3 | [TAR.GZ](downloads/v0.13.0/Pocket-One-0.13.0-linux-x86_64.tar.gz) |
| Fuentes completas con JUCE | [Código fuente correspondiente](downloads/v0.13.0/Pocket-One-0.13.0-corresponding-source.tar.gz) |
| Kit de compilación del proyecto | [ZIP](downloads/v0.13.0/Pocket-One-0.13.0-Windows-build-kit.zip) |
| WAVs originales del banco de fábrica (CC0) | [ZIP de 28 muestras](downloads/v0.13.0/Pocket-One-0.13.0-Factory-Samples.zip) |
| Integridad de las descargas | [SHA256SUMS.txt](downloads/v0.13.0/SHA256SUMS.txt) |

También se ha configurado la creación de una [release de GitHub](https://github.com/jguillenconesa-coder/Tiny-one/releases) al publicar la etiqueta `v0.13.0`. Los archivos versionados de `downloads/v0.13.0` permiten descargarlos directamente desde el repositorio.

**Versión preliminar.** El instalador no tiene firma digital. Quince suites Linux y diecisiete suites Windows bajo Wine, más instalación/desinstalación y carga del VST3 instalado. La validación nativa en Ableton/ASIO Windows real sigue pendiente. La release no incluye un instalador de macOS; CI genera sus artefactos de compilación.

## Instalar en Windows

1. Cierra el DAW y ejecuta el instalador. Requiere Windows 10/11 de 64 bits y permisos de administrador.
2. Elige VST3, aplicación independiente o ambos. El VST3 se instala en `C:\Program Files\Common Files\VST3\Pocket One.vst3`.
3. Abre tu DAW, vuelve a buscar plugins e inserta Pocket One en una pista de instrumento/MIDI. La aplicación independiente aparece en Inicio. Los bancos importados de 0.2.2 siguen siendo compatibles.
4. Para desinstalar, usa Configuración de Windows → Aplicaciones → Pocket One.

La versión portable se extrae sin instalador; copia el paquete `.vst3` completo a una carpeta que tu DAW explore. Un binario Linux no funciona en un DAW de Windows. [Instrucciones detalladas](docs/windows.md).

## Qué incluye

- **Motores con hasta ocho voces:** FM Hybrid de seis voces/cuatro operadores, Phase, String, sampler WAV/AIFF, batería sintética, kits de 24 pads, Dr. Wave y Cluster híbridos de seis voces.
- **Controles hardware:** modos SYNTH/DRUM/TAPE/MIX; T1 motor, T2 ADSR, T3 FX y T4 LFO; accesos independientes SYNTH/DRUM; cinta con SPLIT/JOIN, LIFT/DROP, loop y tape tricks. MIX añade EQ y efectos de master.
- **MIDI y automatización:** velocidad, sustain CC64, pitch bend en motores compatibles y sonidos iniciales.
- **Secuenciadores:** patrón de 1–16 pasos con dirección normal, inversa, péndulo o aleatoria; arpegiador UP/DOWN/UP-DOWN con 1–3 octavas, gate, swing y máscara euclídea. Sincronización con tempo, posición y saltos del host. [Uso](docs/sequencers.md).
- **Cinta:** cuatro pistas mono, bucles de 4/8/16 segundos, grabación, overdub, mezcla, velocidad/reversa y exportación WAV de cuatro canales.
- **Sesiones:** muestras y cinta incorporadas al estado del DAW y a archivos `.pocketone`.

![Página real de cinta](docs/tape.png)

[Análisis estático del firmware 246 con REA](docs/firmware-analysis.md): inventario verificable de 169 presets synth y 24 drum. El firmware y sus recursos no se incluyen en el plugin.

[Editor de kits de 24 pads](docs/kits.md): recorte, volumen, afinación, reversa y gate por pad.

## Uso rápido

Abre **BROWSE**: filtra por banco/tipo, busca por nombre y distingue presets/variantes de muestras únicas. **LOAD SOUND** o doble clic carga; **AUDITION** permite preescuchar. Elige favoritos y asigna sonidos editados a 1–8 con Shift. Selecciona un motor y toca el teclado o envía MIDI. Las flechas cambian de octava en modos de sonido. Arrastra los mandos; el doble clic recupera el valor por defecto. T1–T4 cambian la función de los encoders. Shift + un botón 1–8 asigna el sonido seleccionado a ese acceso rápido.

**Sampler:** elige SAMPLER → LOAD. Importa los primeros 16 segundos del WAV/AIFF; C4/MIDI 60 es la nota raíz. START y LENGTH recortan la ventana; LOOP repite mientras mantienes la nota. Las entradas estéreo se convierten a mono.

**Secuenciador:** PLAY lo activa; en un DAW también debes iniciar el transporte. Activa Shift para mostrar los pasos y pulsa para activarlos/desactivarlos. Toca una nota y haz **Shift-clic** en un paso para escribirla. STOP detiene secuencia/cinta y libera el sonido.

**Cinta:** elige pista y duración en TAPE, arma REC, pulsa PLAY y toca. OVER suma material; desactivado, reemplaza lo que pase bajo el cabezal. La grabación funciona a velocidad normal; la reproducción permite reversa y congelación. CLEAR borra la pista seleccionada sin undo. EXPORT escribe un WAV de cuatro canales con las pistas sin aplicar mezcla/velocidad/master.

**SAVE / OPEN** guardan y recuperan sesiones completas. Guarda antes de cerrar la aplicación independiente. Haz las operaciones de sesión con el transporte detenido: copiar y codificar audio puede interrumpir brevemente el procesamiento.

[Manual completo en inglés](README.en.md) · [Cambios de la versión](docs/change-log.md)

## Compilar y probar

C++17, CMake 3.22+, Git y JUCE 8.0.6, fijado a `51a8a6d7aeae7326956d747737ccf1575e61e209`. CMake descarga JUCE; puedes usar `-DJUCE_PATH=/ruta/a/JUCE`. Windows requiere Visual Studio 2022 con herramientas C++ y NSIS 3.11+ para generar el instalador; JUCE 8 no admite MinGW.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
```

En Windows configura con `cmake -S . -B build -A x64`. `packaging\windows\build.cmd` compila, prueba y genera el instalador. El archivo de fuentes completas permite compilar seleccionando el JUCE incluido sin descargarlo de nuevo.

- [Dependencias y comandos por plataforma](README.en.md#build-from-source)
- [Compilación cruzada y validación Windows](docs/windows-cross-build.md)
- [Entorno en la nube](docs/cloud.md)
- [Pruebas realizadas y límites de la validación](docs/validation.md)
- [Cómo contribuir](CONTRIBUTING.md)

## Alcance, créditos y licencia

Referencias conservadas para una etapa posterior: [índice de la demo de presets](docs/preset-video-index.md) y [piloto espectral Sugar/Pure](docs/preset-audio-pilot.md). No son el frente de trabajo activo.

Pocket One se aproxima a la distribución de controles y al flujo creativo del OP-1; **no es una emulación completa ni se ha demostrado equivalencia sonora**. Faltan cinta larga de seis minutos, todos los secuenciadores originales, radio, data takes y más motores y mejoras de antialiasing. No importa proyectos/presets originales. El fundamental del motor de cuerda no responde todavía a pitch bend.

[Créditos completos](CREDITS.md): Teenage Engineering como inspiración; OTTO/TOP-1 como referencias; JUCE, Steinberg y los proyectos de herramientas/bibliotecas utilizados. No hay afiliación ni recursos originales copiados.

Código original: **AGPL-3.0-only**, con [licencia completa](LICENSE). La distribución abierta usa la opción AGPLv3 de JUCE y GPLv3 del SDK VST3. Los componentes externos conservan sus licencias y avisos: [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

## Presets y bancos de muestras

25 presets originales, tres kits integrados de 24 pads y cuatro instrumentos muestreados. [Guía del banco y referencias de presets OP-1](docs/sound-bank.md).

**BROWSE → IMPORT BANK** importa un ZIP y lo identifica por su banco. **ADD SAMPLES** añade archivos a la biblioteca. La colección de 79 presets y dos kits aportada por el usuario tiene [descarga directa](downloads/user-banks/README.md). El navegador ofrece variantes y vista de muestras únicas sin duplicarlas. Favoritos y slots se guardan con el proyecto. [Uso, formato y generador de bancos](docs/user-banks.md).

## Audio y latencia

El plugin declara al host la latencia fija del bus de salida: `ceil(64 × frecuencia / 44100) + 1` muestras (aproximadamente 1,5 ms), también en bypass. Las pruebas incluyen ocho voces, cinta/FX y buffers de 64/128/256 a 48/96 kHz, además de comprobación de heap en el callback. Consulta [medidas, configuración y límites](docs/realtime-audio.md): la prueba sintética no certifica ausencia de dropouts en Ableton. Usa ASIO en Live y comienza con 128 muestras a 48 kHz. Carga/guarda audio con el transporte parado.

[Correspondencia completa de botones, combinaciones Shift y límites](docs/buttons-and-browser.md).

[Controles MIDI y automatización con nombres claros](docs/midi-controls.md) · [Avance del estudio DSP original](docs/dsp-research.md).

## Referencias originales en el repositorio privado

La 0.6.0 añadió CLEAN SAMPLE para conservar el espectro de los samples y un [banco privado de referencia](downloads/private-banks/README.md) con nueve sampler y 14 kits de 24 pads del firmware suministrado. Se importan desde BROWSE → IMPORT BANK. Son fuentes secas con regiones/frecuencias raíz recuperadas; no son una conversión de los motores FM/Phase ni de sus efectos. Las descargas requieren acceso a Tiny-one.
