# Project Status

## Current Work

**M5 T341 S1 - active.** Allocate the complete 80386DX system-state ledger
before admitting any isolated control, VM86, paging, task, or debug repair.

## M5 T341 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | The owner approved Queue-ordered, single-agent implementation through the four-profile cross-closure verification on 2026-08-13. This is the next Queue candidate after closed T340. |
| Objective | Build the finite 80386DX privileged-state ledger: CR0/CR2/CR3, DR0--DR7, TR/LDT/table forms, VM86 entry/exit and delivery, paging translation/fault composition, task/TSS transitions, and breakpoint/vector-1 delivery. Map every state owner, reader/writer, validation, materialization, preflight, commit, rollback, frame/stack, and fault/delivery boundary; allocate only genuinely unresolved mechanism rows to later T341 S work. |
| Non-goals | No isolated opcode repair, generic interrupt/paging/task rewrite, VME/PVI, 486+ features, persistent TLB, test-register model, x87 execution, timing, device, Windows, product, artifact, or rules change. Existing accepted evidence is not reimplemented merely to increase smoke count. |
| Reference Baseline | `0e7af96b` (`M5 T340 S4 P2: close 80386DX form package`), the Queue proposal, T320/T321/T325/T326/T329/T337 evidence, T339/T340 transfer audits, T336 four-profile closure record, current TODO, and the 80386DX closure map. |
| Candidate Proposal | [80386DX system, VM86, paging, and debug closure](../proposals/m5-80386dx-system-state-closure.md), [80386DX extended-state closure context](../proposals/m5-80386dx-extended-state-closure.md), [four-profile CPU-completeness program](../proposals/m5-four-profile-cpu-completeness-program.md), and [80386DX admission policy](../proposals/m5-80386dx-candidate-policy.md). |
| Files And ABI Surface | Expected: T341 history, indexed state-allocation evidence, Current, and only documentation necessary to retain exact transfers. No production, test, CMake, public ABI, artifact, device, or rule change in S1. |
| Applicable Rules | `docs/rules/EXECUTION.md`: allocation, mechanism sweep, actual-change review, and task lifecycle. `docs/rules/ARCHITECTURE.md`: one mutable-state owner and explicit semantic/layout differences. `docs/rules/CODING.md`: no duplicate abstraction. `docs/rules/DOCUMENT.md`: Queue/Current/history/evidence topology. |
| Verification | Inspect actual execution routes and all relevant current-gate owners; compare the Intel 80386 state/form matrix with T320/T321/T325/T326/T329/T337 records and current TODO. Run exact focused retained state tests, documentation governance, and diff check. |
| Expected Markers | One indexed ledger classifies every T341 row as accepted retained proof, a bounded T341 mechanism owner, or a single external/post-80386 TODO. It names all readers/writers and prevents later S work from using a local opcode smoke as a state-closure claim. |
| Asset Needs | Intel 80386 manuals already cited by retained evidence. No guest media, firmware, third-party source, binary, or trace. |
| Reporting Requirements | Deliver one complete pushed allocation P with the full row-to-owner table, exact static/source and focused-test evidence, later S sequence, transfers, and any material contract objection. |
| Stop Conditions | Stop if a required row has ambiguous ownership, an unproved shared delivery/paging/task mechanism, an incompatible Intel layout, or requires an external source/import. Do not assign an implementation S until the owner and complete validation-to-commit boundary are known. |
| Exit Criteria | Every assigned state/form row is classified once; every unresolved row has a bounded T341 owner with a complete mechanism matrix; accepted evidence and external boundaries are exact; history/index/Current satisfy governance; focused retained proof, governance, and diff checks pass; the allocation P is committed and pushed. |
| Original Owner Request | Implement the Queue in order, in single-agent mode and with a holistic view, through four-profile cross-closure verification. |
| Similar-Issue Sweep | Sweep all tracked CPU execution, machine state, current-gate registration, prior state evidence, Queue, and TODO references for CR/DR/TR, VM86, task/TSS, paging, table, and breakpoint/vector-1 construction. Record every hit's owner or transfer; no broad repository rewrite. |

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
