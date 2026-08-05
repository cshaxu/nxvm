# M5 T222: PIT Elapsed-Tick Waveform

## S1: Contract And Port-Probe Design

**Status:** active. T222 will evolve the existing PIT from a coarse counter
into an elapsed-tick 8253/8254-admitted waveform subset. It will not use host
time or claim RTC behavior.

The core PIT remains the sole owner of counter, mode, gate, latch/read-back,
and output state. It accepts only the T219 elapsed-tick delta and emits IRQ0
through the T216 PIC source boundary. S1 must define per-mode count-zero, GATE,
BCD, output transition, latch/read-back, and IRQ assert/deassert expectations,
then add port-level probes before S2 changes behavior. Existing text-raster,
FDD/HDD boot, DOS prompt, Console, and debugger behavior remain regression
gates.
