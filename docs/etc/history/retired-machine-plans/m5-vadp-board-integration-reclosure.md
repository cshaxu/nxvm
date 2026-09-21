# M5 VADP Video-Adapter And Board-Integration Reclosure

> Retained historical proposal; superseded by the fixed Standard/PC110 product
> direction. Not an active candidate or authority. See [disposition](README.md).

Re-audit the selected CGA/EGA adapter registers, memory windows, CRTC geometry,
display enable, text/planar snapshots, video ROM interface and timing facts.
Prove guest port/memory writes flow through the sole VADP state owner to copied
display snapshots and presentation across the selected profiles.  CGA, EGA and
future VGA extensions retain one VADP route, not parallel mode/frame owners.
Apply the shared [controller reclosure program](m5-controller-board-integration-reclosure-program.md).

Subtasks: List-1/List-2 reconciliation; sole-state/snapshot/board repair;
cross-profile ROM and complete-gate closure.
