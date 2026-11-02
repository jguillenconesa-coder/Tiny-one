# String original mono replay

Standalone C++17 verification, no JUCE or firmware required:

```
cmake -S research/string-render -B /tmp/string-check -DCMAKE_BUILD_TYPE=Release
cmake --build /tmp/string-check --config Release --parallel 2
ctest --test-dir /tmp/string-check -C Release --output-on-failure
```

`string246` compares 4096 original render calls with complete lifecycle states,
RNG, 128 audio samples/call and 16 complete 24576-cell int16 workspaces. Slot 0
has an explicit workspace binding; original allocator/caller timing is outside scope.

Input records are 64 bytes: state 32, controls 8, pitch 4, RNG 4, action 4,
workspace seed 4, two reserved words. Action bits: load 1, reset 2, release 4,
construct/bind slot 0 8. Loaded memory uses successive LCG32 words, unpacked as
little-endian int16 pairs. RNG is loaded only with state; otherwise retained.

Output prefix is 868 bytes: Q15 low table 258, interpolation 450, Q31 decay 96,
eight range descriptors 64. Each result is 328 bytes: pre-render state 32,
post-render state 32, audio 256, retained RNG 4, original division low word 4.
A 49152-byte memory image follows every 256 results. Input/output fixtures are
synthetic stimuli and numeric observations, never firmware or original instructions.

`string246_modulation` uses the shared replay test with String type 3 and separate
Value/Random fixtures in Tests/ReferenceVectors. It validates the original eight
range descriptors, 4096 Value and 4096 Random callbacks, all returned states and
destinations. It does not establish original scheduling or acoustic equivalence.

Regenerate only with operator-supplied rev246 LDR and the documented patched GNU
Blackfin simulator: scripts/verify-string246.py, verify-value-voice246.py --string
and verify-random-voice246.py --string. Choose fresh private output directories
outside the repository. --record stores only numeric fixtures.
