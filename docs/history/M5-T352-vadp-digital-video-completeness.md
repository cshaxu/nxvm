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
