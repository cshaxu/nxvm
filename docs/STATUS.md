# Project Status

## Current Work

**Active: M5 T317 S5.** Migrate the repository to the project fixed-width type
vocabulary in Coordinated Dual-Session Mode.

## M5 T317 S5 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective; the owner approved an in-scope global type-vocabulary repair under the closed-task exception for the most recently closed numeric T317. |
| Admission And Approval | Owner approved this S5 package on 2026-08-11 after review of the 240-file, 4,138-spelling baseline. The approved scope is repository-wide direct fixed-width vocabulary migration with only the explicit type-facade and controlled negative-fixture lexical exceptions. |
| Objective | Replace direct standard fixed-width type spellings in all applicable tracked C, header, CMake, and PowerShell code with project type vocabulary, preserve behavior and ABI, establish a repository-wide static verifier with positive and negative self-checks, and record every exception or deferred unsafe inherited file. |
| Non-goals | No CPU feature, x87, device, timing, product behavior, public API design, test-support-to-production dependency, broad refactor, or warning-policy change; no repository-wide claim for non-fixed-width C-library, platform SDK, or atomic vocabulary not named by the baseline. |
| Reference Baseline | `63e84740` / predecessor 0.5.0316 artifact; baseline is 240 tracked code/script files and 4,138 direct fixed-width spellings: 124 `src/` files / 2,085, 114 `tests/` files / 2,044, and two build/validation scripts / nine. |
| Files And ABI Surface | Expected tracked `src/`, `tests/`, `cmake/`, `tools/`, `src/type.h`, CMake verification/evidence, TODO/history/STATUS, and the required T317 `0.5.0317` developer artifact record. No new public interface and no `src/` include of `tests/support`. |
| Applicable Rules | `docs/rules/EXECUTION.md`, `docs/rules/CODING.md`, `docs/rules/DOCUMENT.md`, `docs/rules/ARCHITECTURE.md`, `docs/design/CODING.md`, `docs/design/ARCHITECTURE.md`, and the T317 history/plan. |
| Verification | Fix and record the exact baseline; compile all affected targets under GCC; run source-equivalence/ABI review appropriate to each substitution; run new global positive/negative static verifier, strict and existing T317 type gates, documentation governance, diff check, full current-gates-gcc, and rebuild/record the T317 0.5.0317 developer artifact SHA-256. |
| Expected Markers | Deterministic global fixed-width vocabulary verifier reports zero forbidden direct spellings outside the exact exception list; its positive and negative self-checks pass; evidence records pre/post file/token counts and every per-file deferment or exception. |
| Asset Needs | None; deterministic local source, CMake, GCC/Ninja, and current-gate inputs only. |
| Reporting Requirements | Executor confirms or materially objects, then returns only one complete committed/pushed P1 or reproducible material blocker. P1 must map every baseline group, lexical exception, changed validator behavior, deferred inherited file, artifact result, and verification result. Coordinator alone accepts and re-closes S5/T317. |
| Stop Conditions | Stop if a replacement changes ABI, native/platform interop, atomic representation, generated-source contract, or runtime behavior; if a fixed-width use lacks a proven project-type equivalent; if the type facade or negative fixture needs a broader exception; or if a legacy file cannot be safely migrated without a named TODO admission path. |
| Exit Criteria | The exact global baseline is zero outside `src/type.h` foundational aliases and explicitly controlled negative validation fixtures; every exception/deferment is enumerated; global verifier self-checks pass; no `src/` include of `tests/support`; ABI/source equivalence and all required gates/artifact checks pass; pushed implementation passes coordinator review and corrective T317 closure. |
| Original Owner Request | Expand T317's prior local vocabulary rule to the whole repository: cover `src/`, `tests/`, and applicable build/verification scripts; use fixed baseline and equivalent migration; add positive/negative static validation; list unsafe inherited files individually rather than blanket-exempting them. |
| Similar-Issue Sweep | Scan all tracked C, headers, CMake, and PowerShell code for exact-width, least/fast, max, and pointer-width standard integer spellings; inspect type facade, project aliases, validators, fixtures, and generated/artifact paths; record each hit as migrated, foundational exception, controlled validation input, or a per-file TODO. |
## Current Technical Baseline

- **Current task artifact:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0317` / `build/output/nxvm_0_5_0317.exe`.
  S5 rebuilt this developer artifact as SHA-256
  `A7EC7165730E2B037C24693E1E6B0EBFA6C67B9126F9CD5D5863A62DCA963F24`;
  its source commit is finalized only by the coordinator's subsequent acceptance commit.
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
