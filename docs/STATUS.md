# Project Status

## Current Work

**Active.** M5 T320 S2 implements the bounded CPL0 32-bit `IRET` return to
VM86 in Coordinated Dual-Session Mode.

## M5 T320 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; M5 T320 S2 follows retained T320 S1 progress and must be its exact next committed subtask. |
| Admission And Approval | The owner approved continued holistic 80386 implementation on 2026-08-11 and authorized the bounded inverse VM86 delivery slice after S1. Td S68 repaired the prerequisite identifier gate. |
| Objective | Implement and prove Intel 80386 CPL0 32-bit `IRET` return to VM86 from a complete nine-dword return frame, including atomic VM86 EIP/CS/EFLAGS/ESP/SS/ES/DS/FS restoration and a direct S1-entry-to-handler-IRET round trip. |
| Non-goals | No VM86-origin IRET, VME/PVI, NT/task return, task switch, paging, call gates, 16-bit-gate breadth, generic interrupt/PIC redesign, arbitrary protected outer return, or x87 work. |
| Reference Baseline | `6b5d294a`; T320 S1 accepted VM86-to-CPL0 32-bit delivery and retains its compact progress row. Artifact remains `vm-0-5-0320`. |
| Files And ABI Surface | Expected local CPU IRET/segment-cache path in `src/core/machine/cpu_instructions.c`; one owner smoke under `tests/machine/`; CMake registration; T320 matrix/history/Status evidence. No public API or provider ABI change without a revised packet. |
| Applicable Rules | Task Reading Set; execution, architecture, coding, documentation, source-layout, and 80386 PRM return-frame/VM86 rules. Intel form audit is mandatory before claim or repair. |
| Verification | Owner smoke: manual valid frames and S1 handler round trip; all nine fields/cache semantics; 66/67 classification; controlled invalid frame/stack/selector atomic boundaries; post-return VM86 execution and pending-IRQ ordering. Fresh configure, exact registration, documentation governance, diff check, full current gate, artifact rebuild/hash/history record. |
| Expected Markers | Owner marker; returned VM86 code executes with real-mode-style segment caches; invalid return does not partially publish VM86 state; current gate passes. |
| Asset Needs | None; synthetic local GDT/IDT/TSS/VM86 fixtures only. |
| Reporting Requirements | Executor first confirms or raises a material contract objection, then returns one complete pushed P1 only. It maps every packet row to actual evidence, changed files, shared-caller impact, gate results, residual boundaries, and artifact facts. Coordinator independently reviews before governance acceptance. |
| Stop Conditions | Stop for a required generic IRET/interrupt/segment helper redesign, a VM86 contract that cannot be supported without VME/PVI or task-switch scope, undocumented frame semantics, nonlocal caller effects needing a broader sweep, or unavailable required facility. |
| Exit Criteria | Valid CPL0 32-bit return to VM86 is proven by manual and S1 round-trip frames, preserves/restores all declared state atomically, every accepted/rejected form has truthful classification, tests/gates/artifact evidence pass, and excluded breadth is explicitly retained. |
| Original Owner Request | Continue the 80386 program holistically and prioritize the VM86 exception/IRQ delivery foundation, with correct bounded task decomposition. |
| Similar-Issue Sweep | Audit all `_e_iret` VM86 branches, protected outer-return helpers, segment-load/cache paths, S1 direct consumers, frame-width/prefix handling, and every changed shared caller; fix, retain, or defer each hit explicitly. |

## Current Technical Baseline

- **Current developer artifact:** T320 selects `vm-0-5-0320` /
  `build/output/nxvm_0_5_0320.exe`; its commit, SHA-256, runtime identity, and
  retained boundaries are in [T320 history](history/M5-T320-vm86-delivery.md).
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
| T320 S1 | Accepted VM86-to-CPL0 32-bit interrupt-gate delivery for `#GP`, `#UD`, `#NM`, and IRQ0: TSS `SS0:ESP0`, full VM86 frame, gate/cache effects, and invalid-facility boundaries; direct consumer disposition is retained in T320 evidence. Artifact `0320` SHA-256 is recorded in history; independent fresh configure, governance/diff checks, and 197/197 current-gate passed. CPL0 `IRET` return remains T320 S2. |
| T319 | Closed the bounded non-VM86 LGDT/LIDT `0F 01 /2,/3` table-load slice: protected CPL>0 now rejects before pseudo-descriptor reads, both forms have real/protected, attribute, source-atomicity, table-consumer, and PIC evidence, and VM86 plus 80286 LOCK remain transferred. Its 0319 artifact and 196/196 current-gate pass are retained in history. |
| T318 | Closed the bounded SGDT/SIDT `0F 01 /0,/1 table-store slice: a local six-byte preflight/publication repair prevents partial pseudo-descriptor writes; both forms have profile, attribute, segment, rejection, protected atomicity, VM86 and IRQ-ordering evidence. 195/195 current-gate passed, and executor plus coordinator 0318 artifact hashes are retained in its history. LGDT/LIDT and wider processor-control work remain future bounded packages. |
| T317 | Closed test-corpus quality plus corrective type-vocabulary and developer-feedback work. S7 retains the 194-test current gate while adding validated default-four-job CTest execution, 15 explicit media labels, and a 179-test non-media fast preset; its full gate, fast run, media verifier, documentation governance, and diff check passed. |
| T316 | Closed the declared 80386 ordinary-execution slices through S66. Residual parent-family breadth remains explicitly transferred in the S66 matrix to named later Queue packages, the legacy LOCK TODO, or the external-coprocessor boundary; no whole-80386 claim. Its predecessor developer artifact and 194 current-gate tests passed. |
| T314 | Closed the EGA CRTC 13h bound, registered the deterministic xasm smoke, restored six historical target-local strict GCC sets while retaining three selected libraries, and added the CRTC static closure verifier; the two inherited-governance debts remain. Its 0315 artifact, 52 gates, 147 CTests, and S2/S3/S6 markers passed. |
| T313 | Completed construction-failure atomicity and startup-failure visibility: single RAM allocation, transactional port/controller assembly, visible session start failure, and explicit debugger mappings. Its accepted artifact, 51 gates, 145 CTests, and S2--S6 focused markers passed. |
| T307 | Completed 80386 32-bit CPL3-to-CPL0 IDT and call-gate entry with TSS32 target stacks, preflighted atomic publication, and Intel-correct target-SS `#TS` classification. Its 0307 artifact, 51 gates, 137 CTests, and 9 focused/retained probes passed; product observation remained host-window limited before guest input. |
| T308 | Completed protected 32-bit same/outer error delivery and bounded contributory `#DF` containment through the existing route. Its 0308 artifact, 51 gates, and 137 CTests passed; triple-fault shutdown/reset remains deferred. |

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
