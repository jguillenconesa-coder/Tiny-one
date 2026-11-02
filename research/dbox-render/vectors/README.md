# D-box 246 numerical observations

Synthetic data only: no original instructions, firmware, audio resources or patches.
The private source is the operator-supplied LDR with the SHA256 in
`docs/firmware/op1-246-ldr-map.json`. Reproduce with `scripts/verify-dbox246.py`
and the documented three-writeback-patch GNU BF524 profile. Output is compared
independently by `Dbox246RenderTests.cpp`, including all twelve states and shared
context; the C++ renderer never receives original post-render state as input.

4096 blocks / 524288 samples. The first 2048 cases supply independent states;
subsequent cases retain previous computed state, with periodic reset/cold starts.
Controls exercise eight mix branches, boundaries and random positive raw values.
Pitch covers zero, extremes and random signed words; gain covers signed int16.
Even/odd cases use word-aligned and halfword-aligned output buffers respectively.
Constructor/bind/reset/release/shared initialization/meter decay/capture reset and
activity-query actions interleave. Padding, inactive voices, constants, peaks,
64-sample histories, counters, pitch/gain/derived/intermediate buffers and four
guards are compared. Coefficients match existing ADSR and Filter table observations.

Input records: 3296 bytes (864-byte twelve-state pool, 1640-byte context, eight
halfword controls, flags/slot, 128 pitch words, 128 gain halfwords). Flag bits:
0 supply state/context, 1 construct/bind, 2 reset, 3 release, 4 shared init,
5 meter decay, 6 reset capture, 7 activity query, 8 halfword output alignment.

Output prefix: 1540 bytes (128 attack, 128 decay and 129 frequency words).
Output records: 6816 bytes (2504-byte pre-state/context, 2504-byte post-state/context,
256-byte audio, 512-byte pitch, 512-byte derived pitch, 256-byte intermediate audio,
256-byte gain and 16 bytes of guard words).

This validates the isolated callback under supplied dependencies and a corrected
CPU model. It does not establish the original caller's tuning, pad allocator,
clock/event cadence, output routing or physical acoustic equivalence.
