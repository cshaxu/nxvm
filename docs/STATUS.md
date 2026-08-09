# Project Status

## Current Work

**Idle. M5 Td S46 is closed; no numeric implementation task is active.** The
first approved Queue candidate receives the next numeric task identifier.

## Current Technical Baseline

- **T300 artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0300` / `nxvm_0_5_0300.exe`.
  T300 closure evidence: the strengthened source-shape gate emits
  `M5:T300:S4:PUBLIC-INTERFACE-BOUNDARY:OK`; the focused debugger snapshot
  probe and artifact build passed. The coordinator closure is recorded below.
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
| T300 | Unified the core directional port registry, propagated typed-provider failures, made VM session initialization failure-atomic, and removed public private layouts. The DMA binding uses a core-issued non-addressable nonce and core-internal revalidation; 51 static/governance gates and 129 CTests passed with `nxvm_0_5_0300.exe`. |

Detailed closure evidence remains in [legacy history](etc/legacy-history/m5.md)
and Git. The [Queue](QUEUE.md), [Roadmap](design/ROADMAP.md), and
[M5 closure checklist](etc/governance/m5-closure-checklist.md) refine current
M5 scope; an active packet becomes an operational authority only after approval.

## Recent Governance

- **M5 Td S35:** tightened T279, T280, T282, and T283 task contracts.
- **M5 Td S36:** recorded the WASM media-backing admission boundary.
- **M5 Td S37:** made M5 history static, capped closure summaries, and
  hardened the mojibake governance check.
- **M5 Td S40:** inserted the then-approved T293--T303 core public-surface
  closure and its then-reserved Windows follow-on queue. The later Td S45
  record in [M5 History](etc/legacy-history/m5.md) supersedes those unstarted numeric
  allocations after T300 consumed the original transition slot.
- **M5 Td S41:** retired completed planning detail, made TODO open-only, removed
  tracked local-path samples, and added recurrence gates for those boundaries.
- **M5 Td S43:** superseded the unimplemented T291 S4 admission by merging its
  protected-return fault-atomicity scope into active T293 S1, which retains its
  original post-`#UD` transition-removal scope. T291 remains closed FDC history.
- **M5 Td S44:** made queue-order allocation, closed-subtask finality, and the
  no-active-task corrective exception explicit; added a contiguous-queue
  governance gate. The pre-policy S43/S42 discontinuity is archival only, so
  `S42` remains unavailable.
- **M5 Td S46:** reset the documentation topology to fixed rules/design/current
  authorities, an unnumbered Queue, numbered implementation history, and an
  owner-approved supporting-material index. Td work has no history record.

## Milestone State

| Milestone | State | Current authority |
| --- | --- | --- |
| M0--M4 | Closed | [Roadmap](design/ROADMAP.md) and Git |
| M5 | Open | [M5 closure checklist](etc/governance/m5-closure-checklist.md) |
| M6--M8 | Not started | [Roadmap](design/ROADMAP.md) |
| M9 onward | Research queue | [Roadmap](design/ROADMAP.md) |

## Operational Reading Order

1. [Documentation Guide](README.md)
2. [Status](STATUS.md) and [Queue](QUEUE.md)
3. [Roadmap](design/ROADMAP.md)
4. [Architecture](design/ARCHITECTURE.md) and [Contracts](design/CONTRACTS.md)
5. [Requirements](design/REQUIREMENTS.md)
