# Melodic sampler 246 numerical observations

Synthetic PCM and numerical observations only; no firmware, original instructions,
factory audio/patches or decompiled code. Reproduce with `scripts/verify-sampler246.py`
and the documented GNU BF524 model with three writeback patches.

4096 blocks / 524288 samples compare the complete isolated callback `0x01a51a7c`:
six 16-byte states, factory/bind/reset/release, forward/reverse, independent loop
boundaries, note release, restart latch, volume and all 128 supplied pitch words.
The first 2048 cases load independent pools; later cases retain computed state
with periodic reloads. Original post-state is never handed to the C++ kernel.
Pitch is nonnegative in the first half and spans signed32 in the second. Five
nonnegative scalar pitch factors include zero and INT32_MAX. Both word/halfword
aligned output addresses are used. FIR conversion checks all 1025 original legible
coefficients against Drum246Tables.h, with no substitutions.

Input prefix: 33024 bytes, four synthetic PCM16 banks (4096 frames with sixteen
zero guards on each side). Each is copied into an explicitly zero-filled 0x200100
byte view beginning at `0x04000000 + bank*0x200100`; PCM origin is view+0x100020.
The large zero regions make signed cursor excursions under arbitrary supplied
pitch/direction sequences explicit. They are not recovered physical RAM. Bounded
C++ access returns zero outside the supplied 4096 frames. The original source
PCM prefix is dumped unchanged; this does not dump every expanded padding byte.

Each 636-byte input record: six states (96), controls (16), flags/slot/scalar (12),
pitch (512). Flag bits: 0 load pool, 1 construct/bind, 2 reset, 3 release,
5 halfword-aligned output; bits 8–9 select the PCM bank. Record offsets are little
endian. The full pool includes untouched latches/padding, not only the active voice.

Output prefix: 2052 bytes (1025 converted FIR halfwords and alignment padding).
Each 968-byte record: pre-pool (96), post-pool (96), audio (256), unchanged pitch
(512), pool/FIR end guards (8). Output suffix: original 33024-byte PCM prefix.
Guard constants: 0x13579bdf and 0x2468ace0.

The oracle supplies source loading, source sample-rate scale, event cadence and
pitch stream. It does not validate a complete physical instrument or the host's
ADSR, velocity, voice allocation, clock, import conversion, trims or resampling.
