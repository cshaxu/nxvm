# Project Status

## Current Work

**Active.** M5 T319 S1 is implementing the bounded Intel 80286/80386 LGDT/LIDT
`0F 01 /2,/3` table-load slice in Coordinated Dual-Session Mode.

## M5 T319 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New; M5 T319 S1; Coordinated Dual-Session Mode; implementation is delegated only to the existing `executor` session and independently reviewed by the coordinator. |
| Admission And Approval | The owner's active complete-80386 goal authorizes continued bounded matrix work. Coordinator admission on 2026-08-11 accepts only this corrected T319 slice after the read-only PRM/erratum and route audit; 80286 LOCK behavior is explicitly transferred to the retained legacy-LOCK policy debt rather than silently weakened. |
| Objective | Implement or correct and prove Intel 80286/80386 `LGDT m16&24/m16&32` (`0F 01 /2`) and `LIDT m16&24/m16&32` (`0F 01 /3`): memory-only six-byte table loads in real mode and protected CPL0, protected CPL>0 and VM86 `#GP(0)` rejection before source-memory publication, 80386 operand/address attributes, and 80386 LOCK rejection. |
| Non-goals | SGDT/SIDT, SMSW/LMSW, SLDT/STR/LLDT/LTR, `MOV CRx`, paging, generic prefix/legacy-LOCK policy redesign, generic interrupt/exception redesign, task switching, broad VM86 work, and x87 implementation. |
| Reference Baseline | `98e0a732` / `vm-0-5-0318`; T318 closed only the paired SGDT/SIDT stores and remains retained evidence, not proof of this load slice. |
| Files And ABI Surface | May change only the local `INS_0F_01` `/2,/3` paths and/or `_s_load_gdtr`/`_s_load_idtr` if a focused defect proves it, a new owner smoke, CMake registration, T319 evidence/history, STATUS, and current 0319 artifact wiring. No public ABI, provider, shared memory/paging, generic decoder, or global prefix-policy change. |
| Applicable Rules | Task Reading Set; `docs/rules/EXECUTION.md`; `docs/rules/CODING.md`; `docs/rules/DOCUMENT.md`; Intel 80386 PRM LGDT/LIDT entry as corrected by Intel's VM86 documentation change; target-local strict GCC and project type vocabulary rules. |
| Verification | Fresh GCC configure; focused owner marker; exact current-gate discovery; real/protected/VM86 and table-consumer vectors for both opcodes; documentation governance; `git diff --check`; current artifact rebuild/runtime identity/hash; full `current-gates-gcc`; commit and push. |
| Expected Markers | A new deterministic `M5:T319:S1:LGDT-LIDT:OK` marker; exactly one `current.core-machine-lgdt-lidt-smoke` registration; full current-gate pass. |
| Asset Needs | None; deterministic local CPU, GDT/IDT/TSS/PIC fixtures only. |
| Reporting Requirements | Executor supplies one complete P1 report with form-to-vector evidence, changed-code and caller sweep, all command results, artifact identity/hash, commit and push. Coordinator reads actual code/tests/docs and independently reruns proportionate gates before accepting or issuing one consolidated corrective P brief. |
| Stop Conditions | Stop rather than broaden if 80286 LOCK correctness requires `PREFIX_LOCK` or global legacy policy; if privilege/VM86 rejection needs generic decode, segment translation, fault priority, or delivery changes; if source atomicity needs shared memory/paging changes; if new table consumers require interrupt/exception redesign; or if any `/0,/1,/4-/6` or unrelated descriptor/task/control form must change. |
| Exit Criteria | For each `/2` and `/3`: 80286/80386 real and protected CPL0 success; protected CPL>0 and 80386 VM86 `#GP(0)` with no source read/table publication; 16/32 operand and 80386 `67`/combined EA layouts; DS, SS-default, and one override source selection; memory-only direct-ModRM/rejected profiles/80386 LOCK boundaries; source-limit and table-state atomicity; no-shadow PIC success; a bounded post-load GDTR/IDTR consumer; all required gates, artifact evidence, commit, and push. 80286 LOCK must be explicitly recorded as transferred, not claimed. |
| Original Owner Request | Continue the complete 80386 plan in dual-session mode, with Intel form-to-implementation-to-test evidence, bounded governance, code quality, and commit/push authority. |
| Similar-Issue Sweep | Inspect both `/2` and `/3` symmetrically; sweep only their direct helper/caller and downstream bounded GDT/IDT consumers. Confirm no change leaks to T318 stores, `/4-/6`, generic prefix policy, or shared exception/memory architecture. |
## Current Technical Baseline

- **Current task artifact:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0318` / `build/output/nxvm_0_5_0318.exe`.
  T318 S1 records the source commit, SHA-256, and runtime identity in
  [its history](history/M5-T318-sgdt-sidt.md).
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
| T318 | Closed the bounded SGDT/SIDT `0F 01 /0,/1 table-store slice: a local six-byte preflight/publication repair prevents partial pseudo-descriptor writes; both forms have profile, attribute, segment, rejection, protected atomicity, VM86 and IRQ-ordering evidence. 195/195 current-gate passed, and executor plus coordinator 0318 artifact hashes are retained in its history. LGDT/LIDT and wider processor-control work remain future bounded packages. |
| T317 | Closed test-corpus quality plus corrective type-vocabulary and developer-feedback work. S7 retains the 194-test current gate while adding validated default-four-job CTest execution, 15 explicit media labels, and a 179-test non-media fast preset; its full gate, fast run, media verifier, documentation governance, and diff check passed. |
| T316 | Closed the declared 80386 ordinary-execution slices through S66. Residual parent-family breadth remains explicitly transferred in the S66 matrix to named later Queue packages, the legacy LOCK TODO, or the external-coprocessor boundary; no whole-80386 claim. Its predecessor developer artifact and 194 current-gate tests passed. |
| T314 | Closed the EGA CRTC 13h bound, registered the deterministic xasm smoke, restored six historical target-local strict GCC sets while retaining three selected libraries, and added the CRTC static closure verifier; the two inherited-governance debts remain. Its 0315 artifact, 52 gates, 147 CTests, and S2/S3/S6 markers passed. |
| T313 | Completed construction-failure atomicity and startup-failure visibility: single RAM allocation, transactional port/controller assembly, visible session start failure, and explicit debugger mappings. Its accepted artifact, 51 gates, 145 CTests, and S2--S6 focused markers passed. |
| T307 | Completed 80386 32-bit CPL3-to-CPL0 IDT and call-gate entry with TSS32 target stacks, preflighted atomic publication, and Intel-correct target-SS `#TS` classification. Its 0307 artifact, 51 gates, 137 CTests, and 9 focused/retained probes passed; product observation remained host-window limited before guest input. |
| T308 | Completed protected 32-bit same/outer error delivery and bounded contributory `#DF` containment through the existing route. Its 0308 artifact, 51 gates, and 137 CTests passed; triple-fault shutdown/reset remains deferred. |
| T309 | Completed the form-level 80386 audit and bounded trace-design record without runtime or artifact change; it selected T310 through evidence rather than a completeness claim. |

## Recent Governance

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

- **M5 Td S60:** routed every DOS 5-bound smoke contract through the explicit
  M1 FDD image while retaining the current MS-DOS 6.22 FDD and EWIN31 Setup
  HDD for their applicable checks. Fresh configuration restored 145/145 current
  gates without weakening old assertions or claiming broad DOS 6.22 support.

- **M5 Td S59:** established the repository-relative current-media root,
  approved-media identities, and protected-media change controls. Its initial
  DOS 5 contract mismatch was classified and corrected by S60.
