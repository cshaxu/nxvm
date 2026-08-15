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
proper later XT receiver. The required firmware slot is an external
owner-managed research asset; do not record bytes, local paths, or a vendor
catalogue in the repository.

## Non-goals and stop conditions

No 5150/XT hybrid, 8086 substitution, generic PC claim, device repair, board
timing allocation, protected-mode claim, Windows claim, or firmware/media
import. Stop at an unidentified board fact and retain a source/probe receiver;
do not borrow PC/AT or clone behavior.

## Evidence standard

Require source-labelled 5160-268 profile provenance, a bill of materials,
component/capability-gap matrix, and an explicit partition between CPU,
functional, and board-timing receivers.
