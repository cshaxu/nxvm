# Project Status

## Current Work

**Active: M5 T305 S4.**

## M5 T305 S4 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective; M5 T305 S4 existing fault-entry and failed-delivery containment; Coordinated Dual-Session Mode. S2/S3 closed the planner and front ends. |
| Admission And Approval | T304 is closed. T305 is the next linear Queue candidate in the owner-approved M5 80386 protected execution/delivery package. |
| Objective | Admit 32-bit IDT interrupt/trap-gate and exception-entry semantics for `INT`, `INT3`, `INTO`, hardware interrupts, and already-admitted faults. Freeze gate/type/DPL/present/target-CS/error-code/frame/recursive-fault matrices before implementation. |
| Non-goals | 32-bit `IRET`, outer returns, call gates, task gates, task switching, virtual-8086, paging-policy expansion, product UX, public ABI, and source import. No new executor or delivery path. |
| Reference Baseline | `69d1b83`; current artifact `vm-0-5-0304` / `nxvm_0_5_0304.exe`. |
| Files And ABI Surface | S1 may inspect core CPU delivery, IDT, fault, stack, selector, PIC-facing interrupt, focused tests, CMake, and task records. It must not alter public interfaces or cross-module ownership. |
| Applicable Rules | `rules/EXECUTION.md`, `rules/ARCHITECTURE.md`, `rules/CODING.md`, `rules/DOCUMENT.md`, and `etc/operations/policy/source-policy.md`; retain one core executor/state owner and NXVM behavior. |
| Verification | Intel 80386 PRM is authoritative. Record versioned read-only Bochs 2.6 and PCjs 2.00.0 behavior paths; do not copy source. Construction uses focused synthetic probes only. S1 is audit only. |
| Expected Markers | S1 records an audit only. A later focused marker and artifact target are admitted only after the matrix. |
| Asset Needs | Read-only local references only; no guest media, firmware, or third-party source is committed. |
| Original Owner Request | Execute the direct M5 80386 protected execution/delivery package in coordinated mode, stopping before Mantle; use Intel as authority with read-only Bochs and PCjs comparison. |
| Similar-Issue Sweep | Audit interrupt/fault entry, IDT gate lookup, stack-frame and selector helpers, PIC/hardware delivery hooks, exception diagnostics, and current focused tests. Classify every production hit before implementation. |
| S1 Audit Record | [T305 interrupt and exception-entry admission audit](etc/evidence/t305-interrupt-exception-entry-admission.md) freezes the gate/profile/origin/frame/commit matrix, current path classification, and bounded S2+ work. |
| S2 Evidence Record | [T305 interrupt and exception-entry admission audit](etc/evidence/t305-interrupt-exception-entry-admission.md#s2-same-cpl-32-bit-gate-evidence) records the gate-type planner, focused prepared-state proof, retained 16-bit checks, and deferred origins. |
| S3 Evidence Record | [T305 interrupt and exception-entry admission audit](etc/evidence/t305-interrupt-exception-entry-admission.md#s3-software-and-hardware-front-end-evidence) records software/external origin separation, staged PIC/NMI publication, and focused front-end proof. |
| S4 Evidence Record | [T305 interrupt and exception-entry admission audit](etc/evidence/t305-interrupt-exception-entry-admission.md#s4-existing-fault-entry-and-containment-evidence) records 32-bit existing-fault error frames, bounded failed-delivery containment, and the retained selector-loader check. |
| Stop Conditions | Stop and report unresolved Intel/reference disagreement, required architecture change, second execution/state path, raw public-layout exposure, or behavior owned by IRET/outer-return/gate/task/V86/later families. |
| Exit Criteria | Existing admitted fault consumers enter the frozen 32-bit planner with correct retained error codes; failed delivery restores the original first fault under a bounded terminal containment observation. Focused probes and narrow gates pass without IRET, task gates, reset/triple-fault policy, or new fault-origin work. |

## Current Technical Baseline

- **T304 accepted artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0304` / `nxvm_0_5_0304.exe`.
  T304 is accepted; T305 is the next linear candidate.
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
| T297 | Replaced default firmware raw profile binding with an opaque core-invoked capability for checked memory/port I/O and stop requests. Configure-time immutable ROM registration now rolls back atomically on callback failure; default BIOS/QDCGA, boot failure, 49 gates, and 126/126 CTests are closed without product UX change. |
| T298 | Replaced formal debugger raw CPU/instruction/execution/RAM/port borrows with paused-or-stopped named core operations and copied observations. The retained Console/debugger UX passed focused checks and the 126-test regression suite without a global target or second execution path. |
| T299 | Removed the remaining public configuration/debug raw borrows and profile raw binding. Production uses typed/copying interfaces; purpose-named test fixtures alone may include private core state. The recurrence gate, 51 current static/governance targets, and 126 CTests passed without product behavior changes. |
| T300 | Unified the core directional port registry, propagated typed-provider failures, made VM session initialization failure-atomic, and removed public private layouts. The DMA binding uses a core-issued non-addressable nonce and core-internal revalidation; 51 static/governance gates and 129 CTests passed with its task-level developer artifact. |
| T301 | Completed the 80386 segment-selector family, including atomic segment loads and selector queries. The S4 sweep corrected non-present query handling and system-descriptor DPL/RPL visibility; 51 static/governance checks, 130 CTests, and the T301 artifact passed. The bounded product observation reached no guest checkpoint because host window discovery failed before guest input; manual observation remains verification-only. |
| T302 | Completed the 80386 operand/address/stack family. It corrected 16-bit fetch wrapping, crossed stack/frame width behavior, shared zero-length range validation, and `OUTSB` direction; all 131 CTests and its 0302 artifact passed. Product observation yielded no guest checkpoint because the process had no discoverable window handle; manual observation remains verification-only. |
| T303 | Completed the 80386 same-privilege control-transfer family. It fixed `LOOP` target-fault count publication, `CALL ptr16:32` decode width, same-CPL `RETF` DPL/RPL validation, and far-indirect register `#UD`; 51 gates, 132 CTests, and the 0303 artifact passed. Product observation remained host-window limited before guest input. |
| T304 | Completed the 80386 descriptor-table and system-control family. It corrected table register forms, fixed r/m16 selector/MSW widths, completed the admitted CR2/CR3 forms, and preserved existing 16/32-bit TSS LTR consumers; 51 gates, 133 CTests, and the 0304 artifact passed. Product observation remained host-window limited before guest input. |

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
