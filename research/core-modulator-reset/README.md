# Core rev246 modulator reset routing

Research only, not linked into the 0.54.0 plugin. See
[contract, five reset profiles and limits](../../docs/core246-modulator-reset.md).

```
cmake -S research/core-modulator-reset -B build-core-modulator-reset -DCMAKE_BUILD_TYPE=Release
cmake --build build-core-modulator-reset --config Release --parallel 2
ctest --test-dir build-core-modulator-reset -C Release --output-on-failure
```

Input: 8192 little-endian 740-byte records. Nine uint32 metadata words: route
(0 six voices, 1 twelve), voice, sound, primary/secondary reset profiles (0 absent,
1 Value synth, 2 Value drum, 3 Random synth, 4 Random drum, 5 Tremolo), signed
primary/secondary this deltas, action bits (bit0 clear, bit1 synthetic selector
mutation in the primary endpoint), new sound. Then 704 bytes of observed windows:

- Core 0x01aaac0c / 68 bytes; tables 0x01aaad60 / 84.
- Primary proxy 0x02010000 / 128; secondary proxy 0x02010080 / 128.
- Tremolo display 0x01dd5a88 / 32.
- Synth controls 0xff8014c4 / 128; drum controls 0xff801644 / 128.
- RNG and neighbor 0xff802bc8 / 8.

Harness installs proxy vptrs at +64 and prepares the selected primary/secondary
pointers. These writes are caller setup, not firmware behavior. Adjusted receivers
remain within the full observed proxy windows. Original Core dispatch uses real
signed this adjustments; authored endpoints trace arguments then tail-call the
five complete original resets. No factory/type-selection or full note-on claim.

Output: 8192 records, 732 bytes each: the same 704 bytes after execution, then
seven uint32 trace words: count, primary receiver/controls/selection, secondary
receiver/controls/selection. Both reset bodies may update the shared display.
All bytes, including inactive states, padding, other route and guards, compare.

Private reproduction: `scripts/verify-core-modulator-reset246.py` requires
`--ldr`, `--assembler`, `--linker`, `--simulator`, `--testutils`, and a new external
`--output` directory. Firmware hash and both instruction probes are checked.
`--record` replaces numeric fixtures; independent ordinary repetition must match.
Only authored harness/models and numeric observations are public. No audio,
firmware, ELF, original instruction stream or decompilation is distributed.
