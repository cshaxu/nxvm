# Td S107 DeskPro Timing Queue Reconciliation

`M5:TD:S107:DESKPRO-TIMING-QUEUE:OK`

## Decision

The completed tiered-fallback foundation is removed from the active Queue and
retained as historical planning context. Td S105 already met its completion
standard; T406 and T407 are also closed historical mechanisms, not "next"
work. The DeskPro physical-cycle and phase-timing closure is now the first M5
candidate.

## Ordered Program

The promoted proposal orders: (1) CPU-to-board transaction/BWAIT/availability
waits/retirement, (2) device DRQ/IRQ/NMI phase and arbitration, (3) the selected
Compaq 40 MB WD/fixed-disk controller, (4) CECG/display/monitor behavior, (5)
firmware-visible BYOB lifecycle, and (6) an independent physical/L3 re-audit.
Each receiver must carry one `original`, `reference-derived`, or `generic-at`
label. The final audit consumes those labels and may reject physical/L3
acceptance; it is not satisfied merely by closing the preceding mechanisms.

## Boundaries

The first admission target is CPU-to-board timing. It must build a real
Core-owned external transaction lifecycle before applying D4's documented
memory-page waits or binding BWAIT/retirement. The fixed-disk item explicitly
forbids presenting generic AT IDE/HDC behavior as the Compaq WD controller.
The BYOB item explicitly forbids ROM bytes, paths, catalogues and default
firmware dependencies. This governance change modifies no executable behavior,
asset, firmware, media, source provenance or L3 status.