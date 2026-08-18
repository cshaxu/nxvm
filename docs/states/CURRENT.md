# Project Status

## Current Work

## M5 T419 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approval: user’s standing authorization in this thread to continue DeskPro Model-40 L3 work using original sources, reference implementations, then labelled generic AT mechanisms; no repeated approval request. Scope: a bounded original-source D4 DMA `BUSRDY` ready gate only. |
| Objective | Add one Core-owned, configurable DMA-cycle ready gate. A false provider result defers the pending DMA grant without a transfer; when ready resumes, the existing one transaction route performs it. Model-40 selects the capability with its default-ready binding. |
| Non-goals | No CPU `BWAIT`, no DCLK/CLK16 calibration, no refresh request/priority, no HAK/HAKDMA waveform, no new VM transaction executor, and no peripheral-specific BUSRDY wiring. |
| Reference Baseline | `master` at `9afae9d7`; T419 S2 evidence; Compaq D3PE processor description pp. 19–21 under `O:\assets\research\compaq_deskpro_386_16\technical_spec_1986_textmode.txt`, which states BUSRDY can extend a DMA cycle. PCjs DeskPro configuration is inspected only as `reference-derived` topology context and is not a timing source. |
| Candidate Proposal | [DeskPro physical-cycle and phase-timing closure](../proposals/m5-deskpro-physical-cycle-and-phase-timing.md), receiver 2. Evidence tier: `original`. Ledger batch: D4 DMA ready input only. |
| Files And ABI Surface | `src/core/machine/machine_interface.h`, `machine.h`, `machine.c`; Model-40 composition/session configuration only; focused Core and Model-40 smoke tests; indexed evidence and Current. Any public config addition is append-only. |
| Applicable Rules | Architecture: one Core CPU/DMA transaction owner; VM composition selects only Model-40 difference; no reverse dependency. Coding: C11/type vocabulary, bounded owner-local helper. Source policy: no imported third-party code or firmware. Documentation/Execution: packet, evidence, review, full gate, artifact refresh, closure audit. |
| Verification | Focused Core smoke proves a false ready provider blocks both existing grant wait completion and DMA memory write, a true result releases exactly one existing DMA transfer, reset restores no stale wait, and default profiles retain current behavior. Model-40 smoke proves default-ready selection. Run `cmake --build build/mingw-gcc-x64 --target run-current-smokes --parallel 4` and documentation governance gate. |
| Expected Markers | `M5:T419:S3:D4-DMA-BUSRDY:OK`; existing S1/S2 and competition markers remain. |
| Asset Needs | Read-only primary research text under `O:\assets`; no ROM, guest media, external source text, or machine-local path committed. |
| Reporting Requirements | Report source tier, exact discrete contract, commit/push, focused/full verification, and remaining D4 transfers. |
| Stop Conditions | Stop and transfer any request for calibrated waveform duration, CPU BWAIT, refresh/DMA priority, external bus adapter semantics, protected media, or a second transaction route. |
| Exit Criteria | One Core production path owns ready-gated DMA progression; Model-40 selects it without affecting other profiles; focused and full gates pass; evidence names remaining transfers; actual diff review and closure audit are committed. |
| Original Owner Request | Implement DeskPro 386 L3 timing and hardware gaps: use original material first, then reference implementations, then explicitly labelled generic IBM PC/AT scaffolding; do not stop merely because a higher evidence tier is unavailable. |
| Similar-Issue Sweep | Before code: inspect all `core_machine_dma_advance_transaction`, DMA clock, hold, and profile configuration callers. Repair at the shared Core grant boundary; do not add a D4-side scheduler. |
## Current Technical Baseline

- **Current developer artifact:** T419 S2 P6 `vm-0-5-0419` /`build/output/nxvm_0_5_0419.exe`, SHA-256 `84662DCABC6BDA05992D09C9E394F3BA95ED10809724AB37900D1F5365228A17`. T419 runs the existing sole Core DMA transaction route under D4 composition and verifies the original-source high-level no-extra-wait property; it does not model D4 waveform duration, BWAIT or refresh arbitration. T418 instruction-boundary locality, T417 refresh, T416 DMA-HOLD and T415 paging fixes remain retained.
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
