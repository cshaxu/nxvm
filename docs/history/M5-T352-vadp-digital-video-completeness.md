# M5 T352: PC/AT Digital Video Completeness

## Task Record

T352 consumes the deterministic PC/AT L3 timeline closed by T346 and the
completed controller/input graph closed by T351.  It reconciles the selected
digital CGA/EGA display state graph before port-topology and bus-timing
candidates proceed.

## Active Subtasks

| Subtask | Purpose |
| --- | --- |
| S1 | Build the selected digital CGA/EGA manual-to-source-to-proof ledger and allocate every state row to S2--S5 or an exact transfer. |
| S2 | Reconcile selected register/CRTC/raster/retrace lifecycle. |
| S3 | Reconcile selected CGA mode, aperture, and copied-frame state. |
| S4 | Reconcile selected EGA planar mode, aperture, and copied-frame state. |
| S5 | Compose the selected display graph, verify reset/timeline/presentation isolation, build the task artifact, and close the package. |

## Accepted Progress

S1 is accepted at `c71c72da`: it establishes
the selected IBM CGA/EGA digital state graph, one VADP/timeline/copied-frame
owner map, exact S2--S5 receivers, and named TODO/Queue transfers.  Its
[ledger](../etc/evidence/t352-s1-digital-video-ledger.md) is the required
admission boundary for every later T352 display change.

S2 is accepted at `d47aa061`: it audits the selected VADP register/CRTC/raster lifecycle and finds
no reproduced mechanism defect: existing owners cover index/mask validation,
status flip-flop reset, phase/reset behavior, timeline advance, and copied
consumer isolation.  Its [evidence](../etc/evidence/t352-s2-vadp-register-raster.md)
retains selected CGA/EGA mode/aperture work for S3/S4 and composition for S5.

S3 is accepted at `30c15d44`: it reconciles the selected digital CGA text, 320x200x4, and mode-06h
640x200x2 decoders, ports, B8000/interleaved layout, palette, copied frame,
reset, and ROM-writer boundary.  Its
[evidence](../etc/evidence/t352-s3-cga-digital-path.md) retains graphics CRTC
page/geometry breadth as an exact digital-CGA TODO instead of adding a
speculative text-start-address shortcut.

S4 is accepted at `2264a979`: it reconciles the selected EGA planar mechanism: one VADP owner for
configuration, indexed registers, aperture, latches, planes, selected 320/640
capture, reset, and copied output.  It adds reset/re-arm/cleared-plane and
captured-value isolation proof without conflating the real 320 and mode-10
layouts.  S5 retains cross-mode/timeline/presentation composition and task
artifact closure.

S5 delivery composes text, selected CGA, selected EGA, reset/re-arm, retained
VADP timeline placement, and the copied session-to-mailbox boundary.  It also
builds the T352 developer artifact.  The detailed state/result matrix is
retained in the S5 evidence pending task acceptance.
