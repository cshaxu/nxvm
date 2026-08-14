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

### S2: Exact 5170 profile and probe-corpus lock

S2 selects a field-configured IBM 5170 Model 339 Type 3 at 8 MHz with 512 KB
system-board RAM, Rev.3 ROM slot, 101-key keyboard, IBM CGA and a documented
TEAC 1.44 MB drive. The Model 339 MFM fixed disk and serial/parallel hardware
are deliberately unselected; the drive is a documented field upgrade, not a
factory Model 339 claim. Its [profile lock](../etc/evidence/t366-s2-5170-profile-lock.md)
corrects the stale 360 KB drive wording, names a repository-owned future probe
corpus and MFM/ST-506 TODO receiver, and narrows the next S to the board
memory-parity NMI lifecycle. It allocates no timing or runtime behavior.
