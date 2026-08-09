# Project Status

## Current Work

**Active: M5 T308 S5.**

## M5 T308 S5 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective; M5 T308 S5 outer-CPL error-frame admission; Coordinated Dual-Session Mode. S1-S4 are accepted. |
| Admission And Approval | T307 is closed. T308 is the next uncompleted candidate in the owner-approved M5 80386 protected execution/delivery package; stale Queue entries for T301--T307 must be removed as part of this audit. |
| Objective | Audit existing 80386 protected outer-CPL `#GP`/`#NP`/`#SS`/`#TS` producers and, only where a focused reproducer proves it necessary, deliver their existing error code through the existing outer 32-bit gate route with a preflighted outer error frame. |
| Non-goals | New fault origins, hardware/NMI integration, recursive or double-/triple-fault policy, task/V86 returns, paging-policy expansion, product UX, public ABI, source import, or a second executor/delivery path. |
| Reference Baseline | `8fd51f5`; accepted artifact `vm-0-5-0307` / `nxvm_0_5_0307.exe`. |
| Files And ABI Surface | S5 may change existing core CPU exception and outer-entry helpers, focused probes, and task records. It must not change public interfaces, cross-module ownership, artifact identity, or product UX. |
| Applicable Rules | `rules/EXECUTION.md`, `rules/ARCHITECTURE.md`, `rules/CODING.md`, `rules/DOCUMENT.md`, and `etc/operations/policy/source-policy.md`; retain one core executor/state owner and existing checked stack/memory routes. |
| Verification | Intel 80386 PRM is authoritative. Record versioned read-only Bochs 2.6 and PCjs 2.00.0 behavior paths; construction uses focused synthetic probes only. |
| Expected Markers | The focused proof must retain T308 S2/S3 and T305/T306/T307 behavior, and demonstrate either the exact outer error frame plus delivered diagnostic or an evidence-backed no-consumer disposition. S5 creates no artifact. |
| Asset Needs | Read-only local references only; no guest media, firmware, or third-party source is committed. |
| Original Owner Request | Execute the direct M5 80386 protected execution/delivery package in coordinated mode, stopping before Mantle; use Intel as authority with read-only Bochs and PCjs comparison. |
| Similar-Issue Sweep | Sweep every existing outer-CPL exception producer, the 16/32-bit outer planners, error-code write order, delivery-failure restoration, and current diagnostic consumers. |
| S1 Audit Record | [T308 exception-delivery admission audit](etc/evidence/t308-exception-delivery-admission.md) records the producer/consumer matrix, error-code and priority rules, terminal preservation, recursion boundary, focused-probe plan, deferrals, and Queue cleanup evidence. |
| S2 Evidence Record | [T308 exception-delivery admission audit](etc/evidence/t308-exception-delivery-admission.md#s2-same-cpl-error-frame-evidence) records each admitted producer's vector/error code, same-CPL dword frame order, successful delivered diagnostic, and invalid-gate/stack failed-delivery restoration. |
| S3 Evidence Record | [T308 exception-delivery admission audit](etc/evidence/t308-exception-delivery-admission.md#s3-same-cpl-ts-delivery-evidence) records the T307 target-SS `#TS` producer, vector 10 dword frame, delivered diagnostic, and invalid/non-present delivery-gate and stack restoration. |
| S4 Evidence Record | [T308 exception-delivery admission audit](etc/evidence/t308-exception-delivery-admission.md#s4-retained-call-gate-completion-evidence) records the retained zero/two-parameter waiting completion and the combined pre-S3 T307 plus S3 `#TS` run. |
| S5 Evidence Record | [T308 exception-delivery admission audit](etc/evidence/t308-exception-delivery-admission.md#s5-outer-cpl-error-frame-evidence) records the outer-CPL producer audit, T307 `#GP(0030)` frame and diagnostic, and delivery-gate, target, and stack restoration. |
| Stop Conditions | Stop and report an Intel/reference disagreement, a required new fault origin or recursive policy, a second execution/state path, public raw-layout exposure, or any delivery path that cannot preserve original-fault restoration on failure. |
| Exit Criteria | S5 passes focused outer-entry, protected-return, interrupt-entry, same-CPL error, and T307 privilege-entry probes, documentation governance, and diff check. It remains active pending coordinator acceptance; it does not add hardware/NMI or recursive-fault behavior. |

## Current Technical Baseline

- **T307 accepted artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0307` / `nxvm_0_5_0307.exe`.
  T307 is closed; the next Queue candidate remains unnumbered.
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
| T300 | Unified the core directional port registry, propagated typed-provider failures, made VM session initialization failure-atomic, and removed public private layouts. The DMA binding uses a core-issued non-addressable nonce and core-internal revalidation; 51 static/governance gates and 129 CTests passed with its task-level developer artifact. |
| T301 | Completed the 80386 segment-selector family, including atomic segment loads and selector queries. The S4 sweep corrected non-present query handling and system-descriptor DPL/RPL visibility; 51 static/governance checks, 130 CTests, and the T301 artifact passed. The bounded product observation reached no guest checkpoint because host window discovery failed before guest input; manual observation remains verification-only. |
| T302 | Completed the 80386 operand/address/stack family. It corrected 16-bit fetch wrapping, crossed stack/frame width behavior, shared zero-length range validation, and `OUTSB` direction; all 131 CTests and its 0302 artifact passed. Product observation yielded no guest checkpoint because the process had no discoverable window handle; manual observation remains verification-only. |
| T303 | Completed the 80386 same-privilege control-transfer family. It fixed `LOOP` target-fault count publication, `CALL ptr16:32` decode width, same-CPL `RETF` DPL/RPL validation, and far-indirect register `#UD`; 51 gates, 132 CTests, and the 0303 artifact passed. Product observation remained host-window limited before guest input. |
| T304 | Completed the 80386 descriptor-table and system-control family. It corrected table register forms, fixed r/m16 selector/MSW widths, completed the admitted CR2/CR3 forms, and preserved existing 16/32-bit TSS LTR consumers; 51 gates, 133 CTests, and the 0304 artifact passed. Product observation remained host-window limited before guest input. |
| T305 | Completed the 80386 32-bit interrupt and exception-entry family. It added same-CPL gate planning, software and external front ends, and bounded existing #GP/#NP/#SS error-frame delivery containment; 51 gates, 134 CTests, and the 0305 artifact passed. Product observation remained host-window limited before guest input. |
| T306 | Completed the 80386 protected-return family. It added same-CPL and outer 16/32-bit `IRET`, aligned outer `RETF`, restored explicit outer nonconforming-code ownership, and fixed IRET IF/IOPL/RF/VM privilege masks; 51 gates, 135 CTests, and the 0306 artifact passed. Product observation remained host-window limited before guest input. |
| T307 | Completed 80386 32-bit CPL3-to-CPL0 IDT and call-gate entry with TSS32 target stacks, preflighted atomic publication, and Intel-correct target-SS `#TS` classification. Its 0307 artifact, 51 gates, 137 CTests, and 9 focused/retained probes passed; product observation remained host-window limited before guest input. |

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
