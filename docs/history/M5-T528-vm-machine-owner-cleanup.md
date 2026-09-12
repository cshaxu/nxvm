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
| S4 | Complete | Deletes the zero-state VM provider-lifecycle facade and makes device composition directly initialize, reset and finalize FDD/HDD providers without changing order or Core/Common boundaries. [Evidence](../etc/evidence/t528-s4-provider-lifecycle-cleanup.md) records focused proof, 299/299 unit and specialized gates. |
| S5 | Complete | Moves the sole request-to-machine factory to `vm/app` and generic PC/AT ROM provider to `vm/profile/default_profile`; Model 40 composition remains the one state-coupled machine owner. [Evidence](../etc/evidence/t528-s5-profile-firmware-owner-disposition.md) records 299/299 unit and specialized-gate proof. |
| S6 | Complete | `05408699` removes the zero-state execution forwarding facade, moves the request value contract to neutral `vm/`, and proves the Common FIFO/lease versus NXVM bounded-quantum-runner boundary. [Evidence](../etc/evidence/t528-s6-vm-machine-owner-closure-audit.md) records 299/299 unit, focused integration and specialized-gate proof. |
| S7 | Active | `0.5.0528` x64, unit and specialized proof pass, but x86 GCC frontend failure and three Model-40/5170 boot-matrix failures block task closure. [Evidence](../etc/evidence/t528-s7-task-closure-attempt.md) preserves the exact disposition. |

## Proposal

[M5 VM-machine owner cleanup proposal](../proposals/m5-vm-machine-owner-cleanup.md)
