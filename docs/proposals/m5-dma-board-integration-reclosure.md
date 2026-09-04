# M5 DMA8237A Manual And Board-Integration Reclosure

Re-audit 8237A command, mode, arbitration, DREQ/DACK, HRQ/HLDA, terminal count,
page registers, reset and refresh semantics against original material.  Prove
the wired chains used by the supported boards, especially PIT1 -> DREQ0 -> DMA
refresh and FDC/HDC DMA transfers, through the sole Core transaction/deadline
path.  Board ratios or wiring may be labelled L2 but must be frozen inputs,
not profile-side DMA state.  Apply the shared
[controller reclosure program](m5-controller-board-integration-reclosure-program.md).

Subtasks: List-1/List-2 reconciliation; owner/wiring repair; cross-profile ROM
and complete-gate closure.
