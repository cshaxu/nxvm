# M5 Core HDC ATA Phase Contract

> Retained proposal for the completed T468 task.

**Status: closed as M5 T468.** The accepted record is retained in
[`M5-T468-core-hdc-ata-phase-contract.md`](../history/M5-T468-core-hdc-ata-phase-contract.md).

## Purpose

Define one selected ATA/IDE PIO controller and logical-media phase contract.

## Admission And Dependencies

The admitted original source is X3T13, *AT Attachment-3 Interface*, 2008D
Rev. 7b (January 1997), stored in the owner-managed archive as
`X3T13_2008D_ATA-3_Rev7b_Jan1997.pdf`, SHA-256
`FC9D1C42B0B3EF916C790BAFED05E96D9EBA3B221C0E0D3E914E6F948DDD0237`.
S1 evaluates, rather than presumes, whether the current generic `ATA_PIO`
task-file shape plus the existing logical 512-byte media provider can form a
bounded ATA-3/Other-L3 personality. It consumes PIC and T449; it
does not consume, represent or imply IBM MFM/ST-506, a specific IDE chipset,
physical drive media or Model-339 fixed-disk support.

## Scope And Completion

Freeze reset, BSY/DRQ, command/data/error/result, IRQ14, IDENTIFY, read/write,
cancellation and logical-media detach rows. Prove no partial command or
host-latency timing leak. Every finite row is Manual L3, bounded Other L3,
Board-L3-capable with source-qualified input, or exact L2; a standard-defined
task-file fact may not be weakened by a missing physical drive model, and a
missing selected controller/medium fact may not be invented from the current
code.

## Planned Subtasks

1. **S1 - ATA-3 source and cross-model ledger.** Verify the archived PDF's
   text/OCR quality and relevant page ranges; compare the finite generic
   ATA_PIO task-file rows with read-only 86Box, MAME, PCjs, Bochs and QEMU;
   create Checklist 1 with source tier and exact L2 disposition.
2. **S2 - implementation-gap audit.** Trace the same rows through `hdc.c`,
   media, PIC, Core scheduler, default profile and focused/guest tests;
   create Checklist 2 with one owner, current behavior, proof and repair or
   transfer.
3. **S3 - task-file/reset/command closure.** Repair all selected ATA-3
   register, reset, command, result and IRQ semantics solely in the existing
   HDC owner, with focused port and state proof.
4. **S4 - data/media/cancellation closure.** Repair selected IDENTIFY,
   read/write, DRQ data phase, media-detach and cancellation behavior through
   the existing logical-media provider; do not introduce a physical drive
   model, host-latency callback or second media state.
5. **S5 - complete closure audit.** Reconcile both ledgers, sweep all HDC
   writers/consumers, run focused and serial current gates, publish stripped
   Release artifact 0468, and transfer only exact L2 boundaries.

## Architecture And Minimalism

`hdc.c` remains the sole task-file, command, phase and IRQ publication owner;
the existing media provider remains the sole logical backing owner; Core's PIC
route remains the sole IRQ consumer. Profile composition may select one copied
immutable topology before construction, but cannot mutate HDC state. Do not
add a host-async bridge, generic timing setter, per-command shadow state,
second parser or ATA-to-MFM compatibility branch. A shared helper is eligible
only if it removes duplicated HDC command-phase logic.

## Boundary

MFM/ST-506, Read/Write Long, ECC, physical-sector semantics, drive mechanics,
controller-chipset timing, unqualified default timing and Model-339 fixed-disk
support remain explicit TODO receivers. No ATA-like fallback may be presented
as Model-339 fixed-disk support.
