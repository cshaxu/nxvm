# Project Status

## Current Work

**T398 S1 active.** Build the independent exact-Model-40 requirement-and-evidence matrix for the final DeskPro Model-L3 audit; do not absorb missing physical-device or timing implementation.

## M5 T398 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner continuing global CPU correctness/timing authorization; T397 is task-level closed and queue order selects the final DeskPro Model-L3 audit. No exception. |
| Objective | Reconcile the selected 1986 DeskPro 386/16 configuration's CPU/state, memory/I/O, DMA, interrupts/NMI, selected devices, reset/cancellation and deterministic ordering against direct evidence and all retained receiver boundaries, producing a finite independent readiness matrix. |
| Non-goals | No code repair, physical timing publication, device/firmware implementation, ROM/media import, expanded DeskPro variant, generic 80386 conclusion, Windows claim, x87 numeric claim or pre-decided ready outcome. |
| Reference Baseline | T386 functional closure, T394 physical qualification, T396 board closure, T397 device closure, selected Model-40 private/BYOB composition, retained TODOs, Model-339 L3 boundary and L3 audit proposal. |
| Candidate Proposal | [DeskPro 386 model-L3 closure audit](../proposals/m5-deskpro-386-l3-audit.md). |
| Files And ABI Surface | Evidence matrix and task history/status only unless a later admitted corrective task proves an in-scope defect; no source, artifact or ABI change. |
| Applicable Rules | Independent requirement-to-evidence reconciliation; Core/VM ownership boundaries; no L3 task implementation; every unresolved boundary has a named receiver; current-model distinction. |
| Verification | Complete finite component/route matrix, prior evidence and TODO sweep, cross-device reset/cancellation/order replay using existing tests, full current-gate and documentation governance. |
| Expected Markers | T398 S1 matrix only; no readiness marker until the decision subtask. |
| Asset Needs | Read-only provenance only; no ROM, guest media or external code import. |
| Reporting Requirements | For every matrix row record direct proof, owner, current status, retained receiver and whether it permits a Model-L3 readiness claim. |
| Stop Conditions | Stop before decision if any selected component lacks a finite disposition, a cited source is ambiguous, or a row would infer physical readiness from logical functionality. |
| Exit Criteria | Pushed finite matrix and accepted decision boundary, with no unclassified selected route. |
| Original Owner Request | Continue the global CPU correctness/timing audit, repair clear in-scope defects, and preserve the Core/VM boundary. |
| Similar-Issue Sweep | Sweep Model-40 private/BYOB composition, all T386/T394/T396/T397 evidence and TODOs, current tests, Model-339 L3 record and PC/XT distinction; no runtime defect class is in scope. |
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
| T397 | Closed: selected functional FDC/HDC/CECG/KBC-NMI/DMA routes and reset/order evidence retained; physical availability, phase and service facts transfer to explicit TODO receivers. Final DeskPro L3 may make only a ready/not-ready decision. [Closure audit](../etc/evidence/t397-s2-task-closure-audit.md). |
| T396 | Closed: finite Model-40 source/owner, six-domain, reset and callback-order ledger retained; all unsourced physical clock, phase, wait and device/firmware visibility facts transfer to the next DeskPro candidate. No physical scalar or L3 claim. [Closure audit](../etc/evidence/t396-s2-task-closure-audit.md). |
| T395 | Closed: private 12-to-64 bounded Core route growth with direct allocation/limit rollback, frozen-registration and order/decline proof. Transfers only the queued DeskPro board-level timing receiver. [Closure audit](../etc/evidence/t395-s2-task-closure-audit.md). |
| T394 | Closed: S1-S7 establish the complete C0 81-key qualification and Core copied-descriptor safety boundary, then transfer physical selection solely to the queued source-backed DeskPro board clock-plan contract. No physical clock or L3 claim. [Closure audit](../etc/evidence/t394-s7-physical-qualification-closure-audit.md). |
| T392 | Closed: S1-S5 exhaust copied C1, VM/session and FDC operation candidates, then repair the missed proposal archival and stale Queue entry; no endpoint or defect exists. Only a separately admitted generic immutable device-operation snapshot plus selected consumer contract can proceed. No Core/VM ABI, CPU timing, physical, board, firmware or L3 claim. [Corrective audit](../etc/evidence/t392-s5-closure-topology-corrective-audit.md). |
| T391 | Closed: S1-S8 establish the bounded C0A startup predecessor, complete C1 aggregate disposition and the truthful transfer that C1 physical qualification remains blocked pending a new finite checkpoint or earliest-owner defect. No Core/VM interface, timing, physical, board, firmware or L3 claim. [Closure audit](../etc/evidence/t391-s8-startup-semantic-closure-audit.md). |
| T389 | Stopped by owner-approved requeue: S1 proves no physical board slice can begin while 80386 successful retirement remains nonphysical; S2 archives the invalid candidate and queues a shared CPU qualification before a fresh board task. No board timing, physical-clock or L3 claim. [Audit](../etc/evidence/t389-s2-stop-and-requeue-audit.md). |
| T388 | Closed: S1--S9 establish the sole publisher, exhaustive exact-or-nonphysical residual disposition and pre-publication physical-contract rejection. No current CPU profile is physically clock-qualified; board work transfers under this restriction. [Closure audit](../etc/evidence/t388-s9-task-closure-audit.md). |

## Recent Governance

- **M5 Td S103 P1:** adopted task-scale convergence and coverage governance: coverage-bearing T work requires a durable universe, batch dispositions, completion predicate and coordinator reconciliation before whole-domain or L3 acceptance.
- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
