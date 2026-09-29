# Project Status

## Current Work

M5 T539 remains open. S1-S18 are accepted. S19 is active under the owner's
automatic-S authorization; S20-S32 remain planned in the
[CPU work packages](../etc/architecture/t539-cpu-work-packages.md).

S18 implementation P1 `0067d80c4` recovers the green incremental baseline.
Coordinator actual-commit review accepts the amended baseline scope, not the
CPU extraction row. Full unit logs pass 370/370 per width; default integration
passes 20/20 per width; tools-off standalone passes 45/45; six vendor boots pass
once; eight rebuilt 0539 artifacts and unchanged INIs are verified. Specialized,
manifest, documentation and diff checks pass.

The [S18 evidence](../etc/evidence/t539-s18-cpu-extraction.md) records review,
verification and counted changes. The
[inventory](../etc/evidence/t539-cpu-incremental-inventory.md) assigns remaining
private consumers and preserved deferred edits to S19-S32. Embedded CPU lifetime
remains until S30; relocation remains S31; final CPU acceptance remains S32.
S24 must resolve the recorded 32-bit BOUND observation. None is silently closed.

### S19 Active Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T539 S19; sole active NXVM S. |
| Admission And Approval | Owner automatic-S authorization and approved S18-S32 decomposition; coordinator admits S19 at 193fd1962. |
| Objective | Complete and qualify the CPU bus boundary retained by S18: memory/I/O, INTA, external-cycle ordering and partial effects; prevent reintroduction of private board dependencies or the unused firmware interception path. |
| Non-goals | No opcode/timing algorithm changes, opaque allocation cutover, Shared relocation, new device framework, MyNES or INI changes. |
| Reference Baseline | Clean master/origin master 193fd1962; accepted S18 implementation 0067d80c4 and eight 0539 EXEs. |
| Candidate Proposal | [Chip proposal](../proposals/m5-shared-chip-extraction.md), [CPU boundary](../etc/architecture/t539-s18-cpu-extraction.md), [work packages](../etc/architecture/t539-cpu-work-packages.md), [inventory](../etc/evidence/t539-cpu-incremental-inventory.md) and [chip ledger](../etc/evidence/t539-chip-migration-ledger.md). |
| Files And ABI Surface | NXVM only: CPU bus/provider and instruction external-cycle call sites; existing transaction/CPU/PIC/observation tests and fixtures; CPU boundary gate and its CMake registration; task evidence/status. No new public capability unless a concrete in-scope boundary defect requires it. |
| Applicable Rules | EXECUTION complete P and coordinator review; DOCUMENT authority; ARCHITECTURE sole owner and neutral bus; CODING original handler style, Types and owner-local tests; architecture/coding skills. Existing source policy and embedded-artifact exception unchanged. |
| Verification | Audit all bus callback families and their callers against pre-migration baseline; test success, admission/transfer failure, cancellation and committed partial effects with 1/2/4-byte I/O, memory provenance, reset/observation reads, single/cascaded PIC and FPU command trace. Run both full NXVM unit suites, boundary/specialized/document gates and six manifests. Rebuild affected eight x64/x86 0539 artifacts only if executable inputs change; rerun affected integration if production behavior changes. |
| Expected Markers | CPU holds only borrowed neutral bus/context; board owns routing/transaction/PIC; observable order and failure side effects match baseline; no software-interrupt hook remains; complete unit suites green without relaxed assertions. |
| Asset Needs | Existing three build/t539-s3 trees and external BYOB inputs; no acquisition or asset/INI edits. |
| Reporting Requirements | Report real boundary/coverage gaps, repair at owner, map proof per callback; distinguish retained S18 implementation from new S19 changes and final extraction. |
| Stop Conditions | Unapproved timing/behavior loss, need for Shared/MyNES edits, new asset dependency or unresolvable architectural conflict. |
| Exit Criteria | Entire bus batch mapped to direct code/test evidence; missing regression and boundary checks completed; complete units and applicable gates pass; actual-diff review, counted changes, commit/push and coordinator acceptance complete. |
| Original Owner Request | Extract independent chips to x86/devices, preserve existing CPU families/style and board behavior; execute individually traceable S tasks automatically. |
| Similar-Issue Sweep | Every read/write/port transfer-completion/INTA/extension callback and external-cycle BEGIN/COMMIT/CANCEL path, including partial transfers and observation-only reads; all nine CPU files checked for board/private-peer dependencies. |

## Retained Progress

| Task | Progress |
| --- | --- |
| T539 S18 | Accepted: NXVM P1 0067d80c4 restores the full incremental baseline with bus/timing/observation separation, CPU-local regression coverage and complete pending-consumer inventory. CPU extraction remains open. |
| T539 S17 | Accepted: Shared P1 cadaf0990 and NXVM P2 f85888d3e make CPU preview/timing and display backing inspection side-effect-free through the existing memory resolver. Both widths pass 370/370 units and 20/20 default integrations; tools-off 45/45, six vendor boots once, six manifests, specialized/document gates and eight artifact identities pass. |

Coordinator actual-commit review accepts the observation prerequisite, not the
CPU extraction row. The sole memory route, explicit provider intent, local
video latch calculation, unchanged operational reads and CPU handler/timing
bodies, original tests and complete receiving proof were reviewed directly.
The missing new-fixture classification was corrected without weakening the
gate. Model40 passes both widths within 90 seconds using the existing
observer-free probe. No guest workaround or timing downgrade was introduced.

The [proposal](../proposals/m5-shared-chip-extraction.md),
[finite ledger](../etc/evidence/t539-chip-migration-ledger.md),
[S17 boundary](../etc/architecture/t539-s17-cpu-observation.md),
[S17 evidence](../etc/evidence/t539-s17-cpu-observation.md) and
[task history](../history/M5-T539-independent-shared-chips.md) retain scope,
requirement-to-proof mapping, limits and delivery.
CPU and all remaining inventory dispositions still belong to T539; none is
silently transferred to the queued board-integration task.

## Current Technical Baseline

Four fixed products remain XT, AT, Model40 and default PC/AT; PC110 is not
runnable. Eight optimized compiler-debug-stripped 0539 EXEs are committed in
0067d80c4 with unchanged owner INIs. S18 evidence records hashes, PE architecture
and verification limits. Both reusable NXVM build trees are restored to default;
the three bounded build/t539-s3 trees remain needed for the next chip batch.

Lib/Common retain 268464d49. Shared x86 PIT is accepted at 24162ac93, RTC at
8a8435648, PIC at d6dc6ca3a, DMA at 53b4be21d, AT keyboard at eb1e2e208,
XT PPI/keyboard at 0f9c6b1a8, FDC at 1d6dc5876, HDC at 9020d8bba, video at
cadaf0990 and FPU at 5fa831a2b. MyNES retains its unchanged 0043 pair: its link
inputs do not include these x86 chip targets.

The owner-approved S12 firmware route remains: project BIOS source lives in
app-nxvm/firmware; commercial originals stay external BYOB; selected ROM bytes
are embedded into the committed EXEs. No runtime ROM-file fallback, host BIOS
service, external-master or owner-INI change is introduced by S18.

The seven [Queue](QUEUE.md) candidates retain dependency order. Cooked-history
rollback debt remains in [TODO](TODO.md). Acceptance does not claim indefinite
absence of intermittent faults.

## Historical Context

[Earlier status](../history/M6-T41-and-prior-status-archive.md) preserves prior
deliveries. [T538 history](../history/M5-T538-deployed-boot-pairs.md) retains
earlier boot qualification. [S12 evidence](../etc/evidence/t539-s12-fdc-drive-status.md)
owns preceding FDC qualification and embedded-firmware recovery.
