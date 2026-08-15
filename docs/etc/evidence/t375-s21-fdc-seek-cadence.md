# T375 S21: FDC Seek Cadence

`fdc.c` now retains a per-drive cylinder position and a pending seek deadline.
Seek and Recalibrate leave the previous cylinder and IRQ6 state visible until
the TEAC FD-235HF-A540 nominal 3-ms-per-track interval expires, converted by
S19 to 24,000 Model-339 ticks per track. Completion then reuses the existing
Sense-Interrupt/IRQ6 owner. Reset clears the pending FDC state.

The focused smoke proves a three-track seek remains pending without IRQ6 at
71,999 ticks and completes at 72,000 ticks, returning cylinder three through
Sense Interrupt Status:

```text
M5:T375:S21:FDC-SEEK-CADENCE:OK
```

This does not claim exact spindle start, rotational search, index phase,
controller command time, seek maximum, or physical waveform timing.
