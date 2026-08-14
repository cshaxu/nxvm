# Project Status

## Current Work

M5 T351 S5 - combined 8042, keyboard, and AUX lifecycle closure
(Single-Session Mode).

## M5 T351 S5 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved continuous holistic device/L3 completion on 2026-08-13; T351 S4 accepted at `cd99fc9b`. |
| Objective | Complete T351's closure audit: prove one KBC FIFO promotes controller/keyboard/AUX origins with correct IRQ1/IRQ12 eligibility and deassertion; reconcile delayed response, full/disabled states, equal-tick KBC ordering, profile/session copied ingress, reset/finalize, all S1--S4 transfers, task artifact, evidence, Queue/history, and T-level closure. |
| Non-goals | No new controller, keyboard, or AUX feature; no advanced AUX protocol, host passthrough, firmware, port-61/PPI, NMI source, generic PIC/timeline refactor, device identity expansion, x87, or Windows runtime claim. |
| Reference Baseline | `cd99fc9b` / T351 S4 accepted; [T351 S1 ledger](../etc/evidence/t351-s1-kbc-aux-ledger.md), [S2 controller](../etc/evidence/t351-s2-kbc-controller.md), [S3 keyboard](../etc/evidence/t351-s3-keyboard-device.md), [S4 AUX](../etc/evidence/t351-s4-aux-device.md), T346 deterministic timeline closure, and T351 proposal. |
| Candidate Proposal | [PC/AT 8042 and AUX completeness](../proposals/m5-kbc-aux-completeness.md). |
| Files And ABI Surface | Expected: a focused combined KBC lifecycle smoke and CMake registration if proof gap requires it; S5 evidence, Current/history, Queue/proposal relocation, current developer artifact target/version/SHA only. No public interface, host/input/profile ABI, second FIFO, advanced device command, or unrelated device change. |
| Applicable Rules | Task Reading Set; one guest-visible FIFO and per-head IRQ owner; controller/keyboard/AUX variants share publication and reset/finalize ownership; machine remains deterministic readiness owner; copied host boundary; full task closure audit; exact advanced-AUX/NMI/port-61 transfers; artifact identity rule; no source import. |
| Verification | Prove mixed controller/keyboard/AUX FIFO order, promotion and source deassert/reassert, disabled command-byte/interface gates, delayed response behind queued origin bytes, reset/finalize cancellation, machine equal-tick trace ordering, copied session ingress, and no duplicate writer. Re-run all retained focused KBC/AUX/keyboard/mouse/timeline tests and full current gate. Build `vm-0-5-0351`, copy `nxvm_0_5_0351.exe`, record SHA-256 and identity, run governance, artifact verifier, and diff check. |
| Expected Markers | Retain S2 controller, S3 keyboard, S4 AUX, guest/DOS mouse, ingress, and T346 timeline markers; add a combined S5 marker only if a dedicated owner smoke is required. |
| Asset Needs | Project sources, published documentation already recorded in S1--S4, and existing project probes only; no firmware, guest media addition, host capture, or third-party source. |
| Reporting Requirements | Map original owner request to S1--S5 evidence; report combined FIFO/IRQ/reset/timing proof, all task-level transfers, developer artifact SHA-256, exact full-gate result, Queue/proposal disposition, and any residual owner decision. |
| Stop Conditions | Stop for a new public/profile/host ABI, an uncovered controller/keyboard/AUX semantic requiring S2--S4 corrective work, generic PIC/timeline mechanism defect, artifact build identity failure, source import, or requirement to implement an unselected advanced device feature. Admit corrective work rather than hide it in closure. |
| Exit Criteria | One stable deterministic L3 state graph covers every selected controller/keyboard/AUX signal and lifecycle; all S1--S4 rows are proven or precisely transferred; a verified T351 artifact and SHA exist; proposal is retained in history, Queue advances, governance passes, and no unclassified KBC/AUX boundary remains. |
| Original Owner Request | Make core-machine devices stable, comprehensive, and reliable at deterministic L3 before deciding any Windows execution route. |
| Similar-Issue Sweep | Inspect all KBC FIFO/origin/IRQ helpers, port writers/readers, delayed and typematic timing paths, machine timeline/profile/reset/finalize routes, session ingress, KBC/AUX/keyboard/mouse current targets, CMake artifact wiring, Queue/TODO/history/proposal records, and prior T346/T350 transfers. |

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
| T351 S4 | Accepted the selected compatible three-byte AUX device: command-byte/interface IRQ12 gates, response delay, report clipping/no-change/full-FIFO atomicity, reset/finalize, copied guest ingress, and DOS consumer proof are reconciled. Wheel, scaling, remote/read-data, resend/error, and capture remain exact advanced-AUX TODO transfers. 228 current-gate tests passed. [Evidence](../etc/evidence/t351-s4-aux-device.md). |
| T350 | Closed the selected PC/AT signal graph: retained 8254/IRQ0 and MC146818/CMOS/IRQ8 owners now have focused lifecycle proof; RTC alarm and IRQF defects are repaired; speaker/PPI and parity/I/O-channel NMI remain exact corpus/contract-gated TODO transfers. 228 current-gate tests passed. [History](../history/M5-T350-pcat-platform-signals.md). |
| T349 | Closed PC/AT dual-8259A command/priority/cascade, OCW3 poll/status/special-mask/SFNM, and retained edge/level producer lifecycle; physical INTA spurious behavior transfers only to L3 bus timing. 226 current-gate tests passed. [History](../history/M5-T349-pcat-pic-compliance.md). |
| T348 | Closed PC/AT dual-8237A DMA topology, page/word layout, request/cascade/EOP, validation-before-publication, M2M lifecycle, and FDC crossing bounce path; 223 current-gate tests passed. [History](../history/M5-T348-pcat-dma-completeness.md). |
| T347 | Closed retained PC/AT FDC and ATA PIO service timing: pending command/data states, reset/control/media cancellation, one FDC-then-ATA readiness owner, trace order, focused VM/DOS/HDD evidence, and 222 current-gate tests passed. [History](../history/M5-T347-storage-controller-service-timing.md). |
| T346 | Closed deterministic PC/AT L3 convergence: one reset-safe due-event owner, equal-tick `DMA -> PIT -> PIC -> FDC -> HDC -> RTC -> KBC -> VADP` ordering, copied host boundaries, exact storage/NMI/peripheral transfers, and evidence-backed Windows handoff. [History](../history/M5-T346-core-machine-device-l3-convergence.md). |
| T345 | Closed direct-compilation strictness convergence: 251/305 direct commands are target-local strict; the 54 remaining commands have a complete 175-row ownership ledger and an exact 51-source residual production record with durable bounded admissions. No global flags, inherited-runtime rewrites, or false linked-dependency claims. [History](../history/M5-T345-direct-compilation-strictness-convergence.md). |
| T344 | Closed build-quality reproducibility: fresh configuration, 305-row truthful strict-compile matrix (130 retained strict/175 deferred), canonical 218-target current-gate registration, 53 historical fixture shapes, and strict-declaration uniqueness are mechanically verified. [History](../history/M5-T344-build-quality-reproducibility.md). |
| T343 | Closed the four-profile CPU program: one final ledger reconciles the 8086, 80186, 80286, and 80386DX execution, protected-state, delivery, task/paging/debug, and CPU-side coprocessor-interface boundaries. VME/PVI, persistent cache, x87 execution, timing/device, and Windows/product work remain explicit external candidates. [Closure ledger](../etc/evidence/t343-s1-four-profile-cross-closure.md). |

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
