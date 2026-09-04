# M5 PIC8259A Manual And Board-Integration Reclosure

Re-audit the 8259A command, mask, priority, cascade, acknowledge, EOI and
spurious-interrupt ledger against Intel material; reconcile it with the sole
PIC state owner and all selected board IRQ wiring.  Prove end-to-end delivery
from PIT/DMA/KBC/FDC/HDC producers through PIC acknowledgement to CPU/ROM
observation across 5160, 5170, default AT and Model 40.  Do not add device- or
BIOS-specific IRQ shortcuts.  Apply the shared
[controller reclosure program](m5-controller-board-integration-reclosure-program.md).

Subtasks: List-1/List-2 reconciliation; source-backed owner/signal repair;
cross-profile ROM and complete-gate closure.
