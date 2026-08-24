# M5 Core Controller Manual Admission And Gap Inventory

## Purpose

Establish one evidence-first admission baseline for the eight earliest Core
controller/device candidates: PIC 8259A, DMA 8237A, PIT 8254, RTC/CMOS,
KBC 8042/NMI, uPD765 FDC plus logical media, VADP, and HDC/ATA. This is a
research and inventory task only. It neither changes emulator behavior nor
allocates an implementation task identifier.

## Boundary

For each unit, the task admits or rejects the exact original/manual source and
the selected board or personality facts required to interpret it. It then
records two durable, per-controller checklists in indexed supporting material:

1. A source, function, and timing checklist names document identity, revision,
   provenance and permitted use; finite register/command/mode/result phases;
   functional rule; timing source or formula; reset/cancellation/IRQ/DRQ or
   equivalent signal relation; board binding; source pages; and an L3, L2, or
   blocked disposition.
1. An implementation-gap checklist names the current owner paths and symbols,
   observed behavior and tests, then classifies every checklist row as
   conforming, wrong-value, missing, unallocated, missing-input, or
   missing-test. It assigns each nonconforming row to exactly one later
   controller candidate; it does not create a duplicate implementation path.

The resulting finite source and gap universe is the shared admission input for
the existing controller candidates. A later implementation task may consume
only its own rows and must preserve T449's sole transaction, arbitration,
reset, time, and observation owners.

## Required Research Dispositions

Each controller receives one explicit disposition:

- **PIC 8259A:** original 8259A semantics plus selected cascade and interrupt
  wiring facts.
- **DMA 8237A:** original 8237A semantics plus request, grant, page and refresh
  bindings.
- **PIT 8254:** original 8254 modes plus selected oscillator, channel and
  output bindings.
- **RTC/CMOS:** MC146818-compatible behavior plus selected RTC clock,
  interrupt and CMOS ownership facts.
- **KBC 8042/NMI:** 8042/UPI host-interface behavior plus AT keyboard, reset,
  A20, NMI and board wiring facts.
- **FDC/uPD765 plus logical media:** uPD765 command/result behavior plus drive,
  DMA/IRQ and selected logical-media format facts.
- **VADP:** the selected digital display adapter's original programming and
  timing evidence; a generic display manual is not a substitute for the
  selected adapter.
- **HDC/ATA:** original ATA/IDE source and selected adapter/backing-media
  contract. IBM MFM/ST-506 material does not satisfy this row; absent ATA/IDE
  admission leaves it explicitly blocked.

Before any source is acquired, copied, derived, or used as evidence, the task
follows the source and research policy. Non-redistributable material remains
outside the repository; committed records retain neutral provenance and page
references rather than source bytes or machine-local paths.

## Completion Standard

All eight units have both checklists and a finite L3, L2, or blocked
disposition. Every missing source, machine fact, test, or implementation route
is assigned once or explicitly blocked. The task makes no claim that any
controller is implemented or L3-complete; it ends with the eight existing
implementation candidates still ordered PIC, DMA, PIT, RTC, KBC, FDC/media,
VADP, HDC/ATA.

## Minimality Constraint

The inventory is one shared evidence collection with one checklist pair per
controller. It must reuse existing T433 and T449 evidence where it is already
sufficient, link rather than copy authorities, and create no generic device
framework, parallel ledger, compatibility path, or speculative source import.
