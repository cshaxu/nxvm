# Project Status

## Current Work

M5 T539 remains open. S2 is closed; S3 is active for the owner-approved PIT
8253/8254 extraction and complete NXVM reconnection.

| Task | Progress |
| --- | --- |
| T539 S3 | Active: extract PIT into Shared x86/devices, retain NXVM board wiring and verify all affected receivers. |

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T539 S3 after S2 P2 9c1eadf0d. |
| Admission And Approval | Owner explicitly approved first PIT extraction and Shared/NXVM reconnection in the 2026-09-28 asynchronous review answer. |
| Objective | Move the sole PIT 8253/8254 mechanism behind an opaque Types-only public boundary, reconnect primary and auxiliary timers and migrate pure chip tests without waveform changes or old copies. |
| Non-goals | Other chip extraction, new PIT modes/algorithms, time-grade changes, Shared Lib/Common changes, MyNES source/artifacts, INIs/media/fonts, firmware hook removal or FDC behavior changes. |
| Reference Baseline | 9c1eadf0d and S2 contracts; original 81-file inventory; eight 0538 binaries are the pre-extraction reference, now replaced by verified 0539 receivers. |
| Candidate Proposal | [Independent chips](../proposals/m5-shared-chip-extraction.md), [contracts](../etc/architecture/t539-boundary-contracts.md), [history](../history/M5-T539-independent-shared-chips.md). |
| Files And ABI Surface | Shared: src/x86 PIT, build/guards/docs/manifest and test/x86 timer cases/guards/manifest. NXVM: PIT old paths, direct callers, board adapters, timer-dependent tests, CMake/tools/docs and eight receiving EXEs. Each target has separate P commits; no sibling writes. |
| Applicable Rules | Task reading set, source policy, Architecture/Coding/Document/Execution authorities and both governance skills. Public opacity, one route/state owner, unchanged waveforms, Types vocabulary, no raw peer pointer, complete failure cleanup. |
| Verification | Standalone PIT source/test builds and full x86 suite x64/x86; full NXVM unit suites and static checks; relevant PIT/IRQ0/refresh/auxiliary tests; affected external boot integration; six manifests, documentation gate, diff check; all four profile Release x64/x86 0539 EXEs with PE/strip/hash verification and unchanged INI hashes. |
| Expected Markers | Only x86 compiles timer mechanism; no old pit files, App include or private PIT access; primary/auxiliary timers work through one public path; all previous behavioral cases retained. |
| Asset Needs | Existing external nxvm-assets only for integration/build; no copies or edits. |
| Reporting Requirements | Report boundary progress, test/build results, actual source/test line delta, target-separated pushed commits and artifact hashes; no claim of T completion. |
| Stop Conditions | New waveform/guest behavior change, unapproved shared-component change or failed required receiver blocks S closure; do not weaken tests or expose mutable internals. |
| Exit Criteria | PIT inventory entries migrated and callers reconnected, no duplicate path, required verification/artifacts/commits complete; coordinator actual-diff review and S closure, T remains open. |
| Original Owner Request | Implement chip/device separation; approved PIT retains counters/registers/Gate/OUT/time while NXVM retains ports/clock conversion/IRQ/refresh/speaker wiring. |
| Similar-Issue Sweep | All PIT functions/types/includes/fields in src/test/build/tools; primary/auxiliary port routes and conflict ownership; reset/destruction output release; shared private-edge verifier negatives; MyNES dependency check proves it does not link the new PIT target. |

[Task history](../history/M5-T538-deployed-boot-pairs.md) retains reviewed packets
and actual-change acceptance. [Archived proposal](../history/M5-T538-deployed-boot-pairs-proposal.md),
[convergence ledger](../etc/evidence/t538-boot-pairs.md), and
[S7 evidence](../etc/evidence/t538-s7-orphan-release.md) record scope, revised
acceptance, tests and hashes.

M5 Td S174 queued the three-stage migration. Its first candidate is now T539;
the seven remaining [Queue](QUEUE.md) candidates retain their dependency order.

## Current Technical Baseline

Four fixed products remain XT, AT, Model 40 and default PC/AT; PC110 is not
runnable. Eight optimized, compiler-debug-stripped 0539 EXEs replace 0538 with
unchanged owner INIs. MyNES retains its two unchanged 0043 receivers; its T43
remains closed.

Lib/Common retain the accepted 268464d49 baseline, including the prior orphan
release repair and test-path corrections. Shared x86 now adds the S3 PIT
component; [S3 evidence](../etc/evidence/t539-s3-pit-extraction.md) owns the
source/artifact mapping. Full sibling parity is not claimed, and no sibling
repository was modified.

Verification: NXVM 337/337 units and 20/20 default-profile external integration
per width; all six non-default profile/width boot matrices pass once. Independent
PIT is 8/8 per width; all 66 specialized static steps and six manifests pass.
MyNES is not a PIT receiver and has no artifact input change. Cooked-history
rollback debt remains in [TODO](TODO.md). This bounded regression acceptance
does not claim complete hardware timing qualification or indefinite absence of
intermittent faults.

## Historical Context

[Earlier status](../history/M6-T41-and-prior-status-archive.md) preserves prior
deliveries and their hosting context. T538 history preserves S6's negative
qualification and S7's unclassified observations alongside the final accepted
single-pass matrix.
