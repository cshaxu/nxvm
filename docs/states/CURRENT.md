# Project Status

## Current Work

## M5 T419 S4 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | The user has standing approval in this thread to continue tier-labelled DeskPro L3 work without repeated authorization. Scope is the original-source D4 refresh request and one Core-owned refresh hold bridge. |
| Objective | Convert the existing Model-40 counter-1 refresh pulse into one pending Core refresh hold. At the next arbitration boundary, service one refresh hold before a pending DMA grant, then preserve the existing DMA route. |
| Non-goals | No calibrated BCLK or DCLK durations, no physical refresh memory read or row retention model, no CPU BWAIT, no source-level PAL translation, no external bus-master path, and no VM transaction executor. |
| Reference Baseline | `master` at `29491f99`; approved Compaq DeskPro 386/16 D3PE material records the refresh request, REFRS arbitration, one refresh cycle, and back-to-back refresh/DMA holds. Existing D4 counter-1 pulse only invalidates locality. |
| Candidate Proposal | [DeskPro physical-cycle and phase-timing closure](../proposals/m5-deskpro-physical-cycle-and-phase-timing.md), receiver 2; evidence tier `original`; ledger batch is the refresh request-to-hold bridge. |
| Files And ABI Surface | Core transaction/machine private state and trace surface as needed; Model-40 D4 configuration only; focused Core/Model-40 tests; Current and indexed evidence. Public changes, if necessary, are append-only. |
| Applicable Rules | One Core owner for scheduling, holds and transactions; VM selects only the D4 difference; C11/type vocabulary; original source is design evidence only, never imported code; task, evidence, full gate and artifact rules apply. |
| Verification | A focused trace proves refresh pulse queues one hold, refresh hold request/acknowledge/release precedes a pending DMA transfer without an intervening CPU transaction, request state clears on reset, and non-D4 profiles retain existing behavior. Run focused smokes, full current smokes, and documentation governance. |
| Expected Markers | `M5:T419:S4:D4-REFRESH-HOLD:OK` plus retained S1-S3 markers. |
| Asset Needs | Read-only approved primary research corpus only; no firmware, media, reference source text or local path committed. |
| Reporting Requirements | Record source tier, discrete ordering contract, exact regressions, artifact hash, remaining physical transfers and pushed commit. |
| Stop Conditions | Stop and transfer calibrated phase duration, refresh memory electrical effects, CPU BWAIT, external bus-master continuation, or any second CPU/DMA transaction path. |
| Exit Criteria | One Core refresh request and hold path is selected only for D4, focused and full gates pass, closure audit maps the original facts to the exact bridge, and remaining details are transferred. |
| Original Owner Request | Implement DeskPro 386 L3 timing and hardware gaps with original material first, reference-derived behavior second, then explicitly labelled generic AT scaffolding. |
| Similar-Issue Sweep | Inspect counter-1 outputs, refresh locality invalidation, all transaction hold callers and DMA grant routing; repair the shared Core arbitration point only. |


## Current Technical Baseline

- **Current developer artifact:** T419 S3 P6 `vm-0-5-0419` /`build/output/nxvm_0_5_0419.exe`, SHA-256 `D0F106BB5173AA5190A0CC0417CEA4642889801D086D37BA302E86651857EE3D`. T419 runs the existing sole Core DMA transaction route under D4 composition, with original-source one-quantum DMA grant delay and BUSRDY gate; it does not model waveform duration, CPU BWAIT or refresh arbitration. T418 instruction-boundary locality, T417 refresh, T416 DMA-HOLD and T415 paging fixes remain retained.
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
