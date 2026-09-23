# M5 T413: DeskPro External-Write And Arbitration Bridge

T413 is the next bounded receiver in the DeskPro physical-cycle and phase-timing
proposal. It starts from the original D4 read facts and the completed T410--T412
Core observability/locality path, then determines whether an external CPU write
or arbitration phase can be implemented as original, reference-derived or
generic-AT behavior without changing the sole Core CPU/DMA transaction owner.

No source, firmware, media or reference implementation is imported. Exact D4
BWAIT, write phase, DMA/refresh arbitration, overlap and physical/L3 claims
remain excluded unless admissible evidence and a project-owned regression bind
them.
P1 ae3839f1 is independently accepted by the S2 closure audit. T413 closes its bounded CPU-write bridge while exact D4 physical phases remain transferred. [Closure audit](../etc/evidence/t413-s2-external-write-locality-closure-audit.md).
