# Project Status

## Current Work

**Active.** M5 T332 S2 establishes one private VM session construction
transaction and failure-atomicity boundary (single-agent execution cycle).

## M5 T332 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved the T332 proposal and its staged VM-session lifecycle transaction on 2026-08-12, then approved continuation after S1 acceptance. Single-agent execution is owner-directed. |
| Objective | Map all VM session construction stages and converge identical failure teardown at the VM composition owner, so failure leaves no active session, core machine, provider binding, or externally visible partial state. Prove the normal and late image/media paths remain intact. |
| Non-goals | No public `vm_session_config` API or ABI change; no core/device/provider semantic change, generic rollback framework, fixture-lifecycle work (S3), rules edit, guest media import, or mantle/DOS/NXVDM work. |
| Reference Baseline | `66da5eef`, accepted T332 S1; active artifact remains `vm-0-5-0332`. |
| Candidate Proposal | [VM session construction transaction](../proposals/m5-vm-session-construction-transaction.md). |
| Files And ABI Surface | Expected private VM composition files under `src/vm/composition/session/`, owner product smoke and CMake/evidence/history/Current updates. No new public header, public state, or product-visible API. |
| Applicable Rules | `docs/README.md` Task Reading Set; `docs/rules/EXECUTION.md` lifecycle, holistic-remediation, actual-change review, and artifact rules; `docs/rules/ARCHITECTURE.md` one stateful construction/rollback owner and composition-root invariants; `docs/rules/CODING.md` bounded owner-local helper rule; `docs/design/ARCHITECTURE.md` VM composition ownership; `docs/design/CODING.md` source/test placement. |
| Verification | First record a stage/caller/rollback map. Add deterministic stage-failure and late image/media failure/recovery proof; retain normal default/configured creation. Build the T332 artifact, run focused session smoke, current artifact verifier, documentation governance, `git diff --check`, specialized gates, and full current CTest selection. |
| Expected Markers | Existing `M5:T300:S3:SESSION-INITIALIZATION-ATOMICITY:OK` and S1 marker remain; add `M5:T332:S2:SESSION-CONSTRUCTION-TRANSACTION:OK`. |
| Asset Needs | None. Failure tests use invalid profile data or nonexistent paths only; no guest image or external source is admitted. |
| Reporting Requirements | Before implementation, record the complete stage/caller/teardown map and either confirm the one-owner boundary or report a stop condition. Deliver one complete P1 with code review, failure/recovery evidence, artifact identity, and similar-issue sweep; push immediately. |
| Stop Conditions | Stop for a required public interface, a core/device/provider behavior defect outside VM composition, an image/media dependency that cannot be deterministically injected, or a rollback that must cross a distinct product owner. Transfer each such item explicitly. |
| Exit Criteria | One private transaction/rollback ownership path replaces identical partial teardown; all admitted stage failures and late image/media failure prove no partial session publication and recovery; normal creation remains valid; evidence/history/artifact are updated and full verification passes. |
| Original Owner Request | The owner asked to create a documentation Td, then execute the two VM-session and fixture findings as multiple subtasks; S2 is the approved lifecycle/failure-atomicity subtask. |
| Similar-Issue Sweep | Inspect every `vm_session_create`, initialize, finalize, storage/control/provider lifecycle, machine-device configuration, image insertion, and failure return path. Classify each as converged, intentionally distinct owner behavior, or separately deferred debt. |

## Current Technical Baseline

- **Current developer artifact:** active T332 S2 selects `vm-0-5-0332` /
  `build/output/nxvm_0_5_0332.exe`; the verified active-P1 output SHA-256 is
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
| T332 S1 | Accepted `f0b1a9bd`: VM session creation now has one private profile-default materialization followed by explicit memory/CPU/FPU overrides. The focused default/override/core-value proof, 49/49 specialized gates, 212/212 current CTests, and T332 artifact SHA-256 are retained in [history](../history/M5-T332-vm-session-construction-transaction.md). |
| T331 | Closed the bounded real-mode `ExecFinal` final-delivery construction: `#DE/#MF/#BR/#NM/#GP` share one rollback/diagnostic plan, and `#GP` IVT success no longer records a terminal fault before transfer. Real-mode `#PF` is architecturally outside paging's protected-mode state. The new `#GP` frame/failed-IVT regression, mechanical construction verifier, artifact, and 212/212 current-gate result are retained in [history](../history/M5-T331-exception-final-delivery.md). |
| T330 | Closed the whole-codebase construction-drift package: one task-transition constructor, FDD/HDD backing/create atomicity, and CALL-gate dual-fault preflight convergence. Its mechanism matrices remain retained evidence; they are not a current mandatory rule. Intel-required layouts remain explicit; T330 artifact, focused regressions, 211/211 current-gate, and governance evidence are in [history](../history/M5-T330-width-path-convergence.md). |
| T329 | Closed the bounded Intel 80286/80386 protected task-transition state machine: 16/32-bit direct and task-gate entry, nested CALL/IRET state, incoming LDT images, source-CR3 preflight/incoming-CR3 commit, and TSS post-switch `#DB`. S7 proves target-page fetch, target-TSS `#PF` atomicity, and a target-state restart frame; the 0329 artifact and 211/211 gate result are in [history](../history/M5-T329-task-transition-state-machine.md). |
| T328 | Closed the historical LOCK-prefix legality matrix: 8086/80186 retain transparent valid-next-instruction semantics; 80286 adds protected `CPL <= IOPL`; retained 80386 memory-whitelist behavior stays intact. S2 reconciled the current closure map and ordinary matrix, removing the stale Deferred/TODO transfer without changing the user-owned Queue edit. Register, memory, REP, I/O, #GP frame, strict compile, artifact, and 211/211 gate evidence are in [history](../history/M5-T328-legacy-lock-legality.md). |
| T327 | Closed the current/specialized-gate reconciliation: fast smoke no longer builds classified media targets, and generated CTest/Ninja evidence now proves the full 210 = 15 media + 195 non-media partition, both developer roots, all specialized verifiers, and the aggregate's two roots. [History](../history/M5-T327-current-gate-reconciliation.md). |
| T326 | Closed ordinary protected-mode invalid-opcode delivery: `#UD` now reaches IDT vector 6 with a restartable three-dword no-error-code frame, while the explicit error-code classifier retains vectors 8/10--14/17. Four producer classes, invalid-gate containment, retained VM86 and 210/210 current-gate pass. Real-mode IVT migration transfers. [History](../history/M5-T326-protected-invalid-opcode-delivery.md). |
| T325 | Closed the CPU-native Intel 80386DX paging/translation package: CR0, CR2/CR3, 4-KiB PDE/PTE, U/S/R/W, A/D, cross-page atomicity, delivered `#PF`, no-persistent-cache behavior, and pre-486 `INVLPG #UD` are reconciled. Protected `#UD` delivery, task/VM86 paging, and persistent TLB/TR6/TR7 state transfer explicitly. [History](../history/M5-T325-80386dx-paging-translation.md). |
| T324 | Closed current-test/specialized-gate separation: `run-current-smokes` now runs only the full 209-test CTest smoke selection, `run-current-fast-smokes` retains the 194-test non-media selection, and `verify-current-specialized-gates` owns 46 named verifiers plus a mechanical target-graph check. `current-gates-gcc` composes exactly both roots; all layer baselines and evidence are retained in [T324 history](../history/M5-T324-current-gate-separation.md). |

## Recent Governance

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

- **M5 Td S68:** reconciled open numeric-task progress with identifier
  allocation: commit history proves used S/P records, Status structural rows
  determine task closure, and the verifier permits only the exact next S of the
  latest retained-open task under `Continuation`. Added positive and
  skipped/foreign/missing-progress/new-task negative self-tests. Documentation
  self-test, default check, and diff check passed. Td work has no runtime or
  artifact change.
