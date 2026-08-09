# Project Status

## Current Work

**Active: M5 T307 S3.**

## M5 T307 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective; M5 T307 S3 32-bit call-gate privilege entry; Coordinated Dual-Session Mode. S1/S2 are accepted. |
| Admission And Approval | T306 is closed. T307 is the next linear Queue candidate in the owner-approved M5 80386 protected execution/delivery package. |
| Objective | Implement the admitted 80386 protected non-V86 CPL3-to-CPL0 32-bit call-gate entry through 32-bit TSS `SS0:ESP0`, frame and parameter semantics, selector/cache checks, and all-or-nothing commit. |
| Non-goals | Task/nested-task return, task gates, general task switching, virtual-8086, new fault origins, reset/triple-fault policy, paging-policy expansion, product UX, public ABI, and source import. No new executor or delivery route. |
| Reference Baseline | `7d97ff6`; accepted artifact `vm-0-5-0306` / `nxvm_0_5_0306.exe`. |
| Files And ABI Surface | S3 may change only core CPU 32-bit call-gate/TSS/stack/selector helpers, focused tests, CMake registrations, and task records. It must not alter public interfaces or cross-module ownership. |
| Applicable Rules | `rules/EXECUTION.md`, `rules/ARCHITECTURE.md`, `rules/CODING.md`, `rules/DOCUMENT.md`, and `etc/operations/policy/source-policy.md`; retain one core executor/state owner, a single checked stack/memory route, and NXVM behavior. |
| Verification | Intel 80386 PRM is authoritative. Record versioned read-only Bochs 2.6 and PCjs 2.00.0 behavior paths; do not copy source. Construction uses focused synthetic probes only; full-system observation is closure-only. |
| Expected Markers | A focused T307 32-bit call-gate marker plus retained T305/T306/T304/T260/T261 markers. S3 creates no artifact. |
| Asset Needs | Read-only local references only; no guest media, firmware, or third-party source is committed. |
| Original Owner Request | Execute the direct M5 80386 protected execution/delivery package in coordinated mode, stopping before Mantle; use Intel as authority with read-only Bochs and PCjs comparison. |
| Similar-Issue Sweep | Sweep 16/32-bit call-gate decoding, parameter-copy counts, TSS stack reads, frame helpers, selector/cache validators, and retained call-gate probes. Error frames and hardware/NMI completion remain deferred. |
| S1 Audit Record | [T307 privilege-transition admission audit](etc/evidence/t307-privilege-transition-admission.md) freezes the admitted 16/32-bit TSS/frame/gate matrix, existing 16-bit path intersections, and implementation batches before changing CPU behavior. |
| S2 Evidence Record | [T307 privilege-transition admission audit](etc/evidence/t307-privilege-transition-admission.md#s2-common-planner-evidence) records the five-dword normal IDT frame, gate IF/TF behavior, software DPL versus external origin rule, and failure-state preservation. |
| S3 Evidence Record | [T307 privilege-transition admission audit](etc/evidence/t307-privilege-transition-admission.md#s3-call-gate-planner-evidence) records 32-bit call-gate DPL/type/present checks, target stack and frame order, zero-count and admitted parameter-copy behavior, and failure-state preservation. |
| Stop Conditions | Stop and report an unresolved Intel/reference disagreement, required architecture change, second execution/state path, public raw-layout exposure, or behavior owned by task/V86/paging/later families. |
| Exit Criteria | S3 passes focused call-gate and retained T305/T306/T304/T260/T261 probes, documentation governance, and diff check. It remains active pending coordinator acceptance; it does not alter Queue, create an artifact, or start error-frame/hardware-NMI work. |

## Current Technical Baseline

- **T306 accepted artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0306` / `nxvm_0_5_0306.exe`.
  T306 is accepted; T307 is the next linear candidate.
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
| T299 | Removed the remaining public configuration/debug raw borrows and profile raw binding. Production uses typed/copying interfaces; purpose-named test fixtures alone may include private core state. The recurrence gate, 51 current static/governance targets, and 126 CTests passed without product behavior changes. |
| T300 | Unified the core directional port registry, propagated typed-provider failures, made VM session initialization failure-atomic, and removed public private layouts. The DMA binding uses a core-issued non-addressable nonce and core-internal revalidation; 51 static/governance gates and 129 CTests passed with its task-level developer artifact. |
| T301 | Completed the 80386 segment-selector family, including atomic segment loads and selector queries. The S4 sweep corrected non-present query handling and system-descriptor DPL/RPL visibility; 51 static/governance checks, 130 CTests, and the T301 artifact passed. The bounded product observation reached no guest checkpoint because host window discovery failed before guest input; manual observation remains verification-only. |
| T302 | Completed the 80386 operand/address/stack family. It corrected 16-bit fetch wrapping, crossed stack/frame width behavior, shared zero-length range validation, and `OUTSB` direction; all 131 CTests and its 0302 artifact passed. Product observation yielded no guest checkpoint because the process had no discoverable window handle; manual observation remains verification-only. |
| T303 | Completed the 80386 same-privilege control-transfer family. It fixed `LOOP` target-fault count publication, `CALL ptr16:32` decode width, same-CPL `RETF` DPL/RPL validation, and far-indirect register `#UD`; 51 gates, 132 CTests, and the 0303 artifact passed. Product observation remained host-window limited before guest input. |
| T304 | Completed the 80386 descriptor-table and system-control family. It corrected table register forms, fixed r/m16 selector/MSW widths, completed the admitted CR2/CR3 forms, and preserved existing 16/32-bit TSS LTR consumers; 51 gates, 133 CTests, and the 0304 artifact passed. Product observation remained host-window limited before guest input. |
| T305 | Completed the 80386 32-bit interrupt and exception-entry family. It added same-CPL gate planning, software and external front ends, and bounded existing #GP/#NP/#SS error-frame delivery containment; 51 gates, 134 CTests, and the 0305 artifact passed. Product observation remained host-window limited before guest input. |
| T306 | Completed the 80386 protected-return family. It added same-CPL and outer 16/32-bit `IRET`, aligned outer `RETF`, restored explicit outer nonconforming-code ownership, and fixed IRET IF/IOPL/RF/VM privilege masks; 51 gates, 135 CTests, and the 0306 artifact passed. Product observation remained host-window limited before guest input. |

## Recent Governance

- **M5 Td S49:** replaced keyword-heavy authority checks with fixed-document
  Markdown schemas and temporary-tree self-tests; semantic ownership remains
  a closure-audit responsibility. Td work has no history record.
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
