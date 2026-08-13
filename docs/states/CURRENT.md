# Project Status

## Current Work

M5 T347 S3 - ATA PIO deferred command and sector-completion service
(Single-Session Mode).

## M5 T347 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; single-session coordinator and executor roles are performed sequentially in this session. |
| Admission And Approval | Owner approved the post-T346 PC/AT device-completeness sequence on 2026-08-13; accepted T347 S1/S2 establish the common lifecycle and FDC precedent. |
| Objective | Move every retained ATA PIO command acceptance and sector/final completion publication from its issuing command/data port transaction to named HDC-local pending states advanced only by the existing readiness timeline owner. |
| Non-goals | No ATA command-set expansion, LBA48, ATAPI, bus mastering, cache policy, host blocking I/O, generic scheduler/DMA/media-provider redesign, Windows media, or physical seek/rotation timing claim. |
| Reference Baseline | `be37c125` / T347 S2 governance closure; [storage lifecycle evidence](../etc/evidence/t347-s1-storage-service-lifecycle.md) and [FDC service evidence](../etc/evidence/t347-s2-fdc-deferred-service.md). |
| Candidate Proposal | [PC/AT storage controller service timing](../proposals/m5-storage-controller-service-timing.md). |
| Files And ABI Surface | Expected: `src/core/machine/{machine,hdc,trace_interface}.*`, retained ATA/FDC timeline tests, evidence, and Current; no controller timeline pointer, media-provider ABI, host ABI, or FDC behavior change. |
| Applicable Rules | Task Reading Set; architecture one owner/one production path; coding owner-local state helpers; T346 due-event order; T347 lifecycle contract; source policy. |
| Verification | Prove command-pending BSY/no-DRQ/no-IRQ state; next-readiness handling of retained READ/WRITE/IDENTIFY and failure paths; each final data-sector boundary deferred before next DRQ/final IRQ; nIEN/status/alternate-status/SRST cancellation; CHS/LBA28 and media-failure behavior; later-arbitration visibility; retained HDC/ATA/DMA/PIC/timeline tests, fresh configuration, governance, diff check, and current gate. |
| Expected Markers | New `M5:T347:S3:ATA-SERVICE:OK` plus retained ATA PIO/DOS, HDC/nIEN, PIC lifecycle, FDC service, and T346 timeline/arbitration/readiness markers. |
| Asset Needs | No guest media, firmware, or third-party code import. Existing ATA PIO register/status reference is research-only. |
| Reporting Requirements | Record command-to-state and data-sector tables, complete command/data/control/read caller sweep, observable status/DRQ/IRQ boundaries, reset/cancellation results, actual-change review, and one pushed complete P delivery. |
| Stop Conditions | Stop for a required generic scheduler/timeline capacity change, DMA/media-provider ABI change, unbounded buffer, CPU/interrupt-delivery change, required command outside the retained surface, or primary ATA contract conflict requiring a different service owner. |
| Exit Criteria | Every retained ATA PIO command and sector/final-completion path enters/leaves named local pending states; command and final-data transactions cannot publish next-DRQ/final-IRQ completion; reset/nIEN/media failure cancel pending work; status versus alternate-status behavior remains exact; FDC stays unchanged and ATA makes no mechanical-duration claim. |
| Original Owner Request | Build high-value PC/AT device completeness and L3 timing holistically before Windows-startup testing. |
| Similar-Issue Sweep | Inspect every HDC command/data/task-file/status/control/nIEN/SRST/media/reset/finalization/refresh path and the machine readiness caller; reject direct command/data-side DRQ/IRQ completion outside local service advance. |

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
| T347 S2 | Accepted FDC deferred command/completion lifecycle: the existing readiness owner alone advances command and final-transfer publication; pending MSR, DMA/non-DMA, DOR/reset/media cancellation, VM FDC, and T242 read-track proof passed. [Evidence](../etc/evidence/t347-s2-fdc-deferred-service.md). |
| T347 S1 | Accepted storage-service lifecycle inventory: retained FDC/ATA command, data, reset, cancellation, media, DRQ/DMA/IRQ, and readiness paths now have one shared visibility rule and an exact S2--S4 migration sequence. [Evidence](../etc/evidence/t347-s1-storage-service-lifecycle.md). |
| T346 | Closed deterministic PC/AT L3 convergence: one reset-safe due-event owner, equal-tick `DMA -> PIT -> PIC -> FDC -> HDC -> RTC -> KBC -> VADP` ordering, copied host boundaries, exact storage/NMI/peripheral transfers, and evidence-backed Windows handoff. [History](../history/M5-T346-core-machine-device-l3-convergence.md). |
| T345 | Closed direct-compilation strictness convergence: 251/305 direct commands are target-local strict; the 54 remaining commands have a complete 175-row ownership ledger and an exact 51-source residual production record with durable bounded admissions. No global flags, inherited-runtime rewrites, or false linked-dependency claims. [History](../history/M5-T345-direct-compilation-strictness-convergence.md). |
| T344 | Closed build-quality reproducibility: fresh configuration, 305-row truthful strict-compile matrix (130 retained strict/175 deferred), canonical 218-target current-gate registration, 53 historical fixture shapes, and strict-declaration uniqueness are mechanically verified. [History](../history/M5-T344-build-quality-reproducibility.md). |
| T343 | Closed the four-profile CPU program: one final ledger reconciles the 8086, 80186, 80286, and 80386DX execution, protected-state, delivery, task/paging/debug, and CPU-side coprocessor-interface boundaries. VME/PVI, persistent cache, x87 execution, timing/device, and Windows/product work remain explicit external candidates. [Closure ledger](../etc/evidence/t343-s1-four-profile-cross-closure.md). |
| T342 | Closed the 80386DX profile ledger: T340 form and T341 state evidence reconcile every assigned row once, and the closure map now names the accepted state owners instead of stale future transfers. Only explicit post-80386, x87, timing/device, and Windows boundaries remain external. [Closure audit](../etc/evidence/t342-s1-80386dx-profile-closure-audit.md). |
| T341 | Closed the 80386DX system-state package: CR/DR/TR/table state, VM86/task/paging composition, and ordinary debug/vector-1 now have one audited owner/proof graph; only explicit later-CPU, x87, timing/device, and Windows boundaries transfer. [Closure audit](../etc/evidence/t341-s5-80386dx-system-state-closure-audit.md). |
| T340 | Closed the 80386DX width, prefix, FS/GS, and non-privileged integer-form package: S1 allocated the finite form/state ledger; S2/S3 reconciled the shared mechanisms and every assigned `0F` family; S4 confirmed accepted proof or one exact T341/external transfer for each row. [Closure audit](../etc/evidence/t340-s4-80386dx-form-closure-audit.md). |
| T339 | Closed the Queue-ordered 80286 descriptor-transfer package: S2--S6 proved table/system-word, selector/cache, protected entry, protected return, and TSS16 transition mechanisms; S7 reconciled the source graph, retained T328 `LOCK` policy, and exact T341--T342 transfers. [Closure audit](../etc/evidence/t339-s7-80286-descriptor-transfer-audit.md). |

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
