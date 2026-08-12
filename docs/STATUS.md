# Project Status

## Current Work

**Active: M5 T318 S1.** Implement Intel 80386 SGDT/SIDT descriptor-table store
forms in Coordinated Dual-Session Mode.

## M5 T318 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New; T318 is the next linear numeric implementation task after closed T317 and begins at S1. |
| Admission And Approval | Owner's continuing approved objective is complete Intel 80386 implementation in Coordinated Dual-Session Mode; on 2026-08-11 the coordinator selected this first bounded processor-control handoff after T316 S66 and executor's independent read-only audit. Scope is only `0F 01 /0` SGDT and `/1` SIDT stores. |
| Objective | Implement or correct and prove the Intel 80286/80386 SGDT and SIDT memory-only pseudo-descriptor store forms, then establish a truthful matrix/evidence baseline for this first processor-control slice. |
| Non-goals | No LGDT/LIDT (`/2,/3`), SLDT/STR/LLDT/LTR, SMSW/LMSW, MOV CRx, paging/TLB, IDT gate delivery redesign, generic exception/IRQ/NMI work, task/V86 breadth, legacy LOCK-policy change, or 80387 implementation. |
| Reference Baseline | `a8aa729e` / predecessor developer artifact `0.5.0317`; T316 S66 explicitly transferred SGDT/SIDT/LGDT/LIDT to the later processor-control package and withdrew its earlier uncommitted table-register smoke as non-evidence. Current route is `INS_0F_01` cases `/0,/1` and `_d_modrm_table_memory` in `src/core/machine/cpu_instructions.c`. |
| Files And ABI Surface | Expected: local CPU handler only if a reproduced SGDT/SIDT defect requires it; one owner smoke under `tests/machine/`, CMake standalone/current-gate registration, T318 matrix/history evidence, `STATUS.md`, and current task artifact `0.5.0318`. No public API, ABI, provider, or test-support-to-production dependency. |
| Applicable Rules | `docs/rules/EXECUTION.md`, `docs/rules/CODING.md`, `docs/rules/ARCHITECTURE.md`, `docs/rules/DOCUMENT.md`, `docs/design/CODING.md`, `docs/design/ARCHITECTURE.md`, `docs/design/ROADMAP.md`, `docs/QUEUE.md`, `docs/history/M5-T316-ordinary-execution-closure.md`, and `docs/etc/evidence/t316-ordinary-execution-matrix.md`. Intel 80386 PRM SGDT/SIDT form semantics are the behavioral authority. |
| Verification | Audit all SGDT/SIDT forms and table-store route; add owner-focused smoke; verify 80286/80386 real and protected behavior, operand/address attributes, exact memory image/EIP/state preservation, ModRM/prefix/LOCK rejection, memory-limit atomicity, and no-shadow IRQ ordering; configure GCC, build focused target, register exact current-gate target, run documentation governance, diff check, full current gates, rebuild/copy `0.5.0318`, record SHA-256/runtime identity, and push. |
| Expected Markers | One deterministic `M5:T318:S1:SGDT-SIDT:OK` owner marker; exact current-gate registration; evidence distinguishes 16-bit operand base representation from 32-bit form, and reports every matrix form as proved, rejected, or explicitly transferred. |
| Asset Needs | None; deterministic CPU fixtures, local GCC/Ninja/CMake, and current-gate inputs only. |
| Reporting Requirements | Executor first confirms or materially objects after its route/PRM/evidence review. It then returns only one complete, self-reviewed, committed and pushed P1 or a reproducible material blocker; no partial test, fixture, registration, documentation, or diagnostic delivery. It reports no intermediate progress unless a material contract change or blocker occurs. Coordinator independently reviews pushed code/evidence and alone accepts/closes the S. |
| Stop Conditions | Stop for a required shared descriptor-memory, decoder, exception/interrupt-delivery, paging, or legacy-LOCK policy change; a semantic ambiguity requiring wider Intel authority; an unsafe state-publication change affecting `/2`--`/6`; or any need to include mutable LGDT/LIDT semantics. Report exact route/caller sweep and request a revised or later S rather than broadening silently. |
| Exit Criteria | Every admitted `/0,/1` form is classified against Intel authority and has focused evidence: memory-only ModRM, 80286/80386 profile/mode behavior, 16/32 operand and address attributes, six-byte pseudo-descriptor image, DS/SS and admitted override addressing, rejection/no-publication, protected write-boundary atomicity, and pending-PIC no-shadow. Any defect receives a bounded sweep/static prevention where mechanically suitable. The exact owner smoke, current-gate, artifact, evidence, independent coordinator review, governance closure, commit, and push all pass. |
| Original Owner Request | Continue the complete Intel 80386 plan in dual-session mode, using an instruction-form to implementation to test matrix, repairing omissions, performing differential/boundary review, and closing each package with evidence. |
| Similar-Issue Sweep | Inspect every `INS_0F_01` subform and `_d_modrm_table_memory` caller, all table-register focused tests/CMake registrations, 0F metadata/profile routing, table-store memory writes, and existing T316 transfer/evidence records. Fix only SGDT/SIDT hits; classify `/2`--`/6` and broader shared paths as retained later-package work unless an approved revision is required. |

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
