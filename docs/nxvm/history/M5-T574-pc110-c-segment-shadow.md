# M5 T574 — PC110 C-Segment ROM And Shadow Contract

This task receives the finite post-reset boundary established by T573.  It
uses the normal PC110 firmware and existing Core route mechanism; it cannot
replace missing board behavior with a firmware-address exception, host
interrupt handler or PC/AT facade.

The active packet in `states/CURRENT.md` is the executable authority for the
current S.  The source/design candidate remains
[`m5-pc110-c-segment-shadow.md`](../proposals/m5-pc110-c-segment-shadow.md).

## Closure

T574 closes as a source-gate receiver.  Its ordinary-executor trace found no
C-segment transition and no candidate configuration I/O before the first
post-reset CPU exception, so a dynamic C-segment map cannot be honestly
implemented.  The App retains only the independently evidenced F-segment
shadow.  The exact evidence and transfer boundary are recorded in
[`t574-s1-pc110-c-segment-trace.md`](../etc/evidence/t574-s1-pc110-c-segment-trace.md).
