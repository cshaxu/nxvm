# Project Status

## Current Work

**Active.** T390 S31 is correcting the Model-40 A20-disabled physical-address policy so the 80386 high-reset ROM routes remain reachable while only the documented second-MiB compatibility window mirrors. It re-establishes a bounded corrected-ROM startup baseline; no board-time or L3 claim follows.

## M5 T390 S31 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; T390 remains the latest open numeric task and S31 follows accepted S30. |
| Admission And Approval | Owner approved continued implementation, external reference comparison and bridge experiments in the current task conversation on 2026-08-17; scope is the confirmed Model-40 startup defect. |
| Objective | Correct Model-40 A20-disabled physical-address policy so high reset/ROM routes remain reachable while only the documented first-to-second-MiB compatibility window mirrors; re-establish a bounded corrected-ROM startup baseline. |
| Non-goals | No CPU timing value, board clock, L3 conclusion, ROM/media import, dynamic external-emulator claim, or generic-PC behavioral change. |
| Reference Baseline | Accepted T390 S30 corrected ROM decode and bounded pre-protected replay; read-only PCjs DeskPro behavior shows only the first-to-second-MiB compatibility mirror while high addresses remain physical. |
| Candidate Proposal | Continue the accepted [T390 physical-qualification proposal](../proposals/m5-80386-physical-retirement-qualification.md) with a Core-owned A20 wrap-policy configuration selected by the Model-40 composition. |
| Files And ABI Surface | Core private RAM policy storage and Core configuration field; Model-40 composition selects its profile policy; focused Core/Model-40 tests and evidence only. Existing request, CPU, VM session and firmware interfaces remain unchanged. |
| Applicable Rules | Execution, coding, architecture, source-policy and documentation authorities from the Task Reading Set; preserve Core ownership of physical routing and VM ownership of profile selection. |
| Verification | Build and run Model-40 ROM-map, KBC A20, generic RAM-port focused tests; run bounded formal-asset capture only through its existing contained helper; then current-source gate, documentation governance and diff review. |
| Expected Markers | `M5:T390:S31:MODEL40-A20-POLICY:OK`, existing Model-40 ROM-map markers, generic RAM-port marker, and contained capture summary. |
| Asset Needs | Read-only owner-managed formal ROM asset; no import, hash, path, bytes, trace or guest media may enter Git. |
| Reporting Requirements | Record route semantics, focused and bounded outcomes, interface-boundary review, similar-issue sweep, and any transfer without raw protected data. |
| Stop Conditions | Stop for any failure requiring a changed board contract, protected asset import, generic-policy regression, or a materially broader interface/architecture change. |
| Exit Criteria | Model-40 A20-off high aliases and second-MiB mirror prove correctly; generic global mask remains covered; bounded replay is recorded; required gates and actual-diff review pass. |
| Original Owner Request | Continue CPU/DeskPro audit holistically, compare PCjs/86Box read-only where useful, bridge experimentally when available, fix confirmed defects, and preserve Core/VM boundaries. |
| Similar-Issue Sweep | Search all Core A20 wrapping call sites and Model-40 configuration constructors; classify every production hit and retain focused regression coverage for generic and Model-40 policies. |


## Current Technical Baseline

- **Current developer artifact:** T390 S22 `vm-0-5-0390` /
  `build/output/nxvm_0_5_0390.exe`, SHA-256
  `7CD172472BADEEE78160548EDE93104C94874CE03F25DDA2392A63E1217FE2C5`.
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
| T390 S30 | Accepted: corrected-ROM aggregate replay remains pre-protected and does not reach C1 within containment; synthetic C0-to-C1 coverage remains. Read-only PCjs/86Box analysis confirms useful behavioral leads but no locally runnable dynamic bridge exists. Two stale test contracts were repaired without changing Core or VM production interfaces. [Evidence](../etc/evidence/t390-s30-corrected-rom-c1-baseline.md). |
| T389 | Stopped by owner-approved requeue: S1 proves no physical board slice can begin while 80386 successful retirement remains nonphysical; S2 archives the invalid candidate and queues a shared CPU qualification before a fresh board task. No board timing, physical-clock or L3 claim. [Audit](../etc/evidence/t389-s2-stop-and-requeue-audit.md). |
| T388 | Closed: S1--S9 establish the sole publisher, exhaustive exact-or-nonphysical residual disposition and pre-publication physical-contract rejection. No current CPU profile is physically clock-qualified; board work transfers under this restriction. [Closure audit](../etc/evidence/t388-s9-task-closure-audit.md). |
| T387 | Closed: source/owner inventory, exact port-`61h` reconciliation and reference qualification transfer the shared physical-timebase then physical-device/firmware prerequisites before any DeskPro L3 audit. No physical-time or L3 claim. [Closure audit](../etc/evidence/t387-s6-task-closure-transfer-audit.md). |
| T386 | Closed: S29 independently reconciles every selected Model-40 functional owner/proof, corrects the omitted S26 HDC current-gate registration and transfers only board, firmware and physical-media work to the next candidate/TODO. No L3 claim. [Closure audit](../etc/evidence/t386-s29-functional-closure-audit.md). |
| T385 | Closed: the selected Model-40 80386DX-16 consumes the shared 80386 CPU/state/retirement owners without a DeskPro CPU fork; a clean current-source 251/251 gate passed, while functionality and board timing transfer unchanged. [Closure audit](../etc/evidence/t385-s1-task-closure-audit.md). |
| T384 | Closed: the Model-40 capability audit freezes its original Compaq system board/storage ledger and owner-selected color EGA, keyboard and external-ROM constraint; its shared-VADP IBM/Compaq personality boundary transfers CPU, functional and timing work without making a runnable or L3 claim. [Closure audit](../etc/evidence/t384-s2-task-closure-audit.md). |
| T383 | Closed: independent current-source replay accepts the frozen Model-339 deterministic L3 contract after T380's strict-start repair; 251/251 current gate and focused matrix are retained in [evidence](../etc/evidence/t383-s1-model339-current-source-reaudit.md). |
| T382 | Reclosed at S8: the prior S1--S7 closure is retained historically, while S8 corrects its Console 64-bit transport and final artifact baseline. [Corrective audit](../etc/evidence/t382-s8-reclosure-audit.md). |

## Recent Governance

- **M5 Td S103 P1:** adopted task-scale convergence and coverage governance: coverage-bearing T work requires a durable universe, batch dispositions, completion predicate and coordinator reconciliation before whole-domain or L3 acceptance.
- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
