# Project Status

## Current Work

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation `T391 S5`. |
| Admission And Approval | Owner approved continued CPU/startup work and requires the Core/VM boundary remain unchanged. S5 is the accepted S4 receiver, limited to a test-only C1 transfer capture. No exception is requested. |
| Objective | Add a bounded test-only capture mode that begins after the existing C0A endpoint and records only a boolean when existing copied `linear_pc` reaches `0:7C00`. |
| Non-goals | No production Core/VM source, observer ABI, provider, port route, profile behavior, physical-retirement selection, board/device time, firmware/media import, raw PC/byte output, or L3 claim. |
| Reference Baseline | Accepted T391 S1--S4, existing Model-40 BYOB retirement-capture test, existing copied retirement observation, and deterministic Model-40 composition. |
| Candidate Proposal | `docs/proposals/m5-deskpro-386-startup-semantic-readiness.md`; consumes S4's accepted `0:7C00` C1 endpoint. |
| Files And ABI Surface | Test-only capture and evidence: `tests/products/vm_model40_byob_retirement_capture.c`, `docs/states/CURRENT.md`, T391 history and new S5 evidence. No public or production ABI changes. |
| Applicable Rules | Architecture/coding: test consumer reads only existing copied snapshot and creates no mirror public contract. Execution convergence: C1 capture is bounded by named C0A and boot-transfer endpoints, never a budget. Documentation: record reset/failure/transfer boundaries. |
| Verification | Extend canonical synthetic capture with C0A-to-boot-transfer sequence; run focused current capture test, documentation governance, diff hygiene and actual P1 review. |
| Expected Markers | Existing `M5:T391:S2:C0A-CAPTURE:OK`; new `M5:T391:S5:C1-TRANSFER-CAPTURE:OK`. |
| Asset Needs | None for the synthetic regression. Any later owner-managed replay remains transient and records no asset identity or raw trace. |
| Reporting Requirements | Record collection start/end, boolean-only terminal, reset/fault/unallocated/form-capacity handling and the absence of an ABI/runtime change. |
| Stop Conditions | Stop if the existing snapshot cannot express the boolean endpoint, if C0A cannot be excluded, if the test would retain PC/bytes, or if a production/interface change is required. |
| Exit Criteria | Canonical synthetic test proves pre-C0A observations are excluded, C0A endpoint is excluded, post-C0A observations aggregate, `0:7C00` sets the terminal boolean, and reset/failure boundaries remain unchanged. |
| Original Owner Request | Continue the global CPU/timing audit toward DeskPro board readiness without breaking the Core/VM interface boundary. |
| Similar-Issue Sweep | Keep legacy C0, C0A and old C1 protected-entry modes behaviorally covered; distinguish the named boot-transfer boolean from a raw PC or a containment terminal. |
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
| T391 S4 | Accepted: the C1 candidate inventory selects only a test-local `0:7C00` boolean endpoint from the existing copied observation; second protected entry and post-C0 I/O remain ineligible. A bounded C1 capture is next; no interface, physical-clock, board-time, firmware or L3 claim. [Evidence](../etc/evidence/t391-s4-c1-semantic-observability.md). |
| T389 | Stopped by owner-approved requeue: S1 proves no physical board slice can begin while 80386 successful retirement remains nonphysical; S2 archives the invalid candidate and queues a shared CPU qualification before a fresh board task. No board timing, physical-clock or L3 claim. [Audit](../etc/evidence/t389-s2-stop-and-requeue-audit.md). |
| T388 | Closed: S1--S9 establish the sole publisher, exhaustive exact-or-nonphysical residual disposition and pre-publication physical-contract rejection. No current CPU profile is physically clock-qualified; board work transfers under this restriction. [Closure audit](../etc/evidence/t388-s9-task-closure-audit.md). |
| T387 | Closed: source/owner inventory, exact port-`61h` reconciliation and reference qualification transfer the shared physical-timebase then physical-device/firmware prerequisites before any DeskPro L3 audit. No physical-time or L3 claim. [Closure audit](../etc/evidence/t387-s6-task-closure-transfer-audit.md). |
| T386 | Closed: S29 independently reconciles every selected Model-40 functional owner/proof, corrects the omitted S26 HDC current-gate registration and transfers only board, firmware and physical-media work to the next candidate/TODO. No L3 claim. [Closure audit](../etc/evidence/t386-s29-functional-closure-audit.md). |
| T385 | Closed: the selected Model-40 80386DX-16 consumes the shared 80386 CPU/state/retirement owners without a DeskPro CPU fork; a clean current-source 251/251 gate passed, while functionality and board timing transfer unchanged. [Closure audit](../etc/evidence/t385-s1-task-closure-audit.md). |
| T384 | Closed: the Model-40 capability audit freezes its original Compaq system board/storage ledger and owner-selected color EGA, keyboard and external-ROM constraint; its shared-VADP IBM/Compaq personality boundary transfers CPU, functional and timing work without making a runnable or L3 claim. [Closure audit](../etc/evidence/t384-s2-task-closure-audit.md). |
| T383 | Closed: independent current-source replay accepts the frozen Model-339 deterministic L3 contract after T380's strict-start repair; 251/251 current gate and focused matrix are retained in [evidence](../etc/evidence/t383-s1-model339-current-source-reaudit.md). |

## Recent Governance

- **M5 Td S103 P1:** adopted task-scale convergence and coverage governance: coverage-bearing T work requires a durable universe, batch dispositions, completion predicate and coordinator reconciliation before whole-domain or L3 acceptance.
- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
