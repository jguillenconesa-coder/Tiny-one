# DNA conditioned render and live RAM replay

```
cmake -S research/dna-render -B /tmp/dna-check -DCMAKE_BUILD_TYPE=Release
cmake --build /tmp/dna-check --config Release --parallel 2
ctest --test-dir /tmp/dna-check --output-on-failure
```

No firmware or JUCE is needed to replay the committed numeric observations.
`dna246` compares 4096 blocks plus a two-call paired RAM-dependence probe.
`dna246_modulation` compares the original DNA (type 11) descriptors and 4096
Value/4096 Random calls using Tests/ReferenceVectors. Original hardware identity,
RAM content and scheduler are not recovered by these tests.

Main input records are 256 bytes: six 32-byte live states, four int16 controls,
pitch/RNG/action/RAM-seed/slot/RAM-profile uint32 words, and 32 identity bytes.
Action bits: load 1, reset selected slot 2, release 4, factory construct slot 8.
RAM profiles: zero or LCG words unpacked little endian; the live pool overwrites
the first 192 RAM bytes. The post-OTP initializer uses the exact supplied bytes.

The output prefix is low table 258 bytes, interpolation 450 and descriptors 64.
Each load emits the first 256 int16 waveform cells (512 bytes). Each callback
emits 492 bytes: pre-render selected state 32, post-render pool 192, shared feedback
word 4, RNG 4, audio 256, original division low word 4. Every 256 records then emit
196608 RAM bytes and the full 24576-byte waveform. Expected main sizes are 1048576
input bytes and 5575428 output bytes; 40 identity loads and 16 full snapshots.

The paired probe has two fresh callbacks with identical state/identity/control/
pitch/RNG, differing only in non-pool RAM. It verifies audio dependence on that
RAM without conflating a different voice state or hardware identifier.

Regenerate with scripts/verify-dna246.py and optionally --ram-probe; fresh private
outputs outside checkout, operator-supplied rev246 LDR and the documented patched
GNU BF524 simulator. --record stores synthetic inputs and numeric observations
only. The original identity continuation is entered with an authored ABI frame;
no OTP/ROM reader is patched or claimed validated. Added simulator RAM is an
explicit experimental input, not a recovered hardware memory map.

The plugin profile is fixed virtual identity + live six-slot pool + zero outside
that pool. Matching an oracle with supplied data does not establish the correct
physical profile, acoustic equivalence, original note scheduling or Live scanning.
