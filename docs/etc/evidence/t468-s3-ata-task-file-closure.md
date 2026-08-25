# T468 S3 ATA Task-File, IRQ And Reset Closure

## Change

`hdc.c` now has one generic-ATA task-file-writable predicate, applied to
Features, Sector Count/Number, Cylinder Low/High, Device/Head and Command.
It is deliberately bypassed for the existing Compaq WD branch. This implements
ATA-3's BSY/DRQ restriction without a second command parser or a per-port
compatibility path.

The existing `irq_pending` field now denotes the one HDC interrupt condition;
one local synchronization helper maps that condition and nIEN to the existing
PIC IRQ source. Status acknowledgement clears the condition. Masking nIEN
only withdraws the PIC line, and clearing nIEN republishes an uncleared
condition through that same line. No public ABI, second IRQ owner or callback
was added.

Generic ATA reset restores Error `01h`, Sector Count `01h` and Sector Number
`01h`, while retaining the existing DRDY/DSC ready status. The Compaq reset
branch retains its former zeroed task-file behavior.

## Proof

- Rendered ATA-3 printed pages 16, 21--22, 27 and 106--110 are the normative
  source. They define command/task-file access restrictions, nIEN, status
  acknowledgement and the logical reset signature.
- 86Box `4fef696` corroborates one busy/DRQ gate for task-file writes and
  recomputation of IRQ visibility when device control changes; its controller
  timers remain outside this change.
- `core-machine-hdc-smoke` now proves reset fields, nIEN mask/unmask
  restoration, Status acknowledgement, blocked Busy task-file writes and a
  blocked DRQ Command write.
- `vm-hdc-port-smoke` proves the same nIEN restoration through the default
  VM composition.
- `core-machine-compaq-hdc-s5-smoke` and `vm-model40-hdc-s26-smoke` pass,
  proving no generic ATA predicate leaked into the Compaq route.
- The existing HDC portal verifier was stale: it required the pre-T434
  VM-to-Core private topology submission. It now verifies the actual sole
  plan path, `machine_devices.c -> core_machine_plan_configure_hdc ->
  core_machine_configure_hdc`; `verify-hdc-portal-closure` and
  `verify-core-controller-authority` pass.

The focused build and direct smoke executions pass. ATA device identity,
DMA, controller/cable timing, board decode/IRQ values and backing-media
selection remain unchanged L2 boundaries.
