# Project Status

## Current Work

**Active.** T390 remains open between accepted subtasks. S28 selects the existing physical-retirement contract only for the complete accepted Model-40 C0 batch, retaining prepublication fault behavior for later unqualified successes. [Evidence](../etc/evidence/t390-s28-model40-physical-contract-selection.md).


## M5 T390 S29 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner redirect on 2026-08-17: investigate the original DeskPro 386 ROM decode before further C0-to-C1 work; directly repair it if the evidence confirms a defect. The owner retains standing authorization for this active T390 work. |
| Objective | Correct the Model-40 Rev-E external-ROM decode from the erroneous E-segment repeated window to the observed DeskPro 386 low-ROM placement and required reset aliases. |
| Non-goals | No ROM/media import, vendor bytes, asset identity/path/hash/provenance in Git, firmware default dependency, board-clock result, C1 timing qualification, public ABI, or L3 claim. |
| Reference Baseline | T390 proposal; accepted S16 D4-map ownership; owner-authorized external research archive; PCjs Rev-E machine configuration and public DeskPro reset-mapping explanation; 86Box used only as a divergent later-revision behavior reference. |
| Candidate Proposal | [80386 physical-retirement qualification](../proposals/m5-80386-physical-retirement-qualification.md). |
| Files And ABI Surface | Model-40 private profile ROM configuration, the private core memory-provider capacity required for its real alias count, the existing D4 mapping smoke, an indexed neutral evidence note and this packet; no public interface or asset change. |
| Applicable Rules | VM profile owns board-local ROM placement; core firmware owns immutable mapping and explicit aliases; core private provider capacity must admit the declared profile routes; D4 replacement must not be hidden by a fabricated ROM mirror; third-party firmware and external source remain research-only. |
| Verification | Project-owned synthetic even/odd chips prove the low mapping, F-segment compatibility alias, high reset aliases, retained D4 protection behavior, and writable E-segment absence of an invented ROM mirror; focused and full current gates, documentation gate, diff review and P1/P2 review. |
| Expected Markers | The existing Model-40 D4 ROM-map smoke remains green while asserting the corrected physical placements and aliases. |
| Asset Needs | Formal owner-managed assets may be read only for non-retained confirmation; no asset data, path, digest, byte, trace, PC, or provenance enters Git. |
| Reporting Requirements | Record the neutral mapping evidence, exact removed fabricated aliases, retained aliases, D4 interaction, similar-issue sweep, focused/full verification and P1/P2 review. |
| Stop Conditions | The sources disagree on Rev-E decode without a reproducible owned probe, the correction needs a core/public-interface change, an alias conflicts with an existing immutable route, or the work would require retained vendor material: stop and transfer rather than infer. |
| Exit Criteria | Owned regression proves exactly the documented low and reset alias surface, proves E-segment writes are no longer blocked by the ROM, and all affected current gates pass. |
| Original Owner Request | Execute the current task: globally audit and repair CPU instruction defects, errors and timing in preparation for DeskPro 386 board timing and final L3 closure; prioritize original DeskPro 386 ROM mapping and repair confirmed defects immediately. |
| Similar-Issue Sweep | Sweep every Model-40 ROM-window constant, materialization loop, firmware alias registration and D4-map assertion; dispose every hit within this mapping boundary or explicitly transfer it. |
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
| T390 S28 | Accepted: both Model-40 composition paths select the existing physical-retirement contract after the complete C0 batch; unqualified successes remain prepublication-rejected. The retained C0 replay reaches 75 parents / 18,255 successes, with no board or L3 claim. [Evidence](../etc/evidence/t390-s28-model40-physical-contract-selection.md). |
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
