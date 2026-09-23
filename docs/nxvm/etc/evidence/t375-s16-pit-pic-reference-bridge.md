# T375 S16: PIT/PIC Reference Bridge

## Scope And Provenance

This is a bounded, secondary behaviour observation for the selected IBM PC/AT
5170 Model 339 timing receiver.  It does not claim an IBM physical waveform or
turn 86Box into an NXVM dependency.

- Reference: local 86Box v6.0 source, revision `4fef696a`.
- Build: owner-local UCRT64 build with `ENABLE_PIT_LOG=1` and
  `ENABLE_PIC_LOG=1`; no source was copied or changed in NXVM.
- Configuration: `machine=ibmat`, 80286 at 8 MHz, 512 KiB, CGA, AT FDC,
  the owner-approved external 5170 Rev.3 ROM bundle, and no mounted guest
  disk, floppy, or optical media.
- Trace budget: one ROM-initialization run, 15 seconds wall-clock and 1 MiB
  raw-log ceiling.  The process was terminated by the harness at the wall-clock
  boundary; the log was 116,088 bytes and was deleted after the checkpoints
  below were extracted.

The input schema is the fixed external configuration above.  The retained
checkpoints are deliberately semantic: raw pointer values, host throughput,
the reference's internal `ctr->count = 18` scheduling cadence, and all ROM
instruction addresses are comparison-masked.

## Observed Checkpoints

The Rev.3 ROM initialization trace first programs the master and slave PICs
through the conventional ICW sequence, then unmasks/masks IRQ0 as it exercises
the timer.  Its final recurring channel-0 setup writes control `36h` and
count `0000h`, which the reference reports as mode 3 and a 65,536-count
period.  A recurring IRQ0 service checkpoint is:

1. channel 0 reaches its active output transition and the master PIC selects
   IRQ0;
2. the CPU receives the two 8086-mode acknowledge phases (`phase 0`, then
   `phase 1`);
3. firmware reads PIC status through OCW3, restores its mask state, then
   writes master non-specific EOI `20h`.

This ordering is consistent with the reference's `pit.c` channel-0 rising-edge
route to `picint(1)` and `pic.c` two-phase i86 acknowledgement implementation.
It is a model behaviour cross-check only; it supplies no physical interval.

## NXVM Comparison And Decision

NXVM's existing focused proof
`core-machine-pit-irq0-s2-smoke` passed with marker
`M5:T350:S2:PIT-IRQ0:OK`.  Its mode-2 vector is intentionally not a firmware
mode-3 waveform comparison, but it proves the shared comparable lifecycle:
PIT output asserts the IRQ0 source; master-PIC vector `08h` becomes available;
the explicit EOI clears the in-service state; a later output-low transition
withdraws the asserted source.  No competing timer/PIC path or contradictory
event ordering was found.

Therefore S16 makes **no production correction**.  It closes only this
reference-bridge experiment.  The remaining receivers are source-backed
channel-0 mode-3 cadence/phase mapping, board wait/ISA arbitration, DMA/FDC
service timing, RTC/KBC timing, and the separate final Model-339 L3 audit.

`M5:T375:S16:PIT-PIC-BRIDGE:OK`
