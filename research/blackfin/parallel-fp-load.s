// SPDX-License-Identifier: AGPL-3.0-only
// Authored word-load/store source-latching probes. No firmware required.
.include "testutils.inc"
start
imm32 SP,0x03000000
FP = SP;
P0.H = results;
P0.L = results;
I0 = P0;
imm32 R0,0xaabbccdd
[FP - 4] = R0;
imm32 R0,0x12345678
MNOP || R0 = [FP - 4] || W[I0++] = R0.H;
[P0 + 4] = R0;
P0 += 8;
I0 = P0;
imm32 R0,0x24681357
R1 = 3;
R2 = R1 + R1 (S) || R0 = [FP - 4] || W[I0++] = R0.L;
[P0 + 4] = R0;
[P0 + 8] = R2;
P0.H = args;
P0.L = args;
R0 = P0;
P0 = 5;
EXCPT 0;
exit 0
.data
.align 4
args: .long 1,results,20
results: .space 20
