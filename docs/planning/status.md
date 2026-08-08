# Project Status

## Current Work

**M5 T300 S1: Core port ownership and conflict contract -- active.**

| Requirement | Acceptance evidence |
| --- | --- |
| One core-owned port registration and dispatch truth source. | PIC, PIT, KBC, DMA, VADP, FDC, HDC, memory-port, and injected providers are composed through one frozen core registry; CPU guest I/O and public `core_machine_bus_read/write` reach the same registered handler. |
| Reject duplicate directional bindings without mutation. | A focused core probe proves a second `(port, direction)` registration returns a defined failure and leaves the first provider active. It exercises PIC, PIT, KBC, and FDC ownership conflicts. |
| Preserve machine lifecycle and product paths. | The registry's configure/freeze/reset semantics are explicit; retained boot, Console/debugger, and normal core-device I/O regressions pass. VDM minimal cannot bind retained PC/AT controller ports under the new contract. |

- **Original request:** admit the prior conditional T300 to repair confirmed
  core/VM/VDM boundary defects. S1 removes the two observable port-handler
  truth sources and freezes ownership, conflict, lifecycle, and CPU/public-bus
  dispatch semantics.
- **Scope:** core port registration/dispatch and its narrow public contract,
  the affected core/VM/VDM composition wiring, focused conflict and same-path
  probes, and compact S1 tracking evidence.
- **Non-goals:** provider failure-result propagation (S2), VM session failure
  atomicity (S3), public-header ABI cleanup (S4), a pre-decode transition
  registry, a generic device framework, a second I/O path, a fake DOS
  controller, or any Console/debugger/boot/DOS UX change.
- **Risk:** merging two I/O paths can silently alter device ordering or leave
  a bypass. Every retained device keeps its existing entity and C-style
  callback; configuration rejects duplicate direction bindings before state
  mutation, and CPU/public bus probes use the same live registration.
- **Similar-issue sweep:** inventory all production port registration and
  dispatch sites in `src/core`, `src/vm`, and `src/vdm`, plus their CMake and
  focused tests. Classify each as migrated, single-route retained, or deferred
  to S2--S4; do not expand into provider status propagation.
- **Rules:** architecture overview, module layout, contracts, coding standard,
  source policy, execution workflow, and execution policy apply. The T300
  admission is owner-approved. No exception is requested.
- **Verification:** run new focused core/VM/VDM probes, retained device and
  CPU I/O regressions, `git diff --check`, and applicable documentation/static
  gates. An S1 build does not advance the current artifact identity; T300
  remains active and no artifact is final until the task closes.
- **Current S1 evidence:** `core-machine-port-ownership-smoke` proves the
  retained owner after duplicate, PIC/PIT/KBC/FDC conflict rejection, and
  public-bus plus guest-`IN`/`OUT` dispatch. It also proves FDC `3F2h` accepts
  a prebound read provider while retaining FDC's write owner, and rejects a
  prebound write provider without replacement. `current-gates-gcc` passed all
  51 static/governance targets and 127/127 CTests on the T299 artifact baseline.
- **Stop condition:** one registry/dispatch path owns every configured port;
  duplicate directional registration is non-mutating and observable; CPU and
  public bus reach that same owner; no VDM minimal PC/AT overwrite remains;
  S2 failure propagation remains explicitly deferred.

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
