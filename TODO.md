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

- [x] **Core-owned generic-device lifecycle.** T141 moved CPU/bus/memory and
  shared VADP/KBC/DMA/PIT/PIC prepare/reset/refresh/finalize order into
  `core_machine`. T159 re-audited the current tree, removed misleading
  composition markers, and added a source gate that rejects any VM/VDM direct
  generic core-device lifecycle call. Composition only binds VM/VDM-only
  devices, firmware, and providers.
- [x] **Public lifecycle hardening.** T160 closes the configuration window
  explicitly: bind execution/port providers only while `INITIALIZED`, freeze
  exactly once, then reset/run the frozen topology. Public memory, port, and
  A20 access require a returned `PAUSED` boundary. The contract smoke rejects
  reset-before-freeze and topology mutation after freeze.
- [x] **Composition naming and separation.** T161 splits the former mixed
  provider source into VM-only device lifecycle, default-profile firmware
  lifecycle, a thin order-only coordinator, and a separate machine-information
  adapter. A source-shape gate rejects a return to the mixed file.
- [x] **VM session same-object alias matrix (`TODO(High)`).**
  `vm_session` embeds VM-only device, provider, platform, and debugger storage
  directly; T176--T184 removed the former same-object pointer aliases and
  retained only the opaque `core_machine` plus the explicitly lazy debug target.
  The direct-owner closure gate preserves session lifecycle order, multi-session
  isolation, retained Console/debugger behavior, and FDD DOS-prompt evidence.
- [x] **Frozen-core API bypass (`TODO(High)`).** The public mutable
  `core_machine_executor_*_borrow` surface can bypass the T160 configuration
  boundary. The retained `DEVICE ram` Console command reaches
  `core_machine_memory_allocate_for` through this route. Define a replacement
  configuration/reconstruction contract before changing it; also constrain
  direct CPU/diagnostic observation to a copied, synchronized boundary. T164
  closed the generic borrow and observation portions. T165 remains to route
  `DEVICE ram` through a temporary stopped-session reconstruction; T166 removed
  machine-local default media paths. T169 supersedes the reconstruction model
  with a core-owned stopped-only cold RAM reconfiguration.
- [x] **Core-owned RAM cold reconfiguration (`TODO(High)`).** T169--T174
  replaced T165's temporary whole-session reconstruction with stopped-only
  `core_machine_reconfigure_memory`: installed-range checking, checked
  provider access, one retained machine/session/device/provider/media/platform/
  debugger graph, and a cold guest reset. CPU/FPU/profile/topology changes
  still require a new session.
- [x] **Default-session media policy (`TODO(High)`).** The NXVM session
  factory must not commit machine-local image paths or silently select host
  media. Preserve explicit `DEVICE` media commands and define the approved
  no-media/default-fixture policy before removing the current paths.
- [x] **VM Console composition adapter (`TODO(Medium)`).**
  The former mixed provider combines session-factory callbacks and
  selected-session Console command adapters. Split it into accurately named
  source owners without changing Console grammar or selected-session semantics;
  remove the empty Console-provider initialize/finalize vtable callbacks.
- [x] **Legacy wait forwarding alias (`TODO(Low)`).** T186 migrated callers
  and tests to `core_product_wait_milliseconds()`, deleted the policy-free
  `core_product_utils_sleep()` alias, and retained explicit caller-owned wait
  scopes with their isolation smoke.
- [x] **Contract and vocabulary drift (`TODO(Medium)`).** Reconcile
  `docs/architecture/contracts.md` with actual lowercase C API names and
  current profile enums. Add the missing `STD_MEMMOVE` facade and remove the
  direct VDM `memmove` use in a bounded vocabulary task.
- [x] **CMake core target clarity (`TODO(Low)`).** `nxvm-core` and
  `nxvm-core-machine` together form one runtime core, not two guest executors;
  make their target names/dependency roles unambiguous in a build-only task.

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
- [x] **Shared product-session implementation.** `core/product/session` owns
  the opaque registry and shared command grammar; NXVM implements it in
  T145--T150, while VDM adopts it only after M6/M8 define its concrete session
  and debug Console contracts. The approved design is
  `docs/planning/m5-product-session-management.md`. Do not start an
  implementation task without explicit activation. NXVM completed T145--T150;
  VDM adoption remains deferred to its separate design/implementation work.

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
  the CPU is 8086-plus with partial i386 decode/execution coverage. T152
  historically reproduced `MEM` as FPU `FNINIT` (`DB E3`) reaching the old
  direct `#UD` route and added a fixed 32-entry in-memory first-fault capture
  plus an owner-local fixture smoke. T153--T156 defined frozen
  `8086`/`80186`/`80286`/`80386` CPU and independent
  `none`/`8087`/`80287`/`80387` FPU profiles, centralized CPU gating, and made
  default `FPU=none` consume legal ESC encodings. T157--T158 add
  session-creation UX and closure probes; present FPU profiles remain
  unavailable until their state/operations are implemented.
  Protected DOS media stays local.
- [x] **8042 KBC controller and one BIOS route (`TODO(Medium)`).** T192
  completed `0x60`/`0x64`, OBF/IBF, command byte,
  `0x20`/`0x60`/`0xAA`/`0xAB`/`0xAD`/`0xAE`/`0xD0`/`0xD1`, keyboard
  ACK/reset/enable/disable/identify, IRQ1, A20/reset, and a full-FIFO
  `NTVDM64_STATUS_INVALID_STATE`. Platform input is consumed at the execution
  boundary, mapped by the default profile, delivered through KBC, and consumed
  by QDKEYB INT 09h before INT 16h reads BDA. See
  [`m5-pc-compatible-device-plan.md`](docs/planning/m5-pc-compatible-device-plan.md).
- [ ] **8042 advanced keyboard protocol (`TODO(Medium)`).** Admit each of
  set-1 break bytes and E0/E1 extended sequences, scan-code-set selection,
  controller translation, LED/typematic commands, AUX mouse/IRQ12, controller
  timing, and error/resend edge cases only with specific port and DOS probes.
  Do not add a second host queue or place host policy in `core/machine`.
- [ ] **POSIX KBC input runtime verification (`TODO(Medium)`).** The Linux
  curses path now submits normalized host events and has Windows-side static
  source parity, but native POSIX compilation and keyboard runtime behavior
  (especially modifiers and extended keys) need an owned POSIX-environment
  probe before claiming support.
- [x] **CGA text-controller slice (`TODO(Medium)`).** T193 moved CRTC
  `0x3d4`/`0x3d5` text indexes `0x0a`--`0x0f`, `0x3d8` text mode, `0x3d9`
  color, stable `0x3da`, B8000 visible-window capture, dirty generation, and
  copied text scanout into core VADP. QDCGA remains INT 10h/BDA firmware; no
  profile VADP alias or platform guest-memory access remains.
- [ ] **CGA `320x200x4` graphics admission (`TODO(Medium)`).** T193 S3
  deliberately defers graphics: define B8000 graphics addressing and scanline
  layout, exact `0x3d8`/`0x3d9` mode/color/palette behavior, a copied pixel
  scanout payload, port/memory/frame probes, and one owned DOS graphics
  fixture before implementation. The current text slice keeps graphics-select
  writes inert and text configuration rejects graphics modes.
- [ ] **EGA/VGA register-family admission (`TODO(Medium)`).** Do not merge it
  with CGA graphics. Admit mapping windows, sequencer, graphics/attribute
  controller, DAC, and planar VRAM/latches as separate bounded work with real
  program probes.
- [ ] **Hardware compatibility corpus.** Audit unproven PIC, DMA, CMOS,
  FDC/FDD, HDC/HDD, timing, and chipset behavior with focused owned probes.
  Promote a concrete incompatibility to its own ledger item only after a
  reproducible port/device vector exists. Preserve full-PC boot and Console
  regressions.
- [ ] **CPU internal naming (`TODO(Low)`).** Rename legacy segment-descriptor
  fields to owner-consistent vocabulary only in a bounded compatibility task;
  preserve layout, CPU behavior, and debugger output.
- [ ] **Present x87 model (`TODO(Medium)`).** `FPU=none` handles legal escape
  encodings, but configured `8087`/`80287`/`80387` profiles remain explicitly
  unavailable. Define the supported FPU state, instructions, exceptions,
  `FWAIT`, and probe corpus before accepting a present-FPU session; do not
  claim complete 80386-era floating-point compatibility first.
- [ ] **Protected-mode I/O permission map (`TODO(High)`).** Implement and test
  the TSS I/O-map permission check used when protected-mode CPL/IOPL rules
  require it. Establish owned probes before changing CPU execution.
- [ ] **Debugger assembler `checkop` review (`TODO(Medium)`).** The retained
  VM debug path can disassemble a guest instruction, reassemble that text, and
  compare the result as a diagnostic (`checkop`). Keep it out of the per-
  instruction execution-refresh path and do not create a broad assembler test
  target now; its corpus and cost are disproportionate to current M5/M6 work.
  Revisit only when an owned BIOS/debugger assembler defect needs bounded
  reproduction, then define the smallest focused probe and expected result.
- [x] **8254 PIT read-back command (`TODO(Medium)`).** T191 S2 implements
  active-low counter selection, non-overwriting count/status latches,
  status-before-count reads, RW byte order, and focused port probes while
  retaining the FDD/HDD session and FDD DOS-prompt baselines. See
  [`m5-pc-compatible-device-plan.md`](docs/planning/m5-pc-compatible-device-plan.md).
- [ ] **8254 PIT waveform and gate semantics (`TODO(Medium)`).** T191 only
  records the existing model's observable OUT state for status read-back. Before
  claiming broader 8254 compatibility, define and probe per-mode OUT pulses,
  GATE edges, mode 1/4/5 trigger behavior, BCD edge cases, and count-zero
  semantics without replacing the retained timer model wholesale.
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
