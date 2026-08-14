# Project Status

## Current Work

**Active.** M5 T354 S1 builds the selected PC/AT transaction/timing ledger
before any L3 physical wait, arbitration, or controller-service change.

## M5 T354 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner approved the holistic CPU-external device/L3 completion direction and authorized implementation through reliable selected L3 on 2026-08-13. This S admits only the T354 S1 ledger; no timing implementation is approved until a later packet consumes its result. |
| Objective | Produce the complete selected PC/AT CPU/DMA/device transaction, timing, lifecycle, and proof ledger that distinguishes deterministic ordering, implemented latency, supported immediate behavior, and unproven physical timing. |
| Non-goals | No production timing/wait/arbitration change; no new device, PPI/speaker, NMI source, external asset, Windows run, generic bus wrapper, or source import. |
| Reference Baseline | `7b36b6f4` on `master`, after T353 selected PC/AT topology closure. |
| Candidate Proposal | [M5 physical L3 bus-timing convergence](../proposals/m5-l3-bus-timing-convergence.md), S1. |
| Files And ABI Surface | `docs/states/CURRENT.md`, one indexed T354 S1 evidence ledger, T354 history record, and only necessary Queue/TODO transfer links. No C/CMake/public ABI/runtime artifact changes. |
| Applicable Rules | `docs/README.md` Task Reading Set; `docs/rules/EXECUTION.md` S/P and mechanism-defect lifecycle; `docs/rules/DOCUMENT.md` authority/index/debt boundary; `docs/rules/ARCHITECTURE.md` ownership; source policy for reference research; T346--T353 evidence and T354 proposal. |
| Verification | Sweep core memory/port/bus/timeline/DMA/PIC/FDC/HDC/PIT/RTC/KBC/VADP and VM profile/session consumers; map all schedule/reset/finalize/trace/commit sites; reconcile every selected row to source/evidence/current behavior and one S2/S3/S4/TODO receiver. Verify documentation governance and diff check. |
| Expected Markers | `Documentation governance checks passed`; no runtime marker applies to this audit-only S. |
| Asset Needs | Repository sources/evidence and permitted reference metadata only. No guest media, firmware, host capture, or external source import. |
| Reporting Requirements | Record each owner, caller/reader/writer, timing unit/state, validation-to-commit boundary, reset/cancellation behavior, existing proof, defect/risk, and exact receiver; report a material timing-contract discovery before changing code. |
| Stop Conditions | Stop for owner direction if the audit needs an unapproved external hardware corpus/manual, changes the selected topology, or demonstrates a shared timing defect requiring implementation before the ledger can truthfully allocate it. |
| Exit Criteria | Every selected transaction/timing row is classified and allocated to exactly one immediate proof, S2/S3/S4 receiver, or TODO transfer; no false L3/cycle-exact claim, unowned timing state, or unclassified selected producer/consumer remains. |
| Original Owner Request | Holistically audit and complete CPU-external chips, buses, devices, and ports to stable reliable L3 before deciding the next product step; use manuals/reference implementations as evidence without importing them. |
| Similar-Issue Sweep | Include all existing timeline callbacks, core memory/port dispatch, CPU retirement, DMA grant/request/EOP, PIC acknowledgement, controller readiness, session reset/finalize, and trace paths; do not restrict the audit to the first reproduced wait symptom. |

## Current Technical Baseline

- **Current developer artifact:** T352 selects `vm-0-5-0352` /
  `build/output/nxvm_0_5_0352.exe`; its closure-build SHA-256 is
  `37A00874C05FB402151AF1A77BD28796D03B2A738D6F3EC08761E4541F2D6BB4`.
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
| T353 | Closed selected PC/AT topology: the default profile's 75 exact directional leaves and five named routes validate before core publication, remain exact through session reset and deterministic-L3 rearm, and have firmware-visible reset proof. Optional PPI/speaker, NMI, and interface breadth remain corpus-gated TODO transfers; physical bus timing transfers to the next L3 package. 231 current-gate tests passed. [History](../history/M5-T353-pcat-port-topology-and-peripherals.md). |
| T352 | Closed selected digital CGA/EGA: one VADP owner for registers, apertures, raster, planar state, reset, and capture; deterministic VADP-after-KBC timing and copied session/mailbox composition; `vm-0-5-0352` SHA-256 `37A00874C05FB402151AF1A77BD28796D03B2A738D6F3EC08761E4541F2D6BB4`; 229 current-gate tests passed. [History](../history/M5-T352-vadp-digital-video-completeness.md). |
| T351 | Closed selected IBM-PC/AT 8042 controller, AT keyboard, and compatible three-byte AUX at deterministic L3: one FIFO, controller/keyboard/AUX head promotion, IRQ1/IRQ12 source lifecycle, delayed/reset/finalize, copied ingress, and exact advanced-AUX/NMI/port-61 transfers. 228 current-gate tests passed. [History](../history/M5-T351-kbc-aux-completeness.md). |
| T350 | Closed the selected PC/AT signal graph: retained 8254/IRQ0 and MC146818/CMOS/IRQ8 owners now have focused lifecycle proof; RTC alarm and IRQF defects are repaired; speaker/PPI and parity/I/O-channel NMI remain exact corpus/contract-gated TODO transfers. 228 current-gate tests passed. [History](../history/M5-T350-pcat-platform-signals.md). |
| T349 | Closed PC/AT dual-8259A command/priority/cascade, OCW3 poll/status/special-mask/SFNM, and retained edge/level producer lifecycle; physical INTA spurious behavior transfers only to L3 bus timing. 226 current-gate tests passed. [History](../history/M5-T349-pcat-pic-compliance.md). |
| T348 | Closed PC/AT dual-8237A DMA topology, page/word layout, request/cascade/EOP, validation-before-publication, M2M lifecycle, and FDC crossing bounce path; 223 current-gate tests passed. [History](../history/M5-T348-pcat-dma-completeness.md). |
| T347 | Closed retained PC/AT FDC and ATA PIO service timing: pending command/data states, reset/control/media cancellation, one FDC-then-ATA readiness owner, trace order, focused VM/DOS/HDD evidence, and 222 current-gate tests passed. [History](../history/M5-T347-storage-controller-service-timing.md). |
| T346 | Closed deterministic PC/AT L3 convergence: one reset-safe due-event owner, equal-tick `DMA -> PIT -> PIC -> FDC -> HDC -> RTC -> KBC -> VADP` ordering, copied host boundaries, exact storage/NMI/peripheral transfers, and evidence-backed Windows handoff. [History](../history/M5-T346-core-machine-device-l3-convergence.md). |

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
