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

## M5 Status

M5 source-root and target-graph migration is complete, with the recorded GCC,
source-DAG, core/VM/VDM smoke, retained Console/debugger, and local FDD/HDD
fixture evidence intact. M5 is reopened for legacy implementation convergence:
delete the `device.h` aggregate, converge the retained executor to one
core-machine CPU/RAM/port authority, and move reusable Win32/Linux host
providers to `core/platform`. The detailed scope and gates are in
`m5-legacy-convergence.md`. The historical `snapshot/m5-nxvm-28fb4b1` remains
a checkpoint, not the eventual M5 closure snapshot.

## Active Work

M5 T15 S1 P2: replace the next `device.h` surface after the completed
composition-control split.

## Completed

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

## Next Eligible Work

- M6 design may begin. M7+ implementation remains blocked until M6 produces
  its approved architecture and bounded task breakdown.

## Delivery State

The M1 NXVM import is preserved by MIT authorization, copyright attribution,
Git history, and exact provenance records; its temporary source root has been
deleted. The repository direction is dual-product: future `nxvm.exe` preserves
bootable whole-machine VM value, and future `ntvdm64.exe` provides the DOS app
runner. No guest media, Microsoft binary, or DOS compatibility claim has been
added. The generated M1 COM probe remains test input only.
