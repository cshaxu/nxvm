# M5 Session Readiness

## Goal

Make every implemented module safe to instantiate for more than one session in
one process, or explicitly model a host resource as a process-exclusive lease.
This is an ownership and synchronization closure, not a multi-session product
or CLI feature.

Every mutable datum must be one of: session-owned, execution-thread-owned,
thread-local scope, or an explicit process-exclusive host lease. Immutable
tables and descriptors are shared. An unclassified mutable global fails this
plan.

## Module Checklist

| Priority | Module | Current judgement | Closure condition |
| --- | --- | --- | --- |
| 1 | `core/machine` | Pass | Guest state, providers, and executor storage are per `core_machine`; retain two-instance regression. |
| 2 | `vm/machine` | Partial | Full-PC objects are per session, but control flags cross threads without synchronization. |
| 3 | `vdm/machine` | Pass for current scope | DOS-minimal instances own their state. It has no concurrent host-entry contract yet, so no new code is required before a VDM run loop exists. |
| 4 | `core/platform` | Gap | Define shared host-capability contexts and host-surface lease contract; it must not hold guest state. |
| 5 | `vm/platform` | Fail | Console/window rendering state becomes context-owned; Linux terminal capability has an explicit exclusive lease. |
| 6 | `vdm/platform` | Absent | No production implementation; define against the core platform contracts when M8/M9 admits it. |
| 7 | `core/product` | Fail | Debugger parser, assembler, and disassembler workspaces become session-owned contexts; thread-local target/wait scopes remain scopes only. |
| 8 | `vm/product` | Fail | Console parser, command buffer, target, and exit state become one Console-session object. |
| 9 | `vdm/product` | Absent | No production CLI/UI implementation; do not add speculative session code. |
| 10 | `vm/composition` | Partial | It owns construction and threads, but must adopt the new machine/platform/product contexts without reintroducing hidden state. |
| 11 | `vdm/composition` | Pass for current scope | Minimal composition is instance-owned; future product composition must bind the same context and lease contracts. |

`vm/profile` and `vdm/profile` currently pass: runtime profile context is
session-owned and descriptors/tables are immutable. They remain checked in
T95 but do not require a dedicated migration. The default priority is the
table order. A task may move earlier only when it is an explicit dependency of
a higher-priority migration.

## Confirmed Inventory

| Owner | Current mutable state | Required disposition |
| --- | --- | --- |
| `core/machine` | `core_machine` and installed provider state | Session-owned; current model passes. |
| `core/platform` | wait/debug scopes | Thread-local scopes are allowed; add host-surface context/lease contract before host code moves here. |
| `core/product` | `debug.c`, `aasm32.c`, and `dasm32.c` parser workspaces | Caller-owned debugger context. |
| `vm/machine` | `vm_composition_control_state` run/reset/pause/step fields | Synchronized command/state boundary. |
| `vm/platform/win32` | Console buffer and GDI renderer state | Per-surface context owned by the VM platform session. |
| `vm/platform/linux` | curses `stdscr` and display generation | Explicit process-exclusive terminal lease plus per-lease state. |
| `vm/product` | `console.c` target, parser buffer, arguments, and exit flag | Console-session object. |
| `vm/profile` | default-profile runtime context | Session-owned; immutable firmware tables remain shared. |
| `vdm/*` | DOS-minimal session and immutable descriptors | Current state is per session or immutable; future host/run loop must declare its synchronization contract. |

## Tasks

| Task | Scope | Depends on | Completion gate |
| --- | --- | --- | --- |
| T87 | Establish the authoritative inventory, repair current GCC presets, and add a no-unclassified-mutable-global scan. It confirms the already-passing `core/machine`, `vdm/machine`, and profile rows. | None | Current preset builds the current task artifact and gates; inventory is approved. |
| T88 | Replace VM control flags with a synchronized command/state boundary. | T87 | Concurrent stop/pause/reset/step regression; retained Console and FDD/HDD gates. |
| T89 | Define `core/platform` host-surface context and lease contracts; move only mechanism-only shared host facilities there. | T87 | No core-to-machine dependency; capability/lease contract tests. |
| T90 | Contextualize VM Win32 Console/window renderers and define Linux curses as an explicit exclusive surface lease. VDM platform remains absent, with no speculative code. | T88, T89 | Two Win32 presentation contexts have independent generations/resources; lease conflict is deterministic. |
| T91 | Make shared debugger parser/assembler/disassembler state session-owned. | T87, T88 for command-boundary tests | Two debug contexts do not cross-target; retained debugger grammar/output gate. |
| T92 | Make NXVM Console parser, target, and exit state session-owned. VDM product remains absent, with no speculative code. | T88, T91 | Two Console contexts do not cross-target; retained Console grammar/output gate. |
| T93 | Update VM root composition to construct, bind, and tear down the new machine/platform/product contexts. | T88--T92 | Two full VM sessions preserve independent control, Console, debugger, and presentation behavior. |
| T94 | Audit VDM root composition against the same contracts; change code only if current minimal composition violates them. | T89, T91, T93 | VDM-minimal remains instance-owned and no VM dependency is introduced. |
| T95 | Run the module checklist closure audit and remove temporary compatibility state. | T88--T94 | Static inventory, default GCC preset, two-session VM, FDD/HDD, display, Console/debugger, and VDM-minimal regressions pass. |

T87 is design and governance only. T88--T95 each change a runnable path only
when its scoped module has an implementation; such tasks produce the matching
task artifact. This plan does not begin M6 or implement a VDM CLI.

## Guardrails

- VM/VDM root composition owns session construction, host threads, and
  teardown; peer modules never select another module's session.
- A platform surface may be exclusive, but rejection/lease ownership is
  explicit and testable. A `static` backend state is not a lease.
- Core product tools receive a caller-owned context or an explicit scope; they
  never retain a process-wide mutable current session.
- Cross-thread host requests are commands; only the execution thread mutates
  guest state at its command boundary.
- T95 may close this plan only after all implemented checklist rows pass or
  carry a documented, tested process-exclusive lease.
