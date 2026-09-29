# Project Status

## Current Work

M5 T539 remains open. S1-S20 are accepted. There is no active S packet.
S21-S32 remain planned under the owner's automatic-S authorization in the
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

S19 implementation `f1b43af46` qualifies the retained CPU bus boundary with
memory/port failure matrices, cascaded INTA and FPU trace checks, plus 72
negative boundary controls. Both complete unit suites pass 371/371; all 66
specialized steps, six manifests and documentation checks pass. Coordinator
actual-commit review accepts S19; no production or executable input changed.
See [S19 evidence](../etc/evidence/t539-s19-cpu-bus-boundary.md). S20 is next;
opaque lifetime and Shared relocation remain S30/S31, not accepted here.

S20 implementation `af06a6259` qualifies copied CPU observation and board
adapters, removes an unnecessary test-private include and adds prevention of
board private-state access. Both widths pass 371/371 units; 66 specialized
steps, six manifests and documentation pass. Coordinator actual-commit review
accepts S20; production and executable inputs are unchanged. See
[S20 evidence](../etc/evidence/t539-s20-cpu-observation-adapters.md).
S21 is next: data movement/addressing/stack test migration. S30/S31 remain the
embedded-lifetime and Shared-relocation receivers.

## Retained Progress

| Task | Progress |
| --- | --- |
| T539 S20 | Accepted: NXVM P1 af06a6259 qualifies copied observations, debug/reset adapters and board access boundary. Full units 371/371 per width; no production or artifact changes. |
| T539 S19 | Accepted: NXVM P1 f1b43af46 verifies CPU bus transaction ownership, failure effects and imports. No production or artifact change; S20 is next. |
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
