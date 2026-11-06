# Drum rev246 complete note-on with active modulator resets

Research only; plugin remains 0.54.0. See
[order, scope and limits](../../docs/drum246-active-note-chain.md).

```
cmake -S research/drum-note-modulation -B build-drum-note-modulation -DCMAKE_BUILD_TYPE=Release
cmake --build build-drum-note-modulation --config Release --parallel 2
ctest --test-dir build-drum-note-modulation -C Release --output-on-failure
```

All words little endian. Input prefix 3072 bytes: eight sounds × 24 pads × eight
int16 controls, same supplied bank format as the event-word corpus. Then 4096
records of 772 bytes. First 740 bytes follow the Core modulator reset corpus:
nine uint32 metadata words and 704 observed-window bytes; route must be 1.
The final eight words are signed transpose, note, velocity, metadata, payload,
signed preview, setup flags (bit0 duplicate held note, bit1 all occupied), and
initial global/pool-counter seed.

Caller prepares the actual Core/audio factory type 8 and binds supplied opaque
references. It patches a tracing audio vtable whose query/reset lead to complete
original bodies. Modulator callbacks use the established authored tail endpoints.
The first 2304 cases cover the accepted 12×8×24 grid; later cases add ignored notes
and occupied-byte allocation. No PCM or audio render is included.

Output record, 7624 bytes:

- Established Core/LFO windows: 704 bytes.
- Voice globals 0xff801038: 812.
- Audio pool 0xffb00000: 240; pointer table 0x01aaad30: 48.
- Complete held image/count 0x01dfe734: 2564.
- Complete supplied pad controls 0x01aaadb4: 3072.
- Authored trace: 184 bytes (46 words).

Trace: count, then three receiver/parameter/selection triples for primary,
secondary and reader reset (10 words total). Three 12-word snapshots follow,
one per callback: sequence number, occupancy, owner pad, unsigned raw velocity
halfword, event pitch, voice-pad map, held count and five pre-reset reader words.
Absent callbacks leave their trace slots zero. Tracing order/bytes belong to the
harness, not to the original firmware ABI.

Native model composes Core reset routing and the five known reset models via
an explicit hook in DrumVoice246. It checks every byte and ordering invariants.
The default null-auxiliary path retains its old numeric fixtures. Index/selection
guards in the model are authored; no invalid original accesses are executed.

Private reproduction: `scripts/verify-drum-note-modulation246.py` requires
`--ldr`, `--assembler`, `--linker`, `--simulator`, `--testutils`, and a new external
`--output` directory. Firmware hash and both instruction probes are checked.
`--record` deliberately replaces numeric fixtures; ordinary repetition must match.
No firmware, ELF, original instruction stream or decompilation is distributed.
