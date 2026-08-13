# Project Status

## Current Work

**Active.** M5 T346 S6 reconciles the accepted device/timing program and
hands an evidence-backed PC/AT L3 boundary to the Windows 3.x readiness map.

## M5 T346 S6 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved completing T346 on 2026-08-13. This packet admits only the T346 closure audit, evidence reconciliation, current developer-artifact update, and Windows-readiness handoff. |
| Objective | Reconcile every S1 ledger family against S2--S5 implementation/evidence or one exact Queue/TODO receiver; state the achieved deterministic PC/AT L3 contract without a cycle-exact or Windows-installation claim; refresh the Windows readiness map with evidence-backed blockers. |
| Non-goals | No new device/controller, x87 execution, Windows guest media, installation/run attempt, renderer/host policy, arbitrary storage delay, generic bus wait-state, or CPU-timing implementation. No M5 milestone closure. |
| Reference Baseline | `30d8223c` / M5 T346 S5 P2, clean worktree. |
| Candidate Proposal | [M5 Core-Machine Device Parity And L3 Timing Convergence](../proposals/m5-core-machine-device-l3-convergence.md), S6. |
| Files And ABI Surface | Evidence/history/current capability/Windows proposal/Queue/TODO/roadmap/CMake current-artifact identity and local developer artifact only. No runtime interface, device ABI, or host boundary change. |
| Applicable Rules | Task Reading Set; execution T-level closure and actual-change review; documentation topology; current-artifact identity; architecture single guest-time owner and copied-host boundary; source/research policy. |
| Verification | Fresh configure; all retained S2--S5 focused markers and exact registrations; actual current artifact build/copy, SHA-256 and banner capture; documentation governance; diff check; full current-gate. |
| Expected Markers | Retained `M5:T346:S2:TIMELINE:OK`, `M5:T346:S3:ARBITRATION:OK`, `M5:T346:S4:RTC-STORAGE-READINESS:OK`, and `M5:T346:S5:INPUT-DISPLAY-TIMELINE:OK`; developer artifact `nxvm_0_5_0346.exe`. |
| Asset Needs | No external source, firmware, guest media, Windows binary, host event, renderer, thread, or wall-clock asset. |
| Reporting Requirements | Publish one indexed reconciliation table mapping every S1 family to accepted evidence or a unique receiver; state artifact SHA-256/source commit/banner; list Windows blockers and explicitly state that no Windows installation was run. |
| Stop Conditions | Stop for an unclassified S1 ledger family, conflicting owner/receiver, artifact build that changes product behavior, needed host/renderer/guest-media work, or a claim requiring a real Windows corpus. Transfer rather than invent evidence. |
| Exit Criteria | Every T346-in-scope family is evidence-backed under the stated L3 contract or transferred once with owner/risk/admission condition; S1/S2--S5 detail is merged into T346 history; Windows readiness proposal names only evidenced blockers; artifact identity is 0.5.0346 with recorded verification; Queue no longer presents closed T346 as a candidate. |
| Original Owner Request | Produce a holistic, polished PC/AT-class core-machine device/L3 plan suitable for Windows 3.x research, excluding x87 execution. |
| Similar-Issue Sweep | Re-read S1's 55-unit ledger; search device advance/refresh calls, timeline scheduling, host input/display crossings, all T346 evidence receivers, Queue, TODO, capability baseline, and Windows proposal. Classify each hit as accepted contract, explicit transfer, or stale claim corrected here. |

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
| T346 S5 | Accepted KBC/VADP timeline migration: a reset-safe peripheral callback completes same-tick order `DMA -> PIT -> PIC -> FDC -> HDC -> RTC -> KBC -> VADP`; input/display host crossings remain copied boundaries. [Evidence](../etc/evidence/t346-s5-input-display-timeline.md). |
| T345 | Closed direct-compilation strictness convergence: 251/305 direct commands are target-local strict; the 54 remaining commands have a complete 175-row ownership ledger and an exact 51-source residual production record with durable bounded admissions. No global flags, inherited-runtime rewrites, or false linked-dependency claims. [History](../history/M5-T345-direct-compilation-strictness-convergence.md). |
| T344 | Closed build-quality reproducibility: fresh configuration, 305-row truthful strict-compile matrix (130 retained strict/175 deferred), canonical 218-target current-gate registration, 53 historical fixture shapes, and strict-declaration uniqueness are mechanically verified. [History](../history/M5-T344-build-quality-reproducibility.md). |
| T343 | Closed the four-profile CPU program: one final ledger reconciles the 8086, 80186, 80286, and 80386DX execution, protected-state, delivery, task/paging/debug, and CPU-side coprocessor-interface boundaries. VME/PVI, persistent cache, x87 execution, timing/device, and Windows/product work remain explicit external candidates. [Closure ledger](../etc/evidence/t343-s1-four-profile-cross-closure.md). |
| T342 | Closed the 80386DX profile ledger: T340 form and T341 state evidence reconcile every assigned row once, and the closure map now names the accepted state owners instead of stale future transfers. Only explicit post-80386, x87, timing/device, and Windows boundaries remain external. [Closure audit](../etc/evidence/t342-s1-80386dx-profile-closure-audit.md). |
| T341 | Closed the 80386DX system-state package: CR/DR/TR/table state, VM86/task/paging composition, and ordinary debug/vector-1 now have one audited owner/proof graph; only explicit later-CPU, x87, timing/device, and Windows boundaries transfer. [Closure audit](../etc/evidence/t341-s5-80386dx-system-state-closure-audit.md). |
| T340 | Closed the 80386DX width, prefix, FS/GS, and non-privileged integer-form package: S1 allocated the finite form/state ledger; S2/S3 reconciled the shared mechanisms and every assigned `0F` family; S4 confirmed accepted proof or one exact T341/external transfer for each row. [Closure audit](../etc/evidence/t340-s4-80386dx-form-closure-audit.md). |
| T339 | Closed the Queue-ordered 80286 descriptor-transfer package: S2--S6 proved table/system-word, selector/cache, protected entry, protected return, and TSS16 transition mechanisms; S7 reconciled the source graph, retained T328 `LOCK` policy, and exact T341--T342 transfers. [Closure audit](../etc/evidence/t339-s7-80286-descriptor-transfer-audit.md). |
| T338 | Closed the 8086/80186 profile baseline: S2 ALU/FLAGS/conditions, S3 inherited data/control/I/O, and S4 80186 extensions cover every S1 allocation; the retained T328 rule is the only legacy `LOCK` owner. The sole reproduced defect was four incorrect INS/OUTS 80386 guards, corrected to 80186. Protected, 80386DX, and x87 boundaries transfer explicitly. Artifact verification, governance, and 218/218 current-gate passed. [History](../history/M5-T338-8086-80186-profile-closure.md). |

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
