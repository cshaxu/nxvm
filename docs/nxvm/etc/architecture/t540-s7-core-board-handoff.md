# M5 T540 S7 Core/Board Handoff

This is the source-inspected cut required after S6. It supersedes S5's
prospective **file-move order**, not the accepted destination: one neutral
`x86/core` and proven `x86/ibmpc-*` board mechanisms. No source or ABI changes
occur in S7.

## Why the earlier physical move is unsafe

`core_machine` is currently one allocation containing both Core execution
state and PC/Compaq chip attachments. `timeline` and `transaction` are embedded
private state, not independently owned public objects. More importantly,
`port.h` exposes mutable `t_port` data and registration links to the board
adapters; `memory.h` exposes mutable `t_ram` storage, A20 and device-provider
arrays. Moving those files first would turn App-private layouts into a Shared
cross-component interface. A forwarding header or second table would conceal,
not remove, the dependency.

The search scope was all live `src/app-nxvm/devices` C and headers, plus the
corresponding Core/build/test receivers. `rg -l` for `t_port`, `executor_port`
and registration/execute names finds **23 files**; the analogous `t_ram`,
`executor_memory` and memory route search finds **19 files**. Seven production
files directly reach named board chip fields through `machine->...`; the
three mixed `machine.c`, `machine_scheduler.c` and `cpu_bus.c` files alone have
142 matching `machine->(shared_|xt_|fdc|hdc|kbc|vadp|d4_)` field references.
These counts locate the cut; they do not
pretend that each reference is a separate public operation.

## One owner on each side

| State/operation | Final owner | Cross-boundary value or operation |
| --- | --- | --- |
| CPU/FPU execution, checked RAM/port storage, transaction, timeline, elapsed ticks, stop/fault state and retirement observation | `x86/core`, one opaque Core instance | Board receives only bounded configuration and bus/signal operations; never a CPU, `t_ram`, `t_port`, transaction or timeline pointer. |
| PIC/PIT/DMA/RTC/KBC/FDC/HDC/video/XT PPI and D4 chip attachments, Port-B/refresh/speaker/parity latches, per-device clock phases | Board attachment, eventually `ibmpc-common`, `ibmpc-at`, `ibmpc-xt` or retained machine composition as S2 allocates | Core receives copied next-deadline, interrupt and completed-device-effect results; it does not include a board header. |
| One frozen construction plan and rollback sequence | Product/board composition; Core validates and retains only its neutral frozen inputs | One prepare/apply/rollback path, not a Core-plan copy and a parallel board plan. |
| Host worker, lifecycle FIFO, media leases, INI and firmware-byte selection | Existing Common/Lib/NXVM App owners | No new host loop, media backend or profile selector enters Shared x86. |

The mixed `machine_display.c` directly reads the VADP chip and belongs with
the board-facing copied display observation, not the neutral CPU executor.
The tiny `display.c` provider slot must follow its actual consumer rather than
being moved into Core because its filename looks generic. This corrects S5's
prospective display-file receiver. Conversely, the bounded firmware operation
mechanism and immutable ROM storage can follow Core; the selected F0000h ROM
role, reset-alias composition and provider implementation remain board/App
decisions. Neither change requires a second display or firmware path.

The eventual product machine is a composition of **one** opaque Core instance
and **one** board attachment with disjoint mutable fields. This is not two
guest clocks or two machine execution paths. Core alone commits guest time and
CPU retirement; board clock domains convert only the Core-provided elapsed
quanta. Construction and destruction order ensure the board's callbacks are
revoked before Core storage is destroyed.

## Required bounded exchanges

- **Ports:** one Core-owned atomic registration of a finite route batch,
  including read/write direction, width and the existing 3F7 wired-OR read
  rule; owner-scoped removal for teardown. Existing board callbacks become
  typed value/status providers. The Core port table and its linked entries
  remain private. An adapter cannot expose a registration checkpoint pointer.
- **Memory:** keep checked Core routing and side-effect-free inspection.
  Board attachment supplies copied range, provider functions and owner token;
  a failed candidate removes its own routes and observers before publication.
  KBC's execution-time A20 output uses a bounded Core signal, distinct from
  the paused-debug setter. DMA uses a bounded Core bus-cycle operation so its
  transaction/HOLD semantics are not reconstructed in the board layer.
- **CPU interrupt and time:** the Core CPU bus asks the board for pending IRQ
  and performs one acknowledged vector transfer through the Core transaction
  owner. Board reports its earliest actual device deadline and advances its
  chips for Core-supplied elapsed quanta; Core reduces this with its timeline
  and FPU deadline and remains the only guest-time publisher. No universal
  event queue or second scheduler is introduced.
- **Reset and failure:** preserve the current cold-reset sequence: CPU/FPU,
  Core port/RAM, board controllers with after-PIT action, then Core time,
  providers and firmware reset. Preserve the 8042 processor-only reset, which
  must not reset RAM or board devices. A single Core reset operation invokes
  one board reset stage; a partial construction rolls back through the same
  owner that published each route.

Existing `core_machine_install_port_provider()` and memory registration
operations establish part of this shape, but they are insufficient as-is:
the port operation accepts only a contiguous range and cannot express one
atomic noncontiguous board batch or wired-OR; the debug A20 setter forbids a
running machine; the memory debug read forbids running and is not observational.
The implementation S must derive the smallest missing typed operations from
these exact callers and delete the corresponding raw adapter paths. It must
not publish `t_port`, `t_ram`, chip state or an unbounded generic-device API.

## Finite source batches and receiving proof

The following are **prospective** linear S receivers; their exact packet is
fixed at intake. If a batch exceeds reviewable scope, split its unstarted
remainder into the next numeric S before implementation. No `S8a` suffixes.

| Batch | Complete change and proof boundary |
| --- | --- |
| S8 | Replace simple board port registration (92h, PIC, PIT and XT PPI) with one Core-owned value-provider/batch path. Delete each old raw callback/registration path; preserve primary-only XT and cascaded AT/Model40 routes and rollback tests. |
| S9 | Convert DMA port routes, including width/page-lane semantics, to the Core-owned batch; retain primary-only and paired routes and allocation-failure regressions. Source inspection found its width-dependent callback contract requires an owner-local Core change and separate review. |
| S10 | Convert KBC port routes and command/BAT/IRQ regressions to the same Core-owned batch. |
| S11 | Convert FDC port routes and status/data/DOR/DIR, DRQ/IRQ and registration-failure regressions to the same Core-owned batch. |
| S12 | Convert VADP staged CGA/EGA/VGA registration, HDC personality ports and 3F7 wired-OR, RTC/board ports; prove failed candidate leaves no partial route. After this batch no board adapter requires `t_port` or port-entry checkpoints. |
| S13 | Convert VADP/D4/ROM board memory attachments and their owner rollback to Core bounded routes; preserve side-effect-free video capture and reset-alias priority. |
| S14 | Convert KBC A20 and DMA/refresh memory cycles to bounded Core signals/transactions; no board adapter then requires `t_ram` or a Core transaction pointer. |
| S15 | Separate board deadline/advance and PIC acknowledge from Core's one CPU bus/timeline. Preserve PIT/RTC/DMA/FDC/HDC/KBC, XT keyboard, D4 and FPU ordering, HLT wake and L1/L2 disposition. |
| S16 | Split the one mixed plan/create/reset/destroy at the private state owner boundary, including processor-only reset and failure rollback. No copied mutable field or second reset/plan path. |
| S17 | Physically move the now-neutral Core source, private state and tests into `src/x86/core` / `test/x86/core`; reconnect NXVM through its public opaque contract and remove old App source definitions. |
| S18 onward | Move only S2-proven board mechanisms into `ibmpc-common`, `ibmpc-at` and `ibmpc-xt` in owner-sized batches; retain D4 and each genuine machine composition at its App owner. Final T gate checks every ledger row and all four external-machine scenarios. |

## S12 Source-Intake Refinement

S8-S11 completed the simple routes, DMA, KBC and FDC batches. S12 intake
confirmed the remaining prospective port row has four distinct publication
and rollback boundaries, so it is divided before more implementation:

| Linear receiver | Remaining port boundary |
| --- | --- |
| S12 | RTC/CMOS index and data routes, one chip lifetime and atomic batch. |
| S13 | Port-B planar parity and D4 board ports, including their parity/signal rollback. |
| S14 | HDC personalities, including XT routes and Compaq 3F7 wired-OR. |
| S15 | Staged VADP CGA/EGA/VGA route changes and failed-candidate rollback. |

The former prospective S13-S17 rows shift to S16-S20 respectively; proven
board extraction begins at S21. These remain prospective and are fixed only
by each later S intake. No suffix identifiers or temporary duplicate route
paths are introduced.

Every code S runs the complete repository-only x64/x86 unit suites, affected
four-profile Release pairs, applicable Shared receivers/manifests, and the
focused board/reset/interrupt/deadline regressions named by its packet. T540
still requires the full external integration gate. A successful structural
move never upgrades any chip's timing grade.
