# Project Status

## Current Work

**M5 T294 S1 -- core/machine public-surface and borrow inventory (active).**

The approved packet is [the T294 S1 inventory](../architecture/core-machine-public-surface-inventory.md).
It is documentation/design only: no runtime C/CMake behavior, migration API, borrow
removal, queue movement, or artifact revision is admitted by this subtask.

| Packet field | Record |
| --- | --- |
| Original request | Create a real-codebase-driven matrix for every `core/machine` public interface, configuration borrow, profile/firmware binding, provider registration, debug access, and test-only probe; identify consumers, owner/lifetime, raw-state exposure, replacement or retention, core initialization responsibility, later task, and regression/gate. |
| Scope | `src/core/machine/*_interface.h`, `machine_interface.h`, their definitions, and all `src/`/`tests/` callers; the matrix also fixes T296's display/port, DMA+RTC/CMOS/NMI, and FDC/HDC stages and T297's required pre-implementation contract topics. |
| Non-goals | Do not begin T295--T299; do not change runtime C/CMake, Console/debugger/startup/artifact/current target, providers, firmware behavior, or protected media/third-party inputs. T300/T302 remain conditional only. |
| Applicable rules | `module-layout.md`: core owns guest state and initialization order while VM composes frozen policy/providers; `contracts.md`: copied observation and non-reentrant callbacks; `coding-standard.md`: smallest existing boundary/no test facade for convenience; `source-policy.md`: no imports/assets; `execution-workflow.md` and `execution-policy.md`: one active packet, evidence, documentation gate. No exception requested. |
| Similar-issue sweep | Not a defect repair. The design audit uses the same repository-wide symbol sweep needed to establish closure; it classifies every hit instead of modifying one instance. |
| Commands and expected markers | `rg -n "core_machine_(configuration|profile_binding|debug)_" src tests --glob '*.[ch]'` (definitions plus all current borrow consumers); `rg -n "core_machine_(bind_execution_provider|freeze_execution_providers|install_port_provider|register_immutable_rom_mapping|set_trace_provider|apply_entry_plan)" src tests --glob '*.[ch]'` (registration/contract callers); documentation governance command must report `Documentation governance verification passed.` |
| Stop condition | Matrix, lifecycle/ownership decisions, deferred conditional-admission statement, and static closure-gate design are reviewable; status stays active after the commit pending owner feedback. |

## Current Technical Baseline

- **T293 artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0293`. Artifact
  `nxvm_0_5_0293.exe` SHA-256:
  `8EA2A81A4EB99F1C541CE9D6CDE3805BBD292A5AFD3A9334A39ED840AE45267E`.
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
| T286 | Fixed the corpus-proven ATA device-control `nIEN` IRQ14 visibility gap through core-owned controller state, with core, VM-port, and guest fixture success evidence; no DMA, timing, or command expansion. |
| T287 | Fixed bounded ROM CHS device/head, AH=08h caller-pointer, FDISK text-service/window-clear, and HDD-only boot-selection defects; external DOS registers C:, presents stable FDISK copied frames, and hands off from HDD-only ATA boot, while the Standard-mode checkpoint remains a research result, not a Windows support claim. |
| T288 | Resolved the external Setup post-copy `#CE` with the bounded 16-bit protected-mode call-gate and outer-`RETF` path; core and real replay now reach the next `MOV CR0,EAX` `#UD` checkpoint without claiming Windows support. |
| T289 | Materialized the default PC/AT ROM image before provider freeze, including its A20 reset alias, while reset restores only IVT/BDA and mutable device tables; ROM, boot, Console, debugger, and display regressions pass. |
| T290 | Replaced the FDC single-media binding with frozen drive slots and exact DOR/unit selection, proven through core ports, VM composition, and DOS FDD0 regression without broadening FDC behavior. |
| T291 | Made FDC generation, disk-change and motor/DMA cancellation per frozen drive slot, with core port and DOS FDD0 evidence while leaving timing, UI, and commands deferred. |
| T292 | Added prepared-state evidence that selected 67h REP comparison, scan, and segment-override forms already conform; no CPU behavior or artifact changed. |
| T293 | Removed the unused post-`#UD` transition surface, made protected outer `RETF`/`IRET` frame validation non-mutating until commit, and completed its exception/atomicity matrix: non-present returned `CS` is `#NP`, non-present `SS` is the retained terminal `#SS`, and invalid entries are `#GP`; all six `RETF`/outer-`IRET` cases preserve the defined pre-commit boundary. |

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
