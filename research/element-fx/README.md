# Element FX audio verifier

C++17/CMake 3.22/Python 3.11, no JUCE or firmware required for the native suites.
See ../../docs/element246-fx-input.md for the contract and commands.

8192 original calls, 444 input / 1204 output bytes each, two identical independent
numeric extractions. 3494 processed (874 source-3 calls return zero), 4698 rejected.
223616 transformed halfwords checked by the core, 111936 kind-2 bridge halfwords.
Full tag/controls/targets/return/source/pitch/gain/audio compared; other 64 audio
halfwords unchanged. Four sources, eight FX types, kinds 2/5 and two curves.

Host tests: 784 rate/effect/source/curve/bypass cases, exact 64/257 audio, signed
sample-aligned external input at 44.1 kHz with exact kernel histories, automation,
PDC, reset, non-finite sidechain guards and source-3 self-product/zero return.
Kind 5 is model-only; host uses kind 2. No original producer or scheduler claim.
