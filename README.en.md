# Pocket One 0.39.0

**MIDI LFO**: four incoming CC streams targeting Instrument, ADSR and FX, with
automation, persistence and contextual encoders. 6,144 original callback cases
verified under supplied inputs; authored receiver and host timing.
[Scope](docs/midi246-hybrid.md) · [validation](docs/releases/v0.39.0-validation.json).

Drum and D-box: **eight-target Value/Random**, banks, state, control routing and UI.
16,384 verified callback cases; ADSR rejected. Own dynamics, tuning and clock.
[Scope](docs/drum246-modulation.md) · [validation](docs/releases/v0.38.0-validation.json).

**Melodic sampler 246**: reconstructed six-voice FIR reader, reverse and loops.
Clock and voice composition remain authored; optional ADSR 246; [scope](docs/sampler246-hybrid.md) · [validation](docs/releases/v0.35.0-validation.json).

**Drum sampler 246**: reconstructed FIR reader, twelve voices and Gated/One Shot/Loop.
Dynamics remains an authored model; [controls and limits](docs/drum246-native.md) · [validation](docs/releases/v0.34.0-validation.json).

**D-box 246**: 24 pads, twelve voices, eight controls per pad and reconstructed internal envelopes.
[Controls and limits](docs/dbox246-native.md) · [Validation](docs/releases/v0.33.0-validation.json).

**D-synth 246**: eight controls, two oscillators, internal envelopes and eight reconstructed mixes.
[Scope and controls](docs/dsynth246-native.md) · [Validation](docs/releases/v0.32.0-validation.json).

**Voltage 246**: tenth engine, four oscillators, XOR/feedback and original filter coefficients.
[Scope and limits](docs/voltage246-native.md) · [Validation](docs/releases/v0.31.0-validation.json).

**DNA Hybrid**: ninth engine, explicit virtual identity, six voices and modulators.
[Scope and limits](docs/dna246-hybrid.md) · [Validation](docs/releases/v0.30.0-validation.json).

**String 246**: reconstructed waveguide, four controls, ADSR, six voices and modulators.
[Scope](docs/string246-native.md) · [Validation](docs/releases/v0.29.0-validation.json).

FM **0.28.0** adds authored fractional tuning and Value/Random/Tremolo routing.
[Scope and limits](docs/fm246-performance.md) · [Validation](docs/releases/v0.28.0-validation.json).

An OP-1-inspired **VST3 instrument and standalone application**, written in C++17 with JUCE 8. Independent of Teenage Engineering; no original firmware, factory samples, graphics or branding are included. The interface uses a grey enclosure, dark animated display, blue/green/white/orange encoders and two rows of button keys.

![Actual JUCE editor](docs/interface.png)

Version 0.27.0 replaces the authored two-oscillator FM with **[four-operator FM Hybrid](docs/fm246-hybrid.md)**: nine verified routings, six voices, reconstructed ADSR, raw controls, automation/state and Mono/Legato/glide. Its 524,288-sample comparison is conditioned on ten explicit authored table constants, retaining twenty legible words. Pitch/clock/output adapters remain authored; no acoustic equivalence or original preset replication is claimed.

Version 0.26.0 adds **[CWO Hybrid](docs/cwo246-hybrid.md)**: Frequency, Delay, Feedback and Sideband, with Value FX, automation and saved state. Complete mono callback verified under four explicit authored phase coefficients; the four legible coefficients are preserved.

Version 0.25.0 adds **[Spring 246](docs/spring246-native.md)**: mono reverb with Tone, Turns, Damping and Send, Value FX, automation and saved state. Static initialization, coefficients and nine delay lines match the bounded instruction oracle.

Version 0.24.1 fixes [master volume](docs/output-routing.md) to scale complete effect tails, with settled silence and retained effect memory. The [Spring lifecycle](docs/spring246-lifecycle.md) is verified as the foundation for the 0.25 integration.

Version 0.24.0 adds **[Phone 246](docs/phone246-native.md)** on **set 3 → FX → Phone 246**: Tone, Phonic, Baud and Telematic, with Value FX, automation and saved state. The mono segmentation, filters, resampler and complete buffer states match the bounded instruction oracle. Explicit guard cells contain the maximum-length sentinel; full allocation/routing remain authored. Shared output latency is unchanged.

Version 0.23.0 adds **[Grid 246](docs/grid246-native.md)** on **set 3 → FX selector → Grid 246**: X Size, Y Size, Feedback and Mix, with Value FX, automation and saved state. Its mono kernel and two memory planes match the bounded instruction oracle. A shared effect selector replaces the separate buttons; output latency is unchanged. Abrupt-resize normalization remains an authored adapter.

Version 0.22.0 adds **[Delay 246](docs/delay246-native.md)** on **set 3 → DELAY 246**: Size, Speed, Feedback and Mix/Send. The mono core, 225 generated coefficients and circular memory match the legible instruction oracle. Value FX, automation and persistence are integrated on the existing fixed-latency output bus. Full original routing and hardware sound matching remain pending.

Version 0.21.0 adds **[Nitro Hybrid](docs/nitro246-hybrid.md)** on **set 3 → NITRO HYBRID**: two frequencies, bipolar Follow and Resonance, with Value FX, automation and saved state. Readable arithmetic is retained with explicit authored substitutions for protected coefficients/pointers. Conditional oracle matching does not establish original Nitro sound equivalence. It shares Filter’s existing output latency.

Version 0.20.0 adds **[Filter 246](docs/filter246-native.md)** on **set 3 → FILTER 246**: Cutoff, Resonance/Follow, Mode and Drive, with Value FX, MIDI and state persistence. The mono core matches 262,144 original-instruction oracle samples. Its authored output-bus adapter reports a fixed latency of about 1.5 ms, including bypass. Hardware matching and real Ableton acceptance remain pending.

Version 0.19.0 adds **[Random 246](docs/random246-native.md)** on **set 4 → RANDOM 246** for Digital/Phase 246 and Dr. Wave/Cluster: amount, speed, Instrument/ADSR destination and LFO envelope. Original RNG, smoothing and both Instrument passes are reconstructed. Host cadence and RNG scheduling remain adapters. MIDI, automation, sessions, slots and clipboard are integrated; original presets remain deferred.

Version 0.17.0 adds **[Mono, Legato and Glide for Dr. Wave / Cluster](docs/hybrid-performance.md)**. On **set 2**, choose the voice mode and 0–2000 ms portamento. Last-note priority, per-channel sustain/bend, automation and session/slot/clipboard persistence. These are authored performance adapters; the verified DSP cores remain unchanged.

Version 0.16.0 extends **[Value Voice to Dr. Wave and Cluster](docs/value-hybrid246-native.md)**: independent modulation of all four instrument controls or ADSR, verified routing/ranges, tempo/free rate and FREE phase. Enable **set 4 → VALUE 246 VOICE**.

Version 0.15.0 adds a playable **[Cluster hybrid](docs/cluster246-hybrid.md)**: six voices, up to eight oscillators per voice, four controls, ADSR, MIDI and state persistence. Choose **08 / CLUSTER** or **Cluster 246 Hybrid → Init / Cluster**. CI also produces a [universal macOS installer and portable ZIP](docs/macos-packaging.md); native package checks belong to that CI run.

Version 0.14.0 adds a playable six-voice **[Dr. Wave hybrid](docs/drwave246-hybrid.md)** with four controls, ADSR, MIDI and state persistence. Choose **07 / DR. WAVE** or **Dr. Wave 246 Hybrid → Init / Dr. Wave**. The verified core uses authored adapters for missing L1 dependencies; full hardware sound matching remains pending. Previously published downloads below remain 0.13.0.

Version 0.13.0 adds **[Value 246 Voice](docs/value-voice246-native.md)**: independent per-voice modulation of Phase/Digital controls or common ADSR. Five additional studies reuse the set-4 MIDI knobs; old FX parameters, mappings and projects are preserved. Other engines/routes and hardware matching remain pending; [full coverage](docs/original-op1-coverage.md).

**Active work: hybrid instrument reconstruction.** [Dr. Wave](docs/drwave246-hybrid.md) and [Cluster](docs/cluster246-hybrid.md) are playable six-voice engines with ADSR, Value and performance modes. Their verified cores retain documented authored adapters for scheduling, voice/output and unrecovered L1 dependencies. See the [component workplan](docs/firmware-dsp-workplan.md) and [current status](docs/project-status.md). Complete original presets remain deferred.

**Dr. Wave voices:** [Core factory and events verified](docs/drwave246-core-routing.md) against another 61,568 samples and pre-render voice pools. [VoiceCtl activation](docs/drwave246-note-activation.md) adds pitch, velocity and activity with explicit tuning and another 61,568 samples. Engine release does not replace the pending common ADSR route.

**Dr. Wave counter:** [all 16 consumer bits verified](docs/drwave246-counter.md), with 131,072 fragment calls and another 61,568 full-render samples crossing counter boundaries. The original clock provider remains pending in partially encrypted L1.

**Dr. Wave base tables:** [six banks / 320 generated waves](docs/drwave246-wavebank.md) from the verified sine, matching 1,474,560 table samples across 18 initializations and now used in the complete-render oracle.

**Dr. Wave transforms:** [three reconstructed callbacks](docs/drwave246-wave-transform.md) match 131,072 wave words and cache state; the complete render verifies their actual order. Six spectral callbacks add 196,608 separately checked words.

**Cluster progress:** [Core note-on/release routing and six interleaved slots](docs/cluster246-core-routing.md) checked against another 131,072 samples with shared RNG. The [five observed normalization entries](docs/cluster246-normalization.md) and [shared initialization with REA support](docs/cluster246-shared-init.md) remain available. Three normalization entries and full integration remain unresolved.

As a dependency of that route, [common ADSR transitions](docs/cluster246-envelope-transitions.md) add 327,680 checked samples: initialization and a fade before the new attack. Full Cluster envelope routing remains pending; this does not change the installed plugin.

[VoiceCtl note preparation](docs/cluster246-note-activation.md) checks 128 pitch words, velocity and activation with an explicit tuning coefficient. The ADSR handoff remains pending in partially encrypted L1.

Version 0.8.0 added optional **[Phase 246 CORE](docs/phase246-native.md)**: a playable independent Pd reconstruction at a fixed 88.2 kHz internal clock with 16 additional raw-control patches. Its core is tested against original instructions in the GNU simulator; NCO, envelope, modulation and output filtering are ours. Existing Phase presets remain available.

Version 0.7.0 added four stable **Screen: Knob 1 Blue / 2 Green / 3 White / 4 Orange** host parameters. Map them once and the assignments follow the current display across sets 1–4. Old mappings to individual sound parameters keep their original destination. **PURGE BANK** permanently deletes the selected imported bank (or all imported banks with All banks selected), unloads its active audio, and clears its library references after confirmation. Factory sounds, source ZIPs and already saved projects are preserved. Close other instances first. Reimporting an identical manifest replaces validated copies and repairs missing audio without accumulating directories.

[Project status, approximate progress and next delivery batches](docs/project-status.md).
The [lazy ADSR pool and cold-start event chain](docs/adsr246-lazy-pool.md) are now
verified; full Dr. Wave integration remains pending.

The [shared voice-event dispatcher](docs/voice246-instrument-lifecycle.md) now
checks 3,072 events across selectors 0–4, cold starts and selector changes.
Dr. Wave pitch/ADSR handoff and complete audio integration remain pending.

## Instruments and controls

[Additional Phase 246 core study bank](docs/phase-core-studies.md): 16 sample
presets sharing 13 unique WAVs, compatible with 0.7.0. Raw stored controls are
applied to the independently reconstructed Pd core. Original control routing,
ADSR/FX/LFO/octave behavior and full voice equivalence remain unverified. The
current factory/user banks and published installers are preserved.

Eight-voice velocity-sensitive **FM**, **phase distortion**, **plucked string**, **sampler**, **drum** and **24-pad sample kit** engines. Select the engine above the display, use the factory-sound menu, or play MIDI and the on-screen keyboard. The arrow buttons transpose the keyboard in sound modes. Computer keyboard notes start on A when the keyboard component has focus. MIDI channels remain independent for note release/sustain, with a fixed +/-2 semitone pitch wheel range on FM, phase and sampler. The string engine's delay-line fundamental is not currently pitch-bent; its tonal overlay can be modulated.

SYNTH/DRUM/TAPE/MIX select modes. T1 selects the engine page, T2 the envelope, T3 effects and T4 LFO. In tape/mixer mode T1–T4 select a track. SEQ reveals the 16-step editor. Shift + a sound slot 1–8 stores the edited sound; synth/drum assignments and favorites persist in project state. The four encoders change function with the selected page:

| Page | Blue | Green | White | Orange |
| --- | --- | --- | --- | --- |
| SYNTH | Tone / sample start | Color / sample length | Envelope macro | Echo mix |
| ENV | Attack | Decay | Sustain | Release |
| LFO | Rate (0.05–20 Hz) | Depth | Pitch / filter | Sine / triangle / square |
| FX | Delay time (50–1000 ms) | Feedback | Echo mix | Drive |
| TAPE | Track 1 gain | Track 2 gain | Track 3 gain | Track 4 gain |

Double-click an encoder to restore its parameter default. Parameters are automatable in a DAW. The five original macros, delay time/feedback and drive use 12 ms smoothing; engine switches and track gains are immediate. The envelope macro scales attack/release in addition to the ENV page's independent ADSR controls. OUTPUT is the master level, affecting live sound and tape playback, and is smoothed over 5 ms.

**Sampler:** choose SAMPLER, then LOAD a WAV/AIFF. Up to the first 16 seconds at 8–192 kHz are imported and stereo files are downmixed to mono. C4/MIDI 60 is the root. START and LENGTH trim the window; LOOP repeats it while the note is held. IN monitors enabled host/standalone audio input; Shift + IN captures up to 16 seconds into the sampler. Configure/unmute the input device first. Empty sampler slots display a load prompt.

**Drums:** C/G pitch classes trigger kick, D/E snare, and other pitch classes hi-hat. Tone, Color and the Envelope macro change pitch/noise/decay. Hits are one-shots. The separate KIT engine includes Studio, Circuit and Dust: three 24-pad kits using the existing original samples, mapped to MIDI 60–83 and common GM aliases.

**Sequencer:** PLAY enables 16th-note playback; in a DAW, also start host transport. Latch Shift to reveal the steps, then click them to toggle gates. Play a note, then **Shift-click** a step to write that pitch. Hover a step to read its note. Host PPQ/tempo drive pattern position, including seeks; otherwise the internal tempo runs the pattern. MIDI sustain (CC64) is supported. STOP disables sequence/tape recording/playback and requests a voice/effect panic.

## Four-track tape and sessions

![Actual tape page](docs/tape.png)

Choose TAPE, select a track and a 4/8/16-second loop. Arm REC, press the tape PLAY button, then play the instrument or sequence. OVER adds to existing material; with OVER off, recording replaces the selected track as the playhead passes. Other tracks keep playing. Turn REC off to listen. In a DAW, host transport must also run.

Playback speed ranges from -2x to +2x, including reverse and freeze. Recording always advances at normal 1x. The left arrow rewinds in tape mode; the right arrow toggles x1/x2 playback; CLEAR immediately erases the selected track. Track audio is recorded before the master OUTPUT gain. Four mono tracks are summed to stereo dual mono. EXPORT writes the selected loop duration as a **four-channel, 24-bit WAV of the raw tracks**; speed/mixer/master settings are not baked into the export.

**SAVE / OPEN** manage complete `.pocketone` sessions. DAW project state also embeds the sample, pattern and all four tape tracks, using 24-bit WAV compressed for storage. Original sample paths are not required. Restoring disarms tape recording. Save/load operations copy and encode audio and can briefly block processing; do them while transport is stopped. Standalone automatic persistence has not been established, so explicitly save before quitting.

This is a constrained loop recorder, not a full recreation of OP-1 tape. It now supports split/join, lift/drop, slide and one-edit undo. It still has no long disk-streamed tape, radio, hardware data takes or stereo panning. It is not compatible with original OP-1 presets/projects. There has been no measured hardware matching. High harmonic settings can alias; production-grade oversampling remains outstanding.

## Windows installer

See [Windows build/install instructions](docs/windows.md). Run `packaging\windows\build.cmd` on Windows with Visual Studio 2022 C++ tools, CMake, Git and NSIS 3.11+ installed. The installer is generated in `dist/`, installs the VST3 bundle to the standard Common Files folder, installs the standalone app/Start menu shortcut, and registers an uninstaller. The C++ runtime is statically linked.

**Windows x64 installer:** `Pocket-One-0.13.0-Windows-x64-Setup.exe`. Seventeen Windows suites and installer/uninstaller checks pass under Wine, including actual VST3 loading and Common Controls manifest inspection. Fifteen Linux suites pass. Native CI for the 0.4.0 SQLite fix also passed Windows/Linux/macOS, including Windows app startup and installer checks; this does not validate a native Ableton project. The installer remains unsigned. See [cross-build instructions](docs/windows-cross-build.md).

## Build from source

CMake 3.22+, a C++17 compiler and Git are required. JUCE is pinned to `51a8a6d7aeae7326956d747737ccf1575e61e209` (8.0.6); use `-DJUCE_PATH=/path/to/JUCE` for an existing checkout. Native Windows builds require MSVC, not MinGW.

Debian/Ubuntu prerequisites:

```sh
sudo apt-get install build-essential cmake pkg-config libasound2-dev \
  libx11-dev libxext-dev libxinerama-dev libxrandr-dev libxcursor-dev \
  libxrender-dev libxfixes-dev libfreetype-dev libfontconfig-dev libgl-dev
```

From the repository root (Windows: use `cmake -S . -B build -A x64` instead of the first command):

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
```

Outputs: `build/PocketOne_artefacts/Release/VST3/Pocket One.vst3` and `Standalone/`. Copy the **entire VST3 bundle** to your plugin directory, rescan, and insert it on an instrument track. Binaries are specific to their build operating system; Linux binaries cannot run in Windows DAWs. For this prepared cloud machine, source `scripts/cloud-env.sh` before building; see [cloud setup](docs/cloud.md).

## Verification and licensing

Thirteen Linux CTest targets (fifteen on Windows) exercise the core engines, tape and plugin, including real audio, MIDI offsets, host seeks, embedded sample/tape state and four-channel export. For dependency-free core tests, configure with `-DPOCKET_BUILD_PLUGIN=OFF`. `engine_tests build/demo.wav` renders an audio demonstration. The smoke executable optionally captures both editor pages under a real/virtual display:

```sh
xvfb-run -a ./build/plugin_smoke_artefacts/Release/plugin_smoke "$PWD/docs/interface.png"
```

See [validation evidence and limitations](docs/validation.md) and [OTTO/TOP-1 design references](docs/references.md). No code or assets from those projects have been imported.

Pocket One's original source is distributed under **AGPL-3.0-only**; see [LICENSE](LICENSE). This open-source distribution uses JUCE's AGPLv3 option and the bundled VST3 SDK's GPLv3 option. Third-party components retain their original terms. See [credits](CREDITS.md) and [third-party notices](THIRD_PARTY_NOTICES.md). Complete corresponding source, including pinned JUCE, is available in `downloads/v0.13.0`.

## Factory sounds

25 original presets, three embedded 24-pad drum kits and four melodic samples. See [the sound bank guide](docs/sound-bank.md) for MIDI mapping, sample credits and OP-1 preset references.

## User banks, animations and realtime audio

**BROWSE > IMPORT BANK** imports a local ZIP bank into a searchable library. Filter bank/type, preset variants, unique samples and favorites. ADD SAMPLES registers multiple WAV/AIFF files. The Loaded label remains separate from the browsing selection. Root notes are supported per sample and selected audio is embedded in DAW state. [User bank guide](docs/user-banks.md). The user-supplied 79-preset collection has a [direct download](downloads/user-banks/README.md); it is not relicensed as the original CC0 factory samples.

The [hardware-style editor and OLED animations](docs/hardware-interface.md) use original JUCE geometry inspired by the supplied OP-1 reference and OTTO-1. [Animated preview](docs/screen-animation.gif).

The processor reports a fixed output-bus latency of `ceil(64 × sample rate / 44100) + 1` samples (about 1.5 ms), including bypass. The realtime test checks tracked heap activity and reports timing across 522 workloads. [Measurements and practical limits](docs/realtime-audio.md). A successful synthetic benchmark does not guarantee dropout-free operation on a particular driver/DAW; load/save/import audio with transport stopped.

See the [complete button/browser guide](docs/buttons-and-browser.md): contextual tape tricks, master EQ/FX pages, input capture, shortcuts and differences from the original firmware.

## Extended sequencing (introduced in 0.4.0)

Use **SEQ SETTINGS / SEQ: …** or **Shift + SEQ** for forward/reverse/pendulum/random patterns, held-note arpeggios, rate, gate, swing, length, velocity and a Euclidean rhythm mask. These independent implementations follow host position/tempo and save with the session. See [sequencer controls](docs/sequencers.md) and the [firmware research ledger](docs/firmware-analysis.md). No original firmware/resources are included.

## 24-pad kits

The **KIT EDIT** panel selects/auditions 24 pads across C4–B5 and edits trim, gain, pitch, reverse and one-shot/gated playback. Load WAV/AIFF per pad; active kits are embedded in projects. Legacy eight-slot banks keep their original note mapping. Factory kits combine the existing Studio/Circuit/Dust resources in three groups of eight; no original OP-1 resources are bundled. [Kit guide](docs/kits.md).
