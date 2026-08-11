# Project Status

## Current Work

**Active: M5 T317 S2.** Migrate the governed T316 CPU smoke corpus to the
project type vocabulary and enforce it with a narrow static gate in Coordinated Dual-Session Mode.

## M5 T317 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective; S2 is the next owner-approved corrective slice inside latest open T317, following accepted S1. |
| Admission And Approval | Owner approved the ordered T317 plan and directed the coordinator to drive T317 through completion with the existing executor; S2 implements the plan's project-type vocabulary requirement. |
| Objective | Replace direct fixed-width C scalar spellings in the exact 47 T316-added owner smoke sources and the necessary test-only support header(s) with the project type vocabulary, then add a narrow mechanical gate that prevents regression in that governed set. |
| Non-goals | No migration of inherited tests, production sources, public headers, or repository-wide type debt; no behavior/assertion, fixture, CMake target-policy, ABI, CPU/FPU/device/timing change; no local aliases or type facade; `src/type.h` remains the foundational definition boundary. |
| Reference Baseline | `58b6b299` / `vm-0-5-0316`; S1's authoritative 47-entry CMake inventory and `tests/support/core_machine_cpu_fixture.h` define the governed corpus/support boundary. |
| Files And ABI Surface | Expected 47 inventory owner smoke sources only where they contain a governed spelling, `tests/support/core_machine_cpu_fixture.h` only if required, a narrow CMake/static verifier and evidence, plus `CMakeLists.txt` and `STATUS.md`; no `src/` production or public-interface source. |
| Applicable Rules | `docs/rules/EXECUTION.md`, `docs/rules/CODING.md`, `docs/rules/DOCUMENT.md`, `docs/design/CODING.md`, and `docs/etc/evidence/t317-test-corpus-quality-plan.md`. |
| Verification | Fresh GCC configure; narrow vocabulary gate with positive and negative self-checks; zero forbidden `uint*_t`/`int*_t` spellings in the 47-source inventory and touched support header boundary; strict 47-target audit; documentation governance, diff check, and full current-gate. |
| Expected Markers | Deterministic `verify-t317-test-type-vocabulary` success marker with governed-file count and zero direct fixed-width spellings; no runtime marker changes. |
| Asset Needs | None; deterministic local source scan and GCC/Ninja build only. |
| Reporting Requirements | Executor first confirms or materially objects, then continues under its existing durable session goal. Return only a complete committed/pushed P1 or reproducible material blocker; no partial reports. P1 maps each type spelling family and support-header impact to evidence. |
| Stop Conditions | Stop for any required type migration outside the exact governed corpus/support boundary, any need for a local alias or public/production type change, any behavior/assertion change, or a scanner rule that cannot remain mechanically narrow; report the hit and minimal alternative. |
| Exit Criteria | Every governed source/support file is free of direct fixed-width integer spellings, the narrow gate proves positive and negative cases without scanning unrelated debt, strict 47-target audit and all required gates pass, and the implementation P is committed/pushed and passes coordinator review and governance closure. |
| Original Owner Request | Repair the proven strict-GCC, project-type, and fixture-quality gaps as a governed T317 package before resuming ordinary 80386 capability work. |
| Similar-Issue Sweep | Derive the source set from S1's 47-entry inventory; scan every governed owner source and each touched support header for `uint*_t`/`int*_t`; compare scanner exclusions with `src/type.h` and retain unrelated repository debt outside its boundary. |
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
| T317 S1 | Accepted target-local strict GCC coverage for all 47 T316-added CPU smoke targets with a CMake inventory and actual-Ninja command gate; warning-clean, behavior-preserving smoke remediation; retained real-mode #NM vector-7 FPU frame regression; and future x87 admission debt. Coordinator verified strict audit, documentation governance, diff check, and 194/194 current-gate tests. |
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
