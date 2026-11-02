# Voltage numerical fixtures

Synthetic data and observations of isolated rev246 BF524 instructions. No original
executable instructions, preset bank or private memory dump is included.

4096 input records, 840 bytes each, little endian:

| Offset | Bytes | Meaning |
| --- | ---: | --- |
| 0 | 312 | Six synthetic states, 52 bytes each |
| 312 | 8 | Four signed16 controls, 0–32767 |
| 320 | 4 | Flags: 1 reload pool, 8 factory, 2 reset, 4 release, in that order |
| 324 | 4 | Selected slot, 0–5 |
| 328 | 512 | 128 unsigned32 pitch words |

Output prefix 324 bytes: 129 converted signed16 coefficients, 64 bytes of original
type13 descriptors, two zero alignment bytes. Then 4096 records of 1472 bytes:

| Offset | Bytes | Meaning |
| --- | ---: | --- |
| 0 | 312 | Pool after selected lifecycle actions, before render |
| 312 | 312 | Pool after render |
| 624 | 256 | 128 signed16 audio samples |
| 880 | 64 | 32 signed16 display values |
| 944 | 512 | Pitch buffer preserved |
| 1456 | 16 | Pool/pitch/display guards and unchanged RNG |

State layout: tag at0; four {phase32,sample16,padding16} at4/12/20/28; filter32
at36, charge16 at40, previous16 at42, detune32 at44, release byte48, padding49–51.
First2048 cases reload independent pools; remaining cases retain state with a
reload every256 calls. Lifecycle cadence and controls/pitch edge cases are defined
by `scripts/verify-voltage246.py`. Value/Random fixtures live in
`Tests/ReferenceVectors` with the existing modulation record formats.

Hashes/provenance: `docs/firmware/voltage246-validation.json`. Simulator comparison
is conditioned on supplied pitch/events/tables; hardware acoustics are unmeasured.
