# M5 T585 — MyPC110 remaining device and model qualification

Status: active, S1. The owner directed completion of the first four PC110
queue tasks. T583 and T584 are closed; the third receiver is the
[device and model proposal](../proposals/m5-pc110-device-model-qualification.md).

S1 audits the current owner paths, tests and source-backed evidence before
planning repairs. Existing boot and interaction evidence does not by itself
qualify PC Card, power, audio or physical display hardware.

The [S1 inventory](../etc/evidence/t585-device-model-inventory.md) records
the present positive evidence, gaps and planned sequential receivers.

S2 corrects card-detect status acknowledgement, shared global-register
addressing and invalid-index bounds. Dual-width qualification and the
remaining IRQ/window/card work are in the [S2 evidence](../etc/evidence/t585-s2-pcic-status.md).

S3 connects the modeled card-detect ISA status IRQ through PC110's existing
PIC and tests two sockets, acknowledgement and reset. INTR#/wake, other card
status inputs, windows and media remain explicit subsequent work; see
[S3 evidence](../etc/evidence/t585-s3-pcic-irq.md).

S4 adds the independent Ricoh memory-window decoder. Production Core routing,
card media and the remaining timing bit are explicitly retained for the next
receiver; see [S4 evidence](../etc/evidence/t585-s4-pcic-memory-decode.md).

S5 adds a runtime-selectable range to one existing Core overlay and validates
cache invalidation through actual 486 execution. The PC110 card backing and
guest route remain S6; see [S5 evidence](../etc/evidence/t585-s5-core-device-range.md).
