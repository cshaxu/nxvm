# Project Status

## Current Work

**Active: M5 T302 S2.**

## M5 T302 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective; M5 T302 S2 implementation batch A; Coordinated Dual-Session Mode. S1 completed the committed admission matrix; S2 implements only prefix, fetch, effective-address, moffs, and general-data semantics. |
| Admission And Approval | Owner approved the direct M5 80386 32-bit protected execution/delivery package and instructed coordinator/executor execution through the package, stopping before Mantle. T301 is closed; T302 is the next approved Queue candidate. |
| Objective | Admit the 80386 32-bit operand, address, and stack-instruction family: `66h`/`67h` forms for ModRM/SIB effective addresses, instruction fetch, general data access, `PUSH`/`POP`/`PUSHA`/`POPA`, `PUSHF`/`POPF`, `ENTER`/`LEAVE`, and string/REP access under 32-bit code/data/stack segments. Freeze exact form, profile, mode, limit, wrapping, segment-default, fault-order, and commit rules before implementation. |
| Non-goals | Control-transfer semantics, descriptor-table/system instructions, interrupt/exception delivery, privilege transitions, paging-policy expansion, virtual-8086, arithmetic-family bulk work, host/product UX, external ABI, and source import. No new executor or execution path. |
| Reference Baseline | `b2f1d6f7f716ce3865e0c05ca2ac98ae2f8d4eac`; current artifact `vm-0-5-0301` / `nxvm_0_5_0301.exe`. |
| Files And ABI Surface | Initial audit may inspect `src/core/machine/cpu_instructions.c`, `cpu.h`, memory/fetch helpers, focused machine tests/CMake, and task/governance records. It must not change public core interfaces or cross-module ownership. |
| Applicable Rules | `rules/EXECUTION.md`, `rules/ARCHITECTURE.md`, `rules/CODING.md`, `rules/DOCUMENT.md`, and `etc/operations/policy/source-policy.md`; retain one core executor and state owner, core-to-VM dependency direction, and NXVM Console/debugger/boot behavior. |
| Verification | Intel 80386 PRM is authoritative; S1 records exact instruction entries and sections. Read-only comparison records the relevant Bochs 2.6 and PCjs 2.00.0 behavior paths and versions; do not copy source. Project verification uses focused synthetic probes, not long-start guest fixtures. Implementation later runs focused probes, `current-gates-gcc`, documentation governance, `git diff --check`, one task artifact, and one owner-supplied Setup observation after family closure. |
| Expected Markers | S1 emits an audit record only. The later family probe marker is `M5:T302:OPERAND-ADDRESS-STACK:OK`; its task artifact is created only when CMake's current target is updated during implementation. |
| Asset Needs | Local read-only reference checkouts and owner-supplied local Setup media may be observed but are never committed, packaged, or made default-build inputs. |
| Stop Conditions | Stop and report on Intel/reference disagreement not resolved by a bounded optional bridge, a required architecture change, a second executor/state path, raw public-layout exposure, retained Console/debugger/boot regression, or an instruction form that belongs to another queued family. |
| Exit Criteria | The S2 paths follow the frozen matrix for 16/32 code defaults, `66h`/`67h`, ModRM/SIB/moffs, DS/SS defaults and overrides, cross-width general data, limit/wrap, and non-mutating decode/access failure. Focused probes and required gates pass without touching stack opcodes, strings/REP, control transfer, or later families. |
| Original Owner Request | Execute the 80386 task package in dual-session mode through the Mantle boundary; use Intel, Bochs, and PCjs to establish logic, observe Setup once per task, and use the optional bridge when uncertain. |
| Similar-Issue Sweep | Audit all existing 66h/67h decode, effective-address, stack, fetch, string/REP, segment-limit, and commit helpers in `src`, `tests`, and CMake. Each production-path hit must be classified in the S1 record; do not silently widen scope. |
| S1/S2 Evidence Record | [T302 operand/address/stack admission audit](etc/evidence/t302-operand-address-stack-admission.md) freezes the form matrix and records S2's focused Batch A correction and probe result. S2 does not change the current artifact target or Queue entry. |

## Current Technical Baseline

- **T301 artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0301` / `nxvm_0_5_0301.exe`.
  S4 rebuilt `nxvm_0_5_0301.exe` with SHA-256
  `54D6BEB2BEF9E495C09DD4AA976288449EB0608567A2F32F61693219FFA23E3D`.
  The focused selector and retained ARPL probes, 51 static/governance targets,
  and 130 CTests passed; the coordinator independently accepted the result.
- **T301 observation limit:** one bounded product-window observation budget
  could not discover its host window before any guest command, stdout/stderr,
  or process residue existed. It is not a Windows Setup checkpoint or product
  regression. A later owner-controlled manual observation remains verification
  only and does not change the CPU conclusion or create a repair task.
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
| T294 | Completed the codebase-driven public-surface/raw-borrow matrix, fixed T296's three stages and T297's future firmware-capability lifecycle precondition, and left the then-reserved pre-decode/combined-commit work conditional; no runtime behavior, CMake graph, or artifact changed. |
| T295 | Moved CPU execution-to-shared-PIC binding into `core_machine_create`; VM session no longer borrows or binds that path. CPU/PIC lifecycle evidence and all current gates passed; RTC/FDC/HDC/DMA remain T296 and debugger raw borrows remain T298/T299. |
| T296 | Completed A VADP/ports, B DMA/RTC/CMOS/NMI, and C FDC/HDC ownership migration through frozen typed core submissions. S2 `e84199e`, S3 `a02a0f0`, and S4 `fa18847d0aed685554f786c89ba0f5908e539fb7` passed focused owner/lifecycle evidence and the final 49-gate, 125-test verification; T298--T299 remained deferred at this closure. |
| T297 | Replaced default firmware raw profile binding with an opaque core-invoked capability for checked memory/port I/O and stop requests. Configure-time immutable ROM registration now rolls back atomically on callback failure; default BIOS/QDCGA, boot failure, 49 gates, and 126/126 CTests are closed without product UX change. |
| T298 | Replaced formal debugger raw CPU/instruction/execution/RAM/port borrows with paused-or-stopped named core operations and copied observations. The retained Console/debugger UX passed focused checks and the 126-test regression suite without a global target or second execution path. |
| T299 | Removed the remaining public configuration/debug raw borrows and profile raw binding. Production uses typed/copying interfaces; purpose-named test fixtures alone may include private core state. The recurrence gate, 51 current static/governance targets, and 126 CTests passed without product behavior changes. |
| T300 | Unified the core directional port registry, propagated typed-provider failures, made VM session initialization failure-atomic, and removed public private layouts. The DMA binding uses a core-issued non-addressable nonce and core-internal revalidation; 51 static/governance gates and 129 CTests passed with its task-level developer artifact. |
| T301 | Completed the 80386 segment-selector family, including atomic segment loads and selector queries. The S4 sweep corrected non-present query handling and system-descriptor DPL/RPL visibility; 51 static/governance checks, 130 CTests, and the T301 artifact passed. The bounded product observation reached no guest checkpoint because host window discovery failed before guest input; manual observation remains verification-only. |

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
