# Project Status

## Current Work

**Active: M5 T330 S4.** Remove the residual unreachable 32-bit task-transition
construction so the T330 plan is the sole private 80386 task-transition model
(Ordinary Mode; corrective reopening approved by the owner).

## M5 T330 S4 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective; T330 is the most recently closed numeric task, and S4 is its next unused subtask. |
| Admission And Approval | Owner approved reopening T330 on 2026-08-12 after the whole-source construction audit found a residual duplicate task-transition model; scope is the first of four ordered corrective S tasks. |
| Objective | Delete the unreachable legacy private 32-bit task-transition plan/commit/wrapper and prove `_ser_task_transition_tss_plan` remains the sole private 80386 task-transition construction path, without changing accepted task-switch behavior. |
| Non-goals | No Intel TSS-layout rewrite, new task-switch semantics, generic descriptor/memory/exception change, call-gate change, media-provider change, or new architecture package. |
| Reference Baseline | `1e0a9f362684ae6de7da1b6e30b105cde2978265` / `vm-0-5-0330`; read current T330 history and width-path inventory before implementation. |
| Files And ABI Surface | `src/core/machine/cpu_instructions.c`, CMake/static-verifier material if needed, T330 evidence/history, and `docs/STATUS.md`; private implementation only, no public ABI or CPU state-layout change. |
| Applicable Rules | `docs/rules/EXECUTION.md` corrective allocation, P lifecycle, similar-issue sweep, and actual-change review; architecture and coding rules; source layout; documentation rules on closure. The invariant is one private construction owner per task-transition mechanism while real 286/386 layouts remain explicit. |
| Verification | Fresh GCC configure; focused `core-machine-task-switch-smoke`; mechanical source closure proving the removed legacy names have no definition/reference and the actual task entry routes converge through `_ser_task_transition_tss_plan`; current-gate; documentation governance; `git diff --check`. |
| Expected Markers | Existing `M5:T330:S1:TASK-TRANSITION:OK` remains green; a committed static verifier, if added, reports its own deterministic success marker. |
| Asset Needs | None; no external source, guest media, firmware, or reference implementation. |
| Reporting Requirements | Ordinary Mode: contract confirmation, then one complete pushed implementation P with requirement-to-proof evidence; coordinator-side actual-change review and a separate governance P on acceptance. |
| Stop Conditions | Stop before changing shared descriptor, memory, exception, or task-entry semantics; stop and revise the packet if deletion exposes a live caller, changes a required Intel fault/order boundary, or needs an external source import. |
| Exit Criteria | The legacy 32-bit plan/commit/wrapper is absent, no task entry bypasses the canonical plan, all retained task-switch evidence and gates pass, and the T330 inventory/history accurately record this corrective closure. |
| Original Owner Request | Reopen T330 and implement the four audited items as four ordered S tasks, beginning now. |
| Similar-Issue Sweep | Audit all task-transition constructors and all callers of task plan/commit helpers in `src/core/machine/cpu_instructions.c`; classify every hit as canonical, deleted, or Intel-required layout helper. |

## Current Technical Baseline

- **Current developer artifact:** T330 selects `vm-0-5-0330` /
  `build/output/nxvm_0_5_0330.exe`; commit `af006a19` SHA-256 is
  `F0AEC7780F2BC0A0AEF6451B91CCD4EE25DF6A9D1EE4A5AFA8D1AD3E58F9707A`.
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
| T330 | Closed the holistic 16/32-bit construction audit: all 286/386 TSS source/target pairs now use an independent-format private plan, the complete 286 LDTR image is saved, 32-bit same-CPL CALL gates accept equal DPL without TSS or outer-frame writes, and `0.5.0330` is the verified current artifact. [History](history/M5-T330-width-path-convergence.md); 211/211 current-gate. |
| T329 | Closed the bounded Intel 80286/80386 protected task-transition state machine: 16/32-bit direct and task-gate entry, nested CALL/IRET state, incoming LDT images, source-CR3 preflight/incoming-CR3 commit, and TSS post-switch `#DB`. S7 proves target-page fetch, target-TSS `#PF` atomicity, and a target-state restart frame; the 0329 artifact and 211/211 gate result are in [history](history/M5-T329-task-transition-state-machine.md). |
| T328 | Closed the historical LOCK-prefix legality matrix: 8086/80186 retain transparent valid-next-instruction semantics; 80286 adds protected `CPL <= IOPL`; retained 80386 memory-whitelist behavior stays intact. S2 reconciled the current closure map and ordinary matrix, removing the stale Deferred/TODO transfer without changing the user-owned Queue edit. Register, memory, REP, I/O, #GP frame, strict compile, artifact, and 211/211 gate evidence are in [history](history/M5-T328-legacy-lock-legality.md). |
| T327 | Closed the current/specialized-gate reconciliation: fast smoke no longer builds classified media targets, and generated CTest/Ninja evidence now proves the full 210 = 15 media + 195 non-media partition, both developer roots, all specialized verifiers, and the aggregate's two roots. [History](history/M5-T327-current-gate-reconciliation.md). |
| T326 | Closed ordinary protected-mode invalid-opcode delivery: `#UD` now reaches IDT vector 6 with a restartable three-dword no-error-code frame, while the explicit error-code classifier retains vectors 8/10--14/17. Four producer classes, invalid-gate containment, retained VM86 and 210/210 current-gate pass. Real-mode IVT migration transfers. [History](history/M5-T326-protected-invalid-opcode-delivery.md). |
| T325 | Closed the CPU-native Intel 80386DX paging/translation package: CR0, CR2/CR3, 4-KiB PDE/PTE, U/S/R/W, A/D, cross-page atomicity, delivered `#PF`, no-persistent-cache behavior, and pre-486 `INVLPG #UD` are reconciled. Protected `#UD` delivery, task/VM86 paging, and persistent TLB/TR6/TR7 state transfer explicitly. [History](history/M5-T325-80386dx-paging-translation.md). |
| T324 | Closed current-test/specialized-gate separation: `run-current-smokes` now runs only the full 209-test CTest smoke selection, `run-current-fast-smokes` retains the 194-test non-media selection, and `verify-current-specialized-gates` owns 46 named verifiers plus a mechanical target-graph check. `current-gates-gcc` composes exactly both roots; all layer baselines and evidence are retained in [T324 history](history/M5-T324-current-gate-separation.md). |
| T323 | Closed the bounded 80386DX non-task, non-VM86 protection/privilege-transfer composition: direct far transfer, loaded segment rights, 16-bit same/outer gate entry, outer IRET, and parameterized 16-bit call gates now join retained selector, 32-bit, and outer-RETF evidence. The sole S7 serializer correction preflights/copies parameter words. Task/LDT/debug/VM86, paging, legacy LOCK, and x87 retain named boundaries; the 0323 artifact SHA-256 and 209/209 gate result are in the [closure audit](etc/evidence/t323-protection-privilege-closure-audit.md). |

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
