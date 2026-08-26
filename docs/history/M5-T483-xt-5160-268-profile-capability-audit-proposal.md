# IBM PC/XT 5160-268 Profile And Capability-Gap Audit

## Purpose

Lock IBM PC/XT 5160-268 as the single 8088 baseline and identify every selected
device and capability gap before its device-function and timing work begins.
This candidate follows the 8088 CPU-profile closure and the completed DeskPro
path; it is an audit, not an implementation or L3 decision.

## Required scope

Use IBM/Intel/component primary material to establish the selected 5160-268
board/revision, CPU clock, memory/ROM constraints, display, input, storage,
interrupt/DMA, timer, NMI, reset, and project-owned corpus. Map each selected
component to its functional and timing gaps, earliest shared owner, and the
proper later XT receiver. It also freezes the 8088-specific CPU attachment,
external-bus/prefetch and timing differences needed by 5160, reusing the
retained 8086 semantic ledger rather than restoring a standalone 8088 candidate.
The required firmware slot is an external
owner-managed research asset; do not record bytes, local paths, or a vendor
catalogue in the repository.

The audit must consume T479 S6's IBM/Xebec source ledger and establish the
selected fixed-disk adapter as required: its 8-bit `320h`--`323h` grammar,
DMA channel 3, IRQ5, immutable controller/media geometry binding, and
external-ROM admission boundary. It must state the earliest functional and
timing receivers; no ATA task-file substitution is permitted.

## Non-goals and stop conditions

No 5150/XT hybrid, 8086 substitution, generic PC claim, device repair, board
timing allocation, protected-mode claim, Windows claim, or firmware/media
import. Stop at an unidentified board fact and retain a source/probe receiver;
do not borrow PC/AT or clone behavior.

## Evidence standard

Require source-labelled 5160-268 profile provenance, a bill of materials,
component/capability-gap matrix, and an explicit partition between CPU,
functional, and board-timing receivers.
