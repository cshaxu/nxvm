# Project Status

## Current Work

**M5 T299 S1 active - public raw-borrow closure.**

### Task Packet

- **Original request:** Delete public configuration borrows, profile-binding
  raw accessors, and debug borrows left without production callers after T298.
  Product headers must expose no raw CPU, RAM, port, controller, or executor
  pointer. Core retains all storage; production uses typed config/providers,
  checked interfaces, copied observations, and the T297/T298 capabilities.
- **S1 objective:** Freeze the final declaration/call/owner/lifetime map in
  `docs/architecture/core-machine-public-borrow-closure.md`, including every
  production and test consumer and its replacement. The approved test seam is
  a narrow `tests/support` fixture adapter: each fixture operation receives a
  `core_machine *`, has no global/TLS or mirror state, may include private core
  implementation headers, and is never included by `src/` or VM/VDM
  composition.
- **S2 objective:** Migrate each test to an existing checked/capability API or
  that fixture adapter, then delete all public configuration/debug raw-borrow
  declarations and definitions. Stop for any production consumer or requested
  new core capability.
- **S3 objective:** Add a source-shape gate covering every public header and
  `src/`; it rejects raw accessor/profile binding exports and any `src` include
  of `tests/support`. Run focused lifecycle/config/provider/capability corpus,
  full current gates, and create `nxvm_0_5_0299.exe` with SHA evidence.
- **Reference baseline:** T298 developer artifact, SHA-256
  `6A9AA9D2C3691F780426C6A78C2AE1C149BB9CC2A84AE835A9AEC10B5313254B`.
- **Non-goals:** Console/debugger/start/boot/DOS behavior, ROM/profile/media,
  VM outer loop, a new core capability, a second machine/executor/state
  mirror, global/TLS selection, source imports, or T300+ work.
- **Applicable rules:** `core/machine` owns storage and checked operations;
  composition supplies only typed configuration/providers; product code sees
  copied observation or named capability. Tests may use the approved external
  fixture seam only. Module-layout, contracts, coding, source, and execution
  policies apply; no exception is approved.
- **Similar-issue sweep:** the defect class is any public raw state accessor or
  profile-binding raw accessor. S1 command:
  `rg -n "core_machine_(configuration|debug)_.*borrow|profile.*(borrow|binding)" src tests --glob '*.[ch]'`.
  Every `src` hit must be removed; each test hit is mapped to a typed API or
  the approved fixture operation. The S3 static gate is the recurrence check.
- **Stop conditions:** a production raw consumer, a test need outside the
  approved fixture boundary, a new core capability, changed product behavior,
  raw export remaining in a public header, or any requirement to start T300+.

## Current Technical Baseline

- **T299 active artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0299` / `nxvm_0_5_0299.exe`.
  Active evidence: the focused raw-borrow corpus passed 27/27; the S3 static
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
| T291 | Made FDC generation, disk-change and motor/DMA cancellation per frozen drive slot, with core port and DOS FDD0 evidence while leaving timing, UI, and commands deferred. |
| T292 | Added prepared-state evidence that selected 67h REP comparison, scan, and segment-override forms already conform; no CPU behavior or artifact changed. |
| T293 | Removed the unused post-`#UD` transition surface, made protected outer `RETF`/`IRET` frame validation non-mutating until commit, and completed its exception/atomicity matrix: non-present returned `CS` is `#NP`, non-present `SS` is the retained terminal `#SS`, and invalid entries are `#GP`; all six `RETF`/outer-`IRET` cases preserve the defined pre-commit boundary. |
| T294 | Completed the codebase-driven public-surface/raw-borrow matrix, fixed T296's three stages and T297's future firmware-capability lifecycle precondition, and left T300/T302 conditional; no runtime behavior, CMake graph, or artifact changed. |
| T295 | Moved CPU execution-to-shared-PIC binding into `core_machine_create`; VM session no longer borrows or binds that path. CPU/PIC lifecycle evidence and all current gates passed; RTC/FDC/HDC/DMA remain T296 and debugger raw borrows remain T298/T299. |
| T296 | Completed A VADP/ports, B DMA/RTC/CMOS/NMI, and C FDC/HDC ownership migration through frozen typed core submissions. S2 `e84199e`, S3 `a02a0f0`, and S4 `fa18847d0aed685554f786c89ba0f5908e539fb7` passed focused owner/lifecycle evidence and the final 49-gate, 125-test verification; T298--T299 remained deferred at this closure. |
| T297 | Replaced default firmware raw profile binding with an opaque core-invoked capability for checked memory/port I/O and stop requests. Configure-time immutable ROM registration now rolls back atomically on callback failure; default BIOS/QDCGA, boot failure, 49 gates, and 126/126 CTests are closed without product UX change. |
| T298 | Replaced formal debugger raw CPU/instruction/execution/RAM/port borrows with paused-or-stopped named core operations and copied observations. The retained Console/debugger UX passed focused checks and the 126-test regression suite without a global target or second execution path. |

Detailed contracts, commands, artifact provenance, and prior closures are in
[M5 History](../history/m5.md) and Git history. The [M5 convergence queue]
(m5-pcat-hardware-convergence.md), [roadmap](roadmap.md), and
[M5 closure checklist](m5-closure-checklist.md) define current M5 scope; an
active packet becomes an operational authority only after approval.

## Recent Governance

- **M5 Td S33:** corrected the external-consumer boundary and shifted the
  former forward queue to T279--T287.
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
  original post-`#UD` transition-removal scope. T291 remains closed FDC history;
  the unstarted continuation remains T294--T308.

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
