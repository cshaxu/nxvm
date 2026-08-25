# M5 Core KBC 8042 And NMI Phase Contract

## Purpose

Define the selected 8042 command/FIFO/output-port/IRQ1/NMI and reset phases.

## Admission And Dependencies

Consumes PIC. Admit IBM AT, UPI-41/42 and selected keyboard-protocol sources;
host ingestion remains outside the controller clock contract.

## Scope And Completion

Freeze command acceptance, input/output FIFO, response, IRQ1 edge, output-port
A20/NMI/reset effects, cancellation and cold reset. Add typematic or serial
cadence only when its manual plus selected clock is admitted.

## Boundary

No host input timestamp or callback drives guest time. CPU reset/shutdown
semantics beyond the existing KBC request route remain separately admitted.
