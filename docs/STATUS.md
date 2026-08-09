# Project Status

## Current Work

**Idle.**

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
