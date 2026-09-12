# M5 T528 VM-Machine Owner Cleanup

## Objective

Remove existing Common/Lib duplicate runtime ownership from NXVM `vm/machine`
while preserving its sole NXVM role as Core/profile/assets/media adapter.

## Task Record

| Subtask | Status | Compact result |
| --- | --- | --- |
| S1 | Complete | `833b920c` inventory proves the current Common FIFO/lease boundary, exhausts every VM-machine file and transfers only the incomplete shared executor mechanism to the queued second candidate. [Ledger](../etc/evidence/t528-s1-vm-machine-owner-ledger.md) |
| S2 | Complete | `27b27298` deletes all VM-machine monitor printing and stale Core-product config, retains one copied information query, and moves the NXVM-only banner to `src/`. [Evidence](../etc/evidence/t528-s2-product-boundary-cleanup.md) records 299/299 unit and full gate proof. T529 remains the executor receiver. |
| S3 | Complete | Moves FDD/HDD storage-to-Core provider ownership to `vm/media`, deletes the reverse-dependent runtime media binding file, and retains one device-composition registry route. [Evidence](../etc/evidence/t528-s3-media-owner-relocation.md) records focused proof, 299/299 unit and specialized gates. |

## Proposal

[M5 VM-machine owner cleanup proposal](../proposals/m5-vm-machine-owner-cleanup.md)
