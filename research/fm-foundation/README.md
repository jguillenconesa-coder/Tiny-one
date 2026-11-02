# Independent FM dependency verification

Build/test without JUCE or firmware:

```sh
cmake -S research/fm-foundation -B build-fm -DCMAKE_BUILD_TYPE=Release
cmake --build build-fm --config Release --parallel 2
ctest --test-dir build-fm -C Release --output-on-failure
```

Own C++/harness source is AGPL-3.0-only. Vectors contain synthetic input and numeric
observations, not original instructions. See [scope and reproduction](../../docs/fm246-foundation.md).
No original FM audio renderer is implemented by this target.

`vectors/fm246-foundation-input.bin`: 8192 little-endian records of 128 bytes:
60-byte self; int16 amount; uint16 padding; uint32 action; 30 int16 table words.
Action 0 retains self, 1 loads self, 2 loads then runs original reset, 3 loads then
runs original constructor. Every record executes original table-copy and all six
internal envelope updates. Per-record tables are explicit synthetic inputs.

`vectors/fm246-foundation-output.bin`: 30 converted Q15 table words (60 bytes),
conditioned on ten explicitly authored binary32 constants. Next six factory
records of 364 bytes: complete six-voice pool 360 and uint32 return pointer. Pool
starts at byte pattern 0xa5 and retains state between factory calls. Finally 8192
records of 128 bytes: complete self 60, table 60, uint32 prefix/suffix guards.
The numeric prefix is 2244 bytes. No output from the audio renderer is included.

The independent C++ test compares conversion, factory neighbor/padding/return
preservation, constructor versus reset, all gains/angles and all per-record bytes.
The source model separately identifies all ten authored words; altering any of
twenty clear words is rejected by the firmware verifier before simulation.
