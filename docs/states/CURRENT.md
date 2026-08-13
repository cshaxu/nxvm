# Project Status

## Current Work

**M5 T341 S5 - active.** Audit the completed 80386DX system-state package
against its original allocation ledger before the four-profile audit.

## M5 T341 S5 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | The owner approved Queue-ordered, single-agent implementation through the four-profile cross-closure verification on 2026-08-13. T341 S1--S4 are accepted; S4 implementation is `b71d8923`. |
| Objective | Audit the complete T341 S1--S4 80386DX system-state allocation ledger against current production owners, focused proof, inherited evidence, Queue, and TODO. Close the package only if every allocated row has an exact accepted disposition; otherwise return the row once to an earlier T341 mechanism S, TODO, or an explicit post-80386 boundary. |
| Non-goals | No production, test, CMake, artifact, product, device, or rule change. No new instruction slice, no reopening a closed form solely for broader coverage, no implicit profile-completeness claim, and no repair under the audit. |
| Reference Baseline | `b71d8923` (`M5 T341 S4 P1: reconcile ordinary debug delivery`), the S1 allocation ledger, S2/S3/S4 evidence, T329/T337 task and delivery records, T340 transfer audit, current Queue/TODO, and the 80386DX closure map. |
| Candidate Proposal | [80386DX system, VM86, paging, and debug closure](../proposals/m5-80386dx-system-state-closure.md), [80386DX extended-state closure context](../proposals/m5-80386dx-extended-state-closure.md), [four-profile CPU-completeness program](../proposals/m5-four-profile-cpu-completeness-program.md), and [80386DX admission policy](../proposals/m5-80386dx-candidate-policy.md). |
| Files And ABI Surface | Expected: T341 closure evidence/history/Current only. No public ABI, source, CMake, artifact, device, product, or rule change. |
| Applicable Rules | `docs/rules/EXECUTION.md`: T-level closure audit, similar-issue sweep, complete P, actual-change review, and lifecycle. `docs/rules/ARCHITECTURE.md`: one mutable-state owner and explicit semantic/layout differences. `docs/rules/CODING.md`: no duplicate abstraction. `docs/rules/DOCUMENT.md`: Current/history/evidence topology. |
| Verification | Re-read the original owner request and S1 ledger. Sweep all allocated CR/DR/TR/LDT/table, VM86/task/paging, and debug/vector-1 owners; compare each source route with the exact accepted proof and current-gate registration. Reconcile every residual with Queue/TODO/external boundary, then run documentation governance and diff check. No runtime gate is claimed by this audit unless it is rerun for a discovered discrepancy. |
| Expected Markers | One indexed closure audit maps every S1 row to an accepted S2/S3/S4 proof, one retained exact owner, or one explicit external transfer. No duplicate mutable-state owner, undocumented production hit, or unclassified transfer remains. |
| Asset Needs | Intel 80386 manuals already cited by retained evidence. No guest media, firmware, third-party source, binary, or trace. |
| Reporting Requirements | Deliver one complete pushed P with the row-by-row closure audit, exact retained proof/transfer, source and evidence sweep, documentation-gate result, and any returned work. |
| Stop Conditions | Stop if a residual requires a production repair, an unallocated instruction family, VME/PVI, later-CPU debug semantics, persistent translation cache, x87 execution, timing/device/product behavior, a rule change, or external source/import. Return it through the named owner rather than repairing it under S5. |
| Exit Criteria | Every S1 allocated row is reconciled exactly once; S2/S3/S4 outcomes and all external transfers are truthful; no in-scope unclassified production hit remains; evidence/history/Current are truthful; and the audit P is committed and pushed. |
| Original Owner Request | Implement the Queue in order, in single-agent mode and with a holistic view, through four-profile cross-closure verification. |
| Similar-Issue Sweep | Sweep all tracked CPU execution and machine-state source, current-gate registration, S1--S4 evidence, Queue, TODO, and closure-map references for CR0--CR3, DR0--DR7, TR/LDT/table state, VM86, task/TSS, paging, vector 1, RF/TF, exception frame, and IRQ/exception construction. Record every hit's exact owner and disposition; no broad repository rewrite. |

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
| T341 S4 | Reconciled ordinary 80386 DR6/DR7 matching with pre-instruction fault rollback, post-access/TF traps, RF, TSS debug `BT`, local/global DR7 task handoff, and real/protected/VM86 vector-1 delivery. Corrected the retained TF-before-PIC ordering claim; 218/218 current-gate passed. [Evidence](../etc/evidence/t341-s4-ordinary-debug-vector1.md). |
| T341 S3 | Reconciled VM86 entry/return, task/TSS, and paging as one state-machine graph. Added mapped VM86 source delivery and mapped IRET-return probes without duplicating page walk, TSS plan, or delivery ownership. [Evidence](../etc/evidence/t341-s3-vm86-task-paging-composition.md). |
| T341 S2 | Reconciled CR/DR/TR/LDT/table forms to their sole decoder and mutable-state owner; retained proof covers exact 80386 forms, while TR6/TR7 stay at the explicit no-persistent-cache boundary and S4 owns ordinary breakpoint/vector-1 behavior. [Evidence](../etc/evidence/t341-s2-control-debug-table-reconciliation.md). |
| T341 S1 | Allocated the complete 80386DX privileged-state ledger. S2 owns CR/DR/TR/LDT/table reconciliation, S3 owns VM86/task/paging composition, S4 owns ordinary breakpoint/vector-1, and S5 remains audit-only. [Allocation ledger](../etc/evidence/t341-s1-80386dx-system-state-allocation.md). |
| T340 | Closed the 80386DX width, prefix, FS/GS, and non-privileged integer-form package: S1 allocated the finite form/state ledger; S2/S3 reconciled the shared mechanisms and every assigned `0F` family; S4 confirmed accepted proof or one exact T341/external transfer for each row. [Closure audit](../etc/evidence/t340-s4-80386dx-form-closure-audit.md). |
| T339 | Closed the Queue-ordered 80286 descriptor-transfer package: S2--S6 proved table/system-word, selector/cache, protected entry, protected return, and TSS16 transition mechanisms; S7 reconciled the source graph, retained T328 `LOCK` policy, and exact T341--T342 transfers. [Closure audit](../etc/evidence/t339-s7-80286-descriptor-transfer-audit.md). |
| T338 | Closed the 8086/80186 profile baseline: S2 ALU/FLAGS/conditions, S3 inherited data/control/I/O, and S4 80186 extensions cover every S1 allocation; the retained T328 rule is the only legacy `LOCK` owner. The sole reproduced defect was four incorrect INS/OUTS 80386 guards, corrected to 80186. Protected, 80386DX, and x87 boundaries transfer explicitly. Artifact verification, governance, and 218/218 current-gate passed. [History](../history/M5-T338-8086-80186-profile-closure.md). |
| T337 | Closed shared CPU state and delivery foundations: vector-6 fault delivery/rollback, software and external origin composition, IRQ/NMI/TF priority, frame timing, inhibition, and post-commit task debug trap have one proof/owner. DR6/DR7 transfers to 80386DX; reset/shutdown/triple-fault remains bounded CPU/machine debt. [History](../history/M5-T337-shared-state-delivery.md). |
| T336 | Closed the four-profile CPU coverage audit: the primary/`0F` metadata and dispatch forms, real/protected/VM86 state rows, historic evidence, explicit 80386 exclusions, and open CPU debt now have a single indexed disposition. Every residual has one Queue, TODO, or external owner; the next candidate is shared state and delivery foundations. [History](../history/M5-T336-four-profile-coverage.md). |
| T335 | Closed the xasm public-capacity/failure contract: the product facade now uses byte spans, explicit capacities, `type_status`, and commit-on-success result publication; debugger, firmware, and VM debug callback callers migrated with no raw bypass. A dedicated allocation-injection smoke proves capacity, malformed-input, facade-allocation, and engine-allocation failures preserve caller sentinels; `current-gate` passed 216/216. [History](../history/M5-T335-xasm-capacity-failure-contract.md). |
| T334 | Closed EGA sequencer registration atomicity: memory owns coupled provider/observer validation and publication; VADP allocates before publication; allocation and either registry-capacity failure preserve state, while retry publishes exactly one of each. Strict focused smoke and 215/215 current-gate passed. [History](../history/M5-T334-ega-registration-transaction.md). |
| T333 | Closed retained interactive-input failure handling: all 44 Console/debugger readers are inventoried; 43 debugger calls converge at one private boundary; Console stops before parse/execute; EOF/allocation failure and context reuse are covered by two owner-separated smoke targets. Corrective S4 restored its required `0.5.0333` artifact and identity. Evidence and review are in [history](../history/M5-T333-interactive-input-failure-contract.md). |

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
