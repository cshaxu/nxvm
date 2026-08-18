# Project Status

## Current Work

## M5 T399 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New; the packet heading allocates `M5 T399 S1`. |
| Admission And Approval | Owner-approved in this thread on 2026-08-17: proceed with the queued DeskPro reference-derived timing bridge because no physical hardware observation is available. Scope is the exact 1986 DeskPro 386 Model 40 only; reference evidence may establish only the explicitly named reference-derived tier. |
| Objective | Freeze and qualify the durable five-batch reference-derived convergence ledger, including exact-reference provenance and a reproducible observation-contract boundary, before any behavior change. |
| Non-goals | No physical-time, physical-L3, generic-PC/AT, source/firmware/media/configuration import, ROM discovery, Model-40-local scheduler, Core/VM interface, or production timing-value claim. |
| Reference Baseline | T396--T398 DeskPro closure evidence; retained D3PE topology facts; the current deterministic Core clock/timeline/transaction owners and VM Model-40 composition. |
| Candidate Proposal | [DeskPro 386 reference-derived timing bridge and L3 re-audit](../proposals/m5-deskpro-386-reference-derived-timing-bridge.md). |
| Files And ABI Surface | `docs/states/CURRENT.md`, `docs/states/QUEUE.md`, the DeskPro closure context, `docs/history/M5-T399-deskpro-386-reference-derived-timing-bridge.md`, and indexed evidence only. No source or ABI surface is admitted in S1. |
| Applicable Rules | Documentation and execution governance; source/research policy; T396/T397 ownership and no-substitution boundaries. Architecture and coding rules are not applicable because S1 changes no code. |
| Verification | Documentation governance combined gate; link and path inspection; read-only exact-reference configuration/provenance sweep; coordinator actual-change review. |
| Expected Markers | `M5:T399:S1:REFERENCE-LEDGER-FROZEN`; five complete batch rows; each row has an exact-reference qualification/disposition, D3PE constraint, input/checkpoint/reset contract, disagreement mask, and named next receiver. |
| Asset Needs | Read-only owner-managed D3PE facts and read-only external reference checkouts only. No asset, ROM, firmware, media, trace, or configuration copy enters NXVM. |
| Reporting Requirements | Record the frozen universe and disposition for all five batches, exact reference revision/configuration or rejection reason, reset/cancellation and disagreement handling, and report any implementation admission separately. |
| Stop Conditions | Stop before a behavior change if no reference proves the exact DeskPro configuration, no bounded reproducible probe/checkpoint exists, D3PE conflicts, a reference tick would be treated as hardware time, or a source/asset import would be required. |
| Exit Criteria | One indexed evidence ledger exhausts all five proposed batches into qualified observation contracts, explicit non-eligibility/contradiction, or a named receiver; queue is truthful; governance passes; no physical or production timing claim is made. |
| Original Owner Request | “那就好好干吧开始” after authorizing the reference-derived DeskPro bridge and Queue/proposal update. |
| Similar-Issue Sweep | Recheck the completed T394 physical-retirement candidate for stale Queue/context references; sweep local PCjs, 86Box, MAME and Bochs availability/configuration before treating any route as reference-derived evidence. |

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
