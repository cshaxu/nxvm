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

S2 is accepted at `6cbb67a9`: it reconciles the retained 8254 counter/output subset and its one channel-0
IRQ0 path. Its [evidence](../etc/evidence/t350-s2-pit-irq0.md) keeps channel-2
and port-61/speaker explicitly outside the counter-0 mechanism.

S3 is accepted at `6af43223`: it reconciles MC146818 event flags and their
Register-B enable image through one IRQF/IRQ8 synchronization owner, repairs
the documented alarm don't-care match, and proves CMOS index/mask separation,
reset/finalize, and retained deterministic readiness. Its
[evidence](../etc/evidence/t350-s3-rtc-cmos.md) retains the exact manual,
source, and proof boundary.

S4 is accepted at `742035c3`: the complete producer/consumer sweep finds no
selected PC/AT parity or I/O-channel-check source contract. It retains the
CPU NMI state as a consumer and CMOS bit 7 as mask-only, then transfers the
missing board source to the bounded NMI TODO rather than synthesize one. Its
[decision evidence](../etc/evidence/t350-s4-pcat-nmi-decision.md) records the
exact future admission condition.

## Closure

T350 closes the selected PC/AT platform-signal graph. PIT channel 0 has one
8254-to-IRQ0 lifecycle owner; MC146818 events have one calendar-to-IRQ8 owner
and a separate CMOS mask/index adapter; all selected reset/finalize and
deterministic-readiness paths have focused proof. Port-61/PPI/speaker and a
real parity/I/O-channel-check producer stay explicit TODO receivers because no
current profile/corpus establishes their board contracts. The task makes no
host-clock, host-audio, firmware, generic CPU-delivery, or Windows claim.
