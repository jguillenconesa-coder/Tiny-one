# Drum sampler 246 numerical observations

Synthetic PCM and numeric observations only. No proprietary executable, factory
samples, patches, database or decompiled code is included. The private LDR hash
is recorded in `docs/firmware/op1-246-ldr-map.json`. Reproduce using
`scripts/verify-drum246.py` and the documented three-writeback-patch GNU BF524 model.

4096 blocks / 524288 samples compare the complete independently implemented mono
callback: twelve 20-byte states, constructor/binding/reset/release/activity query,
forward/reverse, gated/one-shot/loop control bands, retained state and all buffers.
The C++ kernel never receives the original post-render state as input.

The original float-to-fract16 helper converts all 1025 legible FIR coefficients.
They match `Source/Drum246Tables.h`; the 16-tap/64-phase MAC saturates at each W32
accumulation, then applies the supplied nonlinear gain stream and final scaling.
No coefficient is substituted. The supplied PCM banks contain alternating full-
scale samples, a ramp, impulses and deterministic noise. Sixteen explicitly zeroed
samples surround each bank. This bounds the small negative cursor overshoot that
can be retained after reverse playback; it is not recovered physical RAM.

Input prefix: 33024 bytes, four banks of 4096 PCM16 samples each, each with 16
zero samples before and after. Input records: 1032 bytes — 240-byte state pool,
eight signed16 controls, flags/slot, 128 pitch words and 128 gain halfwords.
Pitch streams are nonnegative signed32; gain spans signed16. The first 2048 cases
supply independent pools; later cases retain computed state with periodic reload.
Flag bits: 0 load pool, 1 construct/bind, 2 reset, 3 release, 4 activity query,
5 halfword-aligned output; bits 8–9 select the supplied PCM bank.

Output prefix: 2052 bytes — 1025 FIR halfwords plus two alignment bytes.
Output records: 2032 bytes — pre/post pools (240 each), 256 audio bytes, 512 pitch
bytes, 512 derived-gain bytes, 256 gain bytes and 16 guard bytes. Output suffix:
33024 PCM bytes, compared against the input prefix to detect writes. Guards protect
pool end, FIR end, PCM end and RNG. Word/halfword-aligned output cases alternate.

Original drum dynamics, note/pitch production, scheduling and source loading are
outside this callback. PCM, gain, positive pitch, padding and event order are
supplied. Isolated numerical agreement is not a physical acoustic comparison.
The native adapter test separately checks its authored import/clock/region model;
that test does not turn the own dynamics model into an original reconstruction.
