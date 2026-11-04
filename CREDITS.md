# Créditos

## Proyecto y desarrollo

- **Mantenimiento y dirección del proyecto:** [jguillenconesa-coder](https://github.com/jguillenconesa-coder).
- **Implementación inicial y preparación de compilaciones/documentación:** desarrolladas con asistencia de **OpenAI Codex**, según las indicaciones del mantenedor.
- **Pocket One** es una implementación independiente. El motor de audio y los gráficos propios de este repositorio se escribieron para este proyecto.

## Inspiración y referencias

- **Teenage Engineering — OP-1:** inspiración del instrumento, la disposición de controles y el flujo creativo. Las marcas OP-1 y Teenage Engineering pertenecen a sus respectivos titulares. No hay afiliación, patrocinio ni aprobación. El instrumento no incluye firmware, muestras de fábrica ni logotipos originales. Las reconstrucciones de DSP contrastadas y sus adaptadores se documentan por separado; varias se integran en el audio de producción.
- **OTTO y sus colaboradores:** organización por motores y pantallas, controles vinculados a colores y documentación de un instrumento autónomo. Referencia revisada: [antoinehng/OTTO-1](https://github.com/antoinehng/OTTO-1), revisión `ba534c6c1ed93a6b15cd13a93287b9e8f6f8ca6b`.
- **TOP-1 y sus colaboradores, incluido D-I-S-K-U-S por sus notas de diseño visual:** referencias de sampler, grabadora de cuatro pistas y relación entre gráficos y parámetros. Referencia revisada: [AlbertSmit/TOP-1](https://github.com/AlbertSmit/TOP-1), revisión `e641d273fb8d0eb4ff2ad1b4a7cd15b8786b62e0`.

OTTO/TOP-1 son referencias de diseño, no dependencias incorporadas. Sus fuentes y recursos no se copiaron en Pocket One. Sus instantáneas revisadas usan MPL-2.0. Véase [el análisis de referencias](docs/references.md).

## Bibliotecas y herramientas

| Componente | Autor/proveedor | Uso |
| --- | --- | --- |
| JUCE 8.0.6 | Raw Material Software Limited / JUCE y colaboradores | Audio/MIDI, interfaz, VST3 y aplicación independiente |
| VST3 SDK incluido en JUCE | Steinberg Media Technologies GmbH y colaboradores | Integración del formato VST3 |
| NSIS 3.11 | Colaboradores de NSIS | Instalador y desinstalador de Windows |
| LLVM/clang-cl/lld 19 | LLVM Project y colaboradores | Compilación cruzada de Windows |
| Microsoft CRT y Windows SDK | Microsoft | Runtime y API de Windows |
| xwin 0.10.0 | Jake-Shadle y colaboradores | Obtención y verificación del SDK/CRT para la compilación cruzada |
| Wine 10 y Xvfb/X.Org | Wine Project y X.Org y sus colaboradores | Pruebas de los ejecutables de Windows y renderizado en Linux |
| CMake, Ninja y GCC | Sus respectivos proyectos y colaboradores | Configuración, compilación y pruebas |

Las herramientas de compilación/prueba no se distribuyen dentro del instalador. El código fuente de JUCE conserva sus propios créditos y avisos de dependencias. Véanse [avisos de terceros](THIRD_PARTY_NOTICES.md) y [licencia del proyecto](LICENSE).

Factory sound bank: 25 original parameter recipes, 24 original drum one-shots and four pitched samples generated for this project. The WAV assets are CC0-1.0; the code retains AGPL-3.0-only. Format references: [te-op1](https://github.com/jstnfst/te-op1) (Field) and [OP Patch Utility](https://github.com/AlexCharlton/op-patch-util); no source or sound assets were copied from these references.

OLED animation references: OTTO-1 Nuke, TapeScreen and DrumSampleScreen, revision ba534c6c1ed93a6b15cd13a93287b9e8f6f8ca6b (MPL-2.0 upstream). Pocket One implements original JUCE drawing code and geometry; no OTTO source/assets were copied. Hardware layout also references the user-supplied OP-1 user guide v2.2 and photographs, which are not redistributed.

## Firmware research tooling (0.4.0)

The user-supplied OP-1 revision 246 was inspected with [REA 6.8.0](https://github.com/morluto/rea) (MIT), [Unblob 26.6.4](https://github.com/onekey-sec/unblob), official 7-Zip tooling and Python's standard library. [op1repacker](https://github.com/op1hacks/op1repacker) documents the CRC/LZMA/TAR container format. These are analysis tools/references, not linked product dependencies. No original firmware, databases, sounds, graphics or decompiled DSP are included. See [the evidence and limits](docs/firmware-analysis.md).


GNU Binutils 2.44 (bfin-elf) and the GNU GDB 13.1 Blackfin simulator are external research tools, credited to the GNU projects and contributors. They are not included in the plugin or installer. Own probe harnesses use caller-supplied GDB test helpers only in private generated research programs; [scope and reproducibility](research/blackfin/README.md).


## Phase 246 integration research (0.8.0)

[BertBrandstifter/op1-sdk](https://github.com/BertBrandstifter/op1-sdk/tree/ae1945a9c61468f0e0d7cfdd5dd60c2cf9731ff1)
(MIT) supplied community documentation of the rev246 voice clock, range-table
addresses and control/pitch contracts. We independently tested isolated original
range initialization and frequency conversion. [U-Boot's Blackfin bootrom.h](https://github.com/u-boot/u-boot/blob/v2013.10/arch/blackfin/include/asm/mach-common/bits/bootrom.h)
supplied flag definitions; [sualk/op1-decryptor](https://github.com/sualk/op1-decryptor/tree/20f86baa116b4c7b1da7e0d4e3a24f171c76767b)
was consulted for the encryption layout only (no declared license; no code copied).
None is linked or bundled. Pocket One's reconstruction, adapter, GUI and tests
are independently written. Original firmware/decryption keys are not distributed.


## Envelope research and simulator correction (0.9.0)

GNU GDB 13.1's Blackfin simulator is Copyright (C) 2005–2023 Free Software
Foundation, Inc.; contributed by Analog Devices, Inc.; GPL-3.0-or-later.
`research/blackfin/gnu-13.1-dreg-writeback.patch` is a local modification under
those terms, retained separately with the [GPL text](licenses/GPL-3.0.txt).
No GNU simulator binary/code is linked into the instrument. The product's C++
envelope is an independent reconstruction, with numerical coefficient observations
and test traces from the user-supplied firmware. No firmware images/instruction
snippets are packaged. The local patch is not claimed to be upstream-approved or
hardware validated. See [evidence](docs/envelope246-research.md).


## Digital research (0.10.0)

The Digital voice identification was informed by the same pinned MIT op1-sdk
reference credited above and independently tested through isolated original
render/constructor/reset calls. The C++ oscillator, causal adapter, GUI, harness
and tests are independently written. Stored numeric fields and numeric oracle
traces are documented research observations, not a distributed firmware image.
GNU model limitations and original-preset omissions remain explicit in
[the Digital method](docs/digital246-research.md).


## Punch, manual and preset inventory (0.11.0)

The user-supplied Teenage Engineering OP-1 v2.2 manual defines the documented
control names, octave behavior and Helicopter/Singing Birds exercise targets.
REA's read-only SQLite inspector independently inventories the rev246 factory DB;
evidence and the source digest are recorded. No manual images, original database,
audio or firmware instructions are redistributed in this work. Punch kernel,
adapter, visual drawing, harness and tests are independently written; numeric
coefficients/traces are documented observations. The GNU tool patch retains its
separate terms. Existing reference attributions above continue to apply.

## Value FX reconstruction (0.12.0)

Value246's independently written FX kernel, clock adapter, display drawing and
harness are AGPL-3.0-only project work. The user-supplied rev246 image supplies
numeric observations; GNU BF524 executes the isolated FX path with the documented
local model patch. REA's read-only SQLite inspector confirms seven LFO type/default
records. The OP-1 manual §16.3 informs the additional Helicopter study, whose raw
positions and output remain uncalibrated against hardware. Original firmware,
opcodes, DB, manual illustrations and recordings are not distributed. Public
op1-sdk references and all previous tool attributions continue to apply.

## Value instrument / ADSR routes (0.13.0)

The user-supplied rev246 image provides numeric observations for independent
Digital/Phase Value routing and common ADSR modulation. GNU BF524 with the
documented local model patch executes the isolated originals; no original
instructions or simulator are distributed. The per-voice clock/allocator, UI,
harness and tests are independently written project code. Four additional
variants use inventoried factory controls; missing original FX and mix/play-mode
behavior remain explicit. The supplied preset-demo MP4 could not be transferred
because of the tool's 32 MiB limit; no audio/screens are copied or claimed as
verified for the active reference. Superseded segmented batches are excluded at
the user's request. Existing dependency/reference notices remain valid.

## Preset listening reference index (2026-10-27)

The user supplied 21 definitive excerpts of the demo linked at
<https://www.youtube.com/watch?v=P84SBtIDdSM>, with the supplied filename
“Every Single OP-1 Preset (2021)”. The [screen index](docs/preset-video-index.md)
records 169 reviewed name/category observations and their times, correlated to
rev246 inventory names. It does not claim identical parameters, acoustic matching
or drum-kit coverage. No video, decoded audio or screenshots are distributed.

The local analysis tool uses separately installed FFmpeg/ffprobe, Tesseract,
Python, NumPy and Pillow. These research tools are not added to the plugin or
bundled in its installers. See the index for reproduction and review limits.

## Conditional spectral pilot and study renderer

The Sugar/Pure [pilot](docs/preset-audio-pilot.md) publishes computed magnitudes
and an offline renderer for our current Engine/core integration, not original
audio. It uses the definitive user-supplied reference above and the rev246 numeric
preset observations already credited. It does not establish complete patch or
recording-chain equivalence. The analysis additionally uses separately installed
SciPy and Matplotlib; these are research dependencies, not plugin runtime or
installer additions.

## Cluster rev246 isolated kernel

The independently written [Cluster reconstruction](docs/cluster246-kernel.md)
uses the operator-supplied rev246 firmware only as a private instruction oracle.
The committed fixtures contain numerical test results and scalar observations,
not the firmware, original instructions or L1 payload. The same documented GNU
BF524 simulator patch and source notices apply. The count-normalization values
in the probes are explicitly synthetic caller inputs; the original L1 table and
complete instrument integration are not claimed. No preset/video data is used
for this component. See the [active firmware DSP workplan](docs/firmware-dsp-workplan.md).

## Cluster normalization layout and factory delta

The [initialization delta](docs/cluster246-normalization.md) uses the documented
OP-1 partial-encryption layout described in
[sualk/op1-decryptor's README](https://github.com/sualk/op1-decryptor/blob/20f86baa116b4c7b1da7e0d4e3a24f171c76767b/README.md)
and the previously credited op1-sdk internals. No decryptor implementation is
copied or bundled, and no encryption key is recovered or distributed. Five
unchanged scalar words are converted using the original helper in the private
oracle; the remaining three entries stay unknown. The independent constructor
and fixture harness additionally verify the original six-slot object factory.

## Cluster shared initialization and fractional conversions

The [shared-initializer reconstruction](docs/cluster246-shared-init.md) and
binary32-to-Q15/Q31 helpers are independently written from private instruction
observations of the operator-supplied firmware. Inputs are explicit numerical
test values; no three missing normalization entries are inferred. REA 6.8.0 was
used for read-only auxiliary SQLite schema/sample inspection. Its ELF provider
was unavailable; GNU Binutils checked the private harness layout instead. The
support summary preserves the distinction and does not attribute Blackfin
decompilation or L1 decryption to REA. GNU model outputs with and without the
previously credited local patch agree for these new cases; this is not hardware
validation. Original firmware/ELF, databases and instruction bytes stay private.

## Cluster Core routing and shared six-slot state

The [Core callback reconstruction](docs/cluster246-core-routing.md) independently
implements Cluster note-on/release behavior observed in the operator-supplied
rev246 firmware. Its own harness explicitly schedules six interleaved slots;
it does not implement the original allocator, clock, ADSR routing or mixer.
Only the five previously verified normalization counts are selected. REA 6.8.0
read-only SQLite evidence distinguishes the factory DB's Cluster row ID 1 from
runtime engine type 4, independently checked through the original Core factory.
No presets are read for this delta. Source databases, instructions, firmware and
ELF remain private; only own code and numerical fixtures/metadata are published.

## Common ADSR transition dependency

The independently written [ADSR transition reconstruction](docs/cluster246-envelope-transitions.md)
uses private original callback/render calls from the user-supplied rev246 image
and the previously credited GNU model. Pending words and event order are explicit
test inputs; their complete runtime consumers, MIDI mapping, Cluster envelope
routing and mixer are not inferred. No preset/video data is used. Original
instructions, firmware and ELF are not included; the new header is not yet used
by the plugin. Existing ADSR kernel/provenance notices continue to apply.

## Cluster VoiceCtl activation and note-table observations

The [activation reconstruction](docs/cluster246-note-activation.md) independently
implements an original SDRAM VoiceCtl route specialized to Cluster. The numeric
128-note table is checked through an unchanged bounded original copy fragment in
a private probe, not through the complete initializer. Tuning is an explicit
caller input; no original tuning calibration is claimed. Following renders use
the harness's held-increment schedule. The L1 handoff remains blocked by mixed
ciphertext, using the previously credited community layout model only. No key,
firmware, original instructions, ELF or L1 image is included, and no preset/video
data is used. The new headers are not linked into the plugin.

## Four-input pitch-buffer modifier

The independent [four-input pitch-buffer modifier](docs/drwave246-pitch-buffer-modifier.md)
and its probe/comparator are AGPL-3.0-only project work. The user-supplied rev246
image provides private original-instruction observations under the credited GNU
BF524 deferred D-register model. REA 6.8.0 `inspect-with-objdump`, using the GNU
Blackfin tool and an own synthetic symbol, retains private static evidence;
REA's `function` command does not support this Blackfin ELF. No L1 decryption,
complete LFO identity, original buffer producer or hardware timing is claimed.
Only numeric inputs/results and independently written source are published.

## Dr. Wave reader and five-oscillator reconstruction

The independent [Dr. Wave reader](docs/drwave246-reader.md) is checked against
unmodified SDRAM instructions from the operator-supplied rev246 firmware using
the previously credited GNU BF524 model and local D-register patch. All six wave
buffers are synthetic test inputs authored here, not recovered factory resources.
The numerical reciprocal seeds, object layout and callbacks are private firmware
observations; no original code, firmware image or ELF is published. The probe
verifies only the reader route outside the scheduled wave-update slot and the
inactive early return. Wave generation, complete scheduling and plugin integration
remain pending. The community op1-sdk provided candidate addresses; its WHEEL DSP
is not copied or presented as the original Dr. Wave. No new REA decompilation or
L1 decryption is claimed for this delta.

## Dr. Wave generated base banks

The [base-bank reconstruction](docs/drwave246-wavebank.md) independently computes
six banks of harmonic-sum tables from the previously verified sine. The private
oracle executes the original shared initializer and complete bank builder from
the user-supplied rev246 image using the credited GNU BF524 model and D-register
patch. Only own source/harnesses, numerical test inputs/outputs and metadata are
published. Factory presets, sample recordings, firmware, original instructions
and ELF are not included. Non-default gains are explicit stress inputs. The
scheduled control-driven wave update and full engine integration remain pending;
these generated base tables alone do not implement that route. No WHEEL synthesis
code or new REA decompilation/decryption is used for this delta.

## Dr. Wave wave-buffer transforms

The [three transform reconstructions](docs/drwave246-wave-transform.md) are
independently written from private observations of the operator-supplied rev246
SDRAM routines, executed unmodified using the credited GNU BF524 model and
D-register patch. Internal Q16 words, synthetic wave buffers and callback order
are explicit inputs; no knob mapping, spectral mixer or complete voice update
is inferred. Numerical fixtures include shared caches, lazy flags, guards and
unmodified neighboring voices/configuration. Original instructions, firmware,
SDRAM and ELF remain private. No presets, WHEEL DSP, or new REA L1 decryption
or decompilation are used or distributed for this delta.

## Dr. Wave spectral callbacks and complete isolated render

The [spectral and complete-render reconstructions](docs/drwave246-kernel.md) are
independently written from private observations of the user-supplied rev246
SDRAM routines. Their oracles execute the original base-bank initializer, six
spectral callbacks and entire render unmodified under the previously credited
GNU BF524 model and D-register patch. Only own source/harnesses, numerical test
fixtures and metadata are published. Presets, firmware, original instructions,
SDRAM images and ELF remain private. Full raw-control preparation, internal
update order and shared caches are checked; the external scheduler, allocation,
pitch calibration/provider, ADSR, output and plugin integration are not inferred.
The existing reader shares its inner loop with the complete adapter and retains
its regression oracle. No WHEEL synthesis or new REA L1 decryption is used.

## Dr. Wave counter-consumer reconstruction

The [16-bit dispatch extension](docs/drwave246-counter.md) is independently
checked against every input word, twice with different neighbor halfwords,
by inserting the exact private dispatch fragment in an own bounded caller.
A separate oracle calls the entire original render unmodified across explicit
counter boundaries; previous fixtures remain unchanged. Only numerical results,
own code and metadata are published. The partially encrypted L1 reference is
recorded as a candidate, without inventing its missing pointer/store/flow.
The 8-of-24 format fact uses the already credited community documentation;
no new REA decryption, recovered clock or hardware timing is claimed.

## Dr. Wave Core event routing

The [Core event adapters](docs/drwave246-core-routing.md) are independently
written and checked against the original Core factory, note-on/release callbacks
and full Dr. Wave render under the previously credited GNU BF524 model. Explicit
event scheduling, pitch and valid synthetic vtable tags are harness inputs;
common ADSR release and complete instrument integration are not inferred.
REA 6.8.0 supplies a fresh read-only SQLite observation of `synth_types.id/type`
to distinguish database ID 3 from runtime type 2. No preset rows, original code,
firmware or private ELF are distributed by this delta. The previously credited
GNU tools and deferred D-register patch remain the instruction-level oracle.

## Dr. Wave VoiceCtl activation and shared register preparation

The [Dr. Wave activation](docs/drwave246-note-activation.md) is independently
checked against original VoiceCtl, Core, constructor/reset and complete render
instructions using the previously credited GNU model. The original bounded
128-word note-table copy is re-executed privately, with an own return appended.
Only numerical observations, own source/harnesses and metadata are published.
Tuning and the held render pitch remain explicit inputs, not recovered original
calibration or scheduling. Shared register preparation is extracted from the
previous Cluster adapter; its unchanged fixtures still pass, including Windows
under Wine and sanitizers. No firmware, original instructions, private ELF, new
presets or L1 decryption are distributed or claimed by this delta.

## Four-source volume/pitch buffer modifier

The independent C++ reconstruction and own harness are checked against private
rev246 observations under GNU BF524. A second local GNU GDB 13.1 patch,
`research/blackfin/gnu-13.1-halfword-writeback.patch`, preserves disjoint halfword
loads in the pending write queue. It retains GPL-3.0-or-later and is separate
from the instrument. Own packet probes and numerical observations are published;
firmware, original instructions and ELF remain private. See the
[contract and model limits](docs/drwave246-volume-buffer-modifier.md).

## ADSR lazy pool and consolidated cold-start event chain

Independent C++ reconstruction of the six-state rev246 ADSR accessor, with
1,024 numeric original-instruction comparisons and 1,536 composed cold-start
events. The preceding metadata-preserving notification contribution `ec1c182`
is retained with its original authorship. GNU BF524 with both documented
writeback patches provides numeric observations; REA 6.8.0 / GNU objdump
provides bounded static Evidence. Firmware/PDF, original instructions, ELF and
disassembly remain private; none are linked or packaged in the instrument.
[Contract and limitations](docs/adsr246-lazy-pool.md).

## Shared instrument-event dispatcher

Independent composition of verified rev246 note/voice helpers into a shared
instrument dispatcher, preserving the older bounded interfaces. 3,072 original
events cover selectors 0–4, constructor/raw pitch, cold/warm pools and retained
selector changes. GNU BF524 provides numerical observations; REA 6.8.0 / GNU
objdump provides bounded static inspection. No original instructions, firmware,
ELF, disassembly or presets are packaged.
[Scope and limitations](docs/voice246-instrument-lifecycle.md).

## Blackfin boot decompilation research

Ghidra 12.2 (NSA/contributors, Apache-2.0), geepot/ghidra-blackfin 1.2.0
(GPL-3.0-or-later) and Eclipse Temurin 25.0.5+8 (GPLv2 with Classpath Exception)
are operator-side research tools. Verified release asset digests and six private
decompilations are documented in [the boot investigation](docs/boot246-decompiler.md).
Two original XTEA routines and their privately host-adapted arithmetic pseudocode
are contrasted using synthetic keys. U-Boot v2014.10 Boot ROM definitions identify
the OTP calls. No tool binaries, original pseudocode/firmware, OTP values or keys
are packaged or linked with Pocket One. The existing unlicensed decryptor
reference remains a format reference only; its source is not copied.

## Tremolo 246 (Dual LFO)

Independent reconstruction of the rev246 voice Tremolo callback and note reset,
with five waveforms, pitch/amplitude routing and 24 float32-to-Q31 coefficient
conversions. GNU BF524 with the previously credited D-register/halfword patches
provides numerical observations: 2,048 calls, 262,144 compared positions per
buffer (212,992 active), full self/display bytes and retained streams. Own sample
and buffer adapters reproduce the same callback results. Firmware, original
instructions, disassembly and ELF stay private. No original presets or new tool
binaries are packaged. The instrument clock, display ownership and voice/output
routing remain explicit adapters. [Scope and reproduction](docs/tremolo246-native.md).

## Random synth 246

Independent reconstruction of the readable rev246 synth Random callback and
gate-only note reset. 4096 original BF524 simulator calls compare all state, RNG,
meter, eight target words and return, independently repeated. Instrument uses
eight signals in two passes on four controls; ADSR uses four. Host cadence,
shared dedicated RNG scheduling and seed are authored adapters. Firmware and
original instructions are not distributed. See [scope and reproduction](docs/random246-native.md).

Filter 246: reconstrucción independiente del callback mono y de sus tablas, con
oráculo GNU Blackfin y decompilación privada Ghidra. Los candidatos se localizaron
con el SDK OP-1 de referencia ya acreditado (revisión
`ae1945a9c61468f0e0d7cfdd5dd60c2cf9731ff1`). No se copió su DSP. Se reutiliza la
curva numérica contrastada de Punch; el bus, resampling, DC y mezcla son propios.
[Pruebas y límites](docs/filter246-native.md). Firmware y decompilaciones no incluidos.

Nitro Hybrid: reconstrucción independiente de la ruta mono legible y pruebas GNU
Blackfin con sustituciones explícitas. Las posiciones y etiquetas se contrastaron
con el SDK de referencia ya acreditado y fijado a la misma revisión. Se reutilizan
el seno y frecuencia contrastados; la mezcla simétrica de reemplazo, sus punteros
y el adaptador de salida son propios. [Contrato](docs/nitro246-hybrid.md). No se
atribuyen a firmware descifrado ni se incorpora código del SDK.

Delay 246: reconstrucción independiente del callback mono, reset y tabla Q15
mediante GNU Blackfin y análisis privado Ghidra; localización candidata a partir
del SDK fijado ya acreditado. Se publican resultados numéricos y harness propio,
sin código/ELF/decompilaciones del firmware. Workspace, conversión de tasa y bus
son adaptadores propios. [Contrato y reproducción](docs/delay246-native.md).

Grid 246: reconstrucción independiente del callback mono, reset y escaneo de dos
planos, con oráculo GNU Blackfin y decompilación privada Ghidra. Candidatos del SDK
ya acreditado y fijado a la misma revisión; sin copiar su DSP. Se reutiliza el seno
contrastado. Workspace acotado, resize, bus y selector son adaptadores propios.
[Contrato](docs/grid246-native.md). Firmware y pseudocódigo no se distribuyen.

Phone 246: reconstrucción independiente del callback mono y soft reset mediante
GNU Blackfin y análisis privado Ghidra; candidatos del SDK fijado ya acreditado,
sin copiar su DSP. Coeficientes numéricos de fragmentos legibles compartidos con
Filter/Delay. Workspace acotado, guardas y pánico completo del host son propios.
[Contrato](docs/phone246-native.md). Firmware/ELF/pseudocódigo permanecen privados.

La investigación de Spring 246 en 0.24.1 contrasta únicamente el reset y dos
callbacks de borrado diferido con estados y workspaces sintéticos explícitos.
El modelo de ciclo de memoria, harness, pruebas y corrección del volumen de
salida son propios; no se incorpora pseudocódigo ni código DSP del SDK externo.

Spring 246 (0.25): análisis propio del inicializador y callback mono, con ejecución
acotada y repetición independiente. Kernel C++, adaptador, UI, harness y regresión
propios; las fixtures contienen entradas sintéticas y resultados generados, sin
instrucciones ni pseudocódigo del fabricante. No se copia DSP del SDK externo.

Fundamentos CWO 246: análisis propio de productores/auxiliares y ejecución acotada
con datos explícitos. Cuatro coeficientes protegidos usan un diseño de cuadratura
propio con NumPy/SciPy; no se presentan como firmware descifrado. Reconstrucciones
C++, harness, fixtures sintéticas y validación propios; no se distribuyen código
original, ELF ni pseudocódigo de Ghidra. La composición completa no se integra aún.

CWO Hybrid 0.26 añade la reconstrucción propia de retardo/oscilador y composición
del callback mono, contrastados con estados y workspaces explícitos. La coincidencia
depende de los cuatro coeficientes propios declarados; no se distribuyen código
original ni pseudocódigo. Adaptador, reset inicial, semilla del medidor y ruteo del
host son propios; no representan el allocator/arranque ni la cadena estéreo originales.

## FM internal foundation (instrument remains 0.26.0)

The independently authored diagnostic in `research/fm-study` examines Pocket One's
existing two-oscillator FM using its actual Engine and a mathematical Bessel
reference for its own stationary formula. It contains no original firmware,
manual pages, disassembly or recovered original audio. Its metrics do not claim
OP-1 acoustic equivalence. Own tool source is AGPL-3.0-only.

Independently written reconstruction of the rev246 FM constructor, reset, six
internal envelope update, thirty-word boot conversion and table-copy helpers.
8192 records and six complete factory pools are compared against private original
GNU BF524 instruction execution, with independent repeat and sanitizers. Twenty
clear binary32 words are locked; ten protected words are explicitly authored
substitutions, not decrypted data or acoustically calibrated values. Only own
source/harness, scalar observations and synthetic numeric vectors are published.
No original FM audio renderer or host integration is claimed by this component.
[Scope, source provenance and reproduction](docs/fm246-foundation.md).

## FM Hybrid 0.27.0

`Source/Fm246.h` independently reconstructs the complete mono four-operator FM
callback: nine routings, fixed-point feedback, phase updates, operator weighting
and six internal gain/angle channels. 4096 blocks / 524288 samples, complete
60-byte states, thirty-word table and 512-byte scratch/guards match private
execution of unmodified rev246 SDRAM instructions under the published ten-word
authored table model. Twenty legible source coefficients remain locked.
`Source/Fm246Foundation.h` and `Fm246Model.h` are shared with the earlier independent
verification through forwarding headers. No proprietary code, decompiled
pseudocode, firmware tables containing instructions or original patches are
redistributed; numeric fixtures contain synthetic inputs and observations.

Pitch word rounding, 88.2 kHz clock, per-voice scratch, allocation/steal/glide,
causal filtering/resampling and control smoothing are authored adapters. The
existing reconstructed ADSR is composed with the voice; full original scheduling,
protected constants, acoustic equivalence and actual Live acceptance remain
unverified. The six authored FM recipes are adapted, not original OP-1 presets.
Own implementation, harness and diagnostic/test source are AGPL-3.0-only.
[Scope and reproduction](docs/fm246-hybrid.md).

### FM 0.28.0 performance adapters

The fractional pitch interpolation in `Source/Fm246Pitch.h` is independently
authored and is not recovered firmware arithmetic. Integer pitch preserves the
separate original callback comparison. FM Value/Random numeric vectors are
synthetic stimuli and observations from isolated original callbacks; firmware,
ELFs, disassembly, private media and decompiler output are not distributed.
The four-operator model retains its ten explicit substitutions unchanged.

### String 246

String246.h independently reconstructs the observed mono waveguide arithmetic,
constructor/reset/release and explicit slot-0 binding, using the existing legible
Spring/Delay/Cluster numeric tables without substitutions. op1-sdk's engine_plus
pointer hook supplied the initial render candidate; this is independently checked
against the supplied rev246 LDR and isolated GNU BF524 execution. Numeric replay
vectors contain synthetic stimuli/observations, not original instructions.
String246Voice.h and its clock, pitch/cursor bounds, scheduler, ADSR composition,
voice policies, DC/output filtering and UI are authored host adaptations. The old
Karplus–Strong source remains historical, but is not selected by Engine.

### DNA Hybrid 246

Dna246.h independently reconstructs the original mono render, six live voice
states, factory/reset/release and post-identity-reader transformation. The render
candidate initially came from op1-sdk's engine_plus pointer hooks and was checked
against supplied rev246 bytes and isolated GNU BF524 execution. Fixtures are
synthetic inputs and numeric observations, not original executable instructions.

The hardware OTP reader/ROM and physical memory map are not reconstructed.
Dna246Voice.h uses an authored fixed virtual identity, live six-state pool and
zero bytes outside that pool, plus authored clock, note/pitch/cursor policies,
ADSR composition and output filtering. Four supplied identities, controlled RAM
profiles and an isolated paired RAM experiment condition the numerical comparisons;
they do not recover the physical unit's acoustic profile. No original presets,
private media, disassembly, firmware or decompiler output are redistributed.


### Voltage 246

The candidate callback address came from the previously credited pinned
op1-sdk engine_plus hooks and was verified independently against supplied
rev246 bytes and isolated GNU BF524 execution.

Voltage246.h is an independent reconstruction of the rev246 mono callback,
four-oscillator/XOR/feedback/filter arithmetic and six-slot lifecycle, checked
against isolated BF524 execution. Sine conversion and 129 legible filter
coefficients are verified without substitutions. Only synthetic numeric
fixtures and authored test harnesses are redistributed. Voltage246Voice.h uses
an authored clock, pitch/event/voice policy, ADSR composition, DC/output filtering
and resampling. Labels, init values and four-trace animation are authored.
Numerical parity is conditioned on supplied inputs; it is not acoustic
validation. No firmware, private media/memory dump or decompiler output is included.


### D-synth lifecycle and modulation foundation

The previously credited op1-sdk supplied a candidate pointer; it was checked in
the supplied rev246 vtable. Dsynth246Lifecycle.h independently reconstructs the
72-byte state lifecycle and bounded six-slot capture context. The original audio
callback is executed privately to record synthetic numerical observations; its
audio renderer has not been ported to C++. Value/Random type12's eight direct
destinations are independently reconstructed and numerically checked. Ghidra12.2
with Blackfin1.2.0 was used as a private aid; decompiler output is neither trusted
as audio equivalence nor redistributed. No firmware, memory dump, original
instructions or decompiled code is shipped.


### D-synth 246 complete render and FP-load simulator fix

Dsynth246.h independently reconstructs the mono render, internal envelopes,
oscillator morphing, eight mixes and filters. Complete audio/state/buffer parity
is checked against isolated GNU BF524 execution with caller-supplied pitch/gain.
No original instructions or decompiler output are included. Dsynth246Voice.h
uses authored host clock, pitch/allocation/velocity policies, ADSR composition,
quiet detection, output filtering/resampling and UI; init values are authored.

The GNU GDB13.1 FP-relative Dreg load path bypassed deferred packet writeback.
The additional GPL3-or-later patch uses SET_DREG in that path; authored source-
latching probes and Analog Devices' Blackfin Programming Reference rev2.2 §4-9
support this research correction. Tools are not linked into the product. Twelve
existing original-instruction oracles preserve their numerical results under
this profile, including all previously documented hybrid substitutions. Hardware
acoustics remain unmeasured. The historical foundation corpus is versioned in
its original commit; the current corpus uses the corrected model and signed gain.

### D-box 246

Dbox246.h independently reconstructs the isolated mono callback, two internal
envelopes, eight mix selections, lifecycle/activity query and twelve-slot context.
Synthetic numerical observations compare complete state/audio/buffers against
private rev246 execution under the previously documented three-writeback-patch
GNU BF524 model. Existing ADSR/filter numeric observations are reused without
coefficient substitution. No firmware, original instructions, decompiled code,
factory patches or private tool files are included. Ghidra12.2/Blackfin was a
private aid; simulation/disassembly and full numerical comparison bound the claim.
Dbox246Voice.h and integration add authored 88.2kHz clock, pad/tuning map,
smoothing, event/velocity/allocation policies, output interpolation/filtering,
quiet detection, init, labels and graphics. Shared Tremolo is reused with authored
drum composition; original drum Value/Random dispatch and acoustic validation
remain pending. The legacy Drum synth remains explicitly authored.

### Drum sampler 246 and host kit adaptation

Drum246.h independently reconstructs the isolated mono reader, coarse/fine
positioning, forward/reverse/loop branches, lifecycle/activity and saturating
16-tap FIR. Drum246Tables.h contains numerical observations of all 1025 legible
coefficients after the original float-to-fract16 conversion. Complete synthetic
PCM/state/buffer observations are compared under the previously documented GNU
BF524 profile with three writeback fixes. PCM guards, pitch/gain streams and event
order are supplied; no firmware, factory audio/patches or decompiled code is shipped.
Ghidra/Blackfin was a private aid, not a substitute for the full comparison.

SampleData.h and Drum246Voice.h add authored offline linear import conversion to
PCM16/44100 Hz (first twelve seconds), PCM alignment/guards, host clock/pitch,
independent pad bank, allocation/stealing, output resampling and timing policies.
The dynamic region/level curve is our model informed by operator-supplied manual
v2.2 §6.4, not a recovered or numerically validated original envelope producer.
Tremolo is composed with that supplied gain using our host policy. Original
shared-source kit layout, dynamics producer, drum LFO dispatch and hardware
acoustics remain pending. Original source audio is retained for sessions/legacy use.

### Melodic sampler 246

Sampler246.h independently reconstructs the complete isolated six-state reader,
restart/release and FIR/loop/direction/pitch arithmetic. Existing 1025 numeric FIR
observations are reused without substitution. Synthetic source/control/state and
signed-pitch observations compare full pools, audio, unchanged pitch and guards
under the documented GNU BF524 profile. Explicit zero-filled address views are
harness inputs, not recovered physical RAM. No firmware, original instructions,
factory material or decompiled code is distributed. Ghidra/Blackfin assisted private
inspection, with simulation/comparison bounding the claim. Sampler246Voice.h and
host integration supply our cache/clock, trim-to-loop map, alignment, output
adaptation, ADSR/velocity/allocation policies and UI. Original secondary controls,
full voice/routing, acoustic calibration and drum dynamics remain pending.

### Sampler controls and explicit voice composition (0.36)

Sampler246Controls.h names/exposes the eight previously reconstructed reader
fields and reuses the shared verified ADSR kernel/tables. No new firmware bytes
or factory patches are included. The independent 32768-sample composition test
compares reader pool/state, envelope state/dither and audio, with a supplied call
order; it does not validate the original full voice composer. Physical source
loading/clock, loop permission gate, dither seed/cadence, event/velocity/allocation
policy, portamento and output adaptation remain authored. Unit conversion,
parameter banks, waveform markers, host routing and session/UI integration are
our implementation. Original reader fixtures remain unchanged as a regression.

## Sampler modulation (0.37.0)

Independently authored sampler region arithmetic and host integration are checked
against 4096 Value and 4096 Random rev246 callback observations, including coupled
coarse/fine boundaries, signed stress and all state/display/target/RNG/tempo outputs.
GNU BF524 uses the three documented writeback patches. Fixtures contain synthetic
inputs/numerical observations only. Control clock, Reverse button mapping, RNG
schedule, voice/Tremolo composition and output are authored policies, not recovered
full instrument routing. No original instructions, firmware, factory sounds or
private disassembly are distributed. [Scope](docs/sampler246-modulation.md).

## Drum / D-box modulation (0.38.0)

Independently authored drum region arithmetic, separate Value/Random dispatch and
host integration are checked against 16384 isolated rev246 callback observations
for types 8/9, including full states, displays, RNG, destinations, returns, tempo
and both initialized descriptors. GNU BF524 uses all three documented writeback
fixes, checked by probes. Only synthetic inputs/numerical observations are recorded.
The original gain producer is not reconstructed; authored dynamics/tuning/clock,
allocator/events/RNG order, PCM mapping and output remain explicit adaptations.
No firmware, original instructions, factory patches/audio or private disassembly
are redistributed. [Scope and reproduction](docs/drum246-modulation.md).


## MIDI LFO 246 (0.39.0)

`Source/Midi246.h` is an independently written reconstruction of the readable
rev246 synth/drum/FX MIDI modulation dispatch, tested against numerical execution
observations. The supplied inputs, complete affected/unchanged buffers and return
values match for 6,144 isolated callback cases. The previously tested range and
coupled-region arithmetic is reused. No physical MIDI receiver, protected memory,
original scheduler, complete voice or acoustic equivalence is claimed.

`research/blackfin/midi-lfo-stream.s.in` is our test harness. The compressed
fixtures contain synthetic inputs and numerical observations only, not firmware
or disassembled code. Private binaries remain outside the repository. The host
route choices, fixed CC1–4 mapping, merged channels, event latch, UI, persistence
and audio integration are authored adaptations. Gain/pitch/FX audio modification
is verified only with supplied buffers and remains outside the exposed adapter;
FX kind 4 and auxiliary ADSR selectors are excluded. See
[scope](docs/midi246-hybrid.md) and [evidence](docs/firmware/midi246-validation.json).


## Shared drum source (0.40.0)

`SharedDrum246.h` and the source ownership, import/capture, region UI and portable
snapshot adaptations are authored work informed by manual sections 6.2–6.3. The
initial equal partition and offline conversion are not recovered OP-1 algorithms.
The verified drum reader is reused. The 105 composition cases compare one shared
asset against copies of the same complete PCM with identical absolute regions;
these are host-adapter tests, not new original-firmware or acoustic comparisons.
No new firmware or proprietary assets are distributed. See [scope](docs/shared-drum246.md).


## Bend LFO 246 (0.41.0)

`Bend246.h` independently reconstructs the readable shared manual-input arithmetic
and bounded parameter dispatch, identified through rev246 factory/vtable and
`LfoBend.cpp` diagnostic references. 6,144 interleaved synth/drum/FX observations
match complete shared state/history/flags, all controls/targets and returns,
with synthetic pitch/gain/audio buffers unchanged and independent reproduction.
The contract excludes special audio/pitch dispatch and FX kind-4 auxiliary ENV.

`Bend246Control.h`, host activation/clock, virtual encoder, contextual controls,
state and LFO selector are authored adaptations. Supplied encoder controls and
explicit prepare/reset actions do not establish the physical input path or
original scheduler. No firmware, decompiled source, OTP or private memory dump is
distributed. The gzip fixtures contain synthetic inputs and numerical observations.
See [scope](docs/bend246-hybrid.md) and [evidence](docs/firmware/bend246-validation.json).


## Crank LFO 246 (0.42.0)

`Crank246.h`, its host adapter, native tests and assembler probe driver are
independently authored. Ghidra 12.2/Blackfin and the corrected GNU BF524 simulator
were used privately to study the user-supplied rev246 and compare 18,432 bounded
interleaved callbacks plus an independent repeat. Fixtures contain synthetic
inputs and numerical observations, not firmware or decompiled source. Manual
section 15.7 informed the virtual blue encoder; host clock/lifecycle and UI are
authored. Physical accessory input and sequencer Crank remain unvalidated. See
[scope and reproducibility](docs/crank246-hybrid.md).


## Element LFO 246 (0.43.0)

`Element246.h`, its host frontend, native tests and Blackfin driver are independently
authored. Ghidra 12.2 and the corrected GNU BF524 simulator were used privately
to compare 18,432 bounded LfoReRoute callbacks and an identical independent repeat.
Fixtures contain synthetic inputs and numeric observations, never firmware or
decompiled source. Manual section 15.2 informed the four source choices. Virtual
sensor, host input windows, max voice ADSR and pre-FX level mapping are authored
adaptations, not recovered original producers. [Scope](docs/element246-hybrid.md).


## Envelope publication research (after 0.43.0)

The independent research-only `EnvelopePublisher246` model and its own Blackfin
probe compare 4,096 complete original rev246 blocks / 524,288 audio halfwords,
shared publication and reset/getter without patching original code. Ghidra 12.2
and the corrected GNU BF524 simulator were research tools; no firmware or
decompiled source is distributed. Fixtures are synthetic inputs and numerical
observations. The active caller, control semantics and product routing remain
unvalidated; this model is not linked into Pocket One. [Scope](docs/element246-producers.md).

The follow-up authored composition probe executes the full publisher and 8,192
Element consumer callbacks under explicit before/after scheduling, without
replacing the shared word between them. The scheduling is synthetic, not original
voice/audio routing. Only numerical fixture observations are distributed.
[Composition scope](docs/element246-composition.md).


## Native Apple Silicon validation

The authored macOS probes use CoreFoundation to load the actual VST3 bundle and
check its factory lifecycle, and Foundation/CoreGraphics to observe startup of
the actual standalone process. They distribute no Apple SDK or firmware code.
The native CI architecture guard, full product suite and Envelope research suites
complement the existing universal packaging checks; no Live/audio-device or
acoustic equivalence is inferred. [Scope](docs/native-macos-arm64.md).


## MIDI FX input audio — 0.44.0

The two original MIDI FX audio curves retain their wrapping/saturating Q15
arithmetic and stream order. The per-sample host bridge is compared against 21,568
audio observations from the existing 6,144-call oracle, not a new firmware run.
Placement before the mono FX bus, sample clock and CC receiver policy are authored
adaptations; no original audio routing or acoustic equivalence is inferred.
[Scope](docs/midi246-fx-input.md).


## MIDI voice gain and pitch — 0.45.0

Recovered gain smoothing, sequential saturated Q15 scaling and cubic/Q31 pitch
arithmetic are kept in the independent MIDI special helpers. Both streaming and
blocked host bridges match 11,392 pitch/gain pairs and display words from existing
original observations; the complete 6,144-call oracle is retained. No new firmware
execution count, original display initialization/ownership, buffer producer or
scheduler is inferred. Per-voice zero initialization, clock, routing, lifecycle
and existing engine bounds are authored adaptations. [Scope](docs/midi246-voice.md).

## Random FX modulation (0.46.0)

The independently written Random FX parameter callback and its four-control host
adapter were compared with 4,096 isolated rev246 callbacks using the same credited
GNU Blackfin tooling and supplied firmware. Numeric synthetic fixtures and a
reproducible harness are included; firmware, instruction bytes, disassembly and
private ELF files are not. Shared-bus ownership, clock, seed and lifecycle are
host policy, not recovered original scheduling. See [scope](docs/random246-fx.md).
