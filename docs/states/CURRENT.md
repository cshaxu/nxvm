# Project Status

## Current Work

## M5 T412 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner authorization of 2026-08-18 covers continued DeskPro L3 implementation and master pushes. |
| Objective | Bind a tier-labelled CPU external-read locality wait policy to the now-observable Core prefetch refill boundary: retain the original D4 2 KiB/two-wait/zero-hit facts, use read-only PCjs only for Model-40 composition shape, and use a generic-AT locality mechanism where the exact D4 row/bank PAL mapping is unavailable. |
| Non-goals | Do not claim the generic/reference bridge is the D4 PAL, or infer Compaq queue depth, overlap, writes, DMA/refresh, BWAIT, device timing, or a Model-L3 result. |
| Reference Baseline | T408 original D4 read timing, T409/T410 classified external cycles, and T411 persistent Core prefetch window. |
| Candidate Proposal | [DeskPro physical-cycle and phase-timing closure](../proposals/m5-deskpro-physical-cycle-and-phase-timing.md) |
| Files And ABI Surface | Core observable cycle contract and Model-40 binding only if evidence supports it; preserve all existing owners. |
| Applicable Rules | docs/design/ARCHITECTURE.md; docs/design/CODING.md; docs/rules/ARCHITECTURE.md; docs/rules/CODING.md; docs/rules/EXECUTION.md. |
| Verification | Original-source row mapping review, read-only PCjs comparison, focused read/hit/miss/reset/control-transfer timing tests, current gate, documentation gate and diff review. |
| Expected Markers | M5:T412:S1:EXTERNAL-READ-LOCALITY:OK, with tier-labelled D4 mapping transfer. |
| Asset Needs | O:\assets original D3PE research and read-only PCjs/86Box references; no import. |
| Reporting Requirements | Label every mechanism original, reference-derived or generic-AT and name unbound physical receivers; do not use the exact D4 label for the bridge. |
| Stop Conditions | Stop only if a bounded Core locality policy cannot publish waits at retirement without a second transaction/scheduler path; transfer the exact D4 row/bank mapping and phase boundary separately. |
| Exit Criteria | The Core-owned generic locality policy is Model-40-configured with hit/miss/reset/cancellation proof, while the exact original D4 decoder remains explicitly transferred. |
| Original Owner Request | Implement DeskPro 386 L3 timing/hardware gaps using original then reference then generic-AT tiers; do not stall and preserve Core/VM boundary. |
| Similar-Issue Sweep | Inspect prefetch refill, data reads, page walks, writes, reset, control transfer, HOLD/HLDA, ROM/RAM maps and Model-40 composition. |

## Current Technical Baseline

- **Current developer artifact:** T412 S1 P1 `vm-0-5-0412` /
  `build/output/nxvm_0_5_0412.exe`, SHA-256
  `3B4048CD4C177020530D69734D8ABD376E8B909C4AC0AB0369BDFA2F0B75C2C6`.
  The isolated x64 current gate passes 288/288. T412 adds a Model-40-bound generic-AT external-read locality bridge; the exact D4 PAL remains transferred. T409 labels Core CPU memory
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
| T411 | Closed: Core persistent instruction-prefetch window and task-level lifecycle audit establish the D4 physical-cycle prerequisite, while row/page waits, BWAIT and arbitration remain explicitly transferred. [Closure audit](../etc/evidence/t411-s2-prefetch-closure-audit.md). |
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
