# System Architecture

This is MyNes's approved macro architecture. Apply the
[Architecture Rules](../../rules/ARCHITECTURE.md). Source organization belongs to
[Source Layout](CODING.md); delivery state belongs to [Current](../states/CURRENT.md).

## Product Shape

MyNes is one C11 NES emulator product, first hosted on Windows.
Its App/Common/Lib organization follows SoftPC, with one NES Core replacing
SoftPC's VM/Compat/MVDM backend responsibilities. Reusable Lib/Common mechanisms
are adopted from reviewed source rather than independently recreated.
There is no recovered x86 engine, compatibility ABI or VM selection framework.
Each NES machine has explicit instance ownership; no global current machine exists.

The approved implementation direction combines SoftPC's four-part product
composition with NXVM's machine-centered Core organization and coding idioms.
MyNes places these files directly in `src/app-mynes/core` and component tests in
`test/app-mynes/unit/core`; there is no additional machine directory in either tree.
Machine owns hardware assembly, bus wiring and time; adjacent device owners
retain their own state and narrow interfaces. Core machine driver integration remains
separate from deterministic hardware. The concrete NXVM-style file/header and
vocabulary mapping is defined in [Source Layout](CODING.md#nxvm-machine-style-reference).

Cooked Console owns management/debug interaction. KVM Window and ASCII KVM Console
are equally important supported gameplay backends with the same machine/control
contract and release acceptance weight. Neither is a fallback or optional demo.
Menus and graphical settings are outside the product. Implementation order may
follow prerequisites, but the planned dual-backend gameplay outcome needs both.

## Modules, Ownership, And Assembly

### Component Dependency Contract

Arrows below mean source dependency on public contracts:

```text
App composition ------> Common + Core public integration + Lib
App command/debug ----> Common + Lib + Core public protocol values
Other App ------------> Common + Lib
Common session -------> Common machine + UI
Common ---------------> Lib
Core machine driver ----> Common driver contract + Lib copied values/storage
Core machine driver ----> Core machine
Core machine/devices -> Lib types and deterministic C-runtime vocabulary
Lib leaves -----------> Lib types + explicitly declared neutral Lib support
```

Common never includes Core or App. App composition creates the Core adapter and
injects its machine driver into Common; this is the same real execution boundary
used by SoftPC. Runtime calls from the Common executor through that driver do not
give Common knowledge of NES internals. Other App modules never call Core lifecycle
or access its handle. App command/debug
may include the Core-owned value-only debug protocol and send it through Common;
this header introduces no Core runtime dependency or mutable hardware access.
Core's adapter implements the injected contract; it does not call Common lifecycle
operations or depend on Common implementation headers. Deterministic hardware
modules do not depend on the adapter, Common, host APIs, files, clocks or workers.
Lib contains no NES/application state or policy. No cyclic link dependency is needed.

### State And Resource Owners

| Owner | Owns | Does not own |
| --- | --- | --- |
| App config/command/debug/keyboard | Defaults/effective settings, command/debug semantics, NES key mapping and hotkey policy. | Native resources, executor or hardware state. |
| App composition | Entity construction, callback wiring, running session and ordered teardown. | A second dispatcher, command parser or run state. |
| Common session | One control FIFO, desired/actual reduction, lifecycle dispatch and prompt scheduling. | Executor state, native input or NES policy. |
| Common UI | Monitor logical Console, broker, both KVM presenters and copied input/presentation bindings. | Command interpretation, NES controller state or machine lifecycle. |
| Common machine | One executor, request/input queues, lifecycle completion, generations, frame publication and paused debug rendezvous. | NES CPU/bus semantics or host presentation choice. |
| Core machine driver | NES machine construction/load transaction, driver implementation, input/debug adaptation, pixel/text frame production including graphics-to-text conversion, and hardware teardown. | Product control queue, native resources or a second executor. |
| Core machine | Hardware assembly, power/reset sequence, guest master time and device/interrupt scheduling. | Commands, host pacing or host surface selection. |
| Core CPU | Registers, decode/execute state and CPU-local interrupts. | Address-map policy or host access. |
| Core bus | Sole address routing and ordered memory/device side effects. | Shadow device state or CPU execution. |
| Core cartridge | ROM parsing, immutable content, mapper state and cartridge RAM. | Paths or file handles. |
| Core PPU/APU/controller | Guest device state and pixel/audio/controller behavior. | Native output or keyboard APIs. |
| Lib service | Neutral capability and its native resources. | Product lifecycle, machine state or NES semantics. |

Snapshot capture and restore follow the same boundary. Core owns the portable
NES image: cartridge identity and immutable-reference facts, plus copied CPU,
bus RAM, PPU, APU, controller, mapper and execution state. It never opens a
path or chooses a user-visible lifecycle transition. App owns `save <file>` /
`load <file>`, file streams and atomic destination replacement. Common provides
the serialized executor boundary and state-byte transport without interpreting
the bytes. A load first decodes a bounded candidate, validates its cartridge
reference and only then replaces live Core state; a failed decode or validation
leaves the live machine untouched. This image is MyNes-specific, not a Common
or Lib ABI.

Session's desired actions and last completed facts are projections, not a second
executor state. Common machine alone admits/executes lifecycle requests and reports
completion. App does not maintain another running flag. UI returns copied events
through its injected sink and never invokes session processing inline.

Core machine wires CPU bus access and copied interrupt facts through narrow
contracts. CPU tests may inject that same bus contract. Devices never discover
or mutate peers; the bus never executes the CPU. The Core machine driver is a real
machine boundary, not a forwarding wrapper: it translates NES input, output,
loading and debug semantics into shared contracts. It owns no additional run flag.

### Lib And Shared Common

| Lib component | Responsibility and permitted support |
| --- | --- |
| types | Header-only scalar/status/C and platform declaration vocabulary. |
| base | Clocks, waits, synchronization and worker primitives; uses types. |
| storage | Files and bounded byte storage; uses types. |
| console | Logical text/input endpoint; uses types/base; no native Console ownership. |
| console-broker | Sole native Console owner and raw/cooked binding transaction; uses types/base/console. |
| kvm-base | Copied frame/input, source identities, neutral key/hotkey and private mailbox mechanics; uses types/base. |
| kvm-window | Independent Win32 window/message lifetime and pixels/input; uses types/kvm-base. |
| kvm-console | Raw logical Console, text-frame cell presentation and input; accepts text frames only; uses types/base/console/kvm-base. |

KVM Window never calls Console or its broker. KVM Console uses logical Console
output; Common UI requests broker binding. Audio is a separately admitted leaf,
not hidden inside KVM. Native workers use Lib primitives; Common has no platform
directories, SDK types, native calls or OS-selected implementations.

Lib/Common and their tests form one transferable neutral corpus. SoftPC is the
initial source; the target after reviewed changes is identical shared code usable
by MyNes, SoftPC and NXVM. There are no MyNes-specific branches inside that corpus.
Each receiving product owns composition, machine semantics and debugger binding.
Adapters may need changes when the shared contract changes; unchanged shared
source is the guarantee, not source/binary compatibility of every old adapter.
Independent shared tests use fake providers and no NES/x86 engine, ROM or App.
The four-tree placement is defined only in Source Layout.

Lib is reused unchanged from one reviewed upstream baseline, including its tests
and manifests. MyNes work does not patch Lib for its own frame formats, character
mapping or machine semantics. The adopted Console contract accepts explicit copied character maps. MyNes
supplies ASCII mappings and does not depend on an implicit CP437 conversion.
Required text-only submission behavior belongs to the adopted shared baseline.
Any new Lib revision needs explicit pinned import and verification before use.
A missing required Lib contract blocks the dependent acceptance and is returned
to its upstream owner, rather than creating a local fork. Common preserves its
existing mechanisms wherever neutral; repairs require an evidenced contract gap.

Common machine owns executor lifecycle, synchronization and publication. Common
session owns ordering/reconciliation; Common UI owns host-surface composition.
Debug command language, assembler/disassembler and concrete register/address
records are product capabilities outside this neutral trio. SoftPC/NXVM can
reconnect their own x86 debugger through the same paused request boundary that
MyNes uses for NES debugging. The extracted components are x86-debug and
x86-xasm32, outside both Common and Lib. x86-xasm32 owns x86 assembly/disassembly;
x86-debug owns command/register/address vocabulary, codecs and output formatting,
and may use x86-xasm32 and Common's public neutral debug transport. Common never
includes or links either component by default. The receiving SoftPC/NXVM App
constructs and binds them; MyNes does not include them in its default product.

The real CPU/memory/port operations belong to the receiving machine adapter and
run on the existing Common executor. The App wires this path:
command -> x86-debug -> Common transport -> machine debug adapter -> result ->
x86-debug formatting. Neither x86 component obtains an unmediated mutable machine
pointer or creates another execution worker. Their product protocol is separate
from Common; machine adapters may use its value declarations without linking a
frontend parser. Shared-source reuse does not require unchanged old adapters.

A debug request has a bounded copied payload/result, explicit operation identity,
legal states and lifetime. Common validates transport/state/lease; the product
driver validates semantics and access safety on the executor. No arbitrary
frontend function pointer is executed as a request, and no universal CPU layout
is imposed. Common defines bounds, copying, ownership, operation identity,
allowed states, cancellation and typed transport errors. Product protocols define
operation meanings and request/result layouts, including register snapshots;
Common does not define their fields or interpret their payloads. An opaque content
contract is not an unbounded void pointer or a renamed x86 structure. MyNes's
exact sizes/signatures are defined in the detailed App/Core contract.

Byte storage, optional media replacement and state byte-stream transport remain
typed neutral capabilities where their semantics already fit. Drive geometry,
floppy/hard-disk identity, controller behavior, cartridge formats and state-image
interpretation stay in machine adapters. Inventory any single-floppy assumption
before preserving a media hook; do not introduce a universal disk framework or
convert every existing typed API to opaque debug messages.

Common retains mode selection from the published frame kind and existing display
settings. A graphical frame may select the Window path; a text frame may select
the raw Console path. This shared mechanism remains useful for machines with
native graphics/text modes and is not replaced by producer-format negotiation.
MyNes always generates graphics in its PPU, but its Core adapter publishes pixels
for Window gameplay or converted text for Console gameplay. Common sees only the
published kind, not the guest's original hardware mode or conversion provenance.
No NES-specific branch or new format-negotiation service is needed in Common.
App settings reach Core output adaptation through the product's serialized adapter
control path, whose concrete contract is fixed at integration admission. This is
not direct frontend access to a running Core. Common continues to own publication,
mode transitions, broker handoff and input reset; Lib only presents accepted data.

Compatible source, names and layout are retained. A neutral extension changes
its existing owner and accompanying tests once. Planning identifies a pinned
four-tree source/test manifest and the current publishing owner; receiving
products validate an identical version and reconnect external adapters.
Transfers are explicit snapshots, never runtime/build access to a sibling repo.
Actual upstream/downstream repository edits require their own admitted work.

### Execution, Control And Debug

The product uses one control thread in Common session and one execution worker
in Common machine, matching SoftPC's architecture. Only the executor calls the
driver to mutate guest state after publication. Native readers/KVM workers enqueue
copied events; they never execute commands, debug access or Core operations.
Construction/destruction of unpublished or joined objects is serialized separately.
Driver stop/wake signal hooks may be called by the control thread; they only
publish thread-safe requests and cannot mutate guest hardware or reenter lifecycle.

The control FIFO orders UI and completion facts. The machine queue orders accepted
lifecycle, input and debug work on its executor. These are distinct owned queues,
not alternative command paths. Requests are not completion: session changes its
actual view only from returned/completed facts of the matching generation.
The machine input queue is fixed-capacity; session control FIFO grows and
reports allocation failure. MyNes does not claim a hard bound for that unchanged
shared FIFO. Product work/messages are bounded; shutdown uses the permanent
stop/join path and does not wait for a product command to execute on the guest.
Control ordering uses one Common-owned mechanism per queue; independent media/
debug rendezvous must not become competing mutation or completion authorities.

Core executes finite hardware slices and reports consumed work, boundary and
typed stop/fault reason. The driver services the existing executor callback/wake
contract between slices so pause, input and shutdown are bounded. Future host
pacing belongs to Core machine driver integration on the existing executor,
using Lib clocks; deterministic Core hardware never reads host time. Guest time
comes only from hardware rules and explicit inputs. An instruction-boundary cycle
budget names its allowed final-instruction overshoot; cycle totals do not prove
cycle accuracy. Common delegates heartbeat to its driver and needs no pacing extension.
M2 finite execution slices provide control responsiveness, not real-time speed;
real-time pacing is admitted with M4.
Product heartbeat remains an optional driver capability, not a shared guest clock.
The detailed App/Core contract fixes slice, wake, debug and failure bounds.

The required semantic states are no cartridge, loaded/paused, running and faulted;
Common public states remain authoritative; guest stops are copied Core facts
within PAUSED, while host failure is Common ERROR. Load enters paused.
Pause retains the loaded machine at a safe boundary; resume requires paused.
Shared stop ends the run; cold start may reset hardware, matching SoftPC semantics.
If a MyNes command means suspend-and-continue, App dispatches pause, not stop.
Unload/replacement is distinct from stopping; stop does not implicitly discard ROM.
Step requires paused and stays paused on success. Reset completes hardware reset
before reporting paused. A guest trap stays PAUSED and permits reset/reload;
a host fault enters ERROR, which rejects reset/media. Host recovery requires
orderly exit and process restart with a fresh Common/Core composition. Invalid
requests do not change state. Shutdown is permanent stop/join, distinct from stop.

Machine-dependent debug runs through the existing paused executor rendezvous,
with lease/generation revalidation there. No frontend directly touches Core, even
when paused. A read snapshot copies facts; diagnostic peek is side-effect-free or
explicitly unsupported. Guest bus reads retain production side effects. Core
never prints or picks process exit codes. Operation errors and guest stops are
different typed results. Execution faults report completed work, not rollback.

### ROM Loading And Public Values

App selects the path; the serialized Common machine request invokes the NES
adapter at a safe boundary. The adapter uses Lib storage for bounded bytes; Core
cartridge validates the format and Core machine constructs a candidate owning
its ROM and mutable hardware. Only a complete candidate is published. Failure
releases temporary resources and preserves the previous accepted machine/state.
Replacement requires Common STOPPED or PAUSED, never RUNNING or ERROR. Common does
not parse ROM headers or own mapper policy. Reset failure after mutation reports
faulted; transactional load does not imply rollback of hardware execution.

Public stateful handles are opaque. Every call declares allowed states, ownership,
bounds, output validity, reentrancy and failure effects. Read-only spans are borrowed
only for the call unless explicitly copied; no stack context survives implicitly.
Snapshots leave output unchanged on failure. Stream operations report partial
progress when their contract allows it; unsupported behavior cannot return empty
success. Copied frames carry content, sequence and generation consistently.

### App/Core Production Integration

App composition creates product options and command/debug providers, the Core
adapter, then Common machine, session and UI with explicit borrowed callback
contexts. Session exists before UI can emit events; composition then binds UI
and machine sinks.
It runs the single Common session loop. App main selects exit policy only.
Common never discovers Core, and Core never starts a worker or a monitor reader.

| Boundary | Required behavior |
| --- | --- |
| App config | Own defaults and accepted effective settings; validate candidate values before dispatch. M2 has no automatic persistent configuration. |
| App command | Parse one copied cooked line and submit through the session provider. Return at most one lifecycle request for session dispatch; never call it once directly and once via the returned action. |
| App debug | Encode NES requests and render copied replies. Obtain the Common paused lease; invoke its bounded opaque rendezvous. No direct register or bus pointer. |
| Core machine driver | Implement Common's driver; own ROM I/O transaction, copied configuration, stop/wake signals, input/output and NES debug translation. |
| Core machine | Own the accepted hardware instance, reset, deterministic slices and guest time. CPU accesses memory through the production bus. |
| Cartridge | Parse bounded bytes, validate supported format/mapping and own immutable ROM plus mapper/RAM state; no host path handling. |

The adapter run function invokes the executor callback before executing the first
instruction and between finite slices. This permits Common cold reset to finish
in PAUSED without retiring a guest instruction. A callback may park the run stack
and service media replacement; therefore no pointer into the old hardware may
survive across it. Re-read the adapter's accepted instance after each callback.
Stop/wake hooks are signal-only and thread-safe. Normal stop unwinds successfully;
guest traps request a debug-stop PAUSED fact and remain observable; host failures
return failure after preserving any available copied facts. Neither
App nor adapter keeps a second authoritative lifecycle state.

Loading is serialized on the session control thread through Common's removable
media operation. Existing Common accepts it only in STOPPED or PAUSED, dispatches
it on its worker and returns a boolean. App first verifies those allowed states.
Core machine driver reads through Lib storage, validates and constructs a complete candidate,
then swaps its owned hardware. Parse/allocation/I/O failure preserves the previous
machine. Generic failure must not be fabricated into a detailed parse diagnosis.
A richer copied diagnostic requires an explicitly designed product protocol.

After successful attachment, App requests reset through session; it reports
loaded-and-paused only after RESET_COMPLETED, not after request admission. Reset
failure after a committed attachment faults the new machine; it does not promise
rollback to the prior cartridge. Resume continues paused hardware; start from
STOPPED is a cold start; pause is the user-facing suspend operation. Stop retains
the cartridge. Null media unloads only at an allowed boundary; subsequent run
without a cartridge is rejected. M2 exposes no snapshot persistence.

Common ERROR is terminal for start/reset/media. The product offers help/exit in
ERROR and requires orderly process restart for host recovery. Successful
stop/join precedes destruction; no in-session hidden reconstruction is offered.
Never attempt a hidden state edit or driver call to bypass Common. A failed join
retains borrowed dependencies and makes recovery unavailable.

The product debug protocol is pointer-free and bounded by Common's existing
128-byte request and 1536-byte response capacities. It names version, operation,
lengths and typed results; byte layouts are specified in the detailed contract.
Register observations are copied. Peek must be side-effect-free or unsupported;
step executes through the same CPU/bus path while paused, with a finite bound.
Expected debug stops and operation errors are separate from terminal host faults.
Lease and generation checks remain Common-owned. Failed rendezvous outputs are
not usable; an indeterminate wait requires successful shutdown before further use.

M2 produces no PPU frame: copy_frame returns the documented invalid-frame value,
not fabricated pixels or an ASCII status screen. In M3, PPU images feed Core machine driver
pixel/text adaptation; Common still selects routing from the published frame kind.
Window and Console remain equal acceptance targets without changing shared code.

## Product And Host Boundary

### Cooked Management, Debug And Configuration

Common UI owns the monitor endpoint and one native broker. Session owns one pending
cooked line and prompt scheduling. App consumes complete copied command lines
through its provider; debug uses that same path. Help can run while the guest
executes; configuration remains the startup-only `mynes.ini` file. Machine debug
requires paused executor access.
Cooked editing waits only in its Lib worker, so it cannot block guest execution.

App owns built-in defaults and effective configuration. A command validates a
candidate and requests application through session. Publish the new effective
value only after success; reject unsupported live changes explicitly. Failure
preserves the prior setting/binding, or reports terminal host failure if restoration
itself fails. Persistence is explicit, not a hidden consequence of key changes.
Remapping first releases old inputs and invalidates stale queued input generations.

Notifications use one monitor-output transaction: cancel/join an unfinished read
through the broker before output and rearm exactly one read. A completed queued
line is consumed once; no producer prints over editing or creates another reader.
Unfinished-line interaction is defined in [Product UX](UI.md).

### Equal KVM Backends And Console Handoff

Both presenters display the same machine through different complete frame
representations and emit the same neutral input/hotkey events. Window receives
pixels. KVM Console always receives text frames containing character cells and
optional cell colors; it has no graphics input, pixel sampling or ASCII conversion
path. A graphical frame submitted to it is explicitly rejected without changing
the last accepted text frame; silently skipping it or reporting success is invalid.

Core's machine display implementation converts a completed guest pixel image to a text frame
for ASCII gameplay. The PPU still produces the normal guest image; conversion
does not change hardware timing, pixel resolution or emulated state. It runs at
the existing executor/driver boundary with bounded work and copied format options,
without host Console APIs, native surface ownership or a second execution worker.
App owns user settings; Core's adapter applies the selected output representation
at its serialized machine boundary. Common needs no conversion request or format
selection API for NES. Native UI callbacks never call Core conversion directly.

The converted text frame follows the existing Common machine publication and UI
routing path to kvm-console. Content kind, extent, sequence and run generation
identify one complete representation of the source image. Common's existing
optional graphical-mode status text may remain for products that use it; it is
not the MyNes ASCII gameplay path. Core publishes a genuine text frame for that
path, so no replacement of Common's graphical-mode policy is required. Failed
conversion or unsupported format returns an explicit failure without publishing
a partial/stale representation or altering the accepted binding. Surface changes
must not reuse an incompatible cached representation. The detailed contract fixes
frame capacities, copied settings, color quantization and conversion limits.

M3 proof covers Core sampling, cell aspect, glyph/color choices, deterministic
output for fixed pixels/options and bounded conversion, plus text delivery through
the real production route. Common/Lib tests use supplied text fixtures and verify
routing, bounds and graphic rejection without importing a pixel-to-text converter.
A status text or discarded graphic is not ASCII gameplay.

Window and cooked management can coexist. Console gameplay uses raw input and
leases the same native Console exclusively against the cooked monitor. App selects
Core output representation while paused. Session stays on its automatic
frame-kind route with cooked graphics control enabled, and UI performs broker
replacement when the new complete frame is published after resume.
Retire/join the old reader, reset held inputs/chord prefixes, change native mode
and activate the next endpoint. Failed replacement restores the former binding or
reports terminal failure; two native readers cannot coexist. The broker preserves
the cooked buffer/cursor/scrollback through raw output. A reserved return action
pauses and restores the monitor. Switching surfaces never reloads the guest.

Both backends support the same admitted controller, pause/resume, reset, debug
return, configuration and shutdown semantics. Native differences affect rendering
and focus mechanics, not product priority. Once dual-backend gameplay is admitted,
a failed backend blocks its acceptance; Window success alone cannot close it.

### Input, Frames And Teardown

Lib normalizes physical source events and recognizes App-registered hotkeys.
UI/session preserves their source/order and routes accepted copied input to the
machine queue. The NES adapter maps ordinary keys using App's copied settings
into controller buttons on the executor. Cooked text never enters this path.
Hotkeys are consumed before guest mapping and cannot silently conflict with keys.
Focus loss, source retirement and raw/cooked handoff release held state; queued
generation checks and Core's bounded resume input drain prevent stale makes
from reviving a key. Composition's runtime sink signals Core input reset before
posting pause/stop/error facts; only the executor applies it. This accommodates
Common's actual paused release suppression without altering its source. Lib owns its
local key/chord state; session owns accepted-source routing state;
the driver owns the mapped guest-button state. Reset crosses these boundaries in
order, including while paused and before resume. Source reset is not permanent
retirement. Per-source ledgers prevent one release from clearing another source.
Only the selected
KVM surface delivers gameplay input. Product key policy is in Product UX.

Frames may coalesce to the latest complete image; control/input may not adopt that
dropping policy. Audio has a separate bounded stream contract. A frame notification
is a hint to copy the current generation's snapshot, not a lifecycle completion.

App requests session shutdown with all callback targets alive. Admission closes;
Common machine stops/joins its executor while session/UI sinks still exist. UI
then retires/joins KVM and broker producers and restores native Console state.
After producer quiescence, dispose queued events, UI, session and App providers;
destroy the retained Common machine, then its Core adapter/hardware. A worker that
cannot join retains its resource/sink dependencies and reports terminal failure.
Never free borrowed driver context while the executor can still call it.

## Runtime Admission Boundary

NXVM supplies governance/style; the shared foundation is adopted unchanged from
SoftPC. Current records its exact source/evidence. The inherited x86 separation
is already supplied upstream; no local extraction is a prerequisite for App/Core.
Optional absent capabilities report unsupported, never empty success.

Source import and implementation follow [Execution](../../rules/EXECUTION.md) and
the [source policy](../etc/operations/policy/source-policy.md). Design does not
create source or an executable. M2 design consumes the actual adopted interfaces;
later capability planning belongs to its own milestone admission.

### Detailed Design Contracts

The [App/Core contract](../etc/app-core-contract.md) derives concrete APIs,
commands, lifecycle and debug values from this ownership map. The
[CPU/bus/cartridge contract](../etc/cpu-bus-cartridge.md) fixes the complete first
execution profile and [opcode ledger](../etc/cpu-opcodes.csv).
[Hardware/presentation detail](../etc/hardware-presentation.md) defines device,
output, input, audio and persistence seams without admitting later tasks.
[Reference provenance](../etc/hardware-references.md) distinguishes original MOS
archives, NES hardware research, actual shared ABI and MyNes product choices.
[Design/proof mapping](../etc/app-core-verification.md) is the review checklist.

The admitted M3 [concrete design](../etc/m3-design.md) specializes these seams.
The [M2 remediation ledger](../etc/m3-remediation.md) identifies delivered
violations of this architecture that M3 must correct; historical closure does
not waive component boundaries or failure ownership.

Core guest state is deterministic; RGB/text output may deliberately approximate
physical display. Lib's current 256-color/80x25 frame bounds are preserved.
Future sound and atomic persistence require neutral upstream capabilities not
present in this shared snapshot. App/Core cannot implement native workarounds
or silently patch shared code to bypass those prerequisites.
