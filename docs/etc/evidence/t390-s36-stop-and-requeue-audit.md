# T390 S36: Stop-And-Requeue Audit

`M5:T390:S36:STOP-AND-REQUEUE:OK`

## Dependency Decision

T390 cannot truthfully admit its next C1 CPU corpus from the current Model-40
startup evidence. S30 establishes that the corrected-ROM route did not reach a
meaningful protected-mode/C0 baseline within containment. S32 establishes that
the later protected-entry C1 candidate was absent within its contained run.
S35 requires a finite, enforceable semantic corpus before any later physical
selection.

The earliest missing input is a bounded DeskPro startup semantic checkpoint.
It is not an Intel timing-row defect: it belongs to the selected external-ROM
startup/device-composition behavior. The existing board-timing candidate cannot
own it because that candidate correctly requires physical retirement
qualification first. Leaving T390 active would therefore create a dependency
cycle.

## Requeue Result

T390 stops without physical CPU qualification, board timing, physical clock,
firmware compatibility, or L3 acceptance. The Queue now places DeskPro startup
semantic readiness before a renewed physical-retirement qualification. That
receiver owns only the earliest startup checkpoint and bounded functional
repair/transfer needed to make a future C1 ledger admissible. It does not add a
Core/VM bridge or relax the deterministic Model-40 retirement contract.

## Similar-Issue Sweep

T389's prior stop/requeue establishes the correct lifecycle pattern: do not
manufacture a board delay when a shared CPU prerequisite is absent. T386 S29
is functional-only and explicitly transfers real external firmware execution.
T390 S16, S30, S32, S34 and S35 consistently require finite semantic stages,
contained evidence, deterministic behavior outside qualification, and no
profile-specific Core state. The startup receiver is singular; the current
board and physical-device candidates remain later work.

## Verification

This is documentation-only. It changes no source, build, test, artifact,
ROM/media, runtime configuration, interface, or external-research input.
Required closure checks are documentation governance, diff hygiene, and actual-
diff review against the S36 packet.