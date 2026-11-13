# Synth bank changer 246: empty held registry

This standalone model compares the complete original changer at
`0x01a86420..0x01a86660` in synth UI context0. It includes six envelope resets,
translation initialization, deferred transpose arithmetic and original selection
synchronization. Both L1 endpoints are authored completion services; neither
constructs a voice or models the protected scheduler.

```sh
cmake -S research/synth-bank-switch -B build-synth-bank-switch -DCMAKE_BUILD_TYPE=Release
cmake --build build-synth-bank-switch --config Release --parallel 2
ctest --test-dir build-synth-bank-switch -C Release --output-on-failure
```

Python decompresses numeric fixtures into a temporary directory. Native tests
compare every output byte; firmware is not needed for replay. For a new original
extraction run `scripts/verify-synth-bank-switch246.py` with `--ldr`, `--assembler`,
`--linker`, `--simulator`, `--testutils` and a new external private `--output` path.
The patched Blackfin toolchain and both writeback probes are required. `--record`
intentionally replaces the numeric fixtures. Keep generated images, ELF and
firmware private.

2048 records, each 484 input bytes: target and own delay as two little-endian u32,
then 476 window bytes. Every case reloads all observed windows. Window addresses
and lengths are listed in the [evidence](../../docs/firmware/synth-bank-switch246-validation.json).
The caller supplies empty held count, context0, a nonzero envelope-ready flag,
old active/editor bank, bank transpose table, initialization/deferred flags,
marker/guards, six live envelope states and controls. Instrument mode4 is excluded.

Output: 476 window bytes, four 24-byte own trace slots, then one u32 call count
(576 total). A used slot holds endpoint0(common)/1(special), R0,R1,R2, active bank,
selected bank. Unused slots retain 0x5a. The own endpoint completes after1–4 calls
by copying editor synth bank into active bank; no factory, note event or L1
payload is executed by that endpoint. The original changer and its readable
callees are not patched. The harness budget and native bool/guards are own APIs.

All64 old/target pairs, exact flag1 vs0/2/-1, same-bank nonzero-marker preservation,
fresh/existing translation state, zero/nonzero signed bank deltas and modulo32
pending accumulator boundaries are included. No retained-note translation,
reactivation with notes, mode4 branch, PCM render, product integration or acoustic
comparison. See [findings and scope](../../docs/synth246-bank-switch.md).
