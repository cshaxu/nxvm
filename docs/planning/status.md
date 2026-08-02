# Project Status

## M0 Closure

M0 is complete. Its governance, source and redistribution boundaries, roadmap,
lightweight C11 toolchain contract, deterministic probe laboratory, and
dual-product NXVM-successor direction have all been documented and verified.

## M1 Closure

M1 is complete. The full NXVM `src/` baseline is provenance-recorded, builds on
Windows with the default GCC toolchain, and has bounded FDD/HDD execution
evidence. Its immutable snapshot branch is created from the verified M1 commit
before M2 begins.

## M2 Closure

M2 is complete. Shared Core Architecture Requirements V2 defines the
`nxvm.exe` and `ntvdm64.exe` product split, `nxvm.full_pc` and
`ntvdm64.dos_minimal` profiles, lifecycle, ownership, registries, abstract
host-service and trace boundaries, and M3 regression strategy. The bounded M3
Task/subtask breakdown was executed and M3 is complete.

## M4 Closure

M4 is complete. Firmware, profile/provider, CPU verification, and retained
NXVM Console contracts now bound M5 implementation without changing runtime
behavior.

## M5 Reopened Design

T63 closed the second-pass naming window. M5 is reopened at T64 for the
remaining selected-session-global and explicit instance-authority design. The
retained NXVM full PC still has one composition-owned storage authority, an
acknowledged debugger pause boundary, one product-neutral debugger backend, a
single `src/version.*` identity source, recorded platform ownership boundaries,
and recorded retained-alias exceptions; its execution still resolves those
objects through transitional current-machine aliases.

## M5 Continuity

M5 T19 S1 repaired a full-PC display snapshot performance regression found
after T18 closure. The recorded T13-to-T18 bisection identifies `ff4cd24` as
the first bad commit; its replacement preserves the snapshot contract while
restoring the unchanged-display fast path.
M5 T20 S1 then renamed the shared keyboard provider unit to
`keyboard_interface` without changing its API or behavior.
M5 T21 S1 renamed the core machine event-observer unit to `trace_interface`
without changing trace semantics.
M5 T22 S1 audited the retained debugger and defined its pause-boundary and
unified-backend migration; implementation remains inactive pending review.
M5 T22 S2 corrected the authority model: the live full PC must converge into
one machine instance without parallel CPU/RAM/port state.
M5 T22 S3 schedules device-by-device live-machine convergence; task numbers
were later adjusted by T32's interface-naming prerequisite, and debugger pause
and backend work are now T45 and T46.
M5 T22 S4 defines the no-copy, same-object compatibility method and stop gates
for that convergence. M5 T23 S1 is complete: root composition owns a
non-owning live-machine carrier bound directly to the existing CPU, decoder,
RAM, and port objects without any lifecycle behavior change.
M5 T24 S1 is complete: the canonical `cpu.*` and `cpu_instructions.*` units
now contain the retained CPU/executor implementation, and the live-machine
carrier owns their one full-PC storage instance. The legacy spellings are
direct aliases only; no global CPU/decoder storage remains.
M5 T32 S1 completed the explicit core-machine interface/provider naming
boundary without changing retained machine or Console behavior.
M5 T33 S1 moved CMOS/RTC to the live-machine-owned `cmos_storage` object while
retaining its lifecycle and port-registration order.
M5 T34 S1 moved FDD media state to the live-machine-owned `fdd_storage` object
while retaining image-buffer and transfer behavior.
M5 T35 S1 moved FDC controller state to the live-machine-owned `fdc_storage`
object while retaining its port, DMA, IRQ, and FDD callback order.
M5 T36 S1 moved HDD media state to the live-machine-owned `hdd_storage` object
while retaining image-buffer allocation, transfer, and teardown behavior.
M5 T37 S1 recorded that the retained `vhdc` is deliberately stateless: it has
only empty lifecycle hooks and no controller, port, DMA/IRQ, BIOS, or media
authority to migrate.
M5 T38 S1 moved default-profile BIOS/POST and interrupt-registration state to
the live machine's `default_bios_storage` object while preserving the retained
profile ownership and POST sequence.
M5 T39 S1 moved the default-profile QDX interrupt-dispatch table to the live
machine's `default_qdx_storage` object while preserving firmware registration
and dispatch semantics.
M5 T40 S1 confirmed that default-profile CGA firmware has no separate mutable
state: text/cursor/snapshot state remains in the already bound RAM and video
adapter, while its mode table is private immutable profile data.
M5 T41 S1 confirmed that default-profile keyboard firmware has no separate
mutable state: its provider is bound through core keyboard ingress while flags
and buffering remain in BIOS RAM.
M5 T42 S1 confirmed that default-profile disk firmware has no separate mutable
state: it registers INT 13 handlers in the bound QDX table and accesses the
existing media authorities through the core block interface.
M5 T43 S1 moved VM debug breakpoint, trace, recording, and stop-callback state
to the live machine's `debug_storage` object while retaining the existing
debugger commands and target binding.
M5 T44 S1 closed full-PC authority: a live-loop/reset smoke proves every
retained accessor names the composition-owned machine object, and a static gate
rejects remaining legacy full-PC storage definitions.
M5 T45 S1 added an acknowledged execution-boundary pause protocol. Explicit,
breakpoint, trace, and step pauses now retain the one live loop until continue
or stop, without launching a second VM thread.
M5 T46 S1 extended the product-neutral debug target with pause state, reason,
request, continue, and step operations, and proved that its VM adapter reaches
the one live full-PC authority.
M5 T47 S1 closed the missing version foundation: `vm/main.c` no longer owns
product identity or build timestamp strings, and `src/version.*` formats the
retained task-level NXVM banner.
M5 T48 S1 reopened M5 for naming convergence: low-risk device/file renames,
interface/provider naming cleanup, stale-document removal, and the
`0.5.NNNN` task-version artifact rule are planned before further source moves.
It is complete as a design subtask and produced no executable.
M5 T48 S2 completed the first naming implementation task: shared core
`keyboard_controller.*` is now `kbc.*`, shared core `video_adapter.*` is now
`vadp.*`, and `build/output/nxvm_0_5_0048.exe` is the verified task artifact.
M5 T49 S1 completed VM-private device file naming: `vcmos`, `vdebug`, `vfdd`,
`vfdc`, `vhdd`, and `vhdc` file pairs now use canonical owner filenames, and
`build/output/nxvm_0_5_0049.exe` is the verified task artifact.
M5 T50 S1 completed default-profile BIOS file naming: `vbios.*` is now
`bios.*`, and `build/output/nxvm_0_5_0050.exe` is the verified task artifact.
M5 T51 S1 completed the first interface/provider naming split: block and wait
provider contracts now live in `*_provider.h`, core firmware public names now
use the `core_machine_firmware_*` prefix, and
`build/output/nxvm_0_5_0051.exe` is the verified task artifact.
M5 T52 S1 completed the first ambiguous implementation-name audit:
`machine_memory.c`, `machine_port.c`, and `presentation.c` are now
`memory_interface.c`, `port_interface.c`, and `presentation_interface.c`; core
presentation public names now use `core_machine_*`, and
`build/output/nxvm_0_5_0052.exe` is the verified task artifact.
M5 T53 S1 completed the first `vglobal.h` retirement slice: retained NXVM
numeric aliases, bit/constant helpers, BCD helpers, and `ExecFun` now live in
`type.h`; `vglobal.h` remains only as a transitional include, and
`build/output/nxvm_0_5_0053.exe` is the verified task artifact.
M5 T54 S1 completed the final naming-convergence audit: source roots are only
`src/core`, `src/vm`, and `src/vdm`; source/CMake no longer contain the old
renamed file paths; remaining `v*` names are intentional retained runtime
aliases or historical/provenance references; and
`build/output/nxvm_0_5_0054.exe` is the verified first closure artifact.
M5 T55 S1 reopened second-pass naming cleanup, froze the T55 through T63 plan,
deferred M6 again, and produced `build/output/nxvm_0_5_0055.exe`.
M5 T56 S1 corrected current/future documentation facts for the future
`vdm/main.c` entry point and actual `type.h` contents, and produced
`build/output/nxvm_0_5_0056.exe`.
M5 T57 S1 renamed VM default-profile firmware public symbols to
`vm_profile_default_firmware_*`, preserved firmware/profile behavior, and
produced `build/output/nxvm_0_5_0057.exe`.
M5 T58 S1 renamed low-risk VM session/composition public symbols to
`vm_composition_session_model_*` and `vm_composition_full_pc_session_*`,
preserved session behavior, and produced `build/output/nxvm_0_5_0058.exe`.
M5 T59 S1 audited VM platform Win32/Linux ownership while preserving symmetric
`win32app`, `win32con`, `linuxapp`, and `linuxcon` host-surface naming. No
platform source file was moved; the audit found no safe whole-file T60
migration and produced `build/output/nxvm_0_5_0059.exe`.
M5 T60 S1 closed the platform migration decision with a no-move result: shared
policy-free platform mechanisms are already in `src/core/platform`, and the
remaining Win32/Linux VM platform files stay in `src/vm/platform` until future
context-object extraction work. It produced `build/output/nxvm_0_5_0060.exe`.
M5 T61 S1 inventoried retained runtime aliases, separating current-object
compatibility spellings from executor/session state-authority debt. No stateful
alias was removed, and it produced `build/output/nxvm_0_5_0061.exe`.
M5 T62 S1 recorded that no retained runtime alias is safe to remove inside this
naming window without beginning explicit context-passing and multi-session
state-authority work. It produced `build/output/nxvm_0_5_0062.exe`.
M5 T63 S1 completed the final second-pass naming closure audit over source
roots, CMake, current docs, public symbols, platform ownership, and retained
alias exceptions. It produced `build/output/nxvm_0_5_0063.exe`.

## Active Work

M5 T66 S1 is active: migrate CPU/executor mutable authority from retained
aliases to one explicit session context. M6 T1 is deferred and DOS
implementation remains ineligible.

## Completed

- M5 T65 S1: made the VM root, full-PC wrapper, CPU probe, control loop,
  execution context, Console target, and debug target caller/session owned.
  Full GCC, FDD/HDD profile/session, retained Console, CPU-probe, and bounded
  FDD `INT 21h` watchdog gates passed; the `0.5.0065` artifact is recorded.

- M0 T1 S1 P1: repository governance and architecture foundation.
- M0 T1 S1 P2: recorded the project MIT license after integrating the remote
  initial commit.
- M0 T1 S2: recorded the initial CMake/Visual Studio 2022 contract and added a
  deterministic generated M1 DOS probe; the default toolchain was later
  replaced by M0 T5.
- M0 T2 S1: pinned the local NXVM reference and its then-current upstream
  license identity without importing source.
- M0 T2 S2: recorded NTVDMx64 behavioral-only and WineVDM process-only
  boundaries.
- M0 T3 S1: reviewed M0 closure conditions.
- M0 T4 S1: adopted the initial NXVM-foundation architecture correction, source
  policy, redistributability rules, and directory plan.
- M0 T4 S2: downgraded Microsoft component work from a planned backend to
  then-current research milestones and moved Microsoft material to the research
  boundary. This numbering was superseded by later roadmap revisions.
- M0 T5 S1: corrected milestone/snapshot governance and selected MinGW-w64 GCC
  with CMake and Ninja as the default lightweight C11 toolchain.
- M0 T5 S2: verified the x64 GCC/CMake/Ninja C11 configuration and deterministic
  probe build; adopted the Windows 7 through Windows 11 support policy.
- M0 T6 S1: recorded the future `ntvdm64 run` CLI, display, debugger, host-drive
  visibility, cancellation, and Console-ownership contract without beginning
  implementation.
- M0 T6 S2: changed M1 to a full NXVM baseline-first, subtractive-refactoring
  strategy; recorded its local image-fixture boundary without beginning M1.
- M0 T6 S3: retained the existing Linux platform source in the M1 baseline and
  recorded the future shared `machine + dos` portability direction without
  adding a Linux delivery commitment.
- M0 T7 S1: fixed the owned-DOS profile, service allowlist, memory and
  filesystem boundaries, acceptance probes, and escalation rule without
  beginning implementation. Its milestone placement was superseded by M0 T13.
- M0 T8 S1: recorded the NXVM copyright-holder authorization for MIT imports
  into ntvdm64, while retaining upstream-license capture as historical evidence.
- M0 T9 S1: defined optional differential verification interfaces for machine
  and DOS traces, external-reference boundaries, watchdogs, and cleanup without
  beginning M1 or M2.
- M0 T10 S1: resequenced the then-current delivery plan into baseline,
  architecture, machine refactor, DOS, Platform/CLI, and product milestones;
  clarified M1 fixtures and CLI debug ownership without beginning M1. This
  sequence was superseded by M0 T13.
- M0 T11 S1: closed remaining planning ambiguities around implementation
  dependencies, milestone authority, M1 experiment freezing, containment
  ownership, M3 planning, corpus policy, toolchain identity, and local BYOB
  research. Its old milestone numbering was superseded by M0 T13.
- M0 T12 S1: consolidated the then-current optional-research milestones,
  deferred all product CLI work, and merged redundant short planning,
  requirement, and research documents without beginning M1. This numbering was
  superseded by M0 T13.
- M0 T12 S2: clarified the then-future project-owned pre-product Console
  `load` command, required it to enter through the owned DOS loader, and made
  GCC migration design the mandatory first M1 task without beginning M1.
- M0 T12 S3: constrained debugger `load` to an empty paused debug session and
  aligned its reset, first-instruction, and rejection semantics without
  beginning implementation.
- M0 T12 S4: removed premature implementation subtasks and required each
  preceding design milestone to produce the bounded Task/subtask breakdown for
  its implementation successor without beginning M1.
- M0 T12 S5: defined the ignored local executable-artifact convention for
  completed runnable subtasks and recorded the M1 baseline developer artifact.
- M0 T12 S6: defined runtime identity, pre-cutover task version suffixes, the
  post-cutover DOS Machine banner, and post-cutover version encoding.
- M0 T13 S1: reset governance to the dual-product shared-core direction:
  `nxvm.exe` remains a bootable whole-machine VM, `ntvdm64.exe` becomes the
  non-bootable DOS app runner, and both share `core`, `firmware`, `platform`,
  `runtime`, and product-specific composition.
- M0 T13 S1 P3: clarified the product interaction boundary: NXVM retains its
  interactive whole-machine Console without a new process CLI; ntvdm64 owns
  `run` and its window-mode control Console.
- M1 T1 S1: approved the bounded GCC migration design after a zero-patch GCC
  compile and warning inventory.
- M1 T2 S1: imported and built the full provenance-recorded NXVM baseline;
  recorded bounded FDD/HDD instruction-trace evidence, the raw M1 snapshot,
  and the post-snapshot M1 T2 build identity banner.
- M2 T1 S1: approved Machine/DOS Architecture Requirements V1, including
  explicit boot and DOS-minimal profiles, lifecycle and ownership rules,
  versioned interface contracts, M1 regression mapping, and the bounded M3
  Task/subtask breakdown. No runtime behavior changed. This was superseded by
  M2 T1 S2.
- M2 T1 S2: revised M2 to Shared Core Architecture Requirements V2, promoting
  `nxvm.full_pc` to a first-class product profile, adding explicit registry
  boundaries, and resequencing firmware, DOS, platform, product, compatibility,
  and research milestones without beginning M3.
- M2 T1 S3: adopted parity-first module headers for M3: contract headers live
  beside C implementations in `src/core/`, private headers use `_impl.h`, and a
  top-level `include/` tree is deferred until a future packaging/SDK need.
- M3 T1 S1: defined the Shared Core V1 contract skeleton with module-local
  `src/core/*.h` headers, minimal `src/core/*.c` implementation, CMake target,
  compile/link smoke test, and forbidden-dependency scan. No baseline behavior
  moved.
- M3 T1 S2: created independent core CPU reset state, range-checked RAM with
  explicit A20 state, and session-local typed port dispatch. The untouched M1
  baseline remains the temporary full-PC regression adapter; verification and
  provenance record the distinction.
- M3 T2 S1: made the core Machine lifecycle explicit with observable state,
  finite budget boundaries, atomic requested-stop observation, and stable fault
  reporting. No guest executor or platform thread was introduced.
- M3 T3 S1: made `nxvm.full_pc` an explicit runtime profile and retained
  baseline adapter, with declared whole-PC inventory and FDD/HDD reset-vector
  verification.
- M3 T3 S2: created the no-media `ntvdm64.dos_minimal` profile shell with
  deterministic PIC/PIT, keyboard IRQ/port, and text-snapshot boundaries.
- M3 T4 S1: isolated presentation through adapter-owned input queues and
  copied text snapshots, without platform access to core internals.
- M3 T4 S2: added paused-state Machine debug commands for CPU/memory inspection
  and finite step/continue execution without baseline-global access.
- M3 T5 S1: composed runtime-owned profile sessions, completed the M3 matrix,
  revalidated fixture identities and retained M1 regression evidence, and
  recorded M4 design inputs.
- M3 T5 S1 P4: corrected the local developer artifact to the final bootable
  NXVM Console executable; smoke executables remain build verification only.
- M4 T1 S1: assigned full-PC baseline ownership, M5 migration order, and M1
  regression ownership without changing runtime behavior.
- M4 T2 S1: defined session-owned Firmware V1 service, reset, failure,
  host-capability, and trace contracts without changing runtime behavior.
- M4 T3 S1: defined the retained NXVM Console, presentation, debugger, and
  artifact contract without adding a process CLI or changing runtime behavior.
- M4 T4 S1: froze M5's bounded implementation breakdown, shared verification
  gates, task artifact rule, and closure criteria.
- M4 T5 S1: split NXVM machine profiles from ntvdm64 execution profiles,
  defined firmware providers and BYOR ROM manifests, and limited M5 to
  `nxvm.machine.pc_at_builtin`.
- M4 T6 S1: defined evidence-backed CPU capabilities, `#UD` triage, and the
  optional local Bochx/Bochs CPU differential boundary; M5 now starts there.
- M5 T15 S1 P1: reopened M5 for approved legacy convergence, recorded the
  exact `device.h` consumer map and closure gates, and replaced its
  composition-owned control surface with a private root-composition API. GCC,
  fixture lifecycle, expected-`#UD`, dependency-DAG, and retained Console
  gates passed; the aggregate remains pending further narrow replacements.
- M5 T15 S1 P2: removed the obsolete `device` global and lifecycle/control
  declarations after their composition-control replacement. The affected GCC
  artifact and focused smoke targets rebuild without any old control-symbol
  reference.
- M5 T15 S1 P3: introduced direct core CPU/RAM/port access paths over the
  retained single executor state and moved debugger, probe, reset-vector, and
  memory-sizing callers to them. GCC, debug-target, expected-`#UD`, and
  FDD/HDD profile gates passed; legacy exports remain only for unconverted
  callers pending final deletion.
- M5 T15 S1 P4: moved VM hardware-debug controls to `vm_machine_debug_*` in
  `vdebug.h`; the core debugger adapter, Console recorder, and full-PC session
  no longer access them through `device.h`. GCC, debugger-target, and Console
  gates passed.
- M5 T15 S1 P5: moved retained BIOS/media controls to narrow VM machine and
  default-profile headers; the Console and full-PC session no longer require
  the aggregate for these operations.
- M5 T15 S1 P6: bound text display capture through the core snapshot-provider
  contract, preserving the default-profile text state and existing renderers.
- M5 T15 S1 P7: bound host keyboard state and keypress delivery through the
  core keyboard-provider contract; VM composition owns the provider binding.
- M5 T15 S1 P8: removed the now-unused composition keyboard bridge and all
  inactive direct `device.h` includes; aggregate deletion is the next bounded
  slice.
- M5 T15 S1 P9: deleted `vm/machine/device.h` after the final inactive
  includes were removed. Windows GCC, keyboard input, expected-`#UD`, FDD/HDD
  reset-vector, retained Console `HELP`/`EXIT`, and the zero-edge DAG verifier
  passed with no Console or machine behavior change.
- M5 T16 S1 P1: deleted uncalled RAM/port `deviceConnect*` compatibility
  exports. The existing core-machine operations remain the only public path;
  GCC, expected-`#UD`, FDD/HDD reset-vector, and DAG gates passed.
- M5 T16 S1 P2: deleted uncalled CPU `deviceConnect*` compatibility exports.
  The existing `core_machine_cpu_*` operations remain the only active path;
  GCC, CPU probe/stop, debugger-target, FDD/HDD reset-vector, and DAG gates
  passed.
- M5 T16 S1 P3: renamed default-profile display helpers behind the existing
  snapshot provider. GCC, core presentation, default-profile FDD fixture, and
  DAG gates passed without a text-state or frame-layout change.
- M5 T16 S1 P4: renamed default-profile keyboard helpers behind the existing
  core keyboard provider. GCC, platform-input, default-profile FDD, full-PC
  FDD/HDD, and DAG gates passed without BIOS keyboard-state or IRQ changes.
- M5 T17 S1 P1: moved the shared display-frame mailbox to `core/platform` and
  gave it the `core_platform_display_*` contract. GCC, core presentation,
  VM-platform execution, default-profile FDD, and DAG gates passed.
- M5 T17 S1 P2: moved concrete Win32/Linux sleep to `core/platform` and bound
  it through the existing core-product wait provider. GCC, core wait,
  VM-platform execution, and DAG gates passed; POSIX compilation remains an
  external validation gate.
- M5 T17 S1 P3: completed the platform ownership audit. The remaining VM
  platform files retain VM callback, keyboard mapping, Console/window, or
  execution-loop policy; core platform has no machine/product include.
- M5 T18 S1 P1: closed M5 with `build/nxvm-m5-t18.exe`. Windows GCC, zero-edge
  DAG, expected-`#UD`, debugger-target, input/execution, FDD/HDD reset-vector,
  retained Console `HELP`/`EXIT`, and legacy/forbidden-dependency scans passed.
- M5 T18 S1 P2: removed the unused hand-managed CPU capability manifest and
  its dedicated smoke. The registry retains its generic capability-query test;
  GCC, registry/core smokes, and the DAG verifier passed.
- M5 T18 S1 P3: removed the unused aggregate PIC/PIT/DMA model and its dedicated
  smoke. GCC, core contract, retained NXVM artifact, and DAG gates passed.
- M5 T19 S1 P1: restored the default display provider's unchanged-display fast
  path after a T13-to-T18 full-PC FDD regression bisection.
- M5 T20 S1 P1: renamed the shared core keyboard provider unit to
  `keyboard_interface` and rebuilt the task artifact.
- M5 T21 S1 P1: renamed the core machine trace observer unit to
  `trace_interface` and rebuilt the task artifact.
- M5 T22 S1 P1: recorded the retained debugger audit and migration design
  without changing runtime code.
- M5 T22 S2 P1: recorded the live-machine authority migration rules and source
  map before debugger implementation.
- M5 T22 S3 P1: replaced the premature debugger implementation sequence with
  device-by-device live-machine convergence through T44.
- M5 T22 S4 P1: recorded the controlled no-copy migration method and rollback
  gates before T23 implementation.
- M5 T23 S1 P1: added the composition-owned live-machine carrier with direct,
  non-owning references to the existing `vcpu`, `vcpuins`, `vram`, and `vport`
  objects. It allocates no guest storage and invokes no device lifecycle path.
  GCC, same-object identity, expected-`#UD`, FDD/HDD reset-vector, retained
  Console `HELP`/`EXIT`, and DAG gates passed; `nxvm-m5-t23.exe` is the task
  artifact.
- M5 T25 S1 P1: mapped the retained full-PC `vram` allocation, A20 state,
  port `0x92` callbacks, physical/real address semantics, lifecycle order,
  and direct consumers. The separate M3 minimal-machine memory buffer remains
  outside full-PC composition; T25 S2 has a same-object migration plan and no
  mapping-only artifact is required.
- M5 T25 S2 P2: recorded owner-observed FDD DOS-prompt acceptance for the
  T25 through T31 authority artifacts; the prior scripted Console timeout
  remains historical automation-boundary evidence only.
- M5 T32 S1 P1: completed explicit core-machine interface and provider naming
  without machine, callback-order, Console, or debugger behavior changes.
- M5 T33 S1 P1: moved the VM-only CMOS/RTC provider state into the live
  machine with direct alias access; GCC, authority, FDD/HDD profile, debugger,
  Console, and DAG gates passed.
- M5 T34 S1 P1: moved VM-only FDD media state into the live machine with
  direct alias access; GCC, authority, FDD/HDD profile, debugger, Console,
  and DAG gates passed.
- M5 T9 S1: renamed the built-in PC/AT profile implementation to
  `default_profile` without changing emulated-machine identity.
- M5 T9 S2: moved the original NXVM entry point, Console, and hardware
  debugger to `src/product/vm`, and assembler/disassembler to
  `src/product/core/debug/xasm32`; retained `utils` with its unchanged
  baseline consumers. The original no-media Console and debugger interactions
  were revalidated.
- M5 T10 S1: mapped the actual machine/device/refresh/CPU execution chain,
  bounded its real source-move order, temporary state binding, and regression
  gates without changing runtime behavior.
- M5 T10 S2: moved the original `machine`, `device`, and `vmachine`
  implementations into `src/machine/vm`; baseline headers are temporary
  forwarding compatibility layers only. No-media Console/debugger and full-PC
  profile-link checks passed.
- M5 T10 S4: replaced the temporary carrier binding with a session-owned
  execution context and preserved the retained loop ordering; sequential CPU,
  bounded execution-thread lifecycle, FDD/HDD reset-vector, Console, and
  debugger gates passed.
- M5 T10 S5: verified `nxvm-m5-t10.exe` compiles lifecycle, loop, CPU, RAM,
  port, and instruction dispatch only from their migrated module owners. The
  verified task artifact is recorded in its closure evidence.
- M5 T11 S8: moved the retained HDD implementation into `machine/vm`, with
  forwarding-header compatibility only; GCC, lifecycle, CPU, FDD/HDD reset,
  Console, and debugger regressions passed.
- M5 T11 S9: moved the retained FDC implementation into `machine/vm`, with
  forwarding-header compatibility only; GCC, lifecycle, CPU, FDD/HDD reset,
  Console, and debugger regressions passed.
- M5 T11 S10: moved the retained HDC implementation into `machine/vm`, with
  forwarding-header compatibility only; GCC, lifecycle, CPU, FDD/HDD reset,
  Console, and debugger regressions passed.
- M5 T11 S11: moved retained hardware debug state into `machine/core`, with
  forwarding-header compatibility only; GCC, lifecycle, CPU, FDD/HDD reset,
  Console, and debugger regressions passed without enabling the raw recorder.
- M5 T11 S12: audited QDX ownership; its dispatcher/handlers are default-profile
  firmware, and the direct platform display call requires T12 boundary work
  before a safe QDX source move.
- M5 T12 S1: moved the retained VM platform hub into `platform/vm`, preserving
  its original static-link resolution and passing GCC, lifecycle, CPU,
  FDD/HDD reset, Console, and debugger regressions.
- M5 T12 S2: audited the Win32 dispatch layer; host threads directly mutate
  keyboard/stop state, so an inactive copied-request bridge is required before
  a safe source move.
- M5 T12 S3: added a project-owned inactive copied-request bridge and focused
  FIFO smoke; it has no producer or consumer and all retained regressions pass.
- M5 T12 S4: defined adapter-owned synchronized ingress/egress transport and
  teardown ordering; it avoids `machine <-> platform` dependencies and changes
  no runtime behavior.
- M5 T12 S5: added the inactive synchronized adapter transport and focused
  ownership/lifecycle smoke; all retained regressions pass without a caller.
- M5 T12 S6: bound the inactive transport observer to the retained execution
  boundary; no producer, request consumption, or guest mutation is enabled.
- M5 T12 S7: selected keyboard-state synchronization as the first producer;
  key-down, F9, and display remain direct, with enqueue failure preserving the
  original status path.
- M5 T12 S8: flattened `machine/core/contract` into `machine/core`, removed
  `src/core` forwarding headers, and revalidated the core, profile, product,
  and retained NXVM build gates without changing user-visible behavior. The
  paths are now migration sources under the later `core/vm/vdm` topology.
- M5 T12 S9: migrated the remaining NXVM implementations into `core`, `vm`,
  and `vdm`, removed the baseline and adapter source roots, removed formal
  baseline source/include dependencies, and revalidated GCC, CPU, Console, and
  debugger gates without changing user-visible behavior.
- M5 T12 S10: switched only retained Win32 KEYUP/FOCUS keyboard-state events
  through copied ingress at the execution boundary; KEYDOWN mapping remains
  direct, and queue failure preserves the original direct state path. GCC,
  transport, CPU, FDD/HDD, Console, and debugger gates passed.
- M5 T13 S1: audited historical composition names separately from real
  core-to-product ownership debt; no runtime behavior changed.
- M5 T13 S2: moved the active full-PC composition pair to its final VM owner
  name and revalidated CPU, FDD/HDD, Console, and debugger gates.
- M5 T13 S3: moved the VM request transport to its final owner name and
  revalidated transport, CPU, FDD/HDD, Console, and debugger gates.
- M5 T13 S4: moved the VM-only CPU probe to its final owner name and
  revalidated its `#UD` output plus the retained compatibility matrix.
- M5 T13 S5: converted real core-to-product dependency debt into five bounded
  implementation slices; no runtime source changed.
- M5 T13 S6: moved the shared presentation type into core; VDM retains its
  private PIT/IRQ state and its private snapshot embeds the shared text child,
  which its platform adapter copies across the boundary.
- M5 T13 S7: defined the product-owned session/profile replacement API and
  test migration map; no runtime source changed.
- M5 T13 S8: moved static descriptors and session composition to VM and VDM
  product owners; `core/product/runtime` now contains no VM/VDM selection or
  product include. Focused product/session and retained NXVM gates passed.
- M5 T14 S1: adopted the product-composition dependency DAG and made it the M5
  source/target closure criterion. No runtime or build-graph change occurred.
- M5 T14 S2: recorded the target-by-target dependency migration map; froze the
  approved halt, block-capability, video-notification, and legacy lifecycle
  directions; and added source/CMake dependency checks. No runtime behavior
  changed.
- M5 T14 S3: routed supported INT 10h screen reconfiguration through core
  notification and VM composition, then moved shared text state to core and
  published copied VM-platform display frames. Win32 renderers no longer
  include VM machine state; Linux painting uses the same frame while retaining
  its lifecycle/input adapter. The temporary source allowlist is now 8 edges;
  full GCC, dependency, CPU, retained UX, and FDD/HDD session gates passed.
- M5 T14 S3: replaced the legacy core-product-to-VM-platform sleep call with a
  bound core-product wait provider. VM composition owns the platform binding;
  legacy `utilsSleep` callers and timing arguments remain unchanged. A focused
  provider smoke and the full retained NXVM gates passed; the temporary source
  allowlist is now 7 edges.
- M5 T14 S3: replaced Win32 keyboard-machine calls with a VM-platform input
  sink bound by composition. Key mapping, F9 fallthrough, and existing Console
  and window call order remain unchanged. The focused input smoke and full
  retained NXVM gates passed; the temporary source allowlist is now 6 edges.
- M5 T14 S3: replaced Win32 Console/window lifecycle calls with a VM-platform
  execution sink bound by composition. Existing threads, polling intervals,
  message loop, and close ordering remain unchanged. The focused execution
  smoke and full retained NXVM gates passed; the temporary source allowlist is
  now 4 edges.
- M5 T14 S3: routed the retained full NXVM Console through a product command
  target supplied by VM composition. Console text, parser, default media
  behavior, and command order remain retained; interactive HELP/INFO/EXIT and
  the full GCC regression suite passed. The temporary source allowlist is now
  2 edges.
- M5 T14 S3: routed Linux Console display, input, and execution through the
  shared VM-platform contracts. The source DAG has no Linux machine edge;
  Windows GCC and static verification passed, while POSIX GCC/ncurses build
  evidence remains pending external validation.
- M5 T14 S3: created the typed core debugger-target contract and fake-target
  smoke. It covers shared debugger operations without VM headers; VM/VDM
  composition adapters and the retained UI source move remain pending.
- M5 T14 S3: bound the core debugger target in VM composition and verified
  register and real-memory callbacks against the initialized NXVM machine.
- M5 T14 S3: moved the retained debugger command UI and header into
  `core/product/debug` after replacing all direct device operations with the
  core debug-target access contract. VM composition supplies the retained
  register, memory, port, execution, break/trace, watchpoint, and diagnostic
  callbacks. The source-DAG allowlist is now empty; GCC and focused debugger
  target smokes passed. Retained interactive debugger and full-PC gates remain
  required before this slice is accepted.
- M5 T14 S3: removed the hand-selected full-PC source aggregation from every
  retained task artifact. Each `nxvm-m*` executable now compiles only the
  thin `vm/main.c` entry point and links the common VM product/lifecycle graph;
  no user-visible behavior or artifact identity changes.
- M5 T14 S3: split VM controller implementation sources from the retained
  lifecycle target into the peer `nxvm-vm-machine` CMake target. The lifecycle
  target now compiles root composition sources only and links that machine
  target; the temporary aggregate link dependency remains until the next
  target-graph slice.
- M5 T14 S3: renamed that root target to `nxvm-vm-composition` and replaced
  the temporary `nxvm-legacy-runtime-dependencies` aggregate with explicit
  module links. Retained task artifacts and focused VM smoke targets now link
  the root composition target directly.
- M5 T14 S3: split the mixed historical firmware library into a core-machine
  firmware target and a VM default-profile firmware target. The old
  `nxvm-firmware` name remains only as a zero-source test aggregate.
- M5 T14 S3: removed the final CMake mixed-owner target exception list. The
  configured source-owner verifier now rejects every target that aggregates
  sources from multiple module owners.
- M5 T14 S3: renamed the retained core machine implementation target to
  `nxvm-core-machine`; it compiles only `core/machine` sources.
- M5 T14 S3: reran the fixture-backed VM session and profile gates after the
  target graph changes. A bounded piped Console `START/STOP` attempt retained
  the historical full-machine execution thread beyond its host timeout and was
  terminated; it is recorded as an unsuitable automated stop boundary, not a
  changed product behavior or replacement for the fixture gates.
- M5 T14 S3: consolidated pure `vm/product` implementations (Console media,
  presentation, and session UX helpers) into `nxvm-vm-product`; their retained
  smoke executables now link the module target rather than historical narrow
  product libraries.
- M5 T14 S3: consolidated VM profile metadata and default-profile firmware
  under `nxvm-vm-profile`; historical profile smoke executables now link that
  module target rather than narrow profile libraries.
- M5 T14 S3: consolidated all remaining `vm/composition_*.c` sources under
  `nxvm-vm-composition`. Full-PC, CPU-probe, default-profile, and session
  smoke executables now link the root composition target directly.
- M5 T14 S3: recast the test-only VDM minimal shell as explicit machine,
  profile, and composition targets. No VDM executable, DOS runner, or CLI was
  added.
- M5 T14 S3: completed the M5 closure audit. The full GCC build, all available
  core/VM/VDM smokes, source-DAG with zero allowed edges, source-root scan,
  local FDD/HDD fixture session/profile checks, and retained NXVM Console smoke
  passed. The task artifact `ntvdm64-m5_t14.exe` is built from the same VM
  composition graph with the `m5t14` identity.
- M5 T48 S1 P1: froze the M5 naming-convergence task breakdown,
  stale-document cleanup, and `0.5.NNNN` task-version artifact rule.
  Design subtask; no executable artifact until T48 S2 completes T48.
- M5 T48 S2 P1: renamed shared core KBC/VADP files and current-object
  functions, preserved retained aliases and behavior, and built
  `build/output/nxvm_0_5_0048.exe`.
- M5 T49 S1 P1: renamed VM-private CMOS/debug/FDD/FDC/HDD/HDC file pairs to
  canonical owner filenames, preserved retained aliases and behavior, and
  built `build/output/nxvm_0_5_0049.exe`.
- M5 T50 S1 P1: renamed default-profile BIOS firmware files to
  `bios.c/.h`, preserved retained `vbios` compatibility names and behavior,
  and built `build/output/nxvm_0_5_0050.exe`.
- M5 T51 S1 P1: split block and wait provider contracts into
  `block_provider.h` and `wait_provider.h`, renamed core firmware public
  symbols to `core_machine_firmware_*`, preserved retained behavior, and built
  `build/output/nxvm_0_5_0051.exe`.
- M5 T52 S1 P1: renamed ambiguous core machine implementation files to their
  interface owners, renamed core presentation public symbols to
  `core_machine_*`, preserved retained behavior, and built
  `build/output/nxvm_0_5_0052.exe`.
- M5 T53 S1 P1: moved retained NXVM numeric aliases and constants from
  `vglobal.h` to `type.h`, preserved `vglobal.h` as a compatibility include,
  and built `build/output/nxvm_0_5_0053.exe`.
- M5 T54 S1 P1: closed naming convergence with source/CMake/docs scans,
  recorded intentional retained aliases and historical references, and built
  `build/output/nxvm_0_5_0054.exe`.
- M5 T55 S1 P1: reopened second-pass naming cleanup, froze T55 through T63,
  deferred M6 again, and built `build/output/nxvm_0_5_0055.exe`.
- M5 T56 S1 P1: corrected current/future documentation facts around the
  future `vdm/main.c` entry point and actual `type.h` contents, and built
  `build/output/nxvm_0_5_0056.exe`.
- M5 T57 S1 P1: renamed VM default-profile firmware public symbols to
  `vm_profile_default_firmware_*` and built
  `build/output/nxvm_0_5_0057.exe`.
- M5 T58 S1 P1: renamed low-risk VM session/composition public symbols to
  `vm_composition_session_model_*` and
  `vm_composition_full_pc_session_*`, and built
  `build/output/nxvm_0_5_0058.exe`.
- M5 T59 S1 P1: audited VM platform ownership, preserved symmetric
  host-surface filenames, recorded that no whole VM platform file is safe for
  T60 migration, and built `build/output/nxvm_0_5_0059.exe`.
- M5 T60 S1 P1: recorded the no-move platform migration decision, preserved
  symmetric host-surface filenames, left VM platform files in place, and built
  `build/output/nxvm_0_5_0060.exe`.
- M5 T61 S1 P1: inventoried retained runtime aliases, classified cleanup risk,
  removed no stateful alias, and built `build/output/nxvm_0_5_0061.exe`.
- M5 T62 S1 P1: recorded that no retained runtime alias cleanup is safe inside
  this naming window, preserved source behavior, and built
  `build/output/nxvm_0_5_0062.exe`.
- M5 T63 S1 P1: completed the final second-pass naming closure audit and built
  `build/output/nxvm_0_5_0063.exe`.

## Next Eligible Work

- Complete M5 T65 through T73 instance-authority migration. The task order and
  hard deletion gates are in `planning/m5-instance-authority-plan.md`.
- M6 T1 may resume only after that plan is complete. DOS implementation remains
  ineligible until M6 creates and closes the M7 implementation breakdown.

## Delivery State

The M1 NXVM import is preserved by MIT authorization, copyright attribution,
Git history, and exact provenance records; its temporary source root has been
deleted. The repository direction is dual-product: future `nxvm.exe` preserves
bootable whole-machine VM value, and future `ntvdm64.exe` provides the DOS app
runner. No guest media, Microsoft binary, or DOS compatibility claim has been
added. The generated M1 COM probe remains test input only.
