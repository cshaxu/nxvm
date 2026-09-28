# Project Status

## Current Work

M5 T538 is accepted and closed. S1-S7 are complete; no task or subtask is active.
Owner explicitly authorizes one successful launch per current EXE/INI pair
instead of three repetitions. All eight pairs pass, and the complete required
unit/integration suites pass. Historical failures remain recorded under that
acceptance disposition, not reclassified as successes or explained causes.

| Task | Progress |
| --- | --- |
| T538 | Closed: S7 Shared repair 064b9619b, NXVM delivery 7149e1d9e, MyNES receivers 193530996; eight current boot pairs pass, all ten artifacts verified and pushed. |

[Task history](../history/M5-T538-deployed-boot-pairs.md) retains reviewed packets
and actual-change acceptance. [Archived proposal](../history/M5-T538-deployed-boot-pairs-proposal.md),
[convergence ledger](../etc/evidence/t538-boot-pairs.md), and
[S7 evidence](../etc/evidence/t538-s7-orphan-release.md) record scope, revised
acceptance, tests and hashes.

M5 Td S174 records the owner's three-stage migration request: shared chips,
shared PC board integration, then four independent Apps. The eight
[Queue](QUEUE.md) candidates include these three first and retain the five
qualification candidates with updated future owners. Implementation remains
unadmitted. NXVM-only proposal/design/status changes passed actual-diff,
dependency/reference review, local-link checks, diff whitespace and the NXVM
documentation gate. No source, Shared corpus, MyNES, INI, asset or executable
changed; current binaries need no rebuild. The Td P is the durable record.

## Current Technical Baseline

Four fixed products remain XT, AT, Model 40 and default PC/AT; PC110 is not
runnable. Eight optimized stripped 0538 EXEs are deployed with unchanged owner
INIs. MyNES retains its two rebuilt 0043 receivers; its T43 remains closed.

Canonical Shared revision is 268464d49; executable behavior is 064b9619b, with
P4's key-specific comment clarification only. Lib source/test add the approved orphan
release repair to 0c71110b0. S5's test-path return corrections also remain for
SoftPC to import. Common/x86 production is unchanged; full sibling parity is
not claimed. No sibling repository was modified.

Verification: NXVM 336/336 units, 21/21 static checks and 20/20 external
integration per width; MyNES 132/132 per width; all six manifests and both
documentation gates pass. Current deployed qualification is 8/8, including
confirmed pause/Debug handoff. Cooked-history rollback debt remains in
[TODO](TODO.md). This bounded boot acceptance does not claim complete hardware
timing qualification or indefinite absence of intermittent faults.

## Historical Context

[Earlier status](../history/M6-T41-and-prior-status-archive.md) preserves prior
deliveries and their hosting context. T538 history preserves S6's negative
qualification and S7's unclassified observations alongside the final accepted
single-pass matrix.
