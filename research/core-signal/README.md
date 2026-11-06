# Core rev246 64-word signal callback and bounded reference inventory

Research only; no plugin/routing/sample-rate claim. See
[contract, formula and limits](../../docs/core246-signal-kernel.md).

```
cmake -S research/core-signal -B build-core-signal -DCMAKE_BUILD_TYPE=Release
cmake --build build-core-signal --config Release --parallel 2
ctest --test-dir build-core-signal -C Release --output-on-failure
```

Two suites: numeric comparison of 4864 complete original callbacks and sixteen
synthetic reference-decoder cases (no firmware). Inputs/outputs are gzip-packed
numeric observations. All words are little endian. No prefix; 4864 records:

Input, 332 bytes: mode uint32 (1 independent, 0 retained, 2 Core initializer),
coefficient window 12 bytes at 0x01bab538, history window 28 at 0x01aaac0c,
input window 144 at 0xff801e48 and output window 144 at 0xff800bac.
Coefficient is at +4, three history words at +8, 64 input/output int16 at +8.
Other bytes are guards. Retained mode preserves the previous history window;
coefficient and I/O windows are supplied each time. Mode 2 loads all windows,
executes original Core initializer, then executes the original signal callback.
Output, 328 bytes: coefficient, history, input and output windows in that order.

4096 independent + 512 retained + 256 initializer-composed blocks = 311296
numeric output samples. Complete state, buffers and guards are compared.
Core initializer writes reuse its separately derived bounded model; signal
arithmetic is independently reconstructed. Caller order remains authored.

Private reproduction: `scripts/verify-core-signal246.py` requires `--ldr`,
`--assembler`, `--linker`, `--simulator`, `--testutils`, and a new external
`--output` directory. `--record` replaces numeric fixtures; independent ordinary
repetition must match. Firmware hash and both instruction probes are checked.

`scripts/audit-drum246-render-references.py --ldr ... --output ...` publishes
address metadata only. It scans only known SDRAM bytes and the visible bytes of
the one inventoried L1 block under its stated protection assumption. No L1
execution, decryption, whole-caller analysis or producer recovery is claimed.
Firmware, ELF, original instruction bytes and decompilation stay private.

The expanded inventory now includes 22 targets and separately records a relative
call immediately after a full pair. That callee is not assumed to be the loaded
address. See [buffer initialization](../../docs/core246-buffer-initialization.md).
