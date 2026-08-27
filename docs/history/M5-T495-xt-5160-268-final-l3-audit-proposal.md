# IBM PC/XT 5160-268 Final Model-L3 Audit

## Purpose

Make the final ready/not-ready decision for IBM PC/XT 5160-268 only after its
8088 CPU, board/bus, PIC, DMA, PIT, PPI/keyboard/NMI, FDC/media, CGA/VADP and
Xebec unit contracts complete.

## Required scope

Reconcile the frozen 5160-268 CPU, memory/ROM, ISA, DMA, PIC/PIT, input,
storage, display, NMI, reset, cancellation, and deterministic ordering
evidence. Demonstrate that every selected device is functionally complete and
that each profile-local timing contract is source-backed or explicitly bounded.
Keep this result distinct from both Model 339 and DeskPro Model 40.

## Non-goals and stop conditions

No repair work, 5150/XT hybrid, generic 8088-PC conclusion, physical waveform
claim, or Windows claim. A missing functional proof returns to its earliest
owner; a missing timing proof returns to the XT timing candidate.

## Evidence standard

Require an independent requirement-to-evidence and machine-component closure
matrix, cross-device/reset/cancellation replay, applicable current-gate
evidence, and an owner-visible XT L3 ready/not-ready decision with residual
receivers.

The matrix must separately confirm four retained fixed-disk selections:
`default-at` ATA PIO, IBM 5170 Model 339 WD1003/ST-506, the selected DeskPro
386 Model 40 Compaq/WD personality, and IBM 5160-268 Xebec. The first three
are regression-preservation inputs from T479; the last is accepted only when
the XT profile, Xebec functional route and phase contract are all present.
