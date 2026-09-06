# M5 Cross-Product Library Contract Completion

## Purpose

Complete NXVM's in-repository `src/lib` so that the same C and header corpus
can be copied unchanged into SoftPC and consumed by both products through
thin, product-owned bindings.  The library is a reusable host-side facility;
it is never a machine emulator, a VM profile owner, a controller, firmware,
or guest-service implementation.

This is a corrective successor to T522.  T522 established the five library
roots and one NXVM route for each.  It did not prove that the public contracts
are independent of NXVM types, preserve SoftPC's event-driven presentation
semantics, or carry every host-side fact needed by SoftPC without a fork.

```text
src/lib/
  base/           portable types, status and atomics used by lib only
  ux/             copied frames, input events, actions and presenters
  host/           native synchronization primitives and cancellation waits
  session/        one generic single-session lifecycle mechanism
  storage/        host image access and persistence-policy mechanics
  observability/  bounded copied outcome/event publication

NXVM binding     -> NXVM machine/session APIs
SoftPC binding   -> SoftPC machine/session APIs
```

The two bindings may differ.  The library source and public ABI must not.
Neither product builds against the other repository, and neither gains a
runtime dependency on the other.

## Required Contract Corrections

### Portable Base

No public library header may include NXVM `type.h`, an NXVM product/Core/VM
header, or a Windows/POSIX SDK type.  `lib/base` supplies the small fixed-width
type, status and atomic facade needed by the library.  Native types stay in
the platform-private implementations below `ux/win32`, `ux/linux`,
`host/win32`, `host/linux`, `storage/win32`, and `storage/linux`.

This does not impose a type system on either product.  Product bindings adapt
their local types at the one library boundary.

### Explicit Presentation Control

`ux` publishes copied text or indexed-graphics frames and offers explicit
presenter operations.  It must not infer a Console/Window decision from a
frame type or automatically create, destroy, hide, freeze or restore a native
presenter.

The product app/session policy explicitly requests the active target, for
example `CONSOLE` or `WINDOW`, and explicitly opens or closes native
presenters.  The library coordinates the requested transition and reports
its result.  A product may therefore implement SoftPC's current policy
(Console for text, Window for graphics), an always-Window policy, or a
user-controlled policy without a fork of `lib/ux`.

### Frame Completeness

The public copied-frame contract must represent both current products without
machine-state access:

- text cells, dimensions, attributes, programmable text palette, primary and
  secondary guest glyph tables, font height, selected-font state, cursor
  position, shape and visible phase;
- indexed graphics pixels, stride, visible geometry, palette, dirty rectangle
  and frame sequence; and
- an immutable copied ownership boundary: a native presenter never borrows
  guest VRAM, controller state, a machine pointer, or an original renderer's
  buffer.

SoftPC continues to use its original renderers and guest-loaded font data to
produce its copied frame.  The library is not permitted to decode VGA state,
replace `nt_*ga` renderers, or synthesize guest palette/font state.

### Event-Driven Presentation And Input

The mailbox must carry a latest complete copied frame, sequence and native
wake capability.  Publication wakes the relevant native loop; a loop waits on
its native message source and frame wake source.  No library path may require
a periodic repaint timer, a fixed polling interval, or a full-frame compare
when no frame was published.

The input contract must carry normalized make/break, scan/key identity,
Unicode/RDP input, modifier state, relative/absolute mouse motion, buttons,
wheel and an explicit release-all operation.  A product registers its own
action map and action handler.  The library must not hard-code guest commands;
SoftPC may register Ctrl+Alt+P/M/D/F while NXVM retains its own actions.

Mouse capture is a reusable presenter state machine: only a product-approved
client-area action captures it; loss of focus, pause, terminal transition or
an explicit release action releases it.  A paused presenter cannot recapture
the mouse until the product resumes and permits capture.

### Lifecycle, Host Wait And Storage Boundaries

`host` must provide actual native event, cancellation, wait and join mechanics
rather than only sleep/yield/poll helpers.  Its public contract remains
native-type-free.  `session` serializes generic create/start/pause/resume/
reset/stop/join transitions and command acknowledgement, while an injected
product callback remains the sole owner of machine turns, safe points,
controller reentrancy, configuration, debugger policy and final guest result.

`storage` must expose the common host image modes: direct-readonly,
direct-writable, overlay, explicit commit and explicit discard.  It owns only
bytes and host persistence mechanics.  Products retain FDD/HDD topology,
geometry, controller requests, media selection and guest-visible behavior.

`observability` remains bounded copied publication.  It gains only shared
host facts such as lifecycle state, presenter target, frame sequence, input
drop/release, media result and stop reason; it is not a machine trace or a
second debugger.

## Implementation Sequence

### S1 - Cross-Product Compatibility Ledger

Freeze the actual T522 library corpus and the current SoftPC host-side corpus.
For every public type, function, mutable object, native wait, timer, action,
frame field and storage mode, record one disposition: retain as portable,
change in `lib`, retain product-local with reason, or defer to a named
receiver.  Define the zero-machine-pointer public ABI and the two minimal
binding contracts.  Stop if a necessary feature can be represented only by
exposing guest state or product ownership.

### S2 - Portable Base And Event-Driven Mailbox

Replace NXVM-specific public types with `lib/base`.  Give the mailbox an
event-driven wake contract and platform implementations, then remove each
fixed presentation polling path replaced by it.  Prove idle presenters do not
repaint or wake repeatedly and that a published frame wakes each relevant
native loop promptly.

### S3 - Complete UX Values And Explicit Presenter Operations

Extend the frame/event/action contracts to the stated text, graphics, input
and mouse-capture surface.  Replace automatic frame-type routing with explicit
product requests.  Preserve NXVM user-visible behavior through its binding;
provide a product-neutral conformance fixture for the SoftPC text/font,
palette, dirty-frame and Ctrl+Alt action shapes.

### S4 - Generic Lifecycle, Host And Storage Completion

Complete the event/cancellation/join contract, generic lifecycle
acknowledgement and image modes without moving machine safe-point semantics or
controller behavior into the library.  Delete superseded NXVM generic routes;
do not retain a compatibility wrapper around an old polling path.

### S5 - One-Path NXVM Cutover And SoftPC Adoption Handoff

Make NXVM use the completed shared library through one binding and remove each
superseded product-local generic route.  Publish a source-identical library
manifest and consumer conformance suite.  Create the bounded follow-up SoftPC
adoption task: it copies the manifest-selected library files unchanged,
implements only the SoftPC binding, and proves no source under SoftPC's
preserved machine baseline changes.

### S6 - One Generic File-Mechanics Owner

Replace `core/platform/file.*` with the sole `lib/storage` file-mechanics
owner.  Migrate every caller, including bounded line-reader/writer uses, or
place an irreducibly product-specific operation at its product owner; do not
retain type-adapted forwarding wrappers.  Sweep the repository for equivalent
read, write, replace, remove and exclusive-create implementations.  The
result must leave Core free of host file I/O and leave no second generic file
API.

### S7 - Remove Native Keyboard Code From Core

Move `core/platform/win32/keyboard.*` and its tests to the NXVM Win32 UX/VM
binding that owns host-layout conversion to PC scan-code input.  Preserve the
existing normalized event contract and guest delivery behavior, but leave Core
without a Win32 SDK edge or host-layout/scan-code policy.  Sweep every native
keyboard conversion helper so this is a relocation to one product binding,
not a parallel path.

### S8 - Name And Retain The Core Guest-Boundary Owners

Move `core/platform/display_frame.h`, `input_interface.*`, and
`presentation_mailbox*` into semantically named Core input/presentation
owners.  They remain distinct from `lib/ux`: Core owns the guest-derived frame
and guest-ingress synchronization; the VM performs the single copied
representation conversion; `lib/ux` owns native presentation.  Delete the
empty `core/platform` root and prove that this is not a second native mailbox
or input route.

### S9 - One Generic Monotonic Clock Mechanism

Move the host monotonic-counter mechanics from `vm/platform/virtual_time.*`
to `lib/host`, with platform-private implementations.  VM retains only the
policy that compares completed guest progress against that clock.  Sweep for
other wall-clock readers and consolidate every generic one without creating a
host-to-Core time injection route or changing guest-time ownership.

## Acceptance

- `src/lib` compiles using only its own public headers and standard C/platform
  private headers; it contains no NXVM machine, VM, profile or `type.h`
  dependency.
- A second neutral consumer/conformance build compiles the same library source
  and exercises text, indexed graphics, palette, dirty rectangle, mailbox
  wake, RDP-style input, actions, capture/release, lifecycle and storage modes.
- NXVM has one production route for each replaced generic mechanism, with no
  retained periodic presentation polling.
- A subsequent SoftPC task can copy the declared `src/lib` corpus byte for
  byte, compile it without source edits, and use only a binding outside
  `src/mvdm/softpc.new`.
- Native Console/Window switching occurs only after an explicit product request;
  a change between text and graphics frames alone has no routing side effect.
- `core/platform/file.*`, `core/platform/win32/keyboard.*`, the Core
  display/input/presentation boundary names, and `vm/platform/virtual_time.*`
  each reach their stated sole-owner disposition without a forwarding or
  compatibility path.

## Non-goals

This candidate does not import SoftPC, NTVDM64, ROMs, media, original SoftPC
renderers or guest code into NXVM.  It does not create a generic emulator, a
multi-session manager, a Linux graphics presenter, a new guest input protocol,
or an automatic text/graphics routing policy.  It does not change NXVM or
SoftPC machine, device, firmware, BIOS, controller or guest execution
semantics.
