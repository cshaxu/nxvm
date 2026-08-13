# Project Status

## Current Work

**M5 T338 S3 - active.** Close the complete 8086/80186 inherited data,
string, stack, real-control/interrupt, and ordinary port-I/O form boundary.

## M5 T338 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | The owner approved the Queue-ordered single-agent four-profile CPU-completeness program on 2026-08-13. Accepted T338 S1 ledger assigns this complete legacy mechanism boundary to S3 after S2. |
| Objective | Implement or correct every S3-allocated 8086/80186 inherited form: real data movement and XCHG; segment transfers and LEA; strings and prefixes; GPR/segment stacks; real CALL/JMP/RET, HLT, and INT/IRET forms; ordinary IN/OUT; LES/LDS; and `FF /2`--`/6`. Prove 8086-specific stack quirks, 80186 inherited acceptance, defined state, real fault/restart/nonpublication, and T328 legacy `LOCK` linkage. |
| Non-goals | No arithmetic/condition forms (S2), 80186-only `60`--`62`, `68`--`6F`, `C0/C1`, or `C8/C9` forms (S4), protected selector/table/gate/IOPL rules, VM86, paging, 80386 operand/address width, a new shared prefix policy, or x87 execution. |
| Reference Baseline | `ed2f5e22` (`M5 Td S80 P1: order four-profile closure queue`), T338 artifact `vm-0-5-0338`, the accepted [S1 ledger](../etc/evidence/t338-s1-8086-80186-form-allocation.md), and accepted S2 evidence. |
| Candidate Proposal | [8086 and 80186 profile closure](../proposals/m5-8086-80186-profile-closure.md) and the [T338 S1 allocation](../etc/evidence/t338-s1-8086-80186-form-allocation.md). |
| Files And ABI Surface | Expected: local CPU data/string/stack/control/I/O handlers only if reproduced; owner-bound smoke(s), CMake current-gate registration, T338 evidence/history/Current. No public ABI, device model, generic fixture framework, or shared delivery/prefix interface change. |
| Applicable Rules | `docs/rules/EXECUTION.md`: full P, mechanism/caller/write/fault sweep. `docs/rules/ARCHITECTURE.md`: one validation/commit owner per shared path. `docs/rules/CODING.md`: local style, project types, and no duplicate test framework. |
| Verification | Map each allocated primary and ModRM form to decoder, operand/string/stack/control/I/O owner, validation, publication, restart, and T328 `LOCK` treatment. Build an 8086/80186 form-by-mode ledger with exact stack images, segment selection, REP/DF, port callbacks, IVT frames, and rejected forms. Assert only defined state; prove valid or terminal real delivery as applicable. Run focused owners, exact registrations, metadata/static gates, artifact verifier, documentation governance, diff check, and full current-gate. |
| Expected Markers | Complete S3 evidence has no unclassified allocated row; it names one proof owner per form family and records all Intel-required 8086 stack/layout quirks. New smoke markers, if needed, use `M5:T338:S3:`. |
| Asset Needs | Intel 8086/80186/80386 instruction manuals only. No guest media, firmware, external source, binary, or trace. |
| Reporting Requirements | Before source edits, report the complete route/caller/write/fault inventory and any material boundary objection. Otherwise deliver one complete pushed P with form/profile ledger, all mechanism sweeps, reproduced defects and equivalent fixes, exact transfers, artifact/gate results, and no partial completion claim. |
| Stop Conditions | Stop if correctness needs protected selector/gate/IOPL state, VM86/paging, 80386 width semantics, a global prefix rewrite, generic shared delivery redesign, or Intel behavior cannot be verified. Transfer instead of patching a local symptom. |
| Exit Criteria | Every S3-allocated opcode and ModRM extension has an 8086/80186 disposition and focused proof; Intel-required quirks remain explicit; every discovered same-mechanism defect is fixed or exactly transferred; required gates pass; the complete P is committed and pushed. |
| Original Owner Request | Implement the Queue in order, in single-agent mode and with a holistic view, through four-profile cross-closure verification. |
| Similar-Issue Sweep | Search primary dispatch and all relevant `MOV`, `XCHG`, string, stack, `FF`, transfer, interrupt, `HLT`, and port-I/O handlers; their `_m_*`, `_e_*`, `_p_*`, string iteration, prefix, and `ExecFinal` callers; current tests, CMake owners, T328 evidence, and form metadata. |

## Current Technical Baseline

- **Current developer artifact:** T338 S1/S2 select `vm-0-5-0338` /
  `build/output/nxvm_0_5_0338.exe`; the rebuilt developer output SHA-256 is
  `E9626E829FE8F9A1BE7A25219D48295D704C5831F64C4D2D50709671CB144F13`.
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
| T338 S2 | Accepted the complete 8086/80186 legacy ALU, FLAGS, condition, adjustment, shift, Group 1--3, and grouped-form matrix. No production defect reproduced; the owner smoke proves real `#DE` IVT restart/nonpublication and links every memory-capable form to T328's sole legacy-LOCK contract. `vm-0-5-0338`, documentation governance, and 218/218 current-gate passed. [Evidence](../etc/evidence/t338-s2-legacy-alu-profile-matrix.md). |
| T337 | Closed shared CPU state and delivery foundations: vector-6 fault delivery/rollback, software and external origin composition, IRQ/NMI/TF priority, frame timing, inhibition, and post-commit task debug trap have one proof/owner. DR6/DR7 transfers to 80386DX; reset/shutdown/triple-fault remains bounded CPU/machine debt. [History](../history/M5-T337-shared-state-delivery.md). |
| T336 | Closed the four-profile CPU coverage audit: the primary/`0F` metadata and dispatch forms, real/protected/VM86 state rows, historic evidence, explicit 80386 exclusions, and open CPU debt now have a single indexed disposition. Every residual has one Queue, TODO, or external owner; the next candidate is shared state and delivery foundations. [History](../history/M5-T336-four-profile-coverage.md). |
| T335 | Closed the xasm public-capacity/failure contract: the product facade now uses byte spans, explicit capacities, `type_status`, and commit-on-success result publication; debugger, firmware, and VM debug callback callers migrated with no raw bypass. A dedicated allocation-injection smoke proves capacity, malformed-input, facade-allocation, and engine-allocation failures preserve caller sentinels; `current-gate` passed 216/216. [History](../history/M5-T335-xasm-capacity-failure-contract.md). |
| T334 | Closed EGA sequencer registration atomicity: memory owns coupled provider/observer validation and publication; VADP allocates before publication; allocation and either registry-capacity failure preserve state, while retry publishes exactly one of each. Strict focused smoke and 215/215 current-gate passed. [History](../history/M5-T334-ega-registration-transaction.md). |
| T333 | Closed retained interactive-input failure handling: all 44 Console/debugger readers are inventoried; 43 debugger calls converge at one private boundary; Console stops before parse/execute; EOF/allocation failure and context reuse are covered by two owner-separated smoke targets. Corrective S4 restored its required `0.5.0333` artifact and identity. Evidence and review are in [history](../history/M5-T333-interactive-input-failure-contract.md). |
| T332 | Closed VM session construction drift: one private profile materialization/override path; one early-storage rollback owner with stage-failure and late-media recovery proof; and a fixed 47-owner CPU smoke lifecycle closure with inherited-source migration and static guard. The retained artifact, full gates, and residual historical-fixture transfer are in [history](../history/M5-T332-vm-session-construction-transaction.md). |
| T331 | Closed the bounded real-mode `ExecFinal` final-delivery construction: `#DE/#MF/#BR/#NM/#GP` share one rollback/diagnostic plan, and `#GP` IVT success no longer records a terminal fault before transfer. Real-mode `#PF` is architecturally outside paging's protected-mode state. The new `#GP` frame/failed-IVT regression, mechanical construction verifier, artifact, and 212/212 current-gate result are retained in [history](../history/M5-T331-exception-final-delivery.md). |
| T330 | Closed the whole-codebase construction-drift package: one task-transition constructor, FDD/HDD backing/create atomicity, and CALL-gate dual-fault preflight convergence. Its mechanism matrices remain retained evidence; they are not a current mandatory rule. Intel-required layouts remain explicit; T330 artifact, focused regressions, 211/211 current-gate, and governance evidence are in [history](../history/M5-T330-width-path-convergence.md). |

## Recent Governance

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

- **M5 Td S72:** aligned executor-facing governance: the coordinator creates
  active packets; executor findings are reported for coordinator triage; every
  numbered T packet must link its candidate proposal; and Td P1 is complete
  without a later governance P. Positive and missing-proposal negative
  self-tests, the combined governance check, and diff check passed. Td work
  has no runtime or artifact change.
