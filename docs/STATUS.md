# Project Status

## Current Work

**Active.** M5 T321 S4 closes the bounded 80386 software-interrupt, trap, and
return-composition matrix in Ordinary Mode.

## M5 T321 S4 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | The owner authorized continued single-session implementation toward the 80386DX architecture-coverage closure audit on 2026-08-12. T321 S1 ordered this composition work after accepted S2 and S3. |
| Objective | Reconcile the already bounded `INT3`, `INT ib`, `INTO`, and same/outer `IRET` paths with shared external-event composition: software versus external origin, interrupt/trap gate IF/TF behavior, return restoration versus pending IRQ ordering, and fault/return frame separation. |
| Non-goals | No expansion of the individual INT/INTO/IRET encoding matrices, no VM86 IRET return reopening, no PIC/NMI redesign, no task gate/switch, VME/PVI, debug/breakpoint policy, paging policy, exception recursion, or x87 execution. |
| Reference Baseline | `dc8496f2` / `vm-0-5-0320`; T321 S1 audit, accepted S2/S3 evidence, retained T305/T308 entry evidence, T316 S50--S52 owner smokes, and T320 VM86 return evidence. |
| Files And ABI Surface | An S4 owner smoke, indexed evidence, CMake registration, and Status; existing retained tests only if an assertion must be made precise. `cpu_instructions.c` only for a reproduced composition defect with complete affected-caller sweep. No public interface, PIC-provider, or product change. |
| Applicable Rules | Task Reading Set; `docs/rules/EXECUTION.md`, `docs/rules/ARCHITECTURE.md`, `docs/rules/CODING.md`, `docs/rules/DOCUMENT.md`, Intel 80386 interrupt/trap and IRET rules, T305/T308 evidence, T316 S50--S52 evidence, T320 matrix, and T321 S1 audit. |
| Verification | Audit `_e_int3`, `_e_int_n`, `_e_into`, `_e_iret`, `_e_intr_n`, serializers, and the retained software/return smokes. Add deterministic composition evidence for a pending IRQ across a successful software-interrupt/IRET sequence; verify saved IP, frame order, IF/TF behavior, source acknowledgement, and no error-code confusion. Fresh configure; exact current registration; documentation governance; diff check; full current gate. |
| Expected Markers | One deterministic S4 owner marker and exactly one current-gate registration. Evidence names which retained owner proves each form and distinguishes software, external, and fault frames. |
| Asset Needs | None; deterministic local CPU/IVT/IDT/stack/PIC fixtures only. No guest media, firmware, source import, or external provider. |
| Reporting Requirements | Record source sweep, reused form evidence, the newly exercised composition sequence, frame and FLAGS/acknowledgement results, any changed caller, and transferred boundaries in indexed evidence. Before implementation, confirm this matrix or report a material objection. |
| Stop Conditions | Stop for a required generic event-loop/PIC/NMI redesign, a VM86/task/VME/PVI expansion, exception-recursion/triple-fault policy, paging change, or shared serializer change beyond the admitted software/return callers. |
| Exit Criteria | The admitted software/trap/return composition is directly proven, retained form owners are correctly cross-linked, external event acknowledgement and frame/FLAGS order are exact, no unrelated route changes, and evidence, gates, commit, and push pass. |
| Original Owner Request | Continue single-session implementation through the 80386DX architecture-coverage closure audit with code quality preserved. |
| Similar-Issue Sweep | Search `_e_int3`, `_e_int_n`, `_e_into`, `_e_iret`, `_e_intr_n`, `ExecInt`, serializers, and all T305/T316 S50--S52/T320 focused evidence; classify every relevant route as covered, fixed, retained, or transferred. |

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
