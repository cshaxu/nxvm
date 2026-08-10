# Project Status

## Current Work

**Active: M5 T316 S19.**

## M5 T316 S19 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective; M5 T316 S19 follows accepted T316 S18 within the same active task. |
| Admission And Approval | Owner approved the M5 80386DX program and instructed that T316 continue normally after S1; coordinator accepted S18 at `7073549a` and admits S19 only. |
| Objective | Complete the declared Group-2 shift slice: `SHL/SAL /4`, `SHR /5`, and `SAR /7` through `C0/C1/D0/D1/D2/D3`, while retaining the Intel 80386 `/6` undefined-opcode boundary. |
| Non-goals | Do not claim the wider ordinary arithmetic/FLAGS, string, or data/operand families complete; do not change the accepted rotate slice, refactor shared operand/address/flag helpers, alter decoder ownership/public ABI, implement an 80387, or admit post-80386 forms. |
| Reference baseline | Accepted T314 artifact baseline; accepted T316 S1/S2 matrix and current focused corpus. |
| Files And ABI Surface | `src/core/machine/cpu_instructions.c`, the owned Group-2 focused smoke and CMake registration, T316 matrix/evidence, Status, and task artifact records. No public ABI change. |
| Applicable rules | Architecture: one CPU decoder/executor path; Coding: preserve local CPU style, use no speculative abstraction, and keep tests owner-bound; Execution: complete only the declared matrix slice with focused and retained evidence, artifact, documentation, diff, commit, and push gates; Source policy: Intel authority, no imported external source. |
| Verification | Audit Intel 80386 Group-2 shift forms before implementation; exercise `SHL/SAL`, `SHR`, and `SAR` at 8/16/32 bits across immediate-one, immediate-count, and CL-count encodings, count zero/no-op and 5-bit masking, defined CF/OF and SF/ZF/PF versus AF/OF undefined boundaries, register and memory publication, legacy/profile and 66/67 attributes, `/6` #UD, and protected read/write fault non-publication; scan `_a_shl/_a_shr/_a_sar`, `INS_C0/C1/D0/D1/D2/D3`, flag publication, and every helper caller before any abstraction; rebuild T316 artifact if runtime changes, run focused smoke, current gates, documentation governance, and `git diff --check`. |
| Expected markers | New focused marker `M5:T316:S19:SHIFT:OK`; retained S1--S18 markers and applicable current-gate markers pass. |
| Asset needs | None beyond existing governed current-gate assets. |
| Stop conditions | Stop and report if Intel-correct behavior requires a shared-helper change whose other callers lack coverage, decoder/ABI or exception-delivery architecture change, or an architecture boundary change. |
| Exit criteria | Every declared Group-2 shift form has implementation and focused proof; all encoding/count/width/flag/publication/profile/fault criteria pass; `/6` remains proved #UD; every scan hit is disposed; wider ordinary families remain explicitly partial; required gates, artifact handling, commit, and push pass. |
| Original owner request | Execute the complete 80386 program in Coordinated Dual-Session Mode against an Intel form--implementation--test matrix, repairing omissions without using Windows demand as a scope filter. |
| Similar-issue sweep | Defect class: Group-2 shift routes lacking Intel-form evidence or correct count, CF/OF, undefined-flag, profile, and atomic publication behavior. Scope: tracked CPU implementation, CPU tests, CMake registrations, T316 evidence, and relevant records; use `rg -n "_a_shl|_a_shr|_a_sar|INS_C0|INS_C1|INS_D0|INS_D1|INS_D2|INS_D3|VCPU_EFLAGS_(CF|OF|AF|SF|ZF|PF)" src/core/machine tests CMakeLists.txt docs`. Classify every production hit as covered, fixed, deferred to a named matrix slice, or out of scope with reason. |

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
| T314 | Closed the EGA CRTC 13h bound, registered the deterministic xasm smoke, restored six historical target-local strict GCC sets while retaining three selected libraries, and added the CRTC static closure verifier; the two inherited-governance debts remain. Its 0315 artifact, 52 gates, 147 CTests, and S2/S3/S6 markers passed. |
| T313 | Completed construction-failure atomicity and startup-failure visibility: single RAM allocation, transactional port/controller assembly, visible session start failure, and explicit debugger mappings. Its accepted artifact, 51 gates, 145 CTests, and S2--S6 focused markers passed. |
| T307 | Completed 80386 32-bit CPL3-to-CPL0 IDT and call-gate entry with TSS32 target stacks, preflighted atomic publication, and Intel-correct target-SS `#TS` classification. Its 0307 artifact, 51 gates, 137 CTests, and 9 focused/retained probes passed; product observation remained host-window limited before guest input. |
| T308 | Completed protected 32-bit same/outer error delivery and bounded contributory `#DF` containment through the existing route. Its 0308 artifact, 51 gates, and 137 CTests passed; triple-fault shutdown/reset remains deferred. |
| T309 | Completed the form-level 80386 audit and bounded trace-design record without runtime or artifact change; it selected T310 through evidence rather than a completeness claim. |
| T310 | Completed the admitted `0F` integer bit/data forms: `SETcc`, `MOVZX`/`MOVSX`, bit test/modify, `SHLD`/`SHRD`, `BSF`/`BSR`, and two-operand `IMUL`. Its 0310 artifact, 51 gates, and 143 CTests passed. |
| T311 | Completed the bounded non-PAE paging family: CPL3 U/S/R/W, CPL0 `CR0.WP`, exact producer #PF/CR2 diagnostics, and atomic one-boundary cross-page access. Its 0311 artifact, 51 gates, and 143 CTests passed; TLB/`INVLPG` remains unadmitted without a consumer. |
| T312 | Closed as a withdraw/no-op after audit found no real VM/M5 consumer for the candidate 32-bit system-extension families. It made no runtime change or artifact; future admission requires the per-family consumer evidence recorded in [T312 evidence](etc/evidence/t312-system-extension-admission.md). M6 candidates remain unadmitted. |

## Recent Governance

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

- **M5 Td S58:** made immediate remote push a cross-mode change-discipline
  requirement. Every successful task or subtask commit must be pushed before
  acceptance, closure, or completed-result reporting; a push failure is a
  reportable delivery failure unless the owner records a bounded deferral.

- **M5 Td S57:** completed the direct 80386 package-close audit. It found no
  T313-level runtime or boundary remediation; corrected stale lifecycle and
  recent-closure records in place; and recorded T301--T312 package completion
  before any separate M6 admission. See the
  [package-close audit](etc/evidence/m5-direct-80386-package-close-audit.md).
