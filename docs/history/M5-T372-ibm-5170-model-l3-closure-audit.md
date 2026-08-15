# M5 T372: IBM 5170 Model-L3 Closure Audit

## Task Record

T372 audits the selected IBM PC/AT 5170 Model-339 baseline after T366--T371.
It may only classify readiness or return gaps to their earliest owner; it never
substitutes an audit for missing source-backed phase implementation.

## Active Progress

### S1: readiness matrix

S1's [matrix](../etc/evidence/t372-s1-model339-readiness-matrix.md) finds the
selected baseline not ready: logical ownership/replay is evidenced, but every
board-specific phase conversion remains unallocated.

S1 is accepted at `67ec5177`. S2 must qualify primary-source gaps and any
eligible same-profile reference-model observation before selecting a value.

### S2: reference-model admissibility

S2's [admissibility matrix](../etc/evidence/t372-s2-reference-model-admissibility.md)
finds no current numeric reference candidate: 86Box corroborates only the
512-KB board limit, MAME's 8-MHz default retains incompatible memory and an
undetermined ISA clock, and PCjs documents candidate controls but no
revision-pinned complete observation.  Physical phase work therefore remains
open; S2 makes no 5170 model-L3 claim.
