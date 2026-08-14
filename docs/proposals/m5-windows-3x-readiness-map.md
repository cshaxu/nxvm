# Windows 3.x Readiness Map

Record the latest bounded checkpoint and every remaining CPU, device, timing,
profile, and media prerequisite; do not make a Windows 95 claim.

## T346 Handoff Baseline

T346 provides the deterministic PC/AT L3 foundation consumed by this candidate:
one reset-safe due-event owner ordered by `(due_tick, sequence)`, copied
transaction checkpoints, and equal-tick ordering
`DMA -> PIT -> PIC -> FDC -> HDC -> RTC -> KBC -> VADP`. The host input and
presentation crossings remain copied boundaries and never supply guest time.
This is not cycle-exact timing and does not include a Windows installation or
driver corpus.

## Evidenced Remaining Blockers

| Boundary | Why it blocks a readiness conclusion | Admission condition |
| --- | --- | --- |
| FDC/ATA command-service regression | T347 already moves retained FDC/ATA issue and final-sector publication through pending states and the deterministic readiness owner; a readiness map must verify this baseline remains intact for the selected media path. | Re-run the accepted pending/busy, abort/reset, media-result, DRQ/DMA/IRQ, FDC read-track, and ATA PIO consumer proofs; any regression returns to the storage owner rather than becoming a new readiness feature. |
| PC/AT NMI sources | CMOS exposes only the existing NMI mask; parity/I/O-channel producers are absent. | Select a PC/AT source and prove mask/assert/deassert/reset plus protected/VM86 delivery on the timeline. |
| Windows display/input corpus | VADP/KBC timing ownership is deterministic, but no Windows driver or setup corpus selects the needed video/AUX surface. | Owner-supplied, legally handled Windows corpus with declared display, keyboard, and optional mouse checkpoint. |
| Digital video breadth | The retained CGA/EGA modes are bounded; VGA/VBE and remaining digital CGA behavior are not claimed. | Corpus-selected mode/CRTC contract with core-owned state and copied-frame evidence. |
| Guest media and installation | No Windows image or installation was introduced or run. | BYOB/provenance decision plus a bounded installation/startup checkpoint and captured prerequisite failures. |

CPU T343 is an accepted four-profile form/state closure. Numerical x87
execution, VME/PVI, persistent cache/TLB behavior, broad device parity, and
product compatibility remain outside that conclusion. This candidate consumes
those explicit boundaries; it cannot reinterpret a passing CPU or device smoke
as Windows compatibility evidence.

## Proposed Subtask Sequence

| Subtask | Bounded outcome |
| --- | --- |
| S1 | Audit the existing checkpoint/probe sources, CMake media routing, permitted local-media record, CPU/profile/device/L3 prerequisites, and every probe's host-time or provenance boundary. Produce a single readiness ledger and ordered receivers; do not execute Setup or change runtime behavior. |
| S2 | Establish one explicit BYOB checkpoint command for the retained HDD/INT13 consumer. It must reject missing local configuration before media access and remain outside current-gate; it does not run Setup or alter core timing. |
| S3 | Re-run only the approved bounded checkpoint that consumes S2, classify the first remaining block or stable checkpoint, and transfer every non-selected dependency precisely. |
| S4 | Close the map by reconciling the ledger, media/provenance boundary, retained current-gate consumers, and deferred device/product work. It may not claim installation, Standard Mode, Enhanced Mode, or broad compatibility unless separately admitted. |

S1 treats existing Windows probes as evidence artifacts, not as an authority to
add or distribute Microsoft media.  Any later probe execution must use the
owner-approved local configuration and may report only its declared checkpoint.
