# Project Status

## Current Work

**Active: M5 T314 S6.**

## M5 T314 S6 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective; T314 is the most recently closed numeric task and S6 follows closed S5. |
| Admission And Approval | Owner approved the T314 corrective repair and requires coordinated dual-session execution on 2026-08-09. |
| Objective | Restore six existing target-local GCC strict-option sets and add a narrow CRTC boundary static closure gate. |
| Non-goals | Do not add global flags, claim transitive strict coverage, split targets, change runtime code, or broaden into C semantic analysis. |
| Reference Baseline | Accepted T314 0314 artifact and commit `fa1fe40b`. |
| Files And ABI Surface | `CMakeLists.txt`, a narrow CMake verifier, strict-GCC matrix/evidence, Status/history, and final artifact selection. No public ABI. |
| Applicable Rules | Execution corrective exception and mechanical defect closure; Coding GCC warning discipline; Architecture single VADP state owner; Documentation authority. |
| Verification | Build each restored target and inspect target-local Ninja flags; run new verifier directly and through `run-current-smokes`; run `current-gates-gcc`, documentation governance, `git diff --check`, and final artifact/hash checks. |
| Expected Markers | Restored target-local strict commands retain all four flags; `verify-ega-crtc-boundary` passes; retained S2/S3 markers and all current gates pass. |
| Asset Needs | Existing approved current-gate media only. |
| Stop Conditions | Stop if a restored target no longer compiles strictly, or if a narrow source-shape verifier cannot distinguish the admitted VADP paths without generalized parsing. |
| Exit Criteria | All six existing target-local strict options are restored; matrix states retained/local-only; the new verifier catches the known CRTC bypass shapes and is gate-wired; final evidence/gates/artifact pass and T314 recloses. |
| Original Owner Request | Correct T314: restore six pre-existing strict target-local constraints; add a narrow CRTC boundary closure gate; retain the three new strict libraries and all previous outcomes. |
| Similar-Issue Sweep | Scope: all target-level strict options and VADP CRTC indexing shapes. Use CMake option/target searches and the verifier fixture checks; classify each historical strict target and each dynamic CRTC access. |

## Current Technical Baseline

- **Current accepted artifact:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0314` / `build/output/nxvm_0_5_0314.exe`.
  It was accepted with T314.
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
| T314 | Closed the EGA CRTC 13h bound, registered the deterministic xasm smoke, narrowed strict GCC to three proven owned static libraries, and recorded two inherited-governance debts. Its 0314 artifact, 51 gates, 147 CTests, and S2/S3 focused markers passed. |
| T313 | Completed construction-failure atomicity and startup-failure visibility: single RAM allocation, transactional port/controller assembly, visible session start failure, and explicit debugger mappings. Its accepted artifact, 51 gates, 145 CTests, and S2--S6 focused markers passed. |
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
