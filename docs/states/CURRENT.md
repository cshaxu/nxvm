# Project Status

## Current Work

**Active.** M5 T357 S1 establishes the source-backed, profile-aware
instruction-time contract required before its implementation can advance guest
time.

## M5 T357 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner approved the holistic device/L3 program and ordered implementation through reliable L3 on 2026-08-13. T356 closed at `02e11506` with M5 open and this first queued prerequisite selected. |
| Objective | Establish the complete source-to-form timing contract for 8086, 80186, 80286, and 80386 selected execution forms; identify the one CPU elapsed-time owner, its validation-to-retirement boundary, and the exact implementation-ready profile/form ledger for T357. |
| Non-goals | No guessed cycle values, CPU opcode repair, wait-state or bus-ownership model, prefetch queue, pin waveform, cache model, x87 timing, host-time coupling, Windows claim, or M5 closure. |
| Reference Baseline | `02e11506` on `master`; T356 final reconciliation, current selected-L3 trace/ordering, and the queued instruction-timed-execution proposal. |
| Candidate Proposal | [Instruction-timed execution](../proposals/m5-instruction-timed-execution.md); [T356 receiver plan](../etc/evidence/t356-s2-receiver-plan.md); [T356 reconciliation](../etc/evidence/t356-s3-m5-closure-reconciliation.md). |
| Files And ABI Surface | Documentation, T357 history, indexed evidence, Queue/CURRENT, and source inspection only. No C/CMake/runtime/ABI/current-gate/product change in S1. |
| Applicable Rules | Task Reading Set; execution lifecycle and mechanism-defect sweep; documentation rules; architecture one-owner/variant rules; source-and-research policy; roadmap M5 ordering. |
| Verification | Inventory every elapsed-tick reader/writer and current cost branch; reconcile the current timing smoke with the source contract; cite primary Intel timing references and their stated assumptions; classify each selected form/variant as S2 implementation, later bus-timed receiver, or explicit transfer; run documentation governance and diff checks. |
| Expected Markers | One indexed T357 S1 timing-contract ledger; one implementation-ready selected corpus with profile, form, evidence, and source reference; no unclassified existing elapsed-time writer or cost branch. |
| Asset Needs | No guest media, firmware, or third-party source import. External manuals are research references only; record URLs/provenance without copying them into the repository. |
| Reporting Requirements | Record the existing synthetic-cost limitation, one owner/publication point, timing-reference assumptions, every selected class, every excluded physical dependency, and the S2 implementation boundary. |
| Stop Conditions | Stop for owner decision if an Intel source cannot support a selected profile/form value, if the existing publication point cannot carry a profile/form result without a shared CPU executor redesign, or if a required cost depends on wait states, bus ownership, prefetch, or unavailable corpus. Do not invent values. |
| Exit Criteria | A committed evidence-led timing contract maps all current elapsed-time paths and the selected S2 corpus to primary sources or explicit transfer; Queue/history/status topology and governance pass; S2 can implement without rediscovering timing scope. |
| Original Owner Request | Fully and stably implement high-value devices and complete L3 timing before deciding the next phase; use a holistic mechanism plan rather than incremental opcode patches. |
| Similar-Issue Sweep | Search all tracked core/VM execution, clock, timeline, transaction, test, configuration, profile, CMake, evidence, Queue, TODO, and roadmap paths for elapsed-tick publication, timing configuration, instruction-cost logic, and device-clock consumption. |

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
| T356 | Closed the M5 closure audit with an explicit M5-open decision: all selected device owners are evidenced, but instruction timing, bus availability/wait states and selected-profile cycle exactness remain ordered implementation prerequisites; corpus-gated peripheral, storage and product boundaries remain explicit transfers. [History](../history/M5-T356-m5-closure-audit.md). |
| T355 | Closed the bounded Windows 3.x readiness map: an opt-in BYOB HDD/INT13 checkpoint passed, all four probes remain host-observation/non-current, T347/T354 storage/L3 is a verified prerequisite, and all display, input, NMI, physical-timing, Setup, and compatibility breadth remains explicitly transferred. [History](../history/M5-T355-windows-3x-readiness-map.md). |
| T354 | Closed selected L3 bus timing: core-local checked CPU/DMA transaction begin/commit/cancel trace, deterministic CPU-retire then DMA/PIT/PIC then FDC/HDC ordering, reset cancellation/trace continuity, and retained firmware/DOS consumers. Exact bus cycles, pin waveforms, physical wait tables, host time, and Windows readiness remain explicit transfers. 234 current-gate tests passed. [History](../history/M5-T354-physical-l3-bus-timing-convergence.md). |
| T353 | Closed selected PC/AT topology: the default profile's 75 exact directional leaves and five named routes validate before core publication, remain exact through session reset and deterministic-L3 rearm, and have firmware-visible reset proof. Optional PPI/speaker, NMI, and interface breadth remain corpus-gated TODO transfers; physical bus timing transfers to the next L3 package. 231 current-gate tests passed. [History](../history/M5-T353-pcat-port-topology-and-peripherals.md). |
| T352 | Closed selected digital CGA/EGA: one VADP owner for registers, apertures, raster, planar state, reset, and capture; deterministic VADP-after-KBC timing and copied session/mailbox composition; `vm-0-5-0352` SHA-256 `37A00874C05FB402151AF1A77BD28796D03B2A738D6F3EC08761E4541F2D6BB4`; 229 current-gate tests passed. [History](../history/M5-T352-vadp-digital-video-completeness.md). |
| T351 | Closed selected IBM-PC/AT 8042 controller, AT keyboard, and compatible three-byte AUX at deterministic L3: one FIFO, controller/keyboard/AUX head promotion, IRQ1/IRQ12 source lifecycle, delayed/reset/finalize, copied ingress, and exact advanced-AUX/NMI/port-61 transfers. 228 current-gate tests passed. [History](../history/M5-T351-kbc-aux-completeness.md). |
| T350 | Closed the selected PC/AT signal graph: retained 8254/IRQ0 and MC146818/CMOS/IRQ8 owners now have focused lifecycle proof; RTC alarm and IRQF defects are repaired; speaker/PPI and parity/I/O-channel NMI remain exact corpus/contract-gated TODO transfers. 228 current-gate tests passed. [History](../history/M5-T350-pcat-platform-signals.md). |
| T349 | Closed PC/AT dual-8259A command/priority/cascade, OCW3 poll/status/special-mask/SFNM, and retained edge/level producer lifecycle; physical INTA spurious behavior transfers only to L3 bus timing. 226 current-gate tests passed. [History](../history/M5-T349-pcat-pic-compliance.md). |

## Recent Governance

- **M5 Td S85:** corrected the Windows readiness candidate so FDC/ATA
  pending/readiness service is an accepted regression baseline, rather than a
  stale missing-feature blocker. Physical device timing and Windows
  compatibility remain unclaimed. Documentation governance and diff check
  passed; Td work has no runtime change.

- **M5 Td S84:** retired the stale FDC/ATA command-service TODO after the
  accepted T347 pending-command/readiness-service closure and T354 consumer
  audit reconfirmed its claim was no longer true. Physical wait/cycle fidelity
  remains in the existing bus-timed and cycle-exact debt entries. Documentation
  governance and diff check passed; Td work has no runtime change.

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
