# Project Status

## Current Work

M5 T329 S5: nested task IRET return, IDT task-gate entry, and double-fault
task-chain boundaries (Ordinary Mode).

## M5 T329 S5 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; T329 S1--S4 remain retained task progress. Ordinary Mode performs implementation and acceptance review. |
| Admission And Approval | Owner approved autonomous single-session continuation toward Intel 80386DX architecture closure and the accepted holistic task-switch state-machine method. S5 is the state-return and exceptional-task-entry dependency cut after S4 nested-entry semantics. |
| Objective | Implement and prove protected 80286/80386 nested-task `IRET` return through the outgoing TSS backlink, IDT task-gate task entry, and the bounded double-fault task-gate chain needed to compose those transitions. Each route must reuse the named task-transition planner/commit boundary and preserve architecturally correct busy, NT, TR, TS, frame/restart, and fault ordering semantics. |
| Non-goals | Non-null LDT task images; task paging/TLB; debug state; generic exception/IRQ redesign; ordinary call-gate behavior; arbitrary nested task chains beyond the bounded return/double-fault matrix; VME/PVI; and x87 execution. |
| Reference Baseline | `41692cba` / current `origin/main` after T329 S4 P2; preserve user-owned uncommitted Queue/TODO and debt-evidence changes. |
| Files And ABI Surface | Local task-switch, IRET, IDT task-gate/double-fault execution code, its owner smoke/evidence/closure map and STATUS only. No public ABI, provider contract, or product-visible interface changes. |
| Applicable Rules | Task Reading Set; Intel 80386 architecture as authority; Execution ordinary-mode lifecycle and actual-change review; Architecture/Coding/Documentation rules; accepted [T329 state-machine record](etc/evidence/t329-task-switch-state-machine.md). IRET, IDT task-gate, and double-fault routes must extend the named preflight/plan/commit transition rather than duplicate task state writes. |
| Verification | Fresh GCC configure; focused task-switch marker; actual Ninja target-local strict GCC command; exact current registration; documentation governance; `git diff --check`; full `ctest -L current-gate --output-on-failure -j 16`. |
| Expected Markers | Retain earlier markers and extend `M5:T329:S4:TSS-CALL-GATE:OK` only when task-switch coverage remains a coherent owner smoke; exact registration remains `current.core-machine-task-switch-smoke`. |
| Asset Needs | None; deterministic in-memory GDT/TSS/IDT/PIC fixtures only. |
| Reporting Requirements | Deliver one complete implementation P with an Intel form/state/fault matrix, shared-path audit, and requirement-to-proof evidence; after actual-change review push governance P closure. Report only a reproducible material blocker or accepted completion. |
| Stop Conditions | Stop before shared paging/TLB, generic exception/IRQ, provider, or public-ABI changes; before non-null LDT, paging/debug task state, VME/PVI, or an arbitrary task-chain model; or if IRET/IDT task-gate/double-fault semantics cannot be modeled as preflighted extensions of the accepted planner/commit transition. Transfer rather than silently broaden. |
| Exit Criteria | All admitted nested IRET, IDT task-gate, and bounded double-fault task-chain forms have focused success/fault/attribute/IRQ evidence. Valid routes prove backlink consumption, busy/TR/TS/NT publication and complete image/cache state. Installed fault handlers prove no partial outgoing TSS/descriptor state. S1--S4 direct entry remains green; non-null LDT/paging/debug work is explicitly transferred. |
| Original Owner Request | Complete Intel 80386 with a holistic, maintainable design; avoid incremental symptom patches; commit and push accepted work. |
| Similar-Issue Sweep | Audit `IRET`, `_ser_iret*`, `_ser_task_switch_tss*`, task-gate IDT callers, double-fault producer/delivery paths, all 16/32 TSS busy/backlink/NT updates, and task-switch fixtures. |

## Current Technical Baseline

- **Current developer artifact:** T328 selects `vm-0-5-0328` /
  `build/output/nxvm_0_5_0328.exe`; commit `e5aa9d97` SHA-256 is
  `52D81668FE747C5A1083EE5F9A5C33A5C71F41C5383B299B1728EAD8F523DFEA`.
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
| T329 S4 | Closed protected direct far-CALL and GDT task-gate entry for 16/32-bit TSS images: shared nested transition semantics, backlink/NT, busy/TR/TS, direct/indirect CALL and task-gate JMP/CALL, local `LOCK FF /3` rejection, installed-handler and terminal fault boundaries, and 211/211 current-gate proof. Nested IRET, IDT task gates/double fault, LDT, task paging, and debug remain transferred in [S4 evidence](etc/evidence/t329-s4-task-gate-call-entry.md). |
| T329 S1 | Closed the bounded 80286/80386 protected direct far-JMP-to-16-bit-TSS matrix: direct/indirect and permitted `66h`/`67h` forms, descriptor/TSS faults, busy and `CR0.TS` publication, local pending-IRQ boundary, target-local strict GCC compile, and 211/211 current-gate evidence are in [T329 S1 evidence](etc/evidence/t329-s1-tss16-direct-jump.md). 32-bit TSS, task gates/CALL/NT, LDT task state, task paging, and broader VM86/debug behavior remain transferred. |
| T329 S2 | Closed the bounded 80386 32-bit-TSS direct far-JMP slice: complete outgoing/incoming state including CR3 and FS/GS, `EA`/`FF /5` 66h/67h matrix, descriptor/TSS/stack preflight boundaries, busy/TS, null LDTR, pending IRQ, and direct/indirect LOCK rejection. The targeted `FF /5` LOCK repair leaves the shared prefix policy unchanged; task gates/CALL/NT, non-null LDT, task paging, and debug state remain transferred in [S2 evidence](etc/evidence/t329-s2-tss32-direct-jump.md). |
| T329 S3 | Closed the direct 32-bit TSS image/fault-order contract: named/checked CR3-through-LDTR offsets, full outbound/inbound cache/state proof, and installed `#TS/#GP/#SS` handler checkpoints proving descriptor/TSS/stack preflight failures leave no partial TSS or busy-descriptor write. Task gates/CALL/NT, non-null LDT, task paging, and debug state remain transferred in [S3 evidence](etc/evidence/t329-s3-tss32-image-fault-order.md). |
| --- | --- |
| T328 | Closed the historical LOCK-prefix legality matrix: 8086/80186 retain transparent valid-next-instruction semantics; 80286 adds protected `CPL <= IOPL`; retained 80386 memory-whitelist behavior stays intact. S2 reconciled the current closure map and ordinary matrix, removing the stale Deferred/TODO transfer without changing the user-owned Queue edit. Register, memory, REP, I/O, #GP frame, strict compile, artifact, and 211/211 gate evidence are in [history](history/M5-T328-legacy-lock-legality.md). |
| T327 | Closed the current/specialized-gate reconciliation: fast smoke no longer builds classified media targets, and generated CTest/Ninja evidence now proves the full 210 = 15 media + 195 non-media partition, both developer roots, all specialized verifiers, and the aggregate's two roots. [History](history/M5-T327-current-gate-reconciliation.md). |
| T326 | Closed ordinary protected-mode invalid-opcode delivery: `#UD` now reaches IDT vector 6 with a restartable three-dword no-error-code frame, while the explicit error-code classifier retains vectors 8/10--14/17. Four producer classes, invalid-gate containment, retained VM86 and 210/210 current-gate pass. Real-mode IVT migration transfers. [History](history/M5-T326-protected-invalid-opcode-delivery.md). |
| T325 | Closed the CPU-native Intel 80386DX paging/translation package: CR0, CR2/CR3, 4-KiB PDE/PTE, U/S/R/W, A/D, cross-page atomicity, delivered `#PF`, no-persistent-cache behavior, and pre-486 `INVLPG #UD` are reconciled. Protected `#UD` delivery, task/VM86 paging, and persistent TLB/TR6/TR7 state transfer explicitly. [History](history/M5-T325-80386dx-paging-translation.md). |
| T324 | Closed current-test/specialized-gate separation: `run-current-smokes` now runs only the full 209-test CTest smoke selection, `run-current-fast-smokes` retains the 194-test non-media selection, and `verify-current-specialized-gates` owns 46 named verifiers plus a mechanical target-graph check. `current-gates-gcc` composes exactly both roots; all layer baselines and evidence are retained in [T324 history](history/M5-T324-current-gate-separation.md). |
| T323 | Closed the bounded 80386DX non-task, non-VM86 protection/privilege-transfer composition: direct far transfer, loaded segment rights, 16-bit same/outer gate entry, outer IRET, and parameterized 16-bit call gates now join retained selector, 32-bit, and outer-RETF evidence. The sole S7 serializer correction preflights/copies parameter words. Task/LDT/debug/VM86, paging, legacy LOCK, and x87 retain named boundaries; the 0323 artifact SHA-256 and 209/209 gate result are in the [closure audit](etc/evidence/t323-protection-privilege-closure-audit.md). |
| T322 | Audited and withdrew the duplicate ordinary-execution/FLAGS candidate: T316's accepted S23--S65 owner smokes already cover the transferred Intel 80386 ordinary application forms. Remaining work is explicitly protection/privilege, paging, task/debug/VM86, legacy LOCK, or external x87 scope; no invented implementation slice or artifact was created. Documentation governance and diff checks passed. |
| T321 | Closed the bounded exception, interrupt, return, VM86 table-load, and processor-control composition program: S2 delivers active `#DE/#PF/#MF` vectors; S3 proves NMI/IRQ/TF ordering; S4 composes software INT/IRET with IRQ; S5 enforces VM86 LGDT/LIDT `#GP(0)` before source access; and S6 records the artifact and all transfers. The 0321 artifact, governance, and 202/202 current-gate passed. |

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
