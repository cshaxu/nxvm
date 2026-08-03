# Long-Term Review Ledger

This ledger consolidates known risks, deferred work, and recurring review
failures. It is a planning and verification aid only: it does not authorize a
new milestone, a new active subtask, a behavior change, or an external-source
import. `docs/planning/status.md` remains authoritative for active work.

## Review Rules

- Treat a directory name as a claim to verify, not proof of correct ownership.
- Require one explicit owner for each mutable runtime state and one real run
  path for each product. Test fixtures and compatibility shims are not product
  architecture merely because they compile.
- `core` never depends on `vm` or `vdm`. `core/platform` never mutates guest
  state. Product-root composition is the only cross-module assembly point.
- A session is selected only through an explicit object reference. No
  process-global, `_Thread_local`, or implicit-current-object facade may enter
  a production path. Host resources that are necessarily process-exclusive use
  an explicit lease with a documented owner and release sequence.
- Every structural task must preserve the retained NXVM Console, debugger,
  full-PC FDD/HDD boot checkpoints, and task-artifact rules unless an approved
  task explicitly changes them.

## Current Architecture Closure

### Core Machine And VM Composition

- [ ] **Core-owned generic-device lifecycle.** `vm/composition/providers.c`
  still sequences preparation and finalization for core CPU/bus/memory and
  shared VADP/KBC/DMA/PIT/PIC objects. Composition may bind VM-only devices,
  firmware, and providers, but `core_machine` must own its generic-device
  prepare/reset/finalize order. Closure: move lifecycle authority behind a
  core contract without changing the retained full-PC boot order.
- [ ] **Narrow raw core borrows.** `vm_session` currently caches raw borrows
  of CPU, RAM, port, PIC, PIT, DMA, KBC, and VADP. They are aliases of one
  live core machine, not duplicate state, but expose too much core-private
  representation to VM composition. Closure: retain only necessary explicit
  capabilities/bindings and prove no hidden second storage or reset path.
- [ ] **Public lifecycle hardening.** Audit direct machine/device initialize,
  reset, refresh, and finalize calls. The supported lifecycle must have one
  owner and fail clearly when called out of order; no public helper may create
  an alternate production executor path.
- [ ] **Composition naming and separation.** `vm/composition/providers.*`
  currently combines provider binding, default-profile wiring, and lifecycle
  sequencing. Split or rename only after the core lifecycle boundary is
  complete; do not use a cosmetic rename to hide mixed responsibility.

### Platform, Product, And Session Boundaries

- [ ] **Finish the explicit platform run-handle boundary.** The active M5
  platform work must prove that the session-owned handle is the sole owner of
  worker creation, stop request, join, backend destruction, and host-resource
  release. Workers may report completion but never free session/shared state.
  Windows Console/window and Linux Console/window variants need equivalent
  owner maps and failure-path verification.
- [ ] **Linux verification.** Retained Linux code is a portability asset, not
  validated support. Add native POSIX compile and runtime gates after the
  explicit-handle implementation is stable; preserve the Windows GCC baseline.
- [ ] **Resolve `core/platform/presentation.h`.** It declares an
  `nxvm_platform_input_event` despite living in `core/platform` and containing
  an input event rather than presentation behavior. Decide whether it is a
  generic `core_platform_input_event` in an `input_event.h` contract or a
  VM-only type relocated to `vm/platform`; remove an unused declaration rather
  than preserving an ambiguous abstraction.
- [ ] **Resolve orphan VM product models.** `vm/product/media.*` and
  `vm/product/presentation.*` are currently exercised by smoke tests but are
  not the retained NXVM Console's real policy path. Decide explicitly to wire
  them into the real product, reclassify them as test/design support, or remove
  them. Do not present smoke coverage as implemented user-facing policy.
- [ ] **Multi-session product design.** The state-isolation foundation is not
  a session manager or multi-window feature. Before implementation, design
  session creation/destruction, enumeration, debugger attachment, Console and
  window ownership, host-surface contention, cancellation, and cross-session
  failure semantics.

### VDM Boundary

- [ ] **Keep VDM minimal until its design milestone.** The present VDM code is
  an isolated non-runnable scaffold. It must not quietly acquire a DOS runner,
  host filesystem, CLI, platform UI, or implicit machine boot path before M6
  defines those contracts.
- [ ] **Owned DOS design before implementation.** Specify loader, PSP,
  environment, DTA, DOS interrupt subset, vector ownership, fixture filesystem,
  display/input providers, cancellation, and test corpus before M7 changes
  behavior.

## Compatibility And Hardware

- [ ] **CPU correctness / MS-DOS MEM.** Do not claim complete 80386 support:
  the CPU is 8086-plus with partial i386 decode/execution coverage. Reproduce
  the MS-DOS `MEM` invalid-opcode failure with instruction, wall-clock, and
  no-progress budgets; capture capability/profile, CS:IP, linear PC, bytes and
  prefixes, registers, FLAGS, interrupt/exception state, and a bounded first-
  failure trace. Classify, add a project-owned regression probe, repair, and
  retain the probe. Protected DOS media stays local.
- [ ] **Hardware compatibility corpus.** Audit and prioritize incomplete KBC,
  VADP, PIT read-back, HDC/FDC, DMA, PIC, CMOS, timing, and chipset behavior
  against focused owned probes. Preserve full-PC boot and Console regressions.
- [ ] **Eliminate anonymous source TODOs.** Existing CPU naming, I/O-map test,
  debugger assembler-test, and PIT read-back comments need a ledger item or a
  bounded task. Source comments must use the documented `TODO(High|Medium|Low)`
  form only and must not become the sole record of a compatibility gap.
- [ ] **Bounded differential debugging.** The historical Bochx/Bochs bridge
  may be an optional developer research tool with provenance, comparison
  schema, masks, instruction/time/no-progress/size budgets, and cleanup. It is
  never a default build, runtime dependency, substitute for owned probes, or
  source of copied third-party code without review.
- [ ] **Future machine-profile support.** Design external machine profiles and
  optional user-supplied ROM manifests before adding PC110/Compaq/Award/Phoenix
  behavior. Third-party ROMs are never committed, bundled, downloaded, or made
  a default dependency.

## CLI, Host Integration, And Product Semantics

- [ ] **VDM CLI contract.** Implement only after its design milestone:
  `ntvdm64 run [--display=auto|console|window] [--debug] [--drive <letter>]...`
  `[--hide-drive <letter>]... [<program> [args...]]`, with exact no-program,
  pause-before-first-instruction, cancellation, exit-status, and redirected-
  handle behavior.
- [ ] **Display policy.** `auto` begins in Console text mode and switches once
  to a supported guest-display window; Console mode clearly rejects guest
  graphics; window mode owns guest display/input; host logs remain separate.
- [ ] **Host-drive security.** Whitelisted `--drive` options override hides;
  otherwise hides apply to eligible drives. Canonicalize and reparse-check all
  operations; deny UNC/device namespaces and escape paths; hidden/unselected
  drives appear nonexistent.
- [ ] **Debugger UX.** Window owns guest display/input while Console owns the
  interactive CPU debugger. Support explicit state/stop reasons, registers,
  disassembly, memory, execution breakpoints, step, continue, trace, reset,
  and quit at a synchronized command boundary. No-program debug starts paused
  at the initial execution point.
- [ ] **Windows support matrix.** Target Windows 7 through Windows 11. Verify
  actual divergence during implementation; Windows 11 is the fallback priority
  if compatibility becomes materially different, with Windows 7 gaps recorded
  as bounded follow-ups.

## Verification, Build, And Governance

- [ ] **Keep build gates truthful.** CMake presets, target names, smoke gates,
  task versions, artifacts, and verification records must describe the current
  source graph. A stale preset or a passing fixture-only test is not evidence
  of product readiness.
- [ ] **Artifact and workspace hygiene.** Each runnable behavior-changing task
  produces one verified task-level executable under `build/output/`; remove
  stale owned build trees, logs, traces, and generated binaries after use.
  Never commit guest media, Microsoft binaries, local paths, raw traces, or
  unreviewed third-party material.
- [ ] **Architecture documentation.** Maintain concise product assembly and
  execution diagrams, a unique-state-owner map, module dependency rules, and
  a current task/evidence index. Reconcile status, roadmap, contracts, and
  verification documents before claiming a closure.
- [ ] **Completion audit.** Before closing structural work, scan every
  `core/{machine,platform,product}`, `vm/{machine,platform,product,profile}`,
  `vdm/{machine,platform,product,profile}`, composition root, CMake target,
  test, and entry point for duplicate state, parallel run paths, forwarding
  wrappers, global/TLS selectors, illegal dependencies, out-of-scope code, and
  misleading file names.

## Recurring Failure Patterns To Prevent

- [ ] Do not declare a refactor complete based on directory moves, CMake
  success, or unit smoke tests alone; require retained Console and real boot
  evidence where the runnable path is affected.
- [ ] Do not introduce a second executor, machine/session wrapper, cached
  mirror state, hidden reset sequence, or VM-side instruction loop while
  repairing an existing path.
- [ ] Do not solve host interaction with ambient globals or TLS; use explicit
  provider/context/lease objects with one creator and one destroyer.
- [ ] Do not let trace, recorder, or differential-debug output grow without
  byte, time, no-progress, process-cleanup, and deletion controls.
- [ ] Do not change retained NXVM Console, debugger, startup, or boot behavior
  during structural migration without explicit owner approval and regression
  evidence.

