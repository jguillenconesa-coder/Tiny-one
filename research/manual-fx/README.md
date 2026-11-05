# Bend/Crank FX audio verifier

Independent C++17 verifier and host integration tests; no JUCE or firmware needed.
See ../../docs/manual246-fx-input.md for the contract and reproduction commands.

8192 calls per source, kinds 2/5, destination 3, colours 2/3 plus rejection.
Bend records: 96 input / 1108 output bytes. Crank: 128 / 1140 bytes.
3493 accepted calls per source; 447104 affected halfwords across both sources.
Complete source state, controls, targets, return, pitch/gain/audio checked.
The remaining 64 audio halfwords stay unchanged. Two independent original runs
have identical numeric input/output. These are supplied states and buffers,
not a recovered scheduler or hardware acoustic-equivalence measurement.

Host tests exercise seven rates, seven mono FX, both sources and both curves,
bypass, signed amounts, exact partitioned audio, PDC, reset and silence.
