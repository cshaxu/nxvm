# Project Status

## Current Work

**Active: M5 T310 S8.**

## M5 T310 S8 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective; M5 T310 S8 two-operand IMUL implementation; Coordinated Dual-Session Mode. T310 S1--S7 are accepted. |
| Admission And Approval | T310 is the next linear task selected by T309's accepted form audit; it precedes paging because its forms are metadata-valid and dispatch-reachable but lack focused semantic evidence. |
| Objective | Implement and prove 80386 two-operand signed `IMUL` (`0F AF`) through the existing core decoder, ModRM, checked-memory, and fault routes. |
| Non-goals | SETcc, MOVZX/MOVSX, bit-test/modify, SHLD/SHRD or BSF/BSR changes; one-operand or immediate IMUL; paging, debug/test registers, task/V86/system extensions, later-CPU forms, product UX, public ABI, source import, a second executor, or guest-image build fixtures. |
| Reference Baseline | Accepted T308 artifact `vm-0-5-0308` / `nxvm_0_5_0308.exe`; accepted T310 S7 scan evidence `22ed9f3`. |
| Files And ABI Surface | S8 may change core CPU two-operand IMUL execution, one focused prepared-state probe/CMake registration, and task records. It must not change public interfaces, cross-module ownership, or product UX. |
| Applicable Rules | `rules/EXECUTION.md`, `rules/ARCHITECTURE.md`, `rules/CODING.md`, `rules/DOCUMENT.md`, and `etc/operations/policy/source-policy.md`; retain one core executor/state owner and existing checked memory, stack, and fault routes. |
| Verification | Intel 80386 PRM is authoritative. Record versioned read-only Bochs 2.6 and PCjs 2.00.0 behavior paths; use focused prepared-state probes only. |
| Expected Markers | The IMUL marker proves signed 16/32-bit register and memory sources, in-range and overflow products, CF/OF behavior, `66h`/`67h`, 80186/80286 `#UD` before operand access, and failed-read nonpublication. Undefined flags other than CF/OF are not test oracles. One-operand and immediate IMUL remain outside S8. The retained 8086 `0F` POP CS compatibility path remains unchanged. S8 creates no artifact. |
| S2 Audit Record | [T310 0F integer bit/data admission audit](etc/evidence/t310-0f-integer-bit-data-admission.md) records the authority, form matrix, static candidates, focused-probe rules, batch boundaries, retained intersections, and deferrals. |
| Asset Needs | Read-only local references only; no guest media, firmware, or third-party source is committed. |
| Original Owner Request | Execute the direct M5 80386 protected execution/delivery package in coordinated mode, stopping before Mantle; use Intel as authority with read-only Bochs and PCjs comparison. |
| Similar-Issue Sweep | Sweep AF metadata/table/decoder path, signed multiply helper, register/reference read/write paths, operand/address-size selection, CF/OF behavior, profile gate, and focused probe registration. |
| S3 Evidence Record | [T310 SETcc evidence](etc/evidence/t310-0f-integer-bit-data-admission.md#s3-setcc-evidence) is retained unchanged. |
| S4 Evidence Record | [T310 MOVZX/MOVSX evidence](etc/evidence/t310-0f-integer-bit-data-admission.md#s4-movzxmovsx-evidence) is retained unchanged. |
| S5 Evidence Record | [T310 bit-test/modify evidence](etc/evidence/t310-0f-integer-bit-data-admission.md#s5-bit-testmodify-evidence) is retained unchanged. |
| S6 Evidence Record | [T310 SHLD/SHRD evidence](etc/evidence/t310-0f-integer-bit-data-admission.md#s6-shldshrd-evidence) is retained unchanged. |
| S7 Evidence Record | [T310 BSF/BSR evidence](etc/evidence/t310-0f-integer-bit-data-admission.md#s7-bsfbsr-evidence) is retained unchanged. |
| S8 Evidence Record | [T310 two-operand IMUL evidence](etc/evidence/t310-0f-integer-bit-data-admission.md#s8-two-operand-imul-evidence) records the signed product, CF/OF, profile, and checked-read proof. |
| Stop Conditions | Stop and report an Intel/reference disagreement, a scope expansion to one-operand or immediate IMUL, a pre-80386 path that accesses an operand, any need for a second executor or memory route, a public ABI change, or a failed retained intersection. |
| Exit Criteria | S8 proves all admitted two-operand IMUL forms and failure boundaries, passes relevant focused and retained probes, documentation governance, and diff check, pushes its successful local commit before reporting, and remains active pending coordinator acceptance. |

## Current Technical Baseline

- **T308 accepted artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0308` / `nxvm_0_5_0308.exe`.
  T308 and T309 are closed; T310 is the active linear task.
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
