# Project Status

## Current Work

**M5 T300 S2: Typed port-provider failure propagation -- active.**

| Requirement | Acceptance evidence |
| --- | --- |
| Public bus preserves a typed provider failure. | Read/write return the exact provider `type_status`; failed reads leave output and trace behavior explicit and no state is fabricated. |
| Guest I/O fails deterministically on that same binding. | CPU `IN`/`OUT` maps a bound typed-provider failure to the existing core fault diagnostic without adding an exception class or a second I/O path. |
| Recovery and product paths remain intact. | A failure followed by a successful provider call is observable; retained Console, DOS, boot, controller and display regressions pass. |

- **Original request:** S2 preserves typed provider failures which S1's legacy
  callback adapter deliberately retained but did not surface.
- **Scope:** the existing single core port dispatch, public bus result/output/
  trace behavior, guest CPU I/O diagnostic mapping, and focused recovery probes.
- **Non-goals:** VM session failure atomicity (S3), public-header ABI cleanup
  (S4), a pre-decode transition
  registry, a generic device framework, a second I/O path, a fake DOS
  controller, or any Console/debugger/boot/DOS UX change.
- **Risk:** an error can be swallowed, fabricate a read value, or make CPU and
  public bus diverge. The mapping must use the S1 registry entry only and a
  retained diagnostic result, then prove the following successful access works.
- **Similar-issue sweep:** inventory all typed provider calls, public bus
  trace recording, CPU I/O helpers and fault diagnostics; do not change legacy
  core-device callbacks or add an exception model.
- **Rules:** architecture overview, module layout, contracts, coding standard,
  source policy, execution workflow, and execution policy apply. The T300
  admission is owner-approved. No exception is requested.
- **Verification:** run focused provider-failure/recovery plus retained CPU I/O,
  Console/DOS/boot probes, `git diff --check`, documentation/static gates and
  full current gates. T300 stays active; no S2 artifact identity is final.
- **Current S2 evidence:** `core-machine-port-ownership-smoke` proves public
  failure/recovery and guest `OUT` to the retained `#CE(00E0)` diagnostic;
  `core-machine-trace-smoke` proves failure detail and unchanged caller output.
  Unbound guest directions retain legacy no-op behavior. `current-gates-gcc`
  passed 51/51 static/governance targets and 127/127 CTests on T299 baseline.
- **S1 carry-forward:** c42ca2f and 1f4b100 froze the single directional
  registry and its conflict behavior; 51/51 gates and 127/127 CTests passed.
- **Stop condition:** public bus returns exact typed failures without fabricated
  read output or success trace, guest I/O emits a deterministic existing fault
  diagnostic, the next successful access works, and all use one dispatch path.

## Current Technical Baseline

- **T299 artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0299` / `nxvm_0_5_0299.exe`.
  Closure evidence: the focused raw-borrow corpus passed 27/27; the S3 static
  gate emitted `M5:T299:S3:PUBLIC-RAW-BORROW-CLOSURE:OK`; and the owner-provided
  cache-media `current-gates-gcc` run passed all 51 static/governance targets
  and 126/126 CTests. The local developer artifact is 2,696,377 bytes with
  SHA-256 `4736BA1A74F9C1C3268BC7D13D9F5D53FF823718F4F1EC3975766188DAA76438`.
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
| T292 | Added prepared-state evidence that selected 67h REP comparison, scan, and segment-override forms already conform; no CPU behavior or artifact changed. |
| T293 | Removed the unused post-`#UD` transition surface, made protected outer `RETF`/`IRET` frame validation non-mutating until commit, and completed its exception/atomicity matrix: non-present returned `CS` is `#NP`, non-present `SS` is the retained terminal `#SS`, and invalid entries are `#GP`; all six `RETF`/outer-`IRET` cases preserve the defined pre-commit boundary. |
| T294 | Completed the codebase-driven public-surface/raw-borrow matrix, fixed T296's three stages and T297's future firmware-capability lifecycle precondition, and left T300/T302 conditional; no runtime behavior, CMake graph, or artifact changed. |
| T295 | Moved CPU execution-to-shared-PIC binding into `core_machine_create`; VM session no longer borrows or binds that path. CPU/PIC lifecycle evidence and all current gates passed; RTC/FDC/HDC/DMA remain T296 and debugger raw borrows remain T298/T299. |
| T296 | Completed A VADP/ports, B DMA/RTC/CMOS/NMI, and C FDC/HDC ownership migration through frozen typed core submissions. S2 `e84199e`, S3 `a02a0f0`, and S4 `fa18847d0aed685554f786c89ba0f5908e539fb7` passed focused owner/lifecycle evidence and the final 49-gate, 125-test verification; T298--T299 remained deferred at this closure. |
| T297 | Replaced default firmware raw profile binding with an opaque core-invoked capability for checked memory/port I/O and stop requests. Configure-time immutable ROM registration now rolls back atomically on callback failure; default BIOS/QDCGA, boot failure, 49 gates, and 126/126 CTests are closed without product UX change. |
| T298 | Replaced formal debugger raw CPU/instruction/execution/RAM/port borrows with paused-or-stopped named core operations and copied observations. The retained Console/debugger UX passed focused checks and the 126-test regression suite without a global target or second execution path. |
| T299 | Removed the remaining public configuration/debug raw borrows and profile raw binding. Production uses typed/copying interfaces; purpose-named test fixtures alone may include private core state. The recurrence gate, 51 current static/governance targets, and 126 CTests passed without product behavior changes. |

Detailed contracts, commands, artifact provenance, and prior closures are in
[M5 History](../history/m5.md) and Git history. The [M5 convergence queue]
(m5-pcat-hardware-convergence.md), [roadmap](roadmap.md), and
[M5 closure checklist](m5-closure-checklist.md) define current M5 scope; an
active packet becomes an operational authority only after approval.

## Recent Governance

- **M5 Td S34:** repaired queue wording and inserted the approved T279--T283
  reliability/interface package.
- **M5 Td S35:** tightened T279, T280, T282, and T283 task contracts.
- **M5 Td S36:** recorded the WASM media-backing admission boundary.
- **M5 Td S37:** made M5 history static, capped closure summaries, and
  hardened the mojibake governance check.
- **M5 Td S40:** inserted the approved T293--T303 core public-surface closure
  and shifted remaining Windows CPU/FPU/readiness/closure work to T304--T308.
- **M5 Td S41:** retired completed planning detail, made TODO open-only, removed
  tracked local-path samples, and added recurrence gates for those boundaries.
- **M5 Td S43:** superseded the unimplemented T291 S4 admission by merging its
  protected-return fault-atomicity scope into active T293 S1, which retains its
  original post-`#UD` transition-removal scope. T291 remains closed FDC history.
- **M5 Td S44:** made queue-order allocation, closed-subtask finality, and the
  no-active-task corrective exception explicit; added a contiguous-queue
  governance gate. The pre-policy S43/S42 discontinuity is archival only, so
  `S42` remains unavailable and the next governance allocation follows S44.

## Milestone State

| Milestone | State | Current authority |
| --- | --- | --- |
| M0--M4 | Closed | [Roadmap](roadmap.md) and [history](../history/README.md) |
| M5 | Open | [M5 closure checklist](m5-closure-checklist.md) |
| M6--M8 | Not started | [Roadmap](roadmap.md) |
| M9 onward | Research queue | [Roadmap](roadmap.md) |

## Operational Reading Order

1. [Documentation Guide](../README.md)
2. [Project Status](status.md) and [M5 convergence queue](m5-pcat-hardware-convergence.md)
3. [M5 closure checklist](m5-closure-checklist.md)
4. [Architecture overview](../architecture/overview.md) and
   [contracts](../architecture/contracts.md)
5. [Requirements](../requirements/)
