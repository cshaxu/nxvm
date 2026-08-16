# DeskPro 386 Model 40 Selected-Device Functional Closure

## Purpose

Close functional completeness for the devices selected by the accepted DeskPro
386 Model 40 capability audit before its board timing is refined. This
candidate follows the Model-40 CPU closure and precedes the DeskPro board bus
and device timing closure.

Before implementation proceeds, reconcile the accepted S16--S20 results into
one current functional matrix. It is the T386 planning input, not a new
machine selection: the selected Model-40 baseline remains the fixed 80386DX,
no-FPU, 1 MiB, 101-key, Compaq Enhanced Color Graphics/Color Monitor, one
1.2 MB diskette drive, one 40 MB fixed-disk subsystem and two-chip external
ROM contract. S20 publicly exposes the CPU/memory/ROM/1.2-MB portion only; its
current session has no fitted fixed-disk media route. S20 does not provide a
generic YAML variant mechanism.

## Required scope

For every Model-40-selected platform, input, storage, display, interrupt/DMA,
NMI, reset, memory/ROM, and bus-facing component, close the ledgered register,
state-machine, IRQ/DRQ, error/recovery, reset, and consumer gaps. Reuse a
shared repair only after its validation and state-publication semantics match;
otherwise repair the earliest shared owner and prove the Model-40 binding.
Leave board waits, service durations, arbitration, and phase placement to the
following timing candidate.

The current matrix must dispose every selected row in exactly one of these
ways: complete selected function; a bounded remaining T386 functional
receiver; a later DeskPro timing receiver; or an explicit `TODO.md` physical
media/electrical boundary. At minimum, the remaining T386 functional plan must
state and then close or truthfully transfer:

- D4 parity producers, diagnostic/status latches and IOCHK publication;
  D4-SKEY A20 behavior; and power, shutdown and 8042 reset arbitration;
- the selected 765A/1.2 MB raw-IMG logical command, state, reset, DMA2/IRQ6,
  error and BIOS-visible behavior. Flux, CRC, index, rotation and physical
  CHRN observables remain a separately admitted physical-media boundary;
- the Compaq Multipurpose Fixed Disk Controller's selected normal 40 MB
  CHS/PIO, reset, error and IRQ14 consumer route, plus a constrained startup
  external-media contract for the fitted fixed disk. The image must not be
  hot-swappable after publication or be confused with ATA/IBM MFM. Long,
  format, ECC/recovery and physical-sector semantics remain explicit physical
  media debt unless separately admitted;
- the source-backed Compaq Enhanced Color Graphics personality still visible
  to selected consumers: residual miscellaneous-output/monitor behavior,
  documented status and firmware-programming effects, and its reset/isolation
  contract. No IBM EGA default, copied VADP core or analog-monitor inference
  may fill a Compaq gap; and
- all selected profile bindings' fixed-versus-external declarations. A future
  cross-product task, after this Model-40 L3 audit, owns generic allowed
  variants. This task may establish the fixed Model-40 input to that task but
  must not implement a generic YAML `variant` surface.

The selected display is Compaq Enhanced Color Graphics plus Color Monitor. It
uses the shared VADP core through one small, source-backed Compaq personality
contract; no copied VADP core or IBM EGA default is permitted. IBM EGA remains
outside this candidate and belongs solely to current-product device-capability
L3 closure.

## Dependencies and verification

Begin from the accepted T384 capability selection, T385 CPU closure, and T386
S16--S20 evidence rather than the superseded S15 next-step narrative. Require
a current Model-40 matrix, selected state/reset and IRQ/DRQ traces, focused
consumer regressions, a shared-owner sweep, and an explicit timing handoff.
The matrix must identify the fixed startup profile contract and every deferred
physical-media boundary without recording firmware, guest-media bytes, local
paths or vendor hashes.

Before T386 may transfer to board timing, every row marked as a remaining T386
functional receiver must have its own accepted implementation evidence, and no
selected device may remain merely composed or syntactically selectable. The
DeskPro board candidate receives only documented service-duration, wait,
arbitration, phase-placement, electrical and physical-medium timing questions.

## Non-goals and stop conditions

No generic 386 clone, generic YAML variant schema, free-form machine builder,
486/VME/PVI/x87 expansion, unselected adapter, exact board timing, Windows
claim, or firmware/media import. Stop and return a shared defect to its
earliest owner; do not attach a local delay merely to make an incomplete device
appear ready. Stop a missing fixed-disk startup contract or an unsupported
selected-device fact for owner decision; do not invent a profile override.
