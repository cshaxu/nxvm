# Project Status

## Current Work

## M5 T411 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner authorization of 2026-08-18 covers continued DeskPro L3 work and master pushes. |
| Objective | Independently close the bounded CPU-prefetch prerequisite and transfer the remaining D4 physical-cycle receivers. |
| Non-goals | Do not publish a DeskPro D4 wait scalar, page-hit decision, BWAIT model, or Model-L3 result. |
| Reference Baseline | T411 S1 P1 a9be57f8 and P2 c0032e3c; original D4 timing admission T408. |
| Candidate Proposal | [DeskPro physical-cycle and phase-timing closure](../proposals/m5-deskpro-physical-cycle-and-phase-timing.md) |
| Files And ABI Surface | Evidence, history and current-status records only. |
| Applicable Rules | docs/rules/EXECUTION.md; docs/rules/DOCUMENT.md; docs/rules/ARCHITECTURE.md. |
| Verification | Actual P1/P2 diff review, lifecycle-entry sweep, documentation governance and clean worktree. |
| Expected Markers | M5:T411:S2:PREFETCH-CLOSURE:OK |
| Asset Needs | T408 original D3PE evidence read-only; no import. |
| Reporting Requirements | Name exact completed prerequisite and each transferred D4 physical receiver. |
| Stop Conditions | Stop if closure would imply physical timing from generic/reference behavior. |
| Exit Criteria | Every T411 objective is evidenced or transferred to the existing physical-cycle proposal, and the task is task-level closed. |
| Original Owner Request | Continue DeskPro L3/hardware work with original, reference-derived and generic-AT tiers labelled; do not stall. |
| Similar-Issue Sweep | Review execution init, code reads, sequential advance, reset, interrupt/exception, preview, paging and HOLD/HLDA paths. |

## Current Technical Baseline

- **Current developer artifact:** T411 S1 P1 `vm-0-5-0411` /
  `build/output/nxvm_0_5_0411.exe`, SHA-256
  `E4A0786E8C80CFCCFBB37EE7BB14B502DDBA5A02C825041B8FA89A3074DFC366`.
  The isolated x64 current gate passes 287/287. T409 labels Core CPU memory
  provenance only; it does not publish original D4 timing or a Model-L3 claim.
  T386 closes selected-device functional completeness at S29; its retained
  [closure audit](../etc/evidence/t386-s29-functional-closure-audit.md) fixes
  HDC current-gate coverage and transfers board, firmware and physical work.
- **Model-339 readiness:** T383 S1 accepts the current runnable source graph
  under the frozen deterministic event-and-bus L3 contract. This is only the
  selected Model 339 configuration and preserves every documented physical,
  fixed-disk, generic-PC/AT and later-machine boundary.
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
| T411 S1 | Closed: Core persistent instruction-prefetch window now separates refill from consumed bytes, invalidates on non-sequential CS:EIP and reset, and retains D4 wait/page-hit timing as unbound. [Evidence](../etc/evidence/t411-s1-cpu-prefetch-window.md). |
| T410 | Closed: Core now observes classified CPU external-cycle begin/commit/cancel around physical accesses, but it does not model prefetch overlap or publish D4 waits. [Evidence](../etc/evidence/t410-s1-cpu-external-cycle.md). |`n| T409 | Closed: Core CPU memory transactions now label prefetch, instruction fetch, data, page-table read and page-table writeback, with full-gate proof; no external cycle or D4 page-hit timing is claimed. [Evidence](../etc/evidence/t409-s1-cpu-memory-provenance.md). |
| T408 | Closed: original D4 material proves row-miss two-wait/row-hit zero-wait CPU memory behavior, but current logical accesses cannot safely receive it; external-cycle/prefetch-overlap prerequisite remains transferred. [Evidence](../etc/evidence/t408-s1-original-d4-memory-timing-admission.md). |
| T407 | Closed: existing Core HDC command/sector pending phases publish DRQ/IRQ14 only through the next readiness tick; accepted as generic-AT virtual-time behavior, with Compaq physical/L3 work retained. [Evidence](../etc/evidence/t407-s1-hdc-phase-acceptance.md). |
| T406 | Closed: generic-AT KBC native-byte cadence is Core-owned and Model-40-bound; 287/287 gates pass, while physical keyboard/8042/board timing and L3 remain transferred. [Evidence](../etc/evidence/t406-s1-kbc-serial-cadence.md). |
| T405 | Closed: x86 GCC host build accepts the current product after pointer-width and validated-media-offset repairs; x86 and x64 current gates both pass 286/286, with no L3 claim. [Evidence](../etc/evidence/t405-s1-x86-gcc-host-build-compatibility.md). |
| T404 | Closed: every frozen public profile/catalog/media/controller/display/input route has direct functional proof or a named physical receiver; no timing or L3 claim. [Closure audit](../etc/evidence/t404-s6-batch-b-functional-reconciliation.md). |
## Recent Governance

- **M5 Td S105 P2:** restored a usable tiered DeskPro work path, recorded the five-receiver ledger and admitted next target T406 for a labelled generic-AT KBC serial-delivery mechanism; this is not a physical/L3 acceptance.

- **M5 Td S104 P1:** added the owner-authorized reference-derived DeskPro timing-bridge candidate ahead of CPU audit; it preserves physical-L3 not-ready and requires a fresh independent re-audit.

- **M5 Td S103 P1:** adopted task-scale convergence and coverage governance: coverage-bearing T work requires a durable universe, batch dispositions, completion predicate and coordinator reconciliation before whole-domain or L3 acceptance.
- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
