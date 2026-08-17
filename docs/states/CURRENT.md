# Project Status

## Current Work

**Active.** M5 T390 S8 is admitted as a `Continuation` task packet.

| Field | Required record |
| --- | --- |
| Identifier Mode | `Continuation`; M5 T390 S8 after accepted S7. |
| Admission And Approval | Owner-approved autonomous Queue execution on 2026-08-17 with standing write/commit/push authorization. Scope is only S7's earliest transferred real-address memory-source `MOV Sreg,r/m16`. |
| Objective | Qualify the observed 80386 real-address memory-source `MOV Sreg,r/m16` at its Intel-primary exact Core-clock row, preserve physical prepublication for excluded contexts, and repeat the bounded redacted Model-40 replay. |
| Non-goals | No protected-mode `pm=18/19` forms, other segment destinations, prefixes, board/device/firmware timing, profile selection, boot completion, `0:7C00`, ROM/media import, or L3 claim. |
| Reference Baseline | Accepted S7 P2 `fb509c6f`, S7 evidence, and retained T390 history. Intel 80386 Programmer's Reference Manual (1986), MOV table page 344: `8E /r MOV Sreg,r/m16` is `2/5, pm=18/19`; S8 admits only the observed real-address memory value `5`. |
| Candidate Proposal | `docs/proposals/m5-80386-physical-retirement-qualification.md`; this remains a bounded shared-Core corpus qualification. |
| Files And ABI Surface | Private Core timing ledger/classifier, existing timing-ledger smoke, external-only capture runner, evidence/history/Current. No public ABI, VM/profile interface, or device change. |
| Applicable Rules | Execution: active packet, complete P1, actual review/P2 and similar-form sweep. Architecture: sole Core owner and physical eligibility only for exact source-backed context. Coding: C11 minimal owner-local change/no test API. Document and source policy: indexed evidence; owner media stays external and redacted. |
| Verification | Primary citation; normal and physical exact 5-tick proof; the existing register form remains 2 ticks; prefixed or protected contexts remain nonphysical; focused smoke, product/current-gate build, documentation gate, diff hygiene, and bounded default external replay with output cleanup. |
| Expected Markers | Existing timing-ledger marker; `M5:T390:S8:MOV-SREG-MEMORY:OK`; redacted replay transfers only its next earliest terminal. |
| Asset Needs | Existing owner-managed ROM/media only for external replay; never retain identity, bytes, paths, hashes, provenance, or trace. |
| Reporting Requirements | Executor confirms, then P1 push with evidence/transfer; coordinator actual review then P2 or one consolidated corrective brief. |
| Stop Conditions | Stop/transfer if context differs, exact row is unavailable, a broader CPU mechanism is needed, replay reaches a new unqualified terminal, source containment fails, or any required gate fails. |
| Exit Criteria | Exact real-address memory source uses 5 Core ticks; existing register form remains 2; prefixed and protected forms remain rejected prior to publication; replay is redacted/bounded and next terminal transferred; required verification and P2 acceptance pass. |
| Original Owner Request | Continue the Queue-governed DeskPro 386 completeness/L3 work under single-person dual-role governance and use Intel primary material for exact timing. |
| Similar-Issue Sweep | Search 80386 `0x8e` enum/ledger/classifier/tests and 80286 distinct owner. Qualify only current observed real-address memory form; existing 80386 register owner remains 2; protected `18/19` and prefixed forms are excluded and must retain rejection. |

## Current Technical Baseline

- **Current developer artifact:** T390 S7 `vm-0-5-0390` /
  `build/output/nxvm_0_5_0390.exe`, SHA-256
  `7E6E458D3C70EB6B25D8A6DBAD0085C5E7D6F7C0653316E38B0C373D5AD99A7B`.
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
| T390 S7 | Accepted: P1 qualifies real-mode no-prefix `CLD` at 2 Core ticks and proves prefixed physical rejection. A bounded redacted replay reaches 426 successful retirements and transfers memory-source `MOV Sreg,r/m16`; full boot corpus, physical-profile selection, board timing and L3 remain T390 work. [Evidence](../etc/evidence/t390-s7-cld-qualification.md). |
| T389 | Stopped by owner-approved requeue: S1 proves no physical board slice can begin while 80386 successful retirement remains nonphysical; S2 archives the invalid candidate and queues a shared CPU qualification before a fresh board task. No board timing, physical-clock or L3 claim. [Audit](../etc/evidence/t389-s2-stop-and-requeue-audit.md). |
| T388 | Closed: S1--S9 establish the sole publisher, exhaustive exact-or-nonphysical residual disposition and pre-publication physical-contract rejection. No current CPU profile is physically clock-qualified; board work transfers under this restriction. [Closure audit](../etc/evidence/t388-s9-task-closure-audit.md). |
| T387 | Closed: source/owner inventory, exact port-`61h` reconciliation and reference qualification transfer the shared physical-timebase then physical-device/firmware prerequisites before any DeskPro L3 audit. No physical-time or L3 claim. [Closure audit](../etc/evidence/t387-s6-task-closure-transfer-audit.md). |
| T386 | Closed: S29 independently reconciles every selected Model-40 functional owner/proof, corrects the omitted S26 HDC current-gate registration and transfers only board, firmware and physical-media work to the next candidate/TODO. No L3 claim. [Closure audit](../etc/evidence/t386-s29-functional-closure-audit.md). |
| T385 | Closed: the selected Model-40 80386DX-16 consumes the shared 80386 CPU/state/retirement owners without a DeskPro CPU fork; a clean current-source 251/251 gate passed, while functionality and board timing transfer unchanged. [Closure audit](../etc/evidence/t385-s1-task-closure-audit.md). |
| T384 | Closed: the Model-40 capability audit freezes its original Compaq system board/storage ledger and owner-selected color EGA, keyboard and external-ROM constraint; its shared-VADP IBM/Compaq personality boundary transfers CPU, functional and timing work without making a runnable or L3 claim. [Closure audit](../etc/evidence/t384-s2-task-closure-audit.md). |
| T383 | Closed: independent current-source replay accepts the frozen Model-339 deterministic L3 contract after T380's strict-start repair; 251/251 current gate and focused matrix are retained in [evidence](../etc/evidence/t383-s1-model339-current-source-reaudit.md). |
| T382 | Reclosed at S8: the prior S1--S7 closure is retained historically, while S8 corrects its Console 64-bit transport and final artifact baseline. [Corrective audit](../etc/evidence/t382-s8-reclosure-audit.md). |

## Recent Governance

- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
