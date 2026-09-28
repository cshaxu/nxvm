# Project Status

## Current Work

M5 T539 remains open. S6's DMA first-service repair is accepted after actual-change
review. S7 now extracts the opaque DMA controller and reconnects its board users.
Subsequent bounded batches continue under the owner's automatic-S authorization
dated 2026-09-28; no additional manual admission is required.

| Task | Progress |
| --- | --- |
| T539 S7 | Implementation complete: opaque DMA chip and board cutover, all private-test callers migrated. Final full units 342/342 and default integrations 20/20 per width; independent chips 13/13 per width; all six remaining boots pass once. Eight artifact identities verified. Target-separated delivery and coordinator actual-change acceptance remain pending. |

## Active Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T539 S7, following accepted S6 and commit 0ceb739f6. Single-agent coordinator/executor review. |
| Admission And Approval | Owner's 2026-09-28 automatic-S authorization covers the remaining chip extraction batches. Targets: Shared and NXVM, delivered in separate commits. MyNES and sibling repositories remain unchanged. |
| Objective | Extract the existing single 8237A register/priority/service-phase mechanism into an opaque Types-only x86 device; reconnect NXVM's page/lane, paired arbitration and physical-transfer responsibilities, removing the old chip implementation. |
| Non-goals | No universal device framework, new hardware qualification, timing downgrade, new media behavior, CPU extraction, profile change, INI change or MyNES change. |
| Reference Baseline | 0ceb739f6; S6 evidence records 341 units and 20 default integrations per width plus the six remaining boot rows and eight current 0539 artifacts. |
| Candidate Proposal | [Independent shared chips](../proposals/m5-shared-chip-extraction.md), [boundary contracts](../etc/architecture/t539-boundary-contracts.md), and [S7 boundary review](../etc/architecture/t539-s7-dma-boundary.md). |
| Files And ABI Surface | Shared: src/x86/devices/dma8237, mirrored test/x86/devices, x86 build/boundary rules and manifests. NXVM: devices/dma, its machine/board/scheduler callers, DMA/FDC/HDC and transaction tests, product build/guards, task records and eight affected EXEs. Public chip API exposes bounded registers/signals/cycles, never board pointers or full private state. |
| Applicable Rules | Guide-selected architecture, coding, documentation, execution and source-policy authorities. One state owner, Types-only chip dependency, no private cross-component tests, git mv for relocation, target-separated P commits, immediate push, all affected artifact receivers reviewed. No exception. |
| Verification | Independent chip tests without App/Common or external assets; full NXVM unit suites on x64/x86; full default integration and one boot per remaining profile/width; static aggregate, six manifests, documentation/link checks and actual-diff review. Reuse build/t539-s3 NXVM trees and build/t539-s7 standalone chip trees while S7 is active; test each boot row once, no repeated matrix rounds. |
| Expected Markers | Single chip implementation, no Shared App includes/page registers/peer pointers, no remaining App private-chip reads; preserved register, normal/TM, M2M, terminal/autoinit and 126 first-service cases. All required suites pass; eight stripped 0539 PE artifacts have recorded identity/architecture/hash. |
| Asset Needs | Existing owner-managed external firmware/media only, unchanged INIs. No downloads, protected binary commits or new asset definitions. |
| Reporting Requirements | Report concrete boundary findings, test counts and failures, changed files/ABI, test disposition, artifact hashes and commit/push receipts. Distinguish intermediate work from accepted runnable evidence. |
| Stop Conditions | New unsupported behavioral or licensing decision, irreconcilable hardware semantics, or out-of-scope consumer change requires escalation. Ordinary extraction/build failures are repaired within this S, not reported as completion. |
| Exit Criteria | Entire DMA ledger row implemented, original path deleted, all callers and behavioral tests migrated, construction rollback correct, verification complete, Shared/NXVM P commits pushed and actual-change review accepted. T539 remains open for its remaining inventory. |
| Original Owner Request | Independently decouple existing chips into src/x86/devices; retain only board integration in NXVM; automatically admit every S without repeated approval. |
| Similar-Issue Sweep | Search all src/test/cmake uses of t_dma, VDMA_, DMA private fields, accelerated advance and provider bindings; classify chip versus board responsibility, migrate all production/test callers, and enforce the Shared dependency boundary. Previously fixed secondary first-transfer bypass must not recur. |

The [proposal](../proposals/m5-shared-chip-extraction.md),
[contracts](../etc/architecture/t539-boundary-contracts.md),
[finite ledger](../etc/evidence/t539-chip-migration-ledger.md) and
[task history](../history/M5-T539-independent-shared-chips.md) retain scope,
decisions and acceptance. Automatic admission does not waive evidence,
target-separated commits, regression testing or coordinator review.

M5 Td S174 queued the three-stage migration. Its first candidate is now T539;
the seven remaining [Queue](QUEUE.md) candidates retain their dependency order.

## Current Technical Baseline

Four fixed products remain XT, AT, Model 40 and default PC/AT; PC110 is not
runnable. Eight optimized, compiler-debug-stripped 0539 EXEs are current with
unchanged owner INIs. MyNES retains its two unchanged 0043 receivers; its T43
remains closed.

Lib/Common retain the accepted 268464d49 baseline. Shared x86 PIT is accepted
at 24162ac93, RTC at 8a8435648 and PIC at d6dc6ca3a. Current NXVM source/artifacts
are 0746220bf, including the first-service repair and eight rebuilt artifacts. [S6 evidence](../etc/evidence/t539-s6-dma-first-service.md)
owns current verification/hashes; [S5 evidence](../etc/evidence/t539-s5-pic-extraction.md)
retains PIC source mapping and transaction rollback. Full sibling parity is
not claimed; no sibling repository was modified.

Verification: NXVM 341/341 units and 20/20 default-profile external integration
per width; all six non-default profile/width boot matrices pass once.
S6's 126 DMA first-service cases pass within the unit suite. Independent chip
suites remain S5's 12/12 evidence, not a new standalone S6 run. The specialized
static aggregate and six manifests pass. MyNES has no artifact input change.
The S5 temporary build trees are removed; the two S3 NXVM incremental trees
remain for the immediately next chip batch. Cooked-history rollback debt
remains in [TODO](TODO.md). This bounded regression acceptance does not claim
complete hardware qualification or indefinite absence of intermittent faults.

## Historical Context

[Earlier status](../history/M6-T41-and-prior-status-archive.md) preserves prior
deliveries. [T538 history](../history/M5-T538-deployed-boot-pairs.md) preserves
the earlier boot qualification and final accepted single-pass matrix.
