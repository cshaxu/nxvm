# Project Status

## Current Work

M5 T348 S1 - Dual-8237A contract and gap ledger (Single-Session Mode).

## M5 T348 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner approved the Queue-ordered PC/AT device and L3 program, and instructed continuous holistic implementation on 2026-08-13. |
| Objective | Establish one complete, manual-led dual-8237A/PC-AT DMA behavior, ownership, port, channel, transfer, and timing ledger; reproduce every suspected construction defect before later implementation slices are admitted. |
| Non-goals | No speculative DMA rewrite, controller-duration constants, host DMA, generic bus scheduler, peripheral shortcut, Windows claim, or 8237A behavior not classified by the primary contract. |
| Reference Baseline | `d2e511a6` / closed T347; [DMA proposal](../proposals/m5-pcat-dma-completeness.md), [T346 timeline foundation](../etc/evidence/t346-s2-deterministic-timeline-foundation.md), and T347 storage history. |
| Candidate Proposal | [PC/AT 8237A DMA completeness](../proposals/m5-pcat-dma-completeness.md). |
| Files And ABI Surface | Expected: DMA focused audit/evidence and, only for reproduced probe infrastructure, owner-local tests; no production, public ABI, timeline, FDC, or ATA behavior change in S1. |
| Applicable Rules | Task Reading Set; source/research policy; architecture single owner/validation-publication invariant; coding owner-local fixtures; T346 timeline order; T347 storage-service boundary. |
| Verification | Produce a fixed port/channel/mode/ownership ledger, caller and write sweep, manual-reference map, existing-test coverage map, and focused reproducer results for each suspected defect; fresh configuration, governance, diff check, and current gate. |
| Expected Markers | Retain `M5:T269:S4:DMA-MODES:OK`, DMA channel/binding/authority markers, T346 arbitration/timeline markers, and FDC/ATA service markers; add a T348 S1 audit marker only if a bounded owner smoke is necessary. |
| Asset Needs | Published controller/manual reference only; no third-party source import, firmware, guest media, or reference-emulator dependency. |
| Reporting Requirements | Record exact PC/AT port/page/cascade matrix, every register/mode status, real versus accidental 16-bit-word layout distinction, caller/write/fault/publication boundaries, non-goal receiver, and a complete P1 delivery. |
| Stop Conditions | Stop for a need to import/copy third-party source, redefine global memory/port ownership, add another scheduler, change media/provider ABI, or repair an unbounded shared machine mechanism not isolated by the ledger. |
| Exit Criteria | Every relevant dual-8237A behavior is classified once with source/test evidence; all reproduced defects have bounded later-S owners; all unsupported modes have one named Queue/TODO transfer; no local code change is claimed as DMA completeness. |
| Original Owner Request | Make core-machine devices stable, comprehensive, and reliable at the selected deterministic L3 event-and-bus level before Windows testing. |
| Similar-Issue Sweep | Inspect all DMA ports, page mappings, primary/secondary selection, cascade paths, request bindings, transfer callbacks, memory writes, reset/finalization, machine timeline/arbitration calls, FDC DMA callers, tests, Queue, and TODO. |

## Current Technical Baseline

- **Current developer artifact:** T346 selects `vm-0-5-0346` /
  `build/output/nxvm_0_5_0346.exe`; its closure-build SHA-256 is
  `7715C8C290969A99CCC1137D2DEEC64421FF245A22BD35287A2AED0C75A8E260`.
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
| T347 | Closed retained PC/AT FDC and ATA PIO service timing: pending command/data states, reset/control/media cancellation, one FDC-then-ATA readiness owner, trace order, focused VM/DOS/HDD evidence, and 222 current-gate tests passed. [History](../history/M5-T347-storage-controller-service-timing.md). |
| T346 | Closed deterministic PC/AT L3 convergence: one reset-safe due-event owner, equal-tick `DMA -> PIT -> PIC -> FDC -> HDC -> RTC -> KBC -> VADP` ordering, copied host boundaries, exact storage/NMI/peripheral transfers, and evidence-backed Windows handoff. [History](../history/M5-T346-core-machine-device-l3-convergence.md). |
| T345 | Closed direct-compilation strictness convergence: 251/305 direct commands are target-local strict; the 54 remaining commands have a complete 175-row ownership ledger and an exact 51-source residual production record with durable bounded admissions. No global flags, inherited-runtime rewrites, or false linked-dependency claims. [History](../history/M5-T345-direct-compilation-strictness-convergence.md). |
| T344 | Closed build-quality reproducibility: fresh configuration, 305-row truthful strict-compile matrix (130 retained strict/175 deferred), canonical 218-target current-gate registration, 53 historical fixture shapes, and strict-declaration uniqueness are mechanically verified. [History](../history/M5-T344-build-quality-reproducibility.md). |
| T343 | Closed the four-profile CPU program: one final ledger reconciles the 8086, 80186, 80286, and 80386DX execution, protected-state, delivery, task/paging/debug, and CPU-side coprocessor-interface boundaries. VME/PVI, persistent cache, x87 execution, timing/device, and Windows/product work remain explicit external candidates. [Closure ledger](../etc/evidence/t343-s1-four-profile-cross-closure.md). |
| T342 | Closed the 80386DX profile ledger: T340 form and T341 state evidence reconcile every assigned row once, and the closure map now names the accepted state owners instead of stale future transfers. Only explicit post-80386, x87, timing/device, and Windows boundaries remain external. [Closure audit](../etc/evidence/t342-s1-80386dx-profile-closure-audit.md). |
| T341 | Closed the 80386DX system-state package: CR/DR/TR/table state, VM86/task/paging composition, and ordinary debug/vector-1 now have one audited owner/proof graph; only explicit later-CPU, x87, timing/device, and Windows boundaries transfer. [Closure audit](../etc/evidence/t341-s5-80386dx-system-state-closure-audit.md). |
| T340 | Closed the 80386DX width, prefix, FS/GS, and non-privileged integer-form package: S1 allocated the finite form/state ledger; S2/S3 reconciled the shared mechanisms and every assigned `0F` family; S4 confirmed accepted proof or one exact T341/external transfer for each row. [Closure audit](../etc/evidence/t340-s4-80386dx-form-closure-audit.md). |

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
