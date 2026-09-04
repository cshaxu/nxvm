# M5 PIT8253-8254 Manual And Board-Integration Reclosure

Re-audit counter modes, gates, latches, read-back where supported, clock
formulas, output transitions and reset behaviour against Intel material.  Prove
PIT0 -> IRQ0, PIT1 -> refresh/DMA and PIT2 -> speaker/port-61 board consumers through
one Core clock/deadline route on every affected profile.  No synthetic port-61
or BIOS behaviour may replace the actual signal route.  Apply the shared
[controller reclosure program](m5-controller-board-integration-reclosure-program.md).

Subtasks: List-1/List-2 reconciliation; owner/wiring repair; cross-profile ROM
and complete-gate closure.
