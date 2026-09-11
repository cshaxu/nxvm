# M5 Common Product-Runtime Convergence

## Purpose

Make NXVM's host-side single-machine runtime and its reusable x86 Debug
product capability shareable with SoftPC without moving NXVM Core, firmware,
profile, native-platform details, or either product's machine implementation
into a shared layer.  The resulting common corpus has a small neutral runtime
layer plus two bounded x86-domain components, all above the existing canonical
`src/lib` host services.

This is not an import of SoftPC's current `app/runtime` implementation.  That
code still calls SoftPC-machine APIs directly, just as NXVM's current
`vm/events` records still carry NXVM/Core-shaped values.  Both products are
evidence for the common contract, not inputs to a copied parallel runtime.

## Required NXVM End State

```text
src/lib/                         platform and generic host services only
src/common/                      shared product capability; depends only on lib
  contracts.h                    copied values and opaque adapter contracts
  ui/                            lib UI binding and copied host-event return
  session/                       lifecycle/control FIFO and reducer
  machine/                       safe-point executor bridge and FIFO
  xasm32/                        x86 assembler/disassembler; lib/types only
  debug/                         x86 Debug CLI and neutral paused-target contract

src/core/
  machine/                       Core execution and machine state
  product/                       Core product services

src/vm/
  machine/                       NXVM adapter for core/{machine,product,debug}
  product/                       YAML/profile/CLI/debugger product policy
  app/                           NXVM-only composition root and routing
```

`common` has no `runtime/` component.  Creating and connecting `common/ui`,
`common/session`, and `common/machine` is composition, not a fourth reusable
state owner.  NXVM `vm/app` and SoftPC's corresponding product composition
each remain the one place allowed to know all three components.

`common/ui`, `common/session`, and `common/machine` are neutral runtime
components.  They may include `lib` and copied declarations in
`common/contracts.h`; they do not include or call one another, `xasm32`, or
`debug`.  `core`, `vm`, SoftPC, MVDM, CPU, BIOS, ROM, profile, controller,
guest, native handles, and platform headers are forbidden from that neutral
runtime subset.

`common/xasm32` and `common/debug` are deliberately separate: they are shared
*x86 product capabilities*, not generic runtime mechanisms.  `xasm32` depends
only on the current `lib/types` public definitions (`lib_u*`, `lib_size`,
`lib_status` and, where needed, lib atomics); it has no platform or higher-layer
dependency and must not redefine those types.  `debug` may depend only on
`lib/types`, `xasm32` and the synchronous paused-debug API declared by
`common/machine`; it cannot include Core, VM, MVDM, UI, Console, storage,
platform, or native headers.  Its two intentional internal dependencies are
downward: `common/debug -> common/xasm32` and
`common/debug -> common/machine`; neither is reversed.

`common/debug` owns bounded Debug command parsing and continuation state, not
a second input/result queue.  It is itself a `common/session`-registered CLI
provider: when Debug is active, session delivers its ordered monitor line to
that provider and writes the returned copied text and prompt to the monitor
Console.  Product CLI and Debug CLI are peers registered with session; neither
wraps or calls the other.

## VM Machine Adapter Boundary And Planned Extraction

`vm/machine` is deliberately the NXVM-only Core adapter, not an attempted
second common runtime. It retains facts that cannot be shared without making
common know NXVM/Core semantics: Core creation and plan materialization,
profile-selected firmware and controller topology, Core media/display/input
provider binding, the actual Core execution runner and timing advancement, and
translation between Core callbacks and copied machine facts. Its
`common/machine` driver endpoint likewise remains: it maps neutral safe-point
and paused-Debug requests to `core/machine`, without exposing a Core pointer.

The current adapter is larger because it also contains product and
presentation responsibilities. They have explicit deletion receivers; none is
a reason to enlarge `common/machine`:

| Current mixed responsibility | Final owner | Receiver |
| --- | --- | --- |
| Core Debug parser, target callback table, command continuation and shared breakpoint/trace policy | `common/debug` | S5 |
| NXVM-only raw instruction recorder and file policy | `vm/product` optional capability | S5 audit |
| YAML/session request parsing, profile choice, asset-path validation and user-visible machine/status text | `vm/product` | S5/S7 |
| presenter binding, surface target/title/mouse application and copied UI input/facts | `common/ui` plus product presentation policy | S6 |
| composition-only route selection among product, session, machine and UI | `vm/app` | S7 |
| old `vm/events` carrier values and product/host bridges | named common contracts or their sole product receiver | S7 |

Split VM files only when doing so removes one of these mixed responsibilities
or a duplicate route. Profile-selected Core assembly, media bridges, firmware
providers, execution/pacing and Core-fact adaptation remain adjacent under
`vm/machine`; moving them into common or splitting solely by file size would
add indirection without reducing ownership.

For paused inspection/editing, Debug synchronously calls the narrow
`common/machine` paused-debug API.  That API exposes operations such as
snapshot/read/write/breakpoint and install-execution-plan, but never exposes a
machine pointer or layout.  `common/machine` delegates through its single
product-driver endpoint; `vm/machine` maps that endpoint to `core/machine`,
and a SoftPC VM/MVDM adapter maps the same endpoint to SoftPC.  This is the
deliberate path `core/machine -> vm/machine -> common/machine -> common/debug`
for NXVM, with no direct `common/debug -> core` or per-command product parser
forwarder.  Session remains independent: it knows only a registered CLI
callback, not Debug types.

Core machine or the SoftPC adapter validates the paused lease at the bottom of
that path and invalidates it before resume/reset/stop.  `common/machine` also
rejects a synchronous Debug operation unless its own lifecycle state is
paused.  Lifecycle-changing Debug commands never run a machine directly: they
return a copied ordinary lifecycle request, and session enqueues it through the
ordinary machine route.  The target owner enforces execution plans, actual
retirement accounting and the resulting paused/stopped fact, which it reports
back to session.  Session gives that fact to the active Debug provider when it
needs Debug-local state transition, then prints the returned text/prompt.  This
gives SoftPC the same Debug route without making either machine implementation
public.

`N`, `L`, and `W` remain Debug grammar, but Debug itself never opens a host
file.  Their result is a bounded typed file request handled by each product's
injected file service.  That service uses `lib/storage` (or a later shared
binary-stream extension) and returns copied bytes/status to Debug.  This keeps
host paths, access policy and file handles out of both `xasm32` and Debug.

The DOS-style Debug trace/step command is mandatory Debug functionality: its
execution plan is installed through `common/machine`, and each target owner
enforces it.  NXVM's separate raw instruction recorder is not silently
promoted to common.  A dedicated later audit must compare it with SoftPC's
available instruction observation before introducing a shared copied trace-sink
contract.  If SoftPC cannot provide it, NXVM retains the recorder as an
explicit optional product capability; it is not removed merely because SoftPC
lacks it, and a fake shared recorder is prohibited.

## Ownership And Data Flow

```text
monitor Console / native UI / machine completion
                  |
                  v
       common/session control FIFO       (sole ordered ingress)
          |                    |
          |                    +--> registered CLI provider
          |                              |        |
          |                         text/prompt   paused Debug target
          v                                       v
common/machine FIFO                         common/debug --> common/machine
safe-point executor
         |
         v
NXVM Core adapter --> copied machine facts --> common/session
```

There are deliberately three different transports, each with one owner and
consumer:

1. `common/session` owns the bounded FIFO for ordered facts and lifecycle
   commands: start, pause, resume, reset, stop, native UI input, machine
   completion, presentation completion and delivery failure.  A full queue is
   an explicit failure; no fact may be silently discarded.
2. `common/machine` owns the bounded FIFO crossing to the executor thread.  It
   accepts copied requests only and consumes them at its product driver's
   declared safe point.  It is not a second session FIFO: it has a distinct
   thread boundary and no lifecycle-policy authority.
3. Frames are latest-wins copied state, not ordered control.  Session owns its
   latest-frame ingress and routes the accepted current-run frame to
   `common/ui`; the underlying lib presenter owns its own latest-frame slot.
   A frame cannot flood or reorder lifecycle control.
4. `common/debug` owns parser continuation state only.  As one registered CLI
   provider, it synchronously calls `common/machine` for bounded paused-state
   read/write operations and returns copied results to session; no second Debug
   transport or queue exists.  Lifecycle-changing Debug commands yield copied
   ordinary lifecycle requests for session to submit.  Thus `GO`, `TRACE` and
   `STEP` follow the ordinary session lifecycle route, while Core machine owns
   the actual execution budget and stop fact.

Every cross-thread request, completion, input event and frame carries a
non-zero `run_id`.  `common/session` owns incrementing it at a successful new
run and rejects stale facts before they can affect a later run.

`common/ui` applies only copied plans (`target`, `title`, mouse policy and
`ui_frame`) through `lib`.  It returns copied UI input and component facts; it
does not choose a target, pause a machine, parse commands or access a native
handle.  The existing `lib` mailboxes remain private implementation details.

## Public Common Contracts

The final names may follow the codebase's C naming convention, but the public
shape is fixed:

```c
common_status common_session_submit(common_session *,
    const common_session_command *);
common_status common_session_publish_fact(common_session *,
    const common_session_fact *);
common_status common_session_take_effect(common_session *,
    common_session_effect *, unsigned timeout_ms);

common_status common_machine_submit(common_machine *,
    const common_machine_request *);
common_status common_ui_apply(common_ui *, const common_ui_plan *);
```

`START` carries an opaque, copied product-owned start token, never YAML,
firmware/media paths or a product pointer.  `common` asks its adapter to act
on that token; NXVM `vm/product` is solely responsible for resolving it to a
frozen session request.  Product-specific command grammars and user-visible
messages remain in `vm/product`.  At construction `vm/product` registers
NXVM's ordinary monitor CLI provider with `common/session`.  `common/debug`
registers a separate Debug CLI provider when the user enters Debug.  Each
provider returns text, the next prompt and any ordinary lifecycle request;
session owns provider selection, ordered line delivery, machine-fact delivery
and monitor Console output.  It may emit product-neutral lifecycle status text
itself.

Console presentation policy is the same kind of injected product policy.  It
maps a session snapshot and copied display fact to `VM_RAW`, `MONITOR_COOKED`
or `NONE`; `common/session` orders and applies that plan through an opaque UI
port, while `common/ui` performs the native action.  SoftPC may choose
`console_control`; NXVM may choose different rules without putting either
product's policy in common.

The machine adapter is a small product-supplied callback table: create/start,
safe-point request delivery, stop/leave and copied fact publication.  Its
context is opaque to common.  NXVM `vm/machine` alone supplies that adapter and
owns Core assembly; neither `common` nor `lib` sees a Core pointer.

When the machine is paused, `common/machine` accepts synchronous bounded Debug
operations through its approved paused-debug API.  Its product-driver endpoint
delegates NXVM operations through `vm/machine` to `core/machine`, or delegates
SoftPC operations to its VM/MVDM adapter.  `common/debug` calls that API;
neither product CLI nor `vm/machine` parses or forwards individual Debug
commands.  Debug never changes lifecycle directly.

The shared Debug shape is likewise explicit and independent:

```c
common_debug_status common_debug_create(common_debug **);
common_debug_status common_debug_open(common_debug *, common_machine *);
common_debug_status common_debug_submit_line(common_debug *,
    const common_debug_line *, common_debug_result *);
common_debug_status common_debug_observe_machine_fact(common_debug *,
    const common_debug_machine_fact *, common_debug_result *);
void common_debug_close(common_debug *);
void common_debug_destroy(common_debug *);
```

`common_machine` exposes the target contract as synchronous bounded operations,
not as a pointer returned to a caller.  Its lease token is opaque, bounded and
validated.  NXVM Core machine and a SoftPC VM/MVDM adapter each implement the
bottom endpoint, issue a lease only while paused, and invalidate it before
resume/reset/stop; no function exposes mutable Core or MVDM layout.  Each
result can contain copied text/data plus at most one ordinary lifecycle
request.  Debug close clears only Debug-local pending work. `common/session`
and `common/machine` remain sole owners of product and executor lifecycle;
session activates Debug only after a paused fact.

## Explicit Non-Goals

- No `common/runtime`, generic machine framework, profile parser, native API
  wrapper, second console broker, or compatibility shim.
- No Core instruction, device, timing, debug, firmware, media or storage
  semantics change.
- No move of product command text, YAML policy, profile selection, machine
  implementation, or SoftPC MVDM behavior into common.  `common/debug` owns
  only the shared x86 Debug grammar and target contract.
- No parallel old/new route remains after each migrated ownership batch.
- This NXVM task does not modify the sibling SoftPC repository.  A later
  owner-approved SoftPC task may import the verified common corpus byte for
  byte and bind its own adapter.

## Subtask Plan

1. **S1 - actual-route and portability ledger.** Inventory NXVM's current
   `vm/events`, `vm/session`, `vm/machine`, `vm/presentation`, `vm/product`
   and Core routes against SoftPC's read-only control/runtime/reconciler
   responsibilities.  Record every state owner, queue producer/consumer,
   threading boundary, product-only field and deletion receiver.  Freeze the
   common public vocabulary, `run_id` rules, failure behavior and standalone
   CMake/manifest plan before source moves.
2. **S2 - xasm32 extraction and Debug-contract freeze.** Move only the x86
   assembler/disassembler into `common/xasm32` with `lib/types` as its sole
   dependency, retaining one caller route and its current behavior.  Freeze
   the synchronous `common/machine` paused-Debug API, the registered CLI
   provider contract, `N/L/W` product file-service exchange, and recorder
   disposition before moving the Debug parser.  This order prevents a temporary
   Debug-to-Core shortcut or a speculative target wrapper.
3. **S3 - common contracts and session cutover.** Create the independently
   buildable common corpus and implement the neutral copied ABI plus the sole
   bounded session FIFO, run-generation validation and lifecycle reducer. Add
   only copied monitor/machine/UI facts and lifecycle plans. Session includes no product, UI,
   machine or Debug implementation. NXVM immediately binds that owner through
   a minimal `vm/app` composition leaf and deletes the old `vm/session`
   lifecycle/FIFO/reducer route in the same S; the YAML catalog moves to
   `vm/product`. S5 introduces the live CLI-provider registration together
   with `common/debug` and the product-CLI cutover, rather than exporting an
   unused callback API in S3. The binding owns no queue, lifecycle state, mirrored fact or
   retry policy.
4. **S4 - common machine bridge and paused-Debug API.** Implement the neutral
   safe-point machine request FIFO and opaque driver contract.  Move NXVM Core
   assembly plus its paused-Debug endpoint implementation into `vm/machine`;
   delete the old executor transport at the same time.  Verify that
   `common/machine` accepts synchronous Debug operations only while paused,
   while the Core target owner invalidates leases before run/reset/stop,
   alongside ordered requests, private wakeups, stale-run rejection and fault
   propagation.  The S4 public API is restricted to copied input/lifecycle
   requests and bounded register, port or at-most-32-byte linear Debug
   operations; the Debug parser and its legacy callback-table consumer remain
   the explicit S5 receiver.
5. **S5 - common Debug migration and NXVM CLI cutover.** Move the line/fact
   Debug CLI into `common/debug`, bind it as a registered session provider, and
   route every paused read/write through `common/machine`.  Move the existing
   command handlers and their semantics, then delete the former Core Debug
   parser address and VM callback-table route rather than forwarding through
   compatibility headers. Move shared Debug continuation, breakpoint and trace
   policy with those handlers; retain only the bounded Core-operation mapping in
   `vm/machine`. Preserve DOS-style trace/step; use an injected product file
   service for `N/L/W`; move the NXVM-only raw recorder and its path/open
   policy to `vm/product` as an explicit optional capability unless the
   separate portability audit proves a shared sink. Move product command text,
   YAML/profile request parsing and machine/status rendering out of the VM
   adapter whenever the Debug/CLI cutover reaches their route.
6. **S6 - common UI binding.** Move the generic lib-presenter binding to
   `common/ui`, with copied plans/facts only.  Retain one session-driven
   control Console port while `vm/product` supplies NXVM's immutable
   raw-VM/monitor/none policy and Console text.  Delete the obsolete
   `vm/presentation` path and any VM adapter presenter binding. Keep only the
   Core display-provider/mailbox adaptation in `vm/machine`. Verify
   Console/Window switching, title, target, focus, mouse and latest-frame
   semantics without native API leakage.
7. **S7 - NXVM product and app completion.** Make `vm/product` own only NXVM
   command/YAML/profile/debugger/presentation policy, injected into session at
   construction.  Make `vm/app` the sole composition root, with no queue or
   lifecycle/router state. Delete the remaining old `vm/events` and
   product/host bridge paths rather than retaining adapters; S3 already deletes
   `vm/session`. Complete the similar-route sweep for `vm/machine`: retain
   only Core assembly, provider binding, execution/pacing, media bridging and
   copied Core-fact adaptation; move every remaining product parsing, text,
   Debug grammar, UI binding or composition decision to its named owner.
   Confirm one session only, one product control FIFO, one machine FIFO and
   one UI route.
8. **S8 - reusable-corpus and closure audit.** Build common independently
   using only `lib` public headers; verify its manifest and forbidden-vocabulary
   sweep.  Add neutral two-adapter conformance doubles (not a second product)
   for session/machine/UI contracts.  Run full repository unit and integration
   suites, static owner/route gates, documentation governance, diff review,
   and dual stripped x64/x86 artifact builds.  Record the exact common corpus
   revision eligible for a later SoftPC byte-identical adoption.

## Exit Criteria

- NXVM has exactly the required `core`, `common`, and `vm` component layout;
  every mutable state and queue has the owner stated above.
- `common/{ui,session}` depend solely on `lib`, have no product or machine
  vocabulary, platform API exposure or sibling dependency. `common/machine`
  owns the one shared paused-debug API and its opaque product-driver endpoint;
  it does not parse Debug grammar. `common/xasm32` depends only on `lib/types`;
  `common/debug` depends only on `lib/types`, `xasm32`, and `common/machine`'s
  paused-debug API.
- `vm/machine` is the only NXVM Core adapter; `vm/product` is the only NXVM
  policy owner; `vm/app` is the only composition root.
- `common/debug` has one complete lifecycle, synchronous bounded line/fact
  result path and parser continuation state, usable unchanged by NXVM and
  SoftPC. It is a session-registered CLI provider and calls only
  `common/machine`'s paused-debug API. Each product target owner owns lease
  validity and execution-plan enforcement. Debug has no native, session Core,
  VM or MVDM dependency beyond its public common interfaces.
- There is no retained `vm/events`, `vm/session`, `vm/presentation`, legacy
  executor bridge, direct native UI use, or compatibility wrapper that forms a
  second route.
- Full unit and external integration suites pass, all task evidence is
  recorded, and stripped x64/x86 task artifacts are deployed to both
  `build/output` and `assets/sessions`.
