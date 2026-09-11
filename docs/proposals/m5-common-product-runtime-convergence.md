# M5 Common Product-Runtime Convergence

## Purpose

Make NXVM's host-side single-machine runtime shareable with SoftPC without
moving NXVM Core, firmware, profile, debugger, CLI or native platform details
into a shared layer.  The resulting common corpus is a small runtime mechanism
layer below each product composition root and above the existing canonical
`src/lib` host services.

This is not an import of SoftPC's current `app/runtime` implementation.  That
code still calls SoftPC-machine APIs directly, just as NXVM's current
`vm/events` records still carry NXVM/Core-shaped values.  Both products are
evidence for the common contract, not inputs to a copied parallel runtime.

## Required NXVM End State

```text
src/lib/                         platform and generic host services only
src/common/                      neutral mechanism; depends only on lib
  contracts.h                    copied values and opaque adapter contracts
  ui/                            lib UI binding and copied host-event return
  session/                       lifecycle/control FIFO and reducer
  machine/                       safe-point executor bridge and FIFO

src/core/
  machine/                       Core execution and machine state
  product/                       Core product services
  debug/                         independent debugger lifecycle and command FIFO

src/vm/
  machine/                       NXVM adapter for core/{machine,product,debug}
  product/                       YAML/profile/CLI/debugger product policy
  app/                           NXVM-only composition root and routing
```

`common` has no `runtime/` component.  Creating and connecting `common/ui`,
`common/session`, and `common/machine` is composition, not a fourth reusable
state owner.  NXVM `vm/app` and SoftPC's corresponding product composition
each remain the one place allowed to know all three components.

`common/ui`, `common/session`, and `common/machine` may include `lib` and the
copied declarations in `common/contracts.h`; they do not include or call one
another.  `core`, `vm`, SoftPC, MVDM, CPU, BIOS, ROM, profile, controller,
guest, native handles, and platform headers are forbidden from the common
corpus.

`core/debug` is intentionally not a common component.  It is a complete Core
component with its own create/start/stop/destroy lifecycle, bounded debugger
command FIFO, copied request/result API, and explicit queue-full/fault
outcomes.  It consumes a declared opaque target contract only at a Core-safe
execution boundary.  It neither includes `core/machine` nor receives a raw
machine/CPU/RAM pointer; `vm/machine` is the sole NXVM adapter that binds the
validated target operations.  SoftPC does not bind or depend on `core/debug`
in this task, but can adopt the same public component later without changing
the component's ownership or inventing a SoftPC-specific debug route.

## Ownership And Data Flow

```text
monitor / native UI / machine completion
                  |
                  v
       common/session control FIFO       (sole ordered product ingress)
                  |
            copied effects
          /                 \
         v                   v
common/machine FIFO       common/ui API
safe-point executor       lib presenter binding
         |                   |
         v                   v
NXVM Core adapter       copied UI facts
         \                 /
          +-- vm/app re-publishes facts --+
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
4. `core/debug` owns a distinct bounded debugger-command FIFO.  This is not a
   duplicate machine transport: it belongs to the debugger component, receives
   only debugger commands, and is consumed by its bound target at a declared
   Core-safe boundary.  Debugger outcomes return as copied facts through
   `vm/machine` to the session ingress.

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
messages remain in `vm/product`.

The machine adapter is a small product-supplied callback table: create/start,
safe-point request delivery, stop/leave and copied fact publication.  Its
context is opaque to common.  NXVM `vm/machine` alone supplies that adapter and
owns Core assembly; neither `common` nor `lib` sees a Core pointer.

The `core/debug` public shape is likewise explicit and independent:

```c
core_debug_status core_debug_create(core_debug **);
core_debug_status core_debug_start(core_debug *,
    const core_debug_target_contract *);
core_debug_status core_debug_submit(core_debug *,
    const core_debug_command *);
core_debug_status core_debug_observe_safe_point(core_debug *,
    const core_debug_safe_point *);
core_debug_status core_debug_take_result(core_debug *, core_debug_result *);
void core_debug_stop(core_debug *);
void core_debug_destroy(core_debug *);
```

The target contract is opaque, bounded and copied.  It is implemented in
`vm/machine`; no function above exposes a mutable Core layout.  The component
does not own the machine lifecycle, so stopping a debugger clears only its
pending debugger work and target binding; `common/session` and `common/machine`
remain the sole owners of product and executor lifecycle respectively.

## Explicit Non-Goals

- No `common/runtime`, generic machine framework, profile parser, native API
  wrapper, second console broker, or compatibility shim.
- No Core instruction, device, timing, debug, firmware, media or storage
  semantics change.
- No move of product command text, YAML policy, profile selection, debugger
  meaning or SoftPC MVDM behavior into common.
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
2. **S2 - Core three-way independence and debug component.** Move and correct
   the current Core structure to independent `core/machine`, `core/product`
   and `core/debug` roots.  Make `core/debug` a complete lifecycle component
   with its own command FIFO, safe-point observation and copied result API;
   define the opaque target contract to be supplied later by `vm/machine`.
   Remove any former `core/product/debug` dependency rather than forwarding
   through a compatibility header.  Prove no root depends on a sibling root.
3. **S3 - common contracts and session mechanism.** Create the independently
   buildable common corpus and implement the neutral copied ABI plus the sole
   bounded session FIFO, run-generation validation and lifecycle reducer.
   It produces effects; it does not call UI, machine or product code.
4. **S4 - common machine bridge and NXVM adapter.** Implement the neutral
   safe-point machine request FIFO and opaque driver contract.  Move NXVM Core
   assembly plus the only `core/debug` target-contract binding into
   `vm/machine`; delete the old executor transport at the same time.  Verify
   ordered requests, debugger-command isolation, wakeups, pause/resume/reset/
   stop, stale-run rejection and fault propagation.
5. **S5 - common UI binding.** Move the generic lib-presenter binding to
   `common/ui`, with copied plans/facts only.  Retain one product-owned Console
   broker and move NXVM policy/console text out to `vm/product`; delete the
   obsolete `vm/presentation` path.  Verify Console/Window switching, title,
   target, focus, mouse and latest-frame semantics without native API leakage.
6. **S6 - NXVM product and app cutover.** Make `vm/product` own only NXVM
   command/YAML/profile/debugger policy, and `vm/app` the sole composition
   root that routes common effects and facts.  Delete the old `vm/session`,
   `vm/events` and product/host bridge paths rather than retaining adapters.
   Confirm one session only, one product control FIFO, one machine FIFO and
   one UI route.
7. **S7 - reusable-corpus and closure audit.** Build common independently
   using only `lib` public headers; verify its manifest and forbidden-vocabulary
   sweep.  Add neutral two-adapter conformance doubles (not a second product)
   for session/machine/UI contracts.  Run full repository unit and integration
   suites, static owner/route gates, documentation governance, diff review,
   and dual stripped x64/x86 artifact builds.  Record the exact common corpus
   revision eligible for a later SoftPC byte-identical adoption.

## Exit Criteria

- NXVM has exactly the required `core`, `common`, and `vm` component layout;
  every mutable state and queue has the owner stated above.
- `common` depends solely on `lib`, has no product or machine vocabulary, no
  platform API exposure and no sibling-component dependency.
- `vm/machine` is the only NXVM Core adapter; `vm/product` is the only NXVM
  policy owner; `vm/app` is the only composition root.
- `core/debug` has its own complete lifecycle, command FIFO and copied result
  path, while remaining independent of `core/machine`, `core/product`, common
  and SoftPC.  `vm/machine` is its sole NXVM target binder.
- There is no retained `vm/events`, `vm/session`, `vm/presentation`, legacy
  executor bridge, direct native UI use, or compatibility wrapper that forms a
  second route.
- Full unit and external integration suites pass, all task evidence is
  recorded, and stripped x64/x86 task artifacts are deployed to both
  `build/output` and `assets/sessions`.
