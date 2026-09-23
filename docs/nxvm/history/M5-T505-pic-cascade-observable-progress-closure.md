# M5 T505 PIC Cascade Observable-Progress Closure

The retained [admission proposal](M5-T505-pic-cascade-observable-progress-closure-proposal.md)
records the bounded source-first scope.

T505 closes the selected 8259A slave-request to master-cascade to CPU-wake
route. Intel `231468-003` establishes a logical cascade connection but supplies
no Core elapsed-tick delay; available 86Box, Bochs and PCjs code corroborates
immediate reevaluation while PCjs compatibility instruction delays remain
non-normative. The task therefore neither guessed a deadline nor promoted a
causal relation into a timing claim.

## S1 source and owner ledger

S1 P1 `d0f510fa` freezes C1--C10 in the indexed
[cascade ledger](../etc/evidence/t505-s1-pic-cascade-observable-progress-ledger.md).
It covers programmed topology, source assertion and withdrawal, masking and
priority, acknowledge, EOI, reset, CPU delivery, XT single-PIC nonparticipation
and the physical electrical-timing boundary. Coordinator P2 `65d470a3` accepts
the single receiver: immediate work inside the existing PIC owner.

## S2 implementation and closure

S2 P1 `afc1d540` keeps `cascade_irr` as the only derived master request. A
construction-fixed private PIC pair allows existing source and port-write
mutation paths to invoke the existing `core_machine_pic_refresh()` calculation
before scheduler observation or CPU delivery. No state mirror, new selector,
second dispatcher, profile behavior, VM boundary, host-time input, or guessed
delay was added. The owner-local regression proves edge publication,
mask/unmask withdrawal and republication, reset, and level-triggered
withdrawal without Core time advancement; existing programmed-cascade,
acknowledge/EOI, CPU/PIC, and single-PIC coverage remains in force.

The tracked repair is +18 source lines and +35 owner-local test lines, net
+53. Complete repository-only unit testing passes 312/312; complete
integration passes 20/20; documentation governance passes. The T504
unpublished-slave-cascade L1 receiver is therefore retired. The next queued
receiver is DMA service and arbitration.
