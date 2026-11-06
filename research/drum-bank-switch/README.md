# Drum bank switch 246

Standalone numeric conformance model; original outer routine with an explicitly
authored L1 service. See [scope and findings](../../docs/drum246-bank-switch.md).

```sh
cmake -S research/drum-bank-switch -B build-drum-bank-switch -DCMAKE_BUILD_TYPE=Release
cmake --build build-drum-bank-switch --config Release --parallel 2
ctest --test-dir build-drum-bank-switch -C Release --output-on-failure
```

Python decompresses committed numeric fixtures into a temporary directory; the
native verifier compares every output byte. No firmware is needed for replay.

For original extraction use `scripts/verify-drum-bank-switch246.py --ldr PATH
--assembler PATH --linker PATH --simulator PATH --testutils PATH --output NEW_PRIVATE_DIR`.
Use the patched Blackfin toolchain documented in research/blackfin; the script
checks the firmware hash and both parallel-writeback probes. `--record` replaces
fixtures intentionally. Keep firmware, ELF, SDRAM and generated assembly private.

Each of 2048 input records contains eight little-endian u32 values: reload flag,
UI context, old bank, target bank, new LFO type, old LFO type, held count, and own
service delay (1–4 calls), then 8796 window bytes. Window order and sizes are in
[the evidence JSON](../../docs/firmware/drum-bank-switch246-validation.json).
Outputs are those windows, four 32-byte own trace slots, and a u32 call count.
Each used slot holds six u32 fields: incoming R0, selected bank, active bank,
marker byte, held count, completion flag. Padding and unused slots retain 0x5a.

The caller optionally reloads all windows, then always supplies config1024,
drum references32, held registry2564, voice globals812, instrument controls3072,
and gate bytes8. It sets active bank/context, invokes original old audio/modulator
constructors, then supplies 256 bytes of live old audio states and guards.
Only unpatched regions retain prior-case memory on non-reload rows.

The original changer/sync/setter/factories/resets run unmodified. The only L1
endpoint is a harness trampoline: after the configured calls it sets active bank
and invokes audio type8 then selected LFO factory. This service order and finite
delay are authored. No PCM rendering, original L1 image, host scheduler or acoustic
comparison is included. Model bool returns and profile guards are own API choices.
