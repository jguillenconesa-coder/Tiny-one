# Drum rev246 retained modulation and render composition

Research only. The plugin stays at 0.54.0; see the
[contract and limits](../../docs/drum246-modulated-render.md).

```sh
cmake -S research/drum-modulated-render -B build-drum-modulated-render -DCMAKE_BUILD_TYPE=Release
cmake --build build-drum-modulated-render --config Release --parallel 2
ctest --test-dir build-drum-modulated-render -C Release --output-on-failure
```

Frozen numeric fixtures need no firmware or Blackfin tools. The runner decompresses
into a temporary directory and supplies the previously verified sine table.
All fixtures are little endian; supported native CI targets are little endian.
PCM is heap-backed to avoid the Windows default stack limit.

## Explicit caller contract

288 sessions cover 12 voices × 8 banks × 3 profiles (Value drum, Random drum,
common Tremolo), with all 24 pads across sessions. Each session has 16 blocks:

1. At block 0: zero the caller's globals/pool and held count; run the real type-8
   audio factory. Prepare one free voice, one enabled primary modulator pointer,
   null secondary, and authored vtables with zero this adjustment pointing directly
   to the original reset bodies. Invoke the complete original note-on.
2. Every block: update base instrument controls and LFO controls; copy base controls
   to the target workspace; load explicit pitch/gain; call one original LFO body.
3. At block 12: complete original note-off before LFO/render. Release uses base
   bank controls, while render uses the resulting target workspace.
4. Render 128 samples except block 15 in odd sessions. The skipped output keeps
   its poison; derived gain retains its previous block. LFO still advances.

Control 0 in the Value/Random workspace does **not** produce the pitch buffer.
Tremolo uses explicitly supplied kind 0/3 (and rejected kind 1); original factory
routing to this callback is not established. Continuing render after release,
including mode zero, is intentional caller scheduling, not a recovered scheduler.

## Binary format

Input prefix: 3072 control-bank bytes, then 8 banks of 529200 int16 PCM frames,
each with 16 zero frames before and after: 8470784 bytes total.
Eight pointer-to-pointer handles at 0x02010000 are bound to these actual sources.

Each of 4608 input records has 988 bytes:

| Offset | Content |
| --- | --- |
| 0 | 8 uint32: flags (reset/off/skip), voice, sound, pad, family, signed transpose, tempo, kind |
| 32 | base controls: 8 int16 |
| 48 | LFO controls: 8 int16 |
| 64 | seed states: Value 12, Random 44, Tremolo 28 bytes |
| 148 | Value display 32, Tremolo display 32, meter 4, RNG 4 bytes |
| 220 | 128 uint32 pitch inputs |
| 732 | 128 int16 gain inputs |

Seed bytes are loaded only when reset is flagged; other blocks retain state.
Object tags are replaced with authored reset-vtable addresses.

Output prefix: converted original FIR 2052 bytes (including 2 pad bytes), sine
16384, decay 96, drum range descriptor 64: 18596 bytes.
Each output record concatenates, in order:

- sampler pool 240, voice globals 812;
- Value 12, Random 44, Tremolo 28, Value/Tremolo displays 32 each, meter/RNG 4 each;
- target workspace 16, original callback return 4;
- audio 256, post-LFO pitch 512, post-LFO gain 256, derived gain 512;
- held list/count 2564, all LFO banks 128, audio pointers 48;
- all base control banks 3072, Core reset globals 68, modulator pointers 84.

Total 8728 bytes per block, followed by all 8467712 PCM/guard bytes, checked
unchanged. Native comparison checks every recorded byte and corpus coverage.

## Private reproduction

`scripts/verify-drum-modulated-render246.py` generates the authored caller and
requires `--ldr`, `--assembler`, `--linker`, `--simulator`, `--testutils`, and a new
external `--output`. It checks the LDR hash and both simulator instruction probes.
`--record` deliberately replaces numeric fixtures; ordinary repetition must match.
Use `research/blackfin/drum-render.ld` and the three verified simulator patches.
Firmware, ELF, original instruction streams and decompilation remain private.
