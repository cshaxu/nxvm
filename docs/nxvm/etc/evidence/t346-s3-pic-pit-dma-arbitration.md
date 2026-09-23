# M5 T346 S3: PIC, PIT, And DMA Arbitration

## Owner And Migration Boundary

Before S3, `core_machine_advance_scheduler()` converted a CPU-retirement batch
and then directly advanced DMA, refreshed storage, advanced PIT, advanced
input/display/provider domains, and refreshed PIC. The order was deterministic
but was a fixed implementation sequence rather than a due-event contract.

S3 moves the common immediate arbitration chain only: `core_machine_timeline`
owns one callback for each machine tick. Each callback converts the DMA and PIT
domains for one tick, advances one DMA grant, advances PIT waveform state, and
then refreshes PIC. The callback schedules its successor at the next due tick.
Thus a due tick has the stable order `DMA -> PIT -> PIC`; any DMA request or
PIT IRQ source is visible to PIC at that same arbitration point. No second
machine scheduler advances these three controllers.

The callback is cleared by `core_machine_timeline_reset()` during cold reset;
the reset path schedules exactly one new tick-one callback after all
PIC/PIT/DMA state has reset. A controller cannot retain a prior-reset callback.
Copied trace records now carry both `elapsed_ticks` (the CPU-retirement
observation time) and `timeline_ticks` (the due event time). This preserves the
documented post-retirement run contract while exposing the full 64-bit due-tick
order, rather than making an unproven sub-instruction timing claim.

## Retained And Transferred Paths

The retained scheduler continues to own FDC/HDC refresh, VADP, KBC, RTC,
provider advancement, and all host boundaries. S4 owns RTC/FDC/ATA readiness,
including RTC-sourced NMI questions; S5 owns input/display/presentation. No
storage service duration, FDC transfer timing, ATA PIO readiness, NMI source,
or host-time behavior is claimed by S3.

## Proof

`core_machine_arbitration_s3_smoke` runs one 3-tick NOP retirement and proves
the trace order:

```text
CPU retire at elapsed tick 3
DMA, PIT, PIC for due tick 1
DMA, PIT, PIC for due tick 2
DMA, PIT, PIC for due tick 3
run boundary
```

It then cold-resets the same machine and verifies that time returns to zero,
only the new tick-one arbitration event remains pending, and the sequence is
new. `core_machine_timeline_s2_smoke` proves the single-tick callback order and
the successor token/observation. Retained focused controller proofs cover PIT
waveforms and IRQ0 source behavior, DMA one-grant transfer behavior, and PIC
priority/lifecycle.

The CMake T345 pure owner-test cohort is 120 (plus 3 mixed) after the S3 owner
smoke. The target receives the existing verified target-local strict flags; no
linked-library or global-flag claim is made.

Promotion/retirement: retain through T346 S6, then merge the final
arbitration/readiness/input-display dispositions into T346 history and the
Windows readiness map.
