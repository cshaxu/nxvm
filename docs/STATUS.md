# Project Status

## Current Work

**Active: M5 T311 S4.**

## M5 T311 S4 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective; M5 T311 S4 cross-page checked-access boundary. T311 S1 P0, S2, and S3 are accepted; Coordinated Dual-Session Mode. |
| Admission And Approval | T311 is the next linear task selected by the first remaining direct M5 80386 package candidate in `QUEUE.md`. S4 follows T258's CPL0 4 KiB baseline, T311 S3 permissions, and T305--T308's bounded 32-bit exception delivery. |
| Objective | Make exactly-one-4-KiB-boundary fetch, ordinary data, and constructible stack checked accesses validate both pages before any guest-data or paging A/D publication, using only the existing core translation and checked physical-memory route. |
| Non-goals | `INVLPG`/TLB, page-fault IDT delivery, PAE, large pages, host-memory substitution, generic paging rewrite, V86, task/system expansion, timing or product UX changes, public ABI, source import, a second executor/memory route, a guest-image build fixture, artifact, or Setup observation. |
| Reference Baseline | Accepted T310 artifact `vm-0-5-0310` / `nxvm_0_5_0310.exe`; retained T258 CPL0 paging, T311 S3 permission, T302 operand/address, T303 control-transfer, T306 return, and T308 delivery evidence. |
| Files And ABI Surface | S4 may refactor private core page-walk candidate and A/D publication ordering, extend the existing focused prepared-state paging probe/CMake registration, and update task records. It must not change public interfaces, cross-module ownership, or product UX. |
| Applicable Rules | `rules/EXECUTION.md`, `rules/ARCHITECTURE.md`, `rules/CODING.md`, `rules/DOCUMENT.md`, and `etc/operations/policy/source-policy.md`; retain one core executor/state owner and existing checked memory, stack, and fault routes. |
| Verification | Intel 80386 PRM is authoritative. Record versioned read-only Bochs 2.6 and PCjs 2.00.0 behavior paths; use focused prepared-state probes only. |
| Expected Markers | The cross-page marker proves two-page fetch/data/constructible-stack completion, per-page A and write-page D publication, exact second-page CR2/P-W-U faults, and no guest-data, register, instruction, stack, or A/D publication when the second page fails. S4 creates no artifact. |
| S1 Audit Record | [T311 paging boundary admission](etc/evidence/t311-paging-boundary-admission.md) records the authority, matrix, retained intersections, and batch boundaries. |
| Asset Needs | Read-only local references only; no guest media, firmware, or third-party source is committed. |
| Original Owner Request | Execute the direct M5 80386 protected execution/delivery package in coordinated mode, stopping before Mantle; use Intel as authority with read-only Bochs and PCjs comparison. |
| Similar-Issue Sweep | Sweep all paging translation, permission, A/D, CR2, page-fault, cross-page, TLB, `INVLPG`, instruction-fetch, data, stack, and existing exception-delivery paths that could participate in this scope. |
| Stop Conditions | Stop and report an Intel/reference disagreement, any need for #PF IDT delivery, a second executor/memory route, an A/D atomicity rule that cannot be frozen on the existing route, or expansion into TLB/PAE/large-page/task/V86/system behavior. |
| Exit Criteria | S4 proves the admitted exactly-one-page-boundary outcomes and their producer diagnostics, passes focused and retained intersections plus documentation governance and diff check, pushes its successful local commit before reporting, and remains active pending coordinator acceptance. |

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
