# M5 Fixed-Disk Controller And Board-Integration Reclosure

Re-audit each selected HDC personality separately: XT Xebec, IBM 5170
WD1003/ST-506, DeskPro Compaq/WD, default-AT ATA and selected ESDI where
admitted.  List command phases, DRQ/IRQ, media CHS interaction, reset and
service deadlines from primary materials; use external implementations only as
labelled L2 corroboration.  Prove firmware PIO/DMA consumers wait on the sole
Core state/deadline route rather than assuming command issue means completion.
Apply the shared [controller reclosure program](m5-controller-board-integration-reclosure-program.md).

Subtasks: personality List-1/List-2 reconciliation; sole HDC/media/deadline
repair; profile ROM/media and complete-gate closure.
