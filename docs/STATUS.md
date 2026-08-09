# Project Status

## Current Work

**Active: M5 T313 S5.**

## M5 T313 S5 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective; M5 T313 S5 session/platform startup-failure visibility. |
| Admission And Approval | T313 S4 `fb570637` is accepted. S5 implements only the selected session's platform-start outcome and existing Console visibility. |
| Reference Baseline | Accepted `vm-0-5-0311` / `nxvm_0_5_0311.exe`, HEAD `d9152c6d` before this admission, and retained T300 port/session-atomicity boundaries. |
| Objective | Preserve exact platform-start failure as a session-owned outcome and surface it at the retained selected-session Console START/RESUME boundary. |
| Non-goals | Controller/RAM work, debugger mapping, CPU/paging changes, a new CLI framework, OS/window timing failure dependence, artifact identity, or M6 mantle. |
| Scope And Order | S5 only: VM session lifecycle/control, existing platform seam, and Console adapter/probes. S6 remains debugger enum mapping; S7 closure. |
| Source Touchpoints | `vm/composition/session/*`, platform provider seam, retained Console command flow, and focused VM/product tests. |
| Files And ABI Surface | Keep the outcome session-owned and separate from CPU fault diagnostic. Preserve selected-session policy; the local Console provider's start/resume callbacks return `type_status` so command dispatch can report the exact result immediately. |
| Applicable Rules | `rules/EXECUTION.md`, `rules/ARCHITECTURE.md`, `rules/CODING.md`, `rules/DOCUMENT.md`, and the source policy; retain one core executor, typed provider boundaries, and the existing NXVM product route. |
| Verification | Deterministic platform-start failure proves exact resume/start outcome, Console visibility, no run handle/false running control, and later success recovery in Console/window modes where applicable. |
| Expected Markers | `M5:T313:S5:START-OUTCOME:OK`; S5 creates no artifact. |
| S1 Audit Record | [T313 construction-failure admission](etc/evidence/t313-construction-failure-admission.md) is the active design and acceptance record. |
| Asset Needs | No guest media, firmware, third-party source, or host OOM dependency. |
| Original Owner Request | Repair only session-owned platform-start failure propagation and immediate Console visibility. |
| Similar-Issue Sweep | Inventory resume/start callers, void provider callbacks, selected-session Console dispatch, run-handle/control state, reset/new-start clearing, and platform test seams. |
| Stop Conditions | Stop for a required generic CLI/provider framework, an unavailable deterministic seam, a change to CPU fault ownership, or an immediate-Console requirement impossible without an unapproved callback contract expansion. |
| Exit Criteria | Exact failure is retained and shown by the Console without a false run state; later success rewrites it; focused/retained/governance checks pass before push. |

## Current Technical Baseline

- **Accepted artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0311` / `build/output/nxvm_0_5_0311.exe`.
  T311 is the accepted M5 artifact baseline.
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
| T305 | Completed the 80386 32-bit interrupt and exception-entry family. It added same-CPL gate planning, software and external front ends, and bounded existing #GP/#NP/#SS error-frame delivery containment; 51 gates, 134 CTests, and the 0305 artifact passed. Product observation remained host-window limited before guest input. |
| T306 | Completed the 80386 protected-return family. It added same-CPL and outer 16/32-bit `IRET`, aligned outer `RETF`, restored explicit outer nonconforming-code ownership, and fixed IRET IF/IOPL/RF/VM privilege masks; 51 gates, 135 CTests, and the 0306 artifact passed. Product observation remained host-window limited before guest input. |
| T307 | Completed 80386 32-bit CPL3-to-CPL0 IDT and call-gate entry with TSS32 target stacks, preflighted atomic publication, and Intel-correct target-SS `#TS` classification. Its 0307 artifact, 51 gates, 137 CTests, and 9 focused/retained probes passed; product observation remained host-window limited before guest input. |
| T308 | Completed protected 32-bit same/outer error delivery and bounded contributory `#DF` containment through the existing route. Its 0308 artifact, 51 gates, and 137 CTests passed; triple-fault shutdown/reset remains deferred. |
| T309 | Completed the form-level 80386 audit and bounded trace-design record without runtime or artifact change; it selected T310 through evidence rather than a completeness claim. |
| T310 | Completed the admitted `0F` integer bit/data forms: `SETcc`, `MOVZX`/`MOVSX`, bit test/modify, `SHLD`/`SHRD`, `BSF`/`BSR`, and two-operand `IMUL`. Its 0310 artifact, 51 gates, and 143 CTests passed. |
| T311 | Completed the bounded non-PAE paging family: CPL3 U/S/R/W, CPL0 `CR0.WP`, exact producer #PF/CR2 diagnostics, and atomic one-boundary cross-page access. Its 0311 artifact, 51 gates, and 143 CTests passed; TLB/`INVLPG` remains unadmitted without a consumer. |
| T312 | Closed as a withdraw/no-op after audit found no real VM/M5 consumer for the candidate 32-bit system-extension families. It made no runtime change or artifact; future admission requires the per-family consumer evidence recorded in [T312 evidence](etc/evidence/t312-system-extension-admission.md). M6 candidates remain unadmitted. |

## Recent Governance

- **M5 Td S57:** completed the direct 80386 package-close audit. It found no
  T313-level runtime or boundary remediation; corrected stale lifecycle and
  recent-closure records in place; and recorded T301--T312 package completion
  before any separate M6 admission. See the
  [package-close audit](etc/evidence/m5-direct-80386-package-close-audit.md).

- **M5 Td S50:** made principal sections, active-packet fields, Git-derived
  identifier continuity, supporting-index coverage, and relative Markdown
  links structural closure gates with controlled negative self-tests. Td work
  has no history record.
- **M5 Td S51:** defined ordinary and coordinator/executor execution modes,
  Instructions, report-and-acceptance loops, bounded task packages, and their
  required package-close global governance audit. Td work has no history record.
- **M5 Td S52:** regrouped supporting material into architecture, requirements,
  evidence, operations, research, and history; isolated M5/legacy evidence and
  repaired every governed migration link. Td work has no history record.
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
- **M5 Td S56:** made Intel manual review, versioned Bochs/PCjs behavior
  comparison, one post-family Setup observation, and bounded optional bridge
  escalation mandatory evidence for the 80386 package. Td work has no history
  record.
