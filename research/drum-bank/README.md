# Drum bank / held registry lifecycle, rev246

Since 0.53.0 the selection helper is shared with `Source/Drum246Allocator.h`
and integrated through an explicitly authored host adapter. The remaining
models in this research directory are not linked into the instrument; version
0.52.0 references below describe the initial study. See
[the integration scope](../../docs/drum246-voice-allocation.md).

Standalone research, not linked into the instrument (still 0.52.0). Reconstructs
registry clearing, held-note reactivation, bank begin/factory finish and the
occupied-voice query. See [contract](../../docs/drum246-bank-lifecycle.md) and
[evidence](../../docs/firmware/drum-bank246-validation.json).

```sh
cmake -S research/drum-bank -B /tmp/drum-bank -DCMAKE_BUILD_TYPE=Release
cmake --build /tmp/drum-bank --config Release
ctest --test-dir /tmp/drum-bank -C Release --output-on-failure
```

The Python 3 runner decompresses numeric fixtures to a temporary directory, then
runs the C++17 verifier. The model composes the prior Core/VoiceCtl research
models and the existing sampler callback. No original firmware/ELF/disassembly
or decompiler output is committed.

## Private reproduction

```sh
python3 scripts/verify-drum-bank246.py \
  --ldr /private/OP1_vdk.ldr \
  --assembler /tools/bin/bfin-elf-as \
  --linker /tools/bin/bfin-elf-ld \
  --simulator /tools/bin/run-fp \
  --testutils /tools/src/sim/testsuite/bfin \
  --output /private/new-drum-bank-run
```

Output must be new and outside the checkout. Both simulator probes require all
three [writeback fixes](../blackfin/README.md). `--record` replaces numeric
fixtures explicitly; ordinary runs compare uncompressed input/output bytes.
Input generation imports the existing voice/core generators, with no private
inputs beyond the supplied LDR. The complete original routines run unpatched.

## Fixture format

Little endian. Prefix and 1,156-byte input record retain
[the VoiceCtl format](../drum-voice/README.md). New flags: reset held voices=64,
clear registry=128, begin bank=256, factory finish=512, query busy=1024.
Bank finish uses the selected sound, not the initial factory sound.

A supplied scenario reset initializes phase word `0xa5a5a500`, external note
flags at `0x01aafd60[128]` to byte 127, and MIDI output word `0x01dfe03c` to zero.
It also supplies the prior zeroed globals, type-1 drum mode, empty held count
with `0x5a` row padding, type-8 sampler factory and explicit state preload.
Those values are test inputs, not inferred original defaults.

Output prefix is unchanged (2,052 bytes FIR conversion/padding). Each output
record contains the prior 5,156 bytes (query, states, globals, held backing
storage/count, audio, pitch, gain, derived gain), then the four-byte phase word,
busy-query word and 128 external-note bytes: **5,292 bytes** total. A skipped busy
query is poisoned with `0x2468ace0`; this fixture requests it in every case.
Suffix is the unchanged 16,896-byte PCM prefix.

First 4,096 records independently supply every twelve-bit occupied mask; set
bytes vary over 1/127/128/255. Next 4,096 records contain 64 retained sequences
of 64 events, with two bank transitions, four held resets and two isolated
registry clears per sequence. The isolated clear records skip on/off, activity
and render: the original twelve-object pool must match the preceding record.

All five new routines, note-on/off, the activity query and optional render are
original instructions. MIDI output is disabled: the original external-note helper
may clear supplied flags, but transmitter/IPC is not exercised. Null LFO/FX,
valid indices and phase order, five octave transpositions, explicit PCM/padding,
pitch/gain and cadence define the domain. These callbacks do not recover normal
per-block retirement, gain/pitch producers, full bank loading or the scheduler.
