# Project Status

## Current Work

**Active: M5 T314 S2.**

## M5 T314 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective; M5 T314 S1 admission closed in commit `4932cbe6`, so the repair is its next subtask. |
| Admission And Approval | Owner approved the inserted Queue candidate and T314 package on 2026-08-09. Coordinator accepted S1 admission and admits S2 only; later subtasks require coordinator acceptance of the prior result. |
| Objective | Remove the VADP CRTC 13h out-of-bounds port read/write while retaining `crtc[]` as the sole EGA offset state source. |
| Non-goals | Do not add CRTC behavior, change EGA 320x200x16 or 640x350x16 behavior, alter firmware/display topology/host presentation, or modify inherited CPU, xasm, or Win32 code. |
| Reference baseline | Accepted T313 `vm-0-5-0313` / `build/output/nxvm_0_5_0313.exe`; current source and artifact identity remain T313 until T314 closure. |
| Files And ABI Surface | `src/core/machine/vadp.c`, `src/core/machine/vadp.h`, an owned VADP port smoke, CMake registration, Task evidence, Queue, TODO, and Status. No public ABI change. |
| Applicable rules | Architecture: one VADP owner and one CRTC offset truth source; Coding: C11, cohesive ownership and owner-bound tests; Execution: defect-class sweep, exact evidence, artifact, documentation, diff, commit, and push gates; Source policy: no import or external asset. |
| Verification | Configure `mingw-gcc-x64`; run the focused VADP port smoke; enumerate every `crtc[` occurrence in `vadp.c` and `vadp.h` with disposition; build `current-gates-gcc`; run `powershell -NoProfile -ExecutionPolicy Bypass -File tools/Verify-DocumentationGovernance.ps1 -RepositoryRoot .`; run `git diff --check`. At package closure, build and hash the T314 artifact. |
| Expected markers | New focused marker `M5:T314:S2:EGA-CRTC-BOUNDARY:OK`; retained EGA/VADP and current-gate markers remain passing. |
| Asset needs | None for S2 focused proof; retained current-gate media uses the approved repository-relative assets under existing governance. |
| Stop conditions | Stop and report if the repair requires changed EGA mode semantics, firmware, display ownership, a second offset state source, or a public interface change. |
| Exit criteria | CRTC index 13h has same-array storage; every variable CRTC array access is guarded by the bounded support predicate; CRTC constant indices are compile-time bounded; the port regression verifies read/write and isolation; the stale TODO is closed or narrowed; all stated gates pass and the S2 result is accepted. |
| Original owner request | Implement owner-approved T314 in coordinated dual-session mode: repair EGA CRTC 13h bounds, register the deterministic xasm smoke, audit/apply strict GCC coverage only to confirmed owned targets, and record two inherited-governance debts. |
| Similar-issue sweep | Defect class: accepted CRTC index used as an unchecked array subscript. Scope: tracked VADP production/test/CMake and relevant records, using `rg -n "crtc\\[|supported_crtc_index|CRTC_REGISTER_COUNT" src/core/machine tests CMakeLists.txt docs`. Classify every production hit; variable accesses must route through the bounded predicate, constants through compile-time bounds, and unrelated hits receive a reason. |

## Current Technical Baseline

- **Current accepted artifact:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0313` / `build/output/nxvm_0_5_0313.exe`.
  It was accepted with T313.
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
| T313 | Completed construction-failure atomicity and startup-failure visibility: single RAM allocation, transactional port/controller assembly, visible session start failure, and explicit debugger mappings. Its 0313 artifact, 51 gates, 145 CTests, and S2--S6 focused markers passed. |
| T306 | Completed the 80386 protected-return family. It added same-CPL and outer 16/32-bit `IRET`, aligned outer `RETF`, restored explicit outer nonconforming-code ownership, and fixed IRET IF/IOPL/RF/VM privilege masks; 51 gates, 135 CTests, and the 0306 artifact passed. Product observation remained host-window limited before guest input. |
| T307 | Completed 80386 32-bit CPL3-to-CPL0 IDT and call-gate entry with TSS32 target stacks, preflighted atomic publication, and Intel-correct target-SS `#TS` classification. Its 0307 artifact, 51 gates, 137 CTests, and 9 focused/retained probes passed; product observation remained host-window limited before guest input. |
| T308 | Completed protected 32-bit same/outer error delivery and bounded contributory `#DF` containment through the existing route. Its 0308 artifact, 51 gates, and 137 CTests passed; triple-fault shutdown/reset remains deferred. |
| T309 | Completed the form-level 80386 audit and bounded trace-design record without runtime or artifact change; it selected T310 through evidence rather than a completeness claim. |
| T310 | Completed the admitted `0F` integer bit/data forms: `SETcc`, `MOVZX`/`MOVSX`, bit test/modify, `SHLD`/`SHRD`, `BSF`/`BSR`, and two-operand `IMUL`. Its 0310 artifact, 51 gates, and 143 CTests passed. |
| T311 | Completed the bounded non-PAE paging family: CPL3 U/S/R/W, CPL0 `CR0.WP`, exact producer #PF/CR2 diagnostics, and atomic one-boundary cross-page access. Its 0311 artifact, 51 gates, and 143 CTests passed; TLB/`INVLPG` remains unadmitted without a consumer. |
| T312 | Closed as a withdraw/no-op after audit found no real VM/M5 consumer for the candidate 32-bit system-extension families. It made no runtime change or artifact; future admission requires the per-family consumer evidence recorded in [T312 evidence](etc/evidence/t312-system-extension-admission.md). M6 candidates remain unadmitted. |

## Recent Governance

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


- **M5 Td S53:** recorded the approved M6 mantle-entry queue refinement and a
  five-package, evidence-led 80386 candidate sequence; scrubbed prohibited
  historical identifiers from retained external research prose. Td work has no
  history record.
- **M5 Td S54:** reordered the Queue so the M5 80386 package precedes the M6
  mantle experiment; candidate order changed without assigning implementation
  identifiers or changing a candidate contract. Td work has no history record.
- **M5 Td S55:** replaced the over-granular M5 80386 plan with a direct,
  family-oriented 32-bit protected execution/delivery package before Mantle;
  full-system checkpoints are debug/progress tools, while construction uses
  focused synthetic architecture probes. Td work has no history record.
