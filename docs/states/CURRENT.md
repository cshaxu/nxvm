# Project Status

## Current Work

**Active: M5 T392 S5 Corrective.** Repair the discovered closed-task documentation topology only: archive T392's retained proposal beside its history and remove its stale Queue candidate. No runtime source, Core/VM boundary, asset, timing, physical or L3 work is admitted.

## M5 T392 S5 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective |
| Admission And Approval | The owner authorized continued holistic execution and prompt repair of confirmed in-scope issues in the current session on 2026-08-17. This narrow corrective uses the latest closed numeric task T392 under the Execution Rules exception solely to repair its missed proposal archival and stale Queue entry. |
| Objective | Restore T392 closure topology by retaining its proposal beside the numbered history record, removing the completed candidate from Queue, and recording an evidence-backed corrective closure. |
| Non-goals | No runtime source, tests, developer-artifact identity, Core/VM interface, FDC behavior, firmware/media research, asset import, C1 endpoint selection, CPU timing, physical qualification, board timing or L3 claim. |
| Reference Baseline | Accepted T392 S4 P2 `a42bc248`, T392 S4 closure audit, current documentation topology, and the retained T392 proposal before archival. |
| Candidate Proposal | [Retained T392 proposal companion](../proposals/m5-deskpro-386-c1-semantic-endpoint-investigation.md); this corrective does not admit its transferred future mechanism. |
| Files And ABI Surface | `docs/states/CURRENT.md`, `docs/states/QUEUE.md`, `docs/history/M5-T392-deskpro-386-c1-semantic-endpoint-investigation.md`, the moved proposal companion, `docs/etc/README.md`, and one T392 S5 evidence record only. No public or private runtime ABI changes. |
| Applicable Rules | Documentation Rules topology/status/Queue requirements; Execution Rules corrective allocation, P lifecycle, actual-change review, documentation gate and closure audit; Architecture Rules public-boundary invariant is preserved because no runtime surface changes. |
| Verification | `git diff --check`; a complete T392 reference/path sweep; `ctest --test-dir build/t386-s23-verify --output-on-failure -R '^current\.vm-model40-byob-retirement-capture$'`; and `powershell -NoProfile -ExecutionPolicy Bypass -File tools/Verify-DocumentationGovernance.ps1 -RepositoryRoot .`. |
| Expected Markers | The retained proposal exists only under `docs/history/`; Queue has no T392 candidate; T392 history/evidence/index link the corrective record; no `src/`, `tests/`, asset or ABI path changes occur. |
| Asset Needs | None. No firmware, guest media, third-party source, trace or external asset is read, imported, retained or catalogued. |
| Reporting Requirements | Record the defect, complete-reference sweep, no-runtime-change result, verification and boundary in durable evidence; push P1, independently review it, then push P2 acceptance and report the closure. |
| Stop Conditions | Stop and transfer to a new task if repair requires changing runtime source, a public interface, an artifact identity, a new candidate mechanism, source/firmware research, or any task other than T392. |
| Exit Criteria | T392's proposal is archived beside its numbered history, Queue contains only current candidates, all direct references agree, the current task record is structurally valid, focused regression and documentation governance pass, and coordinator review accepts the P1 diff before P2 closure. |
| Original Owner Request | Owner authorized continued holistic execution, prompt repair of confirmed small in-scope issues, and handling of uncommitted files. |
| Similar-Issue Sweep | Sweep Queue candidates, proposal/history locations, T392 links and all directly affected supporting-index rows for the same closed-task topology defect; transfer any unrelated occurrence rather than widening this corrective. |
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
| T392 | Closed: S1-S4 exhaust copied C1, VM/session and FDC operation candidates without an endpoint or defect; only a separately admitted generic immutable device-operation snapshot plus selected consumer contract can proceed. No Core/VM ABI, CPU timing, physical, board, firmware or L3 claim. [Closure audit](../etc/evidence/t392-s4-c1-semantic-endpoint-closure-audit.md). |
| T391 | Closed: S1-S8 establish the bounded C0A startup predecessor, complete C1 aggregate disposition and the truthful transfer that C1 physical qualification remains blocked pending a new finite checkpoint or earliest-owner defect. No Core/VM interface, timing, physical, board, firmware or L3 claim. [Closure audit](../etc/evidence/t391-s8-startup-semantic-closure-audit.md). |
| T389 | Stopped by owner-approved requeue: S1 proves no physical board slice can begin while 80386 successful retirement remains nonphysical; S2 archives the invalid candidate and queues a shared CPU qualification before a fresh board task. No board timing, physical-clock or L3 claim. [Audit](../etc/evidence/t389-s2-stop-and-requeue-audit.md). |
| T388 | Closed: S1--S9 establish the sole publisher, exhaustive exact-or-nonphysical residual disposition and pre-publication physical-contract rejection. No current CPU profile is physically clock-qualified; board work transfers under this restriction. [Closure audit](../etc/evidence/t388-s9-task-closure-audit.md). |
| T387 | Closed: source/owner inventory, exact port-`61h` reconciliation and reference qualification transfer the shared physical-timebase then physical-device/firmware prerequisites before any DeskPro L3 audit. No physical-time or L3 claim. [Closure audit](../etc/evidence/t387-s6-task-closure-transfer-audit.md). |
| T386 | Closed: S29 independently reconciles every selected Model-40 functional owner/proof, corrects the omitted S26 HDC current-gate registration and transfers only board, firmware and physical-media work to the next candidate/TODO. No L3 claim. [Closure audit](../etc/evidence/t386-s29-functional-closure-audit.md). |
| T385 | Closed: the selected Model-40 80386DX-16 consumes the shared 80386 CPU/state/retirement owners without a DeskPro CPU fork; a clean current-source 251/251 gate passed, while functionality and board timing transfer unchanged. [Closure audit](../etc/evidence/t385-s1-task-closure-audit.md). |
| T384 | Closed: the Model-40 capability audit freezes its original Compaq system board/storage ledger and owner-selected color EGA, keyboard and external-ROM constraint; its shared-VADP IBM/Compaq personality boundary transfers CPU, functional and timing work without making a runnable or L3 claim. [Closure audit](../etc/evidence/t384-s2-task-closure-audit.md). |

## Recent Governance

- **M5 Td S103 P1:** adopted task-scale convergence and coverage governance: coverage-bearing T work requires a durable universe, batch dispositions, completion predicate and coordinator reconciliation before whole-domain or L3 acceptance.
- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
