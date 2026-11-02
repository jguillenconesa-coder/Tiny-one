# D-synth complete render/lifecycle comparison fixtures

Synthetic inputs and observations of original instructions under the documented
GNU BF524 Dreg/halfword/FP-load writeback profile. No original executable
instructions, private dump or factory presets. C++ computes the complete render
and post-state independently; no oracle-state handoff in the render test.
The foundation test remains a separate lifecycle/capture characterization.

4096 little-endian records. Input2048 bytes: pool432, context824, controls8*int16,
flags32, slot32, pitch128*uint32, gain128*int16 (full signed range).

Flags:1 reload;16 shared init;2 factory+bind;4 reset;8 release;32 meter decay;
64 capture-counter reset, applied in that order. First2048 independent; remaining
retain render state, with reload every256 calls. All four oscillator configurations
and eight raw mix selectors are exercised. Slots0..5, capture counts0..64.

Output prefix1604 bytes:256 original converted envelope coefficients,
129 converted frequency coefficients and eight4*int16 descriptors. Each record:

| Offset | Bytes | Meaning |
| --- | ---: | --- |
| 0 | 1256 | Pool432 + context824 after events, before render |
| 1256 | 1256 | Complete pool/context after original render |
| 2512 | 256 | 128 int16 audio samples |
| 2768 | 512 | Caller pitch preserved |
| 3280 | 512 | Derived pitch buffer |
| 3792 | 256 | Second oscillator signal |
| 4048 | 256 | Caller gain preserved |
| 4304 | 16 | Pool/context/frequency guards and unchanged RNG |

Context: constants/padding8, meters24, six64*int16 histories768, counters24.
State72: tag0; envelope words4/8/12/16; slot20; selection16 at24; padding26;
oscillators{phase32,sample16,padding16}28/36; seven filter words44..68.

The old corpus at commit2a33075 used the earlier simulator and nonnegative gain.
Current inputs cover signed gain and outputs use corrected FP-load source latching.
Metadata: docs/firmware/dsynth246-render-validation.json. Value/Random fixtures
retain their existing formats and hashes in Tests/ReferenceVectors.
