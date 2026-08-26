# M5 T487 IBM 5160 8088 Board, Memory, ROM And ISA Phase Contract

T487 is the independent IBM 5160 board/bus unit admitted after the closed
8088 CPU task. It may compose accepted CPU and controller contracts, but it
cannot recreate a controller or repair a controller-local symptom.

| Subtask | Result |
| --- | --- |
| S1 | Accepted: visually verified IBM board, clock, cycle, reset, map, ISA and NMI source facts establish the finite List-1 basis without importing a manual or firmware. [Evidence](../etc/evidence/t487-s1-5160-original-source-ledger.md). |
| S2 | Accepted: every admitted board clock, cycle, reset, map, ISA and NMI fact has one Manual-L3 List-1 row and a controller/CPU/host boundary before code inspection. [Evidence](../etc/evidence/t487-s2-5160-board-function-timing-list-1.md). |
| S3 | Accepted: every board row maps to one production owner/path; the retained Core boundary uses its existing rational clock plan and rejects additive waits for Intel-owned clock totals. [Evidence](../etc/evidence/t487-s3-5160-board-current-code-gap-list-2.md). |
| S4 | Accepted: source and bounded-reference reconciliation retains the single Core 8088 clock-total owner; unsupported bus phase is explicit L2 and no false code path is added. [Evidence](../etc/evidence/t487-s4-5160-board-one-owner-reconciliation.md). |
| S5 | Accepted: complete board closure retains one owner per row, explicit L2 boundaries and no unnecessary production change. [Closure audit](../etc/evidence/t487-s5-5160-board-closure-audit.md). |

The active contract is in [Current](../states/CURRENT.md). Its admitted scope
and the mandatory source/List-1/List-2/one-owner sequence are in the active
[proposal](../proposals/m5-xt-8088-board-bus-phase.md), which moves beside this
record when T487 closes.
