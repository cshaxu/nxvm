# M5 Legacy Convergence

## Authority And Status

This feedback reopens M5. The prior M5 closure proved source-root and CMake
owner separation, but not the implementation convergence now required for the
shared core. It supersedes only the prior M5 closure claim; its recorded build
and regression evidence remains valid as a baseline.

M5 remains subject to its existing compatibility gate: no change to retained
NXVM Console text or grammar, debugger behavior, boot/reset ordering, media
behavior, or executable identity without explicit owner approval and a
recorded before/after acceptance plan.

## Required End State

1. `vm/machine/device.h` is deleted. It is a temporary legacy aggregate, not
   a public VM or core API. Callers use narrow owner-local headers instead.
2. The retained NXVM executor has one authoritative core-machine state and
   API path. `vcpu`, `vram`, `vport`, and related `v*` implementation names
   are progressively absorbed into `cpu`, `memory`, `port`, and discrete core
   device modules. No copied execution implementation or duplicated CPU/RAM/
   port state is permitted.
3. Shared concrete host facilities live in `core/platform/win32` and
   `core/platform/linux`. They expose host primitives only and do not include
   or mutate machine/product state. VM- and VDM-specific display, Console,
   input, cancellation, boot, and execution policies remain in their product
   form and are bound by root composition.
4. Legacy component names converge by ownership, not by a parallel rewrite:
   `vcpu` becomes the implementation behind `cpu`, `vram` behind `memory`,
   and `vport` behind `port`, with one state instance throughout. Public
   contracts use the `core_machine_*` prefix; historical names disappear only
   after all direct callers are moved and focused regressions pass.

## Work Breakdown

| Task | Result | Completion gate |
| --- | --- | --- |
| M5 T15 | Record the exact `device.h` consumer/ownership map and replace one narrow, behavior-preserving surface at a time. | No source includes `device.h`; full retained VM gates pass. |
| M5 T16 | Converge the legacy CPU/RAM/port executor behind `core_machine_*` contracts, then migrate the remaining shared device implementations by the same rule. | One state authority per subsystem; no `deviceConnect*` entry points; CPU probe and fixture/UX gates pass. |
| M5 T17 | Extract reusable Win32/Linux host primitives into `core/platform`, leaving VM-only policy in `vm/*`. | Core provider targets have no machine/product includes; Windows build and Linux static gate pass. |
| M5 T18 | Re-audit source/target DAG, remove transitional compatibility surfaces, publish the final M5 artifact and closure evidence. | Required end state and all existing M5 gates pass. |

## T15 Consumer Map

`device.h` originally had eight direct consumers. It is deleted only after each
consumer changes to the narrow owner listed below.

| Consumer | Current aggregate use | Replacement owner |
| --- | --- | --- |
| `vm/composition_loop.c` | global run/reset/flip state, lifecycle, command boundary, keyboard-host-state bridge | `vm/composition` session-control API plus `core/machine` keyboard API |
| `vm/composition_machine.c` | lifecycle, run state, keyboard, stop | `vm/composition` session-control API and `core/machine` keyboard API |
| `vm/composition_console.c` | media, boot, memory sizing, diagnostics, record control, run state | narrow `vm/machine`, `vm/profile`, `core/machine`, and composition control APIs |
| `vm/composition_debug.c` | register/memory/port/debug-state access | narrow `core/machine` and `vm/machine/vdebug` APIs |
| `vm/composition_cpu_probe.c` | test lifecycle, segment/eip setup, RAM write | narrow core-machine execution/probe API |
| `vm/composition_display.c` | text snapshot reads | `core/machine/display.h` snapshot API |
| `vm/composition_full_pc.c` | command boundary, media, boot, RAM, reset, reset vector, record control | narrow composition, VM-machine, profile, and core-machine APIs |
| `vm/machine/vdebug.c` | stop request | composition-owned stop callback or core-machine pending-stop contract |

The aggregate also declares implementations consumed internally by default
profile firmware: BIOS boot, keyboard state, and text display access. Those
implementation symbols must be renamed or relocated only when their provider
contract is ready; they may not be exposed through a replacement aggregate.

## T15 Progress

The first replacement surface is complete: composition-owned execution control
now has `vm_composition_control_*` operations for initialize/finalize,
start/reset/stop, running/flip observation, status printing, and command-boundary
binding. Its state is private to `composition_loop.c`; root composition and
its focused smokes no longer access `device.flag*` or the old lifecycle
functions. `vm/machine/vdebug` now requests its immediate stop through a
composition-bound callback, preserving breakpoint/trace timing without a
VM-machine-to-composition include.

The obsolete `t_device` global declaration and all lifecycle, run-state,
status, and command-boundary declarations have been removed from `device.h`.
The affected artifact and focused smokes still build; no source references an
old control symbol.

This is not T15 completion. `composition_loop.c` still includes the aggregate
only for the unconverted keyboard bridge, and the remaining consumers still
use its CPU, RAM, port, media, BIOS, debug-recording, and display surfaces.
The slice passed GCC build, FDD execution-context lifecycle, expected-`#UD`
CPU-stop, source-DAG, and retained Console `HELP`/`EXIT` gates.

The next slice adds direct `core_machine_cpu_*`, `core_machine_memory_*`, and
`core_machine_port_*` entry points over the retained single executor state.
Debugger composition, CPU probing, reset-vector inspection, and memory sizing
now use those core paths (or the same explicit `vcpu` state where inspection is
required). The old `deviceConnectCpu*`, `deviceConnectRam*`, and
`deviceConnectPort*` definitions remain only as uncalled legacy exports until
their final deletion; no duplicate machine state was introduced. GCC,
debug-target, expected-`#UD` probe, and FDD/HDD full-PC profile gates passed.

VM hardware debugger controls now live in `vm/machine/vdebug.h` under
`vm_machine_debug_*`. The core debugger adapter, retained Console recorder,
and full-PC session use that narrow interface; the debugger declarations are
gone from `device.h`. GCC plus debugger-target and Console gates passed.

The BIOS/media, display, and keyboard surfaces have since been narrowed. VM
media and built-in BIOS controls use their machine/profile headers; display
uses a copied `core_machine_display_snapshot` provider; and keyboard input
uses a `core_machine_keyboard_provider` bound by VM composition. The old
composition keyboard bridge is deleted rather than retained as a wrapper.
Only its implementation-local default-profile helpers retain historical names
until T16 performs their one-at-a-time naming convergence.

The remaining direct `device.h` includes had no active declaration use, so the
aggregate is deleted. T15 is complete: no source or CMake reference remains.
The deletion gate passed the Windows GCC build, keyboard-input and expected
`#UD` stop smokes, FDD/HDD reset-vector fixture smoke, retained Console
`HELP`/`EXIT` interaction, and the zero-edge dependency-DAG verifier. This
preserves the existing device state, Console grammar, execution loop, and
profile behavior.

## Migration Discipline

Each slice first maps all direct callers and an observable behavior baseline.
Then it changes one narrow surface, repairs direct includes and CMake ownership,
and runs the smallest focused smoke followed by the retained Console,
debugger, CPU, FDD/HDD fixture, and dependency-DAG gates when affected. Source
moves use `git mv`; implementation rewrites are allowed only where needed to
remove a proven duplicate state or forbidden dependency. Stop for approval if
the required change alters any compatibility-gate behavior.

## T16 Convergence Map

T16 removes historical `deviceConnect*` names in four independent groups.
RAM, port, and CPU compatibility exports have no callers and can be deleted
directly because their existing `core_machine_*` operations already cover the
active paths. Default-profile display and keyboard helpers are renamed inside
their provider implementation without changing provider behavior. Each group
retains the same underlying state object and has its own build and focused
smoke gate.

The RAM/port group is complete. The uncalled `deviceConnectRam*` and
`deviceConnectPort*` exports are deleted; their existing `core_machine_memory_*`
and `core_machine_port_*` operations remain the sole public path over the
retained `vram` and `vport` instances. Windows GCC, expected-`#UD` CPU probe,
FDD/HDD reset-vector fixture smoke, and the zero-edge DAG verifier passed.

The CPU group is complete. All uncalled `deviceConnectCpu*` exports are
deleted; linear memory access, segment loading, code metadata, watchpoints,
and debugger display continue through `core_machine_cpu_*` over the existing
`vcpu` and `vcpuins` state. Windows GCC, CPU probe, CPU-stop, debug-target,
FDD/HDD reset-vector fixture, and zero-edge DAG gates passed.

The default-profile display group is complete. Its capture helpers are now
private `vm_profile_default_display_*` functions behind the existing core
snapshot provider; text RAM reads, change detection, and copied-frame layout
are unchanged. Windows GCC, core presentation, default-profile FDD fixture,
and zero-edge DAG gates passed.

The default-profile keyboard group is complete. Its helpers are now local
`qdkeyb*` functions behind the existing core keyboard provider; BIOS data-area
flags, queued scancodes, and IRQ1 delivery are unchanged. Windows GCC,
platform-input, default-profile FDD, full-PC FDD/HDD, and zero-edge DAG gates
passed.

## Ownership Rules

`core/platform` owns host capability, not policy: Console attachment/mode,
window lifecycle and event pumping, host input events, frame submission,
waiting, timers, and process-control primitives can be common. `auto` display
selection, guest graphics transition, NXVM boot display behavior, VDM parent
Console protection, cancellation, and exit semantics are product policy.

`core/machine` owns guest state and never calls platform code. Root composition
adapts a core machine snapshot to a core-platform frame and binds any host
provider. `core/product` debugger UI reaches a concrete machine only through
its debug-target callback contract.
