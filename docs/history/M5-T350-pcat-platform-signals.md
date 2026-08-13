# M5 T350: PC/AT Platform Signals

## Task Record

T350 consumes T346's deterministic due-event order and T349's completed
master/slave PIC request boundary. It audits and then reconciles the selected
8254, MC146818, PC/AT port-61/PPI/speaker, and NMI signal graph before later
KBC, video, port-topology, and L3 bus-timing work proceed.

## Active Subtasks

| Subtask | Purpose |
| --- | --- |
| S1 | Build the complete manual-to-source-to-proof platform-signal ledger and assign every adopted gap to a bounded S2--S4 receiver or an explicit later transfer. |
| S2 | Reconcile the retained 8254 counter/gate/IRQ0 path and any selected channel-2/port-61 behavior. |
| S3 | Reconcile MC146818 event, CMOS-adapter, IRQ8, acknowledgement, and lifecycle behavior. |
| S4 | Resolve the selected PC/AT NMI source only if S1 establishes its wiring and proof, then close the signal graph. |

## Accepted Progress

S1 is accepted at `b5708af1`: it builds the complete 8254/MC146818/IBM-PC/AT signal graph and assigns the
retained PIT/IRQ0, RTC/CMOS/IRQ8, channel-2/PPI/speaker, and NMI rows to one
owner or exact receiver. Its [ledger](../etc/evidence/t350-s1-pcat-platform-signals-ledger.md)
keeps port-61/speaker and a real NMI producer explicitly unimplemented until
their independent admission conditions are met.

S2 reconciles the retained 8254 counter/output subset and its one channel-0
IRQ0 path. Its [evidence](../etc/evidence/t350-s2-pit-irq0.md) keeps channel-2
and port-61/speaker explicitly outside the counter-0 mechanism.
