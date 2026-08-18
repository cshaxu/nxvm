# Project Status

## Current Work

**Active: M5 T401.**
## M5 T401 S76 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner: user; standing approval to complete the CPU audit before returning to the DeskPro L3 work, with direct repair of confirmed small issues. |
| Objective | Independently audit the complete T401 coverage universe against current source, accepted S1--S75 evidence and original proposal exit criteria; either close T401 truthfully or record each unmet item with its earliest receiver. |
| Non-goals | No semantic/timing overclaim, physical CPU/board/L3 claim, ROM/media import, broad refactor or closure solely because prior subtasks passed. |
| Reference Baseline | `7b6ffb46` (S75 P2), T401 proposal, frozen coverage universe/S1 ledger, S72/S73 map reconciliations, S75 disposition matrix and current source/test graph. |
| Candidate Proposal | [Four-profile CPU instruction-correctness audit](../proposals/m5-four-profile-cpu-instruction-correctness-audit.md), independent task-level closure audit. |
| Files And ABI Surface | T401 ledger, closure evidence and CURRENT; source/tests only for a confirmed bounded gap, with no public ABI change. |
| Applicable Rules | Verify every original task requirement against current evidence; no map, test, handler or deterministic tick alone proves completion; preserve physical DeskPro separation. |
| Verification | Re-read original requested outcomes and proposal completion predicate; inspect source/classifier, all map/coverage evidence, current tests and transfer/TODO/Queue state; documentation governance, diff check and full current gate for any production repair. |
| Expected Markers | Existing S1--S75 markers plus a closure audit outcome with explicit requirement-to-evidence or transfer mapping. |
| Asset Needs | None; no new firmware, guest media or third-party implementation is admitted. |
| Reporting Requirements | State each proposal closure requirement, evidence strength and final satisfied/transfer result; if not closed, list only concrete receivers. |
| Stop Conditions | Admit a bounded repair for a confirmed small gap; transfer broad semantics, physical timing, board/device or x87 breadth without weakening the audit. |
| Exit Criteria | T401 is either independently shown to meet its stated bounded CPU semantic/source-timing scope or remains open with every unmet condition explicitly assigned. |
| Original Owner Request | Continue CPU instruction audit before DeskPro L3 work and use source/reference/generic levels honestly instead of blocking progress. |
| Similar-Issue Sweep | Reconcile semantic, lexical, timing, mode/fault, profile-gate, FPU/provider and transfer requirements together. |
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
| T401 S75 | Accepted: every selected successful CPU class is source-backed, explicitly nonphysical-observed, or external; four-profile fallback observation and corrected TR6 lexical evidence pass. Physical DeskPro timing remains separate. [Ledger](../etc/evidence/t401-s1-four-profile-cpu-audit-ledger.md). |
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
