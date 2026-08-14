# M5 T366: Bus-Timed PC/AT Operation

## Task Record

T366 follows the completed instruction, selected-device, and NMI ownership
audits. It may admit a bounded PC/AT bus-availability model only after one
documented profile/corpus identifies its routes, availability contract and
excluded hardware; it does not itself imply physical cycle exactness.

## Active Progress

### S1: Selected-profile bus source and route inventory

S1 inventories the current transaction, CPU memory/I/O, DMA, PIC, FDC and HDC
routes against a selected PC/AT hardware/corpus contract. It records whether a
safe next availability-contract S is admissible, without allocating wait states
or changing runtime behavior.

S1 is accepted at `098a146a`. Its
[profile and route inventory](../etc/evidence/t366-s1-pcat-bus-profile-route-inventory.md)
finds no selected physical target or reproducible corpus from which to allocate
availability state. A later S requires the owner decision recorded there; it
must retain the T365 NMI receiver and may not use reference-model code as a
timing authority.
