# M5 T529 Common-Machine Residual Audit

## Outcome

T529 audits the entire retained `vm/machine` corpus against current
`common/machine` and the read-only SoftPC consumer. It does not manufacture a
generic executor from unlike Core and CCPU execution loops. The only proven
cleanup is a pair of NXVM-local dead controls; the task remains active while
S2 records its implementation and closure evidence.

## Task Record

| Subtask | Status | Compact result |
| --- | --- | --- |
| S1 | Complete | `6e7b43a4` classifies all 29 tracked `vm/machine` files. It proves that Common already owns the sole copied request/run/safe-point/paused-Debug protocol; NXVM's bounded Core runner and SoftPC's CCPU timer rendezvous are distinct product adapters. [Ledger](../etc/evidence/t529-s1-vm-machine-common-residual-ledger.md) identifies two local dead controls as the sole S2 receiver. |
| S2 | Active | Deletes the `control_changed` event and unused completion-wait API without changing Common or either product executor boundary. |

## Proposal

[M5 T529 proposal](../proposals/m5-common-machine-executor-completion.md)
