# T511 S10 VADP Phase-Consumer Audit

`M5:T511:S10:VADP-PHASE-CONSUMER:NO-HIT`

## Sources

The retained IBM CGA/EGA and selected Compaq ledgers define the implemented
port, CRTC, display-enable and aperture contracts. Manual-backed controller
relations remain L3; external 86Box/Bochs/PCjs display models are Other-L2
corroboration only. Core VADP is sole owner of guest video state, ports, VRAM
and captured frame, while VM copies one snapshot into a presentation mailbox.

## Owner/Consumer Matrix

| Boundary | Sole production path | Disposition |
| --- | --- | --- |
| CGA/EGA port and VRAM writes | `vadp.c` owns mode, CRTC, sequencer, graphics, attribute and VRAM mutation. | No firmware/VM copy maintains a second mode or VRAM truth. |
| Display enable/retrace | VADP derives status and attribute display-enable from its current controller state. | A new edge is observed from current state, not authorized by a past status read. |
| Text/planar/graphics capture | VADP creates one copied snapshot and updates its captured-frame comparison before returning it. | Every changed mode/VRAM/geometry produces a new snapshot state. |
| VM presentation | `vm_session_publish_display` obtains only that copied Core snapshot, copies it once to the presentation mailbox, and advances its mailbox generation only after publish. | Cadence affects host frequency only; it cannot reuse an old frame as a guest-state acknowledgement. |
| Reset/output disabled | VADP remains the sole reset/display-disabled owner; VM has no fallback frame or parallel renderer state. | A prior frame cannot re-enable output. |

## Executed Proof And Conclusion

The focused Debug cohort passed: VADP text/status, display-authority,
CGA graphics/640, EGA sequencer/controller/planar, CGA/EGA system,
display-composition, ROM INT 10h, presentation and display-cadence smokes
(17/17). It covers the Core port/VRAM-to-snapshot path and the copied VM
presentation consumer.

No ATA-style stale observation exists. The current VADP owner/snapshot/mailbox
path contains no parallel video state or repeated-ready consumer, so no
production change is warranted.
