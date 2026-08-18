# Project Status

## Current Work

**Active: M5 T401.**
## M5 T401 S78 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner: user; standing approval to complete the CPU audit before returning to DeskPro L3 work, with direct correction of confirmed small issues. |
| Objective | Independently review the S77 concrete-form matrix against the original T401 completion predicate, current dispatch/metadata/classifier graph and retained evidence; close T401 only if every required CPU semantic/source-timing disposition is proved or explicitly transferred. |
| Non-goals | No physical CPU/board/L3 claim, numerical x87 claim, ROM/media import, history rewrite, or closure based solely on green tests. |
| Reference Baseline | `0071e264` (S77 P2), T401 proposal, S76 review, S77 matrix, T359/T360 validators and current Core source/test graph. |
| Candidate Proposal | [Four-profile CPU instruction-correctness audit](../proposals/m5-four-profile-cpu-instruction-correctness-audit.md), independent final closure review. |
| Files And ABI Surface | T401 closure evidence, index and CURRENT; source/tests only for a confirmed bounded discrepancy, with no public ABI change. |
| Applicable Rules | Independently verify all original closure bullets and every transfer; do not treat a timing fallback, map membership or single test as a physical or whole-machine result. |
| Verification | Re-run T359/T360 inventory verifiers, inspect S77 membership against S72/S73 and current source, re-run full current gate, documentation governance and diff check; review exact changes before acceptance. |
| Expected Markers | Existing S1--S77 markers plus a closure audit result and, if accepted, a task-level closure record. |
| Asset Needs | None; no external artifact is added. |
| Reporting Requirements | Report each original T401 closure bullet, proof/transfer and final decision; state the exact post-T401 DeskPro receiver boundary. |
| Stop Conditions | Keep T401 open and assign a concrete corrective receiver for any unclassified form, source-timing gap, missing proof or unsupported transfer. |
| Exit Criteria | T401 is closed only when every selected CPU semantic/source-timing row is mapped, nonphysical/external boundaries are explicit, and no physical DeskPro claim is implied. |
| Original Owner Request | Continue CPU audit before DeskPro L3 work, then continue the DeskPro hardware/timing program with original/reference/generic levels labelled. |
| Similar-Issue Sweep | Reconcile all matrix rows, metadata-invalid forms, prefix variants, FPU/provider boundary, timing origins, source-unallocated observation and physical transfers together. |
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
| T401 S77 | Matrix complete: every frozen primary, `0F`, prefix and FPU/WAIT form class maps to Intel authority, current owner, proof and source/nonphysical/external disposition; independent closure review is next. [Matrix](../etc/evidence/t401-s77-concrete-form-requirement-matrix.md). |
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
