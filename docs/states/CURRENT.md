# Project Status

## Current Work

**M5 T338 S2 - active.** Close the complete 8086/80186 legacy ALU, FLAGS,
condition, adjustment, shift, multiply/divide, and grouped-form boundary.

## M5 T338 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | The owner approved the Queue-ordered single-agent four-profile program on 2026-08-13. T338 S1 form allocation was accepted in `01ee8448`; it assigns this full legacy ALU/FLAGS/condition mechanism as the first implementation slice. |
| Objective | Implement or correct every 8086/80186 form allocated to T338 S2: binary arithmetic `00`--`05` through `38`--`3D`; Group 1 `80/81/83`; DAA/DAS/AAA/AAS; INC/DEC register and `FE`; TEST forms; all conditional branches `70`--`7F` and loops `E0`--`E3`; Group 2 `D0`--`D3` plus 80186 `C0/C1`; AAM/AAD/XLAT; Group 3 `F6/F7` arithmetic; CMC/CLC/STC/CLD/STD; CBW/CWD; LAHF/SAHF; and all applicable reserved/profile rejection. Prove every ModRM extension, defined FLAGS only, valid 8086/80186 profile behavior, real-mode exception/restart/nonpublication, and T328 legacy LOCK linkage. |
| Non-goals | No data/string/stack/control/I/O implementation (T338 S3), no 80186 PUSHA/BOUND/PUSH/IMUL/INS-OUTS/ENTER bundle (T338 S4), no protected selector/privilege/paging/VM86/80386 width semantics, no undefined FLAGS assertion, no new global LOCK policy, and no x87 execution. |
| Reference Baseline | `01ee8448` (`M5 T338 S1 P2: accept legacy form allocation`), current artifact `vm-0-5-0338` / `0.5.0338`, and the accepted [S1 ledger](../etc/evidence/t338-s1-8086-80186-form-allocation.md). |
| Candidate Proposal | [8086 and 80186 profile closure](../proposals/m5-8086-80186-profile-closure.md); [T338 S1 form allocation](../etc/evidence/t338-s1-8086-80186-form-allocation.md). |
| Files And ABI Surface | Expected: local CPU arithmetic/condition handlers only if reproduced; one owner-bound legacy ALU smoke or an expanded coherent existing owner; CMake current-gate registration; T338 evidence/history/Current. No public ABI, device model, generic fixture framework, or shared delivery/prefix interface change. |
| Applicable Rules | `docs/rules/EXECUTION.md`: full P, mechanism/caller/write/fault sweep. `docs/rules/ARCHITECTURE.md`: shared validation/commit owner. `docs/rules/CODING.md`: existing local code style, project types, and no duplicate test framework. `docs/etc/operations/policy/source-policy.md`: Intel authority only. |
| Verification | Before repair, map `_a_*`, `_kaf_set_flags`, grouped decode, `_m_read_rm`/`_m_write_rm`, profile gate, and `ExecFinal` callers/writes/rollback. Build a form-by-profile-by-ModRM ledger with success, reserved, divide/overflow, memory access, and real `#UD`/fault vectors. Assert only Intel-defined FLAGS; check EIP, GPR/memory publication, fault restart, and valid or invalid IVT delivery as appropriate. Run focused smoke, exact current-gate registration, T338 metadata guard, artifact verifier, documentation governance, diff check, and complete current-gate. |
| Expected Markers | One `M5:T338:S2:LEGACY-ALU:OK` owner marker; complete table records all allocated form/extension/profile rows; no repeated handler construction remains without an Intel-required difference; every valid memory-capable form explicitly links to T328 legacy LOCK behavior, while invalid forms stay invalid. |
| Asset Needs | Intel 8086/80186/80386 instruction manuals only. No guest media, firmware, external source, binary, or trace. |
| Reporting Requirements | Report a material mechanism/scope objection before source change. Otherwise deliver one complete pushed P with form/ModRM/profile/FLAGS table, caller/write/fault sweep, reproduced defect and all equivalent fixes, unchanged undefined-FLAGS boundary, exact transfer list, artifact/gate results, and no partial completion claim. |
| Stop Conditions | Stop if a correct fix needs a global prefix/LOCK policy rewrite, generic arithmetic/flags redesign beyond all named callers, protected/VM86/paging state, a later-profile width contract, or Intel behavior cannot be verified. Transfer/revise the packet rather than patch a local symptom. |
| Exit Criteria | Every allocated opcode and ModRM extension has an 8086/80186 disposition and focused proof; every discovered same-mechanism defect is fixed or exactly transferred; defined FLAGS, publication, restart and legacy LOCK linkage are proven; all required gates pass; the complete P is committed and pushed. |
| Original Owner Request | Implement the Queue in order, in single-agent mode and with a holistic view, through four-profile cross-closure verification. |
| Similar-Issue Sweep | Search all tracked production/tests/CMake/evidence for the named primary ranges, `INS_80`, `INS_81`, `INS_83`, `INS_C0`, `INS_C1`, `INS_D0`--`INS_D3`, `INS_F6`, `INS_F7`, `INS_FE`, `_a_`, `_kaf_set_flags`, `CMP_FLAG`, `PREFIX_LOCK`, `UndefinedOpcode`, and legacy profile gates. |

## Current Technical Baseline

- **Current developer artifact:** T338 S1 selects `vm-0-5-0338` /
  `build/output/nxvm_0_5_0338.exe`; the rebuilt developer output SHA-256 is
  `322B835AF2E0647A65627023ACD87D50A9F8F6F3BF0D9F95C384D2A8D213214C`.
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
| T338 S1 | Accepted the exhaustive 8086/80186 primary-form and legacy-LOCK allocation: every in-scope form, extension, reserved rejection, external boundary, and later-profile transfer has one disposition; S2--S5 boundaries are fixed without creating a second LOCK policy. `vm-0-5-0338`, 52 specialized gates, and 217/217 current-gate passed. [Evidence](../etc/evidence/t338-s1-8086-80186-form-allocation.md). |
| T337 | Closed shared CPU state and delivery foundations: vector-6 fault delivery/rollback, software and external origin composition, IRQ/NMI/TF priority, frame timing, inhibition, and post-commit task debug trap have one proof/owner. DR6/DR7 transfers to 80386DX; reset/shutdown/triple-fault remains bounded CPU/machine debt. [History](../history/M5-T337-shared-state-delivery.md). |
| T336 | Closed the four-profile CPU coverage audit: the primary/`0F` metadata and dispatch forms, real/protected/VM86 state rows, historic evidence, explicit 80386 exclusions, and open CPU debt now have a single indexed disposition. Every residual has one Queue, TODO, or external owner; the next candidate is shared state and delivery foundations. [History](../history/M5-T336-four-profile-coverage.md). |
| T335 | Closed the xasm public-capacity/failure contract: the product facade now uses byte spans, explicit capacities, `type_status`, and commit-on-success result publication; debugger, firmware, and VM debug callback callers migrated with no raw bypass. A dedicated allocation-injection smoke proves capacity, malformed-input, facade-allocation, and engine-allocation failures preserve caller sentinels; `current-gate` passed 216/216. [History](../history/M5-T335-xasm-capacity-failure-contract.md). |
| T334 | Closed EGA sequencer registration atomicity: memory owns coupled provider/observer validation and publication; VADP allocates before publication; allocation and either registry-capacity failure preserve state, while retry publishes exactly one of each. Strict focused smoke and 215/215 current-gate passed. [History](../history/M5-T334-ega-registration-transaction.md). |
| T333 | Closed retained interactive-input failure handling: all 44 Console/debugger readers are inventoried; 43 debugger calls converge at one private boundary; Console stops before parse/execute; EOF/allocation failure and context reuse are covered by two owner-separated smoke targets. Corrective S4 restored its required `0.5.0333` artifact and identity. Evidence and review are in [history](../history/M5-T333-interactive-input-failure-contract.md). |
| T332 | Closed VM session construction drift: one private profile materialization/override path; one early-storage rollback owner with stage-failure and late-media recovery proof; and a fixed 47-owner CPU smoke lifecycle closure with inherited-source migration and static guard. The retained artifact, full gates, and residual historical-fixture transfer are in [history](../history/M5-T332-vm-session-construction-transaction.md). |
| T331 | Closed the bounded real-mode `ExecFinal` final-delivery construction: `#DE/#MF/#BR/#NM/#GP` share one rollback/diagnostic plan, and `#GP` IVT success no longer records a terminal fault before transfer. Real-mode `#PF` is architecturally outside paging's protected-mode state. The new `#GP` frame/failed-IVT regression, mechanical construction verifier, artifact, and 212/212 current-gate result are retained in [history](../history/M5-T331-exception-final-delivery.md). |
| T330 | Closed the whole-codebase construction-drift package: one task-transition constructor, FDD/HDD backing/create atomicity, and CALL-gate dual-fault preflight convergence. Its mechanism matrices remain retained evidence; they are not a current mandatory rule. Intel-required layouts remain explicit; T330 artifact, focused regressions, 211/211 current-gate, and governance evidence are in [history](../history/M5-T330-width-path-convergence.md). |

## Recent Governance

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
