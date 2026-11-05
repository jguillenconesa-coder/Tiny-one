# Bend voice gain/pitch validation

Build this standalone CMake project to replay two suites without JUCE or firmware.
The original callback fixtures contain 6144 records: 96 input bytes, 1108 output
bytes. Layout is identical to the existing Bend parameter oracle, but destination
3 and colours 0/1 exercise gain/pitch; source state and buffers are explicit.
All buffers are reinitialized per call, while the source state/display may be
retained. Numeric fixtures are gzip-compressed; the Python runner unpacks them
in a temporary directory outside checkout. No firmware or decompiled code is
included. The block bridge compares every accepted original buffer too.

Regenerate with scripts/verify-bend-voice246.py and a privately supplied rev246 LDR,
assembler/linker/testutils and the GNU BF524 simulator with all three documented
writeback patches. Use a new private output directory. Repeat without --record
requires byte equality; --record stores only synthetic numeric observations.
The host integration suite covers the authored shared coefficient clock and
per-voice capture policy, not the original scheduler or gain/pitch producers.
See docs/bend246-voice.md for the precise scope and limitations.
