# T352 S5: Selected Digital-Video Composition And Closure

## Composed State Graph

The selected guest display graph remains deliberately layered:

```text
guest ports and checked RAM -> VADP owned selected state -> value snapshot
    -> session copied frame -> presentation mailbox -> host consumer
```

VADP is the sole owner of text/CGA/EGA mutable display state.  The core
peripheral timeline advances VADP after KBC at the retained deterministic due
tick; session publication occurs outside that guest-time callback and copies a
snapshot into a mailbox.  Platform code captures the mailbox only.  There is
no renderer, mailbox, or host event-loop participation in guest time.

## Cross-Mode, Reset, And Copy Proof

`current.vm-display-composition-s5-smoke` is the owner-local composition proof.
It creates an ordinary VM session and executes these real boundaries in order:

| Transition | Writer and selected source | Observed copied result |
| --- | --- | --- |
| reset text | session reset and VADP text defaults | text mailbox frame with 80x25 dimensions |
| text to CGA 320 | 3D8h/3D9h plus B8000 even/odd interleaved bytes | indexed 320x200 mailbox frame with selected packed pixels |
| mailbox isolation | guest changes B8000 after publication, before a new publish | captured mailbox generation and pixels remain the prior CGA values |
| CGA to EGA 320 | sequencer/graphics/attribute port setup and A0000 planar byte | indexed 320x200 EGA frame, four-plane pixels and selected palette |
| reset after EGA | `vm_session_reset` invokes core reset then forces publication | text frame has zero graphics dimensions, pixels, and palette; timeline is reset with three due callbacks |
| EGA re-arm after reset | normal guest sequencer/graphics/attribute writes | fresh 320x200 EGA copied frame contains cleared plane data, not the prior EGA pixels |

The focused proof uses values rather than private renderer state.  It therefore
proves mailbox and capture value isolation without inventing a parallel host
display owner.

## Timeline And Lifecycle Reconciliation

T346 S5's retained `current.core-machine-input-display-s5-smoke` proves a CPU
retirement at due tick one is followed by FDC/HDC readiness, then KBC and VADP
in that order, with VADP after KBC and reset restoring exactly three scheduled
callbacks.  This S consumes that result rather than duplicating the timeline
owner.  The new composition smoke observes the reset timeline state at the
session boundary and pairs it with selected VADP reset/re-arm publication.

`vm_session_publish_display` copies the core snapshot into a fresh platform
frame before mailbox publication; `core_platform_presentation_mailbox_capture`
copies that frame to the host consumer.  The `force` path at session reset is
intentional: it replaces a prior graphics frame with the current reset text
frame, avoiding a stale host presentation while leaving guest time unchanged.

## Selected-Row Closure And Transfers

| Selected S1 row | Closure evidence |
| --- | --- |
| CGA text, 320x200x4, and 640x200x2 | S3 VADP port, VM, and DOS evidence. |
| EGA planar 320x200x16 and mode-10 640x350x16 | S4 planar/controller/CRTC/mode-10 and VM/ROM/DOS evidence. |
| register/raster/retrace and VADP due tick | S2 lifecycle evidence and T346 S5 deterministic timeline proof. |
| copied frame and reset composition | S5 composition smoke plus mailbox/session implementation sweep. |
| artifact | `vm-0-5-0352`, `build/output/nxvm_0_5_0352.exe`, SHA-256 `37A00874C05FB402151AF1A77BD28796D03B2A738D6F3EC08761E4541F2D6BB4`. |

No production defect was reproduced.  The exact retained transfers are broad
VGA/VBE/SVGA and EGA/CGA mode/CRTC breadth, composite simulation, dot-clock
and bus contention, host renderer cadence/event-loop policy, and Windows guest
execution.  The former items remain the later port-topology/L3 timing
receivers or the existing digital-video TODOs; Windows remains the readiness
map's consumer rather than a surrogate display proof.

## Verification

The S5 owner marker, T346 input/display timeline owner, selected CGA/EGA core
owners, EGA VM/ROM/DOS owners, governance, diff check, and the full current
gate are required for the implementation delivery.  The complete gate has
229 current tests after S5 registration.
