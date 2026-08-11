# Project Status

## Current Work

**Active: M5 T317 S1.** Apply target-local strict GCC compilation to every
T316-added CPU smoke target in Coordinated Dual-Session Mode.

## M5 T317 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New; T317 is the owner-approved test-corpus quality corrective package following closed T316. |
| Admission And Approval | Owner approved the audit findings and the ordered T317 plan in `docs/etc/evidence/t317-test-corpus-quality-plan.md`; this is S1 of four planned quality-corrective slices. |
| Objective | Apply `-Wall -Wextra -Wpedantic -Werror` as GCC target-local options to all 47 T316-added `tests/machine` smoke executable targets, without implying dependency or global coverage. |
| Non-goals | No global compiler flags; no options on inherited production targets; no CPU/runtime behavior, test semantics, type migration, fixture refactor, public ABI, or target-graph redesign. |
| Reference Baseline | `aeeefce6` / `vm-0-5-0316`; the 47-target audited inventory in the T317 plan is authoritative. |
| Files And ABI Surface | Expected `CMakeLists.txt`, T317 evidence, and `STATUS.md`; test-source changes are prohibited in S1. |
| Applicable Rules | `docs/rules/EXECUTION.md`, `docs/rules/CODING.md`, `docs/rules/DOCUMENT.md`, `docs/design/CODING.md`, and the T317 plan. |
| Verification | Fresh GCC configure; build all 47 targets; actual Ninja command audit verifies each target includes all four options; exact target inventory, documentation governance, diff check, and full current-gate. |
| Expected Markers | A deterministic S1 strict-coverage marker or auditable command matrix for all 47 targets; no runtime marker changes. |
| Asset Needs | None; deterministic local GCC/Ninja build only. |
| Reporting Requirements | Executor first confirms or materially objects, then creates/updates the S1 completion goal. Return only a complete committed/pushed P1 or a reproducible material blocker; no per-target partial reports. |
| Stop Conditions | Stop for a target not reliably attributable to T316, a warning that requires inherited/product changes, or any need for global/transitive flag policy; report the exact target and command. |
| Exit Criteria | All 47 named T316-added smoke targets show the four options in actual Ninja GCC compile commands; no unrelated target loses or gains policy; all required gates pass; implementation P is committed/pushed and passes coordinator review and governance closure. |
| Original Owner Request | Repair the proven strict-GCC, project-type, and fixture-quality gaps as a governed T317 package before resuming ordinary 80386 capability work. |
| Similar-Issue Sweep | Compare every T316-added smoke source to its executable target and current-gate registration; inspect every existing strict-option block for unintended target scope. |
## Current Technical Baseline

- **Current task artifact:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0316` / `build/output/nxvm_0_5_0316.exe`.
  T316 S5 owns this developer artifact; its source commit is finalized only by
  the coordinator's subsequent acceptance commit.
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
| T316 | Closed the declared 80386 ordinary-execution slices through S66. Residual parent-family breadth remains explicitly transferred in the S66 matrix to named later Queue packages, the legacy LOCK TODO, or the external-coprocessor boundary; no whole-80386 claim. Artifact `nxvm_0_5_0316.exe` SHA-256 `672F11D9174B910836F9FF02BC31025C064DE9F7D5F1667A31443D1FEB96AB3E`; 194 current-gate tests passed. |
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
