# Project Status

## Current Work

**Active: M5 T313 S4.**

## M5 T313 S4 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective; M5 T313 S4 RTC and controller configuration rollback. |
| Admission And Approval | T313 S3 `e7016d2a` established registry transactions but requires corrective RTC whole-configuration rollback before acceptance. |
| Reference Baseline | Accepted `vm-0-5-0311` / `nxvm_0_5_0311.exe`, HEAD `d9152c6d` before this admission, and retained T300 port/session-atomicity boundaries. |
| Objective | Make RTC configuration rollback whole-controller atomic and prove FDC/HDC failure paths leave no mutable controller/topology/IRQ state before retry. |
| Non-goals | RAM creation, platform/session/Console work, debugger mapping, CPU/paging changes, a second bus/registry, a production global allocator facade, artifact identity, M6 mantle, or public bus-contract changes. |
| Scope And Order | S4 only: RTC/FDC/HDC configuration rollback and focused proof. S5 remains session/platform start result; S6 debugger enum mapping; S7 closure. |
| Source Touchpoints | `core/machine/machine.c`, RTC/FDC/HDC private state and focused core machine tests. |
| Files And ABI Surface | Preserve the single directional registry, T300 ownership, `core_machine_bus`, CPU I/O, and existing successful controller configuration behavior. |
| Applicable Rules | `rules/EXECUTION.md`, `rules/ARCHITECTURE.md`, `rules/CODING.md`, `rules/DOCUMENT.md`, and the source policy; retain one core executor, typed provider boundaries, and the existing NXVM product route. |
| Verification | Focused proof covers RTC first/second registration failure with no bindings/configuration/state mutation and retry; retain FDC/HDC rollback/retry and port ownership probes. |
| Expected Markers | `M5:T313:S4:CONTROLLER-ROLLBACK:OK`; S4 creates no artifact. |
| S1 Audit Record | [T313 construction-failure admission](etc/evidence/t313-construction-failure-admission.md) is the active design and acceptance record. |
| Asset Needs | No guest media, firmware, third-party source, or host OOM dependency. |
| Original Owner Request | Correct the remaining controller-state rollback defect before later independent failure boundaries. |
| Similar-Issue Sweep | Verify RTC, FDC, and HDC registration failure restores all configuration, connect, data, topology, and IRQ state before retry. |
| Stop Conditions | Stop for a required second registry, public generic fault API, a required VM/product change, or any controller state that cannot be restored through its existing private owner. |
| Exit Criteria | RTC first/second failure has no published port/config/state effect; FDC/HDC failure is equally clean or corrected; focused/retained/governance checks pass before push. |

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
