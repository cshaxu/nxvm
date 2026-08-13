# Project Status

## Current Work

M5 T330 S1 - task-transition source/target-width plan convergence (Ordinary
Mode). Complete the whole-file construction audit, then repair the task
transition mechanism's proven 286/386 source/target-format coupling without
erasing Intel-required TSS layouts.

## M5 T330 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New; Ordinary Mode; owner-authorized T330 S1. |
| Admission And Approval | Owner approved T330 on 2026-08-12 after the holistic 16/32-bit task-transition divergence review, then explicitly expanded it to all analogous accidental construction divergences in `cpu_instructions.c`. It consumes the Queue's holistic-audit and task-switch-unification debts; no prior task history is rewritten. |
| Objective | Establish the semantic inventory of 16/32-bit execution-path pairs and other width-derived sibling routes in `cpu_instructions.c`; distinguish Intel-required layout/semantic differences from accidental duplicated validation, materialization, preflight, commit, fault ordering, or atomicity. A syntactically different pair belongs to the same audit when it implements the same architectural mechanism. S1 repairs the proven task-transition divergence with one private plan/commit vocabulary that chooses the outgoing and incoming TSS formats independently, preserves their actual layouts, writes the complete 286 image including LDTR, and proves all four old-to-new TSS-width pairs. The separately bounded call-gate privilege-rule drift is reserved for a following T330 S2, not silently omitted. |
| Non-goals | No blanket conversion of every width-specific handler into a framework; no public CPU-state or generic transaction interface; no x87 execution, VME/PVI, ordinary debug-register/breakpoint, generic paging/TLB, general exception/IRQ redesign, arbitrary task-chain, double-fault reset, TSS I/O bitmap, or Windows behavior claim. Retain every Intel-required width-specific layout and accepted behavior unless a focused reproducer proves a defect. |
| Reference Baseline | `2f5d3b66` / `vm-0-5-0329`; Queue's holistic-execution-path and task-transition-unification debts; T329 state-machine history; and the Intel 80286/80386 form/state requirements used to classify each audited pair. |
| Files And ABI Surface | Expected private CPU implementation, targeted owner smokes/evidence/Status/closure-map/history updates: principally `src/core/machine/cpu_instructions.c` and existing focused tests, with a new smoke/CMake registration only when an existing owner cannot prove the contract. No public ABI or provider interface change. |
| Applicable Rules | Task Reading Set; `docs/rules/EXECUTION.md`, `docs/rules/ARCHITECTURE.md`, `docs/rules/CODING.md`, `docs/design/ARCHITECTURE.md`, `docs/design/CODING.md`, `docs/rules/DOCUMENT.md`; preserve checked-memory, descriptor/cache ownership, and project type vocabulary. |
| Verification | Commit an inventory that maps every relevant width-parallel execution family and its caller/write/validation/fault/commit boundary. For S1, prove each old-to-new TSS-width pair for direct `JMP`, nested `CALL`/task-gate entry, and bounded nested `IRET` return as applicable, including target-image read, complete old-image write including LDTR, backlink, descriptor-write fault boundaries, and the 386-only CR3/FS/GS/VM/T-bit dispositions. Run configure, exact current registration, documentation governance, diff check, and full current-gate. |
| Expected Markers | Retain accepted markers, including `M5:T329:S1:TSS16-JMP:OK` through `M5:T329:S7:TASK-PAGING-DEBUG:OK`; add T330 owner marker(s) and inventory evidence naming every audited family, its classification, and any caller/fault disposition. |
| Asset Needs | No guest media, firmware, external source import, or external network asset. |
| Reporting Requirements | Report only a complete P1 delivery or a material stop condition; each report maps callers, images/writes, validation/fault/commit boundary, actual changed code, and verification. |
| Stop Conditions | Stop for a required public interface, shared checked-memory/descriptor/paging/exception redesign, an Intel rule requiring an unmodeled partial-publication boundary, a pair whose provenance or architecture rule cannot be established, or an audit finding that requires an unrelated state-family package; record evidence and request owner direction rather than symptom-patching. |
| Exit Criteria | The inventory classifies every relevant width-parallel execution family as shared-mechanism retained, Intel-required divergent, separately admitted, or outside scope with a precise reason. One private task-transition vocabulary selects outgoing and incoming layouts independently; all its callers use it; width-specific TSS readers/writers retain true layouts; all four width pairs and entry/return semantics have focused success/fault evidence. The identified 32-bit call-gate same-CPL rule remains explicitly queued for T330 S2. S1 has a caller/write/fault sweep, excluded breadth is transferred accurately, and all gates pass. |
| Original Owner Request | Begin the 16/32-bit CPU task-transition execution-path divergence review now, map both widths/callers/writes/validation/fault/commit boundaries, distinguish Intel layout differences from accidental divergence, and preserve the active-packet rule. Owner authorized T330, then clarified that it must address this technical debt across all analogous CPU instruction code. |
| Similar-Issue Sweep | Inventory all `cpu_instructions.c` semantic sibling routes that differ by TSS, gate, operand, address, stack, or frame width, not merely similarly named functions. For every candidate, sweep callers, checked-memory/descriptor writes, CPU/cache publication, and exception/IRQ boundaries. The mandatory first sweep covers all `_ser_task_transition_tss` callers and every `_s_write_tss`, `_s_write_xdt`, backlink, TR/LDTR/cache, CR3, `CR0.TS`, and TSS debug-trap publication boundary. |

## Current Technical Baseline

- **Current developer artifact:** T329 selects `vm-0-5-0329` /
  `build/output/nxvm_0_5_0329.exe`; commit `ae91e592` SHA-256 is
  `87982567ACDAC83253A8F6102330F5976B150F2C5AD3CB926D0D6BE4AA41C069`.
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
| T329 | Closed the bounded Intel 80286/80386 protected task-transition state machine: 16/32-bit direct and task-gate entry, nested CALL/IRET state, incoming LDT images, source-CR3 preflight/incoming-CR3 commit, and TSS post-switch `#DB`. S7 proves target-page fetch, target-TSS `#PF` atomicity, and a target-state restart frame; the 0329 artifact and 211/211 gate result are in [history](history/M5-T329-task-transition-state-machine.md). |
| T328 | Closed the historical LOCK-prefix legality matrix: 8086/80186 retain transparent valid-next-instruction semantics; 80286 adds protected `CPL <= IOPL`; retained 80386 memory-whitelist behavior stays intact. S2 reconciled the current closure map and ordinary matrix, removing the stale Deferred/TODO transfer without changing the user-owned Queue edit. Register, memory, REP, I/O, #GP frame, strict compile, artifact, and 211/211 gate evidence are in [history](history/M5-T328-legacy-lock-legality.md). |
| T327 | Closed the current/specialized-gate reconciliation: fast smoke no longer builds classified media targets, and generated CTest/Ninja evidence now proves the full 210 = 15 media + 195 non-media partition, both developer roots, all specialized verifiers, and the aggregate's two roots. [History](history/M5-T327-current-gate-reconciliation.md). |
| T326 | Closed ordinary protected-mode invalid-opcode delivery: `#UD` now reaches IDT vector 6 with a restartable three-dword no-error-code frame, while the explicit error-code classifier retains vectors 8/10--14/17. Four producer classes, invalid-gate containment, retained VM86 and 210/210 current-gate pass. Real-mode IVT migration transfers. [History](history/M5-T326-protected-invalid-opcode-delivery.md). |
| T325 | Closed the CPU-native Intel 80386DX paging/translation package: CR0, CR2/CR3, 4-KiB PDE/PTE, U/S/R/W, A/D, cross-page atomicity, delivered `#PF`, no-persistent-cache behavior, and pre-486 `INVLPG #UD` are reconciled. Protected `#UD` delivery, task/VM86 paging, and persistent TLB/TR6/TR7 state transfer explicitly. [History](history/M5-T325-80386dx-paging-translation.md). |
| T324 | Closed current-test/specialized-gate separation: `run-current-smokes` now runs only the full 209-test CTest smoke selection, `run-current-fast-smokes` retains the 194-test non-media selection, and `verify-current-specialized-gates` owns 46 named verifiers plus a mechanical target-graph check. `current-gates-gcc` composes exactly both roots; all layer baselines and evidence are retained in [T324 history](history/M5-T324-current-gate-separation.md). |
| T323 | Closed the bounded 80386DX non-task, non-VM86 protection/privilege-transfer composition: direct far transfer, loaded segment rights, 16-bit same/outer gate entry, outer IRET, and parameterized 16-bit call gates now join retained selector, 32-bit, and outer-RETF evidence. The sole S7 serializer correction preflights/copies parameter words. Task/LDT/debug/VM86, paging, legacy LOCK, and x87 retain named boundaries; the 0323 artifact SHA-256 and 209/209 gate result are in the [closure audit](etc/evidence/t323-protection-privilege-closure-audit.md). |
| T322 | Audited and withdrew the duplicate ordinary-execution/FLAGS candidate: T316's accepted S23--S65 owner smokes already cover the transferred Intel 80386 ordinary application forms. Remaining work is explicitly protection/privilege, paging, task/debug/VM86, legacy LOCK, or external x87 scope; no invented implementation slice or artifact was created. Documentation governance and diff checks passed. |

## Recent Governance

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

- **M5 Td S65:** aligned active numeric-task progress retention with task-level closure consolidation, added narrow structural checker coverage (including retained-progress self-tests), and preserved the eight-row cap for task-level closures. Td work has no runtime or artifact change.

- **M5 Td S64:** requires an actual-change review before a task or subtask is
  accepted: reports, test summaries, and diff statistics are evidence indexes,
  not substitutes for reading the relevant changed code, build, test, and
  documentation artifacts. In dual-session mode the coordinator owns that
  independent review. Td work has no runtime or artifact change.

- **M5 Td S63:** made Intel 80386 PRM form audits a per-candidate admission
  input and exit gate. A bounded task can close only its declared matrix slice;
  a family cannot close while any in-scope form is partial, missing, or
  unclassified. Td work has no runtime or artifact change.

- **M5 Td S62:** recorded the owner-approved Intel 80386DX
  architecture-completeness program before M6, replaced the closed T314 Queue
  candidate with its ordered family sequence, scoped 80386-to-x87 coupling
  without admitting an 80387, and recorded the required reuse and coverage
  discipline. Td work has no runtime or artifact change.

- **M5 Td S61:** restored Queue dependency order by placing residual M5 work
  before M6 mantle candidates; removed stale package/index detail; and changed
  historical evidence to name closure artifacts rather than current state.
