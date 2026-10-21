# M1 T7 S16 — Bounded Board-to-Sound Transport Observation

## Boundary and evidence grade

S16 changes only the existing selected-set startup Probe. It extends S15's four
non-destructive frame checkpoints with two already-public Board command bytes
and the Board-owned Sound-interrupt-pending latch. It does not acknowledge or
clear either latch, inspect protected payload, add an audio path, alter the
execution budget, or change production code.

All observations below are **L2 runtime evidence**. They are observations of
the owner-supplied selected set running through the current local Board; they
are not an explicit fact from an inspected primary manual PDF. They therefore
cannot establish L3 sound timing, YM/OKI behavior, audible output, title/attract
correctness, or gameplay.

## Result

The existing owner-ROM staging wrapper ran the unchanged 20,000,000-instruction
probe with Coin 1 and Start 1 held. The x64 and x86 runs produced identical
four-sample results. Each sample retained the S15 frame progression values and
observed the same Board-to-Sound state:

| Sample | Executed | Command 0 | Command 1 | Interrupt pending |
| --- | ---: | ---: | ---: | --- |
| 1 | 5,001,184 | `0x00` | `0x00` | yes |
| 2 | 10,003,352 | `0x00` | `0x00` | yes |
| 3 | 15,000,559 | `0x00` | `0x00` | yes |
| 4 | 20,000,000 | `0x00` | `0x00` | yes |

The probe still reached its fixed budget without CPU, COP or Board-control
failure. Its one final destructive PCM read remained unchanged from S15: 4,096
copied frames and zero nonzero samples. The wrapper removed its staging
directory after each run.

## Disposition

The main CPU does reach the local Board-to-Sound interrupt latch during bounded
startup. This excludes the narrower hypothesis that the selected set never
touches the local Sound transport.

It instead identifies a finite local receiver: the current Sound CPU accepts
an interrupt only when its YM timer reports pending, while the Board's existing
Sound-interrupt-pending latch is not supplied to that arbitration. A later,
separately admitted S must design and verify that one Board-to-Sound interrupt
contract. Its model remains L2 unless an inspected primary-manual PDF supplies
a directly applicable rule or constant. This observation does not authorize a
second clock, a second execution loop, a Sound synthesis approximation, or an
audio-success claim.

## Verification

- Rebuilt the existing startup Probe in the stable x64 and x86 Ninja trees.
- Ran the existing owner-ROM staging wrapper once per host width at S15's
  unchanged 20M cap; its neutral sample output matched between widths.
- Ran the twelve registered MyRaidenII unit tests on x64 and x86.
- Ran MyRaidenII documentation governance and whitespace checks.
