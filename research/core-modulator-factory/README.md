# Core rev246 modulator factories: six/twelve voices

Research-only types 0 (Tremolo), 1 (Value), 2 (Random). No plugin changes.
See [contract and limits](../../docs/core246-modulator-factory.md).

```sh
cmake -S research/core-modulator-factory -B build-core-modulator-factory -DCMAKE_BUILD_TYPE=Release
cmake --build build-core-modulator-factory --config Release --parallel 2
ctest --test-dir build-core-modulator-factory -C Release --output-on-failure
```

3072 little-endian records. Each input has five uint32 metadata words:
flags (reload=1, factory=2, clear=4, reset=8), route (synth=0/drum=1), type,
voice and sound. Then these windows, also used as the output record:

| Address | Bytes | Offset in output |
| --- | ---: | ---: |
| 0x01aaac0c | 68 | 0 |
| 0x01aaad60 | 84 | 68 |
| 0xffb009f8 | 588 | 152 |
| 0xff8014c4 | 128 | 740 |
| 0xff801644 | 128 | 868 |
| 0x01dd5d04 | 32 | 996 |
| 0x01dd5a88 | 32 | 1028 |
| 0x01dd5c6c | 4 | 1060 |
| 0xff802bc8 | 4 | 1064 |

Input 1088 bytes; output 1068. Flags retain all windows when reload is unset.
1584 reloads / 1488 retained rows, including 12 family changes and four route
changes without reloading the windows. The first 768 rows request factory and reset
with an enabled noncanonical byte, covering all 432 route/type/sound/voice
combinations. Later rows include omitted factory/reset, clear and disabled gates.
The harness writes the selected route's sound each row, and sets its enable byte
to 0xA5 unless sound zero after row 767, where it writes zero. These are explicit
caller choices; factory does not set selection or enable flags itself.

Reload seeds zero both secondary pointers and all primary pointers so reset
without factory stays valid. Primary pool patterns include zero/FF/A5 and
variable bytes. Constructors preserve padding and unused capacity; synth/drum
factories share physical storage. Selected-type changes are not concurrent pools.

The native comparator reuses the original-reset reconstruction and compares every
window byte. Its guard for types outside 0..2 is authored; IDs 3..6 exist in the
original selector but are outside this model's scope. No invalid original index
or logging/error path is executed.

Private reproduction: `scripts/verify-core-modulator-factory246.py` requires
`--ldr`, `--assembler`, `--linker`, `--simulator`, `--testutils`, and a fresh
external `--output`. Original factories, selectors, constructors, vector helpers,
clear and reset dispatchers run without replacement endpoints. Firmware hash and
both simulator probes are required. `--record` intentionally replaces numeric
fixtures; ordinary repetition must match them. No original code or ELF is shipped.
