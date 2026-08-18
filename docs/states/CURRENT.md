# Project Status

## Current Work

## M5 T410 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner authorization is retained from the DeskPro continuation conversation on 2026-08-18: continue the approved DeskPro 386 Model 40 L3 timing work, use original material first, repair clear in-scope defects, and commit/push to `master` without repeated approval requests. T410 is the next eligible CPU external-cycle receiver in the accepted DeskPro physical-cycle/phase proposal. |
| Objective | Add one Core-owned, observable CPU external-memory-cycle boundary that records classified memory activity and its lifecycle separately from immediate logical transaction completion. |
| Non-goals | Do not add a D4 wait scalar, infer page hits from adjacent logical accesses, emulate a full 80386 prefetch queue, add a second CPU/DMA transaction owner, change VM contracts, or claim physical/L3 completion. |
| Reference Baseline | `master` at `c739e103`; T408 original D3PE material defines the 2 KiB page and sequential-request rule, while T409 provides Core provenance. The new physical-cycle/phase proposal is the candidate authority. |
| Candidate Proposal | [DeskPro physical-cycle and phase-timing closure](../proposals/m5-deskpro-physical-cycle-and-phase-timing.md) |
| Files And ABI Surface | Core machine CPU execution/transaction/trace internals and focused Core regressions only. Preserve public Core/VM direction; public observation is admitted only if a reusable Core contract is complete. |
| Applicable Rules | `docs/design/ARCHITECTURE.md`, `docs/design/CODING.md`, `docs/rules/ARCHITECTURE.md`, `docs/rules/CODING.md`, `docs/rules/DOCUMENT.md`, `docs/rules/EXECUTION.md`, and `docs/etc/operations/policy/source-policy.md`. |
| Verification | Add focused lifecycle/order/reset/cancellation proof; run affected CTests, full current gate, documentation governance, actual-diff review, and build/copy `nxvm_0_5_0410.exe` if runnable paths change. |
| Expected Markers | `M5:T410:S1:CPU-EXTERNAL-CYCLE:OK`; observable provenance and lifecycle distinct from logical CPU transaction; no timing surcharge. |
| Asset Needs | Read-only owner-managed Compaq DeskPro 386/16 Technical Spec D3PE Processor Descriptions (January 1987); no asset copy or repository import. |
| Reporting Requirements | Report the Core boundary, exact lifecycle observations, regressions/gates, commit/push, and the remaining original D4 overlap/page-mode receiver. |
| Stop Conditions | Stop and transfer if an accurate boundary requires unproven 80386 bus microarchitecture, exposes mutable Core internals, requires vendor assets, or would assign D4 timing to serial logical accesses. |
| Exit Criteria | A reset/cancellation-safe Core external-cycle observation boundary covers CPU prefetch, consumed fetch, page-table walk and data access with focused proof, preserves existing transaction/HOLD/retirement behavior, and publishes no D4 timing. |
| Original Owner Request | "把DeskPro 386能做的L3时序部分都做了；有权威资料直接实现；只有参考实现可参考实现；没有参考加入TODO；不要破坏core和vm接口边界。" |
| Similar-Issue Sweep | Review every CPU physical read/write, instruction preview path, page-walk/writeback, port path, transaction cancel, HOLD/HLDA, reset, and CPU-retirement publication. |

## Current Technical Baseline

- **Current developer artifact:** T409 S1 P1 `vm-0-5-0410` /
  `build/output/nxvm_0_5_0410.exe`, SHA-256
  `9E3EB86B62AD1CDF6A3CBEE18B9A16FBA7FF9F3E56F63E4FE060AF21B3B9E98B`.
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
| T409 | Closed: Core CPU memory transactions now label prefetch, instruction fetch, data, page-table read and page-table writeback, with full-gate proof; no external cycle or D4 page-hit timing is claimed. [Evidence](../etc/evidence/t409-s1-cpu-memory-provenance.md). |
| T408 | Closed: original D4 material proves row-miss two-wait/row-hit zero-wait CPU memory behavior, but current logical accesses cannot safely receive it; external-cycle/prefetch-overlap prerequisite remains transferred. [Evidence](../etc/evidence/t408-s1-original-d4-memory-timing-admission.md). |
| T407 | Closed: existing Core HDC command/sector pending phases publish DRQ/IRQ14 only through the next readiness tick; accepted as generic-AT virtual-time behavior, with Compaq physical/L3 work retained. [Evidence](../etc/evidence/t407-s1-hdc-phase-acceptance.md). |
| T406 | Closed: generic-AT KBC native-byte cadence is Core-owned and Model-40-bound; 287/287 gates pass, while physical keyboard/8042/board timing and L3 remain transferred. [Evidence](../etc/evidence/t406-s1-kbc-serial-cadence.md). |
| T405 | Closed: x86 GCC host build accepts the current product after pointer-width and validated-media-offset repairs; x86 and x64 current gates both pass 286/286, with no L3 claim. [Evidence](../etc/evidence/t405-s1-x86-gcc-host-build-compatibility.md). |
| T404 | Closed: every frozen public profile/catalog/media/controller/display/input route has direct functional proof or a named physical receiver; no timing or L3 claim. [Closure audit](../etc/evidence/t404-s6-batch-b-functional-reconciliation.md). |
| T403 | Closed: shared keyboard serial endpoint prevents accepted break loss under full 8042 output; functional KBC flow is complete while physical keyboard/8042/DeskPro timing remains transferred. [Closure audit](../etc/evidence/t403-s2-keyboard-flow-control-closure-audit.md). |
| T402 | Closed: D3PE-first functional D4 matrix accepted; the Model-40 control aperture repair is reference-labelled where primary material is incomplete, while cache/DRAM/ISA physical timing and L3 remain transferred. [Closure audit](../etc/evidence/t402-s2-d4-functional-closure-audit.md). |
## Recent Governance

- **M5 Td S105 P2:** restored a usable tiered DeskPro work path, recorded the five-receiver ledger and admitted next target T406 for a labelled generic-AT KBC serial-delivery mechanism; this is not a physical/L3 acceptance.

- **M5 Td S104 P1:** added the owner-authorized reference-derived DeskPro timing-bridge candidate ahead of CPU audit; it preserves physical-L3 not-ready and requires a fresh independent re-audit.

- **M5 Td S103 P1:** adopted task-scale convergence and coverage governance: coverage-bearing T work requires a durable universe, batch dispositions, completion predicate and coordinator reconciliation before whole-domain or L3 acceptance.
- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
