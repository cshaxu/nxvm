# Project Status

## Current Work

**M5 T333 S4 - active.** Correct the T333 runnable-artifact omission before a
new task is admitted. This is the narrow closed-task corrective exception in
the [Execution Rules](../rules/EXECUTION.md#linear-identifier-allocation).

## M5 T333 S4 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective |
| Admission And Approval | Owner approved this corrective repair on 2026-08-13 after the code-quality audit showed that closed T333 changed `src/core/product/debug/debug.c` and `src/vm/product/console.c` without its own developer artifact. T333 is the latest closed numeric task and no numeric task is open. The permitted scope is only T333's missing runnable-artifact identity and closure evidence. |
| Objective | Build and record T333's required `0.5.0333` VM developer artifact, then correct the CMake target, preset, Current baseline, and T333 closure record so they identify the same build. |
| Non-goals | No production behavior, input-failure mechanism, test behavior, public ABI, Queue ordering, EGA repair, generic artifact framework, or historical artifact regeneration. |
| Reference Baseline | `6f55d22b` (`M5 T333 S3 P2: close interactive input failure contract`), whose source is the accepted T333 implementation but whose current baseline incorrectly remains T332 `0.5.0332`. |
| Candidate Proposal | Retained [T333 interactive-input proposal](../history/M5-T333-interactive-input-failure-contract-proposal.md); this is its narrow post-closure artifact correction. |
| Files And ABI Surface | `CMakeLists.txt`, `CMakePresets.json`, `docs/states/CURRENT.md`, and `docs/history/M5-T333-interactive-input-failure-contract.md`; ignored `build/output/nxvm_0_5_0333.exe`. No source API or ABI changes. |
| Applicable Rules | `docs/rules/EXECUTION.md`: corrective-S exception, artifact identity, final verification, actual-change audit, and closure. `docs/rules/DOCUMENT.md`: Current owns active status/baseline and history owns closed-task detail. `docs/rules/ARCHITECTURE.md` and `docs/rules/CODING.md`: not applicable beyond preserving the existing runtime and ownership boundary because this S changes no production source. |
| Verification | Fresh GCC configuration; build `current-gcc`; prove `verify-current-artifact-target` selects only `vm-0-5-0333`; verify copied artifact existence, SHA-256, and CMake-supplied `0.5.0333` identity; run `current-gates-gcc`, documentation governance, and `git diff --check`. |
| Expected Markers | `vm-0-5-0333`; `build/output/nxvm_0_5_0333.exe`; one current artifact target; recorded SHA-256; `0.5.0333` compile definition. |
| Asset Needs | None. No source, firmware, guest media, third-party code, or Microsoft material. |
| Reporting Requirements | Executor reports artifact-target confirmation and final commit/hash/gate result. In this single-session run, the coordinator role independently reviews every changed CMake and documentation file, the copied artifact identity, and the final diff before closure. |
| Stop Conditions | Stop if producing the artifact changes production behavior, requires a second current artifact target, cannot establish the version identity, or exposes any non-T333 scope change. |
| Exit Criteria | Exactly one current target and preset select `vm-0-5-0333`; the copied artifact exists with recorded SHA-256 and `0.5.0333` identity; all required gates pass; Current and T333 history name the same artifact and source commit; T333 closes again before T334 admission. |
| Original Owner Request | Reopen the T333 artifact issue separately, then repair the EGA registration failure under T334. |
| Similar-Issue Sweep | Inspect every `add_current_vm_artifact`, `current-gcc` preset target, Current artifact reference, and T333 task record; classify old task-named executables as historical evidence only and do not regenerate them. |

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
| T333 | Closed retained interactive-input failure handling: all 44 Console/debugger readers are inventoried; 43 debugger calls converge at one private boundary; Console stops before parse/execute; EOF/allocation failure and context reuse are covered by two owner-separated smoke targets. `current-gates-gcc` passed 214/214; evidence and review are in [history](../history/M5-T333-interactive-input-failure-contract.md). |
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
