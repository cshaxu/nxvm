# Project Status

## Current Work

M5 T352 S1 - selected PC/AT digital-video state ledger
(Single-Session Mode).

## M5 T352 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner approved continuous, holistic device/L3 implementation on 2026-08-13 and authorized the next Queue candidate after T351 closure. |
| Objective | Establish the complete selected digital CGA/EGA manual-to-source-to-proof ledger: every VADP register, CRTC/raster/retrace state, aperture/mode interpretation, timeline event, copied-frame publication, reset/finalize edge, consumer, and proof row receives one S2--S5 owner or an exact later transfer. |
| Non-goals | No production behavior change, renderer rewrite, VGA/VBE/SVGA, composite simulation, host pacing/GPU work, firmware, Windows execution, or unselected display feature. |
| Reference Baseline | `861b943e` / T351 closed; T346 deterministic L3 closure; current VADP/CGA/EGA port, memory, raster, ROM/DOS, and copied-frame smoke evidence; [digital-video proposal](../proposals/m5-vadp-digital-video-completeness.md). |
| Candidate Proposal | [PC/AT digital video completeness](../proposals/m5-vadp-digital-video-completeness.md). |
| Files And ABI Surface | Expected: T352 S1 evidence, its index row, T352 history progress, and Current only. No production, CMake, public ABI, copied-frame format, host boundary, firmware, or artifact version change. |
| Applicable Rules | Task Reading Set; VADP is the sole display-state owner; copied presentation receives immutable/copy publication; deterministic timeline owns readiness/order; equivalent variants share validation/publication unless a real selected hardware difference exists; source policy before new manual research. |
| Verification | Build a finite selected state matrix with source/manual authority, implementation owner, writers/readers, reset/finalize/timeline behavior, current focused proof, missing evidence, receiver, and precise admission condition. Sweep VADP, ports, video RAM, CRTC, raster, BIOS mode writers, timeline, display consumers, CMake current tests, Queue, TODO, and T346/T351 transfers. Run documentation governance and diff check. |
| Expected Markers | Ledger-only S1: retain existing CGA/EGA/VADP/raster/DOS/timeline markers; add no runtime marker. |
| Asset Needs | Existing repository sources, indexed project evidence, and authorized primary manual research only if needed; no firmware, guest media, host capture, or imported code. |
| Reporting Requirements | Record the selected hardware matrix, every owner/reader/writer/proof/transfer disposition, candidate S2--S5 boundaries, source provenance, and every owner decision required before implementation. |
| Stop Conditions | Stop for an ambiguous selected display matrix, an unlicensed/source-policy-blocked manual, a required host/firmware/product ABI decision, a generic timeline/PIC/memory defect, or a missing consumer contract that cannot be truthfully assigned. Request owner decision or create a precise transfer rather than infer support. |
| Exit Criteria | A complete, non-overlapping selected digital-video state ledger exists; every row has one implementation receiver or exact deferred owner; S2--S5 are dependency-ordered and bounded; no mode/port/raster fix is admitted by convenience. |
| Original Owner Request | Make CPU/fpu-external core-machine devices, chips, buses, and ports comprehensive and stable at deterministic L3 before choosing a Windows execution route. |
| Similar-Issue Sweep | Inspect all VADP state structs/helpers, port and memory handlers, CRTC/raster calculations, mode/ROM writers, machine timeline callbacks, copied-frame consumers, reset/finalize paths, display tests, and existing video/TODO/Queue evidence. |

## Current Technical Baseline

- **Current developer artifact:** T351 selects `vm-0-5-0351` /
  `build/output/nxvm_0_5_0351.exe`; its closure-build SHA-256 is
  `C20F4B631F8E63DD4D963C2F85E69D735EC05793B3DCC0C2AC11E65FA6D3FB9C`.
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
| T351 | Closed selected IBM-PC/AT 8042 controller, AT keyboard, and compatible three-byte AUX at deterministic L3: one FIFO, controller/keyboard/AUX head promotion, IRQ1/IRQ12 source lifecycle, delayed/reset/finalize, copied ingress, and exact advanced-AUX/NMI/port-61 transfers. `vm-0-5-0351` SHA-256 `C20F4B631F8E63DD4D963C2F85E69D735EC05793B3DCC0C2AC11E65FA6D3FB9C`; 228 current-gate tests passed. [History](../history/M5-T351-kbc-aux-completeness.md). |
| T350 | Closed the selected PC/AT signal graph: retained 8254/IRQ0 and MC146818/CMOS/IRQ8 owners now have focused lifecycle proof; RTC alarm and IRQF defects are repaired; speaker/PPI and parity/I/O-channel NMI remain exact corpus/contract-gated TODO transfers. 228 current-gate tests passed. [History](../history/M5-T350-pcat-platform-signals.md). |
| T349 | Closed PC/AT dual-8259A command/priority/cascade, OCW3 poll/status/special-mask/SFNM, and retained edge/level producer lifecycle; physical INTA spurious behavior transfers only to L3 bus timing. 226 current-gate tests passed. [History](../history/M5-T349-pcat-pic-compliance.md). |
| T348 | Closed PC/AT dual-8237A DMA topology, page/word layout, request/cascade/EOP, validation-before-publication, M2M lifecycle, and FDC crossing bounce path; 223 current-gate tests passed. [History](../history/M5-T348-pcat-dma-completeness.md). |
| T347 | Closed retained PC/AT FDC and ATA PIO service timing: pending command/data states, reset/control/media cancellation, one FDC-then-ATA readiness owner, trace order, focused VM/DOS/HDD evidence, and 222 current-gate tests passed. [History](../history/M5-T347-storage-controller-service-timing.md). |
| T346 | Closed deterministic PC/AT L3 convergence: one reset-safe due-event owner, equal-tick `DMA -> PIT -> PIC -> FDC -> HDC -> RTC -> KBC -> VADP` ordering, copied host boundaries, exact storage/NMI/peripheral transfers, and evidence-backed Windows handoff. [History](../history/M5-T346-core-machine-device-l3-convergence.md). |
| T345 | Closed direct-compilation strictness convergence: 251/305 direct commands are target-local strict; the 54 remaining commands have a complete 175-row ownership ledger and an exact 51-source residual production record with durable bounded admissions. No global flags, inherited-runtime rewrites, or false linked-dependency claims. [History](../history/M5-T345-direct-compilation-strictness-convergence.md). |
| T344 | Closed build-quality reproducibility: fresh configuration, 305-row truthful strict-compile matrix (130 retained strict/175 deferred), canonical 218-target current-gate registration, 53 historical fixture shapes, and strict-declaration uniqueness are mechanically verified. [History](../history/M5-T344-build-quality-reproducibility.md). |

## Recent Governance

- **M5 Td S83:** corrected the NXVM self-sibling instruction in `AGENTS.md`;
  renamed the VM-platform injected test macro to
  `VM_PLATFORM_TEST_FAILURE_STAGE` across CMake, implementation, and smoke
  sources; and aligned CMake's project identity and diagnostics with NXVM.
  Documentation self-test, default governance check, exact macro/search audit,
  and diff check passed. Td work has no intended runtime or artifact behavior
  change.

- **M5 Td S82:** restored all 19 historical NXVM README screenshots as static
  documentation assets; rebuilt the public README around the current CMake
  path and truthful NXVM-first, future-NXVDM boundary; and aligned the current
  design/rule wording and README schema self-test. Documentation self-test,
  default governance check, README-link check, and diff check passed. Td work
  has no runtime or artifact change.

- **M5 Td S81:** withdrew the duplicate 80286 `LOCK` candidate after the
  T339 closure audit reconfirmed that accepted T328 already owns the complete
  pre-386 prefix-policy matrix. The Queue now advances directly from T339 to
  the 80386DX form closure; current proposals and T339 transfer ledgers name
  T328, T341, and T342 truthfully. Documentation governance and diff check
  passed. Td work has no runtime or artifact change.

- **M5 Td S80:** reordered the four-profile CPU-completeness Queue into
  dependency-bounded 8086/80186, 80286 descriptor-transfer, 80286 `LOCK`,
  80386DX form, 80386DX state, 80386DX audit, and cross-profile closure
  candidates; added the corresponding unnumbered proposals. Documentation
  governance and diff check passed. Td work has no runtime or artifact change.

- **M5 Td S79:** reordered the M5 CPU-completeness program around a
  four-profile audit, shared delivery foundations, 8086/80186, 80286, and
  80386DX closure candidates, then cross-profile verification. Each candidate
  has a linked unnumbered proposal; no implementation task was allocated.
  Documentation governance, Queue-link verification, and diff check passed.

- **M5 Td S77:** moved the closed T323/T325/T328 proposals into matching
  history companions; made every current proposal require a Queue link and
  added the orphan-proposal negative self-test; compacted repeated
  mechanism-defect requirements into role-specific authorities; clarified
  historical terminology retention and the idle T332 artifact wording.
  Documentation self-test, combined governance check, and diff check passed.
  Td work has no runtime or artifact change.

- **M5 Td S76:** retired the closed T330 task-switch debt from TODO and the
  unqueued proposal surface; retained its proposal, debt report, history, and
  matrix as explicitly historical/non-current evidence; and corrected the
  stale T330 closure summary. Documentation governance and diff check passed.
  Td work has no runtime or artifact change.

- **M5 Td S75:** reconciled Queue state with recorded closures: removed only
  the T323 protection/privilege, T325 paging, T328 legacy LOCK, and T330
  transition-unification candidates; retained the remaining candidate order.
  Documentation governance and diff check passed. Td work has no runtime or
  artifact change.
