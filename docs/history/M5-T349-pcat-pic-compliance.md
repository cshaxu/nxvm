# M5 T349: PC/AT 8259A Compliance

## Task Record

T349 consumes T346's deterministic due-event boundary, T347's retained device
request producers, and T348's completed DMA path. It closes the bounded PC/AT
master/slave Intel 8259A-compatible PIC mechanism before later platform-signal
and peripheral packages consume its delivery contract.

## Completed Subtasks

| Subtask | Closed outcome |
| --- | --- |
| S1 | Built the complete manual-to-source-to-proof compliance ledger and assigned every adopted gap to a bounded later S or explicit transfer. |
| S2 | Reconciled ICW/OCW command state, priority, EOI/AEOI, and PC/AT cascade selection/acknowledgement through one PIC-owned mechanism. |
| S3 | Completed OCW3 poll/read/special-mask/SFNM state through the S2 controller selection and acknowledgement owner. |
| S4 | Closed edge/level/source/reset composition and the deterministic-L3 spurious IRQ boundary. |

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

S4 reconciles edge/level source ownership, same-line composition, slave
cascade reassertion, and every retained PC/AT producer reset/finalize path.
It also records the deliberate deterministic-L3 boundary: the current ordered
selection/acknowledgement model has no physical INTA race from which to create
a synthetic spurious IRQ7/IRQ15; that electrical bus transaction remains a
later L3 bus-timing receiver.

## Closure Audit

T349 satisfies the requested deterministic-L3 PC/AT PIC result without
overclaiming electrical bus behavior. S1's manual-to-source ledger has one
accepted proof owner for initialization, command state, priority, cascade,
poll, special mask, SFNM, edge/level lifecycle, all retained producers, and
the CPU request consumer. S2 made selection and acknowledgement controller
owned; S3 made OCW3 a view of that same mechanism; S4 made device source
release explicit across reset/finalize and captured same-line/cascade level
composition.

The closure evidence is retained in the indexed S1 ledger and the S2, S3, and
S4 records. Fresh configuration, focused PIC/device/CPU proofs, documentation
governance, diff check, and 226 current-gate tests passed at S4 acceptance.
The retained proposal accompanies this history record.

## Explicit Transfers

Physical INTA sampling, lost-edge spurious IRQ7/IRQ15 behavior, wire/pin
waveforms, READY/wait duration, APIC, arbitrary cascade boards, and generic
CPU delivery redesign remain outside T349. The queued L3 bus-timing
convergence candidate is the sole receiver for a physical-INTA transaction;
the platform-signal, KBC/AUX, video, and port-topology candidates own their
respective devices. No transfer creates a second PIC state owner.
