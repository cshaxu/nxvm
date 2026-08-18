# Project Status

## Current Work

## M5 T419 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner authorization of 2026-08-18 covers continued DeskPro L3 implementation, original/reference/generic tier-labelled bridges, and master pushes. |
| Objective | Add a Core-owned configurable one-DMA-scheduling-quantum grant delay, selected only by DeskPro Model-40 composition, to represent the original D4 statement that each DMA cycle receives one DCLK-length wait. |
| Non-goals | Do not claim a Core elapsed tick equals 250 ns, model HAK/HAKDMA pins, BUSRDY stretching, controller cascade waveforms, refresh priority, BWAIT or Model-L3 readiness. |
| Reference Baseline | Original D3PE DMA byte-cycle description: one DCLK after HAK produces HAKDMA and a single DCLK-length wait is added to all DMA cycles; DCLK is 4 MHz/250 ns. |
| Candidate Proposal | [DeskPro physical-cycle and phase-timing closure](../proposals/m5-deskpro-physical-cycle-and-phase-timing.md) receiver 2. |
| Files And ABI Surface | Core machine configuration and existing arbitration owner; Model-40 private composition selector; focused Core/Model-40 regressions; preserve Core/VM direction and one CPU/DMA transaction path. |
| Applicable Rules | docs/design/ARCHITECTURE.md; docs/design/CODING.md; docs/rules/ARCHITECTURE.md; docs/rules/CODING.md; docs/rules/DOCUMENT.md; docs/rules/EXECUTION.md. |
| Verification | Original-source review; focused generic zero-delay versus selected one-quantum Model-40 DMA transfer proof; CPU/DMA ordering/reset/cancellation regression; current gate; documentation gate and actual-diff review. |
| Expected Markers | M5:T419:S2:D4-DMA-GRANT-WAIT:OK or M5:T419:S2:D4-DMA-GRANT-WAIT:TRANSFER. |
| Asset Needs | O:/assets/research/compaq_deskpro_386_16/technical_spec_1986_textmode.txt original research; no import. |
| Reporting Requirements | Label it an original-source-backed discrete bridge, explicitly state that the scheduling quantum is not a calibrated DCLK period. |
| Stop Conditions | Stop and transfer any need for pin waveforms, BUSRDY, DMA refresh priority, a physical time base, second scheduler or second CPU/DMA transaction path. |
| Exit Criteria | With zero configured delay existing Core transfer behavior stays immediate; Model-40's selected delay emits no DMA transaction on the first eligible quantum and exactly one transaction on the next, with reset/cancellation clearing pending state. |
| Original Owner Request | Implement DeskPro 386 L3 timing/hardware gaps using original then reference then generic-AT tiers; do not stall and preserve Core/VM boundary. |
| Similar-Issue Sweep | Inspect all Core DMA advance paths, profile constructors, Model-40 BYOB/private construction, direct DMA test fixtures and trace consumers; classify every timing-selector hit. |
## Current Technical Baseline

- **Current developer artifact:** T419 S1 P2 `vm-0-5-0419` /`build/output/nxvm_0_5_0419.exe`, SHA-256 `CE739C2B99E41F473A6F31EDB91DE3D8BD67DBFA56363285DD6EA1CAFC857B67`. T419 runs the existing sole Core DMA transaction route under D4 composition and verifies the original-source high-level no-extra-wait property; it does not model D4 waveform duration, BWAIT or refresh arbitration. T418 instruction-boundary locality, T417 refresh, T416 DMA-HOLD and T415 paging fixes remain retained.
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
