# Project Status

## Current Work

**M5 T296 S3 active - core-owned DMA and RTC/CMOS/NMI authority.**

S1 is committed as `56433a9` and remains recorded in [the T296 migration
matrix](../architecture/core-machine-shared-device-migration.md). S2 is
committed as `e84199e`; S3 alone moves DMA and RTC/CMOS/NMI authority into
`core_machine`. The task remains active for coordinator review. Do not admit
S4 (FDC/HDC) or T297 and later work.

### Task Packet

- **Original request:** Move remaining shared-device initialization, binding,
  reset, and finalization authority to `core_machine` in three fixed stages.
  VM composition/profile may submit only frozen typed topology, configuration,
  and provider/media policy; it must not borrow or bind controller, PIC, or
  port storage.
- **S3 objective and completion condition:** VM composition submits frozen
  typed DMA wiring and RTC/CMOS/NMI declaration while `INITIALIZED`: DMA
  channel, RTC IRQ/tick rate, CMOS default bytes, CMOS port pair, and NMI
  index-port wiring. Core validates, copies, and applies those declarations;
  it owns embedded RTC storage, CMOS port callbacks, DMA binding, reset,
  advance, and finalization. The existing core scheduler is the sole RTC time
  path. Completion requires an S3 owner gate, focused lifecycle smoke, and the
  retained/full GCC gates; this packet stays active pending coordinator review.
- **Reference baseline:** T295; `vm-0-5-0295`,
  `nxvm_0_5_0295.exe` SHA-256
  `52B291B1E1100D945BD44B7B1F88A622F7B2B7D3468BC78997ED90732BCA179A`.
- **In scope:** `core_machine` DMA and RTC/CMOS/NMI typed configuration,
  embedded RTC storage/port provider, lifecycle and scheduler advance; VM
  profile declaration; an S3 static owner gate and lifecycle smoke.
- **Non-goals:** FDC/HDC connection, lifecycle, ports, drive topology, media
  registry/backing/path policy; scheduler or second machine; storage mirror;
  host shortcut; and Console/debugger/boot experience change. T297 firmware
  capability, T298 debugger capability, and T299 raw-borrow deletion remain
  deferred.
- **Applicable rules:** `core/machine` owns neutral mutable guest state and
  lifecycle order; VM/profile owns immutable PC/AT topology, defaults, ROM and
  provider/media policy. Core receives no PC/AT/default-profile/ROM-vendor,
  BIOS/DOS, local-path, or product-policy meaning. Preserve the single media
  route and owner-provided read-only test media rule. No source import or
  license/provenance action is involved.
- **Implementation and call chain:** `vm_session_storage_initialize` submits
  profile-derived DMA wiring and a neutral RTC/CMOS declaration (port pair,
  IRQ/ticks, NMI bit, and six default register bytes) to
  `core_machine_configure_dma` and `core_machine_configure_rtc_cmos` before
  profile binding. Core copies the declarations, owns embedded RTC state and
  the CMOS port callbacks, binds the embedded FDC DMA endpoint, and returns
  only the frozen `core_machine_dma_request_binding` needed by the still-S4
  FDC connect. `core_machine_cold_reset` resets embedded DMA/RTC; its existing
  scheduler advances RTC from the existing provider clock tick before PIC
  refresh. VM's execution provider no longer advances guest time.
- **Similar-issue sweep:** The defect class is VM composition directly
  borrowing/binding DMA or directly initializing, resetting, advancing,
  finalizing, or installing the RTC/CMOS/NMI path. Query:
  `rg -n "core_machine_(configuration_shared_dma_|dma_bind_channel|rtc_initialize|rtc_reset|rtc_advance|rtc_finalize|rtc_select_register|rtc_read_selected|rtc_write_selected|rtc_write_nvram|set_nmi_mask)" src/vm --glob '*.[ch]'`.
  It has no production hits after S3. The remaining HDC port installation and
  FDC/PIC/port connection calls are explicit S4 edges; the S3 gate scans the
  DMA/RTC section separately so they are neither changed nor hidden.
- **S3 evidence commands and result:**
  `cmake --build --preset current-gates-gcc --target core-machine-dma-rtc-authority-smoke verify-core-dma-rtc-authority verify-cmos-rtc-boundary`;
  `build/mingw-gcc-x64/core-machine-dma-rtc-authority-smoke.exe` (marker
  `M5:T296:S3:DMA-RTC-AUTHORITY:OK`);
  `cmake --build --preset current-gates-gcc` (47/47 static/build gates and
  124/124 CTest current-gate tests passed, including retained DMA,
  RTC/CMOS/NMI, timer/IRQ, boot, Console/debugger, and two-session isolation).
  Existing owner-provided FDD/HDD fixtures were selected by untracked cache
  settings only; no fixture was copied, changed, or tracked.
  `powershell -NoProfile -ExecutionPolicy Bypass -File tools/Verify-DocumentationGovernance.ps1 -RepositoryRoot .`;
  `git diff --check`.
- **Deferred edge and risk:** S4 alone may migrate FDC/HDC connection,
  controller/port lifecycle, drive topology, and media policy; S3 preserves
  the single media route and leaves those VM calls intact. T297--T299 remain
  deferred. The S2 VADP EGA CRTC index-`13h` storage-bound warning remains
  `TODO(High)`. Do not close this packet, build T296's final `0296` artifact,
  or advance beyond S3 without coordinator review.

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
