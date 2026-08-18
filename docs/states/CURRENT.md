# Project Status

## Current Work

**Active: M5 T400.**

## M5 T400 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New; M5 T400 S1. |
| Admission And Approval | Owner explicitly authorized generic PCjs/86Box/MAME/QEMU/Bochs implementations as a second-tier reference in this conversation on 2026-08-17 after T399 closed; use them rather than leaving feasible logical work undone. Physical claims, third-party imports and Core/VM ABI changes remain excluded. |
| Objective | Freeze the generic-reference DeskPro continuation universe and qualify exact generic component/configuration baselines for HDC, CECG and CPU/DMA/BWAIT deterministic behavior before CPU audit. |
| Non-goals | No physical Compaq timing/identity claim, ROM/media/config/source import, host-time model, Model-40 scheduler, generic behavior silently relabeled as DeskPro-specific, or CPU instruction repair in S1. |
| Reference Baseline | T399 closure; read-only PCjs `c7f21b4fa2bdedac3d5c73094a6402fdc8b24c70`, local 86Box `4fef696a4eead1d55a28d6ac0e5bd2864e5454da`, existing Core HDC/VADP/DMA and Model-40 tests. |
| Candidate Proposal | [DeskPro generic-reference continuation](../proposals/m5-deskpro-386-generic-reference-continuation.md). |
| Files And ABI Surface | Proposal, generic-reference convergence ledger, evidence index and CURRENT; read-only external implementations. No production or public ABI change in S1. |
| Applicable Rules | Execution convergence ledger; architecture Core/VM ownership; document indexing; source policy read-only reference/no import. |
| Verification | Static configuration/source sweep, exact-revision record, documentation governance and diff check. |
| Expected Markers | `M5:T400:S1:GENERIC-REFERENCE-LEDGER-FROZEN`. |
| Asset Needs | None. No O:\assets or external firmware/media is consumed. |
| Reporting Requirements | State generic-versus-DeskPro distinction for every row, reference revision/configuration, possible project probe, receiver and physical boundary. |
| Stop Conditions | Stop a row if no traceable generic component/configuration exists, a probe needs protected assets, or it would cross into physical time/identity. |
| Exit Criteria | Durable finite ledger records every HDC, CECG and CPU/DMA/BWAIT row as eligible, rejected or CPU-audit deferred with a named receiver. |
| Original Owner Request | “既然你都没办法，那当然就只能用通用的了啊，不然活人被尿憋死？” |
| Similar-Issue Sweep | Check all T399 rejected/non-eligible rows and their existing Core/VM tests so generic reference fills only logical gaps and does not erase a physical TODO. |
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
| T399 | Closed: the five-batch reference-derived DeskPro L3 ledger is exhausted; B2/B3 logical contracts are accepted, B1/B4/B5 retain exact receivers, and both reference-derived and physical L3 are not ready. [Closure audit](../etc/evidence/t399-s5-task-closure-audit.md). |
| T398 | Closed: final exact 1986 DeskPro Model-40 audit is not-ready for L3; five physical/firmware boundaries retain explicit TODO receivers and require a fresh audit after repair. [Closure audit](../etc/evidence/t398-s2-task-closure-audit.md). |
| T397 | Closed: selected functional FDC/HDC/CECG/KBC-NMI/DMA routes and reset/order evidence retained; physical availability, phase and service facts transfer to explicit TODO receivers. Final DeskPro L3 may make only a ready/not-ready decision. [Closure audit](../etc/evidence/t397-s2-task-closure-audit.md). |
| T396 | Closed: finite Model-40 source/owner, six-domain, reset and callback-order ledger retained; all unsourced physical clock, phase, wait and device/firmware visibility facts transfer to the next DeskPro candidate. No physical scalar or L3 claim. [Closure audit](../etc/evidence/t396-s2-task-closure-audit.md). |
| T395 | Closed: private 12-to-64 bounded Core route growth with direct allocation/limit rollback, frozen-registration and order/decline proof. Transfers only the queued DeskPro board-level timing receiver. [Closure audit](../etc/evidence/t395-s2-task-closure-audit.md). |
| T394 | Closed: S1-S7 establish the complete C0 81-key qualification and Core copied-descriptor safety boundary, then transfer physical selection solely to the queued source-backed DeskPro board clock-plan contract. No physical clock or L3 claim. [Closure audit](../etc/evidence/t394-s7-physical-qualification-closure-audit.md). |
| T392 | Closed: S1-S5 exhaust copied C1, VM/session and FDC operation candidates, then repair the missed proposal archival and stale Queue entry; no endpoint or defect exists. Only a separately admitted generic immutable device-operation snapshot plus selected consumer contract can proceed. No Core/VM ABI, CPU timing, physical, board, firmware or L3 claim. [Corrective audit](../etc/evidence/t392-s5-closure-topology-corrective-audit.md). |
| T391 | Closed: S1-S8 establish the bounded C0A startup predecessor, complete C1 aggregate disposition and the truthful transfer that C1 physical qualification remains blocked pending a new finite checkpoint or earliest-owner defect. No Core/VM interface, timing, physical, board, firmware or L3 claim. [Closure audit](../etc/evidence/t391-s8-startup-semantic-closure-audit.md). |

## Recent Governance

- **M5 Td S104 P1:** added the owner-authorized reference-derived DeskPro timing-bridge candidate ahead of CPU audit; it preserves physical-L3 not-ready and requires a fresh independent re-audit.

- **M5 Td S103 P1:** adopted task-scale convergence and coverage governance: coverage-bearing T work requires a durable universe, batch dispositions, completion predicate and coordinator reconciliation before whole-domain or L3 acceptance.
- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
