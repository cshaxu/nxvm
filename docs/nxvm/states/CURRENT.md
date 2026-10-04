# Project Status

## Current Work

M5 T540 S1-S92 are accepted. T540 remains open. S93 is superseded without
acceptance by the owner-requested S94-S97 remaining acceptance plan below;
S94's review result is accepted without partial implementation delivery.
S95's independent verification result is accepted without implementation P.
S96's artifact/boot verification result is accepted without partial P.
M5 T540 S97 is active for complete delivery and T closure review.

Neutral execution/time lives in `src/x86/core`, with one implementation.
Common PIT routes and complete PIC/DMA bus owners live in
`src/x86/ibmpc-common`. The working S93 tree now also contains the complete
board construction/reset/time/deadline source batch and AT/XT family adapters;
that extraction is not yet accepted. Genuine D4 state is Model40-owned.
Actual profile, firmware and media choices stay outside neutral Core.

| Task | Progress |
| --- | --- |
| T540 S93 | Unaccepted implementation carryover; remaining review, verification and delivery are assigned to planned S94-S97. |
| T540 S94 | Review result accepted: 626 current source/test/build/tool dispositions and original coverage reconciled; dual-width full units pass 492/492 and both specialized aggregates pass. No partial implementation P; independent corpus and artifacts remain unaccepted. |
| T540 S95 | Verification result accepted: tools-on 298/298 and tools-off 292/292 per width; six manifests, actual no-App link graph and exact six optional exclusions pass. No implementation delivery until S97. |
| T540 S96 | Verification result accepted: eight optimized stripped 0540 identities and eight unchanged-INI boot checkpoints pass once each; deployment no longer rewrites owner INIs. Complete implementation/artifacts remain unaccepted until S97. |
| T540 S97 | Active: all 50 non-boot integration rows pass; S96 proves the other eight boot rows once each. Both specialized aggregates and six manifests per width pass. Eight artifact identities and owner inputs remain unchanged. Final delivery/acceptance audit is pending; [review](../etc/evidence/t540-s97-delivery-review.md). |

## S97 Active Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T540 S97, after accepted S94-S96 verification results; no partial implementation P has been delivered. |
| Admission And Approval | Owner-approved S94-S97 split and automatic S admission; admit the final delivery step on 2026-10-04. Allowed targets are Shared and NXVM, one target per commit. NXVM consumes the new x86 Core/board receivers; MyNES does not link them and must remain unchanged. |
| Objective | Reconcile the complete pending receiver with S94-S96 proof, finish external integration, deliver all implementation and eight verified artifacts, and audit T540 closure against the original extraction request. |
| Non-goals | No App split, chip algorithm/timing upgrade, PC110, new framework, MyNES build/change, owner INI write, external-master mutation or partial implementation delivery. |
| Reference Baseline | Accepted S92 d76d2b15e; S94 626-path review and complete 492/492 units per width; S95 independent tools-on/off suites and six manifests; S96 eight artifact identities, once-only boots and deployment-cost repair. |
| Candidate Proposal | [T540 proposal](../history/M5-T540-shared-ibmpc-integration-proposal.md#s97-complete-delivery-and-t540-closure-review), [board ledger](../etc/architecture/t540-board-adapter-ledger.md), and [whole-board receiver](../etc/architecture/t540-s93-whole-board-receiver.md). |
| Files And ABI Surface | Complete pending src/x86 and test/x86 receiver plus NXVM source/test/build/tool/document/artifact adaptations already reviewed in S94-S96. Also repair the complete two-family Console-test registration class: launch the deployed EXE beside its owner INI, remove the build-tree INI copy, and guard the boundary. AT's initial check proves the copied relative paths resolve to missing media; this is a test/build repair, not hardware or runtime change. Neutral Core depends on Types/CPU/FPU; flat board families have no common-private back dependency. No new ABI is planned. |
| Applicable Rules | Shared Execution, Architecture, Coding and Document rules; NXVM architecture/layout/source policy. Verify sole owners, preserved assertions, original code style, complete manifest identity, no platform/private leakage, scoped commits and unchanged other-App inputs. Owner-approved embedded vendor-ROM EXEs remain allowed; raw originals stay external. |
| Verification | Reconcile every S94 path hash with recorded S95/S96 follow-ups; retain valid completed full unit/independent/specialized proof rather than repeat unchanged suites. Build the actual 21 remaining default integration targets per width in isolated Release Ninja trees, then run registered integration.vm cases once with boot-matrix excluded because S96 already proves all eight boots. After fixing the Console registration class, configure AT/Model40 in retained width-local Release Ninja trees and run their two extra registered cases once against S96's deployed pairs. Run affected positive/negative boundary controls, six manifest verifiers, documentation governance, diff checks, artifact/hash/INI/MyNES checks and actual pushed-tree review. |
| Expected Markers | No stale/unclassified final path; no failed or skipped required integration; all eight 0540 identities unchanged from S96; six manifests exact; valid full unit/gate evidence; one scope per pushed commit; clean worktree before final acceptance. |
| Asset Needs | Existing owner-managed build firmware and external media referenced by unchanged assets/nxvm profile INIs. Integration uses readonly/overlay access; no source ROM or Microsoft media import. |
| Reporting Requirements | Confirm boundary; report meaningful integration outcomes and newly discovered batch gaps; final report names pushed commits, verification evidence, acceptance and residual decisions without reproducing logs. |
| Stop Conditions | Revise before any new runtime repair; stop for unclassified behavior/ownership changes, modified owner inputs, new protected assets, other-App changes or failed required verification. A failed case never becomes success through a reduced assertion or timeout. |
| Exit Criteria | Entire reviewed/tested implementation and artifacts delivered in ordered Shared/NXVM commits and immediately pushed; every original ledger member disposed, full required unit/integration and gates valid, final architecture/history/current consistent, no carryover and worktree clean. Only then accept S97 and audit/close T540. |
| Original Owner Request | Extract proven common/family PC board integration into x86/core and flat ibmpc-common/at/xt, preserve all four machines and coverage, finish the S93 remainder through S94-S97; optimize avoidable build/test overhead before continuation. |
| Similar-Issue Sweep | Reconcile complete board/private-reader/test/build ownership inventories, check all four deployment callers for INI invalidation and all integration registrations for exact inventory/real external inputs. No fixed focused set or successful-result cache; MyNES exclusions are justified by absent x86 linkage. |

## Current Technical Baseline

Four implemented fixed profiles remain XT, AT, Model40 and default; PC110 is
not runnable. Neutral Core lives in `src/x86/core`; all common/family board
mechanisms live in flat `x86/ibmpc-*`; genuine D4 state remains Model40-owned.
The complete final receiver is reviewed/tested but pending S97 delivery.

[S94](../etc/evidence/t540-s94-source-review.md) owns complete 492/492 root
units per width and specialized proof. [S95](../etc/evidence/t540-s95-independent-verification.md)
owns independent tools-on 298/298 and tools-off 292/292 per width and six
manifest identities. [S96](../etc/evidence/t540-s96-artifacts-and-performance.md)
owns eight optimized stripped 0540 EXEs and once-only unchanged-INI boot
checkpoints. [S97](../etc/evidence/t540-s97-delivery-review.md) reconciles
current integration and follow-ups; full acceptance is still pending.

Eight deployed product hashes and four owner INI hashes remain exact S96
inputs. Runtime debugger stays included. MyNES retains its unchanged 0043
pair and does not link the extracted x86 targets. Native desktop suites run
without cross-tree overlap. Existing valid execution proof is not replayed
for documentary reading or unchanged build registration.

## Historical Context

The [receiver work record](../etc/evidence/t540-s93-whole-board-receiver-work.md#legacy-current-snapshot)
preserves the removed historical Current detail with rebased links; it is
not another status authority. [T540 history](../history/M5-T540-shared-ibmpc-integration.md)
retains accepted prior deliveries; [Queue](QUEUE.md) owns remaining candidates.
