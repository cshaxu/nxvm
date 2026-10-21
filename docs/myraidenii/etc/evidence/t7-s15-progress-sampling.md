# M1 T7 S15 — Bounded Selected-Set Progress Sampling

## Boundary and evidence grade

S15 changes only the existing selected-set integration Probe.  It samples the
already-rendered native frame at four fixed cumulative instruction positions;
it does not add a machine path, a ROM loader, a renderer, a trace, or a
production behavior.  The owner-supplied ROM archive is read only through the
existing validated staging wrapper, whose temporary directory is deleted on
return.

The results below are **L2 runtime observations**.  They establish bounded,
repeatable progression in the current local frame state.  They do not prove a
title screen, attract mode, correct controls, palette/blend fidelity, audible
sound, guest hardware timing, gameplay, or a release qualification.  No
primary manual PDF formula, constant, or rule is used here, so nothing in this
record is L3.

## Probe design

The original Probe exposed only one final frame digest after its finite
execution budget.  A changed final digest could not distinguish a progressing
frame from one early mutation.  S15 retains the maximum 20,000,000-instruction
budget from S9, but obtains four non-destructive frame samples at its
quarter-budget thresholds.  A sample contains only:

- cumulative executed instructions;
- the existing non-reversible FNV-style frame digest; and
- the existing count of non-black pixels.

Frame capture is already a copied Board output.  PCM capture is intentionally
not sampled: the existing PCM API drains the queue, so extra reads would alter
the observed machine.  The prior single final PCM observation remains intact.

## Dual-width result

Both runs held Coin 1 and Start 1, used the existing `raiden2.zip` archive and
its inventory verifier, and left no stage directory beneath
`O:\assets\raidenii\myraidenii-diagnostics\`.

| Sample | x64 executed | x64 digest | x64 non-black | x86 executed | x86 digest | x86 non-black |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 1 | 5,001,184 | `d7e4cff81e7dabcd` | 74,863 | 5,001,184 | `d7e4cff81e7dabcd` | 74,863 |
| 2 | 10,003,352 | `e6d49832ee1363b8` | 74,865 | 10,003,352 | `e6d49832ee1363b8` | 74,865 |
| 3 | 15,000,559 | `2b271d19a52864f7` | 74,865 | 15,000,559 | `2b271d19a52864f7` | 74,865 |
| 4 | 20,000,000 | `baabe5eae2033206` | 74,865 | 20,000,000 | `baabe5eae2033206` | 74,865 |

Both widths completed the finite budget with stop reason `budget`, zero CPU,
COP and Board-control failures, and three observed inter-sample frame
transitions.  Their final PCM observation remains 4,096 copied frames with
zero nonzero samples; this is a retained sound-qualification gap, not a new
or confirmed Sound defect.

## Verification

- Rebuilt `myraidenii-selected-set-startup-probe` in the stable x64 and x86
  Ninja trees.
- Ran the owner-ROM wrapper once per width at the existing 20M cap.
- Ran all 12 MyRaidenII registered unit tests on x64 and x86: all passed.
- Ran MyRaidenII documentation governance and `git diff --check`: passed.

## Disposition

This closes the ambiguity in S9's single final frame summary: the local Board
does produce changing frame output throughout this bounded run and does so
identically on both host widths.  It does not supply an implementation contract
for COPX-D2 or the deferred sprite-blend rule.  Those receivers remain in
`states/TODO.md`; no L1 approximation is introduced merely to turn a progress
observation into a gameplay claim.
