# IBM PC/XT 5160-268 Selected-Device Functional Closure

## Purpose

Close functional completeness for every device selected by the accepted IBM
PC/XT 5160-268 capability audit before assigning its board-local timing. This
candidate follows the XT profile audit and precedes the XT board/device
phase-timing closure.

## Required scope

For each selected XT platform/transaction, input, storage, display,
interrupt/DMA, timer, NMI, reset, memory/ROM, and bus-facing component, close
the documented register, state-machine, IRQ/DRQ, error/recovery, reset, and
consumer gaps. Reuse shared mechanisms only after proving their state and
publication semantics are compatible with 5160-268. Preserve the separate
8088 CPU boundary; board waits, service duration, arbitration, and event phase
belong to the following candidate.

For storage, close the accepted Xebec logical fixed-disk route: one Core HDC
personality with the selected byte-stream ports, DMA3/IRQ5 binding,
source-proven command/result/reset behavior, and the existing sole
logical-image/geometry/persistence owner. Add a focused inserted-image and
boot-consumer regression. Do not create an ATA shim, a second CHS or media
cache, raw-MFM media, or a generic XT profile.

## Non-goals and stop conditions

No PC/AT assumption, generic PC/XT expansion, 8086 replacement, unselected
adapter, synthetic timing value, protected-mode/Windows claim, or
firmware/media import. Stop and transfer a defect to its earliest owner when
the selected XT evidence cannot prove the component relationship.

## Evidence standard

Require a selected-device functional matrix, register/state/reset and
IRQ/DRQ traces, focused consumer regressions, shared-owner sweep, and a
complete handoff of availability/service/phase gaps to the XT timing candidate.
