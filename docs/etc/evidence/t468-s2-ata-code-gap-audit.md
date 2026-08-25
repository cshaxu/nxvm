# T468 S2 ATA/HDC Code-Gap Audit

## Method And Owner Trace

This audit consumes the accepted fifteen-row T468 S1 ledger. It traces the
generic `ATA_PIO` branch through `hdc.c`/`hdc.h`, `machine_board.c`,
`machine_plan.c`, the frozen media registry, PIC IRQ source and default-PC/AT
composition. `CORE_MACHINE_HDC_PROTOCOL_COMPAQ_WD_40MB` is inspected only to
exclude it: no ATA repair may change that selected non-ATA branch.

One construction path copies `core_machine_hdc_config` through the plan into
one HDC owner. One HDC connects one frozen media registry and one PIC IRQ
source; it is the sole task-file, phase and IRQ publication path. The focused
Core and VM HDC smokes exercise command capture, Status versus Alternate
Status, DRQ PIO, multi-sector progress, SRST, nIEN, errors and media changes.
They did not test ATA's busy/DRQ task-file restriction, pending-IRQ restoration
after nIEN, reset signature or write-without-prior-read behavior.

## Checklist 2

| ID | Current path and proof | Gap or exact disposition | Owner |
| --- | --- | --- | --- |
| ATA-R1 | `hdc.c` is the only command/control port provider; `machine_board.c` installs it once. `core-machine-hdc-smoke` and `vm-hdc-port-smoke` cover the task-file shape. | **S3:** generic ATA accepts all command-block writes while BSY/DRQ is set. ATA-3 and 86Box gate them. | `hdc.c` only. |
| ATA-R2 | Status read deasserts the single PIC source; Alternate Status only observes. Both focused HDC smokes prove this. | **S3:** nIEN currently destroys the interrupt condition, so clearing nIEN cannot restore a condition that completed while masked. | `hdc.c` plus existing PIC source. |
| ATA-R3 | One 512-byte buffer and 16-bit Data port serve PIO read/write; phase gates reject inactive Data access. Core/VM HDC smokes cover it. | Manual-L3 logical PIO shape is present. IORDY/DMACK and bus-cycle duration are not claimed here. | Existing `hdc.c`; timing is ATA-F4/F5. |
| ATA-R4 | Command write snapshots parameters, clears a visible interrupt and advances through one pending phase; supported generic commands are read/write/IDENTIFY and others ABRT. | **S3:** busy/DRQ command writes must not replace the active command. The support subset itself is selected-device L2, not a reason to invent commands. | `hdc.c` only. |
| ATA-R5 | IDENTIFY prepares 256 words through the same PIO/DRQ/IRQ route; Core/VM HDC smokes consume it. | Transfer shape is present. Exact model/capability/translation words remain selected-device L2. | Existing HDC plus future device contract. |
| ATA-F1 | Core models logical ports and PIC, not cable signals. | L2: no selected bridge/cable signal contract. | Future selected controller/bridge admission. |
| ATA-F2 | No HDC DMARQ/DMACK path; default descriptor explicitly has no DMA channel. | L2: no selected board DMA/arbitration contract or receiver. | Future board/controller admission. |
| ATA-F3 | SRST sets BSY, cancels the local phase and reset reuses the sole owner. | **S3:** ATA reset signature is not restored (`Sector Count=01h`, `Sector Number=01h`, diagnostic state); nIEN visibility also shares this reset/IRQ path. Physical delay remains L2. | `hdc.c` only. |
| ATA-F4 | Command -> pending -> DRQ/data -> pending-sector uses the shared Core timeline; HDC refresh has no parallel state. | Logical ordering is present. Exact PIO/IORDY wait conversion is L2 because no timing input/receiver is present. | Future L3 integration contract. |
| ATA-F5 | No ATA mode/timing table is copied into the profile or Core. | L2: controller/cable/device timing values and conversion are unselected. | Future selected timing contract. |
| ATA-T1 | Immutable plan configuration accepts all ports and IRQ and passes them to the sole HDC owner; Core controller-authority and VM HDC smokes prove construction. | Board-L3-capable receiver exists. Current default 1F0--1F7/3F6/IRQ14 values remain Current-data L2 until source-qualified. | Existing plan -> `hdc.c` path. |
| ATA-T2 | Topology carries master and optional slave media IDs; `hdc.c` selects by DEV with no second media owner. | Board-L3-capable binding exists. Presence/diagnostics/identity policy remains selected-device L2. | Existing topology/media path. |
| ATA-T3 | Frozen registry is queried for each command/sector and reports absence/read-only/range through the one HDC error path; media/HDC smokes cover changes. | **S4:** PIO data-out wrongly calls the read path merely to validate its target; a writable logical target must not require a prior read. Image grammar/persistence stays L2. | `hdc.c` with existing registry. |
| ATA-T4 | `core_machine_hdc_advance` is timeline-driven; PIC and media are reused, and no HDC scheduler/DMA owner exists. | L2 only for unprovided duration/deadline data; no ownership defect. | Future L3 integration contract. |
| ATA-T5 | Generic ATA and Compaq WD are explicit protocol branches; Model-40 uses the latter. | Conforming boundary: no ATA label or fallback is introduced. | No change. |

## Finite Repair Plan

S3 changes only `hdc.c` and focused HDC tests: share the generic-ATA
task-file-writable predicate, preserve a pending interrupt condition across
nIEN and synchronize it with the existing PIC source, and restore the
source-defined logical reset signature. S4 factors the currently duplicated
logical-sector validation so Data-Out validates/writes without an unnecessary
read; it adds a single forced-read-failure/write-success regression. Neither
subtask selects a controller, drive model, image grammar, DMA path or timing
number.
