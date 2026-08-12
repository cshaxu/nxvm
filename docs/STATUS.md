# Project Status

## Current Work

## M5 T317 S7 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective; Ordinary Mode single-session execution for the retained T317 test-corpus-quality task. |
| Admission And Approval | Owner approved on 2026-08-11: implement the audited low-risk developer-efficiency improvements in single-agent mode. Scope is configurable parallel current-gate execution, explicit media classification, and a non-media fast smoke entry point. No exceptions. |
| Objective | Reduce local feedback time without weakening current coverage: run the complete current-gate suite with a validated, configurable CTest job count; label media-dependent smokes; and provide an explicit fast non-media smoke build preset. |
| Non-goals | Do not remove, merge, weaken, reorder for semantic effect, or change assertions/timeouts of existing smokes; do not alter emulator runtime behavior, media assets, architecture, product artifact, or inherited monoliths. |
| Reference Baseline | `61771bf341bb2d6536565abfe6066ade7a3db926` on `main`; clean worktree; current-gate is 194 CTests and serial evidence totals 147.27 seconds. Historical T317 evidence records `ctest -L current-gate --output-on-failure -j 4` passing 194/194. |
| Files And ABI Surface | `CMakeLists.txt`, `CMakePresets.json`, `docs/STATUS.md`, `docs/history/M5-T317-test-corpus-quality.md`, and `docs/etc/evidence/t317-s7-developer-smoke-efficiency.md`; build/test registration only, with no C/C++ ABI surface. |
| Applicable Rules | `docs/design/ARCHITECTURE.md`: build composition remains explicit and host/media boundaries remain visible. `docs/design/CODING.md`: CMake owns target/test registration. `docs/rules/ARCHITECTURE.md`: preserve explicit dependency direction and machine/media ownership. `docs/rules/CODING.md`: keep build configuration deterministic and fail invalid inputs. `docs/rules/DOCUMENT.md` and `docs/rules/EXECUTION.md`: one active packet, evidence, actual-change review, and closure audit. |
| Verification | Freshly regenerate the MinGW Ninja build; prove invalid `PROJECT_CURRENT_SMOKE_JOBS` fails configuration; list `current-gate` and `media` labels with CTest; run `run-current-fast-smokes` and prove it excludes media; run full `run-current-smokes` at default parallelism and preserve all 194 current tests; run documentation governance and `git diff --check`. |
| Expected Markers | `run-current-smokes` passes `--parallel ${PROJECT_CURRENT_SMOKE_JOBS}` after a positive-integer CMake validation; every media target has `media` alongside `current-gate;smoke`; `run-current-fast-smokes` excludes `media` and depends only on non-media smoke targets; `current-fast-smokes-gcc` selects it. |
| Asset Needs | Existing owner-provided `O:/assets` FDD/HDD images only for the unchanged full media suite; fast entry deliberately requires none. |
| Reporting Requirements | Record baseline and post-change test selection/timing, exact commands, invalid-input result, and 194-test preservation in the S7 evidence. Report any test-registration discrepancy or parallelism-induced nondeterminism before accepting scope. |
| Stop Conditions | Stop and revise the packet if parallel execution introduces a reproducible test race/flakiness, if classification exposes a target requiring media but not listed in the media set, or if the change would require test semantic/runtime/asset changes. |
| Exit Criteria | Configurable full parallel gate, accurate media labels, and fast non-media preset are implemented; no current test is removed; fresh configuration, focused selection checks, fast run, full 194-test current gate, documentation governance, and diff check pass; evidence/history/status are updated and actual changes are reviewed. |
| Original Owner Request | Owner authorized T317 single-session implementation after the developer-efficiency research identified serial current-gate execution and missing media/fast entry classification. |
| Similar-Issue Sweep | Audit every `add_test` path and current-gate custom target/preset with `rg -n "add_test\\(|LABELS|run-current|current-gate|media" CMakeLists.txt CMakePresets.json`; classify non-current tests as out of scope and record all current media/non-media registration dispositions in evidence. |

## Current Technical Baseline

- **Current task artifact:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0317` / `build/output/nxvm_0_5_0317.exe`.
  S5 recorded its executor build as SHA-256
  `A7EC7165730E2B037C24693E1E6B0EBFA6C67B9126F9CD5D5863A62DCA963F24`.
  Coordinator acceptance rebuilt the same source and recorded SHA-256
  `B93DAF1ED9813E120ECBDD20A6E7595EFFA054961E546A15DC6AFB333494D412`.
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
| T317 | Closed test-corpus quality and the corrective repository-wide fixed-width vocabulary migration. S6 corrected its stale facade guidance and changed global verifier discovery to 469 tracked code/script paths, including root CMake entry points and a controlled negative proof. The global verifier, 47 target-local strict GCC command audit, documentation governance, and 194/194 current-gate passed; the 0317 artifact was rebuilt and recorded at coordinator acceptance. |
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
