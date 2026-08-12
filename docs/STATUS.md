# Project Status

## Current Work

**Active.** M5 T321 S1 audits and sequences the 80386DX exception, interrupt,
return, and processor-control closure in Ordinary Mode.

## M5 T321 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner approved single-session execution toward the 80386DX architecture-coverage closure audit on 2026-08-12. T320 has a task-level closure row, so T321 S1 is the next numeric implementation task and its explicit design prerequisite. The scope is the first Queue processor-control candidate, not paging, protection-transfer breadth, task/debug state, or x87 execution. |
| Objective | Reconcile the remaining Intel 80386DX exception, interrupt, return, and processor-control forms with current source and focused evidence; update the closure map and Queue; produce the ordered, bounded T321 implementation-S breakdown. |
| Non-goals | No CPU/runtime behavior, CMake, smoke, artifact, paging, protection/privilege-transfer, task/LDT/debug/test-register, VME/PVI, or x87 numerical-unit change. Do not claim whole-80386 closure. |
| Reference Baseline | `2e49804d` / `vm-0-5-0320`; T320's developer artifact SHA-256 is retained in `docs/history/M5-T320-vm86-delivery.md`. |
| Files And ABI Surface | `docs/STATUS.md`, `docs/QUEUE.md`, `docs/etc/evidence/80386-closure-map.md`, and new T321 supporting audit evidence only. No ABI or runtime surface. |
| Applicable Rules | `docs/rules/EXECUTION.md`, `docs/rules/DOCUMENT.md`, `docs/design/ROADMAP.md`, `docs/QUEUE.md`, `docs/etc/operations/policy/source-policy.md`, and the Intel 80386 PRM form audit required by Td S63. |
| Verification | Re-read every named authority; inspect `ExecFinal`, exception producers, interrupt/return/table/control routes, current focused tests, and retained T304/T308/T318/T319/T320 evidence; run documentation governance and `git diff --check`. |
| Expected Markers | Documentation governance passes. The audit names no runtime marker because S1 changes no runnable path. |
| Asset Needs | None. Intel documentation is a read-only requirements source; no source, firmware, binary, media, trace, or third-party implementation is imported. |
| Reporting Requirements | Record the PRM/source/evidence crosswalk, every complete/partial/missing/explicit-boundary classification, the next-S dependency order, and any stop condition in an indexed evidence record. |
| Stop Conditions | Stop and request owner direction if the audit requires a paging, protection/privilege-transfer, task/debug, VME/PVI, or x87 execution commitment; do not move such work into T321 silently. |
| Exit Criteria | Queue no longer lists the completed VM86 delivery foundation; the closure map records T320 as complete; every T321-owned remaining row has a route/test disposition and exactly one next T321 S or later Queue destination; no partial/missing item is hidden by a broad family claim. |
| Original Owner Request | "请你以单人模式开始执行到80386DX 架构覆盖收口审计". |
| Similar-Issue Sweep | Documentation-only prerequisite; inspect all tracked CPU exception masks, producer sites, dispatcher mappings, system-control routes, focused tests, Queue rows, and closure-map rows. No defect is fixed in S1. |

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
| T320 | Closed the bounded VM86-to-CPL0 32-bit delivery foundation: `#GP/#UD/#NM/IRQ0` entry through TSS `SS0:ESP0`, full VM86 frame and failure boundaries, plus atomic nine-dword CPL0 `IRET` return with a real IRQ0-to-handler-to-VM86 round trip. `66` is classified as non-VM86 return, `67` succeeds, and VME/PVI/task/paging breadth remains transferred. Artifact 0320 SHA-256 is recorded in history; fresh configure, governance/diff checks, and 198/198 current-gate passed. |
| T319 | Closed the bounded non-VM86 LGDT/LIDT `0F 01 /2,/3` table-load slice: protected CPL>0 now rejects before pseudo-descriptor reads, both forms have real/protected, attribute, source-atomicity, table-consumer, and PIC evidence, and VM86 plus 80286 LOCK remain transferred. Its 0319 artifact and 196/196 current-gate pass are retained in history. |
| T318 | Closed the bounded SGDT/SIDT `0F 01 /0,/1 table-store slice: a local six-byte preflight/publication repair prevents partial pseudo-descriptor writes; both forms have profile, attribute, segment, rejection, protected atomicity, VM86 and IRQ-ordering evidence. 195/195 current-gate passed, and executor plus coordinator 0318 artifact hashes are retained in its history. LGDT/LIDT and wider processor-control work remain future bounded packages. |
| T317 | Closed test-corpus quality plus corrective type-vocabulary and developer-feedback work. S7 retains the 194-test current gate while adding validated default-four-job CTest execution, 15 explicit media labels, and a 179-test non-media fast preset; its full gate, fast run, media verifier, documentation governance, and diff check passed. |
| T316 | Closed the declared 80386 ordinary-execution slices through S66. Residual parent-family breadth remains explicitly transferred in the S66 matrix to named later Queue packages, the legacy LOCK TODO, or the external-coprocessor boundary; no whole-80386 claim. Its predecessor developer artifact and 194 current-gate tests passed. |
| T314 | Closed the EGA CRTC 13h bound, registered the deterministic xasm smoke, restored six historical target-local strict GCC sets while retaining three selected libraries, and added the CRTC static closure verifier; the two inherited-governance debts remain. Its 0315 artifact, 52 gates, 147 CTests, and S2/S3/S6 markers passed. |
| T313 | Completed construction-failure atomicity and startup-failure visibility: single RAM allocation, transactional port/controller assembly, visible session start failure, and explicit debugger mappings. Its accepted artifact, 51 gates, 145 CTests, and S2--S6 focused markers passed. |
| T307 | Completed 80386 32-bit CPL3-to-CPL0 IDT and call-gate entry with TSS32 target stacks, preflighted atomic publication, and Intel-correct target-SS `#TS` classification. Its 0307 artifact, 51 gates, 137 CTests, and 9 focused/retained probes passed; product observation remained host-window limited before guest input. |

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
