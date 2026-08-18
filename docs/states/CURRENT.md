# Project Status

## Current Work

## M5 T419 S6 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | The user has standing approval in this thread to continue tier-labelled DeskPro L3 work without repeated authorization. Scope is the original-source D4 CPU-pipeline producer for the existing Core external-cycle overlap contract. |
| Objective | Make the actual Core instruction-prefetch path publish a sequential external-cycle overlap declaration only when its prefetch lifecycle retains the predecessor request, so the Model-40 can reach the already-bound D4 2 KiB page-hit result through a real CPU route. |
| Non-goals | No calibrated BCLK duration, full PAL simulator, fabricated row/bank interval, general cache model, VM scheduler, firmware import, or whole-machine L3 claim. |
| Reference Baseline | `master` at `966b7ae8`; approved original 1986 DeskPro 386/16 technical material D4-RCTLA names CPU PIPE TO MEMORY hit/miss, idle/miss recovery, and distinct DMA/refresh states. Current code defines `OVERLAP_DECLARE`, but the production CPU path never emits it. |
| Candidate Proposal | [DeskPro physical-cycle and phase-timing closure](../proposals/m5-deskpro-physical-cycle-and-phase-timing.md), receiver 1 continuation; evidence tier `original`. |
| Files And ABI Surface | Core CPU prefetch lifecycle and private external-cycle state/trace as necessary; Model-40 remains a profile selector; focused Core and Model-40 tests; Current and indexed evidence. Public changes, if required, are append-only. |
| Applicable Rules | Core remains the sole CPU/DMA transaction and prefetch owner; VM selects no alternate route; C11/type vocabulary; original source is design evidence only; source, task, evidence, artifact and gate rules apply. |
| Verification | A focused real-CPU flow proves a sequential queued prefetch emits a declaration and can consume the matching Model-40 hit; data/page-walk/nonsequential paths do not declare; cancellation, reset, HOLD, refresh and stopped-state write invalidate it; non-D4 profiles retain disabled policy. Run focused, adjacent and full gates. |
| Expected Markers | A new `M5:T419:S6` marker plus retained S1-S5 markers. |
| Asset Needs | Read-only approved original research corpus only; no firmware, media, reference source text or local path committed. |
| Reporting Requirements | Record source tier, exact producer predicate, production-vs-test declaration evidence, regressions, artifact hash, remaining PAL/phase transfers and pushed commit. |
| Stop Conditions | Stop and transfer calibrated duration, an unproven predicate beyond the source's discrete hit/miss/idle facts, CPU BWAIT, external bus-master behavior, or a second Core/VM transaction route. |
| Exit Criteria | The production Core prefetch lifecycle, not a test-only callback, can publish the exact declared sequential overlap required by the existing Model-40 policy; focused and full gates pass, and all remaining physical phase work transfers. |
| Original Owner Request | Implement DeskPro 386 L3 timing and hardware gaps with original material first, reference-derived behavior second, then explicitly labelled generic AT scaffolding. |
| Similar-Issue Sweep | Inspect all external-cycle publications, CPU prefetch fills/consumption/invalidation, data and page-walk paths, transaction cancellation, HOLD, D4 refresh, reset, stopped-state writes, and Model-40/5170 selection; repair the shared Core owner only. |
## Current Technical Baseline

- **Current developer artifact:** T419 S5 P2 `vm-0-5-0419` /`build/output/nxvm_0_5_0419.exe`, SHA-256 `5820F4B1A7D7B92548C0AA144C717276D8E3F8A14E29ABBF75BF5FAC92BB5BDB`. T419 keeps the single Core DMA transaction route and adds the D4-selected explicit external CPU-cycle/prefetch-overlap policy; completed adjacency cannot manufacture a hit, and stopped-state physical writes invalidate queued prefetch. Exact asynchronous producer, calibrated waveform, CPU BWAIT, row/bank PAL, and electrical refresh remain transferred. T418 instruction-boundary locality, T417 refresh, T416 DMA-HOLD and T415 paging fixes remain retained.
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
| T419 S1 | Closed receiver: actual D4 composition qualifies the Core DMA no-extra-wait transaction route; physical DMA phases remain transferred. [Closure audit](../etc/evidence/t419-s2-d4-dma-no-wait-closure-audit.md). |
| T418 | Closed: generic-AT locality ends at every instruction boundary; physical D4 overlap timing remains transferred. [Closure audit](../etc/evidence/t418-s2-instruction-boundary-locality-closure-audit.md). |
| T417 | Closed: D4 counter-1 refresh low pulses invalidate generic-AT CPU locality; physical refresh timing and arbitration remain transferred. [Closure audit](../etc/evidence/t417-s2-refresh-locality-closure-audit.md). |
| T416 | Closed: Core generic-AT locality invalidates at acknowledged DMA HOLD only; exact D4 page retention and phase work remain transferred. [Closure audit](../etc/evidence/t416-s2-dma-hold-locality-closure-audit.md). |
| T415 | Closed: Core generic-AT locality now covers committed page-table reads/writebacks, and CR0 PG/CR3 invalidate stale prefetch translation context; CPU/DMA page retention remains transferred. [Closure audit](../etc/evidence/t415-s2-page-walk-locality-closure-audit.md). |
| T414 | Closed: Core generic-AT external-memory locality now covers committed CPU data reads, while page walks and exact D4 physical phases remain transferred. [Closure audit](../etc/evidence/t414-s2-data-read-locality-closure-audit.md). |
| T413 | Closed: Core generic-AT external-memory locality now covers Model-40 prefetch reads and CPU data writes; exact D4 phase/arbitration remains transferred. [Closure audit](../etc/evidence/t413-s2-external-write-locality-closure-audit.md). |
| T412 | Closed: Model-40 now selects a Core generic-AT prefetch-locality bridge (2 KiB, miss +2/hit +0); the exact D4 row/bank PAL and physical timing remain transferred. [Closure audit](../etc/evidence/t412-s2-external-read-locality-closure-audit.md). |








## Recent Governance

- **M5 Td S105 P2:** restored a usable tiered DeskPro work path, recorded the five-receiver ledger and admitted next target T406 for a labelled generic-AT KBC serial-delivery mechanism; this is not a physical/L3 acceptance.

- **M5 Td S104 P1:** added the owner-authorized reference-derived DeskPro timing-bridge candidate ahead of CPU audit; it preserves physical-L3 not-ready and requires a fresh independent re-audit.

- **M5 Td S103 P1:** adopted task-scale convergence and coverage governance: coverage-bearing T work requires a durable universe, batch dispositions, completion predicate and coordinator reconciliation before whole-domain or L3 acceptance.
- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
