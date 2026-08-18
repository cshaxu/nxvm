# Project Status

## Current Work

## M5 T419 S5 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | The user has standing approval in this thread to continue tier-labelled DeskPro L3 work without repeated authorization. Scope is the original-source D4 CPU external-memory-cycle and prefetch-overlap receiver. |
| Objective | Replace the retained logical-access locality assumption with a Core-owned observable CPU external-memory-cycle/prefetch-overlap boundary that applies the documented 2 KiB miss/hit outcome only to a proven overlapping sequential request. |
| Non-goals | No calibrated BCLK duration, PAL translation, general cache model, VM scheduler, firmware import, or claimed whole-machine L3 acceptance. |
| Reference Baseline | master at f6912668; original D4 material fixes the 2 KiB page and initial/row-miss versus row-hit wait distinction. Current TODO records that a logical-access scalar cannot publish this result. |
| Candidate Proposal | [DeskPro physical-cycle and phase-timing closure](../proposals/m5-deskpro-physical-cycle-and-phase-timing.md), receiver 1; evidence tier original. |
| Files And ABI Surface | Core machine transaction/timing private state and trace surface as necessary; Model-40 selects only the difference; focused Core and Model-40 tests; Current and indexed evidence. |
| Applicable Rules | Core remains the sole CPU/DMA transaction owner; VM selects no alternate transaction path; C11/type vocabulary; original source is design evidence only; task, evidence, artifact and gate rules apply. |
| Verification | A focused trace distinguishes speculative prefetch, consumed fetch, page walk and data access; proves hit eligibility requires declared overlap identity and sequence; proves reset/cancel/HOLD lifecycle; retains non-D4 behavior; runs adjacent and full gates. |
| Expected Markers | A new T419 S5 marker plus retained S1-S4 markers. |
| Asset Needs | Read-only approved primary research corpus only; no firmware, media, reference source text or local path committed. |
| Reporting Requirements | Record source tier, request identity and overlap contract, exact regressions, artifact hash, remaining physical transfers and pushed commit. |
| Stop Conditions | Stop and transfer calibrated duration, any unproven page-hit predicate, CPU BWAIT, external bus-master behavior, or a second Core/VM transaction route. |
| Exit Criteria | The Core-owned external-cycle/prefetch-overlap contract is selected only for D4, has focused lifecycle proof and full gate evidence, and transfers remaining physical timing. |
| Original Owner Request | Implement DeskPro 386 L3 timing and hardware gaps with original material first, reference-derived behavior second, then explicitly labelled generic AT scaffolding. |
| Similar-Issue Sweep | Inspect every CPU external memory transaction, existing locality timing, prefetch path, page-walk path, cancellation, HOLD and Model-40 configuration; repair the shared owner only. |

## Current Technical Baseline

- **Current developer artifact:** T419 S4 P2 `vm-0-5-0419` /`build/output/nxvm_0_5_0419.exe`, SHA-256 `B25078C401D466C54BC08445BB3E7A4846A0491B58D17052332D143D655DC5D6`. T419 runs the existing sole Core DMA transaction route under D4 composition, with original-source one-quantum DMA grant delay BUSRDY gate, and one discrete refresh hold before pending DMA; calibrated waveform duration, CPU BWAIT, and electrical refresh remain transferred. T418 instruction-boundary locality, T417 refresh, T416 DMA-HOLD and T415 paging fixes remain retained.
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
