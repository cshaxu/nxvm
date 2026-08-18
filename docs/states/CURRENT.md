# Project Status

## Current Work

## M5 T415 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner authorization of 2026-08-18 covers continued DeskPro L3 implementation, tier-labelled reference/generic bridges, and master pushes. |
| Objective | Determine and implement a bounded page-table read/writeback locality policy on the Core external-cycle boundary, using original D4 page-mode values with an explicit generic-AT page-key fallback where needed. |
| Non-goals | Do not claim exact D4 row/bank mapping, paging microarchitecture overlap, BWAIT, DMA/refresh arbitration, device timing or Model-L3 readiness. |
| Reference Baseline | T408 original page-mode facts, T409 page-walk provenance, T410 external cycles, and T412--T414 CPU-memory locality bridges. |
| Candidate Proposal | [DeskPro physical-cycle and phase-timing closure](../proposals/m5-deskpro-physical-cycle-and-phase-timing.md) |
| Files And ABI Surface | Existing Core external-cycle/transaction/retirement owner and Model-40 composition only; preserve Core/VM direction and no second scheduler. |
| Applicable Rules | docs/design/ARCHITECTURE.md; docs/design/CODING.md; docs/rules/ARCHITECTURE.md; docs/rules/CODING.md; docs/rules/EXECUTION.md. |
| Verification | Original-source review, focused paging read/writeback/hit/cancel/reset trace, current gate, documentation gate and actual-diff review. |
| Expected Markers | M5:T415:S1:PAGE-WALK-LOCALITY:OK or M5:T415:S1:PAGE-WALK-LOCALITY:TRANSFER. |
| Asset Needs | O:\assets original D4 research and read-only PCjs/other available references; no import. |
| Reporting Requirements | Label mechanisms original, reference-derived or generic-AT; name every unbound D4 physical receiver. |
| Stop Conditions | Stop and transfer any phase that would fabricate paging overlap or require a second CPU/DMA transaction or scheduler path. |
| Exit Criteria | Committed page-table reads/writebacks participate in a bounded locality policy with focused proof, or the missing phase fact is transferred without blocking later receivers. |
| Original Owner Request | Implement DeskPro 386 L3 timing/hardware gaps using original then reference then generic-AT tiers; do not stall and preserve Core/VM boundary. |
| Similar-Issue Sweep | Inspect page-table reads/writebacks, data reads/writes, prefetch, TLB/paging faults, reset/cancellation, HOLD/HLDA, DMA, ROM/RAM maps and Model-40 composition. |
## Current Technical Baseline

- **Current developer artifact:** T415 S1 P1 `vm-0-5-0415` /
  `build/output/nxvm_0_5_0415.exe`, SHA-256
  `F57D7A70EBD78307786C4B0C98BB0D3EEA715070DFCDA025284C3439D94BE5B8`.
  T415 extends the Model-40 generic locality bridge to committed page-table
  reads and writebacks. It also repairs persistent-prefetch invalidation across
  CR0 PE/PG and CR3 translation-context changes. Exact D4 PAL phases remain
  transferred; T409 classification itself does not publish original D4 timing
  or a Model-L3 claim.
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
| T414 | Closed: Core generic-AT external-memory locality now covers committed CPU data reads, while page walks and exact D4 physical phases remain transferred. [Closure audit](../etc/evidence/t414-s2-data-read-locality-closure-audit.md). |
| T413 | Closed: Core generic-AT external-memory locality now covers Model-40 prefetch reads and CPU data writes; exact D4 phase/arbitration remains transferred. [Closure audit](../etc/evidence/t413-s2-external-write-locality-closure-audit.md). |
| T412 | Closed: Model-40 now selects a Core generic-AT prefetch-locality bridge (2 KiB, miss +2/hit +0); the exact D4 row/bank PAL and physical timing remain transferred. [Closure audit](../etc/evidence/t412-s2-external-read-locality-closure-audit.md). |
| T411 | Closed: Core persistent instruction-prefetch window and task-level lifecycle audit establish the D4 physical-cycle prerequisite, while row/page waits, BWAIT and arbitration remain explicitly transferred. [Closure audit](../etc/evidence/t411-s2-prefetch-closure-audit.md). |
| T410 | Closed: Core now observes classified CPU external-cycle begin/commit/cancel around physical accesses, but it does not model prefetch overlap or publish D4 waits. [Evidence](../etc/evidence/t410-s1-cpu-external-cycle.md). |`n| T409 | Closed: Core CPU memory transactions now label prefetch, instruction fetch, data, page-table read and page-table writeback, with full-gate proof; no external cycle or D4 page-hit timing is claimed. [Evidence](../etc/evidence/t409-s1-cpu-memory-provenance.md). |
| T408 | Closed: original D4 material proves row-miss two-wait/row-hit zero-wait CPU memory behavior, but current logical accesses cannot safely receive it; external-cycle/prefetch-overlap prerequisite remains transferred. [Evidence](../etc/evidence/t408-s1-original-d4-memory-timing-admission.md). |
| T407 | Closed: existing Core HDC command/sector pending phases publish DRQ/IRQ14 only through the next readiness tick; accepted as generic-AT virtual-time behavior, with Compaq physical/L3 work retained. [Evidence](../etc/evidence/t407-s1-hdc-phase-acceptance.md). |



## Recent Governance

- **M5 Td S105 P2:** restored a usable tiered DeskPro work path, recorded the five-receiver ledger and admitted next target T406 for a labelled generic-AT KBC serial-delivery mechanism; this is not a physical/L3 acceptance.

- **M5 Td S104 P1:** added the owner-authorized reference-derived DeskPro timing-bridge candidate ahead of CPU audit; it preserves physical-L3 not-ready and requires a fresh independent re-audit.

- **M5 Td S103 P1:** adopted task-scale convergence and coverage governance: coverage-bearing T work requires a durable universe, batch dispositions, completion predicate and coordinator reconciliation before whole-domain or L3 acceptance.
- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
