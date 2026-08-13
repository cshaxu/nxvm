# M5 T349: PC/AT 8259A Compliance

## Task Record

T349 consumes T346's deterministic due-event boundary, T347's retained device
request producers, and T348's completed DMA path. It closes the bounded PC/AT
master/slave Intel 8259A-compatible PIC mechanism before later platform-signal
and peripheral packages consume its delivery contract.

## Active Subtasks

| Subtask | Purpose |
| --- | --- |
| S1 | Build the complete manual-to-source-to-proof compliance ledger and assign every adopted gap to a bounded later S or explicit transfer. |

## Accepted Progress

S1 is accepted at `0153f6ff`: the retained Intel 8259A/PC/AT ledger maps
every adopted controller mode, request producer, CPU consumer, reset/finalize
route, and L3 boundary. The task continues with S2 command/priority/cascade,
S3 OCW3 modes, and S4 request/spurious/composition reconciliation. See the
[S1 evidence](../etc/evidence/t349-s1-pcat-pic-compliance-ledger.md).
