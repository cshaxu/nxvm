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
