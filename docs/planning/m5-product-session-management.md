# M5 Shared Product Session Management Design

## Decision

This owner-approved M5 extension defines the shared product-session mechanism
used first by the retained NXVM Console and later by the VDM debug Console.
It is built on completed session readiness work; it does not start M6,
implement VDM behavior, or alter `core_machine` execution semantics.

`core/product/session` owns the generic opaque registry, selection, numeric
ID allocation, copied snapshots, and shared `SESSION` command grammar. It
knows no concrete VM/VDM session, guest state, profile, platform handle, or
execution loop. Product-root composition supplies all concrete lifecycle work
through an explicit provider.

NXVM uses this mechanism in M5. It creates session `0` at process startup and
selects it. Each registered item is a complete `vm_session`, which owns one
`core_machine`, VM-only devices, default-profile context, debugger state,
presentation mailbox, and platform run handle. VDM adopts the same mechanism
only when M6/M8 define its DOS session and debug Console contracts.

## Shared Command Contract

The shared debug/management Console grammar is:

```text
SESSION LIST
SESSION OPEN
SESSION SELECT <id>
SESSION CLOSE [id]
```

- `LIST` prints every live session in ascending numerical ID order. The
  selected item begins with `*`, like `git branch`, and reports its execution
  state and configured display mode.
- `OPEN` creates an initialized, stopped session. Its ID is one greater than
  the largest currently live ID; when none exist, it is `0`. It does not change
  selection.
- `SELECT <id>` changes the manager-owned selection. Product machine commands
  apply only to that selected session.
- `CLOSE [id]` defaults to the selected session. It requests stop, waits for
  the session-owned run handle to join, finalizes the session, and destroys it.
  The last live session cannot close and reports a defined error. If it closed
  the selected item, the manager selects the lowest remaining ID.
- The manager always has at least one live session and exactly one selected
  session. A no-selection state is not a valid product state.
- Product exit refuses while any session is running and names those IDs. It
  does not silently terminate a guest.

For NXVM, `Console>` remains unchanged and `*` appears only in `SESSION LIST`.
Session `0` preserves the retained initial-session initialization path. The
historical machine-local media attachment in the old single-session Console
target must not be copied into `OPEN`; new VM sessions have no attached
FDD/HDD. A later bounded cleanup may remove or replace that legacy path only
with explicit compatibility approval.

For VDM, non-debug `ntvdm64 run` creates manager session `0`, configures that
session from the CLI workload, runs it, and tears it down without allocating a
management Console. `ntvdm64 run --debug` exposes the same shared commands.
Its session `0` is configured from the CLI and pauses at the defined debug
boundary. A later VDM debug-contract task defines how an `OPEN` session
receives an additional program; the shared manager must not infer program
paths, DOS loader policy, or a shell.

## NXVM Display, Input, And Debug

Each NXVM session has its own configured display policy.

- A Window session owns its window, renderer, platform run handle, and guest
  input route. Starting it returns control to the management Console, allowing
  several Window sessions to run simultaneously. Selecting a different
  session never redirects input from an already running window.
- A Console session acquires the existing explicit process-exclusive host
  Console/terminal lease and preserves its synchronous behavior. While it is
  running, the management Console is occupied by guest I/O, so users cannot
  practically issue concurrent session commands. This is intentional, not a
  selected-session limitation.
- A second direct attempt to start Console-mode execution while its host
  surface is leased receives the existing defined acquisition failure. It may
  not overwrite or steal the lease.
- `DEBUG` resolves only the selected session's debugger target. The retained
  debugger prompt remains serial with the management Console and never holds a
  cross-session current-object pointer.

Session media/device state is in-memory and belongs to its session. An input
image may be loaded into more than one session as independent state; explicit
save/remove operations remain Console commands and are serialized by the one
management Console.

## Ownership And Interfaces

```text
vm/main.c
  -> core_product_session_manager (opaque table, IDs, selection)
  -> VM composition session provider
       -> vm_session #0
       -> vm_session #N
  -> vm_product_console_main(machine provider, session manager)

Console SESSION command
  -> core/product/session command
  -> core_product_session_manager
  -> VM/VDM composition provider
  -> copied session snapshots

Console machine command
  -> explicit product machine provider
  -> product composition resolves selected opaque item
  -> concrete session lifecycle/debug/media operation
```

`core_product_session_manager` is the sole owner of the session table,
selected ID, and ID allocation. It owns opaque entry registration only; the
provider creates, describes, stops, joins, finalizes, and destroys the
concrete product session. The manager enforces a nonempty table and exactly one
selected entry. It is called only by the management Console thread. Platform
workers and guest execution threads receive only their own concrete session;
they never access the manager or its table. A close provider first quiesces the
target run handle, so no callback can retain a destroyed session.

The generic manager exposes these core-product contracts:

- `core_product_session_provider`: composition supplies `open`, `describe`,
  and synchronous `close` for opaque concrete sessions.
- `core_product_session_interface`: manages opaque entries, selection, and
  copied snapshots.
- `core_product_session_command_interface`: parses the shared grammar and
  emits lines through an explicit caller-owned output provider.

`vm/product` owns its retained Console loop and routes the `SESSION` verb to
the shared command facility. It owns no selected ID, session pointer,
machine/device storage, or platform handle. VM composition provides a
`vm_product_console_machine_provider` for retained machine commands; it
resolves the generic manager's selected opaque entry as a `vm_session`.

VDM product code routes the same command facility only in debug mode. VDM
composition supplies its own concrete session provider and selected-machine
adapter. No `core/composition/` directory is permitted: any common code that
can remain opaque and policy-free belongs in `core/product/session`; concrete
assembly remains in the product-root composition.

## Target Source Shape

```text
src/core/product/session/
  manager.h/.c              opaque table, selection, IDs
  session_interface.h       generic public manager/snapshot contract
  session_provider.h        injected composition lifecycle contract
  command_interface.h/.c    shared SESSION grammar and output contract

src/vm/
  main.c
  composition/session/
    session.h/.c            one complete VM session
    lifecycle.h/.c          initialize/reset/start/stop/resume/finalize
    control.h/.c            run/pause/reset command state
    runner.h/.c             one outer loop using core_machine_run()
    execution.h/.c          execution-boundary helpers
    providers.h/.c          profile and provider binding
    display.h/.c
    debug_target.h/.c
    block.h/.c
    provider.h/.c           VM concrete-session and selected-machine adapters
  product/
    console.h/.c            Console loop and general command dispatch
    console_machine_provider.h

src/vdm/
  composition/session/      future VDM concrete-session provider and lifecycle
  product/                  future debug-mode Console routing only
```

The directories remove redundant `session_` file prefixes where the path
already establishes ownership. Public C symbols remain owner-qualified:
`core_product_session_*`, `vm_session_*`, and `vdm_session_*`.

The existing `vm/product/console_target.h` and
`vm/composition/console_target.*` are single-session forwarding vocabulary.
NXVM implementation must migrate them with `git mv`/focused edits into the
shared session contract plus the VM selected-machine provider, then delete
them. No compatibility wrapper remains.

## Immediate Implementation Breakdown

| Task | Scope | Behavior protection and exit gate |
| --- | --- | --- |
| T144 | Freeze this contract and migration map. | Design review only; no source/runtime change or artifact. |
| T145 | Move single-session VM composition files into `vm/composition/session/`; define generic core-product session/provider/command contracts without changing the one-session path. | GCC, retained Console/debugger, FDD DOS prompt, source dependency gate. |
| T146 | Implement the generic `core_product_session_manager`; bind the VM composition provider, create/select session `0`, and replace the old target while retaining all old commands for session `0`. | Complete: manager/session smoke, retained Console lifecycle, and FDD DOS-prompt gates passed; no global/TLS/current selector. |
| T147 | Route NXVM Console `SESSION` grammar through the shared command facility; add `OPEN`, `SELECT`, and copied list snapshots for stopped sessions. | Two configured sessions have independent RAM/media/display configuration; selection never changes guest state. |
| T148 | Add deterministic `CLOSE`, last-session rejection, ID allocation/reuse, and final product shutdown behavior. | Stop/join/finalize/destroy ordering; last session cannot close; repeated open/close; no worker callback after close. |
| T149 | Enable and verify concurrent Window sessions; preserve Console lease semantics and selected-session debugger routing. | Two independent windows, stop one while the other runs, Console lease rejection, retained FDD boot/debugger gates. |
| T150 | Perform NXVM feature closure audit and rebuild the current artifact; record the VDM adoption boundary for M6/M8/M9. | Full GCC gates, session-manager structural scan, Console/debugger/FDD regression, artifact and build hygiene. |

Each implementation task records its applicable `RULES.md` provisions,
provider lifetime/threading contract, a source migration map, and exact
expected markers before changing code. Every behavior-changing task produces
its task-numbered local NXVM artifact. Stop immediately on a duplicate session
owner, global/TLS selection, second guest executor, Console grammar regression,
or FDD DOS-prompt failure.
