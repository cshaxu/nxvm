# Project Status

## Current Work

**M5 T334 S1 - active.** Repair EGA sequencer registration failure atomicity at
the memory registry owner. The owner-approved candidate is
[the EGA registration transaction](../proposals/m5-ega-registration-transaction.md).

## M5 T334 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner approved on 2026-08-13: after T333 closes, admit the audited EGA observer/provider registration defect as T334 and execute it to closure. Scope is the shared memory registry and its sole coupled VADP caller. |
| Objective | Make planar EGA sequencer configuration failure-atomic: failed allocation, observer-capacity failure, and provider-capacity failure leave no registration or VADP-owned allocation; a later valid retry publishes exactly one provider and observer. |
| Non-goals | No EGA behavior breadth, registry-capacity increase, public product ABI, generic transaction framework, xasm work, CPU work, or unrelated memory-registry migration. |
| Reference Baseline | `81a602fd` (`M5 T333 S4 P2: close artifact correction`) and its recorded `0.5.0333` developer artifact. The audited defect is in `core_machine_vadp_configure_ega_sequencer` after early observer publication. |
| Candidate Proposal | [M5 EGA registration transaction](../proposals/m5-ega-registration-transaction.md). |
| Files And ABI Surface | `src/core/machine/memory.c`, `src/core/machine/memory.h`, `src/core/machine/vadp.c`, one owner-bound `tests/machine/` smoke, CMake registration/current artifact target and preset, Current/history/proposal/Queue records, and ignored `build/output/nxvm_0_5_0334.exe`. No product-facing ABI changes. |
| Applicable Rules | `docs/rules/EXECUTION.md`: mechanism-defect owner/variants/preflight-commit boundary, actual-change audit, artifact and closure. `docs/rules/ARCHITECTURE.md`: one owner for stateful failure boundaries. `docs/rules/CODING.md`: repair at the owning boundary with no duplicate VADP side path; tests prove the owned boundary. `docs/rules/DOCUMENT.md`: Current owns the active contract and history retains closure. |
| Verification | Static caller and failure-point inventory; strict focused smoke proving allocation failure, observer-capacity failure, provider-capacity failure, retry, and exactly-one success registration; fresh GCC configuration; `verify-ega-sequencer-boundary`, documentation governance, current-artifact verification, `current-gates-gcc`, artifact SHA-256/version identity, and `git diff --check`. |
| Expected Markers | `M5:T334:S1:EGA-REGISTRATION-TRANSACTION:OK`; `vm-0-5-0334`; `build/output/nxvm_0_5_0334.exe`; one provider and one observer only after successful planar EGA configuration. |
| Asset Needs | None. No source, firmware, guest media, third-party code, or Microsoft material. |
| Reporting Requirements | Executor reports the owner/caller inventory before implementation, then delivers focused and full-gate evidence. In this single-session run, coordinator review inspects registry validation/publication ordering, all failure cleanup, CMake source separation for allocation injection, artifact identity, and every changed document before closure. |
| Stop Conditions | Stop if a second coupled caller requires different rollback semantics, if proof needs mutable public registry access or a process-global allocator hook, or if the repair changes guest-visible EGA behavior. Record a transferred issue rather than broadening the task. |
| Exit Criteria | The sole coupled caller uses the memory-owned atomic operation; no failed case changes registry counts or VADP configured/allocation state; retry and successful exact-one registration are proven; all listed verification passes; T334 `0.5.0334` artifact/hash is recorded; proposal moves to history and Queue/current state close consistently. |
| Original Owner Request | Reopen T333 only for its artifact correction, then create T334 to repair the EGA registration failure and execute both fully. |
| Similar-Issue Sweep | Inventory all `core_machine_memory_register_write_observer` and `core_machine_memory_register_device_provider` callers, all VADP EGA configuration fallible operations, and every test using EGA setup. Classify whether any other caller needs coupled atomic publication; cover it only if it shares the same contract, otherwise transfer it explicitly. |

## Current Technical Baseline

- **Current developer artifact:** T334 S1 selected `vm-0-5-0334` /
  `build/output/nxvm_0_5_0334.exe`; the verified P1 output SHA-256 is
  `B71758ABB6EA25B6F818786FCDA8D668F773925917E0C6385F26B0D419C25579`.
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
| T333 | Closed retained interactive-input failure handling: all 44 Console/debugger readers are inventoried; 43 debugger calls converge at one private boundary; Console stops before parse/execute; EOF/allocation failure and context reuse are covered by two owner-separated smoke targets. Corrective S4 restored its required `0.5.0333` artifact and identity. Evidence and review are in [history](../history/M5-T333-interactive-input-failure-contract.md). |
| T332 | Closed VM session construction drift: one private profile materialization/override path; one early-storage rollback owner with stage-failure and late-media recovery proof; and a fixed 47-owner CPU smoke lifecycle closure with inherited-source migration and static guard. The retained artifact, full gates, and residual historical-fixture transfer are in [history](../history/M5-T332-vm-session-construction-transaction.md). |
| T331 | Closed the bounded real-mode `ExecFinal` final-delivery construction: `#DE/#MF/#BR/#NM/#GP` share one rollback/diagnostic plan, and `#GP` IVT success no longer records a terminal fault before transfer. Real-mode `#PF` is architecturally outside paging's protected-mode state. The new `#GP` frame/failed-IVT regression, mechanical construction verifier, artifact, and 212/212 current-gate result are retained in [history](../history/M5-T331-exception-final-delivery.md). |
| T330 | Closed the whole-codebase construction-drift package: one task-transition constructor, FDD/HDD backing/create atomicity, and CALL-gate dual-fault preflight convergence. Its mechanism matrices remain retained evidence; they are not a current mandatory rule. Intel-required layouts remain explicit; T330 artifact, focused regressions, 211/211 current-gate, and governance evidence are in [history](../history/M5-T330-width-path-convergence.md). |
| T329 | Closed the bounded Intel 80286/80386 protected task-transition state machine: 16/32-bit direct and task-gate entry, nested CALL/IRET state, incoming LDT images, source-CR3 preflight/incoming-CR3 commit, and TSS post-switch `#DB`. S7 proves target-page fetch, target-TSS `#PF` atomicity, and a target-state restart frame; the 0329 artifact and 211/211 gate result are in [history](../history/M5-T329-task-transition-state-machine.md). |
| T328 | Closed the historical LOCK-prefix legality matrix: 8086/80186 retain transparent valid-next-instruction semantics; 80286 adds protected `CPL <= IOPL`; retained 80386 memory-whitelist behavior stays intact. S2 reconciled the current closure map and ordinary matrix, removing the stale Deferred/TODO transfer without changing the user-owned Queue edit. Register, memory, REP, I/O, #GP frame, strict compile, artifact, and 211/211 gate evidence are in [history](../history/M5-T328-legacy-lock-legality.md). |
| T327 | Closed the current/specialized-gate reconciliation: fast smoke no longer builds classified media targets, and generated CTest/Ninja evidence now proves the full 210 = 15 media + 195 non-media partition, both developer roots, all specialized verifiers, and the aggregate's two roots. [History](../history/M5-T327-current-gate-reconciliation.md). |
| T326 | Closed ordinary protected-mode invalid-opcode delivery: `#UD` now reaches IDT vector 6 with a restartable three-dword no-error-code frame, while the explicit error-code classifier retains vectors 8/10--14/17. Four producer classes, invalid-gate containment, retained VM86 and 210/210 current-gate pass. Real-mode IVT migration transfers. [History](../history/M5-T326-protected-invalid-opcode-delivery.md). |

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
