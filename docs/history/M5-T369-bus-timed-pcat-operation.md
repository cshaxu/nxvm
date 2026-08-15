# M5 T369: Bus-Timed PC/AT Operation

## Task Record

T369 consumes the closed T368 80286 successful-retirement CPU ledger and the
T366 Model 339 Type 3 baseline. It owns the next bounded step toward the IBM
PC/AT 5170 bus-availability model: CPU memory/I/O availability, named wait
states, DMA exclusion, selected device readiness visibility, and their shared
transaction lifecycle.

It does not convert the existing deterministic timeline into a physical clock,
does not select or emulate a fixed-disk adapter, and does not close 5170
model-L3. The following device-service, phase-refinement and model-L3 audit
candidates remain ordered receivers.

## Active Progress

### S1: Bus fact and production-route inventory

S1 maps the locked Model 339/Type 3 facts and every relevant current production
route to its source status, availability/lifecycle owner, trace/reset behavior,
and bounded later receiver. It distinguishes IBM primary evidence from allowed
86Box, MAME and PCjs behavior leads, and leaves any value absent from both as
an explicit transfer. S1 adds no runtime timing, artifact, or 5170-L3 claim.

S1 is accepted at `87460471`. The next receiver is a bounded board-evidence
and availability-contract reconciliation; it cannot invent a scalar from the
current deterministic scheduling order or from a non-comparable emulator
configuration.

### S2: HOLD/HLDA availability reconciliation

S2 is accepted at `a5d11c48`. Intel 210760-002 supplies the 80286 local-bus
HOLD/HLDA semantic boundary, including a documented minimum but a
software/LOCK/interrupt/wait-dependent maximum. IBM's selected-board documents
do not turn that boundary into a universal wait scalar. The accepted
[reconciliation](../etc/evidence/t369-s2-pcat-hold-availability-reconciliation.md)
therefore assigns S3 one shared logical execution-round handoff mechanism and
retains board waits, physical waveforms and device durations as explicit
transfers.
