# Common corpus

This package selects C11 without extensions in standalone and embedded builds.
GNU/Clang builds enable -Wall -Wextra -Wpedantic -Werror in this package only.

`src/common` contains shared, ISA-neutral virtual-machine coordination. It
uses public `lib` contracts and accepts product behavior only through copied
options and injected callbacks.  It never includes app, host, or MVDM source.
Common has no platform directories, platform types, native calls or OS-selected
implementations. Base owns blocking mutex/event/task/wait; Types owns the atomic
vocabulary. Common retains queue, lifecycle and frame-publication ownership.

Machine frame snapshots combine a publication sequence, a typed Window frame
and text-only Console character maps. Copying transfers the active Window
extent and, for text only, both character maps. They specify the expected run
and leave the destination unchanged on rejection. UI publishes the Window
value and builds the small Console text value; it owns neither character
encoding nor a duplicate graphics buffer. Machine compares both maps and
font banks when detecting text changes.
Session treats frame events as publication hints and
derives content, sequence and graphics/text routing from that one snapshot,
which may be newer than the notification. UI owns the shared action vocabulary;
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

Session owns one pending cooked line until its normal/rejected event is consumed
or the broker confirms cancellation/handoff. Frame events never request input.
Provider prompt readiness is level-triggered, not consumed by notification;
text, explicit requests and prompt admission use one result outlet. While editing,
notification text first cancels and joins the reader, discarding the partial line.
A completed queued line remains pending and is still consumed. UI only forwards
that cancellation to its broker; it owns no second reader state or editor.

Session command results may borrow additional text until the next provider
call. Session consumes it synchronously through its existing monitor transaction,
normalizing LF/CRLF in bounded chunks. No large result is silently truncated;
allocation/formatting and native write failures remain explicit.

| Component | One responsibility | Public contract |
| --- | --- | --- |
| `machine` | executor, lifecycle/input queues, frame publication, optional paused debug adapter | `machine_interface.h` |
| `session` | one control queue, completed-fact reduction and dispatch | `session_interface.h` |
| `ui` | monitor logical Console, broker and KVM composition | `ui_interface.h` |

`session` calls `machine` and `ui`.
No other common component edge is permitted.  Application and host code may
compose only the root `*_interface.h` contracts; implementation headers and
source files remain component-local. Importing products decide whether to inject
an adapter or expose a command. Common never depends on the optional x86 corpus.
The machine debug contract is synchronous to the control-thread caller and
serviced by the existing paused executor.
Machine copies opaque pointer-free request/response bytes with explicit lengths,
bounded by 128/1536 bytes in its single request slot. It does not interpret CPU
operations. Protocol vocabulary belongs to the requesting frontend and product
adapter, which use aligned typed copies.
Driver execution validates protocol-specific sizes and access constraints.
Failures return zero response length without changing caller output bytes.
An unsuccessful completion wait is not proof of quiescence: shut down before
reusing the slot. Existing cancellation clears product plans, not in-flight
requests. Common builds only its three neutral components; products separately
select any architecture-specific frontend.

Machine shutdown synchronously joins its worker and all callbacks without
freeing the machine object. The serialized owner may then release callback
targets and finally destroy the machine/driver; destroy reuses shutdown.
Both return status. Failed shutdown/destroy retains ownership; callback targets
and the driver must remain alive until shutdown succeeds.
Shutdown is permanent and idempotent, unlike the restartable product stop.

## Independent verification

Source-only Common needs just src/common and src/lib. The neutral test suites
need only src/common, src/lib, test/common and test/lib. x86 CLI/protocol and
assembly tests are owned by test/x86, not the neutral Common suite.
No importing-product sources, configuration or resources are needed:

```text
cmake -S test/common -B build/common
cmake --build build/common
cmake --build build/common --target common-verify
ctest --test-dir build/common --output-on-failure
```

Production sources/verification and the external test suite have separate
LF-normalized MANIFEST.sha256 files. Verification rejects missing, extra,
changed or duplicate entries. The source/build DAG gate rejects platform
bypasses and sibling private includes; its negative tests live in test/common.
The source-only build still accepts an explicit COMMON_LIB_ROOT when configured
directly. It does not register or carry test code.
Common synchronization tests run against actual Base primitives without sleeps.
Platform backend availability does not imply complete native display parity.
