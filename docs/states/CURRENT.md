# Project Status

## Current Work

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation `T391 S6`. |
| Admission And Approval | Owner approved continued CPU/startup audit, use of external owner-managed assets, and Core/VM boundary preservation. S6 is the accepted S5 receiver, limited to one contained C1 replay and aggregate disposition. No exception is requested. |
| Objective | Run the existing test-only C1 transfer capture with the owner-managed DeskPro ROM and startup medium; retain only its normalized terminal and aggregate result. |
| Non-goals | No asset import, repository asset path/hash/provenance, raw PC/byte/trace retention, production code, Core/VM interface, physical clock, board/device time, firmware compatibility or L3 claim. |
| Reference Baseline | Accepted T391 S1--S5, deterministic Model-40 composition, and the test-only `--c1-transfer-diagnostic` capture. |
| Candidate Proposal | `docs/proposals/m5-deskpro-386-startup-semantic-readiness.md`; consumes the S5 finite C1 transfer endpoint. |
| Files And ABI Surface | Evidence/history/current only unless the replay identifies an in-scope test defect. No production or public ABI changes. |
| Applicable Rules | Source policy: external assets are transient owner-managed inputs and never committed. Execution convergence: terminal is `0:7C00` or a named failure/containment condition, never budget success. Architecture: preserve existing copied observer and deterministic composition. |
| Verification | Run one 45-second contained replay with the existing C1 flag; retain normalized output only; run focused capture test and documentation governance; inspect actual P1 diff. |
| Expected Markers | `M5:T391:S5:C1-TRANSFER-CAPTURE:OK` and one normalized replay terminal. |
| Asset Needs | Existing owner-managed external DeskPro Rev-E ROM pair and one 1.2 MB startup medium under `O:\assets`; do not retain their identities, paths, hashes or contents. |
| Reporting Requirements | Record terminal class, classified/unallocated/form aggregate, whether C1 reached, and exact transfer; omit asset identity and raw output. |
| Stop Conditions | Stop if asset validation/session creation fails, terminal output would retain prohibited data, execution exceeds containment, or a result requires a new interface or timing inference. |
| Exit Criteria | One bounded replay produces a normalized C1 result with no retained protected data and names either the complete C1 batch receiver or earliest blocked owner. |
| Original Owner Request | Continue the global CPU correctness/timing audit toward DeskPro board readiness without breaking Core/VM interface boundaries. |
| Similar-Issue Sweep | Retain C0/C0A distinction, differentiate boot-transfer success from containment/fault/unallocated terminals, and keep physical selection excluded. |
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
| T391 S5 | Accepted: test-only C1 collection begins after C0A and stops at a boolean-only `0:7C00` endpoint. A contained replay and complete C1 batch disposition are next; no interface, physical-clock, board-time, firmware or L3 claim. [Evidence](../etc/evidence/t391-s5-c1-boot-transfer-capture.md). |
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
