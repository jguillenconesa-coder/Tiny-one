# Core drum sampler routing, OP-1 rev246

Research-only bounded reconstruction of type 8's twelve audio producers, factory,
reset and release routing. The instrument remains at 0.52.0; this directory is
not linked into the plugin. These audio producers are not the missing producer
of drum dynamics. See [contract and limitations](../../docs/drum246-core-routing.md).

```sh
cmake -S research/drum-core -B /tmp/drum-core -DCMAKE_BUILD_TYPE=Release
cmake --build /tmp/drum-core --config Release
ctest --test-dir /tmp/drum-core -C Release --output-on-failure
```

One suite decompresses the numeric fixtures into a temporary directory and
compares every byte of all 4,608 records against the C++ model. No firmware,
disassembly or decompiler output is included. Existing `Source/Drum246.h`
provides the separately verified callback. Python 3 and a C++17 compiler suffice.

## Reproduction with privately supplied firmware

```sh
python3 scripts/verify-drum-core246.py \
  --ldr /private/OP1_vdk.ldr \
  --assembler /tools/bin/bfin-elf-as \
  --linker /tools/bin/bfin-elf-ld \
  --simulator /tools/bin/run-fp \
  --testutils /tools/src/sim/testsuite/bfin \
  --output /private/new-drum-core-run
```

The output directory must be new and outside the checkout. The script checks
the firmware hash and both simulator probes for all three documented writeback
fixes before executing original instructions. It checks decompressed numeric
output against the fixtures; `--record` explicitly replaces fixtures for review.
Use the Blackfin patches documented in [the simulator setup](../blackfin/README.md).

## Binary fixture format

All integer fields are little endian. Input prefix: eight PCM banks, each 1,024
signed halfwords with 16 zero halfwords on either side (16,896 bytes total), then
eight sounds × 24 pads × eight signed control halfwords (3,072 bytes).

Input record (1,096 bytes): flags, voice, selected sound, factory sound, signed
reset/release this-adjustments stored in 32-bit words (24 bytes); twelve pad
indices (48); twelve 20-byte state records (240); updated selected controls (16);
128 unsigned pitch words (512); 128 signed gain halfwords (256). Flags: load
pool=1, factory=2, reset=4, release=8, render=16. Unloaded state and control rows
persist between records. Negative adjustments use their low signed halfword.

Output prefix: 1,025 FIR conversion halfwords plus two padding bytes (2,052).
Output record (5,156 bytes): trace `reset_this, reset_controls, release_this,
release_controls, callmask` (20); twelve producer pointers (48); pool before
render (240); pool after render (240); audio (256); pitch (512); gain (256);
derived gain words (512); all control banks (3,072). Missing trace endpoints stay
`0x13579bdf`; mask bits 1/2 indicate reset/release. Skipped audio keeps alternating
`0x1357/0x2468` halfwords; derived gain starts `0x6c6c6c6c` and persists. The
output suffix is the complete unchanged PCM prefix (16,896 bytes).

Authored trace endpoints test original virtual dispatch with synthetic signed
this-adjustments. The same original Core methods then call real original sampler
reset/release with the actual vtable's zero adjustments. The callback receives
explicit padded PCM, pitch, gain and controls. The test does not establish the
original scheduler, MIDI allocator, dynamic envelope, D-box route or acoustic
identity. Own invalid-index guards are checked separately; original invalid
inputs assert and are deliberately not exercised.
