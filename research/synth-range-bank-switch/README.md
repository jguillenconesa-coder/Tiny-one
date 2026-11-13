# Synth range/removal bank changer 246

Original complete synth changer with out-of-range note removal, Dr. Wave
reactivation, MIDI event producers and original busy-pump ring enqueue.
[Scope and limits](../../docs/synth246-range-bank-switch.md).

```sh
cmake -S research/synth-range-bank-switch -B build-synth-range-bank-switch -DCMAKE_BUILD_TYPE=Release
cmake --build build-synth-range-bank-switch --config Release --parallel 2
ctest --test-dir build-synth-range-bank-switch -C Release --output-on-failure
```

The runner expands gzip numeric fixtures to a temporary directory. No firmware
is required for native replay. Original extraction uses
`scripts/verify-synth-range-bank-switch246.py` with `--ldr`, `--assembler`,
`--linker`, `--simulator`, `--testutils` and a new private external `--output`.
The supported firmware hash and both patched Blackfin writeback probes are checked.
`--record` replaces fixtures intentionally. Firmware, ELF, SDRAM and generated
private assembly stay outside the repository. The own input section is at 0x06000000
because expanded inputs exceed the space below the original SDRAM mapping.

2048 independent records: 8332 input bytes (u32 target, u32 authored delay 1–4,
then 8324 window bytes), 8424 output bytes (windows, 96-byte own trace, u32 count).
Window order/addresses/sizes are in the
[evidence JSON](../../docs/firmware/synth-range-bank-switch246-validation.json).
The first 4156 bytes retain the previous held-bank layout. Appended windows:

| Offset | Address | Bytes | Supplied state |
| --- | --- | ---: | --- |
| 4156 |0x01dfe730|4| Release gate 0 and guards |
| 4160 |0x01aadb20|40| MIDI context/channel and event buffers |
| 4200 |0x018b600c|4| Pending byte+1 and guards |
| 4204 |0x01ab1b38|4120| Guard, read counter, busy count,1024-word ring, write counter, gate and guard |

The generator imports prior held-bank numeric input generation, then supplies
boundary notes, nonnegative sustains, old selectors 0–3 for effective changes,
MIDI enable 0/1, transport gate 0/1/165, busy count 1/2/165 and valid ring occupancy.
All windows reload per record. Initial notes are 0–127; translated notes may be
invalid. The callback composition reuses original-conformant release adapters,
including field-wise compaction/preserved padding and return-to-previous-note RNG.

The old public adapter retains its output-disabled/no-removal contract. The new
callback API and count-reduction check are own adapter policy. Both L1 endpoints
remain authored completion trampolines without factories. Event helpers and
ring enqueue execute original code; busy count prevents original DMA pump entry.
No hardware/USB/host transport, concurrent consumer, render or plugin change.
Native bool/error guards and finite service budgets are not original return codes.
