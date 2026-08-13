# Project Status

## Current Work

**M5 T344 S2 - active.** Establish a truthful direct-compilation strictness
matrix for every production and current-gate target without a global warning
policy.

## M5 T344 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | The owner approved the T344 four-repair code-quality program on 2026-08-13 and directed its single-agent completion. S1 was accepted as `eb423b88`, which restored a fresh configuration and current-gate baseline without weakening either gate. This packet admits the Queue-ordered S2 coverage-matrix repair. |
| Objective | Derive a source-to-target strict-compilation matrix from actual GCC/Ninja direct compile commands for every configured production target and every current-gate executable; classify each direct source compilation as retained strict, newly strict, or deferred with a precise inherited/mixed ownership reason and admission condition. Add a narrow mechanical verifier and apply target-local strict flags only to proven repository-owned targets. Recover the three stale protected/external-delivery smoke owners exposed by the fresh strict rebuild: their external-event rows must not also claim TF debug-trap priority, which remains owned by the accepted T341 debug smoke. |
| Non-goals | No global `-Werror`, no claim that a linked dependency is strictly compiled, no warning suppression or diagnostic weakening, no inherited executor warning cleanup, no xasm/debug redesign, no CPU semantic change, and no fixture extraction. The baseline repair may change only the three affected owner tests and their evidence; it must not alter `ExecInt`, debug scheduling, or delivery precedence. A target that cannot safely adopt strict flags remains an explicit deferred matrix row, never an implicit exemption. |
| Reference Baseline | `eb423b88` (`M5 T344 S1 P1`), [T344 baseline audit](../etc/evidence/t344-code-quality-baseline-audit.md), the build-quality proposal, `CMakeLists.txt`, generated Ninja commands, Queue, and TODO. |
| Candidate Proposal | [Build-quality reproducibility](../proposals/m5-build-quality-reproducibility.md). |
| Files And ABI Surface | Expected: `CMakeLists.txt`, a CMake-only verifier, T344 evidence/history/Current, and only proven repository-owned target-local compile-option declarations; plus the three stale current-gate owner tests `core_machine_protected_16_external_s4_smoke.c`, `core_machine_protected_16_outer_s5_smoke.c`, and `core_machine_hardware_delivery_s3_smoke.c`. No public ABI, target membership, product behavior, or production-source implementation change. |
| Applicable Rules | `docs/rules/EXECUTION.md`: complete source-scope and similar-issue sweep. `docs/rules/ARCHITECTURE.md`: one build-quality ownership path. `docs/rules/CODING.md`: project-owned strictness without duplicate facade. `docs/rules/DOCUMENT.md`: Current/history/evidence topology. |
| Verification | Fresh GCC/Ninja configure; generate the matrix from direct Ninja commands; mechanically verify every matrix target/source row and its declared classification; build every strict target; inspect representative deferred direct commands to prove they are not misreported; prove the three repaired external-delivery owners pass without TF and that the retained T341 TF-before-PIC owner still passes; run documentation governance, diff check, and full current-gate. |
| Expected Markers | Every configured production/current-gate target has one direct-compilation disposition; every retained/new strict source command contains `-Wall -Wextra -Wpedantic -Werror`; no deferred row claims linked-dependency coverage; full gate remains green. |
| Asset Needs | No external assets, firmware, guest media, or source import. |
| Reporting Requirements | Deliver one complete pushed P with the generated matrix, target/source count, each deferred reason/admission condition, actual direct-command verifier result, newly strict target rationale, and gate results. |
| Stop Conditions | Stop if a proposed strict extension needs warning suppression, a global policy, production semantic change, inherited warning cleanup, unsupported generator parsing, or an unclassifiable target/source ownership boundary. Record that exact blocker and transfer rather than falsely classify it. |
| Exit Criteria | Every in-scope production/current-gate target has one evidence-backed direct-compilation disposition; strict rows are mechanically verified from actual commands and build; all newly strict targets are proven repository-owned; every deferred row has an exact reason and admission condition; governance/diff/full current-gate pass; and the complete P is committed and pushed. |
| Original Owner Request | Audit the entire codebase for code quality, record all findings, and admit the four resulting quality improvements for implementation. |
| Similar-Issue Sweep | Enumerate all configured non-imported production targets and every `PROJECT_CURRENT_SMOKE_TARGETS` executable. For each direct C source compile command, record strict flags and source ownership; search all target-local strict declarations for omissions, duplicates, and claims based only on linked libraries. |

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
| T344 S1 | Restored fresh GCC configuration by giving VM86 delivery one explicit non-real T337 `#UD` disposition; repaired shared TF post-interrupt trap scheduling and stale RF smoke expectations; exact regressions and 218/218 current-gate passed. [Evidence](../etc/evidence/t344-code-quality-baseline-audit.md). |
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
