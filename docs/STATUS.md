# Project Status

## Current Work

**Active: M5 T311 S1.**

## M5 T311 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New; M5 T311 S1 P0 CPL3 paging and user/kernel boundary admission; Coordinated Dual-Session Mode. T310 is accepted and closed. |
| Admission And Approval | T311 is the next linear task selected by the first remaining direct M5 80386 package candidate in `QUEUE.md`. It follows T258's CPL0 4 KiB baseline and T305--T308's bounded 32-bit exception delivery. |
| Objective | Audit and freeze the smallest consumer-neutral 80386 paging permission scope: CPL3 U/S and R/W checks, `CR0.WP`, precise `#PF` error bits and CR2, cross-page checked accesses, and only any TLB/`INVLPG` behavior that the established execution path demonstrably requires. |
| Non-goals | PAE, host-memory substitution, generic paging rewrite, new instruction families beyond an evidence-required `INVLPG`, page-fault policy beyond the admitted existing delivery boundary, task/V86/debug/test/system expansion, product UX, public ABI, source import, a second executor, or a guest-image build fixture. |
| Reference Baseline | Accepted T310 artifact `vm-0-5-0310` / `nxvm_0_5_0310.exe`; retained T258 CPL0 paging, T305--T308 interrupt/delivery, and T306 return evidence. |
| Files And ABI Surface | S1 changes only task evidence and status records. It must not change CPU behavior, CMake current identity, public interfaces, cross-module ownership, or product UX. |
| Applicable Rules | `rules/EXECUTION.md`, `rules/ARCHITECTURE.md`, `rules/CODING.md`, `rules/DOCUMENT.md`, and `etc/operations/policy/source-policy.md`; retain one core executor/state owner and existing checked memory, stack, and fault routes. |
| Verification | Intel 80386 PRM is authoritative. Record versioned read-only Bochs 2.6 and PCjs 2.00.0 behavior paths; use focused prepared-state probes only. |
| Expected Markers | S1 adds no executable marker or artifact. It records the form/access matrix, existing owner/path map, exact page-fault bit and delivery assumptions, focused prepared-state probe plan, batch boundaries, and deferred policy. |
| S1 Audit Record | [T311 paging boundary admission](etc/evidence/t311-paging-boundary-admission.md) records the authority, matrix, retained intersections, and batch boundaries. |
| Asset Needs | Read-only local references only; no guest media, firmware, or third-party source is committed. |
| Original Owner Request | Execute the direct M5 80386 protected execution/delivery package in coordinated mode, stopping before Mantle; use Intel as authority with read-only Bochs and PCjs comparison. |
| Similar-Issue Sweep | Sweep all paging translation, permission, A/D, CR2, page-fault, cross-page, TLB, `INVLPG`, instruction-fetch, data, stack, and existing exception-delivery paths that could participate in this scope. |
| Stop Conditions | Stop and report an Intel/reference disagreement, any required host-policy or product dependency, an unsplittable privilege/delivery change, a need for a second executor or memory route, or a required expansion into PAE/task/V86/system behavior. |
| Exit Criteria | S1 records the exact admitted matrix and implementation batches, cites Intel and read-only comparison identities, passes documentation governance and diff check, is committed and immediately pushed, and remains active pending coordinator acceptance. |

## Current Technical Baseline

- **T310 pending artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0310` / `nxvm_0_5_0310.exe`.
  T310 remains active pending coordinator acceptance.
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
