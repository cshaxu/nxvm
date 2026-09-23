# M5 Core DMA 8237A Phase Contract

## Purpose

Complete one 8237A logical service-phase contract over T449's existing DREQ,
HOLD, transaction and BUSRDY owners.

## Admission And Dependencies

Consumes the accepted PIC contract and T449. Admit the original 8237A manual
and selected AT channel/cascade wiring before assigning a timing rule.

## Scope And Completion

Freeze request eligibility, channel selection, single/block/demand/cascade,
auto-init, memory-to-memory, terminal count/EOP, release, cancellation and
reset rows. Prove DRQ-to-transfer-to-IRQ/terminal outcomes without double
grant or partial transaction.

## Boundary

DACK/AEN/HRQ/HLDA pin waveforms are outside L3. DMA cannot publish time or
write CPU state; FDC is a later consumer of this contract.
