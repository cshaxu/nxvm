# Project Status

## Current Work

**Active.** M5 Td S67 is aligning the 80386DX completeness plan with its
roadmap, Queue, evidence, and deferred-boundary records.

## M5 Td S67 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Governance; M5 Td S67; Ordinary Mode. |
| Admission And Approval | The owner approved this post-T319 documentation governance correction on 2026-08-11: establish the two-view 80386DX completeness plan without placing project-specific context in global execution rules. |
| Objective | Define the project-level 80386DX completeness boundary, connect horizontal instruction-form evidence to vertical architecture-state closures, order the Queue accordingly, and record explicit deferred boundaries. |
| Non-goals | No CPU, test, build, artifact, CMake, global-rule, or active implementation-task change; no 80386/80387 or Windows compatibility completion claim. |
| Reference Baseline | `04d4b182` / closed T319, whose 0319 artifact remains the current developer artifact. |
| Files And ABI Surface | `docs/design/GOAL.md`, `docs/design/ROADMAP.md`, `docs/QUEUE.md`, `docs/TODO.md`, `docs/etc/evidence/80386-closure-map.md`, the T316 ordinary matrix cross-reference, `docs/STATUS.md`, and the documentation index only; no runtime or ABI surface. |
| Applicable Rules | Task Reading Set; `docs/rules/DOCUMENT.md`; `docs/rules/EXECUTION.md` identifier and closure requirements; principal-document authority boundaries. |
| Verification | Documentation governance default command, documentation and governance-state scopes, `git diff --check`, actual-document authority review, commit, and push. |
| Expected Markers | One indexed `80386-closure-map.md`; Roadmap and Queue state the complementary closure model; TODO has bounded VME/PVI, x87, and legacy-LOCK admission paths. |
| Asset Needs | None. |
| Reporting Requirements | Record the authority-boundary review, exact documents changed, verification results, commit, push, and confirmation that no global rule received project-specific content. |
| Stop Conditions | Stop if the change requires a runtime claim, alters global execution policy, allocates a numeric task, or conflicts with a principal-document authority; request owner direction rather than inferring a new architecture decision. |
| Exit Criteria | Goals define the bounded 80386DX outcome; Roadmap defines the two closure views; Queue names the closure-map handoff; map distinguishes complete/planned/deferred composition boundaries; TODO records VME/PVI, x87, and legacy LOCK admission conditions; all documentation gates pass and the Td record is committed and pushed. |
| Original Owner Request | Avoid a piecemeal 80386 plan; retain fine-grained evidence while making the architecture-state dependency route explicit, without polluting global rules. |
| Similar-Issue Sweep | Documentation-only: reviewed Goal, Roadmap, Queue, TODO, T316 matrix, Status, and `etc/` index coverage. No source defect class applies. |

## Current Technical Baseline

- **Current developer artifact:** T319 selects `vm-0-5-0319` /
  `build/output/nxvm_0_5_0319.exe`; its commit, SHA-256, runtime identity, and
  retained boundaries are in [T319 history](history/M5-T319-lgdt-lidt.md).
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
| T319 | Closed the bounded non-VM86 LGDT/LIDT `0F 01 /2,/3` table-load slice: protected CPL>0 now rejects before pseudo-descriptor reads, both forms have real/protected, attribute, source-atomicity, table-consumer, and PIC evidence, and VM86 plus 80286 LOCK remain transferred. Its 0319 artifact and 196/196 current-gate pass are retained in history. |
| T318 | Closed the bounded SGDT/SIDT `0F 01 /0,/1 table-store slice: a local six-byte preflight/publication repair prevents partial pseudo-descriptor writes; both forms have profile, attribute, segment, rejection, protected atomicity, VM86 and IRQ-ordering evidence. 195/195 current-gate passed, and executor plus coordinator 0318 artifact hashes are retained in its history. LGDT/LIDT and wider processor-control work remain future bounded packages. |
| T317 | Closed test-corpus quality plus corrective type-vocabulary and developer-feedback work. S7 retains the 194-test current gate while adding validated default-four-job CTest execution, 15 explicit media labels, and a 179-test non-media fast preset; its full gate, fast run, media verifier, documentation governance, and diff check passed. |
| T316 | Closed the declared 80386 ordinary-execution slices through S66. Residual parent-family breadth remains explicitly transferred in the S66 matrix to named later Queue packages, the legacy LOCK TODO, or the external-coprocessor boundary; no whole-80386 claim. Its predecessor developer artifact and 194 current-gate tests passed. |
| T314 | Closed the EGA CRTC 13h bound, registered the deterministic xasm smoke, restored six historical target-local strict GCC sets while retaining three selected libraries, and added the CRTC static closure verifier; the two inherited-governance debts remain. Its 0315 artifact, 52 gates, 147 CTests, and S2/S3/S6 markers passed. |
| T313 | Completed construction-failure atomicity and startup-failure visibility: single RAM allocation, transactional port/controller assembly, visible session start failure, and explicit debugger mappings. Its accepted artifact, 51 gates, 145 CTests, and S2--S6 focused markers passed. |
| T307 | Completed 80386 32-bit CPL3-to-CPL0 IDT and call-gate entry with TSS32 target stacks, preflighted atomic publication, and Intel-correct target-SS `#TS` classification. Its 0307 artifact, 51 gates, 137 CTests, and 9 focused/retained probes passed; product observation remained host-window limited before guest input. |
| T308 | Completed protected 32-bit same/outer error delivery and bounded contributory `#DF` containment through the existing route. Its 0308 artifact, 51 gates, and 137 CTests passed; triple-fault shutdown/reset remains deferred. |

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
