# Envelope publication — rev246 research

Independently authored, research-only model of the readable `0x01a66922` writer,
its shared word and reset/getter. It is not linked into Pocket One.

See the [contract and limits](../../docs/element246-producers.md). The native
suite needs C++17, CMake 3.22 and Python 3.11; it needs neither JUCE nor firmware.
The fixture observations cover 4,096 blocks / 524,288 audio halfwords, four
artificial caller states sharing one publisher, reset/getter and full state/audio.

```
cmake -S research/element-producers -B build-publisher -DCMAKE_BUILD_TYPE=Release
cmake --build build-publisher --config Release
ctest --test-dir build-publisher -C Release --output-on-failure
```

The actual call context and control semantics remain unvalidated. Do not use
the published value as a voice ADSR or identify it as master compression solely
from this isolated numerical correspondence.
