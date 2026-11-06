# Core rev246 shared-buffer initializer

Research only; see [contract and limits](../../docs/core246-buffer-initialization.md).

```sh
cmake -S research/core-buffer-init -B build-core-buffer-init -DCMAKE_BUILD_TYPE=Release
cmake --build build-core-buffer-init --config Release --parallel 2
ctest --test-dir build-core-buffer-init -C Release --output-on-failure
```

No firmware or Blackfin toolchain is needed for frozen numeric reproduction.
All fixture words are little endian; native CI targets are little endian.

1024 records. Input flags uint32 followed by these windows, in order:

| Start | Bytes | Native offset (after flags) |
| --- | ---: | ---: |
| 0x01df4dec | 24 | 0 |
| 0xff801948 | 1552 | 24 |
| 0x01aaac0c | 28 | 1576 |
| 0x01bab538 | 12 | 1604 |
| 0xff800bac | 144 | 1616 |

Input record 1764 bytes, output record 1760 bytes (windows only).
Flags: bit0 reload all windows, bit1 invoke initializer 0x01a3b34c,
bit2 invoke Core signal callback 0x019674f8 after initialization. Unset reload
retains all five windows; unused input seeds are deliberately ignored.
528 reloads, 496 retained rows; all four init/kernel combinations occur.
512 initializations, 512 kernel calls, 256 composed init→kernel rows.

Initial input windows include zero, FF, A5 and deterministic variable seeds,
with signed extreme and ordinary Q31 coefficients. Initializer modifies six
bytes inside the first window and clears 1536 bytes inside the second; guards,
Core signal history/coefficient and output are preserved by that call.
The optional kernel subsequently updates its own three history words and
64 output halfwords, consuming the first 64 of the 128 input halfwords.
Comparisons include every window byte, not only expected changed fields.

Private reproduction: `scripts/verify-core-buffer-init246.py` requires `--ldr`,
`--assembler`, `--linker`, `--simulator`, `--testutils`, and a fresh external
`--output`. It checks firmware hash, startup pointer and both simulator probes;
`--record` intentionally replaces numeric fixtures. Ordinary reproduction must
match the existing fixtures. Original code and helpers execute without patches
or authored callback replacements. No original instruction bytes, ELF, firmware
or decompilation are distributed.
