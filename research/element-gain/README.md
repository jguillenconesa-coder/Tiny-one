# Element voice gain verifier

C++17, CMake 3.22, Python 3.11; native suites require neither JUCE nor firmware.
See ../../docs/element246-voice-gain.md for reproduction and limits.

8192 original calls, 444 input / 1204 output bytes; two identical independent
numeric extractions. 1540 processed (385 Level calls return zero), 6652 rejected.
197120 gain words checked also through the host bridge for all four sources.
Complete return, controls/targets and source/gain/pitch/audio buffers compared.
External uses 64 signed samples driving 128 gains, each input twice. Pitch excluded.

Integration: 364 rate/engine/source cases, seven rates, ten melodic engines and
drum/legacy rejection, exact 64/257 audio, Mono/Legato/glide, signed amounts,
causal external ring rotation/capture, owner changes, reset and non-finite guards.
SRC, historical window, voice clocks/lifecycle and buffer placement are authored.
No recovered original producer/scheduler or acoustic-equivalence claim.
