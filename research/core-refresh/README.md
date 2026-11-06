# Complete Core rev246 refresh: explicit sampler/Delay profile

Research only. See [contract and limits](../../docs/core246-refresh.md).

```sh
cmake -S research/core-refresh -B build-core-refresh -DCMAKE_BUILD_TYPE=Release
cmake --build build-core-refresh --config Release --parallel 2
ctest --test-dir build-core-refresh -C Release --output-on-failure
```

Frozen numeric reproduction needs no firmware or Blackfin tools. All words are
little endian; supported native CI targets are little endian. The model reuses
Drum246State, Sampler246State, Delay246State and CoreModulatorFactory246.

4160 input records, each eight uint32 metadata words followed by these windows:

| Start | Bytes | Output offset |
| --- | ---: | ---: |
| 0x01aaac0c | 68 | 0 |
| 0x01aaad18 | 156 | 68 |
| 0xffb00000 | 256 | 224 |
| 0xffb005f8 | 48 | 480 |
| 0xffb009f8 | 588 | 528 |
| 0xff907aa8 | 48 | 1116 |
| 0xff801374 | 1024 | 1164 |
| 0x01cc10cc | 32 | 2188 |
| 0x01df6f78 | 32 | 2220 |
| 0x01dfe734 | 2564 | 2252 |
| 0xff801038 | 812 | 4816 |

Metadata: flags (reload=1, refresh=2), drum/synth modulator IDs, actual Core
drum/synth banks, editor drum/synth banks, row index. Input 5660 bytes, output
5628 bytes (windows only). 3168 reloads / 992 retained rows; 3956 complete
refreshes / 204 skipped calls. First 3136 rows cover 7×7×8×8 actual modulator/bank
combinations; later rows exercise retention and omission.

Before every optional refresh, the authored caller supplies Core/editor banks,
audio type arrays (synth 6, drum 8), FX arrays (both 0), selected modulator IDs,
and eight explicit nonzero opaque references per audio route. References need
no allocated PCM here: the original constructors bind them without rendering.
These caller writes happen even on skipped calls and are modeled separately.
Every fourth row has matching editor/Core banks; others deliberately differ.
Other bytes in the supplied windows remain untouched by caller setup.

Private reproduction: `scripts/verify-core-refresh246.py` takes `--ldr`,
`--assembler`, `--linker`, `--simulator`, `--testutils`, and a fresh external
`--output`. It verifies firmware hash and both simulator probes. `--record`
intentionally replaces numeric fixtures; ordinary repetition must match them.
The complete original routine 0x01967b28 and dependencies execute without patches,
spies or replacement callbacks. The profile is deliberately bounded; invalid
original selector/error branches are not executed.

`research/blackfin/core-refresh.ld` places inputs at 0x06000000, separate from
SDRAM at 0x01000000 and results at 0x03000000, below the explicit stack near
0x07f00000 in the 128 MiB simulator. The input corpus would overlap the original
SDRAM image if put into the ordinary 0x10000 data section.
Firmware, ELF, original instructions and decompilation remain private.
