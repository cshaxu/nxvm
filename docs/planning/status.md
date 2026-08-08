# Project Status

## Current Work

**M5 T296 S4 active - core-owned FDC/HDC controller authority.**

S1 is committed as `56433a9`, S2 as `e84199e`, and S3 as `a02a0f0`; their
evidence remains recorded in [the T296 migration
matrix](../architecture/core-machine-shared-device-migration.md). S4 alone
moves FDC/HDC controller authority into `core_machine`. The task remains
active for coordinator review. Do not admit T297 or later work.

### Task Packet

- **Original request:** Move remaining shared-device initialization, binding,
  reset, and finalization authority to `core_machine` in three fixed stages.
  VM composition/profile may submit only frozen typed topology, configuration,
  and provider/media policy; it must not borrow or bind controller, PIC, or
  port storage.
- **S4 objective and completion condition:** VM composition submits frozen,
  neutral FDC and ATA topology while `INITIALIZED`: declared controller ports,
  IRQ/DMA wiring, opaque DMA request binding, drive-slot IDs, and the existing
  typed media-registry provider route. Core validates and copies topology,
  connects and initializes embedded controllers, registers their ports, and is
  their sole reset/refresh/finalize owner. VM keeps FDD/HDD backing and path
  policy. Completion requires a full-composition S4 owner gate, focused
  lifecycle smoke, the retained/full GCC gates, and the final `0296` artifact;
  this packet stays active pending coordinator review.
- **Reference baseline:** T295 historical behavior and its recorded artifact
  provenance remain in Git history. S4 advances only the current developer
  artifact identity; it does not alter product behavior or import media.
- **In scope:** `core_machine` FDC/HDC typed topology configuration, embedded
  controller connection/initialization/port registration/lifecycle; VM profile
  declaration and existing typed media-registry provider policy; an S4 static
  owner gate and lifecycle smoke.
- **Non-goals:** FDD/HDD backing or local-path policy; media registry routes;
  scheduler or second machine; storage mirror; host shortcut; and
  Console/debugger/boot experience change. T297 firmware capability, T298
  debugger capability, and the remaining T299 raw-borrow deletion remain
  deferred.
- **Applicable rules:** `core/machine` owns neutral mutable guest state and
  lifecycle order; VM/profile owns immutable PC/AT topology, defaults, ROM and
  provider/media policy. Core receives no PC/AT/default-profile/ROM-vendor,
  BIOS/DOS, local-path, or product-policy meaning. Preserve the single media
  route and owner-provided read-only test media rule. No source import or
  license/provenance action is involved.
- **Implementation and call chain:** `vm_session_storage_initialize` creates
  the existing VM-owned media registry, translates the immutable profile into
  neutral FDC/ATA topology, and submits it through core configuration before
  profile binding. Core copies the topology, consumes only the typed registry
  and opaque DMA request handle, and connects/initializes/registers embedded
  FDC/HDC using its own PIC and port storage. VM later binds and freezes the
  unchanged registry around VM-owned FDD/HDD objects; it no longer borrows or
  directly drives either controller. Core cold reset, scheduler refresh, and
  destruction retain the sole controller lifecycle order.
- **Similar-issue sweep:** The defect class is a VM composition source directly
  borrowing FDC/HDC/PIC/port storage or connecting, initializing, registering,
  resetting, refreshing, or finalizing controller state. Query:
  `rg -n "core_machine_(configuration_(fdc|hdc|shared_pic|port)_borrow|fdc_(connect|initialize|reset|refresh|finalize)|hdc_(connect|initialize|reset|refresh|finalize|port_provider)|install_port_provider)" src/vm/composition --glob '*.[ch]'`.
  Every production hit must become core topology submission or be an explicit
  T297+ exclusion; no FDD/HDD/path or firmware/debugger source may be changed.
- **S4 evidence commands and expected result:** focused
  `core-machine-controller-authority-smoke` plus
  `verify-core-controller-authority`; retained FDC/ATA/no-media/boot/DOS,
  Console/debugger, and two-session checks; then the managed absolute CMake
  `current-gates-gcc` command. Preserve only the resulting
  `build/output/nxvm_0_5_0296.exe`, record source commit and SHA-256, and run
  `powershell -NoProfile -ExecutionPolicy Bypass -File tools/Verify-DocumentationGovernance.ps1 -RepositoryRoot .` and `git diff --check`.
  Existing owner-provided FDD/HDD fixtures may be selected only through
  untracked cache settings; no fixture may be copied, changed, or tracked.
- **Deferred edge and risk:** FDC/HDC controller topology/lifecycle is the
  final T296 stage, but media backing/path policy and the single existing
  registry route remain VM-owned. T297--T299 remain deferred. The S2 VADP EGA
  CRTC index-`13h` storage-bound warning remains `TODO(High)`. Do not close
  this packet or begin later work without coordinator review after S4.

## Current Technical Baseline

- **T296 artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0296`. The S4 verification
  record will retain only `nxvm_0_5_0296.exe` under ignored `build/output/`.
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
| T288 | Resolved the external Setup post-copy `#CE` with the bounded 16-bit protected-mode call-gate and outer-`RETF` path; core and real replay now reach the next `MOV CR0,EAX` `#UD` checkpoint without claiming Windows support. |
| T289 | Materialized the default PC/AT ROM image before provider freeze, including its A20 reset alias, while reset restores only IVT/BDA and mutable device tables; ROM, boot, Console, debugger, and display regressions pass. |
| T290 | Replaced the FDC single-media binding with frozen drive slots and exact DOR/unit selection, proven through core ports, VM composition, and DOS FDD0 regression without broadening FDC behavior. |
| T291 | Made FDC generation, disk-change and motor/DMA cancellation per frozen drive slot, with core port and DOS FDD0 evidence while leaving timing, UI, and commands deferred. |
| T292 | Added prepared-state evidence that selected 67h REP comparison, scan, and segment-override forms already conform; no CPU behavior or artifact changed. |
| T293 | Removed the unused post-`#UD` transition surface, made protected outer `RETF`/`IRET` frame validation non-mutating until commit, and completed its exception/atomicity matrix: non-present returned `CS` is `#NP`, non-present `SS` is the retained terminal `#SS`, and invalid entries are `#GP`; all six `RETF`/outer-`IRET` cases preserve the defined pre-commit boundary. |
| T294 | Completed the codebase-driven public-surface/raw-borrow matrix, fixed T296's three stages and T297's future firmware-capability lifecycle precondition, and left T300/T302 conditional; no runtime behavior, CMake graph, or artifact changed. |
| T295 | Moved CPU execution-to-shared-PIC binding into `core_machine_create`; VM session no longer borrows or binds that path. CPU/PIC lifecycle evidence and all current gates passed; RTC/FDC/HDC/DMA remain T296 and debugger raw borrows remain T298/T299. |

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
