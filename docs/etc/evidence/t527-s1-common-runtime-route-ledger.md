# T527 S1: Common Product-Runtime Route And Portability Ledger

## Scope And Method

This is an S1 design ledger over NXVM `98ac51a4` and read-only SoftPC
`291afe4`.  It is a source-route inventory, not an imported-source record and
not a claim that SoftPC has adopted the result.  `src/lib` has the same
manifest SHA-256 in both trees:

`47E2DAA9A47F9FAD6DB680399DCE509E42C04A3EB4BCFEB61C4BFCBE875E1400`.

The ledger consumes the finite host-side execution/control/presentation
surface named below.  CPU, controller, firmware, profile and guest semantics
are deliberately excluded: they remain product-machine implementation.

## Approved Dependency Graph

```text
lib
 +- common/contracts.h
 +- common/ui
 +- common/session
 `- common/machine

core/machine     core/product     core/debug
       \              |              /
        \             |             /
                 vm/machine
                     |
                vm/product
                     |
                  vm/app
                     |
        common/{session,machine,ui}
```

The diagram is an ownership/composition map, not permission for peer
dependencies.  `common/ui`, `common/session`, and `common/machine` may include
only `lib` public headers and `common/contracts.h`; they do not include each
other.  `vm/app` is the sole NXVM composition root permitted to know all three.
It owns routing effects to the target component and facts back to session.
There is no `common/runtime` owner.

`core/machine`, `core/product`, and `core/debug` are sibling components.  They
may share only the project type foundation.  `vm/machine` is the only place
that knows all three and binds Core debug to a validated machine target.

## Current NXVM Route Inventory

| Current owner/path | Current fact or mutable state | Existing transport | T527 disposition and deletion receiver |
| --- | --- | --- | --- |
| `core/machine/*` | CPU, memory, device, timeline, execution and machine lifecycle | direct Core public interfaces | Retain. It never enters `common`; later `vm/machine` remains its sole host adapter. |
| `core/product/*` | product banner/config services | Core-local APIs | Retain as independent Core sibling. No common consumer. |
| `core/debug/debug.*`, `debug_target.h`, `debug_access.*` | debugger parsing/state and a synchronous `core_debugger_run(target)` call | target callback table with `void *context` | S2 replaces the synchronous call with a complete debug component lifecycle, bounded command FIFO, copied result API and opaque target binding. No raw Core/session layout becomes public. |
| `core/machine/debug_interface.*` | validated machine debug reads/writes/step/continue | Core-machine public debug interface | Retain as Core machine's only debug capability. S4's `vm/machine` binding adapts it; `core/debug` does not include it. |
| `vm/events/machine_event.h` | VM-specific input/request/result/frame records | copied records through session and executor queues | Delete in S6. Split its neutral input/lifecycle/fault/frame values into `common/contracts.h`; retain product configuration elsewhere. |
| `vm/events/presentation_plan.h` | target/title/mouse/frame/notice plan | `vm/session` reducer to `vm/presentation` | Delete in S6. Its neutral UI plan becomes `common_ui_plan`; NXVM notice text stays `vm/product`. |
| `vm/events/session_request.h` | YAML-resolved profile, CPU, assets and media paths | product Console to session construction | Remove from `vm/events` in S6, but retain as a `vm/product` immutable request/plan. It is product policy and cannot enter common. |
| `vm/session/control.*` | bounded 64-fact FIFO, ready event, run generation, latest display slot, source-specific pressed input | Console, presentation and machine result producers; Console consumes/reduces | S3 migrates its generic FIFO, latest-frame, run-id and stale-fact rules to `common/session`. Product text and NXVM input-to-machine meaning leave with `vm/product`/`vm/app`. Delete old path in S6. |
| `vm/session/session.*` | session owns `vm_machine *`, lifecycle requests and profile opening | direct product calls plus session control | Delete in S6. `vm/app` becomes composition root; `vm/product` resolves immutable start plans; `vm/machine` owns NXVM machine object. |
| `vm/machine/executor_queue.*`, `executor_fifo.*` | bounded request FIFO, lock, ready event and safe-boundary dequeue | control producer to executor consumer | S4 migrates generic bounded request/wake/safe-point mechanics to `common/machine`. Delete old path in S4. |
| `vm/machine/runtime/{runner,lifecycle,control,executor_state,fault,waiting}.*` | NXVM machine worker, actual lifecycle, Core run and timing/pause behavior | worker and Core callbacks | Retain in `vm/machine` as NXVM adapter implementation. S4 replaces only its generic ingress transport, not Core behavior. |
| `vm/machine/runtime/debug_target.*` and `vm/machine/debug.*` | Core-machine debugger target and NXVM recording/pause semantics | direct target callback binding | Retain and converge in S4 as the sole `core/debug` target binder. It adapts copied debugger commands/results at safe points; Core debug owns its own command FIFO. |
| `vm/machine/{fdd,hdd}.*`, runtime media/ROM/model composition and `vm/profile/*` | profiles, assets, media, board topology and Core plans | product configuration to NXVM machine construction | Remain NXVM-only. `vm/product` resolves an immutable start plan; `vm/app` passes it to `vm/machine`; common sees only a copied start token. |
| `vm/presentation/*` | shared-lib binding, Console broker, UI input and copied frame conversion | session plan to lib UI; lib input to session FIFO | S5 migrates generic binding to `common/ui`. The product Console's text/command policy remains `vm/product`; delete `vm/presentation` in S5. |
| `vm/product/console.*` | NXVM CLI grammar, messages, YAML catalog selection and direct cross-owner orchestration | direct session/presentation calls | S6 retains CLI/YAML/message policy in `vm/product`, but moves component construction and effect/fact routing to `vm/app`. Its direct session/presentation orchestration is deleted. |
| `vm/main.c` | creates session and product console | direct construction | S6 replaces with `vm/app` construction only. |

## Read-Only SoftPC Comparison

| SoftPC path | Useful mechanism | Product-specific content that blocks direct sharing | Common disposition |
| --- | --- | --- | --- |
| `app/control.*` | one ordered control queue for UI, monitor, runtime and presentation-completion facts; run generation | Win32 `CRITICAL_SECTION`/`HANDLE`, SoftPC event types and product hotkey handling | S3 uses the mechanism through `lib/host`, not this source. |
| `app/input_queue.*` | machine-executor input FIFO, cleared at a new run | SoftPC runtime/machine wake behavior | S4 uses the mechanism behind a neutral safe-point driver. |
| `app/runtime.*` | worker lifecycle, input delivery at executor boundary, latest copied-frame publication | direct `softpc_machine_*` calls and SoftPC frame state | It remains SoftPC product adapter evidence; no source is copied into common. |
| `app/reconciler.*` | separates desired UI policy from actual component completion | SoftPC display and Console policy | S5 may reuse only the neutral "desired versus actual" principle. Target policy stays product-owned. |
| `app/presentation.*` and `app/monitor.*` | lib UI components, Console broker and copied UI fact publication | SoftPC titles, status surface, monitor commands and direct app/control linkage | S5 creates neutral lib binding callbacks; product behavior stays outside common. |
| `host/*` outside `src/lib` | legacy SoftPC host/platform/DIB mechanics | native Windows/MVDM details | Excluded. Existing `src/lib` is the only common host/platform boundary. |

## Queue And Run-Generation Contract

| Owner | Queue/value | Producers | Sole consumer | Overflow and stale-run rule |
| --- | --- | --- | --- | --- |
| `common/session` | bounded control FIFO | `vm/app` monitor adapter, `common/ui` fact sink, `common/machine` fact sink | `vm/app`'s session-control loop invokes common reducer | Full returns explicit failure and produces a terminal delivery fact; never overwrite. Every non-monitor fact carries `run_id`; session rejects an older run. |
| `common/session` | latest copied frame slot | `common/machine` fact sink | session-control loop | Latest-wins only. `run_id` mismatch is discarded. Frames are not lifecycle/control records. |
| `common/machine` | bounded safe-point request FIFO | `vm/app`, after a session effect | product driver's safe execution boundary | Full is explicit failure. Requests carry `run_id`; driver refuses stale work before Core mutation. |
| `core/debug` | bounded debugger command FIFO | `vm/app` via `vm/machine` debugger binding | `core/debug` at its declared target safe point | Full is explicit debugger result/failure. It accepts no ordinary lifecycle/input command and does not own product run generation. |
| `lib` presenter | private frame/control mailboxes | `common/ui` only | native presenter worker | Existing lib contract: frame latest-wins; control FIFO reports overflow. These are not exposed to a product. |

The two FIFO boundaries are intentional and non-duplicative: session serializes
product policy; machine serializes executor-thread mutation.  Debug serializes
only debugger commands.  All three differ in owner, producer class, consumer
and payload.  No component polls or directly shares another component's queue.

## Frozen Common API Vocabulary

`common/contracts.h` may define only copied values built from `lib` scalar and
UI types:

```text
common_run_id                 non-zero monotonically increasing session run
common_session_command        start(token), pause, resume, reset, stop
common_session_fact           UI input, monitor line, machine result, UI completion/failure
common_session_effect         submit-machine, apply-UI, product-notice
common_machine_request        lifecycle or UI input at a safe point
common_machine_fact           lifecycle/fault or copied ui_frame
common_ui_plan                target, latest title, capture policy, copied ui_frame
```

`START` contains a copied opaque product start token, not a pointer or an
asset/profile structure.  `vm/product` maps a token to an immutable NXVM
machine plan; `vm/app` delivers that plan to `vm/machine`.  `common` neither
resolves nor retains that plan.

`common/machine` receives a product-supplied opaque driver binding with
create/start, request-at-safe-point, stop/leave and copied-fact publication.
The binding context is an opaque handle that common never dereferences.  It
does not expose CPU, RAM, port, controller, Core-machine, executor or session
layout.

`core/debug` uses an equivalent opaque target binding, but its own contract is
Core-local rather than common.  S2 must replace the current public raw callback
context with an opaque binding lifetime owned by `vm/machine`, and must publish
only copied debugger results.  SoftPC has no binding in this task.

## Required Legacy-Path Deletion Batches

| Later S | Deletes/converges | Retained single path |
| --- | --- | --- |
| S2 | synchronous `core_debugger_run(target)` surface and any `core/product/debug` relation | `core/debug` lifecycle + command FIFO + opaque target binding |
| S3 | generic state from `vm/session/control.*` | `common/session` control FIFO/frame slot/reducer |
| S4 | `vm/machine/executor_{queue,fifo}.*` generic transport | `common/machine` safe-point FIFO; NXVM runtime remains driver only |
| S5 | `vm/presentation/*` lib binding | `common/ui` -> canonical `src/lib` |
| S6 | `vm/events/*`, `vm/session/*`, direct Console orchestration in `vm/product/console.*`, `vm/main.c` composition | `vm/app` composition; `vm/product` policy; `vm/machine` adapter |

## Forbidden Vocabulary Sweep

The S7 corpus sweep must reject these tokens under `src/common/` except in
negative-test fixture text: `nxvm`, `softpc`, `mvdm`, `core`, `cpu`, `bios`,
`rom`, `profile`, `controller`, `guest`, `HWND`, `HANDLE`, `windows.h`,
`pthread`, `X11`, `Wayland`, `SDL`, `vm_` and `app_`.

The names `machine`, `session`, `ui`, `run_id`, `frame`, `input`, `target`,
`fault`, `start`, `pause`, `resume`, `reset` and `stop` remain permitted only
in the finite common copied contracts above.

## S1 Conclusion

The approved component graph is implementable without `common/runtime` and
without a second production route.  The present T526 architecture supplies
clear migration sources, while current Core debug is the one material missing
component boundary: it is directory-separated but not independently
lifecycle-complete.  S2 must repair that first.  No S1 source, ABI, asset,
build or behavior change is justified.

## Verification

- Complete repository unit suite: `ctest --test-dir build/mingw-gcc-x64 -L
  unit --output-on-failure -j 8` passed 299/299 tests in 201.02 seconds.
- Documentation governance passed for the `vm-0-5-0526` baseline.
- `git diff --check` passed; the S1 change surface is documentation and
  evidence only.
