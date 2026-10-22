# Emulator corpus

Standalone CMake projects are `shared-emulator` (source) and `shared-test-emulator` (tests).

This package selects C11 without extensions in standalone and embedded builds.
GNU/Clang builds enable -Wall -Wextra -Wpedantic -Werror in this package only.

`src/emulator` contains shared, ISA-neutral virtual-machine coordination. It
uses public `lib` contracts and accepts product behavior only through copied
options and injected callbacks.  It never includes app, host, or MVDM source.
Emulator has no platform directories, platform types, native calls or OS-selected
implementations. Base owns blocking mutex/event/task/wait; Types owns the atomic
vocabulary. Emulator retains queue, lifecycle and frame-publication ownership.

Machine owns a four-slot exchange for each of Window and Console.
Window publication uses the Lib reference/schema directly; there is no Machine
frame wrapper. The driver reports no-ready-frame separately through produced,
never by publishing an invalid payload. The executor passes writable Window
storage and Console text directly to the driver; each
consumer borrows its exchange through UI. Character maps belong only to the
Console frame. Session retains
metadata, never another frame. Machine can keep replacing pending output while
Window processes and retains older slots. UI/Window must be destroyed before
Machine and its exchanges.
Exchange creation allocates slot metadata only. Drivers prepare actual-size
leaf payloads on their unheld writer; equal specifications reuse storage.
Leaf retirement frees incompatible unheld storage without interpreting content
in Machine. The injected status-page builder also returns preparation errors.
Diagnostic copies own independent payloads and require explicit release.
Session treats events as hints: one bounded observation checks run, sequence
and graphics/text routing without copying payloads. It may observe a newer
publication than the notification. UI owns immutable status text and its pure
frame builder; composition injects that builder into the executor's existing
Console production flow for graphics mode. UI owns no frame exchange.
Machine publishes every produced frame without content comparison. Readiness
and optional dirty tracking belong to the driver; consumers optimize drawing.
Explicit diagnostic snapshot copies remain available, with the expected run and
unchanged destination on rejection; they are not the presentation transport. UI owns the shared action vocabulary;
session does not maintain a second action enum or mapping.
UI APIs have one control-thread caller. Immutable creation options are shared
with input callbacks; their run-generation tag uses a Types atomic. Frame
deduplication and object ownership remain control-thread-local. Native producers
must be destroyed before the event sink/context is released.
UI destruction first stops the broker reader and removes its output binding,
then joins the KVM producers. Any failure retains UI and remaining callback
dependencies; the application must not continue freeing them. Constructors
clear a valid output pointer before validating other arguments.

Session uses one internal control state
for completed facts and presentation actions; pure derivation stays separate.
Session forwards TEXT through the same running-only input sink as MOUSE,
without a held-key entry; character support belongs to the machine adapter.
UI submits the fixed graphical Console explanation only when entering that
content kind or creating a fresh Console. Text still follows frame sequences;
only successful submissions update these control-thread-local markers.
This raw Console explanation is a fixed 80x25 status surface, independent of
guest text frames (which may have up to 50 rows) and the cooked monitor.
Its injected text supports printable ASCII, ignores CR and starts a new row
on LF. Other bytes display as spaces; there is no UTF-8 decoding or automatic
line wrapping. Characters outside the 80x25 surface are omitted. Products
must supply status/help text within this existing bounded contract.

Session owns one pending cooked line until its normal/rejected event is consumed
or the broker confirms cancellation/handoff, plus the one lifecycle request in
flight, delayed monitor text and the next requested prompt. Frame events never
request input. Provider prompt readiness is level-triggered, not consumed by
notification; text, explicit requests and prompt admission use one result outlet.
While editing, notification text first cancels and joins the reader, discarding
the partial line. A completed queued line remains pending and is still consumed.
UI only forwards that cancellation to its broker; it owns no second reader state
or editor. Providers hold no duplicate transition, prompt or monitor-text state.

Session command results may borrow additional text until the next provider
call. Session consumes it synchronously through its existing monitor transaction,
normalizing LF/CRLF in bounded chunks. No large result is silently truncated;
allocation/formatting and native write failures remain explicit.

| Component | One responsibility | Public contract |
| --- | --- | --- |
| `machine` | executor, lifecycle/input queues, frame publication, optional paused debug adapter | `machine_interface.h` |
| `session` | one control queue, completed-fact reduction and dispatch | `session_interface.h` |
| `ui` | monitor logical Console, broker and KVM composition | `ui_interface.h` |
| `product` | neutral Machine/Session/UI construction and ordered teardown | `composition_interface.h` |

`session` calls `machine` and `ui`; `product` composes those three neutral
owners. No other emulator component edge is permitted. Application and host code may
compose only the root `*_interface.h` contracts; implementation headers and
source files remain component-local. Importing products decide whether to inject
an adapter or expose a command. Emulator never depends on an outer component.
The machine debug contract is synchronous to the control-thread caller and
serviced by the existing paused executor.
Machine FIFO admission and push share one mutex. At a user pause/debug stop,
the executor closes admission and freezes pending input, then publishes PAUSED.
Genuine resume delivers pending input in order at the normal safe-point cadence.
Pending host input is not part of driver snapshots; a cold run or successful
state replacement clears the previous run's queue. Parked state-read continuation keeps guest
input closed; genuine running/resume opens it. Enqueue returns OK for acceptance,
INVALID_STATE for normal closed admission and LIMIT_EXCEEDED for overflow.
Neither rejected events nor accepted prefixes are replayed by Session.
Pause does not preserve ordinary host input transitions or resynchronize host
keys on resume. Session still handles source reset/retirement and keyboard
release bookkeeping, without injecting those releases into a paused machine.
Hotkeys and window-close control retain their existing admission. Mouse input
has no Session ledger and is admitted only while running.
Machine copies opaque pointer-free request/response bytes with explicit lengths,
bounded by 128/1536 bytes in its single request slot. It does not interpret CPU
operations. Protocol vocabulary belongs to the requesting frontend and its
architecture-specific adapter, which use aligned typed copies.
Driver execution validates protocol-specific sizes and access constraints.
Failures return zero response length without changing caller output bytes.
Request admission and executor claiming share one lock. Failed completion reset
or command wake rejects admission without executing the request. Completion
notification failure permanently faults the executor; synchronous callers also
wait on its independent task cancellation and join before returning an error.
Commands are never replayed. Concurrent failure of native completion and task
cancellation, or inability to prove thread exit, is outside recoverable operation:
retain all callback/stream contexts until successful shutdown. Debug cancellation
clears frontend plans, not in-flight requests. Emulator builds only its four neutral components; products separately
select any architecture-specific frontend.

Machine shutdown synchronously joins its worker and all callbacks without
freeing the machine object. The serialized owner may then release callback
targets and finally destroy the machine/driver; destroy reuses shutdown.
Both return status. Failed shutdown/destroy retains ownership; callback targets
and the driver must remain alive until shutdown succeeds.
Shutdown is permanent and idempotent, unlike the restartable product stop.

## Independent verification

Source-only Emulator needs just src/emulator and src/lib. The neutral test suites
need only src/emulator, src/lib, test/emulator and test/lib. Outer CLI/protocol
and assembly tests are not part of the neutral Emulator suite. No importing
application sources, configuration or resources are needed:

```text
cmake -S test/emulator -B build/emulator
cmake --build build/emulator
cmake --build build/emulator --target emulator-verify
ctest --test-dir build/emulator --output-on-failure
```

Production sources/verification and the external test suite have separate
LF-normalized MANIFEST.sha256 files. Verification rejects missing, extra,
changed or duplicate entries. The source/build DAG gate rejects platform
bypasses and sibling private includes; its negative tests live in test/emulator.
The source-only build still accepts an explicit EMULATOR_LIB_ROOT when configured
directly. It does not register or carry test code.
Emulator synchronization tests run against actual Base primitives without sleeps.
Platform backend availability does not imply complete native display parity.
