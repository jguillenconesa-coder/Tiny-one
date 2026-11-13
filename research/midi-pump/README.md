# Original MIDI pump/completion 246

Numeric conformance model with original enqueue, complete callback entry in
event 9/event 16 profiles, pump/copy and submission. Supplied detached requests
and busy endpoint prevent driver activation; no code replacement or L1 endpoint.
[Findings/limits](../../docs/midi246-pump.md).

```sh
cmake -S research/midi-pump -B build-midi-pump -DCMAKE_BUILD_TYPE=Release
cmake --build build-midi-pump --config Release --parallel 2
ctest --test-dir build-midi-pump -C Release --output-on-failure
```

Python expands committed numeric gzip fixtures into a temporary directory.
Native replay needs no firmware. Original extraction uses
`scripts/verify-midi-pump246.py` with `--ldr`, `--assembler`, `--linker`,
`--simulator`, `--testutils`, a fresh external private `--output`, and optionally
`--record` to replace fixtures. The LDR hash, both patched Blackfin writeback
probes and three PC-watch forms are checked. Execution watches exclude driver
activation, error helper and L1 range. Keep firmware, SDRAM, ELF and private
assembly external to Git. The input section is at 0x06000000 to avoid SDRAM overlap.

4096 independent records: 5108 input bytes, 5092 output bytes. Input consists of
five little-endian u32 words: action 0(enqueue)/1(direct pump)/2(event 9)/3(enqueue
then event 9)/4(event 16), count(words or direct bytes), completion bytes, extra
callback metadata 4 and metadata 5, then 5088 bytes of observed windows. Callback
extra words are supplied in the own caller stack; no physical ABI beyond the
observed routine is inferred. Every window reloads per case.

Window addresses/order/lengths are in the
[evidence JSON](../../docs/firmware/midi-pump246-validation.json). Ring layout
matches the earlier queue corpus; word 0 is now observed as completion statistic.
Own simulated objects live at 0x02021000(context),0x02021100(owner),
0x02021200/240(requests),0x02021300(endpoint),0x02021400/500(buffers),
0x02021600(payload). Requests start detached with self links and either null
endpoint or the supplied endpoint. Endpoint sentinel links start empty and its
active-request field is explicitly nonzero. At most one pump attempt per case
is scheduled after enqueue/completion, apart from the standalone direct action.

Output: all 5088 window bytes plus u32 direct return, captured only for action 1;
other actions retain 0x5a5a5a5a in this own observation. Model bool and unsupported
profile guards are own APIs. Original direct returns 0/-1 are compared; wrapper
return codes are not inferred. All contiguous chunk sizes 1–32, direct copy sizes
0–128, null/valid requests and endpoints, enable gates, ring full/wrap, completion
and reset paths are included. No descriptor reuse, driver activation, hardware
transport, concurrent consumer, physical timing, DSP or plugin integration.
