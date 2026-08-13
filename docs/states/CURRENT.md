# Project Status

## Current Work

**Active.** M5 T346 S5 migrates guest input/display clock consumption to the
deterministic timeline and reconfirms the copied host-presentation boundary.

## M5 T346 S5 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved continued completion of T346 on 2026-08-13; this packet admits only guest KBC/VADP clock migration and host-boundary reconciliation. |
| Objective | Move KBC/AUX response/typematic and VADP guest display progression from the post-retirement batch scheduler to one reset-safe due-time peripheral callback; prove deterministic ordering and explicitly retain host presentation/input cadence outside guest time. |
| Non-goals | No new AUX protocol, speaker/PPI, VGA/VBE, display-mode breadth, renderer, host input polling, thread, wall-clock, or platform policy change. No new host presentation cadence claim. |
| Reference Baseline | `4f27846e` / M5 T346 S4 P2, clean worktree. |
| Candidate Proposal | [M5 Core-Machine Device Parity And L3 Timing Convergence](../proposals/m5-core-machine-device-l3-convergence.md), S5. |
| Files And ABI Surface | Expected: machine/trace private ownership, focused core smoke/CMake/evidence/CURRENT/history/TODO only. No platform or host-facing ABI. |
| Applicable Rules | Task Reading Set; execution and actual-change review; architecture single guest-time owner and copied-host-boundary invariant; C11/project-type/strict owner-test rules; documentation topology. |
| Verification | Fresh configure; focused S5 trace smoke; retained KBC/AUX/VADP/display, S2/S3/S4 timing smokes; exact registration; governance/diff checks; full current-gate. |
| Expected Markers | New `M5:T346:S5:INPUT-DISPLAY-TIMELINE:OK`; retained markers remain unchanged. |
| Asset Needs | No external source, firmware, guest media, host event, renderer, thread, or wall-clock asset. |
| Reporting Requirements | Record old scheduler paths, new callback order/reset behavior, and every host presentation/input crossing. Commit and push the complete P only. |
| Stop Conditions | Stop for platform/renderer/host-input policy change, a needed guest device semantic change beyond clock ownership, or an unclassified alternate KBC/VADP advance path. Do not turn a host refresh into guest time. |
| Exit Criteria | KBC and VADP each have one timeline-owned machine advance path with reset replacement and trace proof; all host crossings are copied/snapshot boundaries or transfer entries; no batch scheduler call remains for the migrated guest domains. |
| Original Owner Request | Produce a holistic, polished PC/AT-class core-machine device/L3 plan suitable for Windows 3.x research, excluding x87 execution. |
| Similar-Issue Sweep | Search all KBC/VADP advance calls, guest input submission paths, display snapshot/presentation publication paths, and scheduler callbacks. Classify every hit as migrated, intentionally standalone test/controller use, host boundary, or TODO transfer. |

## Current Technical Baseline

- **Current developer artifact:** T344 selects `vm-0-5-0344` /
  `build/output/nxvm_0_5_0344.exe`; the rebuilt developer output SHA-256 is
  `84674E5B32F3CD5C21834F23277E46BEC86156958878D4A6DD5223D325BD74A2`.
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
| T346 S4 | Accepted RTC/FDC media-observation readiness: an independent RTC clock and reset-safe due-time callback establish `DMA -> PIT -> PIC -> FDC -> HDC -> RTC` ordering. Synchronous FDC/ATA service and absent PC/AT NMI sources transfer precisely; no arbitrary physical delay claim. [Evidence](../etc/evidence/t346-s4-rtc-storage-readiness.md). |
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
