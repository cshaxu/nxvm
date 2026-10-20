# M1 T5 S3 — Initial Local Z80 And Sound Map

## Scope

This is the first bounded S3 delivery, not a claim that the selected sound ROM
or its complete Z80 instruction set has been qualified. It replaces S2's
clock-only object with one private Core Sound owner which can consume Board
time and execute a small independently written Z80 subset through the selected
fixed sound map.

## L2 Facts Adopted

The inspected local MAME Raiden II driver at
`O:/repos.external/mame-raiden2-l2/raiden2.cpp` identifies the selected map:
fixed ROM `0000--1fff`, RAM `2000--27ff`, Seibu handoff registers
`4000--401b`, and banked ROM `8000--ffff`. It also identifies the 28.636363 MHz
divided-by-eight Z80 clock already recorded by S1. These board-integration
facts are L2. No MAME source, ROM byte, table or generated output is retained.

## Retained Ownership

Board remains the only owner of main-side command/response/pending/interrupt
state and the only guest-time callback. Sound owns the copied optional sound
program, Z80 register/RAM state, fixed map decode and bank selection. The local
transport is a narrow private contract: a mapped operation is the only route
which can acknowledge a command, observe pending state, update a response or
read coin input.

## Initial Execution Boundary

The current decoder covers the reset/bootstrap load, memory-transfer, direct
jump, halt and simple register-transfer forms exercised by the synthetic map
programs. Unimplemented Z80 instructions return `LIB_STATUS_UNSUPPORTED`; they
are not silently treated as NOPs. YM2151/OKI addresses likewise remain rejected
until S4 owns their real register semantics. Consequently this record makes no
selected-ROM, sound-output or playable-game claim.

## Verification

The existing Sound unit now executes synthetic Z80 programs proving command
read, pending observation, response write, acknowledgement, reset and banked
ROM transfer. The existing Board bus test continues to prove the independent
main-side handoff. Cached x64/x86 builds and the complete six-test MyRaidenII
unit selection pass. A supplied external sound image was inspected only to
confirm that the real bootstrap needs a later instruction-set batch; no
protected asset, bytes, table or generated output is retained or committed.

The existing opt-in selected-set probe now stages exactly the two program
lanes, seven graphics images and the one sound-program image it passes to
Board. On the owner-supplied set, both x64 and x86 reach the same existing
terminal main-machine observation after 12,721 V30 instructions and 205,420
elapsed ticks. This is an ingress/ownership check only: it neither proves
complete Z80 execution nor interprets the terminal main-machine observation as
sound, rendering or playable-game qualification.
