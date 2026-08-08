# Project Status

## Current Work

**Idle - M5 T298 closed; T299 is the next linear task.**

T297 is the reference baseline. T298 replaces the retained NXVM debugger's
production raw CPU, instruction, execution-context, RAM, and port borrows
with copied observations and named core debug operations, without changing
Console commands, text, prompt, startup, step, breakpoint, trace, reset, or
quit behavior. T298 is closed after coordinator review: the formal debugger
path now uses named core operations and copied observations only.

### T298 Closure Evidence

- **Original request:** Remove the formal NXVM debugger path's raw core CPU,
  instruction metadata, execution-context, RAM, and port borrows. Preserve
  retained Console/debugger UX and startup flow through copied observations
  and the smallest operation-specific core debug capability.
- **S1 frozen production matrix:** `vm_debug_cpu` supplies register reads,
  register patches, and code-segment metadata; `vm_debug_execution` supplies
  segment validation/patching, linear reads/writes, and diagnostic printing;
  `vm_debug_memory` supplies real-memory reads/writes; `vm_debug_port`
  supplies port reads/writes; and `vm_debug_instructions` supplies the three
  watchpoint configurations. `vm_session_control_initialize` also supplies
  CPU/instruction borrows to the VM-owned breakpoint/trace recorder. The
  admitted replacements are copied CPU/segment/control/diagnostic observation;
  validated field-mask register/segment patch; checked physical/linear/real memory
  read/write; checked port read/write; named code-default-size/code-base and
  execution-point queries; and named watchpoint/breakpoint/trace operations.
  No current-object getter, whole-machine snapshot, executor pointer, mutable
  pointer, global, TLS target, or second execution route is admitted.
- **Owner and boundary contract:** core owns all guest CPU, instruction,
  execution, RAM, and port state and performs every admitted core operation.
  VM owns Console policy, the session-bound adapter, and existing breakpoint/
  trace UI instrumentation. The capability is usable only at a synchronized
  paused command boundary or `STOPPED`; `RUNNING`, unbound, expired/destroyed,
  and cross-session calls fail deterministically. A call returns copied data
  only; no operation retains a borrow. Register/segment patches validate before
  commit and fail atomically. Memory/port use their existing checked machine
  paths. Reset preserves defined VM breakpoint/trace semantics; destroy closes
  the target/capability; no callback re-entry or cross-thread mutation route is
  introduced.
- **Reference baseline:** T297, developer artifact revision `0.5.0297`, SHA-256
  `7ED04D9014F084154C250601C7BFBD186DF2DD4B5652F4D7B4E4CB9CCE327FA5`.
- **In scope:** S1 contract/matrix and inventory update; S2 core debug
  operations plus VM adapter/recorder migration; focused lifecycle, UX,
  isolation, and source-shape gates; retained product regressions, full GCC
  gates, and the `0.5.0298` developer artifact.
- **Non-goals:** T299 public raw-borrow deletion or test migration; any new
  debugger command, Console wording/prompt/startup policy, VM outer loop,
  ROM/profile/firmware capability, media/controller behavior, DOS/BIOS policy,
  core dependency on VM, global/TLS target, or alternate executor.
- **Applicable rules:** `core/machine` owns generic state, validation,
  lifecycle, and checked operations; `vm/composition` binds the session-scoped
  product adapter; `vm/machine` owns retained debug UX instrumentation. All
  cross-module data are copied or named operations. No source import,
  third-party material, guest media, Microsoft research, or license/provenance
  change is involved.
- **Similar-issue sweep:** defect class is a debugger production route that
  borrows or stores raw CPU, instruction, RAM, port, or executor state. Query:
  `rg -n "core_machine_debug_.*_borrow|core_machine_configuration_(cpu|cpu_instructions)_borrow|t_cpu \*|t_cpuins \*|t_ram \*|t_port \*|core_machine_cpu_execution_context \*" src/vm/composition/session src/vm/machine`.
  Every production hit is migrated in T298 or explicitly left for T299 only
  when it is non-debug configuration/test scope; the focused static gate names
  this distinction.
- **S3 evidence:** focused capability and retained UX corpus covers register,
  disassembly, memory, execution control, breakpoint/trace, out-of-window
  rejection, and two-session non-crossing; a source gate rejects all formal
  debugger raw borrows from `src/`; retained Console/debugger, FDD boot, DOS
  prompt, Windows setup probe, and full GCC gates pass. The rebuilt artifact
  is `build/output/nxvm_0_5_0298.exe` with recorded SHA-256.
- **Active evidence (not closure):** GCC build of `vm-0-5-0298`, focused
  debugger smoke targets, and `verify-debugger-capability` passed; the static
  marker was `M5:T298:S3:DEBUGGER-CAPABILITY-STATIC:OK`. The focused core
  probe verifies copied observation outside/inside the eligible lifecycle,
  a multi-register field-mask commit, and rejected invalid-mask patch without
  partial CPU mutation. With the untracked
  owner-provided FDD/HDD paths configured only in this worktree's CMake cache,
  `run-current-smokes` passed all 126/126 cases, including DOS prompt, timer,
  FDD boot, debugger pause/unified backend, and HDD/DOS coverage. The complete
  current GCC gate set, documentation governance, and `git diff --check` also
  passed. The produced active artifact is
  `build/output/nxvm_0_5_0298.exe`, 2,701,319 bytes, SHA-256
  `6A9AA9D2C3691F780426C6A78C2AE1C149BB9CC2A84AE835A9AEC10B5313254B`.
- **Stop conditions:** stop and request direction for any unlisted capability,
  command/text/prompt/startup change, global/TLS/current target, core-to-VM
  dependency, discovered non-debug production borrow, or requirement to start
  T299+.

T297 replaced the default-profile firmware's long-lived raw core binding with
one core-invoked opaque capability. Its runtime whitelist is checked physical
memory and port I/O plus a stop request; no copied CPU-state API was admitted
because no consumer justified it. Configure-time ROM registration is copied
and atomic: a failed callback rolls back only its later ROM routes and owned
copies, retaining earlier mappings and permitting a valid rebind. The callback
borrow ends synchronously; freeze is immutable, reset retains the provider,
and destroy invalidates its context. Default BIOS/QDCGA and boot-failure
handling now use that boundary, with no Console or startup behavior change.

T297 verification and developer-artifact evidence are retained in M5 history;
T298's active packet below is the sole current artifact authority.

T298 is active. It replaces debugger raw borrows with copied or
operation-specific core debug capability while preserving the NXVM Console,
debugger commands, prompt, and startup behavior.

## Current Technical Baseline

- **T298 active artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0298` / `nxvm_0_5_0298.exe`.
  Its local developer artifact awaits the active task's focused and full-gate
  evidence.
- **T285 display implementation:** `INT 10h` mode `10h` /
  `EGA-640x350x16-direct` has a VADP-owned planar frame path and copied-frame
  consumer boundary; mode 0Dh remains a separate retained path.
- **Core boundary:** T243--T246 retain checked physical memory, immutable ROM
  mapping, and validated real-mode entry plans. The obsolete post-`#UD`
  transition has no public or runtime path.
- **Product boundary:** `nxvm.exe` is the retained runnable product. `mantle`,
  `dos`, and `nxvdm.exe` remain future architecture commitments; they are not
  current runtime or release products.

## Recent M5 Closures

| Task | Compact result |
| --- | --- |
| T291 | Made FDC generation, disk-change and motor/DMA cancellation per frozen drive slot, with core port and DOS FDD0 evidence while leaving timing, UI, and commands deferred. |
| T292 | Added prepared-state evidence that selected 67h REP comparison, scan, and segment-override forms already conform; no CPU behavior or artifact changed. |
| T293 | Removed the unused post-`#UD` transition surface, made protected outer `RETF`/`IRET` frame validation non-mutating until commit, and completed its exception/atomicity matrix: non-present returned `CS` is `#NP`, non-present `SS` is the retained terminal `#SS`, and invalid entries are `#GP`; all six `RETF`/outer-`IRET` cases preserve the defined pre-commit boundary. |
| T294 | Completed the codebase-driven public-surface/raw-borrow matrix, fixed T296's three stages and T297's future firmware-capability lifecycle precondition, and left T300/T302 conditional; no runtime behavior, CMake graph, or artifact changed. |
| T295 | Moved CPU execution-to-shared-PIC binding into `core_machine_create`; VM session no longer borrows or binds that path. CPU/PIC lifecycle evidence and all current gates passed; RTC/FDC/HDC/DMA remain T296 and debugger raw borrows remain T298/T299. |
| T296 | Completed A VADP/ports, B DMA/RTC/CMOS/NMI, and C FDC/HDC ownership migration through frozen typed core submissions. S2 `e84199e`, S3 `a02a0f0`, and S4 `fa18847d0aed685554f786c89ba0f5908e539fb7` passed focused owner/lifecycle evidence and the final 49-gate, 125-test verification; T298--T299 remained deferred at this closure. |
| T297 | Replaced default firmware raw profile binding with an opaque core-invoked capability for checked memory/port I/O and stop requests. Configure-time immutable ROM registration now rolls back atomically on callback failure; default BIOS/QDCGA, boot failure, 49 gates, and 126/126 CTests are closed without product UX change. |
| T298 | Replaced formal debugger raw CPU/instruction/execution/RAM/port borrows with paused-or-stopped named core operations and copied observations. The retained Console/debugger UX passed focused checks and the 126-test regression suite without a global target or second execution path. |

Detailed contracts, commands, artifact provenance, and prior closures are in
[M5 History](../history/m5.md) and Git history. The [M5 convergence queue]
(m5-pcat-hardware-convergence.md), [roadmap](roadmap.md), and
[M5 closure checklist](m5-closure-checklist.md) define current M5 scope; an
active packet becomes an operational authority only after approval.

## Recent Governance

- **M5 Td S33:** corrected the external-consumer boundary and shifted the
  former forward queue to T279--T287.
- **M5 Td S34:** repaired queue wording and inserted the approved T279--T283
  reliability/interface package.
- **M5 Td S35:** tightened T279, T280, T282, and T283 task contracts.
- **M5 Td S36:** recorded the WASM media-backing admission boundary.
- **M5 Td S37:** made M5 history static, capped closure summaries, and
  hardened the mojibake governance check.
- **M5 Td S40:** inserted the approved T293--T303 core public-surface closure
  and shifted remaining Windows CPU/FPU/readiness/closure work to T304--T308.
- **M5 Td S41:** retired completed planning detail, made TODO open-only, removed
  tracked local-path samples, and added recurrence gates for those boundaries.
- **M5 Td S43:** superseded the unimplemented T291 S4 admission by merging its
  protected-return fault-atomicity scope into active T293 S1, which retains its
  original post-`#UD` transition-removal scope. T291 remains closed FDC history;
  the unstarted continuation remains T294--T308.

## Milestone State

| Milestone | State | Current authority |
| --- | --- | --- |
| M0--M4 | Closed | [Roadmap](roadmap.md) and [history](../history/README.md) |
| M5 | Open | [M5 closure checklist](m5-closure-checklist.md) |
| M6--M8 | Not started | [Roadmap](roadmap.md) |
| M9 onward | Research queue | [Roadmap](roadmap.md) |

## Operational Reading Order

1. [Documentation Guide](../README.md)
2. [Project Status](status.md) and [M5 convergence queue](m5-pcat-hardware-convergence.md)
3. [M5 closure checklist](m5-closure-checklist.md)
4. [Architecture overview](../architecture/overview.md) and
   [contracts](../architecture/contracts.md)
5. [Requirements](../requirements/)
