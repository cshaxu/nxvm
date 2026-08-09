# Project Status

## Current Work

**Active: M5 T303 S5.**

## M5 T303 S5 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective; M5 T303 S5 completion of implementation batch C; Coordinated Dual-Session Mode. S4 fixed far immediate and same-CPL `RETF` defects but did not yet prove far indirect forms or the required negative-state boundary. |
| Admission And Approval | Owner approved the direct M5 80386 32-bit protected execution/delivery package and instructed coordinator/executor execution through the package, stopping before Mantle. T302 is closed; T303 is the next approved Queue candidate. |
| Objective | Admit the 80386 32-bit same-privilege control-transfer family: near/far `JMP`, `CALL`, `RET`, `RETF`, conditional-relative and loop-family forms, including operand-size variants, code-limit checks, return-frame ordering, and failure-before-visible-commit rules. Freeze exact form/profile/mode and commit matrices before implementation. |
| Non-goals | Privilege transitions, call gates, task switching, interrupt/exception delivery, descriptor/system changes, paging policy, virtual-8086, stack primitive rework already closed by T302, host/product UX, external ABI, and source import. No new executor or execution path. |
| Reference Baseline | `95950e0`; current artifact `vm-0-5-0302` / `nxvm_0_5_0302.exe`. |
| Files And ABI Surface | Initial audit may inspect core CPU control-transfer, stack, descriptor, and fault helpers plus focused tests/CMake and task/governance records. It must not change public core interfaces or cross-module ownership. |
| Applicable Rules | `rules/EXECUTION.md`, `rules/ARCHITECTURE.md`, `rules/CODING.md`, `rules/DOCUMENT.md`, and `etc/operations/policy/source-policy.md`; retain one core executor/state owner, core-to-VM dependency direction, and NXVM Console/debugger/boot behavior. |
| Verification | Intel 80386 PRM is authoritative; S1 records exact instruction entries and sections. Read-only comparison records relevant Bochs 2.6 and PCjs 2.00.0 behavior paths and versions; do not copy source. Project verification uses focused synthetic probes, not long-start guest fixtures. Implementation later runs focused probes, `current-gates-gcc`, documentation governance, `git diff --check`, one task artifact, and one owner-supplied Setup observation after family closure. |
| Expected Markers | S1 emits an audit record only. The later family probe marker is `M5:T303:CONTROL-TRANSFER:OK`; its task artifact is created only when CMake's current target is updated during implementation. |
| Asset Needs | Local read-only reference checkouts and owner-supplied local Setup media may be observed but are never committed, packaged, or made default-build inputs. |
| Stop Conditions | Stop and report on Intel/reference disagreement not resolved by a bounded optional bridge, a required architecture change, a second executor/state path, raw public-layout exposure, retained Console/debugger/boot regression, or a form owned by privilege/exception/task/V86 work. |
| Exit Criteria | Far immediate and indirect `CALL`/`JMP`, plus same-CPL `RETF`, follow the frozen pointer-width, selector/cache, descriptor/type/present/privilege/limit, return-frame, and failure-before-visible-commit matrix. Negative far-return cases prove first-fault plus EIP/SP/ESP/CS cache/flags/accessed-state preservation. Focused probes and required gates pass without touching outer return, gates, tasks, delivery, or later families. |
| Original Owner Request | Execute the 80386 task package in dual-session mode through the Mantle boundary; use Intel, Bochs, and PCjs to establish logic, observe Setup once per task, and use the optional bridge when uncertain. |
| Similar-Issue Sweep | Audit all near/far branch, call, return, condition/loop, code-limit, stack-frame, selector/cache, and fault/commit helpers in `src`, `tests`, and CMake. Each production-path hit must be classified in the S1 record; do not silently widen scope. |
| S1 Audit Record | [T303 control-transfer admission audit](etc/evidence/t303-control-transfer-admission.md) freezes the form/profile/mode, fault/commit, focused-probe, and deferred-owner matrices without changing CPU behavior. |

## Current Technical Baseline

- **T302 artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0302` / `nxvm_0_5_0302.exe`.
  The S5 build hash, full-gate result, and bounded Setup observation are
  recorded in [T302 history](history/M5-T302-operand-address-stack-family.md)
  and were independently accepted by the coordinator.
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
| T295 | Moved CPU execution-to-shared-PIC binding into `core_machine_create`; VM session no longer borrows or binds that path. CPU/PIC lifecycle evidence and all current gates passed; RTC/FDC/HDC/DMA remain T296 and debugger raw borrows remain T298/T299. |
| T296 | Completed A VADP/ports, B DMA/RTC/CMOS/NMI, and C FDC/HDC ownership migration through frozen typed core submissions. S2 `e84199e`, S3 `a02a0f0`, and S4 `fa18847d0aed685554f786c89ba0f5908e539fb7` passed focused owner/lifecycle evidence and the final 49-gate, 125-test verification; T298--T299 remained deferred at this closure. |
| T297 | Replaced default firmware raw profile binding with an opaque core-invoked capability for checked memory/port I/O and stop requests. Configure-time immutable ROM registration now rolls back atomically on callback failure; default BIOS/QDCGA, boot failure, 49 gates, and 126/126 CTests are closed without product UX change. |
| T298 | Replaced formal debugger raw CPU/instruction/execution/RAM/port borrows with paused-or-stopped named core operations and copied observations. The retained Console/debugger UX passed focused checks and the 126-test regression suite without a global target or second execution path. |
| T299 | Removed the remaining public configuration/debug raw borrows and profile raw binding. Production uses typed/copying interfaces; purpose-named test fixtures alone may include private core state. The recurrence gate, 51 current static/governance targets, and 126 CTests passed without product behavior changes. |
| T300 | Unified the core directional port registry, propagated typed-provider failures, made VM session initialization failure-atomic, and removed public private layouts. The DMA binding uses a core-issued non-addressable nonce and core-internal revalidation; 51 static/governance gates and 129 CTests passed with its task-level developer artifact. |
| T301 | Completed the 80386 segment-selector family, including atomic segment loads and selector queries. The S4 sweep corrected non-present query handling and system-descriptor DPL/RPL visibility; 51 static/governance checks, 130 CTests, and the T301 artifact passed. The bounded product observation reached no guest checkpoint because host window discovery failed before guest input; manual observation remains verification-only. |
| T302 | Completed the 80386 operand/address/stack family. It corrected 16-bit fetch wrapping, crossed stack/frame width behavior, shared zero-length range validation, and `OUTSB` direction; all 131 CTests and its 0302 artifact passed. Product observation yielded no guest checkpoint because the process had no discoverable window handle; manual observation remains verification-only. |

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
