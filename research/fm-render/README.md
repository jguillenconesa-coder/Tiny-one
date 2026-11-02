# Four-operator FM render verification

Own C++ and call harness: AGPL-3.0-only. Fixtures contain synthetic arguments and
numerical observations, not firmware, disassembly or decompiled pseudocode.
The audio callback is compared under the explicit ten-word substitution model;
this is not an acoustic equivalence claim. [Scope](../../docs/fm246-hybrid.md).

Build with C++17/CMake, independently of JUCE:

```sh
cmake -S research/fm-render -B build-fm-render -DCMAKE_BUILD_TYPE=Release
cmake --build build-fm-render --config Release --parallel 2
ctest --test-dir build-fm-render -C Release --output-on-failure
```

Input: 4096 little-endian records of **844 bytes**. State 60; four int16 controls
8; uint32 pitch 4; uint32 reload action 4; scratch 512; output seed 256. A nonzero
action reloads state/scratch. Every record reloads the output seed. First 2048
records are independent; later records keep sequences with reload every 128.
Every 64th selector is invalid, cycling negative and positive out-of-range values.

Output: 4096 records of **896 bytes**. Full state 60, table 60, full scratch 512,
audio 256, two uint32 guards 8. Comparison includes padding, retained/untouched
scratch, output preservation on invalid selectors and complete sequenced state.
The 30-word table is the separately verified conversion of the published model.
The sine is supplied/generated using the same definition as existing core tests.

The private oracle executes callback `0x019a23d4` unmodified, with explicit pitch
in `0xff801950`, table/workspace pointer and complete 128-sample output allocation.
The harness is independently written. Proprietary instructions remain only in
the private SDRAM image; no simulator executable, ELF, private image or log is
recorded by `--record`. Original scheduler, note lifecycle and output adaptation
are outside this oracle.
