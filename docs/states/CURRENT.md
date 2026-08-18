# Project Status

## Current Work

**T396 S2 active.** Perform the explicit task-closure audit for the accepted DeskPro 386 Model 40 board-timing ledger; do not publish an unsourced clock or expand into device/firmware timing or L3.

## M5 T396 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner continuing global CPU correctness/timing authorization; accepted T396 S1 retains the required latest-task progress row. No exception. |
| Objective | Reconcile every T396 proposal requirement against the accepted S1 source/owner ledger, primary-source boundary, focused regressions and current Core/VM contracts; close T396 or record the exact remaining receiver. |
| Non-goals | No new Core/VM behavior, physical clock scalar, DMA wait placement, ROM/media import, device-service timing, firmware change, host wall-clock coupling or L3 claim. |
| Reference Baseline | T396 S1 P3/P4, source-and-owner acceptance audit, T396 proposal, T394 physical-qualification closure, current neutral Model-40 constructors and current `vm-0-5-0395` artifact. |
| Candidate Proposal | [DeskPro 386 board-level timing closure](../proposals/m5-deskpro-386-board-level-timing-closure.md). |
| Files And ABI Surface | Closure evidence, task history, status and retained proposal archive only; no source, artifact or ABI change. |
| Applicable Rules | Coordinator actual-change review; primary source before bounded secondary observation; Core shared time/transaction ownership; VM selected topology declaration; task-scale closure and documentation governance. |
| Verification | Inspect actual S1 delivery and every proposal bullet, reconcile the consumer ledger and transfers, documentation governance and diff hygiene. |
| Expected Markers | T396 S2 task-closure audit only; no runtime or physical-timing marker. |
| Asset Needs | Read-only provenance/evidence only; no ROM, guest media or external code import. |
| Reporting Requirements | Record each accepted board-level fact, each rejected physical publication, verification result and named physical-device/firmware or L3 receiver. |
| Stop Conditions | Stop task closure if any board-clock proposal requirement lacks direct evidence, a remaining fact lacks a named receiver, or closure would imply a physical/L3 claim. |
| Exit Criteria | Pushed closure audit and status transition to one task-level T396 summary with its proposal archived, or a documented remaining T396 implementation receiver. |
| Original Owner Request | Continue the global CPU correctness/timing audit, repair clear in-scope defects, and preserve the Core/VM boundary. |
| Similar-Issue Sweep | Reconcile both Model-40 routes, every six-domain consumer/reset row, D3PE source facts, local reference disqualifications and all T396 proposal bullets; no new defect class is in scope. |
## Current Technical Baseline

- **Current developer artifact:** T395 S1 P4 `vm-0-5-0395` /
  `build/output/nxvm_0_5_0395.exe`, SHA-256
  `5DBD4B50762BAD5E393CD8EFA8A0CB7E06FD9A1304A44F8993CC64915A99D4A6`.
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
| T396 S1 | Accepted: finite Model-40 source/owner, six-domain, reset and callback-order ledger; neutral deterministic plan retained. Transfers only a sourced physical clock contract or explicit board-task closure. [Acceptance audit](../etc/evidence/t396-s1-source-owner-acceptance-audit.md). |
| T395 | Closed: private 12-to-64 bounded Core route growth with direct allocation/limit rollback, frozen-registration and order/decline proof. Transfers only the queued DeskPro board-level timing receiver. [Closure audit](../etc/evidence/t395-s2-task-closure-audit.md). |
| T394 | Closed: S1-S7 establish the complete C0 81-key qualification and Core copied-descriptor safety boundary, then transfer physical selection solely to the queued source-backed DeskPro board clock-plan contract. No physical clock or L3 claim. [Closure audit](../etc/evidence/t394-s7-physical-qualification-closure-audit.md). |
| T392 | Closed: S1-S5 exhaust copied C1, VM/session and FDC operation candidates, then repair the missed proposal archival and stale Queue entry; no endpoint or defect exists. Only a separately admitted generic immutable device-operation snapshot plus selected consumer contract can proceed. No Core/VM ABI, CPU timing, physical, board, firmware or L3 claim. [Corrective audit](../etc/evidence/t392-s5-closure-topology-corrective-audit.md). |
| T391 | Closed: S1-S8 establish the bounded C0A startup predecessor, complete C1 aggregate disposition and the truthful transfer that C1 physical qualification remains blocked pending a new finite checkpoint or earliest-owner defect. No Core/VM interface, timing, physical, board, firmware or L3 claim. [Closure audit](../etc/evidence/t391-s8-startup-semantic-closure-audit.md). |
| T389 | Stopped by owner-approved requeue: S1 proves no physical board slice can begin while 80386 successful retirement remains nonphysical; S2 archives the invalid candidate and queues a shared CPU qualification before a fresh board task. No board timing, physical-clock or L3 claim. [Audit](../etc/evidence/t389-s2-stop-and-requeue-audit.md). |
| T388 | Closed: S1--S9 establish the sole publisher, exhaustive exact-or-nonphysical residual disposition and pre-publication physical-contract rejection. No current CPU profile is physically clock-qualified; board work transfers under this restriction. [Closure audit](../etc/evidence/t388-s9-task-closure-audit.md). |
| T387 | Closed: source/owner inventory, exact port-`61h` reconciliation and reference qualification transfer the shared physical-timebase then physical-device/firmware prerequisites before any DeskPro L3 audit. No physical-time or L3 claim. [Closure audit](../etc/evidence/t387-s6-task-closure-transfer-audit.md). |

## Recent Governance

- **M5 Td S103 P1:** adopted task-scale convergence and coverage governance: coverage-bearing T work requires a durable universe, batch dispositions, completion predicate and coordinator reconciliation before whole-domain or L3 acceptance.
- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
