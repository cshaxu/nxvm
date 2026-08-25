# T467 S5 CGA Board-Timing Closure

VADP already has one construction-only board input: profile composition copies
`clock_plan.vadp` and `cga_text_timing` into Core, Core initializes and resets
`vadp_clock`, and the scheduler alone advances VADP. VADP then owns the copied
raster phase and status relation. No host clock, live setter, device callback
or second scheduler participates.

The selected Model 339 route is Board L3: its profile supplies the qualified
`315/1408` VADP clock ratio and the direct smoke emits
`M5:T375:S2:MODEL339-CLOCK-CONTRACT:OK` and
`M5:T375:S13:MODEL339-CGA-REFERENCE-CONTRACT:OK`. Its reset replay is covered
by the same smoke. Default PC/AT's `1/1` and `48/8/8` are Current data L2:
they flow through the same Board-L3-capable route but have no recorded source
qualification. The IBM source supplies logical raster/status relations, not a
selected Core-tick/ISA-wait conversion; no formula is invented.

This closes CGA-T1/T2 at the correct boundary. Physical display-buffer
contention, composite/monitor waveform and any absent light-pen lifecycle
remain exact L2 inputs, not VADP timing defects.
