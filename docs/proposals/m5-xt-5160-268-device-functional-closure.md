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
logical-image/geometry/persistence owner.  `08h` reads its documented one
logical sector; `0Ah` uses only its documented nonzero block count, with an
unproven zero convention rejected rather than inferred. Add a focused
inserted-image and boot-consumer regression. Do not create an ATA shim, a
second CHS or media cache, raw-MFM media, or a generic XT profile.

The required DMA proof is the configured Core 8237A service path itself: a
device-to-memory Read must enter Core RAM and a memory-to-device Write must
reach the existing media provider.  Calling a channel provider directly is
only a component test and cannot replace this route proof.

For IBM 5160 PPI parity/I/O-check controls, the selected XT PPI is the sole
owner of PB4/PB5 enable latches, PC6/PC7 input levels and NMI request state.
Board sources submit only typed input levels; Core alone performs CPU NMI
delivery and mask handling.  Do not borrow PC/AT planar parity state, add a
second fault latch, or invent fault-generation timing.

For B6 session admission, profile selection is immutable and separate from
owner-supplied content: one typed session request validates the source-policy
BYOB manifest, copies the resolved XT plan and presents Core with generic
read-only mappings only.  The required 64 KiB base system-ROM window is
`F0000h`--`FFFFFh`; the selected Xebec option-ROM window is optional and
`C8000h`--`C9FFFh`.  The existing session creator, rather than an XT helper or
new command grammar, owns validation, resource lifetime and failure.  Fixed
FDD/HDD images are session resources routed to their existing sole Core media
owners.  No vendor byte, local path, ROM catalogue, AT synthetic-firmware
fallback or board-timing assertion may cross that boundary.

## Non-goals and stop conditions

No PC/AT assumption, generic PC/XT expansion, 8086 replacement, unselected
adapter, synthetic timing value, protected-mode/Windows claim, or
firmware/media import. Stop and transfer a defect to its earliest owner when
the selected XT evidence cannot prove the component relationship.

## Evidence standard

Require a selected-device functional matrix, register/state/reset and
IRQ/DRQ traces, focused consumer regressions, shared-owner sweep, and a
complete handoff of availability/service/phase gaps to the XT timing candidate.
