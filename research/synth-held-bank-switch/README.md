# Synth held-bank changer 246

Original complete synth bank changer with supplied retained-note registry,
Dr. Wave graph, initialized six-envelope pool and absent auxiliary modules.
Both L1 completion endpoints are authored; original MIDI helpers run with output
disabled. [Scope/findings](../../docs/synth246-held-bank-switch.md).

```sh
cmake -S research/synth-held-bank-switch -B build-synth-held-bank-switch -DCMAKE_BUILD_TYPE=Release
cmake --build build-synth-held-bank-switch --config Release --parallel 2
ctest --test-dir build-synth-held-bank-switch -C Release --output-on-failure
```

The Python runner expands numeric gzip fixtures into a temporary directory.
Native replay requires no firmware. For original extraction use
`scripts/verify-synth-held-bank-switch246.py` with `--ldr`, `--assembler`,
`--linker`, `--simulator`, `--testutils` and a fresh external private `--output`.
It verifies the supported LDR hash and both patched Blackfin writeback probes.
`--record` intentionally replaces fixtures. Keep images, ELF and firmware private.

2048 independent records, each with 4164 input bytes: two little-endian u32 words
(target bank and authored delay 1–4), followed by 4156 window bytes. All windows
reload every case. Addresses/order/lengths are in the
[evidence JSON](../../docs/firmware/synth-held-bank-switch246-validation.json).
The held count is observed twice through overlapping windows; both input copies
agree. Pool states, audio/aux pointers, common/Core controls, note table,
tuning index/coefficient, RNG, MIDI bitmap, output-enable flag, selection and
translation state are supplied explicitly. No constructor or setter is executed.
The note table comes from the earlier original activation numeric fixture header.

Output is the 4156 window bytes, four 24-byte authored trace slots and u32 count
(4256 bytes total). Used slots hold endpoint 0(common)/1 (special), R0/R1/R2,
active bank and selected bank. Unused slots retain 0x5a. The original changer,
MIDI helpers, Core note callbacks and pitch refresh are unmodified. Only two L1
entry trampolines are authored; they set active=selected on the final call,
without factories. Delay and budget are harness choices, not recovered timing.

Supplied type 2, common selectors 0–4, lengths 0–128, notes in 0–127 before/after
translation, voice indices −2/−1/0…5 and adjustment words −4…4 are covered.
Velocity 0 and 0x10000 distinguish full-word MIDI semantics from low-half voice
velocity conversion. Duplicate notes and noncanonical active bytes exercise
ordered bitmap changes. Out-of-range note removal, MIDI transport, type 4 branch,
auxiliary graph, lazy initialization, rendering and host/plugin behavior are excluded.
Native bool returns and profile guards are own APIs, not original error semantics.
