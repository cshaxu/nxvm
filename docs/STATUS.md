# Project Status

## Current Work

**Active.** M5 T321 S2 closes the bounded 80386 `#DE`, `#PF`, and `#MF`
exception-vector and frame-delivery gap in Ordinary Mode.

## M5 T321 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | The owner authorized continued single-session 80386DX architecture coverage on 2026-08-12. Accepted T321 S1 identified this shared exception-delivery gap and ordered S2 before dependent IRQ/control consumers. |
| Objective | Implement and prove Intel 80386 delivery for the existing `#DE`, `#PF`, and `#MF` producers: exact vectors 0, 14, and 16; real/protected mode disposition; error-code versus no-error-code frame shape; restart/trap state; and failed-delivery containment. |
| Non-goals | No paging policy or translation change, no x87/80287/80387 numerical execution, no debug/breakpoint/overflow/NMI policy, no task switch, VME/PVI, reset/shutdown/triple-fault policy, generic PIC redesign, or new public/provider ABI. |
| Reference Baseline | `2f18e0bb` / `vm-0-5-0320`; T321 S1 audit in `docs/etc/evidence/t321-s1-processor-control-closure-audit.md`. |
| Files And ABI Surface | `src/core/machine/cpu_instructions.c` only for proven shared finalizer/delivery fixes; a new owner smoke; direct producer regressions `core_machine_inc_dec_smoke.c`, `core_machine_80386_paging_smoke.c`, `core_machine_fpu_interface_s65_smoke.c`, and `core_machine_fpu_8087_smoke.c` only where their former terminal expectation must become an installed-vector delivery assertion; CMake registration/artifact wiring, T321 evidence/history, and Status. No shared paging walk, FPU provider, memory, or public API change. |
| Applicable Rules | Task Reading Set; `docs/rules/EXECUTION.md`, `docs/rules/ARCHITECTURE.md`, `docs/rules/CODING.md`, `docs/rules/DOCUMENT.md`, project source layout, Intel 80386 exceptions/interrupts and page-fault frame rules, and T308 retained delivery evidence. |
| Verification | Focused real/protected vectors for divide error, page fault, and WAIT pending FPU error; exact IDT entry, diagnostic, saved frame and error-code checks; failure-preflight preservation; migrate each named direct producer regression only to its equivalent installed-vector contract; direct producer/caller sweep; fresh configure; exact current registration; artifact build/SHA-256; documentation governance; diff check; full current gate. |
| Expected Markers | New deterministic owner marker and exactly one current-gate registration. `#DE` and `#MF` frames contain saved EIP/CS/EFLAGS only; `#PF` adds its unchanged producer error code below that frame. |
| Asset Needs | None; deterministic local CPU/GDT/IDT/stack/page/FPU-provider fixtures only. No source, firmware, guest media, or external provider import. |
| Reporting Requirements | Record every producer, exact vector, frame layout, real/protected outcome, failed-delivery disposition, modified caller, and excluded architecture boundary in indexed evidence. Before implementation, confirm this matrix or report a material objection. |
| Stop Conditions | Stop for a required paging-walker change, FPU provider ABI/numerical implementation, generic interrupt/PIC redesign, debug/NMI/trap policy, task/VM86 expansion, or a double/triple-fault/reset contract beyond retained T308 behavior. |
| Exit Criteria | Every current `#DE`, `#PF`, and `#MF` producer has an exact delivery/terminal disposition; valid real/protected vectors prove correct frame, restart/error-code, gate, and handler behavior; invalid delivery proves contained no-partial publication; no unrelated exception class changes; required artifact, evidence, gates, commit, and push pass. |
| Original Owner Request | Continue in Ordinary Mode toward the 80386DX architecture-coverage closure audit. |
| Similar-Issue Sweep | Search all `_SetExcept_DE/_PF/_MF` producers, `ExecFinal` exact-mask dispatch, `_e_except_n` error-frame selection, real/protected interrupt planners, paging/fpu focused tests, and every changed helper caller. |

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
