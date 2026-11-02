# D-synth original observations / lifecycle reconstruction fixtures

These are synthetic inputs and numeric observations, not a reconstructed audio
engine or proprietary instructions. Four thousand ninety-six records, little
endian. Input2048 bytes: pool432, shared context824, controls8*int16, flags32,
slot32, pitch128*uint32, gain128*int16.

Flags: 1 reload pool/context; 16 shared init; 2 factory then explicit bind;
4 voice reset; 8 release; 32 meter decay; 64 capture-counter reset. Apply in that
order. First2048 independent; remaining retain original render results with
reload every256. Pitch covers full32 bits and changes within blocks; controls
are0..32767, state slots0..5 and capture counters0..64.

Output prefix1604 bytes:256 original converted envelope coefficients,129
converted frequency coefficients, eight4*int16 descriptors. Then4096 records:

| Offset | Bytes | Meaning |
| --- | ---: | --- |
| 0 | 1256 | Pool432 + context824 after lifecycle, before render |
| 1256 | 1256 | Original renderer's resulting pool/context |
| 2512 | 256 | Original128 int16 audio samples |
| 2768 | 512 | Caller pitch preserved |
| 3280 | 512 | Original derived pitch buffer |
| 3792 | 256 | Original second oscillator signal |
| 4048 | 256 | Caller gain preserved |
| 4304 | 16 | Pool/context/frequency guards and unchanged RNG |

Context: constants8, six2*int16 meters24, six64*int16 histories768 and counters24.
State72: tag0, envelope words4/8/12/16, slot20, selection16 at24, padding26,
oscillators{phase32,sample16,padding16}28/36 and seven filter words44..68.

C++ lifecycle tests use the original post-render state as explicit input for the
next call. They do not reconstruct the audio. Modulation's4096+4096 fixtures use
the existing Value/Random formats in Tests/ReferenceVectors. Provenance and hashes:
`docs/firmware/dsynth246-foundation-validation.json`.
