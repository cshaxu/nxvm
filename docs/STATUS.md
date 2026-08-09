# Project Status

## Current Work

**Active: M5 T301 S4.**

## M5 T301 S4 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective; M5 T301 S4 closure; Coordinated Dual-Session Mode. S1 admitted the family, S2 implemented the complete focused synthetic form/profile/mode matrix, and the committed S3 packet admitted the closure review. |
| Admission And Approval | Owner approved the direct M5 80386 32-bit protected execution/delivery package and instructed coordinator/executor execution through the package, stopping before Mantle. Coordinator additionally approved S2 with an instruction-form, CPU-profile, and execution-mode matrix. |
| Objective | Independently close the Queue's 32-bit segmentation and selector-instruction family: `MOV`/`POP` segment loads, `LDS`/`LES`/`LFS`/`LGS`/`LSS`, `ARPL`, `LAR`, `LSL`, `VERR`, and `VERW`, with all legal 80386 operand-size forms and correct descriptor/cache semantics. |
| Non-goals | LDT breadth, task gates, virtual-8086, task switching, paging-policy expansion, host/product UX, external ABI, and source import. Reserved and later-CPU forms remain `#UD`. |
| Reference Baseline | `87cb79993c5901cefd25d9228cf60b87d54b7d4e`; current artifact `vm-0-5-0301` / `nxvm_0_5_0301.exe`. |
| Files And ABI Surface | `src/core/machine/cpu_instructions.c`, `cpu.h`, existing core CPU interfaces, focused machine tests/CMake, and task/governance records only. No new public raw borrow or cross-module interface. |
| Applicable Rules | `rules/EXECUTION.md`, `rules/ARCHITECTURE.md`, `rules/CODING.md`, `rules/DOCUMENT.md`, and `etc/operations/policy/source-policy.md`; preserve one core executor/state owner and retained NXVM behavior. |
| Verification | Intel 80386 PRM Ch. 3, 5, 14, and 17 is authoritative; record the relevant instruction entries. Read-only comparison references are Bochs 2.6 (`cpu/protect_ctrl.cc`, `cpu/segment_ctrl.cc`, and `cpu/fetchdecode.cc`) and PCjs 2.00.0 (`machines/pcx86/modules/v2/x86ops.js`, `x86op0f.js`, and `segx86.js`); do not copy source. The form matrix is: `ARPL`/`LAR`/`LSL`/`VERR`/`VERW` are selector-check forms and are `#UD` outside protected mode; `MOV`/`POP`/`LDS`/`LES`/`LFS`/`LGS`/`LSS` retain their architecturally valid real-mode segment-load forms, while virtual-8086 remains deferred. Every LxS second operand is memory-only. `ARPL` operands remain `r/m16,r16`; `66h` changes `POP` segment stack transfer width and LxS offset/destination width, never the 16-bit selector source. FS/GS forms require 80386; reserved/later encodings remain `#UD`. Run focused form/profile/mode/atomicity probes, `current-gates-gcc`, documentation governance, `git diff --check`, and one owner-supplied local Windows Setup observation after family closure. |
| Expected Markers | Family probe marker `M5:T301:SEGMENT-SELECTOR:OK`; T301 developer artifact after its CMake target is updated; Setup observation records checkpoint/stop/timeout only. |
| Asset needs | Local read-only reference checkouts and owner-supplied local Setup media may be observed but are never committed, packaged, or made default-build inputs. |
| Stop conditions | Stop and report on a source/license ambiguity, Intel/Bochs/PCjs disagreement not resolved by a bounded optional bridge, required architecture change, second executor/state path, public raw-layout exposure, or retained Console/debugger/boot regression. |
| Exit criteria | The full named family is covered by project probes and profile rejections; all allowed forms follow the frozen form/profile/mode matrix; protected segment caches, destination registers, stack state, descriptor accessed state, and flags mutate only after all required validation succeeds; required gates pass; Setup observation is recorded; artifact exists with SHA-256; coordinator independently accepts the result. |
| Original owner request | Execute the 80386 task package in dual-session mode through the Mantle boundary; use Intel, Bochs, and PCjs to establish logic, observe Setup once per task, and use the optional bridge when uncertain. |
| Similar-issue sweep | Search all selector/descriptor instruction handlers, 16/32-bit forms, profile gates, and fault/commit paths in `src`, `tests`, and CMake; every production-path hit is fixed, explicitly not applicable, or deferred in `TODO.md`. |

## Current Technical Baseline

- **T301 artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0301` / `nxvm_0_5_0301.exe`.
  S4 rebuilt `nxvm_0_5_0301.exe` with SHA-256
  `54D6BEB2BEF9E495C09DD4AA976288449EB0608567A2F32F61693219FFA23E3D`.
  The focused selector and retained ARPL probes, 51 static/governance targets,
  and 130 CTests passed; implementation evidence remains pending coordinator
  acceptance.
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
| T293 | Removed the unused post-`#UD` transition surface, made protected outer `RETF`/`IRET` frame validation non-mutating until commit, and completed its exception/atomicity matrix: non-present returned `CS` is `#NP`, non-present `SS` is the retained terminal `#SS`, and invalid entries are `#GP`; all six `RETF`/outer-`IRET` cases preserve the defined pre-commit boundary. |
| T294 | Completed the codebase-driven public-surface/raw-borrow matrix, fixed T296's three stages and T297's future firmware-capability lifecycle precondition, and left the then-reserved pre-decode/combined-commit work conditional; no runtime behavior, CMake graph, or artifact changed. |
| T295 | Moved CPU execution-to-shared-PIC binding into `core_machine_create`; VM session no longer borrows or binds that path. CPU/PIC lifecycle evidence and all current gates passed; RTC/FDC/HDC/DMA remain T296 and debugger raw borrows remain T298/T299. |
| T296 | Completed A VADP/ports, B DMA/RTC/CMOS/NMI, and C FDC/HDC ownership migration through frozen typed core submissions. S2 `e84199e`, S3 `a02a0f0`, and S4 `fa18847d0aed685554f786c89ba0f5908e539fb7` passed focused owner/lifecycle evidence and the final 49-gate, 125-test verification; T298--T299 remained deferred at this closure. |
| T297 | Replaced default firmware raw profile binding with an opaque core-invoked capability for checked memory/port I/O and stop requests. Configure-time immutable ROM registration now rolls back atomically on callback failure; default BIOS/QDCGA, boot failure, 49 gates, and 126/126 CTests are closed without product UX change. |
| T298 | Replaced formal debugger raw CPU/instruction/execution/RAM/port borrows with paused-or-stopped named core operations and copied observations. The retained Console/debugger UX passed focused checks and the 126-test regression suite without a global target or second execution path. |
| T299 | Removed the remaining public configuration/debug raw borrows and profile raw binding. Production uses typed/copying interfaces; purpose-named test fixtures alone may include private core state. The recurrence gate, 51 current static/governance targets, and 126 CTests passed without product behavior changes. |
| T300 | Unified the core directional port registry, propagated typed-provider failures, made VM session initialization failure-atomic, and removed public private layouts. The DMA binding uses a core-issued non-addressable nonce and core-internal revalidation; 51 static/governance gates and 129 CTests passed with its task-level developer artifact. |

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
