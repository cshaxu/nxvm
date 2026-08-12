# Project Status

## Current Work

**M5 T325 S1: correct Intel 80386 CR0 paging-control and supervisor page-write semantics (Ordinary Mode).**

## M5 T325 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New; Ordinary Mode. `M5 T325 S1` is the next linear implementation identifier after closed T324. |
| Admission And Approval | Owner-approved autonomous continuation of the ordered Queue candidate **80386DX paging and translation closure** on 2026-08-12. This S admits only the 80386 CR0/page-protection correction and its focused evidence. |
| Objective | Reconcile the CPU executor with the Intel 80386 CR0 and page-protection contract: make `MP`/`EM`/`TS`/`ET` CR0 controls reachable through `MOV CR0`, retain only the defined PE/PG transition boundary, and remove the post-80386 `CR0.WP` supervisor-write rule from the 80386 page walk. Prove CPL0 still writes read-only pages, while CPL3 U/S and R/W protection remains intact. |
| Non-goals | No PE-clear/re-entry transition, PAE, PSE, 80486 `WP`, CR4, `INVLPG`, persistent-TLB/test-register design, task-switch paging state, page-table ABI, host-memory substitution, public API, x87 execution, VM86 paging breadth, or generic exception/PIC redesign. |
| Reference Baseline | `ed376b82` / `vm-0-5-0323`; retain T258/T311 paging probes and T321 delivered-`#PF` foundation as evidence inputs, not as unreviewed authority. |
| Files And ABI Surface | Expected: `src/core/machine/cpu.h`, `src/core/machine/cpu_instructions.c`, one owner-focused paging/control smoke and its target registration, the paging evidence/closure map, and task history. No public header or product ABI change. |
| Applicable Rules | `docs/design/GOAL.md` goal 6; Queue paging candidate; `docs/etc/evidence/80386-closure-map.md` paging row; Architecture Rules one CPU-executor/page-walk owner; Coding Rules project types and target-local strict compilation; Documentation and Execution Rules packet/P lifecycle. Intel 80386 PRM Chapters 4--6 is form authority. |
| Verification | Focused real/protected CPL0/CPL3 prepared-state smoke; `MOV CR0` control-bit and fault/publication checks; supervisor versus user read-only-page outcomes; retained paging smoke; CMake configure; exact current registration; documentation governance; `git diff --check`; full current-gate CTest and specialized verification aggregate. |
| Expected Markers | New focused marker `M5:T325:S1:CR0-PAGING-CONTROL:OK`; retained T258/T311 paging markers; full current-gate and specialized-verifier success. |
| Asset Needs | None; deterministic in-memory GDT/IDT/page-table fixtures only. |
| Reporting Requirements | Before code, record the CR0/80386 PRM form audit and any material objection. P1 is one complete, pushed implementation delivery with requirement-to-proof evidence; then perform the Ordinary-Mode actual-change review before a pure governance P2. |
| Stop Conditions | Stop before accepting uncertain PE/PG transition semantics, a change to task-switch/TLB/test-register behavior, a public interface, a generic exception delivery policy, or an architecture requirement beyond 80386. A failure requiring any shared route outside the named CR0/page-walk helpers requires a revised packet or later S. |
| Exit Criteria | Intel 80386 `MP`/`EM`/`TS`/`ET` writability and CPL0/CPL3 page-write behavior are explicitly classified and mechanically tested; no 80486 `WP` behavior remains in the 80386 page walker; every touched helper/caller is swept; focused and retained paging regressions, current gate, specialized verifiers, governance, and diff checks pass; evidence names PE-clear, paging/task/debug transfers. |
| Original Owner Request | Continue in single-session mode through the Intel 80386DX architecture-coverage closure audit, prioritizing correct, bounded architectural implementation and evidence. |
| Similar-Issue Sweep | Sweep all CR0 readers/writers, all page-protection branches, direct control-register forms, existing T311 WP claims, and their focused tests; classify test-register/TLB and task-switch consumers rather than silently absorbing them. |

## Current Technical Baseline

- **Current developer artifact:** T323 selects `vm-0-5-0323` /
  `build/output/nxvm_0_5_0323.exe`; its source commit, SHA-256, and bounded
  protection/privilege closure transfers will be recorded at T323 closure.
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
| --- | --- |
| T324 | Closed current-test/specialized-gate separation: `run-current-smokes` now runs only the full 209-test CTest smoke selection, `run-current-fast-smokes` retains the 194-test non-media selection, and `verify-current-specialized-gates` owns 46 named verifiers plus a mechanical target-graph check. `current-gates-gcc` composes exactly both roots; all layer baselines and evidence are retained in [T324 history](history/M5-T324-current-gate-separation.md). |
| T323 | Closed the bounded 80386DX non-task, non-VM86 protection/privilege-transfer composition: direct far transfer, loaded segment rights, 16-bit same/outer gate entry, outer IRET, and parameterized 16-bit call gates now join retained selector, 32-bit, and outer-RETF evidence. The sole S7 serializer correction preflights/copies parameter words. Task/LDT/debug/VM86, paging, legacy LOCK, and x87 retain named boundaries; the 0323 artifact SHA-256 and 209/209 gate result are in the [closure audit](etc/evidence/t323-protection-privilege-closure-audit.md). |
| T322 | Audited and withdrew the duplicate ordinary-execution/FLAGS candidate: T316's accepted S23--S65 owner smokes already cover the transferred Intel 80386 ordinary application forms. Remaining work is explicitly protection/privilege, paging, task/debug/VM86, legacy LOCK, or external x87 scope; no invented implementation slice or artifact was created. Documentation governance and diff checks passed. |
| T321 | Closed the bounded exception, interrupt, return, VM86 table-load, and processor-control composition program: S2 delivers active `#DE/#PF/#MF` vectors; S3 proves NMI/IRQ/TF ordering; S4 composes software INT/IRET with IRQ; S5 enforces VM86 LGDT/LIDT `#GP(0)` before source access; and S6 records the artifact and all transfers. The 0321 artifact, governance, and 202/202 current-gate passed. |
| T320 | Closed the bounded VM86-to-CPL0 32-bit delivery foundation: `#GP/#UD/#NM/IRQ0` entry through TSS `SS0:ESP0`, full VM86 frame and failure boundaries, plus atomic nine-dword CPL0 `IRET` return with a real IRQ0-to-handler-to-VM86 round trip. `66` is classified as non-VM86 return, `67` succeeds, and VME/PVI/task/paging breadth remains transferred. Artifact 0320 SHA-256 is recorded in history; fresh configure, governance/diff checks, and 198/198 current-gate passed. |
| T319 | Closed the bounded non-VM86 LGDT/LIDT `0F 01 /2,/3` table-load slice: protected CPL>0 now rejects before pseudo-descriptor reads, both forms have real/protected, attribute, source-atomicity, table-consumer, and PIC evidence, and VM86 plus 80286 LOCK remain transferred. Its 0319 artifact and 196/196 current-gate pass are retained in history. |
| T318 | Closed the bounded SGDT/SIDT `0F 01 /0,/1 table-store slice: a local six-byte preflight/publication repair prevents partial pseudo-descriptor writes; both forms have profile, attribute, segment, rejection, protected atomicity, VM86 and IRQ-ordering evidence. 195/195 current-gate passed, and executor plus coordinator 0318 artifact hashes are retained in its history. LGDT/LIDT and wider processor-control work remain future bounded packages. |
| T317 | Closed test-corpus quality plus corrective type-vocabulary and developer-feedback work. S7 retains the 194-test current gate while adding validated default-four-job CTest execution, 15 explicit media labels, and a 179-test non-media fast preset; its full gate, fast run, media verifier, documentation governance, and diff check passed. |

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
