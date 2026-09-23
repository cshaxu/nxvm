# T375 S13: Model-339 CGA Reference Contract

`M5:T375:S13:MODEL339-CGA-REFERENCE-CONTRACT:OK`

## Qualified Secondary Result

The owner-authorized, read-only 86Box source at revision `4fef696a` is tagged
`v6.0`, matching the local v6.0 reference-tool generation.  Its standalone
IBM-CGA model uses a character-time base of `157500000 / 88` Hz.  The selected
Model-339 nominal CPU source is 8 MHz, so the project-owned rational conversion
is:

```text
(157500000 / 88) / 8000000 = 315 / 1408
```

The Model-339 descriptor now publishes that ratio only to its existing VADP
clock domain.  It leaves generic PC/AT untouched and remains explicitly a
source-labelled secondary-reference cadence, not an IBM board fact.  IBM's
PC/AT technical reference establishes the oscillator family, while the exact
divider is not established by the selected IBM manuals.  No ROM table, guest
execution, 86Box binary output, third-party code, or local asset identity is
committed.

## Reproducible Contract And Disposition

The experiment uses a project-owned, non-factory CRTC program through `3D4h`
and `3D5h`: R0=4, R1=3, R2=3, R3=11h, R4=2, R5=1, R6=2, R7=2 and R9=3.
The comparison points are CRTC mask/readback; `3DAh` bit 0 at the displayed and
horizontal-blank partitions; and `3DAh` bit 3 at the vertical-sync partition.
The sampling budget is one frame, 30 seconds wall-clock, and two frame periods
without a new checkpoint.

The qualified v6.0 source provides the independent CGA character-rate model
and CRTC-driven display/blank partition for this contract.  It does **not**
provide an accepted binary runtime trace: no full machine boot or unbounded GUI
observation was required or performed.  Consequently this S admits only the
rational VADP publisher; it does not alter the existing S11 logical CRTC/status
rules and makes no measured board-phase, wait, snow, monitor, or L3 claim.

## Focused Proof

The Model-339 clock-contract smoke proves the descriptor-to-session binding,
the exact `315/1408` VADP conversion, and reset replay.  The existing VADP
logical-raster smoke retains the project-owned CRTC stimulus and status
checkpoints; the adjacent EGA controller and CRTC tests remain required
regressions.

## Similar-Issue Sweep And Transfer

The sweep covers both PC/AT descriptors, their session clock-plan transfer,
the sole VADP clock-domain consumer, VADP CRTC/status ownership, and adjacent
EGA routes.  There is no second timer, reference-emulator call, ROM-derived
default table, or test-only product API.

Exact IBM board cadence, selected adapter revision, default CRTC programming,
ISA contention/waits, snow, monitor waveform, and a runnable reference trace
remain open in the T375 board/device phase receiver and final 5170 audit.
