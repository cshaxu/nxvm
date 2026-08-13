# Project Status

## Current Work

**M5 T344 S1 - active.** Reconcile the T337 `#UD` inventory so a fresh GCC
configuration is reproducible without weakening the source-sensitive gate.

## M5 T344 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | The owner approved the full-tree audit report and its four ordered build-quality repairs on 2026-08-13. This packet admits the first Queue-ordered T344 S1 repair. |
| Objective | Give `core-machine-vm86-delivery-smoke` exactly one truthful T337 current-gate `#UD` disposition so a fresh out-of-tree GCC configure succeeds while the source-sensitive inventory remains enforced. |
| Non-goals | No CPU semantic, VM86 behavior, current-gate membership, test assertion, warning-policy, global compiler-flag, rule, or fixture change. Do not delete, bypass, or weaken the T337 inventory verifier. |
| Reference Baseline | `1b88740f` (`M5 T343 S1 P2`), [T344 baseline audit](../etc/evidence/t344-code-quality-baseline-audit.md), [T337 `#UD` evidence](../etc/evidence/t337-s1-real-ud-delivery.md), current CMake source, Queue, TODO, and the build-quality proposal. |
| Candidate Proposal | [Build-quality reproducibility](../proposals/m5-build-quality-reproducibility.md). |
| Files And ABI Surface | Expected: `CMakeLists.txt`, T344 evidence/history/Current only. No public ABI, production source, smoke source, target membership, or runtime artifact change. |
| Applicable Rules | `docs/rules/EXECUTION.md`: mechanism-owner sweep and complete P. `docs/rules/ARCHITECTURE.md`: one current-gate classification owner. `docs/rules/CODING.md`: no duplicate route. `docs/rules/DOCUMENT.md`: Current/history/evidence topology. |
| Verification | Configure a fresh disposable GCC/Ninja build directory; prove the specific target has exactly one inventory/disposition entry; build its target and run its exact current-gate CTest; run documentation governance and diff check. |
| Expected Markers | Fresh configure succeeds; T337 inventory still rejects an unclassified `#UD` owner; `current.core-machine-vm86-delivery-smoke` remains registered and passes. |
| Asset Needs | No external assets, firmware, guest media, or source import. |
| Reporting Requirements | Deliver one complete pushed P with the classification rationale, fresh configure result, exact smoke result, inventory sweep, and gate results. |
| Stop Conditions | Stop if the smoke actually requires source/CPU behavior change, a second T337 policy category, a broader inventory redesign, external source, or any warning-policy change. Return that issue to the coordinator rather than weakening the gate. |
| Exit Criteria | The fresh configure succeeds; the target has one truthful classification and no duplicate entry; its current-gate smoke passes; documentation governance/diff check pass; and the complete P is committed and pushed. |
| Original Owner Request | Audit the entire codebase for code quality, record all findings, and admit the four resulting quality improvements for implementation. |
| Similar-Issue Sweep | Inspect every target in `PROJECT_CURRENT_SMOKE_TARGETS` whose machine source contains `VCPUINS_EXCEPT_UD`, `_SetExcept_UD`, or `UndefinedOpcode`; verify each has exactly one T337 inventory and disposition path. |

## Current Technical Baseline

- **Current developer artifact:** T338 selected `vm-0-5-0338` /
  `build/output/nxvm_0_5_0338.exe`; the rebuilt developer output SHA-256 is
  `7D79417889821695DB4993DFEA5134B01E5B16D69007C20A6F1E3CBB8C75C05F`.
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
| T343 | Closed the four-profile CPU program: one final ledger reconciles the 8086, 80186, 80286, and 80386DX execution, protected-state, delivery, task/paging/debug, and CPU-side coprocessor-interface boundaries. VME/PVI, persistent cache, x87 execution, timing/device, and Windows/product work remain explicit external candidates. [Closure ledger](../etc/evidence/t343-s1-four-profile-cross-closure.md). |
| T342 | Closed the 80386DX profile ledger: T340 form and T341 state evidence reconcile every assigned row once, and the closure map now names the accepted state owners instead of stale future transfers. Only explicit post-80386, x87, timing/device, and Windows boundaries remain external. [Closure audit](../etc/evidence/t342-s1-80386dx-profile-closure-audit.md). |
| T341 | Closed the 80386DX system-state package: CR/DR/TR/table state, VM86/task/paging composition, and ordinary debug/vector-1 now have one audited owner/proof graph; only explicit later-CPU, x87, timing/device, and Windows boundaries transfer. [Closure audit](../etc/evidence/t341-s5-80386dx-system-state-closure-audit.md). |
| T340 | Closed the 80386DX width, prefix, FS/GS, and non-privileged integer-form package: S1 allocated the finite form/state ledger; S2/S3 reconciled the shared mechanisms and every assigned `0F` family; S4 confirmed accepted proof or one exact T341/external transfer for each row. [Closure audit](../etc/evidence/t340-s4-80386dx-form-closure-audit.md). |
| T339 | Closed the Queue-ordered 80286 descriptor-transfer package: S2--S6 proved table/system-word, selector/cache, protected entry, protected return, and TSS16 transition mechanisms; S7 reconciled the source graph, retained T328 `LOCK` policy, and exact T341--T342 transfers. [Closure audit](../etc/evidence/t339-s7-80286-descriptor-transfer-audit.md). |
| T338 | Closed the 8086/80186 profile baseline: S2 ALU/FLAGS/conditions, S3 inherited data/control/I/O, and S4 80186 extensions cover every S1 allocation; the retained T328 rule is the only legacy `LOCK` owner. The sole reproduced defect was four incorrect INS/OUTS 80386 guards, corrected to 80186. Protected, 80386DX, and x87 boundaries transfer explicitly. Artifact verification, governance, and 218/218 current-gate passed. [History](../history/M5-T338-8086-80186-profile-closure.md). |
| T337 | Closed shared CPU state and delivery foundations: vector-6 fault delivery/rollback, software and external origin composition, IRQ/NMI/TF priority, frame timing, inhibition, and post-commit task debug trap have one proof/owner. DR6/DR7 transfers to 80386DX; reset/shutdown/triple-fault remains bounded CPU/machine debt. [History](../history/M5-T337-shared-state-delivery.md). |
| T336 | Closed the four-profile CPU coverage audit: the primary/`0F` metadata and dispatch forms, real/protected/VM86 state rows, historic evidence, explicit 80386 exclusions, and open CPU debt now have a single indexed disposition. Every residual has one Queue, TODO, or external owner; the next candidate is shared state and delivery foundations. [History](../history/M5-T336-four-profile-coverage.md). |

## Recent Governance

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

- **M5 Td S74:** clarified that an executor reports a discovered issue while
  the coordinator alone revises the active packet/brief or admits later S
  work, and renamed the architecture status reference to `CURRENT.md`.
  Documentation self-test, combined governance check, and diff check passed.
  Td work has no runtime or artifact change.

- **M5 Td S73:** replaced ordinary/dual-session branches with one
  coordinator/executor role cycle. One session switches roles and cannot claim
  independent review; two sessions obtain independent coordinator review. P,
  Td/T closure, packet, stop, handoff, and role-authority constraints remain
  explicit. Documentation self-test, combined governance check, and diff check
  passed. Td work has no runtime or artifact change.
