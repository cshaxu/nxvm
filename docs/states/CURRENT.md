# Project Status

## Current Work

**M5 T339 S3 - active.** Close the 80286 selector-query and segment-cache
materialization matrix before protected gate, return, or task work proceeds.

## M5 T339 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | The owner approved Queue-ordered single-agent implementation through four-profile cross-closure verification on 2026-08-13. Accepted T339 S2 (`84a46efd`) closes the preceding descriptor-table and system-word matrix. |
| Objective | Reconcile and, only where a reproducer requires it, implement the complete 80286 selector-query and cache-materialization matrix: `0F 00 /0`--`/5` (SLDT, STR, LLDT, LTR, VERR, VERW), `0F 02/03` (LAR, LSL), `8C/8E` MOV Sreg, legacy PUSH/POP Sreg, and protected LES/LDS (`C4/C5`). Prove selector/table lookup, null/TI/GDT/LDT/type/DPL/RPL/present/accessed rules, source preflight, cache/register publication, restart, and post-success IRQ ordering under one mechanism-owned ledger. |
| Non-goals | No 80286 `LOCK` legality matrix; no protected gate entry, return/frame, task/TSS work; no 80386 `66/67`, FS/GS, LSS/LFS/LGS, VM86, paging, debug, VME/PVI, x87, generic fixture framework, or generic selector/delivery rewrite. |
| Reference Baseline | `84a46efd` (`M5 T339 S2 P1: close descriptor system-word matrix`), the accepted [S1 allocation](../etc/evidence/t339-s1-80286-descriptor-transfer-allocation.md), retained T316 S41/S46/S57/S58/S61 evidence, and the 80286 descriptor-transfer proposal. |
| Candidate Proposal | [80286 descriptor-table and protected-transfer closure](../proposals/m5-80286-descriptor-transfer-closure.md) and the accepted [T339 allocation](../etc/evidence/t339-s1-80286-descriptor-transfer-allocation.md). |
| Files And ABI Surface | Expected: local selector/cache handlers only if a reproducer requires it; owner-bound smokes, CMake registration, T339 evidence/history/Current, and required artifact identity. No public ABI, generic descriptor/table/decoder/exception/fixture interface, device model, or product boundary change. |
| Applicable Rules | `docs/rules/EXECUTION.md`: full P, complete form/caller/write/fault sweep, and actual-change review. `docs/rules/ARCHITECTURE.md`: one validation/materialization/commit owner; no partial table image or post-commit fault. `docs/rules/CODING.md`: project types, local style, and owner-bound tests. |
| Verification | Audit `INS_0F_00`, `LAR_R32_RM32`, `LSL_R32_RM32`, `MOV_RM16_SREG`, `MOV_SREG_RM16`, legacy segment-stack routes, LES/LDS, `_d_modrm_sreg`, `_ksa_prepare_*_sreg`, `_ksa_load_sreg`, `_s_load_{es,cs,ss,ds,ldtr,tr}`, all callers and every cache/table/register writer. For each form prove 80286 real/protected disposition; 16-bit selector/image rules; null/TI/GDT/LDT/type/DPL/RPL/present/accessed outcomes; source limit and publication/restart atomicity; and pending-IRQ ordering where successful. Run focused owners, exact registrations, static inventories, artifact verifier, documentation governance, diff check, and full current gate. |
| Expected Markers | A complete S3 matrix has one current-gate owner per allocated form; no 80286 selector/cache row is represented only by 80386 width, FS/GS, or VM86 proof. New marker, if necessary, uses `M5:T339:S3:`. |
| Asset Needs | Intel 80286 and 80386 architecture manuals already used by project evidence. No guest media, firmware, third-party source, binary, or trace. |
| Reporting Requirements | Before source edits, record the handler/caller/write/fault inventory and any material boundary objection. Otherwise deliver one complete pushed P with form matrix, historic-evidence disposition, defect/similar-issue result, artifact/gate results, and no partial-completion claim. |
| Stop Conditions | Stop if correctness requires `66/67`, FS/GS, VM86, paging/CR0 32-bit state, gate/return/task mechanics, generic descriptor-table/decoder/delivery redesign, or Intel behavior cannot be classified. Transfer the row rather than fitting a local symptom. |
| Exit Criteria | Every S3 form and ModRM extension has an 80286 real/protected/privilege disposition; selector/cache/table state, fault/restart/nonpublication, and successful IRQ boundaries are proved or exactly transferred; any mechanism defect is fixed with its complete caller sweep; required gates pass; the P is committed and pushed. |
| Original Owner Request | Implement the Queue in order, in single-agent mode and with a holistic view, through four-profile cross-closure verification. |
| Similar-Issue Sweep | Search `0F 00`, `0F 02`, `0F 03`, `8C`, `8E`, `C4`, `C5`, `INS_0F_00`, `LAR_R32_RM32`, `LSL_R32_RM32`, selector load/prepare helpers, all selector/cache/table writers, profile metadata, current owner smokes, CMake registrations, and T316/T323/T329 evidence. |

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
| T339 S2 | Closed the 80286 descriptor-table and system-word matrix: SGDT/SIDT, LGDT/LIDT, SMSW/LMSW, and CLTS retain one decode/materialization owner, full source/destination preflight, and successful IRQ proof. LGDT/LIDT now exercise genuine 80286 CPL3 `#GP` delivery through a 16-bit gate; `LMSW`/`CLTS` CPL3 gate-frame completion transfers to S4. The only reproduced defect was a stale T337 CMake disposition entry, corrected after its complete explicit-list sweep. [Evidence](../etc/evidence/t339-s2-descriptor-system-word-matrix.md). |
| T339 S1 | Allocated every 80286 descriptor/table, selector/cache, protected-entry, protected-return, and TSS16 transfer row to mechanism-owned S2--S6 work; S7 is audit-only. The Queue is now an explicit 80286-to-80386DX dependency chain, and every 32-bit, VM86, paging, debug, and x87 boundary names its receiving owner. [Ledger](../etc/evidence/t339-s1-80286-descriptor-transfer-allocation.md). |
| T338 | Closed the 8086/80186 profile baseline: S2 ALU/FLAGS/conditions, S3 inherited data/control/I/O, and S4 80186 extensions cover every S1 allocation; the retained T328 rule is the only legacy `LOCK` owner. The sole reproduced defect was four incorrect INS/OUTS 80386 guards, corrected to 80186. Protected, 80386DX, and x87 boundaries transfer explicitly. Artifact verification, governance, and 218/218 current-gate passed. [History](../history/M5-T338-8086-80186-profile-closure.md). |
| T337 | Closed shared CPU state and delivery foundations: vector-6 fault delivery/rollback, software and external origin composition, IRQ/NMI/TF priority, frame timing, inhibition, and post-commit task debug trap have one proof/owner. DR6/DR7 transfers to 80386DX; reset/shutdown/triple-fault remains bounded CPU/machine debt. [History](../history/M5-T337-shared-state-delivery.md). |
| T336 | Closed the four-profile CPU coverage audit: the primary/`0F` metadata and dispatch forms, real/protected/VM86 state rows, historic evidence, explicit 80386 exclusions, and open CPU debt now have a single indexed disposition. Every residual has one Queue, TODO, or external owner; the next candidate is shared state and delivery foundations. [History](../history/M5-T336-four-profile-coverage.md). |
| T335 | Closed the xasm public-capacity/failure contract: the product facade now uses byte spans, explicit capacities, `type_status`, and commit-on-success result publication; debugger, firmware, and VM debug callback callers migrated with no raw bypass. A dedicated allocation-injection smoke proves capacity, malformed-input, facade-allocation, and engine-allocation failures preserve caller sentinels; `current-gate` passed 216/216. [History](../history/M5-T335-xasm-capacity-failure-contract.md). |
| T334 | Closed EGA sequencer registration atomicity: memory owns coupled provider/observer validation and publication; VADP allocates before publication; allocation and either registry-capacity failure preserve state, while retry publishes exactly one of each. Strict focused smoke and 215/215 current-gate passed. [History](../history/M5-T334-ega-registration-transaction.md). |
| T333 | Closed retained interactive-input failure handling: all 44 Console/debugger readers are inventoried; 43 debugger calls converge at one private boundary; Console stops before parse/execute; EOF/allocation failure and context reuse are covered by two owner-separated smoke targets. Corrective S4 restored its required `0.5.0333` artifact and identity. Evidence and review are in [history](../history/M5-T333-interactive-input-failure-contract.md). |
| T332 | Closed VM session construction drift: one private profile materialization/override path; one early-storage rollback owner with stage-failure and late-media recovery proof; and a fixed 47-owner CPU smoke lifecycle closure with inherited-source migration and static guard. The retained artifact, full gates, and residual historical-fixture transfer are in [history](../history/M5-T332-vm-session-construction-transaction.md). |
| T331 | Closed the bounded real-mode `ExecFinal` final-delivery construction: `#DE/#MF/#BR/#NM/#GP` share one rollback/diagnostic plan, and `#GP` IVT success no longer records a terminal fault before transfer. Real-mode `#PF` is architecturally outside paging's protected-mode state. The new `#GP` frame/failed-IVT regression, mechanical construction verifier, artifact, and 212/212 current-gate result are retained in [history](../history/M5-T331-exception-final-delivery.md). |

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
