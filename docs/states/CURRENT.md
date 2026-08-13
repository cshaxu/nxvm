# Project Status

## Current Work

**Active.** M5 T345 S2 promotes only the pure project-owned current-gate test
targets identified by S1 to target-local strict GCC compilation.

## M5 T345 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved 2026-08-13 the admitted T345 strictness-convergence task and its holistic implementation. S1 evidence bounds this continuation to pure project-owned test targets only. |
| Objective | Promote every current-gate executable whose direct C sources are exclusively project-owned `tests/` sources to target-local `-Wall -Wextra -Wpedantic -Werror`, remediate its direct warnings, and prove actual Ninja coverage. |
| Non-goals | Do not promote a target that directly compiles production source, add a global flag, suppress a warning, change runtime behavior, alter fixture architecture, repair inherited/mixed production, or modify docs/rules. |
| Reference Baseline | `e5140f39`; T345 S1 fixed 121 owner-test rows, of which 118 targets are pure and 3 owner-test rows share targets with six embedded production rows. The pure cohort has 18 warnings in 11 sources. |
| Candidate Proposal | [M5 direct-compilation strictness convergence](../proposals/m5-direct-compilation-strictness-convergence.md), S2. |
| Files And ABI Surface | CMake target-local options and T345 verifier/evidence/history/state records; the 11 warning-owning project test sources only if required. No public or runtime ABI surface. |
| Applicable Rules | Task Reading Set; Execution lifecycle, similar-issue sweep, and closure review; Coding test boundaries and project vocabulary; Documentation authority boundaries. Architecture runtime invariants are preserved because no production behavior changes. |
| Verification | Fresh GCC configure; exact pure-cohort inventory; actual Ninja command verifier proving all four flags on every promoted direct test source and no promotion of mixed targets; focused affected smokes; specialized gates; documentation governance; diff check; full current-gate. |
| Expected Markers | A mechanical S2 verifier reports the exact promoted target/source count and zero mixed-target promotions; affected smoke markers and current-gate pass. |
| Asset Needs | No external source, firmware, guest media, or protected asset. |
| Reporting Requirements | Record promoted targets/sources, every repaired warning class/source, actual command proof, and the three owner-test rows intentionally retained with their embedded-production targets. Report any warning requiring test semantic or shared-fixture change. |
| Stop Conditions | Stop if a pure target contains an unclassified non-test direct source, a warning repair changes test/runtime semantics, or a target-local option reaches embedded production source. Transfer rather than weaken `-Werror`. |
| Exit Criteria | Every S1-proven pure owner-test target is directly strict and warning-clean; all 18 baseline warnings are resolved without suppression; the 3 mixed owner-test rows remain unpromoted and are explicitly carried into S3; required gates pass. |
| Original Owner Request | Holistically consume T344's deferred strict-compilation surface, rather than leaving 175 untracked rows or repairing warnings one at a time. |
| Similar-Issue Sweep | Recompute the entire T345 owner-test target partition from actual target sources. Reject a pure/mixed mismatch and inspect every baseline warning, not only the first failing smoke. |

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
| T345 S1 | Classified all 175 T344 deferred direct source/target rows: 121 project-owned test, 6 embedded production test, 1 type foundation, and 47 mixed/inherited production. Actual Ninja-command warning baseline records 225 warnings; exact non-current request-bridge test drift transferred to TODO. [Evidence](../etc/evidence/t345-s1-direct-compilation-ownership.md). |
| T344 | Closed build-quality reproducibility: fresh configuration, 305-row truthful strict-compile matrix (130 retained strict/175 deferred), canonical 218-target current-gate registration, 53 historical fixture shapes, and strict-declaration uniqueness are mechanically verified. [History](../history/M5-T344-build-quality-reproducibility.md). |
| T343 | Closed the four-profile CPU program: one final ledger reconciles the 8086, 80186, 80286, and 80386DX execution, protected-state, delivery, task/paging/debug, and CPU-side coprocessor-interface boundaries. VME/PVI, persistent cache, x87 execution, timing/device, and Windows/product work remain explicit external candidates. [Closure ledger](../etc/evidence/t343-s1-four-profile-cross-closure.md). |
| T342 | Closed the 80386DX profile ledger: T340 form and T341 state evidence reconcile every assigned row once, and the closure map now names the accepted state owners instead of stale future transfers. Only explicit post-80386, x87, timing/device, and Windows boundaries remain external. [Closure audit](../etc/evidence/t342-s1-80386dx-profile-closure-audit.md). |
| T341 | Closed the 80386DX system-state package: CR/DR/TR/table state, VM86/task/paging composition, and ordinary debug/vector-1 now have one audited owner/proof graph; only explicit later-CPU, x87, timing/device, and Windows boundaries transfer. [Closure audit](../etc/evidence/t341-s5-80386dx-system-state-closure-audit.md). |
| T340 | Closed the 80386DX width, prefix, FS/GS, and non-privileged integer-form package: S1 allocated the finite form/state ledger; S2/S3 reconciled the shared mechanisms and every assigned `0F` family; S4 confirmed accepted proof or one exact T341/external transfer for each row. [Closure audit](../etc/evidence/t340-s4-80386dx-form-closure-audit.md). |
| T339 | Closed the Queue-ordered 80286 descriptor-transfer package: S2--S6 proved table/system-word, selector/cache, protected entry, protected return, and TSS16 transition mechanisms; S7 reconciled the source graph, retained T328 `LOCK` policy, and exact T341--T342 transfers. [Closure audit](../etc/evidence/t339-s7-80286-descriptor-transfer-audit.md). |
| T338 | Closed the 8086/80186 profile baseline: S2 ALU/FLAGS/conditions, S3 inherited data/control/I/O, and S4 80186 extensions cover every S1 allocation; the retained T328 rule is the only legacy `LOCK` owner. The sole reproduced defect was four incorrect INS/OUTS 80386 guards, corrected to 80186. Protected, 80386DX, and x87 boundaries transfer explicitly. Artifact verification, governance, and 218/218 current-gate passed. [History](../history/M5-T338-8086-80186-profile-closure.md). |
| T337 | Closed shared CPU state and delivery foundations: vector-6 fault delivery/rollback, software and external origin composition, IRQ/NMI/TF priority, frame timing, inhibition, and post-commit task debug trap have one proof/owner. DR6/DR7 transfers to 80386DX; reset/shutdown/triple-fault remains bounded CPU/machine debt. [History](../history/M5-T337-shared-state-delivery.md). |

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
