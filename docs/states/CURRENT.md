# Project Status

## Current Work

**Active packet:** T390 S9 (Continuation) -- 80386 real-address far-indirect JMP timing qualification.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation -- T390 is the latest open numeric task; S9 follows accepted S8. |
| Admission And Approval | Owner approved continuation in this conversation on 2026-08-17. The owner-approved root-cause continuation now covers the observed 80386 CS-overridden `FF /5` far-indirect JMP and its exact real-address and successful protected same-privilege direct-code rows at the existing shared Core owner. No exception is approved. |
| Objective | Reconcile the exact Intel 80386 PRM `FF /5` `JMP m16:16` rows reached by the observed CS-overridden form: real-address `43+m` and successful protected same-privilege direct-code `31+m`. Prove their `m` next-instruction lexical term through the existing shared preview owner, while retaining gate/task/other privilege routes nonphysical. |
| Non-goals | No VM86, call-gate, task-switch, privilege-changing or other protected far-transfer qualification; no broad prefix policy change; no decoder/execution semantics, ROM/media, profile selection, board clock, device timing, or L3 claim. |
| Reference Baseline | Accepted T390 S8 artifact `vm-0-5-0390`, SHA-256 `846506BC1A43D4F2BEDFEF3FFCB2595ADEDF74E359B4FB590F055036BAA3495E`; Intel 80386 PRM, instruction `JMP` page 318 and clocks definition page 248; current Core control/stack classifier and T390 S8 evidence. |
| Candidate Proposal | [80386 physical-retirement qualification](../proposals/m5-80386-physical-retirement-qualification.md). |
| Files And ABI Surface | Private Core implementation/ledger in `src/core/machine/machine.c`; focused Core timing-ledger smoke; T390 evidence/history/status/index as needed. No public ABI or VM/profile change. |
| Applicable Rules | Execution, documentation, architecture and coding rules; source policy. Invariants: one shared Core timing owner; exact Intel-primary row; immutable external assets; physical-contract failure before publication for any excluded context; tests do not create a public test-only API. |
| Verification | Focused timing-ledger smoke proves the CS-overridden memory far JMP real-address `43+m` and successful protected same-privilege direct-code `31+m` rows with known target lexemes; a gate/other protected route remains rejected prepublication; similar `FF /3` and direct far-JMP rows are swept; normal bounded redacted BYOB replay advances or records its next terminal; build, current gates, documentation governance, `git diff --check`, and coordinator actual-diff review pass. |
| Expected Markers | Existing timing-ledger smoke success marker; existing bounded `M5:T390:S8:BYOB-BOOT-CAPTURE` output or its successor only if the runner changes; S9 evidence references source citation and final artifact digest. |
| Asset Needs | Optional owner-managed external ROM and boot media only for bounded replay; no media, ROM bytes, identity, path, hash, raw trace or terminal bytes enter Git. |
| Reporting Requirements | P1 is rejected by coordinator review: P2 must repair the discovered documentation bytes/formatting and add a project-owned positive protected same-privilege direct-code regression; it then reports focused/full verification and returns for acceptance. |
| Stop Conditions | Stop and transfer if the exact source row or required preview context is unavailable, if the form fails functionally, if a wider decoder/state mechanism is necessary, or if replay reaches a new unallocated success. Do not substitute an emulator count or enable a partial physical profile. |
| Exit Criteria | The exact real-address and successful protected same-privilege direct-code memory far-JMP rows and next-term semantics are source-cited, one Core owner produces their expected ticks for the observed prefix form, gate/task/other protected routes stay nonphysical, the similar-form sweep is recorded, and required verification/evidence/acceptance are complete. |
| Original Owner Request | Continue the Queue-ordered 80386 physical-retirement qualification before returning to DeskPro board timing; user additionally required Intel manual authority for 80386 instruction behavior. |
| Similar-Issue Sweep | Inspect all existing 80386 far CALL/JMP direct/memory ledger entries and source consumers, all accepted segment-prefix routes through the control/stack owner, and existing evidence that states the affected row; repair only exact in-scope current ownership and record historical discrepancy without rewriting accepted history. |

## Current Technical Baseline

- **Current developer artifact:** T390 S9 `vm-0-5-0390` /
  `build/output/nxvm_0_5_0390.exe`, SHA-256
  `F20E1C1B8398B5D915C60FBBD31C304E7A330C2DA65E748FE57DF8E841EA97C4`.
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
| T390 S8 | Accepted: P1/P2 qualify real-address memory-source `MOV Sreg,r/m16` at 5 Core ticks and directly prove protected-mode physical rejection. A bounded redacted replay reaches 18,228 successful retirements and transfers its next terminal; full boot corpus, physical-profile selection, board timing and L3 remain T390 work. [Evidence](../etc/evidence/t390-s8-mov-sreg-memory-qualification.md). |
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
