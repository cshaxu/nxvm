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

## M5 Reopened

The former M5 closure established product composition and regression evidence,
but its user-facing full-PC execution still entered `src/nxvm-baseline/`
through a temporary adapter. M5 T12 S9 has now migrated the actual execution,
firmware, platform, retained Console/debugger sources, and deleted that source
root. M5 remains open for composition, fixture, and deferred interface work
before M6 may begin. The historical `snapshot/m5-nxvm-28fb4b1` remains a
checkpoint, not an M5 completion snapshot. Completed M5 records retain their
then-current paths as historical evidence; the forward source layout is defined
solely by `docs/architecture/module-layout.md`.

## Active Work

M5 T14 S3: remove core-to-VM/default-profile dependencies through approved
core contracts and VM root-composition bindings, beginning with the preserved
legacy `vmachine` lifecycle sequence.

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

## Active Subtask

`docs/planning/subtasks/m5-t14-s3.md`.

## Next Eligible Work

- M5 T14 S3 may remove the approved core-to-VM/default-profile dependency
  edges. T13 S9-S11 are superseded; M6 remains blocked until the new M5
  closure rule is met.

## Delivery State

The M1 NXVM import is preserved by MIT authorization, copyright attribution,
Git history, and exact provenance records; its temporary source root has been
deleted. The repository direction is dual-product: future `nxvm.exe` preserves
bootable whole-machine VM value, and future `ntvdm64.exe` provides the DOS app
runner. No guest media, Microsoft binary, or DOS compatibility claim has been
added. The generated M1 COM probe remains test input only.
