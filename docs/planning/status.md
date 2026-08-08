# Project Status

## Current Work

**M5 T296 S2 active - core-owned VADP/display-port configuration authority.**

S1 is committed as `56433a9` and remains recorded in [the T296 migration
matrix](../architecture/core-machine-shared-device-migration.md). S2 alone
moves the display declaration authority into `core_machine`; the task remains
active for coordinator review. Do not admit S3 (DMA/RTC/CMOS/NMI), S4
(FDC/HDC), or T297 and later work.

### Task Packet

- **Original request:** Move remaining shared-device initialization, binding,
  reset, and finalization authority to `core_machine` in three fixed stages.
  VM composition/profile may submit only frozen typed topology, configuration,
  and provider/media policy; it must not borrow or bind controller, PIC, or
  port storage.
- **S2 objective and completion condition:** VM composition submits one typed,
  exact VADP display declaration while `INITIALIZED`: neutral text timing, EGA
  sequencer/controller configuration, declared VADP port groups, and a typed
  display-provider slot. `core_machine_configure_display` validates and copies
  the declaration, configures core-owned VADP/RAM state, freezes the provider,
  and rejects subsequent submission. Core retains the existing VADP/port
  create, reset, and finalize order. Completion is the focused owner gate,
  lifecycle smoke, and full current GCC gate below; this packet stays active
  pending coordinator review.
- **Reference baseline:** T295; `vm-0-5-0295`,
  `nxvm_0_5_0295.exe` SHA-256
  `52B291B1E1100D945BD44B7B1F88A622F7B2B7D3468BC78997ED90732BCA179A`.
- **In scope:** `core_machine` display configuration API and retained
  VADP/port lifecycle; VM composition's typed profile declaration/provider;
  an S2 static owner gate and lifecycle smoke. The copied display snapshot
  boundary is unchanged.
- **Non-goals:** DMA; RTC/CMOS/NMI; FDC/HDC; scheduler; second machine;
  storage mirror; host shortcut; media/path policy migration; and any
  Console/debugger/boot experience change. T297 firmware capability, T298
  debugger capability, and T299 raw-borrow deletion remain deferred.
- **Applicable rules:** `core/machine` owns neutral mutable guest state and
  lifecycle order; VM/profile owns immutable PC/AT topology, defaults, ROM and
  provider/media policy. Core receives no PC/AT/default-profile/ROM-vendor,
  BIOS/DOS, local-path, or product-policy meaning. Preserve the single media
  route and owner-provided read-only test media rule. No source import or
  license/provenance action is involved.
- **Implementation and call chain:** `vm_session_storage_initialize` binds its
  typed provider slot, submits immutable profile timing/EGA values and declared
  VADP port ranges to `core_machine_configure_display`, then creates the
  retained profile binding. The core entry applies VADP text/EGA state against
  embedded executor RAM, records the exact topology, and freezes the slot.
  The existing `core_machine_create`, `core_machine_reset`, and
  `core_machine_destroy` continue to initialize, reset, and finalize embedded
  VADP and port storage. The three VADP profile-binding configurators are
  removed; no VM display source configures VADP, RAM, or port storage directly.
- **Similar-issue sweep:** The defect class is VM composition directly using
  profile-binding VADP configurators or display-port installation. Query:
  `rg -n "core_machine_(profile_binding_configure_|vadp_configure_|install_port_provider)" src/vm --glob '*.[ch]'`.
  The three display configurator hits in `session.c` are replaced. Remaining
  RTC and HDC port-install hits are recorded S3/S4 edges in the migration
  matrix and are excluded from S2; the S2 static gate covers only the two
  display-composition sources so it cannot mask those deferred owners.
- **S2 evidence commands and result:**
  `cmake --build --preset current-gates-gcc --target core-machine-display-authority-smoke verify-core-display-authority`;
  `build/mingw-gcc-x64/core-machine-display-authority-smoke.exe` (marker
  `M5:T296:S2:DISPLAY-AUTHORITY:OK`);
  `cmake --build --preset current-gates-gcc` (46/46 static/build gates and
  123/123 CTest current-gate tests passed, including VADP text/status,
  CGA/EGA, ROM-video, Console, and two-session isolation). The GCC CMake cache
  used existing owner-provided FDD/HDD fixtures through untracked FILEPATH
  overrides only; no fixture was copied, changed, or tracked.
  `powershell -NoProfile -ExecutionPolicy Bypass -File tools/Verify-DocumentationGovernance.ps1 -RepositoryRoot .`;
  `git diff --check`.
- **Deferred edge and risk:** The S2 LLVM syntax check exposes a pre-existing
  VADP EGA CRTC index-`13h` storage-bound warning. It is deferred as
  `TODO(High)` with its required focused admission gate; this authority-only
  subtask does not alter it. RTC and HDC direct port installation likewise
  remain untouched for S3/S4. Do not close this packet, build T296's final
  artifact, or advance beyond S2 without coordinator review.

## Current Technical Baseline

- **T295 artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0295`. Artifact
  `nxvm_0_5_0295.exe` SHA-256:
  `52B291B1E1100D945BD44B7B1F88A622F7B2B7D3468BC78997ED90732BCA179A`.
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
