# Project Status

## Current Work

**Active: M5 T317 S3.** Consolidate governed CPU smoke lifecycle mechanics into
test-only support without changing instruction-family assertions in Coordinated Dual-Session Mode.

## M5 T317 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective; S3 is the next owner-approved corrective slice inside latest open T317, following accepted S2. |
| Admission And Approval | Owner approved the ordered T317 plan and directed the coordinator to drive T317 through completion with the existing executor; S3 implements its test-only fixture-consolidation requirement. |
| Objective | Centralize repeated behavior-neutral CPU-smoke mechanics: machine creation, execution-provider bind/freeze/reset, real-mode preparation, bounded run, and copied observation move to `tests/support`; instruction input, GDT/IDT/PIC setup, family assertions, and diagnostic policy remain in owner smokes. |
| Non-goals | No production/public API or `src/` change; no test-only mirror state, product-visible interface, generic test framework, assertion/marker/input/behavior change, target-policy change, or consolidation of instruction-family, descriptor, interrupt, port, or device semantics. |
| Reference Baseline | `1124eb3c` / `vm-0-5-0316`; the S1 47-source inventory and S2 type-vocabulary boundary remain authoritative. |
| Files And ABI Surface | Expected `tests/support/core_machine_cpu_fixture.h`, governed owner smokes that consume a clearly identical support operation, CMake/evidence/`STATUS.md`; no `src/` production or public-interface path. |
| Applicable Rules | `docs/rules/EXECUTION.md`, `docs/rules/CODING.md`, `docs/rules/DOCUMENT.md`, `docs/design/CODING.md`, and `docs/etc/evidence/t317-test-corpus-quality-plan.md`. |
| Verification | Record pre/post caller inventory and every migration/local-exception disposition; focused proof for each support operation's representative consumers; prove no `src/` source includes `tests/support`; rerun type and strict gates, documentation governance, diff check, and full current-gate. |
| Expected Markers | A deterministic S3 support-caller inventory/evidence record naming centralized operations, migrated callers, and retained owner-local semantics; existing runtime markers are unchanged. |
| Asset Needs | None; deterministic local C/GCC/Ninja test corpus only. |
| Reporting Requirements | Executor first confirms or materially objects, then continues under its existing durable session goal. Return only a complete committed/pushed P1 or reproducible material blocker; no partial reports. P1 maps each helper's responsibility to every migrated caller and every deliberate local exception. |
| Stop Conditions | Stop if a proposed helper carries instruction, descriptor, interrupt, port, device, or assertion semantics; would mirror mutable machine state; lacks caller coverage; needs a public/product API; or cannot preserve owner behavior mechanically. |
| Exit Criteria | All duplicate mechanics matching the admitted support-operation shapes are centralized or explicitly retained with a precise local-semantic reason; every helper has caller inventory and focused evidence; no `src/` include of support and no product/API change; all required gates pass; implementation P is committed/pushed and passes coordinator review and governance closure. |
| Original Owner Request | Repair the proven strict-GCC, project-type, and fixture-quality gaps as a governed T317 package before resuming ordinary 80386 capability work. |
| Similar-Issue Sweep | Search the 47 inventory sources for create/bind/freeze/reset, real-mode preparation, run/result/diagnostic/copy shapes; compare all hits with support operations and record migrated or retained-local disposition. |
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
| T317 S2 | Accepted the exact 47-source project-type migration plus one necessary test-support header, with an inventory-bounded positive/negative type-vocabulary gate. Coordinator proved type-normalized source equivalence and verified strict audit, documentation governance, diff check, and 194/194 current-gate tests. |
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
