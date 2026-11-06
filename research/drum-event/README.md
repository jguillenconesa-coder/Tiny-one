# Drum rev246 event words

Numeric contract of complete original note-on `0x01a84ba0`: all signed velocity
halfwords and integer transpositions −128…127. See
[scope and host policies](../../docs/drum246-event-words.md).

```
cmake -S research/drum-event -B build-drum-event -DCMAKE_BUILD_TYPE=Release
cmake --build build-drum-event --config Release --parallel 2
ctest --test-dir build-drum-event -C Release --output-on-failure
```

The runner expands gzip fixtures into a temporary directory. Inputs start with
3072 bytes: eight banks × 24 pads × eight signed little-endian int16 controls.
Then 65536 records of six little-endian uint32 words (24 bytes): selected sound,
signed transpose, note, raw velocity, metadata and payload. The deterministic
Latin permutation crosses each low velocity byte with each transpose once.

Output has 65536 records of 1076 bytes: twelve five-word audio objects (240),
812 global bytes, first retained event row (20), retained count (4). Velocity
halfword is at offset 932; pitch word at 956. No audio render is included.
Fixtures are numeric observations, not firmware. Derived pitch values are not
an original ROM table. Native CI requires no firmware or Blackfin tools.

Private reproduction: `scripts/verify-drum-event246.py` requires `--ldr`,
`--assembler`, `--linker`, `--simulator`, `--testutils` and a new external `--output`
directory. It verifies the firmware hash, both instruction probes, numeric
fixtures and derived header. `--record` deliberately replaces numeric fixtures
and the derived header; independent normal repetition must then match.
