# M5 T220: CGA Text Raster And Retrace

## S1: Contract And Probe Design

**Status:** active. T220 replaces the temporary deterministic `3DAh` phase
with an owned VADP text-raster observation model. It does not claim CGA
graphics, EGA, VGA, host pacing, or cycle-accurate CRT timing.

### Owner And Boundary

`core_machine` owns the elapsed-tick clock. Its VADP owns the text-raster
phase, mode/status registers, text VRAM window, dirty generation, and copied
snapshot. The default PC/AT profile freezes the admitted text-raster ratios.
VM composition may copy a display snapshot for a renderer but never reads or
mutates guest VRAM directly. Platform timing and host display events never
advance the raster.

### Required Semantics

- Port `3DAh` is read-only and derives bit 0 (display enable) and bit 3
  (vertical retrace) from VADP phase and elapsed ticks.
- The phase must have explicit active-display, horizontal-blank, and vertical-
  retrace regions. A reset starts at a deterministic phase.
- Text memory/CRTC behavior remains under the existing VADP owner. Direct
  port and B8000h access observe the same state as ROM `INT 10h` services.
- `core_machine_run()` remains the only elapsed-time source. Repeated status
  reads do not change phase.

### S1 Probe Plan

Add port-level probes that sample `3DAh` across one frozen raster period,
assert deterministic bit-0/bit-3 transitions and reset origin, and verify that
reads alone do not advance phase. Retain text snapshot, DOS prompt/video-port,
Console/debugger, FDD, and HDD regression coverage.

### Stop Conditions

Stop if this requires a host timer, renderer access to guest state, a second
VADP state object, or any NXVM Console/debugger/startup behavior change. CGA
graphics belongs to T225.
