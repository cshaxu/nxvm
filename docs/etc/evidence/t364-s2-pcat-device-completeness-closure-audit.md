# T364 S2: PC/AT Device Completeness Closure Audit

## Decision

T364 closes the selected-PC/AT device, chip, port, route and timing-owner
completeness audit. It does not close model-L3 timing: the audit establishes
the bounded hardware inventory required for later NMI, bus-availability and
device-service work.

## Independent reconciliation

The default profile's exact leaf table and named PIT/KBC/CMOS/FDC routes,
separate ATA PIO descriptor, and VM composition configuration paths match the
S1 ledger. Core initialization supplies the corresponding port, memory, PIC,
PIT, KBC, VADP, DMA, RTC, FDC, HDC, transaction, trace, timeline and clock
owners. The selected event order remains `DMA -> PIT -> PIC -> FDC -> ATA ->
RTC -> KBC -> VADP`; no parallel timing or device route was found.

| Audit class | Result |
| --- | --- |
| Selected ports/routes and mutable lifecycle | Complete: every selected leaf and IRQ/DRQ route has one profile/composition and core owner. |
| Memory/ROM/A20, PIC/PIT/RTC, DMA, KBC/AUX, VADP, FDC/ATA | Complete selected inventory with existing source/proof records and reset/timeline owners. |
| PPI/speaker, parity/I/O-check NMI, serial/parallel/game | Explicit absent/incomplete surfaces retained at their TODO or next-Queue receiver; no placeholder provider exists. |
| Wait states, HOLD/HLDA, INTA, prefetch/cache, pin phase and device duration | Explicit bus/service receivers; deterministic callback order is not represented as a duration. |

## Verification

The audit swept profile leaves/routes and ATA PIO descriptor, VM composition
configuration, all core device initialization/configuration paths, clock and
timeline owners, retained T346--T354 evidence, Queue and TODO. Documentation
governance and `git diff --check` passed. No runnable path changed and no
artifact is created. The next Queue task is PC/AT NMI source ownership; it
must select one documented board producer before implementation.
