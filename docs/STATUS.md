# Project Status

## Current Work

**Active.** M5 T321 S6 reconciles the bounded processor-control closure,
builds the T321 developer artifact, and performs the task-level closure audit
in Ordinary Mode.

## M5 T321 S6 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | The owner authorized continued single-session implementation toward the 80386DX architecture-coverage closure audit on 2026-08-12. T321 S1 defines S6 as the final bounded processor-control reconciliation and task closure. |
| Objective | Reconcile the S2--S5 exception, event, return, table, CR0/MSW/CLTS, and CPU-side ESC/WAIT evidence with their state-consumer boundaries; classify every remaining T321 row as complete or transferred; build and record the required `0.5.0321` developer artifact; and close T321 only if the audit is truthful. |
| Non-goals | No new paging policy, MOV CR implementation, task/LDT/debug/test-register implementation, VME/PVI, x87 numerical execution, 80186/80286 LOCK policy, device redesign, or Windows claim. |
| Reference Baseline | `40da2307`; T321 S1 audit, S2--S5 evidence, T304/T316 S61--S65, T318--T320 records, 80386 closure map, Queue, and TODO ledger. |
| Files And ABI Surface | Current artifact target/version only; closure-map, Queue, Status, T321 history and indexed evidence. Production source changes are prohibited unless the final audit reproduces an in-scope unclassified T321 control/delivery defect; any such finding requires a packet revision. |
| Applicable Rules | Task Reading Set; execution, architecture, coding, and documentation rules; artifact identity and milestone-closure requirements; Intel 80386 control, table, exception, and interrupt forms; all cited owner evidence. |
| Verification | Re-audit exact current producers/routes/forms and their owner smokes; build `vm-0-5-0321`, run artifact-target verification and SHA-256, fresh configure, documentation governance, diff check, and full current gate. Map every T321 objective to evidence or an explicit Queue/TODO/external boundary. |
| Expected Markers | Existing S2--S5 markers remain green; current developer artifact is `build/output/nxvm_0_5_0321.exe` with SHA-256 and runtime identity in T321 history. |
| Asset Needs | No new assets. Build output only; no firmware, guest media, or external source/provider import. |
| Reporting Requirements | Record row-by-row closure/transfer evidence, source and test sweep, artifact commit/hash/banner, gates, remaining Queue order, and any rejected closure claim. |
| Stop Conditions | Stop and revise for a new in-scope production defect, an incomplete or contradictory transfer, an artifact build/identity failure, or any required change beyond current artifact/closure documentation. |
| Exit Criteria | All T321-bounded rows are evidenced or explicitly transferred; no partial/unclassified row remains under T321 ownership; artifact and full gates pass; history/Queue/closure map/Status agree; commit and push complete. |
| Original Owner Request | Complete the owner-approved Intel 80386DX architecture program by matrix, implementation, test, boundary review, and governed evidence closure. |
| Similar-Issue Sweep | Sweep all cited S2--S5 routes and current producer masks; the current artifact CMake target; closure-map/Queue/TODO transfers; and every retained T321 evidence link. |

## Current Technical Baseline

- **Current developer artifact:** T321 selects `vm-0-5-0321` /
  `build/output/nxvm_0_5_0321.exe`; its SHA-256, runtime identity, and
  bounded closure transfers are in
  [T321 S6 evidence](etc/evidence/t321-s6-processor-control-closure.md).
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
| T321 S5 | Closed the transferred VM86 LGDT/LIDT boundary: `/2` and `/3` now raise `#GP(0)` before ModRM/source access; exact ten-dword VM86 error-frame, source/table nonpublication, and CPL0 handler transfer are proven. Non-VM86 table behavior remains T319; no table/TSS/PIC refactor. 202/202 current-gate passed. |
| T321 S4 | Closed bounded software/return composition: `INT 31h` enters an IRET handler while IRQ0 is pending; IRET consumes the software frame, restores IF, and IRQ0 then enters with saved IP 2 and the restored FLAGS. Retained S50--S52/T320 evidence owns individual INT, gate, IRET, outer-return, and VM86 matrices. No production or PIC change; 201/201 current-gate passed. |
| T321 S3 | Closed the bounded shared hardware-delivery composition: a strict owner smoke proves NMI priority over IRQ0 and TF in real, protected, and VM86 entry, plus the VM86 NMI-mask/IRQ acknowledgement boundary; retained T305/T320/T316 evidence covers detailed gate, rejection, frame, and post-instruction TF cells. No PIC, NMI-device, or production event-loop change; 200/200 current-gate passed. |
| T321 S2 | Closed exact active `#DE/#PF/#MF` producer delivery: `ExecFinal` now selects vectors 0/14/16 (including vector-zero admission), real `#DE/#MF` and protected `#PF` frames have direct producer evidence, and retained no-handler paging cases document the IDT-preflight PDE accessed-bit effect. New strict owner smoke and 199/199 current-gate passed; paging policy, x87 execution, IRQ composition, and other exception classes remain transferred. |
| T321 S1 | Accepted the Intel/source/evidence closure audit: completed VM86 delivery was removed from Queue and marked complete in the closure map; all active exception-producer and processor-control intersections now have an explicit next-S or later-Queue destination. S2 owns the missing `#DE/#PF/#MF` delivery closure. Documentation governance and diff check passed. |
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
