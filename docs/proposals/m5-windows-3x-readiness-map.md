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
| FDC/ATA command-service timing | Current command/sector service can publish DRQ/DMA/IRQ synchronously, so installer media readiness is not yet evidenced. | One primary controller contract plus deterministic pending, completion, abort/reset, media-result, and DRQ/DMA/IRQ probes. |
| PC/AT NMI sources | CMOS exposes only the existing NMI mask; parity/I/O-channel producers are absent. | Select a PC/AT source and prove mask/assert/deassert/reset plus protected/VM86 delivery on the timeline. |
| Windows display/input corpus | VADP/KBC timing ownership is deterministic, but no Windows driver or setup corpus selects the needed video/AUX surface. | Owner-supplied, legally handled Windows corpus with declared display, keyboard, and optional mouse checkpoint. |
| Digital video breadth | The retained CGA/EGA modes are bounded; VGA/VBE and remaining digital CGA behavior are not claimed. | Corpus-selected mode/CRTC contract with core-owned state and copied-frame evidence. |
| Guest media and installation | No Windows image or installation was introduced or run. | BYOB/provenance decision plus a bounded installation/startup checkpoint and captured prerequisite failures. |

CPU T343 is an accepted four-profile form/state closure. Numerical x87
execution, VME/PVI, persistent cache/TLB behavior, broad device parity, and
product compatibility remain outside that conclusion. This candidate consumes
those explicit boundaries; it cannot reinterpret a passing CPU or device smoke
as Windows compatibility evidence.
