# Project Status

## Current Work

**Active: M5 T306 S7.**

## M5 T306 S7 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective; M5 T306 S7 protected-return family closure preparation; Coordinated Dual-Session Mode. S2--S6 are accepted. |
| Admission And Approval | T305 is closed. T306 is the next linear Queue candidate in the owner-approved M5 80386 protected execution/delivery package. |
| Objective | Close the admitted 80386 protected-return family: same-CPL and outer `IRET`, aligned outer `RETF`, atomic selector/cache/stack behavior, and EFLAGS privilege masks. |
| Non-goals | Task return, nested-task return, task gates, call gates, general task switching, virtual-8086 returns, new exception origins, reset/triple-fault policy, paging-policy expansion, product UX, public ABI, and source import. No new executor or return path. |
| Reference Baseline | `5bf0127`; T305 is the accepted prior artifact baseline, and S7 stages the current T306 identity. |
| Files And ABI Surface | S7 may change only CMake target/preset identity, T306 evidence/history/STATUS, and task artifact records. It must not alter CPU semantics, public interfaces, or cross-module ownership. |
| Applicable Rules | `rules/EXECUTION.md`, `rules/ARCHITECTURE.md`, `rules/CODING.md`, `rules/DOCUMENT.md`, and `etc/operations/policy/source-policy.md`; retain one core executor/state owner, a single checked stack/memory route, and NXVM behavior. |
| Verification | Intel 80386 PRM is authoritative. Record versioned read-only Bochs 2.6 and PCjs 2.00.0 behavior paths; do not copy source. Construction uses focused synthetic probes only; full-system observation is closure-only. |
| Expected Markers | T306 S2/S4/S5 markers plus retained T293/T303/T304/T305/T260/T261 markers and the current closure artifact after its target switch. |
| Asset Needs | Read-only local references only; no guest media, firmware, or third-party source is committed. |
| Original Owner Request | Execute the direct M5 80386 protected execution/delivery package in coordinated mode, stopping before Mantle; use Intel as authority with read-only Bochs and PCjs comparison. |
| Similar-Issue Sweep | Review the completed T306 return, stack, selector/cache, flags, and retained-probe surfaces for an unclassified production hit before closure. Any new semantic defect stops for coordinator direction. |
| S1 Audit Record | [T306 protected-return admission audit](etc/evidence/t306-protected-return-admission.md) freezes the admitted 16/32-bit matrix, producer/consumer intersections, batches, and stop boundaries before implementation. |
| S2 Evidence Record | [T306 protected-return admission audit](etc/evidence/t306-protected-return-admission.md#s2-same-cpl-iret-evidence) records same-CPL frame order, `66h`/`67h`/SS address-size behavior, failure preservation, and retained T293/T303/T305 intersections. |
| S3 Evidence Record | Restore outer `RETF`'s explicit nonconforming target rule at its retained validation priority; prove conforming-code rejection and unchanged state through the T293 retained probe. |
| S4 Evidence Record | [T306 protected-return admission audit](etc/evidence/t306-protected-return-admission.md#s4-outer-retf-evidence) records outer `RETF` 16/32-bit frame order, `66h`/`67h`/SS address-size/imm16 behavior, new SS/ESP validation, and pre-commit failure preservation. |
| S5 Evidence Record | [T306 protected-return admission audit](etc/evidence/t306-protected-return-admission.md#s5-outer-iret-evidence) records outer `IRET` 16/32-bit frame order, EFLAGS/CPL constraints, new SS/ESP validation, and pre-commit failure preservation. |
| S6 Evidence Record | [T306 protected-return admission audit](etc/evidence/t306-protected-return-admission.md#s6-outer-iret-eflags-privilege-evidence) records the exact Intel outer-`IRET` EFLAGS writable-mask decision, allowed and masked IF/IOPL/RF/VM cases, and failure-state preservation. |
| S7 Closure Record | [T306 protected-return admission audit](etc/evidence/t306-protected-return-admission.md#s7-closure-preparation-evidence) records the 0306 artifact, full current gates, retained probes, bounded product observation, and deferred task/V86/gate/paging boundary. |
| Stop Conditions | Stop and report an unresolved Intel/reference disagreement, required architecture change, second execution/state path, public raw-layout exposure, or behavior owned by task/V86/paging/later families. |
| Exit Criteria | Advance the current target during closure, generate its artifact and SHA-256, pass full current gates, documentation governance, diff check, and one bounded product observation. T306 remains active/pending coordinator acceptance; Queue is unchanged and T307 does not start. |

## Current Technical Baseline

- **T306 closure artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0306` / `nxvm_0_5_0306.exe`.
  T306 remains active pending coordinator acceptance.
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
| T298 | Replaced formal debugger raw CPU/instruction/execution/RAM/port borrows with paused-or-stopped named core operations and copied observations. The retained Console/debugger UX passed focused checks and the 126-test regression suite without a global target or second execution path. |
| T299 | Removed the remaining public configuration/debug raw borrows and profile raw binding. Production uses typed/copying interfaces; purpose-named test fixtures alone may include private core state. The recurrence gate, 51 current static/governance targets, and 126 CTests passed without product behavior changes. |
| T300 | Unified the core directional port registry, propagated typed-provider failures, made VM session initialization failure-atomic, and removed public private layouts. The DMA binding uses a core-issued non-addressable nonce and core-internal revalidation; 51 static/governance gates and 129 CTests passed with its task-level developer artifact. |
| T301 | Completed the 80386 segment-selector family, including atomic segment loads and selector queries. The S4 sweep corrected non-present query handling and system-descriptor DPL/RPL visibility; 51 static/governance checks, 130 CTests, and the T301 artifact passed. The bounded product observation reached no guest checkpoint because host window discovery failed before guest input; manual observation remains verification-only. |
| T302 | Completed the 80386 operand/address/stack family. It corrected 16-bit fetch wrapping, crossed stack/frame width behavior, shared zero-length range validation, and `OUTSB` direction; all 131 CTests and its 0302 artifact passed. Product observation yielded no guest checkpoint because the process had no discoverable window handle; manual observation remains verification-only. |
| T303 | Completed the 80386 same-privilege control-transfer family. It fixed `LOOP` target-fault count publication, `CALL ptr16:32` decode width, same-CPL `RETF` DPL/RPL validation, and far-indirect register `#UD`; 51 gates, 132 CTests, and the 0303 artifact passed. Product observation remained host-window limited before guest input. |
| T304 | Completed the 80386 descriptor-table and system-control family. It corrected table register forms, fixed r/m16 selector/MSW widths, completed the admitted CR2/CR3 forms, and preserved existing 16/32-bit TSS LTR consumers; 51 gates, 133 CTests, and the 0304 artifact passed. Product observation remained host-window limited before guest input. |
| T305 | Completed the 80386 32-bit interrupt and exception-entry family. It added same-CPL gate planning, software and external front ends, and bounded existing #GP/#NP/#SS error-frame delivery containment; 51 gates, 134 CTests, and the 0305 artifact passed. Product observation remained host-window limited before guest input. |

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
