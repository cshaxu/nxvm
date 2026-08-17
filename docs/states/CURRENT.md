# Project Status

## Current Work

**Active.** M5 T390 S7 is admitted as a `Continuation` task packet.

| Field | Required record |
| --- | --- |
| Identifier Mode | `Continuation`; M5 T390 S7, following accepted S6. |
| Admission And Approval | The owner approved continuation of the queue plan on 2026-08-17 and permanently authorizes repository writes, commits and pushes. This packet admits only the earliest S6 transferred terminal: unprefixed real-address `CLD`. |
| Objective | Qualify the observed 80386 `CLD` form at its Intel-primary exact Core-clock row, preserve the physical prepublication boundary, and repeat the bounded owner-managed Model-40 boot-media capture to transfer its next earliest terminal truthfully. |
| Non-goals | No board ratio, physical-profile selection, device, firmware, ROM/media, full-boot, `0:7C00`, Model-40 L3, other flag-control opcode, prefixed form, protected/VM86 context, or generic 80386-completeness claim. No external asset, path, hash, provenance, raw byte record or trace enters Git. |
| Reference Baseline | T390 S6 acceptance `57d0a4a0`; T390 proposal `../proposals/m5-80386-physical-retirement-qualification.md`; S6 evidence `../etc/evidence/t390-s6-sal-register-one.md`; retained history `../history/M5-T390-80386-physical-retirement-qualification.md`. Intel 80386 Programmer's Reference Manual (1986), CLD entry, page 283, opcode `FC`, `2` clocks. |
| Candidate Proposal | `docs/proposals/m5-80386-physical-retirement-qualification.md`; this is the proposal's bounded actual-corpus Core qualification path, not a new board or profile candidate. |
| Files And ABI Surface | Private Core source-timing ledger/classifier in `src/core/machine/machine.c`; existing internal timing-ledger smoke; external-only Model-40 capture runner; indexed evidence, history and Current status. No public ABI, VM/profile contract, Core observer ABI or device interface change. |
| Applicable Rules | `docs/rules/EXECUTION.md`: active packet, single S, complete pushed P1, actual-change review, evidence, similar-issue sweep and acceptance P2. `docs/rules/ARCHITECTURE.md`: one Core timing owner, physical eligibility only after exact source row, no external asset dependency. `docs/rules/CODING.md`: C11, owner-local minimal change and no test-only production contract. `docs/rules/DOCUMENT.md`: Current is sole active authority and evidence stays indexed. `docs/etc/operations/policy/source-policy.md`: owner-managed firmware/media and raw diagnostics stay external. |
| Verification | Cite the primary-manual `CLD` row; add exact normal and physical-boundary regression (2 ticks, one execution-provider advance; prefixed counterpart remains nonphysical); run focused smoke, product build, full current gates, documentation governance and `git diff --check`; run the existing contained default external replay with normal output redaction, bounded process/output and cleanup. |
| Expected Markers | Existing timing-ledger success marker; evidence marker `M5:T390:S7:CLD:OK`; default external replay has `raw-lines=0` and transfers only the next normalized earliest terminal, if reached. |
| Asset Needs | Existing owner-managed external Model-40 ROM pair and 1.2 MB boot media only for the explicitly local replay. They are neither copied nor identified in repository files. |
| Reporting Requirements | Report executor confirmation, after P1 the pushed commit plus focused/full verification and transfer, then coordinator actual-change review and P2 acceptance or a consolidated corrective brief. |
| Stop Conditions | Stop and transfer if primary material lacks the exact `CLD` row, observed context differs from unprefixed real-address `CLD`, the change requires a broader CPU mechanism, capture reaches a new unqualified terminal, physical eligibility changes outside the exact form, source-policy containment cannot be maintained, or a required gate fails. |
| Exit Criteria | One exact unprefixed 80386 `CLD` Core row at 2 ticks; no-prefix classifier and focused normal/physical proofs; prefixed `CLD` remains rejected before elapsed/device publication; default replay remains redacted and bounded, captures next terminal or records no further progress; no external content in Git; all required gates and actual-change acceptance pass. |
| Original Owner Request | Continue queue-directed DeskPro 386 work autonomously under single-person dual-role governance until L3 closure, with Intel manuals primary and external ROM/media local only. |
| Similar-Issue Sweep | Defect class: observed 80386 no-prefix flag-control successful form has no exact source-timing disposition. Search the full tracked Core timing enum, 80386 ledger/classifier, timing smoke and existing 80286 flag cases for `0xfc`, `CLD`, and adjacent flag-control forms. The sole observed 80386 hit is qualified here; adjacent 80386 opcodes are outside this S and remain source-unallocated unless already ledger-owned; 80286 is a distinct CPU ledger and is not changed. The focused prefixed negative proves the prepublication boundary. |

## Current Technical Baseline

- **Current developer artifact:** T390 S6 `vm-0-5-0390` /
  `build/output/nxvm_0_5_0390.exe`, SHA-256
  `55C94D369708ADBDA0B779E108539546D8C2124FC34031362F1A71D59F88A5A9`.
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
| T390 S6 | Accepted: P1 adds an opt-in terminal-only external diagnostic, qualifies real-mode register `SAL r/m8,1` at 3 Core ticks and proves memory-form physical rejection. A contained replay reaches the next `CLD` terminal at 122 retirements; full boot corpus, physical-profile selection, board timing and L3 remain T390 work. [Evidence](../etc/evidence/t390-s6-sal-register-one.md). |
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
