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
| 1 | `core/machine` | Pass | Guest state, providers, executor storage, and CPU trace workspace are per `core_machine` execution context. |
| 2 | `vm/machine` | Pass | Full-PC objects are per session; control state is atomic and guest reset remains execution-boundary work. |
| 3 | `vdm/machine` | Pass for current scope | DOS-minimal instances own their state. It has no concurrent host-entry contract yet, so no new code is required before a VDM run loop exists. |
| 4 | `core/platform` | Pass | Shared host-surface contexts carry opaque handles; process-exclusive host resources use explicit atomic leases. |
| 5 | `vm/platform` | Pass | Console/window rendering state is context-owned; Console and Linux terminal capabilities have explicit exclusive leases. |
| 6 | `vdm/platform` | Absent | No production implementation; define against the core platform contracts when M8/M9 admits it. |
| 7 | `core/product` | Pass | Debugger command workspace is caller-owned; assembler/disassembler workspaces are invocation-owned; target/wait remain thread-local scopes only. |
| 8 | `vm/product` | Pass | Console parser, command buffer, target, and exit state are caller-owned Console-context fields. |
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
| `core/machine` | `core_machine`, installed provider state, and CPU instruction trace workspace | Session-owned. |
| `core/platform` | wait/debug scopes; host-surface contexts and leases | Thread-local scopes and caller-owned opaque contexts are allowed; leases name one composition owner. |
| `core/product` | debugger command state, assembler, and disassembler workspaces | Caller-owned debugger context and invocation-owned assembler/disassembler contexts. |
| `vm/machine` | `vm_composition_control_state` run/reset/pause/step fields and debug instrumentation | Atomic command/state boundary; instrumentation is session-owned or disabled. |
| `vm/platform/win32` | Console buffer and GDI renderer state | Per-surface context owned by the VM platform session; the shared Console is explicitly leased. |
| `vm/platform/linux` | curses `stdscr` and display generation | Explicit process-exclusive terminal lease plus session-owned generation. |
| `vm/product` | Console target, parser buffer, arguments, and exit flag | Caller-owned Console context. |
| `vm/profile` | default-profile runtime context | Session-owned; immutable firmware tables remain shared. |
| `vdm/*` | DOS-minimal session and immutable descriptors | Current state is per session or immutable; future host/run loop must declare its synchronization contract. |

## Tasks

| Task | Scope | Depends on | Completion gate |
| --- | --- | --- | --- |
| T87 | Freeze the inventory, repair current GCC presets, and add a no-unclassified-mutable-global gate. | None | The preset builds the latest verified runnable artifact and all current structural gates; inventory is recorded. |
| T88 | Close machine state in priority order: make the core instruction trace workspace instance/call-owned, then replace VM control flags and debug instrumentation with a synchronized session command/state boundary. | T87 | Two core-machine trace contexts and concurrent VM stop/pause/reset/step regression; retained Console and FDD/HDD gates. |
| T89 | Audit current `vdm/machine`, then define `core/platform` host-surface context and lease contracts; move only mechanism-only shared host facilities there. | T87 | VDM-minimal remains instance-owned; no core-to-machine dependency; capability/lease contract tests. |
| T90 | Contextualize VM Win32 Console/window renderers and define Linux curses as an explicit exclusive surface lease; audit absent VDM platform against that contract without speculative code. | T88, T89 | Complete: two Win32 presentation contexts have independent resources; Console/terminal leases use the shared deterministic contract. |
| T91 | Make shared debugger parser/assembler/disassembler state session-owned. | T88 | Complete: caller-owned command workspace and invocation-owned assembler/disassembler workspaces; retained debugger/FDD gates pass. |
| T92 | Make NXVM Console parser, target, and exit state session-owned; audit absent VDM product against the same contract without speculative code. | T88, T91 | Complete: caller-owned Console context; retained `help`/`exit` grammar and output pass. |
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
