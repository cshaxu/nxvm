# M5 T360: Four-Profile Intel Timing Source Reconciliation

## Task Record

T360 consumes the explicit source-conflict and range transfers from T359. It
reconciles primary Intel timing authority by processor profile without
inventing an average, borrowing a later-profile row, or turning a secondary
emulator into a specification. It is a prerequisite for later selected-profile
cycle work, not a physical-timing or CPU-semantics task.

## Accepted Progress

### S1: Four-profile source-authority and consumer inventory

S1 is accepted at `219043bd`. Its
[source-authority/consumer inventory](../etc/evidence/t360-s1-four-profile-source-authority-consumer-inventory.md)
binds the existing ten timing consumers to the 8086 Tables 2-20/2-21, 80186
Table 1-16, 80286 Appendix B, and 80386 PRM section 17.2.2.3 sources. It
records the formal 80286 NOP table `3` versus prose `2` conflict without
changing the provisional table-based consumer, and distinguishes source
ranges, following-byte/EA context, 80386 `m`/mode context, delivery, physical
service, x87, and VME/PVI boundaries.

The inventory establishes the non-overlapping later units: S2 authority
edition/NOP precedence, S3 8086/80186 range forms, S4 80286/80386 contextual
normalization, and S5 closure audit. It adds only a static source-inventory
verifier; no runtime timing value, artifact, ABI, or CPU behavior changed.
Documentation governance, the verifier, and 245/245 current-gate tests
passed.
