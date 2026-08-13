# Project Status

## Current Work

**M5 T333 S3 - active.** Prove retained interactive-input failure behavior and
close T333. The approved candidate is
[the P1 interactive input failure contract](../proposals/m5-interactive-input-failure-contract.md).

## M5 T333 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner-approved T333 scope on 2026-08-12. Coordinator accepted S2 P1 (`2a59ab52`) after reviewing both owner-local repairs and their successful Console lifecycle smoke. This S admits only deterministic failure proof and task closure. |
| Objective | Prove EOF does not parse, execute, scan, or mutate through VM Console/debugger prompt classes; prove both argument allocations prevent parser entry on failure; and close the task with applicable gate evidence. |
| Non-goals | No product API/ABI, command-UX, xasm API, generic input framework, process-global allocator hook, guest-execution, or session-selection change. |
| Reference Baseline | `2a59ab52` (`M5 T333 S2 P1: converge interactive input failure handling`); current artifact remains `vm-0-5-0332` / `build/output/nxvm_0_5_0332.exe`. |
| Candidate Proposal | [M5 interactive input failure contract](../proposals/m5-interactive-input-failure-contract.md). |
| Files And ABI Surface | Add one focused test source and CMake target. Its two direct production source copies may redirect `STD_MALLOC` only at their own test compilation to a test-owned allocator; normal libraries, headers, and ABI do not change. Update packet/evidence and task closure record. |
| Applicable Rules | `docs/rules/EXECUTION.md`: lifecycle, mechanism-defect owner/variants/commit boundary, actual-change audit. `docs/rules/CODING.md`: owner-local repair and no broad framework. `docs/rules/ARCHITECTURE.md`: one production path and no mutable public internals. `docs/rules/DOCUMENT.md`: task closure and evidence ownership. |
| Verification | Build and execute the focused smoke; test Console EOF/reuse and allocation failure, debugger main EOF/reuse and allocation failure, plus assembly/single-value/register nested EOF without mutation; run documentation governance, static reader sweep, and applicable current gate. |
| Expected Markers | `M5:T333:S3:INTERACTIVE-INPUT:OK`; no parser/execute mutation after failure; `arguments == STD_NULL` after every return; normal target builds use normal `STD_MALLOC`. |
| Asset Needs | None. No source, firmware, guest media, third-party code, or Microsoft material. |
| Reporting Requirements | Executor reports test-boundary confirmation, then concise commit and closure evidence. Coordinator independently reviews the test source, source-specific allocation redirection, CMake target, changed production source, and all task documents before acceptance. |
| Stop Conditions | Stop if the focused test requires a public test API, a process-global allocator override, a production-only allocation hook, or an unbounded/non-deterministic resource-exhaustion test. |
| Exit Criteria | Focused smoke proves all listed EOF/allocation outcomes, its source-specific allocator redirection cannot affect normal target compilation, current-gate evidence passes, task documents close T333, and the coordinator accepts actual changed files. |
| Original Owner Request | Create and queue a task for the P1 interactive input failure issue, then commit the governance change and start T333 implementation. |
| Similar-Issue Sweep | Re-run the 44-reader static inventory; cover every distinct failure outcome class rather than each textual duplicate; prove the special test target alone compiles the redirected allocator symbols, while normal `core-product-debug` and `vm-product` retain their ordinary source graph. |

## Current Technical Baseline

- **Current developer artifact:** T332 S2 selected `vm-0-5-0332` /
  `build/output/nxvm_0_5_0332.exe`; the verified P1 output SHA-256 is
  `0429D3ED2D0D55CB8DB7E8BE0F89344B2FF842C1A0DD25E33A07C4FAD7463D5D`.
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
| T333 S1 | Completed the 44-reader retained Console/debugger inventory and fixed the two-owner S2 boundary in [evidence](../etc/evidence/t333-s1-interactive-input-inventory.md). S2 is active; no runtime repair has yet been accepted. |
| T333 S2 | Repaired every inventoried reader at its product owner: core debugger now has one private read-result boundary and VM Console checks before parse/execute. [Static/build evidence](../etc/evidence/t333-s2-interactive-input-repair.md) is retained; S3 focused proof is active. |
| T332 | Closed VM session construction drift: one private profile materialization/override path; one early-storage rollback owner with stage-failure and late-media recovery proof; and a fixed 47-owner CPU smoke lifecycle closure with inherited-source migration and static guard. The retained artifact, full gates, and residual historical-fixture transfer are in [history](../history/M5-T332-vm-session-construction-transaction.md). |
| T331 | Closed the bounded real-mode `ExecFinal` final-delivery construction: `#DE/#MF/#BR/#NM/#GP` share one rollback/diagnostic plan, and `#GP` IVT success no longer records a terminal fault before transfer. Real-mode `#PF` is architecturally outside paging's protected-mode state. The new `#GP` frame/failed-IVT regression, mechanical construction verifier, artifact, and 212/212 current-gate result are retained in [history](../history/M5-T331-exception-final-delivery.md). |
| T330 | Closed the whole-codebase construction-drift package: one task-transition constructor, FDD/HDD backing/create atomicity, and CALL-gate dual-fault preflight convergence. Its mechanism matrices remain retained evidence; they are not a current mandatory rule. Intel-required layouts remain explicit; T330 artifact, focused regressions, 211/211 current-gate, and governance evidence are in [history](../history/M5-T330-width-path-convergence.md). |
| T329 | Closed the bounded Intel 80286/80386 protected task-transition state machine: 16/32-bit direct and task-gate entry, nested CALL/IRET state, incoming LDT images, source-CR3 preflight/incoming-CR3 commit, and TSS post-switch `#DB`. S7 proves target-page fetch, target-TSS `#PF` atomicity, and a target-state restart frame; the 0329 artifact and 211/211 gate result are in [history](../history/M5-T329-task-transition-state-machine.md). |
| T328 | Closed the historical LOCK-prefix legality matrix: 8086/80186 retain transparent valid-next-instruction semantics; 80286 adds protected `CPL <= IOPL`; retained 80386 memory-whitelist behavior stays intact. S2 reconciled the current closure map and ordinary matrix, removing the stale Deferred/TODO transfer without changing the user-owned Queue edit. Register, memory, REP, I/O, #GP frame, strict compile, artifact, and 211/211 gate evidence are in [history](../history/M5-T328-legacy-lock-legality.md). |
| T327 | Closed the current/specialized-gate reconciliation: fast smoke no longer builds classified media targets, and generated CTest/Ninja evidence now proves the full 210 = 15 media + 195 non-media partition, both developer roots, all specialized verifiers, and the aggregate's two roots. [History](../history/M5-T327-current-gate-reconciliation.md). |
| T326 | Closed ordinary protected-mode invalid-opcode delivery: `#UD` now reaches IDT vector 6 with a restartable three-dword no-error-code frame, while the explicit error-code classifier retains vectors 8/10--14/17. Four producer classes, invalid-gate containment, retained VM86 and 210/210 current-gate pass. Real-mode IVT migration transfers. [History](../history/M5-T326-protected-invalid-opcode-delivery.md). |
| T325 | Closed the CPU-native Intel 80386DX paging/translation package: CR0, CR2/CR3, 4-KiB PDE/PTE, U/S/R/W, A/D, cross-page atomicity, delivered `#PF`, no-persistent-cache behavior, and pre-486 `INVLPG #UD` are reconciled. Protected `#UD` delivery, task/VM86 paging, and persistent TLB/TR6/TR7 state transfer explicitly. [History](../history/M5-T325-80386dx-paging-translation.md). |

## Recent Governance

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
