# Project Status

## Current Work

**Active: M5 T304 S3.**

## M5 T304 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective; M5 T304 S3 selector-table and bounded task-register batch; Coordinated Dual-Session Mode. S2 closed descriptor-table forms. |
| Admission And Approval | T303 is closed. T304 is the next linear Queue candidate in the owner-approved M5 80386 protected execution/delivery package. |
| Objective | Admit the 80386 descriptor-table and system-control family: `SGDT`/`SIDT`, `LGDT`/`LIDT`, `SLDT`/`LLDT`, `STR`/`LTR`, `SMSW`/`LMSW`, `CLTS`, and permitted `MOV CRx` forms. Freeze exact 16/32 form, profile, privilege, fault, and commit matrices before implementation. |
| Non-goals | Debug registers, 32-bit TSS switching, paging-policy expansion, task gates, virtual-8086, exception delivery, later-CPU opcodes, product UX, public ABI, and source import. No new executor or execution path. |
| Reference Baseline | `05d4b4d`; current artifact `vm-0-5-0303` / `nxvm_0_5_0303.exe`. |
| Files And ABI Surface | S1 may inspect core CPU decoder, descriptor/table/control helpers, focused tests, CMake, and task records. It must not change public interfaces or cross-module ownership. |
| Applicable Rules | `rules/EXECUTION.md`, `rules/ARCHITECTURE.md`, `rules/CODING.md`, `rules/DOCUMENT.md`, and `etc/operations/policy/source-policy.md`; retain one core executor/state owner and NXVM behavior. |
| Verification | Intel 80386 PRM is authoritative. Record versioned read-only Bochs 2.6 and PCjs 2.00.0 behavior paths; do not copy source. Construction uses focused synthetic probes only. S1 is audit only. |
| Expected Markers | S1 records an audit only. Any later focused probe marker and artifact target are admitted after the matrix. |
| Asset Needs | Read-only local references only; no guest media, firmware, or third-party source is committed. |
| Original Owner Request | Execute the direct M5 80386 protected execution/delivery package in coordinated mode, stopping before Mantle; use Intel as authority with read-only Bochs and PCjs comparison. |
| Similar-Issue Sweep | Audit decoder metadata, profile gates, descriptor/table/control helpers, privilege checks, fault helpers, current focused tests, and CMake registrations. Classify every production hit before implementation. |
| S1 Audit Record | [T304 descriptor-table and system-control admission audit](etc/evidence/t304-descriptor-system-control-admission.md) freezes the instruction matrix, implementation batches, focused-probe boundary, and deferred-owner paths without changing CPU behavior. |
| Stop Conditions | Stop and report any unresolved Intel/reference disagreement, required architecture change, second state/execution path, public raw-layout exposure, or behavior owned by delivery, task, paging, V86, or a later family. |
| Exit Criteria | `SLDT`, `LLDT`, `STR`, and the existing bounded 16-bit-TSS `LTR` path satisfy fixed r/m16, mode/CPL, null/TI/type/present/busy, cache/selector/busy-byte commit, and failure-preservation rules. Focused probes and required narrow gates pass without 32-bit TSS, gate, or delivery work. |

## Current Technical Baseline

- **T303 accepted artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0303` / `nxvm_0_5_0303.exe`.
  The S7 build hash, full-gate result, and bounded product-observation
  limitation are recorded in [the T303 evidence record](etc/evidence/t303-control-transfer-admission.md).
  T303 is accepted; T304 is the next linear candidate.
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
| T296 | Completed A VADP/ports, B DMA/RTC/CMOS/NMI, and C FDC/HDC ownership migration through frozen typed core submissions. S2 `e84199e`, S3 `a02a0f0`, and S4 `fa18847d0aed685554f786c89ba0f5908e539fb7` passed focused owner/lifecycle evidence and the final 49-gate, 125-test verification; T298--T299 remained deferred at this closure. |
| T297 | Replaced default firmware raw profile binding with an opaque core-invoked capability for checked memory/port I/O and stop requests. Configure-time immutable ROM registration now rolls back atomically on callback failure; default BIOS/QDCGA, boot failure, 49 gates, and 126/126 CTests are closed without product UX change. |
| T298 | Replaced formal debugger raw CPU/instruction/execution/RAM/port borrows with paused-or-stopped named core operations and copied observations. The retained Console/debugger UX passed focused checks and the 126-test regression suite without a global target or second execution path. |
| T299 | Removed the remaining public configuration/debug raw borrows and profile raw binding. Production uses typed/copying interfaces; purpose-named test fixtures alone may include private core state. The recurrence gate, 51 current static/governance targets, and 126 CTests passed without product behavior changes. |
| T300 | Unified the core directional port registry, propagated typed-provider failures, made VM session initialization failure-atomic, and removed public private layouts. The DMA binding uses a core-issued non-addressable nonce and core-internal revalidation; 51 static/governance gates and 129 CTests passed with its task-level developer artifact. |
| T301 | Completed the 80386 segment-selector family, including atomic segment loads and selector queries. The S4 sweep corrected non-present query handling and system-descriptor DPL/RPL visibility; 51 static/governance checks, 130 CTests, and the T301 artifact passed. The bounded product observation reached no guest checkpoint because host window discovery failed before guest input; manual observation remains verification-only. |
| T302 | Completed the 80386 operand/address/stack family. It corrected 16-bit fetch wrapping, crossed stack/frame width behavior, shared zero-length range validation, and `OUTSB` direction; all 131 CTests and its 0302 artifact passed. Product observation yielded no guest checkpoint because the process had no discoverable window handle; manual observation remains verification-only. |
| T303 | Completed the 80386 same-privilege control-transfer family. It fixed `LOOP` target-fault count publication, `CALL ptr16:32` decode width, same-CPL `RETF` DPL/RPL validation, and far-indirect register `#UD`; 51 gates, 132 CTests, and the 0303 artifact passed. Product observation remained host-window limited before guest input. |

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
