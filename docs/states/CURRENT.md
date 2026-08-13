# Project Status

## Current Work

M5 T348 S4 - PC/AT DMA transaction, memory-to-memory, and consumer lifecycle reconciliation (Single-Session Mode).

## M5 T348 S4 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved continuous holistic PC/AT device/L3 implementation on 2026-08-13; accepted T348 S1--S3 evidence assigns transaction/lifecycle closure here. |
| Objective | Establish DMA validation-before-publication for ordinary and memory-to-memory transfers, complete memory-to-memory terminal/EOP/auto-init ownership, and reconcile reset/finalize/FDC/ATA/PIC/timeline visibility with the retained one-grant owner.  The default FDC firmware must treat the 8237A 64KiB address wrap as a hardware boundary: it must not depend on page-register carry and must route an INT 13h buffer-crossing sector through its owned reserved bounce path. |
| Non-goals | No physical DREQ/DACK/EOP waveform, compressed/READY duration, generic bus wait-state model, host DMA, new media command semantics, device policy, or Windows claim; electrical timing remains the later L3 bus-timing candidate. |
| Reference Baseline | `79ed1cb0` / accepted T348 S3; [T348 ledger](../etc/evidence/t348-s1-dual-8237a-gap-ledger.md), [S2 evidence](../etc/evidence/t348-s2-dma-port-page-layout.md), [S3 evidence](../etc/evidence/t348-s3-dma-request-cascade.md), and [DMA proposal](../proposals/m5-pcat-dma-completeness.md). |
| Candidate Proposal | [PC/AT 8237A DMA completeness](../proposals/m5-pcat-dma-completeness.md). |
| Files And ABI Surface | Expected: local DMA transaction/terminal implementation and owner smoke, potentially checked-memory query use, default-profile FDC firmware and its VM boundary proof, FDC/ATA/PIC/timeline evidence/index/Current; no raw provider layout, scheduler, host API, media format, or product ABI expansion. |
| Applicable Rules | Task Reading Set; architecture one validation-to-commit owner and no partial publication; coding shared owner-local transaction helper; Intel 8237A M2M/EOP/auto-init semantics; checked physical-memory routing; T346 deterministic `DMA -> PIT -> PIC -> FDC -> HDC` due-event boundary; T347 service owner. |
| Verification | Force ordinary device-to-memory write-route and memory-to-device read-route failures and M2M source/destination failures; require no provider callback, latch/counter/address/request/terminal publication before a failed preflight. Prove M2M channel-0 software initiation, terminal count and binding isolation, and any supported auto-init disposition. Prove reset/finalize clear transient request/service/EOP state while preserving valid binding ownership.  Invoke default-profile floppy INT 13h with a one-sector buffer that crosses a 64KiB DMA window; prove both destination fragments receive the sector, the reserved bounce region is not guest-visible conventional memory, and channel 2 uses no page carry. Re-run FDC/ATA/PIC/timeline focused owners and full current gate. |
| Expected Markers | Retain T269/T230/T348 S2/S3, FDC/DMA binding, T346 arbitration and T347 storage markers; add `M5:T348:S4:DMA-TRANSACTION-LIFECYCLE:OK`. |
| Asset Needs | Published Intel 8237A and IBM PC/AT documentation only; no source import, firmware, guest media, or host device. |
| Reporting Requirements | Record exact validation/commit sequence for every transfer direction, M2M terminal source, memory-route failure result, reset/finalize consumer sweep, retained electrical timing boundary, and one complete pushed P1. |
| Stop Conditions | Stop for a need to alter generic memory-route semantics, expose raw DMA/provider layout, change FDC/ATA command ABI, reduce reported conventional memory beyond the profile's existing reserved top 1KiB, create a scheduler, or model electrical/bus-cycle timing without separate L3 admission. |
| Exit Criteria | No failed physical route can publish a provider effect, latch, counter, address, request, terminal callback, or memory write; every adopted M2M terminal/auto-init state has one owner/proof; the default FDC firmware has no dependency on DMA page carry across its 64KiB boundary and has an owned tested crossing path; reset and due-event consumer visibility are deterministic; later electrical timing remains exact. |
| Original Owner Request | Make core-machine devices stable, comprehensive, and reliable at the selected deterministic L3 event-and-bus level before Windows testing. |
| Similar-Issue Sweep | Inspect every DMA physical read/write/query, latch/temp/counter/address/request/mask/TC/EOP mutation, ordinary/M2M terminal branch, reset/finalize, FDC and ATA DMA/PIO state, PIC observation, readiness and timeline caller, and all relevant owner smokes. |

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
| T348 S3 | Unified logical DMA request/grant selection: held DREQ, valid software request, fixed/rotating priority, true channel-4 cascade, controller disable isolation, and binding-scoped ordinary EOP. Memory-to-memory terminal/auto-init and transaction/reset work transfer to S4. [Evidence](../etc/evidence/t348-s3-dma-request-cascade.md). |
| T348 S2 | Repaired the shared dual-8237A page-wrap mechanism and secondary page-port controller selection; primary/secondary sparse register, control, page, byte/word transfer, and same-page wrap regressions are permanent. S3 retains request/priority/cascade/EOP. [Evidence](../etc/evidence/t348-s2-dma-port-page-layout.md). |
| T348 S1 | Accepted the dual-8237A contract/gap ledger: a reproduced shared page-boundary crossing defect, exact port/page/word/cascade/test map, and S2--S4 ownership plan. [Evidence](../etc/evidence/t348-s1-dual-8237a-gap-ledger.md). |
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
