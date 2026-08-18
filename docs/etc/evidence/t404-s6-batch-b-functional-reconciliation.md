# T404 S6: Batch B Functional Reconciliation

`M5:T404:S6:BATCH-B-RECONCILIATION:OK`

## Frozen Batch B matrix

| Public capability | Functional owner and direct proof | Disposition |
| --- | --- | --- |
| Removable FDD lifecycle | VM session; `current.vm-session-media-lifecycle-s3-smoke`, T404 S3/S4 | Accepted: startup configuration, stopped insert/remove, running rejection, retained-state updates and failure atomicity. |
| Model40 1.2 MB FDD/FDC/DMA2/IRQ6 | VM floppy profile plus Core FDC/DMA/PIC; `current.vm-model40-fdd-s18-smoke`, `current.vm-model40-fdc-s24-smoke` | Accepted logical route; physical media rotation, command-to-DRQ/IRQ phase and timing remain DeskPro physical receivers. |
| Model40 fixed 40 MB HDC/IRQ14 | Model40 composition plus Core HDC; `current.vm-model40-hdc-s26-smoke` | Accepted startup-only logical media and controller route; Compaq physical-media behavior/timing remains transferred. |
| Default PC/AT and Model339 controller topology | VM default profile; `current.vm-pcat-topology-s2-smoke`, `current.vm-pcat-composition-s4-smoke`, `current.vm-ibm-5170-model-339-composition-smoke` | Accepted declared functional topology; selected-machine board timing remains profile-local. |
| Text/CGA/EGA/CECG presentation | Core VADP plus copied VM display provider/mailbox; `current.vm-rom-ega-int10-system-smoke`, `current.vm-model40-cecg-s9-smoke` through `s13`, `s28`, `current.vm-runner-display-cadence-smoke` | Accepted selected digital register/frame/presentation routes; raster, monitor and board timing remain physical receivers. |
| Keyboard and AUX mouse | Core KBC plus VM ordered request transport; T403 KBC regressions, `current.vm-keyboard-host-ingress-smoke`, `current.vm-kbc-aux-guest-smoke`, `current.vm-mouse-driver-dos-smoke` | Accepted copied host ingress, keyboard endpoint flow and guest AUX packet route; physical keyboard/mouse/8042 timing remains transferred. |
| Public catalog, firmware manifest and media selection | VM product/session; `current.vm-product-session-catalog-smoke`, T404 S2 | Accepted: named profiles only, BYOB manifest validation and declared startup media. |
| Fixed/removable publication boundary | VM session; T404 S3/S4 | Accepted: HDD replacement unavailable after publication, FDD changes only stopped. |
| Input error boundary | VM session; T404 S5 | Accepted: only mapped key/mouse events enter ordered ingress; unknown kinds reject without guest effect. |

## Completion decision

Every S1 Batch B row now has direct functional proof, a bounded T404 repair, or
an explicit physical/reference receiver. The complete functional product
contract is exhausted. This does not establish any board clock, device phase,
firmware visibility, monitor/raster behavior, reference-derived timing, or
DeskPro Model-40 L3 readiness. Those boundaries remain in the queued
current-product L3 and DeskPro physical/L3 candidates and retained evidence.

Verification: source/test-registration reconciliation above; full current gate
286/286 from S5; documentation governance before acceptance.