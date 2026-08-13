# Project Status

## Current Work

**Idle.** M5 Td S72 is closed. The next ordered Queue candidate requires owner
admission.

## Current Technical Baseline

- **Current developer artifact:** T330 selects `vm-0-5-0330` /
  `build/output/nxvm_0_5_0330.exe`; commit `f8116f99` SHA-256 is
  `926D0F4A2CDB48367C522AD799FF4E9C2DE7AD6BF6C75EDB7889E2D8B9B62CAA`.
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
| T330 | Closed the whole-codebase construction-drift package: one task-transition constructor, FDD/HDD backing/create atomicity, CALL-gate dual-fault preflight convergence, and mandatory CPU state-machine mechanism matrices before P1. Intel-required layouts remain explicit; T330 artifact, focused regressions, 211/211 current-gate, and governance evidence are in [history](../history/M5-T330-width-path-convergence.md). |
| T329 | Closed the bounded Intel 80286/80386 protected task-transition state machine: 16/32-bit direct and task-gate entry, nested CALL/IRET state, incoming LDT images, source-CR3 preflight/incoming-CR3 commit, and TSS post-switch `#DB`. S7 proves target-page fetch, target-TSS `#PF` atomicity, and a target-state restart frame; the 0329 artifact and 211/211 gate result are in [history](../history/M5-T329-task-transition-state-machine.md). |
| T328 | Closed the historical LOCK-prefix legality matrix: 8086/80186 retain transparent valid-next-instruction semantics; 80286 adds protected `CPL <= IOPL`; retained 80386 memory-whitelist behavior stays intact. S2 reconciled the current closure map and ordinary matrix, removing the stale Deferred/TODO transfer without changing the user-owned Queue edit. Register, memory, REP, I/O, #GP frame, strict compile, artifact, and 211/211 gate evidence are in [history](../history/M5-T328-legacy-lock-legality.md). |
| T327 | Closed the current/specialized-gate reconciliation: fast smoke no longer builds classified media targets, and generated CTest/Ninja evidence now proves the full 210 = 15 media + 195 non-media partition, both developer roots, all specialized verifiers, and the aggregate's two roots. [History](../history/M5-T327-current-gate-reconciliation.md). |
| T326 | Closed ordinary protected-mode invalid-opcode delivery: `#UD` now reaches IDT vector 6 with a restartable three-dword no-error-code frame, while the explicit error-code classifier retains vectors 8/10--14/17. Four producer classes, invalid-gate containment, retained VM86 and 210/210 current-gate pass. Real-mode IVT migration transfers. [History](../history/M5-T326-protected-invalid-opcode-delivery.md). |
| T325 | Closed the CPU-native Intel 80386DX paging/translation package: CR0, CR2/CR3, 4-KiB PDE/PTE, U/S/R/W, A/D, cross-page atomicity, delivered `#PF`, no-persistent-cache behavior, and pre-486 `INVLPG #UD` are reconciled. Protected `#UD` delivery, task/VM86 paging, and persistent TLB/TR6/TR7 state transfer explicitly. [History](../history/M5-T325-80386dx-paging-translation.md). |
| T324 | Closed current-test/specialized-gate separation: `run-current-smokes` now runs only the full 209-test CTest smoke selection, `run-current-fast-smokes` retains the 194-test non-media selection, and `verify-current-specialized-gates` owns 46 named verifiers plus a mechanical target-graph check. `current-gates-gcc` composes exactly both roots; all layer baselines and evidence are retained in [T324 history](../history/M5-T324-current-gate-separation.md). |
| T323 | Closed the bounded 80386DX non-task, non-VM86 protection/privilege-transfer composition: direct far transfer, loaded segment rights, 16-bit same/outer gate entry, outer IRET, and parameterized 16-bit call gates now join retained selector, 32-bit, and outer-RETF evidence. The sole S7 serializer correction preflights/copies parameter words. Task/LDT/debug/VM86, paging, legacy LOCK, and x87 retain named boundaries; the 0323 artifact SHA-256 and 209/209 gate result are in the [closure audit](../etc/evidence/t323-protection-privilege-closure-audit.md). |

## Recent Governance

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
