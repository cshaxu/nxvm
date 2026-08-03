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

## Priority And Checklist

The implementation and closure order is fixed below. A task may pull a later
module forward only when that module is a direct, documented dependency of the
current higher-priority migration. That exception does not change the closure
order or authorize speculative VDM code.

| Priority | Module | Task ownership | Current judgement | Closure condition |
| --- | --- | --- | --- | --- |
| 1 | `core/machine` | T88, T95 | Pass | Guest state, providers, executor storage, and CPU trace workspace are per `core_machine` execution context. |
| 2 | `vm/machine` | T88, T95 | Pass | Full-PC objects are per session; control state is atomic and guest reset remains execution-boundary work. |
| 3 | `vdm/machine` | T89, T95 | Pass for current scope | DOS-minimal instances own their state. It has no concurrent host-entry contract yet, so no new code is required before a VDM run loop exists. |
| 4 | `core/platform` | T89, T95 | Pass | Shared host-surface contexts carry opaque handles; process-exclusive host resources use explicit atomic leases. |
| 5 | `vm/platform` | T90, T95 | Pass | Console/window rendering state is context-owned; Console and Linux terminal capabilities have explicit exclusive leases. |
| 6 | `vdm/platform` | T90, T95 | Absent | No production implementation; define against the core platform contracts when M8/M9 admits it. |
| 7 | `core/product` | T91, T95 | Pass | Debugger command workspace is caller-owned; assembler/disassembler workspaces are invocation-owned; target/wait remain thread-local scopes only. |
| 8 | `vm/product` | T92, T95 | Pass | Console parser, command buffer, target, and exit state are caller-owned Console-context fields. |
| 9 | `vdm/product` | T92, T95 | Absent | No production CLI/UI implementation; do not add speculative session code. |
| 10 | `vm/composition` | T93, T95 | Pass | Composition owns machine, platform, debugger, and Console context lifetimes without product-global selection. |
| 11 | `vdm/composition` | T94, T95 | Pass for current scope | Minimal composition is instance-owned; future product composition must bind the same context and lease contracts. |

`vm/profile` and `vdm/profile` are cross-cutting checks, not another runtime
priority tier: runtime profile context is session-owned and descriptors/tables
are immutable. T95 verifies them without inventing a dedicated migration.

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
| T88 | Close `core/machine`, then `vm/machine`: make the instruction trace workspace execution-context-owned and make VM control/debug instrumentation session-owned with a synchronized command boundary. | T87 | Two core-machine trace contexts and concurrent VM stop/pause/reset/step regression; retained Console and FDD/HDD gates. |
| T89 | Close current-scope `vdm/machine`, then `core/platform`: audit DOS-minimal ownership and define shared host-surface contexts and leases. | T87 | VDM-minimal remains instance-owned; no core-to-machine dependency; capability/lease contract tests. |
| T90 | Close `vm/platform`, then audit absent `vdm/platform`: contextualize VM renderers and model terminal ownership as an explicit lease. | T88, T89 | Two Win32 presentation contexts have independent resources; Console/terminal leases use the shared deterministic contract. |
| T91 | Close `core/product`: make debugger parser state caller-owned and assembler/disassembler workspaces invocation-owned. | T88 | Retained debugger and FDD gates pass. |
| T92 | Close `vm/product`, then audit absent `vdm/product`: make NXVM Console state caller-owned without changing its grammar or output. | T88, T91 | Retained `help`/`exit` grammar and output pass. |
| T93 | Close `vm/composition`: construct, bind, and tear down the machine, platform, debugger, and Console contexts. | T88--T92 | Two full VM sessions preserve independent construction state; GCC, FDD, and HDD gates pass. |
| T94 | Close current-scope `vdm/composition`: audit DOS-minimal composition against the same contracts; change code only for a demonstrated violation. | T89, T91, T93 | VDM-minimal remains instance-owned and introduces no VM dependency. |
| T95 | Perform the ordered checklist and profile closure audit; remove only proven-unused temporary compatibility state. | T88--T94 | Static inventory, default GCC preset, two-session VM, FDD/HDD, display, Console/debugger, VDM-minimal, and profile regressions pass. |

T87 is design and governance only. T88--T95 each change a runnable path only
when its scoped module has an implementation; such tasks produce the matching
task artifact. This plan does not begin M6 or implement a VDM CLI.

T96 is a post-closure retained-Console regression repair. It preserves the
session model by making live-machine initialization single-lifetime and adds a
real Console command lifecycle gate; it does not expand M6 or VDM scope.

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
