# T566 S2: Neutral Controller Substrate

## Change

`x86-nxvm-board-base` retains its single board lifetime and Core attachment.
It now makes the three previously forced PC/AT endpoint groups explicit
construction selections:

| Selection | Existing default | New no-endpoint result |
| --- | --- | --- |
| Keyboard topology | 8042 (zero) or existing XT PPI | No KBC/PPI, keyboard IRQ sources or host-input facade. |
| Display topology | VADP (zero) | No VADP instance or display I/O routes. |
| System facade | PC/AT (zero) | No port-92 route and no automatic high reset-ROM alias. |

The existing zero-valued configuration still selects all existing behavior.
The new choices do not name PC110 and do not introduce a peer-App dependency.
They are a construction boundary for any board with its own endpoints.

## Ownership and safety

PIC, PIT, DMA, clock and attachment ownership stay in the same Board object.
The no-keyboard path rejects host keyboard/mouse calls as unsupported rather
than dereferencing an absent 8042.  Reset and finalization conditionally touch
only the selected input device; VADP destruction/reset remains null-safe.
There is no new executor, timing source, route manager, buffer or fallback
behavior.

## Proof

`board_neutral_controller_smoke` constructs a no-endpoint configuration,
freezes and resets it, then proves PIC 20h and PIT 40h exist while KBC 60h,
port 92h and VADP status 3DAh are absent.  It also proves the host keyboard
facade reports unsupported.  This detects both accidental retention of a
forced AT endpoint and accidental removal of the reusable controller substrate.

The focused test passed with the existing Ninja/ccache x64 and x86 trees.
Both source/test manifests verify.  The broader X86 NXVM corpus is scheduled
as the S2 qualification run; this record does not claim PC110 firmware POST,
RTC, panel, input, storage or artifact qualification.
