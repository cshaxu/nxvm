# M4 Shared Audio Hardening

## Outcome

Turn the owner-accepted MyNes PCM playback path into a shared Lib capability
that SoftPC can import unchanged. Lib accepts only typed interleaved PCM and
depends only on `lib/types` and `lib/base`; it contains no MyNes, SoftPC, Core,
App, Common, guest-device, or product-policy dependency.

## Boundaries

Lib owns its bounded FIFO, output worker, native-slot lifecycle and wake/cancel
contract. Core and SoftPC Compat own PCM production and may never manipulate
native audio state. `query` and `wait_writable` describe the same producer FIFO
capacity. A failed native submission cannot discard accepted PCM silently.
Control operations synchronize with the worker before they retire or reset the
native sink. The task changes MyNes only; SoftPC is read-only and receives no
source edit or claim of import until its separate owner action.

## Acceptance

The Audio target explicitly depends only on Types and Base among Lib components.
Its public header exposes Types vocabulary only. Tests prove FIFO-full waiting,
cancellation, ordered worker submission, failed-submission retention or terminal
failure, clear/deactivate/destroy synchronization, and native callback behavior.
Both Windows architectures build and pass the complete repository suite; the
owner receives rebuilt artifacts for final audible confirmation. The final audit
names the exact files SoftPC may import unchanged and the product-level adapter
work still required there.

## S6 Delivery Record

Lib Audio now owns the 4096-frame producer FIFO, an asynchronous worker and
the native-slot lifecycle. A producer-visible frame remains in the FIFO until
the native backend accepts its complete 1024-frame playback batch. A full FIFO
is the single meaning of `query` capacity and `wait_writable`; `cancel_wait`
wakes one concurrent waiter with `INVALID_STATE`. A native timeout keeps the
batch pending, while any other native failure becomes the stream's terminal
status and wakes blocked producers. `clear` and deactivation first retire FIFO
contents, then wait for the worker to reset the native sink. Explicit clear is
the sole intentional discard path for unsounded accepted PCM.

The public header imports only `lib/types`. The implementation imports its own
private header and the public `lib/base` synchronization interface; the Audio
target has the sole admitted Lib edge `audio -> base` (plus `types`). No Core,
App, Common, guest or product symbol appears in the source. The worker is the
only live task that calls the platform stream; destruction joins it before the
platform sink is released.

The unchanged-import set for SoftPC is `src/lib/audio/`, the Audio target edge
in `src/lib/CMakeLists.txt`, the exact Audio dependency declaration in
`src/lib/verify_component_dependencies.cmake`, and the related Lib tests. A
SoftPC adapter keeps ownership of PCM generation and calls the public FIFO
interface only; no MyNes source is required. Its importer must update its
shared corpus manifests and run its own full suite. MyNes verified all 112
tests on x64 and x86. The native x86 probe treats an unavailable RDP waveOut
endpoint as unavailable hardware, while deterministic Lib and Win32-platform
tests still verify the contract.

## S7 Corpus Readiness Amendment

The follow-up SoftPC audit identified three remaining corpus-readiness gaps.
They are closed here. The worker's idle FIFO wait is now infinite and event
driven; the native backend waits indefinitely on either a WinMM completion or
a private interruption event. There is no periodic 50ms or 5ms polling path.
`clear` and destruction signal that interruption before waiting for the worker,
so a native wait cannot retain its worker or backend indefinitely.

The Audio CMake target and dependency verifier both declare `audio -> base` in
addition to `audio -> types`; a native Audio consumer links `audio` alone in
the test graph, proving this is not supplied incidentally by a final product.
The deterministic stream test holds a worker in native wait, fills its FIFO,
then proves a native write failure wakes a producer with terminal failure. It
also proves destruction interrupts a native wait, joins the worker, and avoids
a post-cancellation native write. The Win32 leaf test verifies completion and
interruption selection plus retry-safe native destruction.

With these amendments, SoftPC's only product adaptation is its Core/Compat
conversion from `LazyBeep(frequency, duration)` to typed PCM production. It
does not own WinMM slots, native wake events, or host playback timing.
