# T366 S8: FDC Service-Timing Non-Admission

`core_machine_readiness_tick` invokes `core_machine_fdc_advance` once per
project timeline tick. That tick is CPU-retirement time, not a declared
µPD765 controller-clock domain. Consequently it cannot faithfully allocate the
µPD765 SRT (1--16 ms), HLT (2--254 ms), or HUT (16--240 ms) quantities.

The [µPD765A documentation](https://cpctech.cpcwiki.de/docs/upd765a/necfdc.htm)
defines those timers at an 8 MHz controller clock and defines seek/recalibrate
completion through IRQ plus Sense Interrupt Status. The [IBM PC/AT Technical
Reference](https://www.bitsavers.org/pdf/ibm/pc/at/6139362_PC_AT_Technical_Reference_Sep85.pdf)
establishes IRQ6, while S7 selects DMA2. Neither creates a conversion between
the project execution tick and the controller clock.

S8 therefore makes no guessed timing allocation. A later T366 S must introduce
one source-labelled FDC clock-domain conversion before it may time reset,
seek/recalibrate, transfer, DMA or IRQ service. Existing phase/IRQ behavior is
retained but is not L3 timing evidence.
