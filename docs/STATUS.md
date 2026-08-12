# Project Status

## Current Work

M5 T329 S2: 80386 32-bit TSS direct far-JMP transition (Ordinary Mode).

## M5 T329 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; T329 S1 remains retained task progress. Ordinary Mode performs both implementation and acceptance review. |
| Admission And Approval | Owner approved autonomous single-session progress toward the Intel 80386DX architecture-coverage closure audit on 2026-08-12, and on 2026-08-12 approved the T329 task-switch state-machine discipline: establish the complete bounded model before further production repairs; use Intel as authority and retain Bochs/PCjs only as non-product diagnostic references. This packet continues the open T329 task-system package after S1. |
| Objective | First record and prove the bounded 80386 direct-task-JMP state machine, then implement and prove protected-mode direct far-JMP transfer between valid 32-bit TSS images, with paging disabled and a null incoming LDTR: direct `EA ptr16:16` / `EA ptr16:32` and memory-indirect `FF /5` forms, correctly spaced TSS state save/load, CR3 image transfer, descriptor busy state, `CR0.TS`, segment-cache publication, defined rejections, and the bounded pending-IRQ boundary. |
| Non-goals | 16-bit TSS regression changes; task gates; far CALL-to-TSS; nested-task `IRET` / NT; non-null LDT task images; task switching while paging is enabled; task paging/TLB behavior; task-level debug state; generic exception/IRQ redesign; and any x87 claim. |
| Reference Baseline | `778c35ca` / current `origin/main` after T329 S1 P2; preserve the user-owned uncommitted `docs/QUEUE.md`. |
| Files And ABI Surface | Expected changes are local task-switch execution code, its focused owner smoke/CMake registration, T329 evidence, closure map, and STATUS. No public ABI, provider contract, or product-visible interface may change. |
| Applicable Rules | Read the Task Reading Set; Execution P lifecycle and ordinary-mode self/acceptance review; Architecture single-owner/no-new-interface invariants; Coding project-type/C11/test-boundary rules; Documentation packet/evidence/closure rules. Apply [the T329 task-switch state-machine record](etc/evidence/t329-task-switch-state-machine.md): preflight must resolve every admitted failure before externally visible writes, commit must have no new ordinary fault point, and post-commit execution checks must be pre-proven. Each planned invariant is evidenced by the owner smoke, actual compile command, matrix record, and full current gate. |
| Verification | Fresh GCC configure; focused task-switch smoke with a distinct S2 marker; actual Ninja compile command proving target-local strict GCC flags; exact current-gate registration; documentation governance; `git diff --check`; and full `ctest -L current-gate --output-on-failure -j 4`. |
| Expected Markers | Retain T261 markers and emit `M5:T329:S2:TSS32-JMP:OK`; the exact CTest registration remains `current.core-machine-task-switch-smoke`. |
| Asset Needs | None; deterministic in-memory GDT/TSS/IDT/PIC fixtures only. |
| Reporting Requirements | Before production implementation, record the ordinary-mode contract review and a checkpointed diagnostic baseline that distinguishes preflight failure, commit completion, and first target instruction. Deliver one complete pushed P1 with requirement-to-proof evidence; after actual-change review, record and push the governance P2 closure. Report only a reproducible material blocker or the accepted completion. |
| Stop Conditions | Stop before changing shared paging/TLB, generic segment/exception/IRQ, provider, or public-ABI code; before accepting a non-null LDT or paging-enabled task image; if preflight cannot cover a later post-commit fault; or if the required behavior crosses the S2 direct-JMP state machine. Record/transfer any such finding rather than silently broadening scope. |
| Exit Criteria | The state-machine record maps every S2 preflight, commit, and post-commit invariant to owner evidence. The 80386-only bounded matrix is complete: default and `66h` direct/indirect forms plus `67h` indirect EA and combined form; complete 32-bit outgoing/incoming state including CR3 and FS/GS, busy/TS effects, null-LDTR boundary, defined descriptor/TSS/stack fault atomicity, `LOCK` rejection, and a pending IRQ accepted after a successful incoming-IF transition. No tested failure leaves an unclassified partial TSS or descriptor write. All required verification passes and remaining task-system breadth is explicitly transferred. |
| Original Owner Request | Continue in single-session mode through the Intel 80386DX architecture-coverage closure audit, with rapid but quality-gated implementation, direct commit/push authorization, and an owner-approved holistic construction method rather than incremental symptom patches. |
| Similar-Issue Sweep | Audit `_ser_jmp_far_tss`, its direct/indirect far-JMP callers, all 16/32 TSS field accesses, descriptor busy writes, CR3 loading, preflight/commit boundaries, and existing task-switch fixtures. Every 32-bit TSS hit is fixed, bounded, or transferred with a reason. |

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
| T329 S1 | Closed the bounded 80286/80386 protected direct far-JMP-to-16-bit-TSS matrix: direct/indirect and permitted `66h`/`67h` forms, descriptor/TSS faults, busy and `CR0.TS` publication, local pending-IRQ boundary, target-local strict GCC compile, and 211/211 current-gate evidence are in [T329 S1 evidence](etc/evidence/t329-s1-tss16-direct-jump.md). 32-bit TSS, task gates/CALL/NT, LDT task state, task paging, and broader VM86/debug behavior remain transferred. |
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
