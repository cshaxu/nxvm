# T399 S3 B2 Acceptance Audit

The coordinator reviewed the active S3 packet, the actual worktree, the
reference-derived ledger, PCjs exact-configuration limitation, D3PE topology,
and the existing Core/VM owner split. B2 has a complete project-owned,
asset-free observation path: the Model-40 test selects the 1.2 MB FDC/DMA2/IRQ6
route, while Core FDC tests cover the shared command, cancellation and reset
mechanism. No behavior change was justified; the exploratory assertion was not
retained because it conflated the controller's separate reset/terminal
confirmation lifecycle with the B2 transfer contract.

All three focused CTests pass, and documentation governance passes. B2 is
accepted as `M5:T399:S3:B2-CONTRACT-RECONCILED` with PCjs-only behavioral
confidence and no qualified second reference. Physical command-to-DRQ/IRQ,
DACK, media and board-time facts remain at the existing DeskPro
physical-observable device-timing TODO receiver. This acceptance does not
advance physical L3.