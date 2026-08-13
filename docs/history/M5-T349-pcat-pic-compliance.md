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
| S2 | Reconcile ICW/OCW command state, priority, EOI/AEOI, and PC/AT cascade selection/acknowledgement through one PIC-owned mechanism. |
| S3 | Complete OCW3 poll/read/special-mask/SFNM state through the S2 controller selection and acknowledgement owner. |

## Accepted Progress

S1 is accepted at `0153f6ff`: the retained Intel 8259A/PC/AT ledger maps
every adopted controller mode, request producer, CPU consumer, reset/finalize
route, and L3 boundary. The task continues with S2 command/priority/cascade,
S3 OCW3 modes, and S4 request/spurious/composition reconciliation. See the
[S1 evidence](../etc/evidence/t349-s1-pcat-pic-compliance-ledger.md).

S2 is accepted at `8cce3b3a`: ICW restart, command-visible IRR/ISR, priority,
EOI/AEOI, and cascade acknowledgement now share one controller-owned
selection/commit path. Its completion evidence is retained in [the S2
command/priority record](../etc/evidence/t349-s2-pcat-pic-command-priority.md).

S3 is accepted at `c79553f8`: OCW3 status, poll, special-mask, and PC/AT
SFNM state now use the same controller selection/acknowledgement mechanism.
Its completion evidence is retained in [the S3 OCW3 record](../etc/evidence/t349-s3-pcat-pic-ocw3.md).
