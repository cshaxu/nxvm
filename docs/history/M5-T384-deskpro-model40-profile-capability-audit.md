# M5 T384: DeskPro Model-40 Profile And Capability-Gap Audit

## Task Record

T384 is the first bounded DeskPro 386 Model 40 task. It selects only facts
supported by the original 1986 Compaq DeskPro 386/16 primary documentation,
maps them to the current source inventory, and hands each gap to the later
CPU, selected-device-function or board-timing candidate. It does not implement
the profile or claim DeskPro readiness.

## Accepted Progress

### S1: Primary-source profile and capability reconciliation

S1 requalified the T373 preliminary BOM from Compaq's September 1986 Technical
Reference Guides and the related D4 primary technical material.  The resulting
[evidence](../etc/evidence/t384-s1-deskpro-model40-profile-capability-audit.md)
freezes the Model-40 system board, 80386DX-16, 1 MiB 32-bit memory, Compaq
Multipurpose Fixed Disk Controller, one 1.2 MB diskette drive, one 40 MB fixed
disk, dual PIC/DMA/PIT, MC146818, 8042/A20 PAL, NMI/reset and D4 bus contract.

The audit also proves that the Model-40 product designation does not select a
video/monitor pair, physical keyboard or ROM revision.  These remain explicit
owner configuration choices.  S1 accepts the primary-source capability ledger
and its CPU/functional/timing transfers; it does not accept a runnable profile
or any L3 result.  Documentation governance and actual-change review passed.

## Open Task Boundary

T384 remains open.  Its next continuation requires the owner to choose one
documented Compaq display/monitor pair, one documented keyboard, and an
owner-managed ROM revision/provenance.  Until then, no generic CGA/EGA/VGA,
ATA/HDC, IBM MFM, AUX, or IBM 5170 slot may be substituted.  The next queued
CPU task may proceed only with its bounded architectural scope and may not
claim a runnable DeskPro profile.
