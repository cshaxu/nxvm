# T450 S18 ATA/HDC Current-Code Gap Audit

## Scope And Method

This audit compares the frozen S17 ATA-3 universe with the sole
`core/machine/hdc` owner, frozen Core media registry, PIC/scheduler bindings,
VM composition/profile and focused HDC tests.  It makes no runtime, test,
asset or reference change.  The current HDC provides one task-file-like port
path and PIO sector buffer for generic and Compaq WD 40-MB personalities; it
is not a selected ATA controller.  Every partial, missing or blocked row
transfers exactly once to the **M5 HDC/ATA Phase Contract** receiver, which
must extend existing owners rather than add a second controller, media
registry, command parser, IRQ route or scheduler.

## Row Dispositions

| S17 ID | Current owner and observed behavior | Focused proof | Disposition and sole receiver |
| --- | --- | --- |
| ATA-R1 | `hdc.c` owns data, feature/error, sector, cylinder, drive/head, status/command and alternate-status/device-control ports; profile submits 1F0--1F7/3F6. | `core-machine-hdc-smoke`, `vm-hdc-port-smoke`. | Partial: task-file shape exists, not selected ATA personality proof. |
| ATA-R2 | Status has BSY/DRDY/DSC/DRQ/ERR-like state; Error is read at feature port; Status read clears IRQ while Alternate Status does not. | Core and VM HDC port smokes. | Partial: values/timing are not full ATA proof. |
| ATA-R3 | Data port transfers 16-bit words through one 512-byte PIO buffer; DRQ and Core media callbacks control multi-sector phase changes. | Core and VM HDC port smokes. | Partial: ATA IORDY/PIO protocol timing absent. |
| ATA-R4 | Dispatch supports read/write/Identify and selected generic/Compaq diagnostics, initialize, verify, recalibrate and seek; other forms abort. | Core HDC, Compaq HDC and VM HDC smokes. | Partial: ATA-3 command/capability set is unselected/incomplete. |
| ATA-R5 | Identify creates project-defined 512-byte data from current media/configuration, not a selected ATA device identity. | Core HDC smoke. | Missing selected device IDENTIFY words. |
| ATA-F1 | HDC models ports and an IRQ source, not ATA cable/device signals (DIOR-/DIOW-/CS/DA/DD/RESET/DASP/PDIAG/IORDY). | Port smokes prove logical port operations only. | Missing interface signal model; receiver needs selected controller/bridge facts. |
| ATA-F2 | Profile declares no DMA channel; HDC has no DMARQ/DMACK route and is PIO only; existing PIC source carries IRQ14. | `vm-hdc-port-smoke`. | Missing ATA DMA; receiver only after board/controller selection. |
| ATA-F3 | SRST asserts idle/BSY then resets on release; nIEN clears/suppresses current IRQ.  ATA reset/power timing values are not derived. | Core and VM HDC smokes. | Partial reset shape; receiver owns selected timing/device behavior. |
| ATA-F4 | Command capture -> pending -> `core_machine_hdc_advance` -> PIO DRQ is deterministic, without DIOR-/DIOW-/IORDY setup/hold/recovery. | Core HDC smoke. | Missing ATA PIO timing conversion; receiver owns one phase contract. |
| ATA-F5 | Scheduler calls logical HDC advance; no ATA timing tables or transfer modes are represented. | Direct-advance tests; no timing-table proof. | Missing selected ATA timing contract; receiver only. |
| ATA-T1 | Board/profile define generic PC/AT ports/IRQ14 and a Compaq WD personality, neither a selected ATA controller decode. | VM HDC and Model-40 HDC smokes. | Blocked selected ATA controller source; receiver only. |
| ATA-T2 | Config can bind media IDs and current personality rules; current generic profile has no slave/secondary route. | VM and Core HDC smokes. | Blocked selected ATA device/master-slave policy. |
| ATA-T3 | Core registry supplies frozen logical sectors/errors; VM supplies current backing images, not a selected ATA media grammar/persistence contract. | Core/VM media provider and HDC smokes. | Blocked selected backing-media contract. |
| ATA-T4 | HDC reuses scheduler, PIC and media owners; it creates no second DMA/arbitration/IRQ path. | Source review and HDC smokes. | Current ownership conforming; board phase transfers once. |
| ATA-T5 | Frozen profiles retain generic and Compaq WD 40-MB personalities without ATA conformance claim. | Compaq HDC and Model-40 HDC smokes. | Current boundary conforming; ATA implementation blocked. |

## Completeness, Minimality And Transfer

All `ATA-R1`--`ATA-R5`, `ATA-F1`--`ATA-F5` and `ATA-T1`--`ATA-T5` occur once.
The existing task-file/PIO/media/IRQ path remains the only owner path.  This
audit neither relabels compatible ports as ATA nor proposes a parallel
interface; selected controller/device/backing-media facts must precede ATA
implementation.  S18 changes documentation only, so P review requires
`git diff --check` and documentation governance, not a build.
