# Project Status

## Current Work

M5 T539 remains open. S1-S14 are accepted. S15 is automatically admitted
after the video ownership/dependency review.

| Task | Progress |
| --- | --- |
| T539 S14 | Accepted: Shared P1 9020d8bba and NXVM P2 86e0f82cb extract the sole opaque HDC family and reconnect board/media wiring. Units 358/358 and default integrations 20/20 per width; six vendor boots once; tools-off 25/25; six manifests, static/document gates and eight artifact hashes pass. |
| T539 S15 | Verification complete; coordinator delivery review pending. Shared P1 522d0b27f is pushed. One opaque x86-video owner and board-only VADP; dead presentation helpers removed. Final units 367/367 and default integrations 20/20 per width; all six vendor boots reach DOS5 installer once. Tools-off 43/43, six manifests and specialized gates pass. NXVM P2 delivers eight optimized EXEs and the receiver cutover; INIs unchanged. [Evidence](../etc/evidence/t539-s15-video-extraction.md) records case/diff review, rollback and hashes. |

## Active S15 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, M5 T539 S15; coordinator/executor roles in one session. |
| Admission And Approval | Owner automatic-S authorization dated 2026-09-28; approved Shared/NXVM extraction scope. Separate target commits; no sibling writes, external-master or owner-INI changes. |
| Objective | Consume video and related display ledger rows with one independent register/VRAM/frame-generation owner and real NXVM board adaptation. |
| Non-goals | No new display mode, geometry limit, silicon/timing upgrade, Common/Lib UX change, CPU/FPU migration or generic device framework. |
| Reference Baseline | Accepted 8831f05c5; Shared HDC 9020d8bba, NXVM source/artifacts 86e0f82cb. |
| Candidate Proposal | [T539](../proposals/m5-shared-chip-extraction.md), [finite ledger](../etc/evidence/t539-chip-migration-ledger.md), [S1 design](../etc/architecture/t539-independent-chip-design.md), [S15 boundary](../etc/architecture/t539-s15-video-extraction.md). |
| Files And ABI Surface | x86/devices/video and tests/build/manifests; NXVM VADP adapter, display values/provider split, construction/memory registration/scheduler, affected profiles/tests/diagnostics/gates and eight artifacts. Retire presentation_interface.c/h and their isolated smoke after confirmed absence of production consumers; retain the live guest frame/provider slot. Opaque instance, bounded register/memory/backing-read and copied frame/observation contracts. |
| Applicable Rules | Architecture/Coding rules and skills, product design/layout, Execution, Documentation, source policy. Single state/route owner; no private peer access; complete preparation/rollback and explicit receiver verification. |
| Verification | Full units x64/x86; standalone tools-off tests; original video-case map and construction fault/retry proof; default integrations both widths; six other profile/width boots once; eight optimized EXEs, PE/hash, six manifests, static/document/whitespace gates. MyNES link review, rebuild only if affected. |
| Expected Markers | Original text/CGA/EGA/VGA/Compaq register/memory/frame and boot predicates unchanged; independent test graph; no old chip implementation or private caller access. |
| Asset Needs | Existing embedded firmware and external media; code-owned unit fixtures. Reuse three bounded build/t539-s3 trees; no original or INI changes. |
| Reporting Requirements | Confirm boundary, complete original-case ownership and construction review, record actual diff/line counts and retained-path rationale, verification and artifacts. No partial implementation P. |
| Stop Conditions | New licensing/source need or product/public-contract expansion beyond this boundary; coordinator revises packet before implementation. |
| Exit Criteria | Complete video batch moves with one state owner; board-only roles explicitly retained; all original cases preserved, failures/lifetimes verified, obsolete paths removed, full checks and artifacts delivered/pushed, actual-change acceptance recorded. |
| Original Owner Request | Independent chips in x86/devices, board integration stays NXVM; preserve machines, remove duplicates and automatically admit subsequent S batches. |
| Similar-Issue Sweep | All video personalities, port aliases, CPU aperture versus frame geometry/output, backing-memory routes, capture/dirty/glyph state, construction/reset/destruction and every private diagnostic/test caller. |

Coordinator actual-commit review accepts the complete HDC ledger row: explicit
ATA/Compaq/WD1003/Xebec personalities, one state/sector-buffer owner, absolute
deadlines, output withdrawal, large-sector addressing, checked construction
and rollback, original-case migration and paused terminal diagnostics.
The review inspected actual source/test/build/document changes, not only green
checks. There is no new complete-silicon or timing-grade claim.

The [proposal](../proposals/m5-shared-chip-extraction.md),
[finite ledger](../etc/evidence/t539-chip-migration-ledger.md),
[S14 boundary](../etc/architecture/t539-s14-hdc-extraction.md),
[S14 evidence](../etc/evidence/t539-s14-hdc-extraction.md) and
[task history](../history/M5-T539-independent-shared-chips.md) retain scope,
requirement-to-proof mapping, code-size review and delivery.
CPU/FPU, video and all remaining inventory dispositions still belong to T539.
Their dependency/boundary review precedes each next automatic S admission;
none is silently transferred to the queued board-integration task.

## Current Technical Baseline

Four fixed products remain XT, AT, Model 40 and default PC/AT; PC110 is not
runnable. Eight optimized compiler-debug-stripped 0539 EXEs are committed in
86e0f82cb with unchanged owner INIs. S14 evidence records SHA-256, PE architecture
and verification limits. Both reusable NXVM build trees are restored to default;
the three bounded build/t539-s3 trees remain needed for the next chip batch.

Lib/Common retain 268464d49. Shared x86 PIT is accepted at 24162ac93, RTC at
8a8435648, PIC at d6dc6ca3a, DMA at 53b4be21d, AT keyboard at eb1e2e208,
XT PPI/keyboard at 0f9c6b1a8, FDC at 1d6dc5876 and HDC at 9020d8bba.
MyNES retains its unchanged 0043 pair: its link inputs do not include these
x86 chip targets.

The owner-approved S12 firmware route remains: project BIOS source lives in
app-nxvm/firmware; commercial originals stay external BYOB; selected ROM bytes
are embedded into the committed EXEs. No runtime ROM-file fallback, host BIOS
service, external-master or owner-INI change is introduced by S14.

The seven [Queue](QUEUE.md) candidates retain dependency order. Cooked-history
rollback debt remains in [TODO](TODO.md). Acceptance does not claim indefinite
absence of intermittent faults.

## Historical Context

[Earlier status](../history/M6-T41-and-prior-status-archive.md) preserves prior
deliveries. [T538 history](../history/M5-T538-deployed-boot-pairs.md) retains
earlier boot qualification. [S12 evidence](../etc/evidence/t539-s12-fdc-drive-status.md)
owns preceding FDC qualification and embedded-firmware recovery.
