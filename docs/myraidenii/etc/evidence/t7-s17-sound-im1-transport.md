# M1 T7 S17 — Board Sound-Interrupt IM1 Transport

## Boundary and evidence grade

S17 extends only the local MyRaidenII Board-to-Sound transport. The existing
Board latch is supplied to the existing local Z80 interrupt gate as a read-only
second source beside the existing YM timer source. No source is cleared by
delivery: the guest retains the existing acknowledge-register write as the sole
clear path. There is still one Board clock, one Sound CPU executor and one PCM
queue.

The union of the Board latch with local Z80 IM1 arbitration is an **L2 model**.
S16 is a runtime observation; the inspected external sound-map behavior is also
secondary evidence. Neither is an explicit directly applicable formula,
constant or rule from an inspected primary manual PDF, so this record makes no
L3 claim.

## Local contract and regression

`myraidenii_core_sound_transport` now requires one read-only
`interrupt_is_pending` callback. Board supplies its existing
`sound_interrupt_pending` latch. The local Sound CPU queries it at its one
existing interrupt acceptance point. When IM1 and EI permit delivery, either a
YM pending source or that Board source enters the existing `0x0038` handler.
The handler must still write the existing guest acknowledge address to clear the
Board latch.

The existing Sound unit fixture now proves this complete synthetic path:

1. Board-source pending is asserted before Sound execution.
2. The existing Z80 program enters its existing IM1 vector after EI/HALT.
3. The handler writes a response and the existing acknowledge address.
4. The source is clear only after that guest write.

Existing YM timer-interrupt coverage remains unchanged. This establishes the
local contract; it does not establish the selected ROM's actual mode/vector
protocol.

## Bounded selected-set result

The existing owner-ROM wrapper reran the unchanged 20,000,000-instruction
probe with Coin 1 and Start 1 held, once per host width. The x64 and x86
outputs remained identical:

| Item | x64 | x86 |
| --- | ---: | ---: |
| Executed instructions | 20,000,000 | 20,000,000 |
| Board Sound interrupt pending at all four samples | yes | yes |
| Command bytes at all four samples | `0x00`, `0x00` | `0x00`, `0x00` |
| Copied PCM frames | 4,096 | 4,096 |
| Nonzero copied PCM frames | 0 | 0 |
| Frame transitions | 3 | 3 |

The unchanged pending latch means the selected ROM did not reach this local IM1
clear path within the bounded run. It does **not** prove that the Board request
is wrong, that the local Z80 did not execute, or that audio hardware is silent.

## Disposition

S17 closes one real local omission: an asserted Board Sound source is no longer
invisible to the existing IM1 gate. The selected ROM still requires a separate
receiver for its actual Sound CPU interrupt mode/vector and acknowledgement
protocol. That receiver must first establish a finite L2 contract from allowed
reference behavior or a directly applicable primary-manual PDF; it may not
guess a vector, add another interrupt loop, or reinterpret a persistent latch
as successful audio.

## Verification

- Rebuilt `myraidenii-core-sound-smoke` and the selected-set Probe in stable
  x64 and x86 Ninja trees.
- Ran all twelve registered MyRaidenII unit tests on x64 and x86: all passed.
- Ran the bounded owner-ROM Probe once per width at the unchanged 20M cap;
  both exited successfully and wrapper staging was removed.
- Ran MyRaidenII documentation governance and `git diff --check`: passed.
