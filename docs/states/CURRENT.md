# Project Status

## Current Work

**Active.** M5 T331 S1 audits and converges the real-mode final exception
delivery construction in single-session coordinator/executor mode.

## M5 T331 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New; T331 begins from the first ordered M5 Queue candidate and S1 is the first implementation subtask. |
| Admission And Approval | Owner approved single-session completion of T331 on 2026-08-12. This S admits only the real-mode `ExecFinal` final-delivery construction and its direct evidence. |
| Objective | Establish the complete real-mode final-exception delivery matrix for the currently evidenced `#DE`, `#MF`, `#BR`, `#NM`, and `#GP` producers; replace equivalent duplicated final-delivery construction in `ExecFinal` with one private plan while preserving each Intel vector, frame width, restart, diagnostic, and terminal behavior. Real-mode `#PF` transfers because no current real producer evidence exists. |
| Non-goals | Do not redesign `_ser_int_real`, protected-mode delivery, `IRET`, PIC/NMI policy, real-mode `#UD`, reset/triple-fault policy, VME/PVI, x87 execution, public ABI, or `docs/rules/*`. |
| Reference Baseline | `83338834` / `vm-0-5-0330`; the current developer artifact remains T330 until T331 produces its verified artifact. |
| Candidate Proposal | [Holistic execution-path architecture audit and remediation plan](../proposals/m5-holistic-execution-path-audit.md), constrained by the [M5 80386DX admission policy](../proposals/m5-80386dx-candidate-policy.md). |
| Files And ABI Surface | Expected: `src/core/machine/cpu_instructions.c`, focused owner test(s), CMake registration only if a new test is needed, `docs/etc/evidence/`, `docs/history/`, and this packet. No public interface or product ABI change. |
| Applicable Rules | `docs/rules/EXECUTION.md`, `docs/rules/DOCUMENT.md`, `docs/rules/ARCHITECTURE.md`, `docs/rules/CODING.md`, `docs/design/ARCHITECTURE.md`, and `docs/design/CODING.md`; only a standalone Td may modify rules. |
| Verification | Audit every `ExecFinal` exception-mask branch and direct final-delivery caller; run focused producer/vector/frame/handler regressions for each admitted real path, fresh GCC configuration, applicable static verifier, documentation governance, diff check, current-gate CTest, and T331 developer-artifact build/hash. |
| Expected Markers | Existing focused markers remain valid; any new owner smoke emits `M5:T331:S1:REAL-EXCEPTION-FINAL:OK`. |
| Asset Needs | No guest media, firmware, external source, or runtime asset. Intel 80386 architecture documentation is a requirements reference only. |
| Reporting Requirements | In this single-session run, first record the coordinator brief, then executor-side contract and actual-change review before implementation P1; report only a pushed complete P, a reproducible material blocker, or a packet-revision need. |
| Stop Conditions | Stop for a required change to `_ser_int_real`, protected serializer, shared memory/descriptor/stack helper, external/public ABI, or an Intel outcome contradiction not resolvable within final-delivery construction; classify and transfer it rather than silently widening scope. |
| Exit Criteria | One evidence-backed real-mode delivery plan owns all currently evidenced masks; no duplicate plan-equivalent tail remains; each in-scope vector has success and failed-delivery/terminal classification plus producer/frame/restart proof; all retained gates pass; real-mode `#PF` and every excluded path have a named transfer; T331 artifact and SHA-256 are recorded. |
| Original Owner Request | Thoroughly clean up patch-on-patch construction consequences such as repeated construction, multiple mergeable paths, and nesting, using a top-down design. |
| Similar-Issue Sweep | Search all `ExecFinal` exception classification/delivery branches and all `_e_except_n` callers; classify protected, real, and deferred paths. Mechanically prevent reintroduction of a direct real-mode final-delivery bypass outside the private plan where feasible. |

## Current Technical Baseline

- **Current developer artifact:** T330 selects `vm-0-5-0330` /
  `build/output/nxvm_0_5_0330.exe`; commit `f8116f99` SHA-256 is
  `926D0F4A2CDB48367C522AD799FF4E9C2DE7AD6BF6C75EDB7889E2D8B9B62CAA`.
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
| T330 | Closed the whole-codebase construction-drift package: one task-transition constructor, FDD/HDD backing/create atomicity, CALL-gate dual-fault preflight convergence, and mandatory CPU state-machine mechanism matrices before P1. Intel-required layouts remain explicit; T330 artifact, focused regressions, 211/211 current-gate, and governance evidence are in [history](../history/M5-T330-width-path-convergence.md). |
| T329 | Closed the bounded Intel 80286/80386 protected task-transition state machine: 16/32-bit direct and task-gate entry, nested CALL/IRET state, incoming LDT images, source-CR3 preflight/incoming-CR3 commit, and TSS post-switch `#DB`. S7 proves target-page fetch, target-TSS `#PF` atomicity, and a target-state restart frame; the 0329 artifact and 211/211 gate result are in [history](../history/M5-T329-task-transition-state-machine.md). |
| T328 | Closed the historical LOCK-prefix legality matrix: 8086/80186 retain transparent valid-next-instruction semantics; 80286 adds protected `CPL <= IOPL`; retained 80386 memory-whitelist behavior stays intact. S2 reconciled the current closure map and ordinary matrix, removing the stale Deferred/TODO transfer without changing the user-owned Queue edit. Register, memory, REP, I/O, #GP frame, strict compile, artifact, and 211/211 gate evidence are in [history](../history/M5-T328-legacy-lock-legality.md). |
| T327 | Closed the current/specialized-gate reconciliation: fast smoke no longer builds classified media targets, and generated CTest/Ninja evidence now proves the full 210 = 15 media + 195 non-media partition, both developer roots, all specialized verifiers, and the aggregate's two roots. [History](../history/M5-T327-current-gate-reconciliation.md). |
| T326 | Closed ordinary protected-mode invalid-opcode delivery: `#UD` now reaches IDT vector 6 with a restartable three-dword no-error-code frame, while the explicit error-code classifier retains vectors 8/10--14/17. Four producer classes, invalid-gate containment, retained VM86 and 210/210 current-gate pass. Real-mode IVT migration transfers. [History](../history/M5-T326-protected-invalid-opcode-delivery.md). |
| T325 | Closed the CPU-native Intel 80386DX paging/translation package: CR0, CR2/CR3, 4-KiB PDE/PTE, U/S/R/W, A/D, cross-page atomicity, delivered `#PF`, no-persistent-cache behavior, and pre-486 `INVLPG #UD` are reconciled. Protected `#UD` delivery, task/VM86 paging, and persistent TLB/TR6/TR7 state transfer explicitly. [History](../history/M5-T325-80386dx-paging-translation.md). |
| T324 | Closed current-test/specialized-gate separation: `run-current-smokes` now runs only the full 209-test CTest smoke selection, `run-current-fast-smokes` retains the 194-test non-media selection, and `verify-current-specialized-gates` owns 46 named verifiers plus a mechanical target-graph check. `current-gates-gcc` composes exactly both roots; all layer baselines and evidence are retained in [T324 history](../history/M5-T324-current-gate-separation.md). |
| T323 | Closed the bounded 80386DX non-task, non-VM86 protection/privilege-transfer composition: direct far transfer, loaded segment rights, 16-bit same/outer gate entry, outer IRET, and parameterized 16-bit call gates now join retained selector, 32-bit, and outer-RETF evidence. The sole S7 serializer correction preflights/copies parameter words. Task/LDT/debug/VM86, paging, legacy LOCK, and x87 retain named boundaries; the 0323 artifact SHA-256 and 209/209 gate result are in the [closure audit](../etc/evidence/t323-protection-privilege-closure-audit.md). |

## Recent Governance

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

- **M5 Td S72:** aligned executor-facing governance: the coordinator creates
  active packets; executor findings are reported for coordinator triage; every
  numbered T packet must link its candidate proposal; and Td P1 is complete
  without a later governance P. Positive and missing-proposal negative
  self-tests, the combined governance check, and diff check passed. Td work
  has no runtime or artifact change.

- **M5 Td S71:** adopted the shared `states/` and `proposals/` topology:
  migrated Current, Queue, and TODO; extracted every Queue candidate detail to
  a linked proposal; updated current links and the topology/self-test verifier;
  and made execution the single authority for proposal admission, task
  numbering, history naming, retention, and withdrawal. Only standalone Td
  work may modify `docs/rules/`. Documentation self-test, combined governance
  check, and diff check passed. Td work has no runtime or artifact change.

- **M5 Td S70:** restored concise purpose-specific reporting and the
  active-contract/compact-Status boundary after later commits had superseded
  them; removed the Queue candidate already closed as T324; recorded bounded
  holistic-execution and task-transition debt candidates with evidence; and
  refreshed the retained governance summary. Documentation default/self-test
  and diff check passed. Td work has no runtime or artifact change.

- **M5 Td S68:** reconciled open numeric-task progress with identifier
  allocation: commit history proves used S/P records, Status structural rows
  determine task closure, and the verifier permits only the exact next S of the
  latest retained-open task under `Continuation`. Added positive and
  skipped/foreign/missing-progress/new-task negative self-tests. Documentation
  self-test, default check, and diff check passed. Td work has no runtime or
  artifact change.

- **M5 Td S67:** defined the two complementary 80386DX completion views in
  goal, roadmap, Queue, and an indexed supporting closure map; retained the
  T316 form matrix as horizontal evidence, and recorded VME/PVI, x87, and
  legacy LOCK boundaries without adding project-specific content to global
  rules. Documentation and state checks plus diff check passed. Td work has no
  runtime or artifact change.

- **M5 Td S66:** replaced all-documents startup reading with a role- and
  change-triggered reading set; aligned implementation and governance P
  lifecycle in both execution modes; clarified owner approval, coordinator and
  executor reporting, S-brief authority, corrective re-admission, and Status
  retention; added the `Reporting Requirements` packet field; and split the
  governance verifier into diagnostic documentation and state scopes while
  retaining its combined closure gate. Documentation and state checks, combined
  check, self-test, and diff check passed. Td work has no runtime or artifact
  change.
