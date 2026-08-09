# Project Status

## Current Work

**M5 T300 S4: Public interface boundary and recurrence gate -- active.**

| Requirement | Acceptance evidence |
| --- | --- |
| Public interfaces carry no private implementation layout. | `machine_interface.h` uses only copyable display/controller configuration contracts; public headers do not include private core-machine headers. |
| Debugger observation is explicit and copied. | The record names retained debugger CPU/instruction/memory fields without `t_cpu`, `t_cpuins`, executor, RAM, or port layout. |
| The boundary cannot silently regress. | The strengthened raw-borrow gate rejects private public-header includes/layouts, raw borrows, profile bindings, `core_token` pointer conversion, and `src` use of `tests/support`. |

- **Original request:** eliminate private device/CPU layouts from public core
  interfaces, retain only necessary narrow copied configuration, and make the
  debugger instruction observation an explicit copied contract.
- **Scope:** core public interface headers, retained VM debugger consumer,
  focused debugger evidence, and the T299 recurrence gate.
- **Non-goals:** any new device framework, CPU/executor mutation contract,
  pre-decode registry, T301 work, host shortcut, second video owner, or
  Console/debugger/boot/DOS UX change.
- **Frozen S4 contract:** `controller_interface.h` is the sole shared narrow
  declaration for a core-issued opaque DMA binding nonce, FDC drive/config,
  and HDC config values. The nonce is validated only against the receiving
  machine's private DMA state; it is neither an address nor a dereferenceable
  capability. FDC retains only core-installed request callbacks, which
  revalidate against their owning machine before DRQ changes.
  `display_interface.h` owns copied display capability
  configuration. Public machine/debug interfaces expose no private device header or complete
  CPU/instruction layout. The debugger record is copied and limited to fields
  used by breakpoints, recorder/disassembly, and memory-access output.
- **Similar-issue sweep:** `rg -n -g '*_interface.h'` scanned private
  core-machine includes and raw layout names across `src`; there are no
  production hits. Two core tests retain direct private includes only for
  same-module test constants (`cpu.h` entry-plan flags and `vadp.h` display
  topology); neither is a public or product route.
- **Active P2 evidence:** `core-machine-dma-binding-token-smoke` creates two
  core machines, proves their nonces differ, rejects each foreign binding, and
  freezes/resets each matching binding. `core-machine-dma-channel-smoke`
  retains same-machine DRQ transfer coverage.
- **Rules:** architecture overview, module layout, contracts, coding standard,
  source policy, execution workflow, and execution policy apply. The T300
  admission is owner-approved. No exception is requested.
- **Verification:** focused core debugger observation and public-interface
  gate; retained Console/DOS/FDD/HDD boot probes; `git diff --check`,
  documentation governance, and full `current-gates-gcc`. The active S4
  artifact is `nxvm_0_5_0300.exe` (2,588,088 bytes, SHA-256
  `B02BEF7B349BAD880086CDAB6485A64702C11E302060519D57E4743AD232023C`).
- **Carry-forward:** S1's directional ownership, S2's exact provider failure
  propagation, and S3's session failure atomicity remain unchanged.
- **Stop condition:** every public header passes the source-shape gate, the
  copied debugger observation retains its consumer behavior, and all retained
  product regressions and the 0.5.0300 artifact evidence pass. T300 remains
  active pending coordinator final review.

## Current Technical Baseline

- **T300 artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0300` / `nxvm_0_5_0300.exe`.
  S4 active evidence: the strengthened source-shape gate emits
  `M5:T300:S4:PUBLIC-INTERFACE-BOUNDARY:OK`; the focused debugger snapshot
  probe and the active artifact build passed. Final closure remains coordinator
  owned.
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
