# Project Status

## Current Work

**T394 S4 active.** Implement the Core-owned normalized eligibility key and construction-time copied descriptor, retaining deterministic Model-40 behavior until a later C0 mapping admits physical selection.

## M5 T394 S4 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner’s continuing instruction-correctness/timing audit authorization; T394 S3 acceptance (`6fe832c6`) selects this bounded Core implementation. No exception. |
| Objective | Materialize the S3 value-only eligibility key in Core, validate and copy a finite descriptor at construction, and reject physical successful retirement when source timing is unallocated or its classified key is absent. |
| Non-goals | No C0 descriptor contents, Model-40 physical selection, VM callback/state coupling, firmware/media handling, board timing, cycle changes or L3 claim. |
| Reference Baseline | T394 S1 ledger, T394 S3 collision audit, existing retirement classifier/observation and prepublication physical rejection. |
| Candidate Proposal | [80386 physical-retirement qualification](../proposals/m5-80386-physical-retirement-qualification.md). |
| Files And ABI Surface | Core machine configuration/retirement interfaces and private state, focused machine regressions, CMake registration, current/history/evidence/index. The public descriptor is immutable input copied synchronously; no VM interface changes. |
| Applicable Rules | Architecture invariants: Core owns decode/timing and copies external configuration; no mutable layout/pointer retention or reverse VM dependency. Coding: minimum durable interface and cohesive owner-local helpers. Execution: complete affected batch, focused regression, artifact and actual-change review. |
| Verification | Focused key equality/normalization, invalid descriptor, copied-lifetime, reset, classified-absent-key nonpublication and unallocated rejection regressions; existing timing and both Model-40 deterministic composition tests; current gate/artifact and documentation governance. |
| Expected Markers | `M5:T394:S4:ELIGIBILITY-KEY:OK`; `M5:T394:S4:PHYSICAL-ABSENT-KEY:OK`; Model-40 deterministic assertions remain green. |
| Asset Needs | None. |
| Reporting Requirements | Record owner/lifetime, validation-to-copy boundary, fault/nonpublication semantics, source sweep and full C0 mapping transfer. |
| Stop Conditions | Stop if a key field requires raw/dynamic firmware data, descriptor copying cannot be failure-atomic, or an existing physical test needs a C0-specific descriptor. |
| Exit Criteria | Key/descriptor implementation and focused proofs are committed, pushed and accepted; no profile becomes physical-qualified; C0 mapping remains explicitly transferred. |
| Original Owner Request | Continue the global CPU correctness/timing audit, repair clear in-scope defects, and preserve the Core/VM boundary. |
| Similar-Issue Sweep | Sweep all physical-retirement publication points, Core construction variants and Model-40 routes; repair the shared Core mechanism or transfer any excluded variant. |

## Current Technical Baseline

- **Current developer artifact:** T394 S4 `vm-0-5-0394` /
  `build/output/nxvm_0_5_0394.exe`, SHA-256
  `2215B0DD8E0BF2B85E34DB705230A6ECF9A7D872A946952FD7469087A0C59CBE`.
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
| T394 S3 | Accepted: Core-owned normalized eligibility-key schema and C0 collision dimensions are defined; materialization, copied descriptor and full mapping remain required before physical selection. Both Model-40 routes remain deterministic. [Evidence](../etc/evidence/t394-s3-normalized-eligibility-key-collision-audit.md). |
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
