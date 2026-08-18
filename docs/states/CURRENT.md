# Project Status

## Current Work

**Active: M5 T401.**
## M5 T401 S77 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner: user; standing approval to continue the complete CPU audit before DeskPro L3 work and repair confirmed small in-scope issues. |
| Objective | Produce the finite four-profile concrete decoded-form Intel requirement-to-source matrix required by the T401 proposal, reusing verified inventories only where they expose every required row and current owner. |
| Non-goals | No manual/source fabrication, invented timing, physical CPU/board/L3 claim, third-party import, broad refactor, or whole-T401 closure before the matrix is independently reviewed. |
| Reference Baseline | `2e6b7a60` (S76 P2), S76 independent review, T401 proposal/S1 universe, T359/T360/T363 timing inventories and current source/test graph. |
| Candidate Proposal | [Four-profile CPU instruction-correctness audit](../proposals/m5-four-profile-cpu-instruction-correctness-audit.md), concrete-form requirement matrix. |
| Files And ABI Surface | T401 ledger or indexed S77 matrix and CURRENT; production/test files only for a matrix-discovered bounded defect, with no public ABI change. |
| Applicable Rules | Each row must state profile/form, Intel manual locus, current owner, timing disposition, proof, conclusion and repair/transfer; aggregation is valid only with full finite membership. |
| Verification | Compare all primary/`0F`/FPU/prefix map members with T359/T363 inventories and current metadata/classifier source; audit row cardinality/membership and current evidence links; documentation governance and diff check; full gate for production change. |
| Expected Markers | S1/S3/S4/S72/S73 finite-map markers plus an S77 concrete-form matrix cardinality/result record. |
| Asset Needs | None; retained Intel manual references are read-only, and no ROM/media or external source code is imported. |
| Reporting Requirements | Report matrix universe/cardinality, every aggregation rule, row disposition and any exact missing receiver; distinguish original manual, reference-derived and generic boundaries. |
| Stop Conditions | Admit a bounded source/test repair for a confirmed gap; transfer an unresolvable manual/context class without claiming it is closed. |
| Exit Criteria | Every selected decoded form has a durable row or explicit finite aggregation with the proposal-required fields, or every missing class has one concrete receiver. |
| Original Owner Request | Complete the CPU audit honestly before DeskPro L3 work, using original material first, reference implementations second and generic structures only as labelled fallback. |
| Similar-Issue Sweep | Reconcile primary, secondary, FPU/WAIT, prefix and four profile gates together, including valid, invalid and external dispositions. |
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
| T401 S76 | Closure review: map/family/timing evidence is sound but does not yet meet the proposal's concrete-form Intel requirement matrix; S77 is the sole remaining T401 closure receiver. [Review](../etc/evidence/t401-s76-independent-closure-audit.md). |
| T400 | Closed: the HDC, CECG and CPU/DMA/BWAIT generic-reference ledger is exhausted; accepted logical repairs retain every DeskPro physical boundary. [Closure audit](../etc/evidence/t400-s1-task-closure-audit.md). |
| T399 | Closed: the five-batch reference-derived DeskPro L3 ledger is exhausted; B2/B3 logical contracts are accepted, B1/B4/B5 retain exact receivers, and both reference-derived and physical L3 are not ready. [Closure audit](../etc/evidence/t399-s5-task-closure-audit.md). |
| T398 | Closed: final exact 1986 DeskPro Model-40 audit is not-ready for L3; five physical/firmware boundaries retain explicit TODO receivers and require a fresh audit after repair. [Closure audit](../etc/evidence/t398-s2-task-closure-audit.md). |
| T397 | Closed: selected functional FDC/HDC/CECG/KBC-NMI/DMA routes and reset/order evidence retained; physical availability, phase and service facts transfer to explicit TODO receivers. Final DeskPro L3 may make only a ready/not-ready decision. [Closure audit](../etc/evidence/t397-s2-task-closure-audit.md). |
| T396 | Closed: finite Model-40 source/owner, six-domain, reset and callback-order ledger retained; all unsourced physical clock, phase, wait and device/firmware visibility facts transfer to the next DeskPro candidate. No physical scalar or L3 claim. [Closure audit](../etc/evidence/t396-s2-task-closure-audit.md). |
| T395 | Closed: private 12-to-64 bounded Core route growth with direct allocation/limit rollback, frozen-registration and order/decline proof. Transfers only the queued DeskPro board-level timing receiver. [Closure audit](../etc/evidence/t395-s2-task-closure-audit.md). |
| T394 | Closed: S1-S7 establish the complete C0 81-key qualification and Core copied-descriptor safety boundary, then transfer physical selection solely to the queued source-backed DeskPro board clock-plan contract. No physical clock or L3 claim. [Closure audit](../etc/evidence/t394-s7-physical-qualification-closure-audit.md). |
| T392 | Closed: S1-S5 exhaust copied C1, VM/session and FDC operation candidates, then repair the missed proposal archival and stale Queue entry; no endpoint or defect exists. Only a separately admitted generic immutable device-operation snapshot plus selected consumer contract can proceed. No Core/VM ABI, CPU timing, physical, board, firmware or L3 claim. [Corrective audit](../etc/evidence/t392-s5-closure-topology-corrective-audit.md). |

## Recent Governance

- **M5 Td S104 P1:** added the owner-authorized reference-derived DeskPro timing-bridge candidate ahead of CPU audit; it preserves physical-L3 not-ready and requires a fresh independent re-audit.

- **M5 Td S103 P1:** adopted task-scale convergence and coverage governance: coverage-bearing T work requires a durable universe, batch dispositions, completion predicate and coordinator reconciliation before whole-domain or L3 acceptance.
- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
