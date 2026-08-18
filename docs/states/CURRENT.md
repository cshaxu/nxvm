# Project Status

## Current Work

**T396 S1 active.** Establish the finite source and implementation baseline for the DeskPro 386 Model 40 board-level timing closure before publishing any clock or bus timing value.

## M5 T396 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner continuing global CPU correctness/timing authorization; T395 is task-level closed and queue order selects the next DeskPro board-level timing prerequisite. No exception. |
| Objective | Reconcile the accepted CPU qualification input, Model-40 clock/topology declarations, primary-source availability and existing Core time/DMA/PIT/PIC/NMI/reset consumers into a finite implementation ledger. |
| Non-goals | No speculative timing constant, physical clock publication, CPU repair, ROM/media import, firmware service timing, host wall-clock coupling, VM/Core ABI change or L3 claim. |
| Reference Baseline | T394 physical-qualification closure, T395 closure, DeskPro board-level timing proposal, current Model-40 private/BYOB composition and Core rational-clock owners. |
| Candidate Proposal | [DeskPro 386 board-level timing closure](../proposals/m5-deskpro-386-board-level-timing-closure.md). |
| Files And ABI Surface | Source/evidence ledger and task history/status only unless a later admitted implementation batch proves a source-backed Core/VM owner change. |
| Applicable Rules | Core owns shared clock/transaction/DMA/PIC/PIT/reset mechanisms; VM owns Model-40 topology declaration; primary source precedes bounded secondary observation. |
| Verification | Complete owner/caller and source-availability sweep, explicit clock/reset/visibility ledger, focused existing Model-40 and Core clock regressions, documentation governance. |
| Expected Markers | T396 S1 source-and-owner ledger only; no physical timing marker. |
| Asset Needs | Read-only metadata/provenance only; no ROM, guest media or external code import. |
| Reporting Requirements | Record every timing domain, current neutral value, owner, source strength, consumer, reset phase and earliest receiver for unresolved facts. |
| Stop Conditions | Stop before implementation if the required primary source is absent, only range/unknown phase exists, or a proposed value would publish a scalar without a board contract. |
| Exit Criteria | Pushed finite ledger and accepted implementation/non-admission boundary, with every unresolved board/device fact transferred explicitly. |
| Original Owner Request | Continue the global CPU correctness/timing audit, repair clear in-scope defects, and preserve the Core/VM boundary. |
| Similar-Issue Sweep | Sweep Model-40 private/BYOB construction, Core clock plans, PIT/DMA/PIC/RTC/KBC/NMI/reset consumers, evidence/provenance and local reference implementations. |
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
| T395 | Closed: private 12-to-64 bounded Core route growth with direct allocation/limit rollback, frozen-registration and order/decline proof. Transfers only the queued DeskPro board-level timing receiver. [Closure audit](../etc/evidence/t395-s2-task-closure-audit.md). |
| T394 | Closed: S1-S7 establish the complete C0 81-key qualification and Core copied-descriptor safety boundary, then transfer physical selection solely to the queued source-backed DeskPro board clock-plan contract. No physical clock or L3 claim. [Closure audit](../etc/evidence/t394-s7-physical-qualification-closure-audit.md). |
| T392 | Closed: S1-S5 exhaust copied C1, VM/session and FDC operation candidates, then repair the missed proposal archival and stale Queue entry; no endpoint or defect exists. Only a separately admitted generic immutable device-operation snapshot plus selected consumer contract can proceed. No Core/VM ABI, CPU timing, physical, board, firmware or L3 claim. [Corrective audit](../etc/evidence/t392-s5-closure-topology-corrective-audit.md). |
| T391 | Closed: S1-S8 establish the bounded C0A startup predecessor, complete C1 aggregate disposition and the truthful transfer that C1 physical qualification remains blocked pending a new finite checkpoint or earliest-owner defect. No Core/VM interface, timing, physical, board, firmware or L3 claim. [Closure audit](../etc/evidence/t391-s8-startup-semantic-closure-audit.md). |
| T389 | Stopped by owner-approved requeue: S1 proves no physical board slice can begin while 80386 successful retirement remains nonphysical; S2 archives the invalid candidate and queues a shared CPU qualification before a fresh board task. No board timing, physical-clock or L3 claim. [Audit](../etc/evidence/t389-s2-stop-and-requeue-audit.md). |
| T388 | Closed: S1--S9 establish the sole publisher, exhaustive exact-or-nonphysical residual disposition and pre-publication physical-contract rejection. No current CPU profile is physically clock-qualified; board work transfers under this restriction. [Closure audit](../etc/evidence/t388-s9-task-closure-audit.md). |
| T387 | Closed: source/owner inventory, exact port-`61h` reconciliation and reference qualification transfer the shared physical-timebase then physical-device/firmware prerequisites before any DeskPro L3 audit. No physical-time or L3 claim. [Closure audit](../etc/evidence/t387-s6-task-closure-transfer-audit.md). |
| T386 | Closed: S29 independently reconciles every selected Model-40 functional owner/proof, corrects the omitted S26 HDC current-gate registration and transfers only board, firmware and physical-media work to the next candidate/TODO. No L3 claim. [Closure audit](../etc/evidence/t386-s29-functional-closure-audit.md). |

## Recent Governance

- **M5 Td S103 P1:** adopted task-scale convergence and coverage governance: coverage-bearing T work requires a durable universe, batch dispositions, completion predicate and coordinator reconciliation before whole-domain or L3 acceptance.
- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
