# Project Status

## Current Work

M5 T539 remains open. S1-S12 are accepted. S13 is automatically admitted
under the owner's standing authorization, after the concrete FDC boundary
review. Its complete implementation is being delivered in target-separated
commits; coordinator acceptance remains pending.

| Task | Progress |
| --- | --- |
| T539 S12 | Accepted: P1 408a31cc7 supplies qualified FDC drive/status behavior, recovered BIOS source and one embedded-ROM route. Full units 350/350 per width; default integrations 20/20 per width and vendor boots once per width. Eight EXEs, static gates and manifests verified. FDC extraction remains pending. |
| T539 S13 | Complete implementation delivery: Shared P1 1d6dc5876 supplies the opaque chip; the NXVM delivery reconnects the PC adapter and removes the old combined owner. Units 355/355 and default integrations 20/20 per width, six final vendor boots once, tools-off 22/22, static gates/manifests and eight refreshed EXEs pass. S13 evidence records coverage and hashes. Coordinator acceptance pending. |

## Active S13 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, M5 T539 S13; coordinator/executor roles in one session. |
| Admission And Approval | Owner 2026-09-28 automatic-S authorization for independent chip extraction. Targets Shared and NXVM, separate commits; MyNES receiver verification and artifact-only update if affected. No sibling mutation, raw-ROM or INI change. |
| Objective | Consume the complete FDC ledger row: one Types-only opaque 8272A mechanism in x86/devices, with real PC board/drive integration remaining in NXVM and the former chip path removed. |
| Non-goals | No new command coverage or timing grade, generic device framework, flux engine, media cache, host firmware service, HDC/video/CPU migration or App split. |
| Reference Baseline | Accepted S12 implementation 408a31cc7 and governance 579f4e35a; source-qualified semantics and eight embedded-ROM artifacts. |
| Candidate Proposal | [T539](../proposals/m5-shared-chip-extraction.md), [finite ledger](../etc/evidence/t539-chip-migration-ledger.md), [S13 boundary](../etc/architecture/t539-s13-fdc-extraction.md), S10 boundary and S12 evidence linked there/from the ledger. |
| Files And ABI Surface | x86/devices/fdc8272, independent x86 tests/build/manifests; NXVM FDC adapter, machine construction/scheduler, copied diagnostics, provider/test/gate call sites and eight artifacts. Public chip API: byte operations, TC, sampled drive/record provider, timing/deadline and outputs; no App/media/port pointers. |
| Applicable Rules | Architecture and Coding rules/skills, product architecture/layout, Execution, Documentation and source policy. Unique command/PCN/physical-head owners; source-style preservation; opaque shared contract; target-separated complete P deliveries and actual-diff acceptance. |
| Verification | Full NXVM units x64/x86; independent tools-off x86 chip suites; existing FDC cases mapped without loss; allocation/registration rollback, signal and due-now ordering; default integration both widths and other profile/width boots once. Rebuild eight 0539 EXEs; inspect MyNES link inputs and rebuild 0043 pair only if affected. Six manifests, corpus/static/document gates, artifact identities and hashes. |
| Expected Markers | Existing FDC functional, drive and firmware markers remain, with independent FDC chip tests and no reverse/private production dependency. No new hardware-completeness claim. |
| Asset Needs | Existing external BYOB inputs/media unchanged, embedded build route retained. Units use code-owned inputs. Reuse build/t539-s3 trees and bounded fixtures; no new external-source acquisition. |
| Reporting Requirements | Confirm the boundary, report source/test ownership and caller disposition, review actual diff, line counts and positive growth, test evidence and artifact hashes. No partial implementation P or claim that relocation qualifies complete silicon. |
| Stop Conditions | New unsupported semantic contract, source/licensing need or target expansion outside this batch; coordinator consolidates any required corrective brief before implementation proceeds. |
| Exit Criteria | All FDC mechanism/state and owner-local cases are shared and independent; NXVM only owns board/drive adaptation; no old production copy, private scheduler inspection or lost coverage; required checks and receiver artifacts pass; complete target commits pushed and coordinator actual-diff accepted. |
| Original Owner Request | Independent chip components in src/x86/devices, board integration retained, no duplicate paths, automatic admission of subsequent S tasks. |
| Similar-Issue Sweep | All 15 commands/invalid encodings; drive sampling/STEP/marks/format; reset/ready/seek/TC/output/deadline paths; every private FDC caller in source/tests/diagnostics; allocation, port registration and DMA/IRQ lifetime rollback. Keep every hit assigned to chip or adapter, not a compatibility mirror. |

The [proposal](../proposals/m5-shared-chip-extraction.md),
[contracts](../etc/architecture/t539-boundary-contracts.md),
[finite ledger](../etc/evidence/t539-chip-migration-ledger.md) and
[task history](../history/M5-T539-independent-shared-chips.md) retain scope and
decisions. Automatic admission does not waive boundary review, target-separated
commits, full verification or coordinator actual-diff acceptance.

S10 [decision record](../etc/architecture/t539-s10-fdc-boundary.md) and
[evidence](../etc/evidence/t539-s10-fdc-characterization.md) record 120 new
characterization cases and 347/347 full units per width. Intel READY/SEEK
contradictions and pending-completion safety remain explicit cutover gates;
no production, Shared or artifact change is claimed.

S11 [evidence](../etc/evidence/t539-s11-fdc-seek-ownership.md) records reproduced
identity/duplicate/overflow-admission failures, their single-owner correction,
347/347 units per width, 20/20 default integrations per width, each other
profile boot once and eight updated artifacts. Shared and INI are unchanged.

S12 [evidence](../etc/evidence/t539-s12-fdc-drive-status.md) records source
qualification, actual-diff acceptance, negative controls and packaging proof.
Commercial originals remain external; owner-approved embedded EXEs are tracked.

M5 Td S174 queued this three-stage migration. Its first candidate is T539;
the seven remaining [Queue](QUEUE.md) candidates retain their dependency order.

## Current Technical Baseline

Four fixed products remain XT, AT, Model 40 and default PC/AT; PC110 is not
runnable. Eight optimized compiler-debug-stripped 0539 EXEs are current with
unchanged owner INI contents. MyNES retains its two unchanged 0043 receivers;
its T43 remains closed.

Lib/Common retain the accepted 268464d49 baseline. Shared x86 PIT is accepted
at 24162ac93, RTC at 8a8435648, PIC at d6dc6ca3a, DMA at 53b4be21d, the AT
keyboard chain at eb1e2e208, and XT PPI/keyboard at 0f9c6b1a8.
Current NXVM source/artifacts are 408a31cc7. [S12 evidence](../etc/evidence/t539-s12-fdc-drive-status.md)
owns qualification, firmware recovery, negative controls and artifact hashes.

Coordinator-role review accepts S12's actual source/build/test/document diff,
including the deleted runtime loaders and machine-specific unready policy,
guest-owned firmware services, source-qualified status changes and stopped-HLT
cancellation. P1 is pushed to origin/master; no S12 packet item remains. The
packet is removed. FDC extraction and the remaining ledger keep T539 open.

Final units pass 350/350 per width; default external integration passes 20/20
per width and every other profile/width boot passes once. The latter boot
evidence precedes the host-only stopped-HLT cancellation repair, as explicitly
qualified in S12 evidence; it is not claimed as a fresh boot of each final hash.
The post-commit specialized static aggregate, six manifests,
documentation governance, local links and whitespace checks pass.
Both reusable NXVM build trees are restored to default configuration.
No other product or sibling repository changed. Cooked-history rollback debt
remains in [TODO](TODO.md). Acceptance does not claim complete 8255 silicon,
new timing grades or indefinite absence of intermittent faults.

## Historical Context

[Earlier status](../history/M6-T41-and-prior-status-archive.md) preserves prior
deliveries. [T538 history](../history/M5-T538-deployed-boot-pairs.md) preserves
the earlier boot qualification. [S8 evidence](../etc/evidence/t539-s8-kbc-extraction.md)
retains the AT chain; [S7 evidence](../etc/evidence/t539-s7-dma-extraction.md)
retains DMA extraction and [S6](../etc/evidence/t539-s6-dma-first-service.md)
its first-service repair.
