# M5 T529 Common-Machine Residual Audit

## Outcome

T529 audits the entire retained `vm/machine` corpus against current
`common/machine` and the read-only SoftPC consumer. It does not manufacture a
generic executor from unlike Core and CCPU execution loops. The only proven
cleanup is a pair of NXVM-local dead controls; Common retains its existing
sole copied request/run/safe-point/paused-Debug protocol.

## Task Record

| Subtask | Status | Compact result |
| --- | --- | --- |
| S1 | Complete | `6e7b43a4` classifies all 29 tracked `vm/machine` files. It proves that Common already owns the sole copied request/run/safe-point/paused-Debug protocol; NXVM's bounded Core runner and SoftPC's CCPU timer rendezvous are distinct product adapters. [Ledger](../etc/evidence/t529-s1-vm-machine-common-residual-ledger.md) identifies two local dead controls as the sole S2 receiver. |
| S2 | Complete | `cc5fd4a7` deletes `control_changed` and the unused completion-wait API while preserving `completion_ready`, pause acknowledgement and step state. [Evidence](../etc/evidence/t529-s2-dead-control-cleanup.md) records 2/2 focused, 299/299 unit, specialized/documentation gates and stripped 0528 x64/x86 artifacts. The unchanged 39/42 external-integration exception remains transferred to the named Model-40/5170 boot-chain TODO. |

## Proposal

[M5 T529 proposal companion](M5-T529-common-machine-residual-audit-proposal.md)

T529 is closed.
