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

## M5 NXVM PC/AT Workstream

M5 remains the NXVM and shared-core workstream. The future VDM product does not
define NXVM completion, and M6 must not begin merely because an NXVM device
task is pending. This is an ROI-ordered queue, not an activated task breakdown:
numeric `T` identifiers are assigned only when implementation is admitted.

Every admitted device task follows the three-subtask
[hardware-device verification template](docs/compatibility/hardware-device-verification.md):
contract and port-level probe; implementation at the real
CPU/bus/device/firmware/profile owner; and DOS or system-image regression.
Bochs 2.6 may supply bounded differential evidence, but never implementation
code, a runtime dependency, or substitute acceptance evidence. No task may
rely on a BIOS hack, host shortcut, or application special case for a
guest-observable behavior.

**Active implementation task -- T213: HDD controller profile / port contract.**
S1 declares and probes the primary ATA PIO contract; S2 admits the declared
controller state machine and composition binding; S3 retires the HDD F4/F5
shortcut and records its real system-image regression. See
[`M5 T213`](docs/planning/m5-t213-hdd-controller-profile.md).

1. **Architecture and validation foundation.** Reaffirm NXVM-first M5 scope;
   make the default PC/AT profile declarative for topology, ports, IRQ, DMA,
   CMOS defaults, ROM mapping, and firmware hooks; audit that every observable
   behavior has a CPU, bus, device, firmware, or profile owner; establish the
   reusable port-probe, DOS-regression, and bounded-differential template.
   Keep CPU work to a real-mode 8086/80186 trust baseline needed by these
   devices; do not block this stream on full 286/386 or x87 work.
2. **Highest-ROI real-mode devices.** Complete PIC IRQ lifecycle, PIT
   waveform/GATE/count-zero/BCD behavior, and advanced KBC keyboard protocol
   before broadening device scope. Then admit CGA `320x200x4`, AUX mouse/IRQ12,
   DMA channel/page/mode behavior, FDC/FDD command/result behavior, and
   CMOS/RTC interrupts and deterministic-time policy.
3. **Controllers and display expansion.** Design one explicit HDD controller
   profile, then implement a minimum real port/IRQ/PIO controller over a file
   image backend. Expand EGA/VGA in bounded register-family increments:
   mapping, sequencer, graphics controller, attribute controller, DAC, then
   planar/latch/raster behavior. BIOS INT 13h or host-backed storage cannot
   stand in for controller compatibility.
4. **Long-horizon CPU/FPU track.** After real-mode device work has an adequate
   baseline, admit 286 descriptors/exceptions/protected transfers; 386 paging,
   CPL/IOPL, TSS I/O map, and task switching; then x87 state, operations,
   exceptions, and `FWAIT`. The MS-DOS `MEM`/`FNINIT` workaround remains a
   regression sample, not a 80386 or FPU correctness claim.
5. **Compatibility tiers.** First target stable DOS image boot and text
   software; then interactive PC/AT DOS including graphics/mouse/storage;
   then EGA/VGA software. Win3.x and especially Win95 remain later corpus
   goals gated by their required CPU, display, controller, and interrupt
   evidence, not by a calendar milestone.

## Current Architecture Closure

- [x] **Linux Console private naming cleanup (`TODO(Low)`, T207).**
  Replace local-dialect helper names and macros with typed
  `vm_platform_linuxcon_*` private functions without changing input or display
  behavior.

- [x] **Current-gate graph de-duplication (`TODO(Low)`, T206).**
  Remove redundant preset smoke targets and obsolete custom targets that only
  execute CTest-registered smoke already covered by `run-current-smokes`.

- [x] **Console adapter cohesion closure (`TODO(Low)`, T205).**
  Keep the typed selected-session borrow as an adapter-private helper, delete
  its one-consumer source/header split, and make the closure gate validate the
  unique borrow fact rather than a helper file.

- [x] **CMake media-smoke and Linux adapter hygiene (`TODO(Low)`, T204).**
  Derive media-smoke registration and skipping from one FDD/FDD+HDD list, then
  remove uncalled platform sleep facades and unreachable Linux adapter noise
  without changing smoke coverage or platform behavior.

- [x] **Console adapter readability and selected borrow closure (`TODO(Medium)`, T203).**
  T203 retains the Console machine-provider vtable, expands its callbacks into
  ordinary functions, and makes an adapter-local helper the sole composition
  call to the core selected-borrow API. Failed-borrow and debugger
  pause/wait/context behavior are unchanged; a source gate prevents duplicate
  direct borrowing and obsolete selected-session wrappers.

- [x] **VM keyboard transport surface closure (`TODO(Medium)`, T202).**
  T202 removed the test-only platform modifier enum/query callback and session
  forwarding switch. Core keyboard-provider modifier observation is retained;
  platform transport now carries only discrete guest keypress ingress and its
  smoke. The T202 source gate prevents the façade from returning.

- [x] **Host cancellation boundary convergence (`TODO(High)`, T201).**
  Windows F9 formerly reached session stop through keyboard transport, while
  Linux reported `STOP_REQUESTED` to the live run handle. T201 removed keyboard
  lifecycle callbacks: every host cancellation producer now reports only
  through its backend's borrowed run handle and the runner consumes the report. Retain
  existing guest F9 key forwarding, Console/debugger, window/Console,
  two-session, and FDD DOS-prompt behavior. Linux native runtime evidence
  remains deferred; source/CMake parity is verified here. The T201 source gate
  rejects a return of keyboard lifecycle callbacks or direct platform
  session/core stop calls.

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

- [x] **Current artifact target truthfulness.** T197 removes every historical
  task/version executable target from CMake. Only the current source artifact
  is buildable; earlier `build/output/` executables remain historical evidence
  and must never be regenerated from current source under their old names.
- [x] **Current smoke execution truthfulness.** T198 registers the configured
  runnable set with labeled CTest cases and makes `current-gates-gcc` build and
  execute all 40 cases. Source/inventory checks remain explicitly static gates;
  compiling a smoke executable alone is no longer runtime evidence.

- [x] **VM input and transport boundary convergence (`TODO(High)`, T199--T200).**
  `vm_session` owns both the host-input boundary and its request transport.
  Preserve that ownership while removing a historical outbound queue and
  converging the remaining direct host-state update:
  - [x] **T199, request-transport egress removal.** Deleted the unconsumed egress
    storage, API, and test-only coverage. Correct the historical T194 wording:
    worker lifecycle/cancellation reports use the session-owned run handle,
    whose runner consumer is the only cancellation/join/finalization path.
    Keep ingress unchanged. Add a source gate proving no egress API remains;
    retain run-handle, Console, debugger, two-session, and FDD DOS-prompt
    regressions. Stop for any second cancellation route or user-visible change.
  - [x] **T200, host keyboard-state ingress.** Modifier/toggle host-state
    snapshots through `session.request_transport.ingress`, consuming them only
    at the runner command boundary like discrete key presses. Define bounded
    coalescing for replaceable snapshots so host polling cannot fill the
    command queue. Add a focused proof that platform input cannot mutate the
    keyboard provider before that boundary, then retain KBC, BIOS/DOS `ver`,
    Console/debugger, window/Console, two-session, and FDD DOS-prompt gates.
     The direct VM platform callback is gone; adjacent snapshots coalesce while
     a key press remains an ordering barrier. Stop for key-up/focus,
     modifier/toggle, timing, or retained UX regression.

- [x] **Finish the explicit platform run-handle boundary.** M5 T194--T196
  converged a worker-report-only event path, session-owned cancellation/join/
  teardown, Win32 failure-path verification, and Linux source/CMake parity.
  The session-owned handle remains the sole owner of worker creation, join,
  backend destruction, and host-resource release; workers never free shared
  state or directly control the guest. Native POSIX runtime proof remains the
  separate deferred item below. See
  [`m5-platform-run-handle-contract.md`](docs/planning/m5-platform-run-handle-contract.md).
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

- [x] **Firmware interrupt-portal migration (`TODO(High)`, T209).** Replace
  default-profile QDX `F1 <command>` with frozen, ROM-origin-checked private
  firmware `INT F0h`--`F5h` providers. Core must retain normal `INT`/IVT
  semantics for every nonmatching call; profile handlers must use existing
  firmware/device bindings rather than raw decoder-table mutation or an opaque
  current-profile extension. Preserve the default `80386 + no FPU` baseline
  and add explicit `8086 + no FPU` instruction-profile proof before removing
  QDX. The portal stage passes under 8086. The owner-local full FDD vector
  reaches runtime `C1 EA 04` (`SHR DX, 4`) at linear `0000:AA98`; this is a
  real 80186+ immediate-shift form, so strict 8086 correctly raises `#UD`.
  Keep the focused 8086-rejects/80186-accepts probe and retain the FDD run only
  as an expected-negative compatibility diagnostic; it is not a T209 blocker.

- [x] **Keyboard firmware-portal retirement (`TODO(High)`, T210).** Default
  profile keyboard input now follows core KBC `60h/64h` and IRQ1 through ROM
  `INT 09h`, BDA buffering, and ROM `INT 16h`; F1/F3 portal registration and
  C callback handlers are removed. The T210 closure gate, KBC port probe, DOS
  `ver` regression, Console/debugger, and two-session matrix pass. Rapid
  typeahead and the broader advanced-KBC protocol remain deferred below.
- [ ] **DOS keyboard regression timing stability (`TODO(Low)`).** One T213 S1
  current-matrix run timed out in `vm-dos-keyboard-smoke`, while its immediate
  direct rerun and the following complete 49-test matrix passed unchanged.
  Preserve the existing guest behavior; only investigate after a reproducible
  failure, with a bounded readiness/host-input timing probe rather than an
  arbitrary timeout increase.

- [x] **Boot-failure firmware-portal retirement (`TODO(High)`, T211).** The
  default ROM no longer calls `INT F0h`; after its existing key acknowledgement
  it writes the reset-cleared profile BDA POST report. The session runner clears
  and consumes that report at an instruction-budget boundary, then owns the
  sole stop request. No core policy, host callback, queue, fake port, or
  host-side BDA mutation was introduced. The current matrix passes; completed
  T212 ROM-video regression now covers the no-media error-display sequence.

- [x] **Retire default-profile firmware shortcuts (`TODO(High)`, T209--T213).**
  T209 removes the QDX `F1 <command>` CPU opcode hack; T210 retires F1/F3,
  T211 retires F0, T212 retires F2, and T213 retires F4/F5. The resulting
  default profile has no private `F0h`--`F5h` registration: keyboard, stop,
  text display, and CHS disk I/O each have their declared device/ROM/session
  owner. `firmware_portal.*` and obsolete QD disk helpers are gone; ordinary
  guest `INT`/IVT delivery and NXVM Console/debugger/boot behavior remain.

- [x] **Runner display cadence (`TODO(Medium)`, T212).** Normal execution now
  uses a 256-instruction quantum and publishes at most one copied display frame
  per quantum; command, pause, stop, and debug refresh remain at the same
  bounded boundary, while single-step remains one instruction. The no-media
  mailbox smoke rejects a return to per-instruction full-frame copying.

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

- [ ] **Preserve CPU-fault evidence at the product boundary (`TODO(Medium)`).**
  `core_machine` already retains its bounded first-fault snapshot until a cold
  reset; the problem is not that diagnostic evidence is inherently discarded.
  A real-mode CPU exception still reaches the VM runner as generic
  `TYPE_STATUS_FAULT`, which is handled as a generic stop rather than a stable,
  session/debugger-consumable machine outcome. Define that distinct fault
  outcome, preserve the diagnostic window at the returned boundary, and make
  the session runner stop on it without initiating a reset. Keep ordinary
  profile/provider stop and user cancellation semantics separate; do not add a
  global trace or a second execution path.
- [x] **8042 KBC controller and one BIOS route (`TODO(Medium)`).** T192
  completed `0x60`/`0x64`, OBF/IBF, command byte,
  `0x20`/`0x60`/`0xAA`/`0xAB`/`0xAD`/`0xAE`/`0xD0`/`0xD1`, keyboard
  ACK/reset/enable/disable/identify, IRQ1, A20/reset, and a full-FIFO
  `NTVDM64_STATUS_INVALID_STATE`. Platform input is consumed at the execution
  boundary, mapped by the default profile, delivered through KBC, and consumed
  by default-ROM INT 09h before INT 16h reads BDA. See
  the hardware-device verification template.
- [ ] **8042 advanced keyboard protocol (`TODO(Medium)`).** Admit each of
  set-1 break bytes and E0/E1 extended sequences, rapid DOS typeahead queue
  delivery, scan-code-set selection,
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
- [ ] **PC/AT hardware completeness baseline.** The current machine is a
  bootable, deliberately bounded PC/AT subset, not a Bochs-equivalent machine.
  Bochs 2.6 may be used only as behavior/state-decomposition reference; its
  source, plugin architecture, and broader device set are not an implementation
  target or import source. Preserve full-PC boot, Console, debugger, and DOS
  prompt regressions for every device task.

  - **CPU/FPU:** CPU profiles gate 8086, 80186, 80286, and partial 80386
    execution. Do not claim complete 80386 compatibility until protected-mode
    control transfer, task-switch, descriptor, exception, paging, and I/O-map
    behavior have owned instruction probes. `FPU=none` consumes legal ESC
    encodings; present FPU profiles remain unavailable until state, operations,
    exceptions, and `FWAIT` are implemented. The historical MS-DOS `MEM`
    `FNINIT` `#UD` is bypassed, but `MEM` is not a complete CPU-correctness
    acceptance claim.
  - **Memory and I/O bus:** RAM has installed-range faults, A20, frozen mapping
    registration, and stopped-only cold reconfiguration. Establish an explicit
    PC memory-map admission before adding ROM/UMA/MMIO mappings. The port bus
    has provider dispatch but no broad device-width, unmapped-I/O, or bus-timing
    compatibility claim.
  - **PIC/DMA:** dual 8259-style PIC and dual 8237-style DMA support the
    current IRQ/DMA boot path, including FDC DMA channel 2. Audit IRQ
    deassertion, edge/level behavior, priority/EOI modes, DMA channel modes,
    masking, page/register edge cases, and memory-to-memory behavior with
    focused port vectors before relying on them for new hardware.
  - **PIT:** 8254 counter ports and read-back/status latches are covered.
    Waveforms, GATE edges, BCD, count-zero, and modes 1/4/5 remain explicitly
    unimplemented compatibility work.
  - **KBC:** the bounded 8042 path covers `60h`/`64h`, FIFO, command byte,
    IRQ1, A20/reset, and fixed set-1 make-code delivery to BIOS. Break bytes,
    E0/E1 sequences, scan-set selection, translation, LED/typematic, AUX
    mouse/IRQ12, timing, resend, and error behavior require separate probes.
  - **Display:** VADP owns CGA-style text `B8000`, bounded CRTC state, text
    ports, dirty generation, and copied text snapshots. CGA `320x200x4`, then
    EGA/VGA register families, palettes, raster timing, planar VRAM/latches,
    and VBE are separate admissions; graphics is currently unsupported.
  - **CMOS/RTC:** VM CMOS currently supplies indexed `70h`/`71h` access,
    checksum, BCD host-time refresh, and NMI-mask state. It lacks persistent
    NVRAM, RTC periodic/update/alarm interrupts, full register semantics, and
    deterministic time policy.
  - **FDC/FDD:** the 8272A-style controller, DMA2/IRQ6 route, and in-memory
    floppy images are sufficient for recorded boot. Validate command/result
    state, disk-change/motor/data-rate behavior, non-DMA, media formats,
    transfer timing, and error paths before broad floppy compatibility claims.
  - **HDD:** T213 provides a bounded primary, master-only ATA PIO controller
    (`1F0h`--`1F7h`, `3F6h`, IRQ14, CHS, no DMA) with a session-owned image
    backend and ROM `INT 13h` port transactions. Secondary/slave channels,
    LBA, bus-master DMA, ATAPI, timing, write cache, and host-path policy
    remain deferred; do not call this full IDE compatibility.
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
  retaining the FDD/HDD session and FDD DOS-prompt baselines. Future device
  work follows the hardware-device verification template.
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

The following open requirements use the [M5 closure checklist](docs/planning/m5-closure-checklist.md)
as their single operational evidence index. `RULES.md` and the execution policy
remain the rule authorities.

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
