# CWO 246 building blocks

Independent C++17 verification project; no JUCE or original firmware is needed to
run committed fixtures. Since 0.26 the plugin includes the composed CWO Hybrid callback; foundation,
time and complete-kernel scopes remain separate. Headers are promoted to Source.
See [scope, provenance and phase-model limits](../../docs/cwo246-foundation.md).

Inputs: 65,536 records of 196 bytes:

- Controls8, signed clipper input2, independent allpass state/input/coefficient6.
- Two phase inputs8, phase state136, state-load flag4.
- Filter state12, input2, padding2, coefficient4.
- Smoother state8, coefficient4 (destination is control 4).

Outputs: 22,684-byte table/global/declared-phase prefix, then 65,536 records of
196 bytes: frequency4, Feedback8, Delay4, clipper return/input4, allpass output/state4,
phase outputs8/state136, filter state12/output2/padding2, smoother state8/output2/
padding2. Phase state loads every 4096 calls; other filter states are independent
inputs. Four Hilbert coefficients are explicitly authored substitutions.

```sh
cmake -S research/cwo-foundation -B build-cwo -DCMAKE_BUILD_TYPE=Release
cmake --build build-cwo --config Release --parallel 2
ctest --test-dir build-cwo -C Release --output-on-failure
```

Time fixtures: 8192 records of 44 bytes; outputs have a 22684-byte prefix, 272-byte
records (delay state/output and two 64-sample arrays) and a 131072-byte workspace
snapshot every 512 records. Kernel fixtures: 4096 records of 460 bytes (state252,
controls8, audio128, display64, load4, seed4); outputs have the same prefix, then
444-byte records (audio128, state252, display64) and a full workspace per 512 records.
