# Core rev246 selection synchronization, explicit L1 responses

Research only; [contract and limits](../../docs/core246-selection-sync.md).

```sh
cmake -S research/core-selection-sync -B build-core-selection-sync -DCMAKE_BUILD_TYPE=Release
cmake --build build-core-selection-sync --config Release --parallel 2
ctest --test-dir build-core-selection-sync -C Release --output-on-failure
```

Frozen reproduction needs no firmware/emulator. All words are little endian.
4096 input records, 308 bytes each:

- Eight uint32: route (synth 0/drum 1), planned callback count 0..6, flag value,
  actual synth/drum banks, selected synth/drum banks, ambient R2 seed.
- 132 seed bytes: 68 at 0x01aaac0c, 32 at 0xff8016cc, 32 at 0xff801038.
- Six scheduled responses, each six uint32: new actual synth/drum, new selected
  synth/drum, new flag, new ambient R2. Only requested responses are consumed.

Caller seeds the windows and writes metadata fields to their observed addresses.
Each original synchronizer is invoked twice: first to converge under the explicit
schedule, then again with banks already equal. The second call must bypass both
endpoints. No render or factory is part of these authored responses.

Output record, 392 bytes:

- Final windows in the same order: 132 bytes.
- Six authored trace slots of 40 bytes: endpoint ID, incoming R0/R1/R2, actual
  synth/drum, selected synth/drum, flag, then one poison guard word. Unused slots
  remain 0x5A throughout.
- Callback count: 4 bytes.
- R0/R2 observed after first and second outer calls: 16 bytes. These are register
  diagnostics, not a recovered public return ABI.

Flag values include 0, 1, 2, 0xFFFFFFFF, 0x80000000. Some schedules retain selection
and flag, others retarget or change flag between calls. Responses clobber R0/R1
and supply R2 so reloading can be observed. Completion is deliberately ensured
within six callbacks; this is not an original timeout or hardware timing model.

Private reproduction: `scripts/verify-core-selection-sync246.py` requires `--ldr`,
`--assembler`, `--linker`, `--simulator`, `--testutils`, and a new external
`--output`. It checks firmware hash and both instruction probes. `--record`
intentionally replaces numeric fixtures; ordinary repetition must match them.

The original outer routines remain unchanged. The ELF supplies authored
trampolines at 0xffa0bd2c and 0xffa017ec, pointing to the scheduled-response
handler. The simulator explicitly maps only 32 bytes for each trampoline in L1;
**no original L1 image is loaded, decrypted or executed**. The linker script is
`research/blackfin/core-selection-sync.ld`. Handler traces and mutation schedules
belong to the harness, not to recovered firmware data structures or semantics.
A harness budget aborts invalid schedules; the native budget guard is likewise
an authored extension. Nonconverging original execution is not tested.
No firmware, ELF, original instruction bytes or decompilation are distributed.
