# Project Status

## Current Work

**T394 S3 active.** Define and collision-audit a Core-owned normalized eligibility key for the complete C0 universe before any descriptor, physical-mode selection, board timing or L3 claim.

## M5 T394 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approval is the continuing request to complete the global CPU correctness/timing audit while preserving Core/VM boundaries, including the explicit instruction to continue implementation and to repair in-scope issues. T394 S2 acceptance (`3309d338`) selects this bounded prerequisite; no exception is granted. |
| Objective | Define the smallest Core-owned, normalized retirement eligibility-key schema that can represent semantic C0 membership without firmware bytes, PC, asset identity, VM state, callback or retained external pointer; audit it against the complete 82-child C0 universe and identify every required decoder/classifier producer and collision risk. |
| Non-goals | No descriptor activation, physical-contract selection, elapsed/device-time publication change, VM ABI change, firmware/media import, board/device timing, C0A/C1 work, guessed cycle count or L3 claim. |
| Reference Baseline | Current T394 S2 decision; T394 S1 qualification ledger; T390 S27 complete C0 matrix; existing Core retirement observation and classifier surfaces. |
| Candidate Proposal | [80386 physical-retirement qualification](../proposals/m5-80386-physical-retirement-qualification.md). |
| Files And ABI Surface | `docs/states/CURRENT.md`, `docs/history/M5-T394-80386-physical-retirement-qualification.md`, `docs/etc/evidence/t394-s3-normalized-eligibility-key-collision-audit.md`, and `docs/etc/README.md` only. This is a design/evidence S; no production or public ABI change. |
| Applicable Rules | Execution: active-packet, coverage-ledger, evidence, P lifecycle and actual-change review; Documentation: principal-versus-supporting authority and indexed evidence. Architecture/Coding: no implementation change; the evidence must preserve Core semantic ownership and prohibit VM/firmware identity coupling. Source policy: no source, firmware, media or third-party import/research action. |
| Verification | Static source sweep of retirement classifier/observation producers and all C0-specific normalized capture fields; reconcile every C0 child group in T390 S27 against required key dimensions; document exclusion of dynamic literal data and reset/failure implications; run documentation governance and diff hygiene. |
| Expected Markers | `M5:T394:S3:NORMALIZED-KEY-COLLISION-AUDIT:OK`; no production file change; both Model-40 construction routes remain deterministic. |
| Asset Needs | None. No asset, ROM, guest media, local capture or external reference is read or changed. |
| Reporting Requirements | Record schema, producer ownership, C0 coverage/collision disposition, omitted-dimension risk, reset/failure semantics and exact next receiver in indexed evidence. Report any mechanism defect immediately; no physical claim. |
| Stop Conditions | Stop and transfer if the complete C0 matrix cannot be represented without raw bytes/dynamic values, if a required semantic dimension has no Core-owned producer, or if the result requires a VM/firmware identity or wider decoder redesign. |
| Exit Criteria | A bounded key schema, complete C0 group-to-dimension reconciliation, collision/omission decision, stable Core producer inventory, reset/failure requirements and an implementation-ready next receiver are committed, verified and accepted. |
| Original Owner Request | Continue the CPU instruction correctness/timing audit, use external implementations only as read-only references where useful, consider bridge experiments, repair clear in-scope defects, and do not break the Core/VM interface boundary. |
| Similar-Issue Sweep | Design-only: sweep every current retirement observation/classifier producer and both Model-40 construction paths. No defect is repaired in this S; any uncovered production coupling transfers to the next admitted receiver. |

## Current Technical Baseline

- **Current developer artifact:** T390 S22 `vm-0-5-0390` /
  `build/output/nxvm_0_5_0390.exe`, SHA-256
  `AF45AB7BF8D76CBFAD2EEE1C53BB9710CF408997CA9C78948196EE880AB140F8`.
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
| T394 S2 | Accepted: Existing descriptors cannot express complete C0 membership; only a Core-owned normalized eligibility-key design may proceed. Both Model-40 routes remain deterministic; no physical, board or L3 claim. [Evidence](../etc/evidence/t394-s2-immutable-stage-mechanism-decision.md). |
| T392 | Closed: S1-S5 exhaust copied C1, VM/session and FDC operation candidates, then repair the missed proposal archival and stale Queue entry; no endpoint or defect exists. Only a separately admitted generic immutable device-operation snapshot plus selected consumer contract can proceed. No Core/VM ABI, CPU timing, physical, board, firmware or L3 claim. [Corrective audit](../etc/evidence/t392-s5-closure-topology-corrective-audit.md). |
| T391 | Closed: S1-S8 establish the bounded C0A startup predecessor, complete C1 aggregate disposition and the truthful transfer that C1 physical qualification remains blocked pending a new finite checkpoint or earliest-owner defect. No Core/VM interface, timing, physical, board, firmware or L3 claim. [Closure audit](../etc/evidence/t391-s8-startup-semantic-closure-audit.md). |
| T389 | Stopped by owner-approved requeue: S1 proves no physical board slice can begin while 80386 successful retirement remains nonphysical; S2 archives the invalid candidate and queues a shared CPU qualification before a fresh board task. No board timing, physical-clock or L3 claim. [Audit](../etc/evidence/t389-s2-stop-and-requeue-audit.md). |
| T388 | Closed: S1--S9 establish the sole publisher, exhaustive exact-or-nonphysical residual disposition and pre-publication physical-contract rejection. No current CPU profile is physically clock-qualified; board work transfers under this restriction. [Closure audit](../etc/evidence/t388-s9-task-closure-audit.md). |
| T387 | Closed: source/owner inventory, exact port-`61h` reconciliation and reference qualification transfer the shared physical-timebase then physical-device/firmware prerequisites before any DeskPro L3 audit. No physical-time or L3 claim. [Closure audit](../etc/evidence/t387-s6-task-closure-transfer-audit.md). |
| T386 | Closed: S29 independently reconciles every selected Model-40 functional owner/proof, corrects the omitted S26 HDC current-gate registration and transfers only board, firmware and physical-media work to the next candidate/TODO. No L3 claim. [Closure audit](../etc/evidence/t386-s29-functional-closure-audit.md). |
| T385 | Closed: the selected Model-40 80386DX-16 consumes the shared 80386 CPU/state/retirement owners without a DeskPro CPU fork; a clean current-source 251/251 gate passed, while functionality and board timing transfer unchanged. [Closure audit](../etc/evidence/t385-s1-task-closure-audit.md). |

## Recent Governance

- **M5 Td S103 P1:** adopted task-scale convergence and coverage governance: coverage-bearing T work requires a durable universe, batch dispositions, completion predicate and coordinator reconciliation before whole-domain or L3 acceptance.
- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
