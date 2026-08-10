# Project Status

## Current Work

**Active: M5 T315 S1.**

## M5 T315 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New; M5 T315 S1 is the next global numeric task after closed T314. |
| Admission And Approval | Owner approved the M5 80386DX architecture-completeness program, matrix-driven admission/exit, and Coordinated Dual-Session Mode on 2026-08-09. Coordinator admits S1 only; every later subtask requires coordinator acceptance of the prior result. |
| Objective | Produce the Intel 80386 PRM-to-source-to-test matrix for the first Queue family, ordinary execution and flag completeness, and select one independently correctable implementation slice for S2. |
| Non-goals | Do not modify CPU behavior, claim a family is complete, implement an 80387, admit later IA-32 forms, change a public ABI, use Windows demand as a scope filter, or copy/import Bochs, PCjs, or other third-party source. |
| Reference baseline | Accepted T314 `vm-0-5-0315` / `build/output/nxvm_0_5_0315.exe`; current source, test corpus, T309 audit, and the M5 80386DX completeness program. |
| Files And ABI Surface | New `docs/etc/evidence/t315-ordinary-execution-matrix.md` and, if needed, Status/evidence cross-links only. No production source, test executable, CMake target, public ABI, or artifact identity changes in S1. |
| Applicable rules | Architecture: one CPU decoder/executor ownership path and no new cross-component coupling; Coding: preserve existing CPU ownership/style and record reusable-helper caller coverage before abstraction; Execution: PRM matrix-driven admission, evidence, documentation, diff, commit, and push gates; Source policy: Intel is behavioral authority and external implementations are read-only references only. |
| Verification | Inspect the Intel 80386 ordinary execution/FLAGS/operand/address/string/control forms against the initialized primary dispatch, metadata/profile gates, operand/flag helpers, focused tests, and T309 baseline; record code/test evidence and all complete/partial/missing/outside/boundary classifications; verify every retained partial or missing form names its next Queue family or candidate slice; run documentation governance and `git diff --check`. |
| Expected markers | Evidence marker `M5:T315:S1:80386-ORDINARY-MATRIX:OK`, emitted only when every reviewed form has a classification, source/test disposition, and next placement; no runtime marker or artifact is expected because S1 changes no runnable path. |
| Asset needs | None. Intel PRM citation material and read-only local/reference metadata may be consulted under source policy; no guest media, third-party source, binary, trace, or external asset enters the repository. |
| Stop conditions | Stop and report if the family cannot be bounded without changing decoder ownership, a shared helper lacks caller coverage needed for safe alteration, the Intel form boundary is ambiguous after authority review, or a proposed abstraction changes architecture/public ABI. |
| Exit criteria | The ordinary-family matrix covers its relevant 80386 forms and records each classification, source route, existing focused evidence, and remaining gap. The matrix identifies one S2 slice with exact forms, helper-caller coverage, focused proof plan, non-goals, and similar-issue sweep; no unclassified in-scope form is hidden, and no completeness claim is made before the whole family matrix is resolved. |
| Original owner request | Adopt Coordinated Dual-Session Mode to execute the complete 80386 plan against an Intel 80386 form--implementation--test matrix, repair omissions, perform differential/boundary review where needed, and close the package with governance evidence; 80387 implementation is out of scope while 80386 coprocessor-interface behavior remains in scope. |
| Similar-issue sweep | Not a defect repair in S1. Matrix inventory scope: tracked CPU production source, CPU-focused tests, dispatch/profile metadata, prior 80386 evidence, Queue, TODO, and relevant rules; use `rg -n "insTable|INS_0F|ExecIns|core_machine_cpu_instruction_metadata_get|flag|operand|opcode" src/core/machine tests docs` plus exact PRM-form review. Every in-scope primary-form hit receives a matrix disposition; external-reference source is read-only and excluded from repository scan. |

## Current Technical Baseline

- **Current accepted artifact:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0315` / `build/output/nxvm_0_5_0315.exe`.
  It was accepted with T314 S7 closure.
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
| T314 | Closed the EGA CRTC 13h bound, registered the deterministic xasm smoke, restored six historical target-local strict GCC sets while retaining three selected libraries, and added the CRTC static closure verifier; the two inherited-governance debts remain. Its 0315 artifact, 52 gates, 147 CTests, and S2/S3/S6 markers passed. |
| T313 | Completed construction-failure atomicity and startup-failure visibility: single RAM allocation, transactional port/controller assembly, visible session start failure, and explicit debugger mappings. Its accepted artifact, 51 gates, 145 CTests, and S2--S6 focused markers passed. |
| T307 | Completed 80386 32-bit CPL3-to-CPL0 IDT and call-gate entry with TSS32 target stacks, preflighted atomic publication, and Intel-correct target-SS `#TS` classification. Its 0307 artifact, 51 gates, 137 CTests, and 9 focused/retained probes passed; product observation remained host-window limited before guest input. |
| T308 | Completed protected 32-bit same/outer error delivery and bounded contributory `#DF` containment through the existing route. Its 0308 artifact, 51 gates, and 137 CTests passed; triple-fault shutdown/reset remains deferred. |
| T309 | Completed the form-level 80386 audit and bounded trace-design record without runtime or artifact change; it selected T310 through evidence rather than a completeness claim. |
| T310 | Completed the admitted `0F` integer bit/data forms: `SETcc`, `MOVZX`/`MOVSX`, bit test/modify, `SHLD`/`SHRD`, `BSF`/`BSR`, and two-operand `IMUL`. Its 0310 artifact, 51 gates, and 143 CTests passed. |
| T311 | Completed the bounded non-PAE paging family: CPL3 U/S/R/W, CPL0 `CR0.WP`, exact producer #PF/CR2 diagnostics, and atomic one-boundary cross-page access. Its 0311 artifact, 51 gates, and 143 CTests passed; TLB/`INVLPG` remains unadmitted without a consumer. |
| T312 | Closed as a withdraw/no-op after audit found no real VM/M5 consumer for the candidate 32-bit system-extension families. It made no runtime change or artifact; future admission requires the per-family consumer evidence recorded in [T312 evidence](etc/evidence/t312-system-extension-admission.md). M6 candidates remain unadmitted. |

## Recent Governance

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

- **M5 Td S58:** made immediate remote push a cross-mode change-discipline
  requirement. Every successful task or subtask commit must be pushed before
  acceptance, closure, or completed-result reporting; a push failure is a
  reportable delivery failure unless the owner records a bounded deferral.

- **M5 Td S57:** completed the direct 80386 package-close audit. It found no
  T313-level runtime or boundary remediation; corrected stale lifecycle and
  recent-closure records in place; and recorded T301--T312 package completion
  before any separate M6 admission. See the
  [package-close audit](etc/evidence/m5-direct-80386-package-close-audit.md).

- **M5 Td S55:** replaced the over-granular M5 80386 plan with a direct,
  family-oriented 32-bit protected execution/delivery package before Mantle;
  full-system checkpoints are debug/progress tools, while construction uses
  focused synthetic architecture probes. Td work has no history record.
