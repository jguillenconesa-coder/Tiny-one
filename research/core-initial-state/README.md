# Core rev246 initial state and drum event composition

Research-only, instrument remains 0.54.0. See the
[contract, observed windows and limits](../../docs/core246-initial-state.md).

```
cmake -S research/core-initial-state -B build-core-initial-state -DCMAKE_BUILD_TYPE=Release
cmake --build build-core-initial-state --config Release --parallel 2
ctest --test-dir build-core-initial-state -C Release --output-on-failure
```

All words are little endian. Input starts with 3072 bytes: eight sounds × 24 pads
× eight int16 controls (same supplied bank format as the drum event corpus).
Then 256 records of 5880 bytes: a 5856-byte snapshot plus six uint32 event words
(selected sound, signed transpose, note, raw velocity, metadata, payload).

Snapshot concatenates four windows: `0xff800ff8` / 2024 bytes,
`0x01aaabd4` / 3560, `0x01bab538` / 16, `0xffb00000` / 256.
Output record: snapshot after full initializer (5856), snapshot after shared
workspace initialization + explicit controls/preconditions + factory/note-on
(5856), retained row (20), retained count (4), total 11736 bytes. No audio.

The generated header contains 185 derived write spans for the first snapshot,
not an original ROM table or general soft-float implementation. Native tests
compare every byte and compose existing workspace, Core and event models for
the second snapshot. This does not validate original scheduling or gain/pitch
buffer production, and is not linked into the plugin.

Private reproduction: `scripts/verify-core-initial-state246.py` takes `--ldr`,
`--assembler`, `--linker`, `--simulator`, `--testutils`, and a new external
`--output` directory. It checks the firmware hash, two instruction probes,
initializer pointer entries, fixtures and derived header. `--record` deliberately
replaces fixtures/header; an independent ordinary repetition must match.
No firmware, machine code, ELF or decompiler output is distributed here.
