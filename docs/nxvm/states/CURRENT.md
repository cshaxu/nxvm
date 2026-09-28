# Project Status

## Current Work

M5 T539 remains open. S7's DMA extraction is accepted after actual-change
review. S8 is automatically admitted for the AT keyboard/controller chain,
following the approved dependency order.
The owner's 2026-09-28 automatic-S authorization remains in force; no further
manual admission is required within the approved scope.

| Task | Progress |
| --- | --- |
| T539 S8 | S1-S7 closed. AT keyboard/controller extraction is active; PIT, RTC, PIC and DMA are accepted. XT PPI/keyboard, FDC, HDC, video, CPU/FPU and final finite-ledger review remain; none is implicitly accepted. |

## Active S8 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, M5 T539 S8; one session performs coordinator and executor roles in sequence. |
| Admission And Approval | Owner's 2026-09-28 automatic-S authorization admits this bounded Shared/NXVM extraction. No new behavior grade, other-product change or rules exception is authorized. |
| Objective | Complete the finite ledger's kbc.c/h batch: separate qualified AT-controller transport, attached keyboard and AUX state; reconnect NXVM board ports, IRQ, A20/reset and time inputs without duplicate implementations. |
| Non-goals | No generic 8042 MCU, new keyboard protocol, XT extraction, BIOS compatibility branch, host input-policy change, timing upgrade, MyNES change or sibling write. |
| Reference Baseline | 23b732201, clean S7 acceptance; eight 0539 products and unchanged owner INIs. |
| Candidate Proposal | [Chip extraction](../proposals/m5-shared-chip-extraction.md), [finite ledger](../etc/evidence/t539-chip-migration-ledger.md), [concrete contracts](../etc/architecture/t539-boundary-contracts.md) and [S8 boundary](../etc/architecture/t539-s8-kbc-boundary.md). |
| Files And ABI Surface | Shared src/x86/devices and mirrored test/x86, their standalone build/boundary/manifests; NXVM devices KBC adapter, construction/scheduler/input callers, related profile/unit/integration callers, build entries, documentation and eight 0539 EXEs. Opaque chip APIs and copied byte/signal contracts only. |
| Applicable Rules | Execution, Architecture, Coding and Document rules; NXVM Architecture/Coding and source policy; architecture-governance then coding-governance skills. Prove unique state owners, one output path, explicit borrowed-sink lifetime, failure rollback and Types-only standalone dependency. No imported source/ROM or source-policy exception. |
| Verification | Full repository unit suite x64/x86; independent extracted-device tests both widths; complete default integration both widths and each other profile/width boot once; specialized static aggregate, six manifests, documentation governance, Markdown links and diff check; inspect all eight optimized stripped artifact identities. |
| Expected Markers | No App header/private peer in extracted components, no old KBC mechanism in board adapter, no private chip-field assertions or diagnostic casts; preserved command/BAT/ACK/scan/typematic/AUX/IRQ/reset cases; green verification and eight current artifacts with unchanged INIs. |
| Asset Needs | Existing owner-managed external ROM/CMOS/media only, no downloads or asset-master writes; retain existing deployment directories. |
| Reporting Requirements | Confirm boundary; report discovered ownership/order defects before changing their contract; report test/artifact evidence and counted source/test added/deleted/net lines, separate Shared/NXVM commits and actual-diff acceptance. |
| Stop Conditions | New firmware/licensing need, unsupported hardware semantic change or downgrade, external writer conflict, or inability to preserve an existing receiver without an unreviewed alternate path. Ordinary design/implementation work is not a blocker. |
| Exit Criteria | Complete KBC ledger batch extracted and connected; every original case mapped to public-chip or board proof; no unresolved S8 defect or partial P delivery; all required verification, artifacts, self-review, pushed target-separated commits and coordinator actual-change review completed. T remains open. |
| Original Owner Request | Decouple each chip into Shared x86, keep machine wiring in NXVM, preserve behavior and remove old copies; automatically admit successive S tasks without further manual approval. |
| Similar-Issue Sweep | All KBC writes/reads, startup and explicit reset, controller/keyboard/AUX pending parameters, delayed replies and serial backlog, IRQ-origin transitions, typematic/deadline paths, construction teardown/failure, private production/test/diagnostic accesses across all four profiles. |

S8 implementation and verification are complete. Controller, keyboard and AUX
have separate opaque owners; the old private fields and board duplicates are
removed. Final full units pass 345/345 and independent chip suites 16/16 per
width; default integrations pass 20/20 per width. All six other profile/width
boots pass once. The specialized static aggregate, six manifests, documentation
governance, local links and diff check pass. Eight optimized stripped 0539
artifacts are rebuilt with unchanged INI contents. The two incremental trees
are restored to default configuration.

Shared P1 eb1e2e208 is pushed. The NXVM receiver P2 and coordinator actual-change
acceptance are next; S8 is not yet closed. The
[S8 evidence](../etc/evidence/t539-s8-kbc-extraction.md) owns test mapping,
source/test counts, verification boundaries and all artifact hashes.

The [proposal](../proposals/m5-shared-chip-extraction.md),
[contracts](../etc/architecture/t539-boundary-contracts.md),
[finite ledger](../etc/evidence/t539-chip-migration-ledger.md) and
[task history](../history/M5-T539-independent-shared-chips.md) retain scope,
decisions and acceptance. Automatic admission does not waive boundary review,
target-separated commits, complete tests or coordinator actual-diff review.

M5 Td S174 queued the three-stage migration. Its first candidate is T539;
the seven remaining [Queue](QUEUE.md) candidates retain their dependency order.

## Current Technical Baseline

Four fixed products remain XT, AT, Model 40 and default PC/AT; PC110 is not
runnable. Eight optimized, compiler-debug-stripped 0539 EXEs are current with
unchanged owner INIs. MyNES retains its two unchanged 0043 receivers; its T43
remains closed.

Lib/Common retain the accepted 268464d49 baseline. Shared x86 PIT is accepted
at 24162ac93, RTC at 8a8435648, PIC at d6dc6ca3a and DMA at 53b4be21d. Current
NXVM source/artifacts are 217125697. [S7 evidence](../etc/evidence/t539-s7-dma-extraction.md)
owns verification/hashes and actual-change acceptance. [S6 evidence](../etc/evidence/t539-s6-dma-first-service.md)
retains the DMA first-service prerequisite; [S5 evidence](../etc/evidence/t539-s5-pic-extraction.md)
retains PIC mapping/rollback. Full sibling parity is not claimed; no sibling
repository was modified.

Verification: final NXVM units 342/342 per width, default-profile external
integration 20/20 per width, independent chip suites 13/13 per width. All six
remaining profile/width boot rows pass once. The specialized static aggregate,
six manifests, documentation governance and diff checks pass. MyNES has no
artifact input change. The temporary S7 standalone chip build trees are removed;
the two S3 NXVM incremental trees remain, restored to default configuration,
for the immediately next chip batch. Cooked-history rollback debt remains in
[TODO](TODO.md). This bounded acceptance does not claim complete hardware
qualification or indefinite absence of intermittent faults.

## Historical Context

[Earlier status](../history/M6-T41-and-prior-status-archive.md) preserves prior
deliveries. [T538 history](../history/M5-T538-deployed-boot-pairs.md) preserves
the earlier boot qualification and final accepted single-pass matrix.
