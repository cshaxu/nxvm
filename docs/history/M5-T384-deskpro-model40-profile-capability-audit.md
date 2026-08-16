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

### S2: Configuration and personality allocation

The owner selected Compaq Enhanced Color Graphics plus Color Monitor, the
101-key Enhanced Keyboard, and an owner-managed DeskPro 386/16 Rev-E ROM
constraint. The S2 evidence records these choices without retaining ROM bytes,
path, hash, part catalogue or a runtime dependency. It also allocates one
shared VADP core with disjoint IBM-current-product and Compaq-DeskPro EGA
personality/closure paths. No VADP or device implementation is included in
this delivery.

## Closure

Coordinator review accepted S2 against the owner approval, S1 primary-source
ledger, the actual source ownership boundary and the task exit criteria. The
review found one generic shared VADP core, no current DeskPro personality or
runtime binding, and no protected ROM bytes, paths, hashes or catalogue in the
repository. T384 therefore closes as a selection and capability-gap audit only:
it makes no runnable DeskPro, device-completeness, firmware-execution, timing
or L3 claim.

The next CPU candidate receives only the 80386DX-16 architectural/state
contract. The later selected-device functional candidate receives the first
runnable Compaq composition, including the small Compaq EGA personality and
selected input/storage/platform capability work. The later board candidate
receives Compaq board/device timing. IBM EGA remains a separate
current-product L3 receiver. The task-level closure audit is retained in
[S2 closure evidence](../etc/evidence/t384-s2-task-closure-audit.md).
