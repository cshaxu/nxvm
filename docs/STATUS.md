# Project Status

## Current Work

M5 T323 S1 is active: close the protected-mode direct far `CALL`/`JMP`
code-descriptor privilege and atomicity matrix (Ordinary Mode).

## M5 T323 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner authorization to continue the ordered Intel 80386DX Queue program in Ordinary Mode, 2026-08-12. The coordinator performs the executor-side implementation and independent acceptance review sequentially. This is the first bounded slice of Queue P1, 80386DX protection and privilege-transfer closure. |
| Objective | Complete the Intel 80286/80386 protected, non-VM86 direct far code-segment transfer matrix for `CALL ptr16:16/32`, `JMP ptr16:16/32`, and their memory-indirect `FF /3` and `FF /5` forms: descriptor type/present/DPL/RPL classification, code-limit preflight, CS/cache and CALL-frame publication, and fault atomicity. |
| Non-goals | Call gates, task gates/TSS transfers, outer-privilege `RETF`/`IRET`, interrupt/exception redesign, VM86, paging, general stack-helper redesign, near transfers, and product/guest behavior are outside S1. Existing T303 same-CPL smoke is retained evidence, not a substitute for this matrix. |
| Reference Baseline | `8c1da3ec` / `vm-0-5-0321`; T303, T307, T320, T321, and T322 history/evidence are the retained boundary records. |
| Files And ABI Surface | Expected: one owner-local `tests/machine` smoke, target-local CMake/current-gate registration, T323 protection evidence, STATUS, and (only if a reproduced local defect requires it) `src/core/machine/cpu_instructions.c`. No public ABI, provider interface, product composition, or shared helper change is admitted by default. |
| Applicable Rules | Read the Task Reading Set; `docs/design/ARCHITECTURE.md`, `docs/design/CODING.md`, `docs/rules/ARCHITECTURE.md`, `docs/rules/CODING.md`, and `docs/rules/DOCUMENT.md`. Intel 80386 PRM direct far-transfer rules are the semantic authority; no reference implementation source is imported. |
| Verification | New focused owner marker; configure; exact CTest current-gate discovery; `ctest --test-dir build\\mingw-gcc-x64 -L current-gate --output-on-failure -j 4`; documentation governance; `git diff --check`; and actual command/evidence review. Test 80286/80386 protected same-CPL nonconforming and conforming success/deny paths, immediate and memory-indirect forms, 16/32 operand and 80386 address attributes, pre-386 attributes, LOCK, selector/type/present/limit and stack-write/read boundaries, plus successful pending-IRQ no-shadow. |
| Expected Markers | A deterministic `M5:T323:S1:PROTECTED-FAR:OK` marker and exactly one `current.core-machine-protected-far-s1-smoke` current-gate item. |
| Asset Needs | No firmware, guest media, network source, or non-repository asset. |
| Reporting Requirements | Before implementation, perform the Ordinary-Mode contract confirmation. The single implementation P must include all source/test/CMake/evidence and verification while retaining this active packet; report a material stop condition immediately. After self-review, commit and push; then independently re-review the actual pushed changes before the separate governance P records acceptance and closes S1. |
| Stop Conditions | Stop before any call-gate/task/outer-return/VM86/paging or generic exception/stack-helper change. A reproduced need to alter a shared descriptor, memory, or delivery helper requires a caller sweep and packet revision or later S; do not broaden silently. |
| Exit Criteria | Every declared form and mode is either focused-proven or explicitly classified at its stated boundary; all failure paths prove no unintended CS/cache/EIP/ESP/EFLAGS/frame publication at the observable boundary; no in-scope PRM row remains partial, missing, or unclassified; current-gate and documentation checks pass; evidence records production change or no-defect result; and the coordinator completes actual-change acceptance and a pushed governance closure P. |
| Original Owner Request | Continue the owner-approved Intel 80386 implementation program in single-session mode through the 80386DX architecture coverage closure audit, with code quality and complete evidence. |
| Similar-Issue Sweep | Audit all `_ser_call_far_cs_*`, `_ser_jmp_far_cs_*`, `_e_call_far`, `_e_jmp_far`, and `FF /3,/5` callers. Record direct code-descriptor coverage separately from call-gate/task and outer-return consumers; any equivalent defect outside this direct-transfer matrix is transferred rather than silently fixed. |

## Current Technical Baseline

- **Current developer artifact:** T321 selects `vm-0-5-0321` /
  `build/output/nxvm_0_5_0321.exe`; its SHA-256, runtime identity, and
  bounded closure transfers are in
  [T321 history](history/M5-T321-exception-processor-control-closure.md).
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
| T322 | Audited and withdrew the duplicate ordinary-execution/FLAGS candidate: T316's accepted S23--S65 owner smokes already cover the transferred Intel 80386 ordinary application forms. Remaining work is explicitly protection/privilege, paging, task/debug/VM86, legacy LOCK, or external x87 scope; no invented implementation slice or artifact was created. Documentation governance and diff checks passed. |
| T321 | Closed the bounded exception, interrupt, return, VM86 table-load, and processor-control composition program: S2 delivers active `#DE/#PF/#MF` vectors; S3 proves NMI/IRQ/TF ordering; S4 composes software INT/IRET with IRQ; S5 enforces VM86 LGDT/LIDT `#GP(0)` before source access; and S6 records the artifact and all transfers. The 0321 artifact, governance, and 202/202 current-gate passed. |
| T320 | Closed the bounded VM86-to-CPL0 32-bit delivery foundation: `#GP/#UD/#NM/IRQ0` entry through TSS `SS0:ESP0`, full VM86 frame and failure boundaries, plus atomic nine-dword CPL0 `IRET` return with a real IRQ0-to-handler-to-VM86 round trip. `66` is classified as non-VM86 return, `67` succeeds, and VME/PVI/task/paging breadth remains transferred. Artifact 0320 SHA-256 is recorded in history; fresh configure, governance/diff checks, and 198/198 current-gate passed. |
| T319 | Closed the bounded non-VM86 LGDT/LIDT `0F 01 /2,/3` table-load slice: protected CPL>0 now rejects before pseudo-descriptor reads, both forms have real/protected, attribute, source-atomicity, table-consumer, and PIC evidence, and VM86 plus 80286 LOCK remain transferred. Its 0319 artifact and 196/196 current-gate pass are retained in history. |
| T318 | Closed the bounded SGDT/SIDT `0F 01 /0,/1 table-store slice: a local six-byte preflight/publication repair prevents partial pseudo-descriptor writes; both forms have profile, attribute, segment, rejection, protected atomicity, VM86 and IRQ-ordering evidence. 195/195 current-gate passed, and executor plus coordinator 0318 artifact hashes are retained in its history. LGDT/LIDT and wider processor-control work remain future bounded packages. |
| T317 | Closed test-corpus quality plus corrective type-vocabulary and developer-feedback work. S7 retains the 194-test current gate while adding validated default-four-job CTest execution, 15 explicit media labels, and a 179-test non-media fast preset; its full gate, fast run, media verifier, documentation governance, and diff check passed. |
| T316 | Closed the declared 80386 ordinary-execution slices through S66. Residual parent-family breadth remains explicitly transferred in the S66 matrix to named later Queue packages, the legacy LOCK TODO, or the external-coprocessor boundary; no whole-80386 claim. Its predecessor developer artifact and 194 current-gate tests passed. |
| T314 | Closed the EGA CRTC 13h bound, registered the deterministic xasm smoke, restored six historical target-local strict GCC sets while retaining three selected libraries, and added the CRTC static closure verifier; the two inherited-governance debts remain. Its 0315 artifact, 52 gates, 147 CTests, and S2/S3/S6 markers passed. |

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
