# Project Status

## Current Work

**Active: M5 T399.**

## M5 T399 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; the packet heading allocates `M5 T399 S2`. |
| Admission And Approval | Owner-approved in this thread on 2026-08-17 to complete every DeskPro L3-timing item possible before the queued CPU audit: implement primary-backed behavior, otherwise use exact reference-derived evidence, transfer no-reference gaps to TODO, and defer CPU-audit prerequisites until the post-audit next task. S2 consumes only B3. |
| Objective | Reconcile the complete B3 KBC/D4/port-`61h`/fail-safe/IOCHK-NMI observation contract against D3PE and PCjs DeskPro-specific topology, then implement only any missing bounded project-owned behavior and regression. |
| Non-goals | No physical timing scalar, reference tick, firmware execution/import, generic AT substitution, new Core/VM interface, scheduler, B1/B2/B4/B5 behavior, or physical/reference-derived L3 decision. |
| Reference Baseline | T386 D4 functional regressions, T396/T397 owner ledgers, T399 S1 frozen B3 disposition, retained D3PE topology facts, and PCjs exact DeskPro-specific `48h`/port-`61h` model facts. |
| Candidate Proposal | [DeskPro 386 reference-derived timing bridge and L3 re-audit](../proposals/m5-deskpro-386-reference-derived-timing-bridge.md). |
| Files And ABI Surface | Existing Core D4/platform/NMI and VM Model-40 composition owners, focused `tests/machine` smoke coverage, T399 evidence/history and `CURRENT.md`. No new public ABI is admitted. |
| Applicable Rules | Execution, documentation, architecture and coding rules; source/research policy. Preserve Core-neutral ownership, VM-only Model-40 composition, immutable/copy-safe public boundary, no third-party source/asset import, and no host-time coupling. |
| Verification | Focused existing/new B3 smoke; current-gate registration if code changes; reset/cancel replay; source-owner/reference sweep; documentation governance; coordinator actual-diff review. |
| Expected Markers | `M5:T399:S2:B3-CONTRACT-RECONCILED`; if a gap is repaired, focused `M5:T399:S2:B3-*` regression markers. |
| Asset Needs | Read-only D3PE and reference source/configuration metadata only. No ROM, media, source, configuration, trace or binary is copied into NXVM. |
| Reporting Requirements | Record direct-source facts, reference-derived topology facts, every B3 input/checkpoint/reset/cancel result, disagreement outcome, behavior decision, and any remaining receiver. |
| Stop Conditions | Stop implementation if B3 requires an electrical phase/value, an unbounded firmware observation, a generic reference substitution, a new public interface, a protected asset, or a CPU-audit prerequisite. Transfer the whole unsupported class. |
| Exit Criteria | B3 is accepted with direct/reference-derived normalized project proof or transferred whole to one TODO/post-CPU receiver; no unresolved B3 form is hidden; all required verification and governance pass. |
| Original Owner Request | “把DeskPro 386能做的L3时序部分都做了；有权威资料直接实现；只有参考可参考实现；没有参考加入TODO；需等待CPU审计的放到CPU审计之后的下一任务。” |
| Similar-Issue Sweep | Recheck B3 D4/port-`61h`/NMI routes in Core and both Model-40 compositions; distinguish PCjs DeskPro-only bits from generic AT/86Box behavior; review B1/B2/B4/B5 disposition consistency before any transfer. |
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
| T399 S1 | Accepted: five-batch reference-derived timing ledger is frozen; no runtime behavior or physical claim. [Acceptance audit](../etc/evidence/t399-s1-reference-ledger-acceptance-audit.md). |
| T398 | Closed: final exact 1986 DeskPro Model-40 audit is not-ready for L3; five physical/firmware boundaries retain explicit TODO receivers and require a fresh audit after repair. [Closure audit](../etc/evidence/t398-s2-task-closure-audit.md). |
| T397 | Closed: selected functional FDC/HDC/CECG/KBC-NMI/DMA routes and reset/order evidence retained; physical availability, phase and service facts transfer to explicit TODO receivers. Final DeskPro L3 may make only a ready/not-ready decision. [Closure audit](../etc/evidence/t397-s2-task-closure-audit.md). |
| T396 | Closed: finite Model-40 source/owner, six-domain, reset and callback-order ledger retained; all unsourced physical clock, phase, wait and device/firmware visibility facts transfer to the next DeskPro candidate. No physical scalar or L3 claim. [Closure audit](../etc/evidence/t396-s2-task-closure-audit.md). |
| T395 | Closed: private 12-to-64 bounded Core route growth with direct allocation/limit rollback, frozen-registration and order/decline proof. Transfers only the queued DeskPro board-level timing receiver. [Closure audit](../etc/evidence/t395-s2-task-closure-audit.md). |
| T394 | Closed: S1-S7 establish the complete C0 81-key qualification and Core copied-descriptor safety boundary, then transfer physical selection solely to the queued source-backed DeskPro board clock-plan contract. No physical clock or L3 claim. [Closure audit](../etc/evidence/t394-s7-physical-qualification-closure-audit.md). |
| T392 | Closed: S1-S5 exhaust copied C1, VM/session and FDC operation candidates, then repair the missed proposal archival and stale Queue entry; no endpoint or defect exists. Only a separately admitted generic immutable device-operation snapshot plus selected consumer contract can proceed. No Core/VM ABI, CPU timing, physical, board, firmware or L3 claim. [Corrective audit](../etc/evidence/t392-s5-closure-topology-corrective-audit.md). |
| T391 | Closed: S1-S8 establish the bounded C0A startup predecessor, complete C1 aggregate disposition and the truthful transfer that C1 physical qualification remains blocked pending a new finite checkpoint or earliest-owner defect. No Core/VM interface, timing, physical, board, firmware or L3 claim. [Closure audit](../etc/evidence/t391-s8-startup-semantic-closure-audit.md). |
| T389 | Stopped by owner-approved requeue: S1 proves no physical board slice can begin while 80386 successful retirement remains nonphysical; S2 archives the invalid candidate and queues a shared CPU qualification before a fresh board task. No board timing, physical-clock or L3 claim. [Audit](../etc/evidence/t389-s2-stop-and-requeue-audit.md). |

## Recent Governance

- **M5 Td S104 P1:** added the owner-authorized reference-derived DeskPro timing-bridge candidate ahead of CPU audit; it preserves physical-L3 not-ready and requires a fresh independent re-audit.

- **M5 Td S103 P1:** adopted task-scale convergence and coverage governance: coverage-bearing T work requires a durable universe, batch dispositions, completion predicate and coordinator reconciliation before whole-domain or L3 acceptance.
- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
