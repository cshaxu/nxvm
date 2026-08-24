# M5 Core PIC 8259A Phase Contract

## Purpose

Complete the retained 18-row 8259A command, priority, cascade,
acknowledgement, EOI, mask and reset contract through the existing Core IRQ
and transaction boundary.

## Admission And Dependencies

Consumes T449 and T450's frozen PIC source/audit records. Intel 8259A and the
selected IBM AT cascade wiring are normative. 86Box, Bochs, MAME, PCjs and
QEMU may be inspected only as independently versioned reference observations:
they neither select the machine personality nor override source material.
PIT, RTC, KBC, FDC and HDC consume this contract.

## Scope And Completion

The frozen universe is `PIC-R1`--`PIC-R5`, `PIC-F1`--`PIC-F8` and
`PIC-T1`--`PIC-T5`: 18 rows, not a new inventory. It includes IRR/ISR/IMR,
fixed/rotating priority, ICW/OCW, cascade, IRQ visibility, acknowledgement,
EOI, cancellation and cold reset. Prove each row with command-to-visible-state
traces and focused cascade/reset regressions.

## Bounded Subtasks

1. **S1: source, reference and contract reconciliation.** Consume T450's
   ledger; inspect rendered manual pages and recorded reference implementations;
   freeze the resulting one-owner implementation plan. No behavior changes.
2. **S2: acknowledgement visibility contract.** Make the existing transaction
   boundary own the logical request-to-visible-to-acknowledgement sequence,
   including the 8086/88 two-INTA logical contract. PIC remains the sole owner
   of PIC state and CPU remains a consumer; no private delivery path or
   electrical-timing claim is permitted.
3. **S3: programmed cascade and initialization contract.** Make the single
   PIC selection mechanism consume ICW3/SNGL consistently, retain the IBM AT
   default as composition data, and cover ICW1 reinitialization. This removes
   the current accepted-but-ignored configuration state; it does not add an AT
   compatibility path.
4. **S4: retained-command proof and closure.** Close each of the 18 rows with
   focused command, priority, poll, mask, EOI, AEOI, reset, cascade and delivery
   regressions; remove any superseded helper/path and perform the minimalism
   audit before task closure.

## Boundary

INTA pin waveform, electrical propagation values and unselected board wiring
are outside L3. The data sheet also leaves the post-acknowledgement INT
reassertion interval unspecified; it cannot become a fabricated Core delay.
Do not add a second IRQ dispatch path or a controller-specific CPU delivery
mechanism.
