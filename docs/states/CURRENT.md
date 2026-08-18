# Project Status

## Current Work

## M5 T420 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | The user has standing approval to continue the requested CPU instruction correctness and timing audit without repeated authorization. |
| Objective | Establish the finite Intel-authoritative form and shared-execution-state universe for the next four-profile CPU audit increment, classify every row and duplicate path, and repair only a reproduced shared Core defect. |
| Non-goals | No speculative full rewrite, undefined-FLAGS claim, later CPU feature, reference-emulator substitution for Intel semantics, physical timing scalar, x87 numerical unit, VME/PVI, persistent TLB, or VM-side execution path. |
| Reference Baseline | T340/T341 close prior bounded 80386DX form and state ledgers; T419 is closed and transfers only DeskPro physical timing. The holistic execution-path debt remains the explicit audit trigger. |
| Candidate Proposal | [CPU execution architecture audit](../proposals/m5-cpu-execution-architecture-audit.md), evidence tier `original` for Intel-defined forms and state behavior. |
| Files And ABI Surface | Audit evidence and focused tests first; any repair remains in the existing Core execution owner. VM profiles remain selectors and do not acquire semantics. |
| Applicable Rules | Intel authority, four-profile program, Core ownership, source/research policy, C11/type vocabulary, task/evidence/gate rules. |
| Verification | Publish an enumerated matrix and owner map; run focused and adjacent tests for any repair, static inventories, current gate, documentation governance and diff check. |
| Expected Markers | A new `M5:T420:S1` audit marker; retained T419 closure and prior CPU markers. |
| Asset Needs | Read-only approved Intel research corpus and existing project evidence only; no firmware, media or reference code import. |
| Reporting Requirements | Record every row disposition, authority, shared owner, duplicate-path decision, test proof, artifact hash if rebuilt, and exact transfer conditions. |
| Stop Conditions | Stop and transfer a row that lacks Intel-authoritative behavior, needs a new physical-time model, requires user media/firmware, or would create a second Core/VM path. |
| Exit Criteria | The admitted finite audit slice has no unclassified row; all discovered shared defects are repaired or transferred with evidence. |
| Original Owner Request | Continue the comprehensive CPU instruction implementation, correctness and timing audit after the DeskPro L3 work reaches its available evidence boundary. |
| Similar-Issue Sweep | Inspect opcode metadata, profile gates, decode, shared memory/stack/control helpers, validation/preflight/commit/rollback, exception delivery, timing classification, tests and all profile callers. |
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
| T419 | Closed: bounded D4 discrete timing mechanisms retain one Core owner and shared Model-40/IBM-5170 selection; the asynchronous physical producer and phase work transfer without a Model-L3 claim. [Closure audit](../etc/evidence/t419-s7-task-closure-audit.md). |
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
