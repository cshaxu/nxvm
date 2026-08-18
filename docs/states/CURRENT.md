# Project Status

## Current Work

## M5 T417 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner authorization of 2026-08-18 covers continued DeskPro L3 implementation, tier-labelled reference/generic bridges, and master pushes. |
| Objective | Implement one bounded generic-AT refresh locality policy: a D4-configured system PIT counter-1 refresh pulse invalidates the CPU external-memory locality key before a later CPU cycle, using the original D4 refresh topology and existing PIT output owner. |
| Non-goals | Do not claim D4 DMA/refresh/BWAIT clocks, exact DRAM page retention, a physical HOLD waveform, device service timing, or Model-L3 readiness. |
| Reference Baseline | T408 original D4 page-mode facts, T410 external cycles, T412--T416 locality bridges, original D3PE refresh/HLDA topology, and the existing Core PC/AT counter-1 refresh timer/output owner. |
| Candidate Proposal | [DeskPro physical-cycle and phase-timing closure](../proposals/m5-deskpro-physical-cycle-and-phase-timing.md) |
| Files And ABI Surface | Existing Core shared-PIT output/locality owner and D4 composition only; preserve Core/VM direction and one CPU/DMA transaction owner. |
| Applicable Rules | docs/design/ARCHITECTURE.md; docs/design/CODING.md; docs/rules/ARCHITECTURE.md; docs/rules/CODING.md; docs/rules/EXECUTION.md. |
| Verification | Original-source/topology review, focused D4 counter-1 pulse/locality miss/reset trace, retained DMA-HOLD competition regression, current gate, documentation gate and actual-diff review. |
| Expected Markers | M5:T417:S1:REFRESH-LOCALITY:OK or M5:T417:S1:REFRESH-LOCALITY:TRANSFER. |
| Asset Needs | O:\assets original D3PE research and read-only PCjs/other available references; no import. |
| Reporting Requirements | Label the policy generic-AT and the schematic topology original; name every unbound D4 physical receiver. |
| Stop Conditions | Stop and transfer any policy requiring exact DMA/refresh/BWAIT phase clocks, physical page retention proof, a second scheduler, or another transaction path. |
| Exit Criteria | A D4-configured refresh pulse invalidates CPU locality before the next CPU external cycle with focused reset/cancellation proof, or the receiver is transferred without blocking later work. |
| Original Owner Request | Implement DeskPro 386 L3 timing/hardware gaps using original then reference then generic-AT tiers; do not stall and preserve Core/VM boundary. |
| Similar-Issue Sweep | Inspect PIT counter-1 refresh output, CPU locality, reset/cancellation, DMA HOLD, port 61h, page walks, prefetch, data reads/writes, and Model-40 composition. |
## Current Technical Baseline

- **Current developer artifact:** T417 S1 P1 `vm-0-5-0417` /
  `build/output/nxvm_0_5_0417.exe`, SHA-256
  `3D509C6680FFEAEFD039744DCF959BC1BED89CEEDB94DA05B30A669DEC1AD4DE`.
  T417 binds only the D4-configured system PIT counter-1 refresh low pulse to
  the Core generic-AT locality invalidation boundary. Original D4 source proves
  refresh topology, not a physical page-retention interval or phase timing;
  T416 DMA-HOLD and T415 page-walk/CR0 PG/CR3 fixes remain retained.
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
| T416 | Closed: Core generic-AT locality invalidates at acknowledged DMA HOLD only; exact D4 page retention and phase work remain transferred. [Closure audit](../etc/evidence/t416-s2-dma-hold-locality-closure-audit.md). |
| T415 | Closed: Core generic-AT locality now covers committed page-table reads/writebacks, and CR0 PG/CR3 invalidate stale prefetch translation context; CPU/DMA page retention remains transferred. [Closure audit](../etc/evidence/t415-s2-page-walk-locality-closure-audit.md). |
| T414 | Closed: Core generic-AT external-memory locality now covers committed CPU data reads, while page walks and exact D4 physical phases remain transferred. [Closure audit](../etc/evidence/t414-s2-data-read-locality-closure-audit.md). |
| T413 | Closed: Core generic-AT external-memory locality now covers Model-40 prefetch reads and CPU data writes; exact D4 phase/arbitration remains transferred. [Closure audit](../etc/evidence/t413-s2-external-write-locality-closure-audit.md). |
| T412 | Closed: Model-40 now selects a Core generic-AT prefetch-locality bridge (2 KiB, miss +2/hit +0); the exact D4 row/bank PAL and physical timing remain transferred. [Closure audit](../etc/evidence/t412-s2-external-read-locality-closure-audit.md). |
| T411 | Closed: Core persistent instruction-prefetch window and task-level lifecycle audit establish the D4 physical-cycle prerequisite, while row/page waits, BWAIT and arbitration remain explicitly transferred. [Closure audit](../etc/evidence/t411-s2-prefetch-closure-audit.md). |
| T410 | Closed: Core now observes classified CPU external-cycle begin/commit/cancel around physical accesses, but it does not model prefetch overlap or publish D4 waits. [Evidence](../etc/evidence/t410-s1-cpu-external-cycle.md). |
| T409 | Closed: Core CPU memory transactions now label prefetch, instruction fetch, data, page-table read and page-table writeback, with full-gate proof; no external cycle or D4 page-hit timing is claimed. [Evidence](../etc/evidence/t409-s1-cpu-memory-provenance.md). |





## Recent Governance

- **M5 Td S105 P2:** restored a usable tiered DeskPro work path, recorded the five-receiver ledger and admitted next target T406 for a labelled generic-AT KBC serial-delivery mechanism; this is not a physical/L3 acceptance.

- **M5 Td S104 P1:** added the owner-authorized reference-derived DeskPro timing-bridge candidate ahead of CPU audit; it preserves physical-L3 not-ready and requires a fresh independent re-audit.

- **M5 Td S103 P1:** adopted task-scale convergence and coverage governance: coverage-bearing T work requires a durable universe, batch dispositions, completion predicate and coordinator reconciliation before whole-domain or L3 acceptance.
- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
