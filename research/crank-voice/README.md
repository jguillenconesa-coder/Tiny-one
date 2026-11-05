# Crank voice gain/pitch verification

Build this standalone CMake project to run the numeric callback replay and host
integration without JUCE or firmware. The gzip fixtures are synthetic numeric
inputs/observations only; the Python driver unpacks them in a temporary directory.
18432 input records of 128 bytes and output records of 1140 bytes follow the old
Crank packet with a full gain-state word inserted after the previous-angle word.
All buffers are initialized per call; source and coefficient may be retained.
2060 accepted blocks and 16372 rejected calls compare all fields and buffers.
The owner-aware shared voice bridge replays every accepted block too.

Regenerate using scripts/verify-crank-voice246.py with privately supplied rev246
LDR, binutils/testutils and GNU BF524 with three documented writeback patches.
Choose a new private output directory; --record commits only numeric observations.
No physical source, original scheduler, buffer producer or acoustic equivalence
is inferred. See docs/crank246-voice.md for coefficient curves and host limits.
