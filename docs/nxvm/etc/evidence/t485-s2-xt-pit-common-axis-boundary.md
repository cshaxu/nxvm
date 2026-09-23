# T485 S2 XT PIT Common-Axis Boundary

`M5:T485:S2:XT-PIT-COMMON-AXIS:BLOCKED`

IBM's 1.193182 MHz PIT input is a Manual-L3 hardware fact.  The implied
`1/4` relation is valid only against the 4.77 MHz XT board-clock axis.  Current
XT execution has no such axis: `ticks_per_instruction = 1` feeds deterministic
successful-retirement scheduling, and Core explicitly distinguishes that from
physical retirement time.  Therefore configuring the current PIT clock ratio
as `1/4` would advance PIT once per four instructions, not once per four CPU
clocks.

The existing Core rational-clock plan is retained unchanged.  It is the right
receiver once a qualified 8088 retirement plus XT bus-cycle axis exists, but it
cannot create that axis.  The earliest receiver is a later T485 CPU/board-axis
S: it must source-map successful 8088 retirement, normal/five-clock I/O and
external-cycle waits onto one Core-owned axis before any controller ratio is
selected. No profile setter, VM tick writer, scheduler, host pacing change or
PIT behavior is added here.
