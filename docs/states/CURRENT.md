# Project Status

## Current Work

**Active: M5 T437 S8.** The owner-approved final continuation closes the
complete 80386DX result set and task-level audit.

## M5 T437 S8 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; M5 T437 S8 follows accepted S7 in the latest open numeric task. |
| Admission And Approval | The owner directed continuous complete 80386DX implementation on 2026-08-22. Accepted S1 assigns final result publication, regression, obsolete-route disposition and task audit to S8. |
| Objective | Publish and prove the complete 1,410-key 80386DX result set, preserving the manual's separate MCP clock domain for ESC while every scalar CPU key remains an actual retirement observation. |
| Non-goals | Do not reopen accepted S3--S7 instruction semantics or replace the CPU/FPU handoff with x87 arithmetic emulation; do not assign board or host timing, alter public ABI, import source or fabricate CPU clocks. |
| Reference Baseline | [S1 partition](../etc/cpu-timing/t437-s1-80386-key-partition.md), [S2 producer](../etc/cpu-timing/t437-s2-80386-result-producer.md), [80386DX ledger](../etc/cpu-timing/t435-s1-80386-ledger.md), [manifest](../etc/cpu-timing/t435-s2-80386-timing-manifest.json), [audit](../etc/cpu-timing/t435-s2-80386-implementation-audit.md), [result contract](../etc/cpu-timing/t435-s2-manifest-result-contract.md), [S3 x87 contract](../etc/cpu-timing/t437-s3-x87-interface-contract.md), [S7 evidence](../etc/cpu-timing/t437-s7-protected-system-execution.md), and the retained Intel 80386DX / 80387 manuals. |
| Candidate Proposal | [M5 80386DX instruction timing closure](../proposals/m5-80386dx-instruction-timing-closure.md). |
| Files And ABI Surface | The private 80386DX manifest runner, shared result verifier/contract, result artifact, S8 evidence and CURRENT only. No public header, external ABI, device, board or artifact identity change. |
| Applicable Rules | Execution: bounded continuation/P lifecycle, dual-role review, actual-change review and similar-issue sweep. Architecture: one Core CPU/port/memory transaction and retirement publisher, with no board policy; the existing CPU/FPU transaction remains its single handoff owner. Coding: C11 cohesive private helpers and no parallel selector. Documentation: CURRENT is the active contract. Source: no import. |
| Verification | Build/run the 80386DX manifest runner; require 1,409 classified CPU retirement records plus one verified ESC MCP-domain handoff; run `Verify-CpuTimingResults.ps1` for the result, the partition and documentation gates, and relevant decoder/prefix/cross-profile regressions. |
| Expected Markers | Retain S1--S7 markers; add S8 result-closure, ESC-domain and task-audit markers before acceptance. |
| Asset Needs | Repository fixtures and retained 80386DX manual evidence only; no firmware, guest media, source download, Microsoft component or third-party code. |
| Reporting Requirements | Record final result shape, ESC-domain evidence, code delta, focused proof, actual changed files, cross-profile disposition and task closure audit. The executor delivers one complete P and the same session then performs actual-change review. |
| Stop Conditions | Stop for manual/manifest/contract contradiction, unrepresentable ESC-domain result, required public ABI or board timing, source-policy issue, or verifier contradiction. Do not guess clocks, omit a key or retain a generic successful fallback. |
| Exit Criteria | The writer emits exactly 1,410 legal keys: 1,409 real classified CPU observations and one verified ESC MCP-domain handoff with no scalar CPU clock claim; result and all applicable gates pass; task-level audit and dual-role review accept the complete implementation. |
| Original Owner Request | Owner request, faithfully translated: continuously and completely implement the correct 80386 instruction function and timing model, under the single-person dual-role governance lifecycle. |
| Similar-Issue Sweep | Defect class: a completed 80386 legal key that cannot be truthfully represented by the final result contract, including non-CPU clock domains, missing/incorrect ownership, result capture or unallocated fallback. Scope: all 1,410 manifest keys, shared result verifier, sole timing publisher, CPU/FPU handoff and adjacent profile result records; non-80386 changes are limited to contract compatibility and otherwise transfer through their own task. |
## Current Technical Baseline

- **Current developer artifact:** target `vm-0-5-0434`; `nxvm_0_5_0434.exe` / `build/output/nxvm_0_5_0434.exe`, SHA-256 `F8562F4623D53303470408837249CF89C985E0A0B1130212BBD9CE26B5E1B38A`. T434 has one copied Core timing-plan publication route for default PC/AT, IBM 5170 Model 339 and Model-40 BYOB session composition.
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
| T437 S7 | Accepted: all 115 protected segment, descriptor, system and special-register keys are classified through the sole retirement publisher; [S7 evidence](../etc/cpu-timing/t437-s7-protected-system-execution.md) retains real protected descriptors and manual-first corrections. |
| T437 S6 | Accepted: all 89 protected, privilege, gate, task and VM86 control-transfer keys are classified through the sole retirement publisher; [S6 evidence](../etc/cpu-timing/t437-s6-protected-control-execution.md) retains the successful-path fixtures and manual corrections. |
| T437 S5 | Accepted: 234 corrected ordinary-control 80386DX keys retire with classified non-unallocated results; the two omitted stack-memory segment contexts are executed, while protected/VM86/task paths remain S6. [S5 evidence](../etc/cpu-timing/t437-s5-ordinary-control-execution.md) retains the real-mode and formula inputs. |
| T437 S4 | Accepted: all 162 direct-I/O and primitive/REP-string 80386DX keys retire with classified non-unallocated results, including real segment and repeat-phase contexts; [S4 evidence](../etc/cpu-timing/t437-s4-string-io-execution.md) retains the direct-port and continuation proof. |
| T437 S3 | Accepted: 809 ordinary scalar/formula 80386DX keys retire with classified results, including SETcc; ESC is separately verified as the cross-profile CPU/FPU handoff, BUSY/WAIT, transaction and 80387-range contract in [S3 x87 evidence](../etc/cpu-timing/t437-s3-x87-interface-contract.md). |
| T437 S2 | Closed: the private 80386DX [result producer](../etc/cpu-timing/t437-s2-80386-result-producer.md) captures real retirement metadata and refuses final emission until all 1,410 legal keys exist; S3--S7 retain the missing recipes. |
| T436 | Closed: [result closure](../etc/cpu-timing/t436-s8-80286-result-closure.md) proves the verified 771-key result set, 8086/80186/80286/80386 regression disposition, current-gate completion and developer artifact. |
| T435 | Stopped by owner-approved requeue; not completed. Its profile-specific 80286 and 80386DX closure candidates now lead the Queue. [Record](../history/M5-T435-core-cpu-instruction-timing-program-requeue.md). |
| T434 | Closed: all 30 copied plan declarations/dispositions, every current VM materializer and atomic Core publication route are reconciled; full task closure proof is retained in the [S3 audit](../etc/evidence/t434-s3-task-closure-audit.md) and [task history](../history/M5-T434-core-timing-contract-machine-plan.md). |
| T433 | Closed after corrective S7: 30 frozen Core capabilities have a source-sufficiency and ownership disposition; chip/manual semantics are separated from VM-profile inputs, host/product boundaries and the explicit ATA/HDC source block. [S7 ledger](../etc/evidence/t433-s7-core-source-sufficiency-ledger.md). |
| T432 | Closed: separate Core-private Set-2 translation and typematic break states prevent released keys from restarting typematic; focused regression, Model-339 contract and owner Console/Window verification pass. [Record](../history/M5-T432-keyboard-keyup-reliability-repair.md). |
| T431 | Closed: Model-40 selects a reference-derived unready-drive READ `ST0=48h`/IRQ/result completion through one Core policy; generic PC/AT stays generic, and physical FDC/L3 timing remains transferred. [Evidence](../etc/evidence/t431-s1-deskpro-fdc-not-ready-reference.md). |
| T430 | Closed: original/reference-backed Compaq WD 40 MB C:/D: selection now uses frozen Core media slots with optional Model-40 secondary backing; generic ATA stays unchanged, and physical/L3 timing remains transferred. [Evidence](../etc/evidence/t430-s1-deskpro-dual-fixed-disk.md). |


## Recent Governance

- **M5 Td S122 P1:** decomposed the queued 80286 timing closure by observable timing and architectural path: coverage freeze, result-contract publication, non-control forms, strings, ordinary and protected control transfer, protected system forms, then full closure. No numeric task, source, artifact, ABI or runtime behavior changed.

- **M5 Td S121 P1:** bound the queued CPU timing program to complete 8086/80186 evidence/model research before implementation audit, then finite repair and closure; manual/reference-derived models are labelled L2, only an exhausted ladder may declare L1, and no emulator model lowers the L3 source threshold. T435 remains unallocated.

- **M5 Td S120 P1:** changed code-size restraint from a hard numerical admission/closure gate to a soft concise-design goal; actual-delta reporting, explanation and obsolete-path retirement/disposition remain mandatory. T434 remains unallocated.

- **M5 Td S119 P1:** compressed Td S118's code-size/cleanup rule without weakening its baseline, default non-positive budget, prior numeric approval, final reconciliation, retirement or closure-block requirements. T434 remains unallocated.

- **M5 Td S118 P1:** made a reproducible code-size baseline, default non-positive net code, owner-approved positive cap, obsolete-path disposition and actual-delta report mandatory for every code-changing S. T434 remains unallocated.

- **M5 Td S117 P1:** froze T434's complete 30-ID seam/default-disposition ledger, three current VM session materialization families and two bounded implementation batches; it requires one atomic plan-only publication cutover and records architecture/coding proof as a task-local closure barrier. T434 remains unallocated.

- **M5 Td S116 P1:** made default-plan equivalence concrete and prohibits T434 closure when any public capability is undisposed, any production path bypasses the plan, or plan semantics conflict with Core/VM ownership; T434 remains unallocated.

- **M5 Td S115 P1:** made the Core timing-plan candidate require one production publication path, all current consumer seams, explicit runnable L2 dispositions, atomic invalid-plan/required-L3 rejection, and no audit-only handoff; T434 remains unallocated.

- **M5 Td S114 P1:** established the mandatory three-level hardware implementation ledger for every M5 hardware candidate: document-primary complete implementation first, reproducible mature-emulator reference contract second, then deterministic explicit maintained boundary; no Rules, runtime or source import changed.

- **M5 Td S113 P1:** linked every Core L3 proposal directly to the shared timing design and its exact finite admission batch; the five earliest receiver batches cover all 30 frozen Core capability IDs once, without changing Queue order, runtime or architecture authority.

- **M5 Td S112 P1:** replaced pre-Windows Queue positions 7-10 with PC/AT 5170 root normalization, DeskPro child convergence, AT resolver/default-at migration, and YAML cutover; each has a proposal. The VM design now makes the current AT tree direct under pc-at-5170; XT remains deferred.

- **M5 Td S111 P1:** reordered all pre-Windows M5 candidates: six Core L3 tasks, 5170-derived runnable DeskPro, VM profile foundation, retained XT closure, then Windows; withdrew four stale candidates while preserving their evidence links.


- **M5 Td S110 P1:** recorded the target VM profile resolver and copied Core machine-plan design: 5160 and 5170 are the two roots; YAML has one built-in profile selector, no variant or compatibility-version layer; 486/P5 is deferred. No runtime, Queue, proposal, or ABI changed.

- **M5 Td S109 P1:** made Project timing levels explicit: L1 is ordered functional boot without timing guarantee; L2 is implemented macro-scale proportional timing; L3 is the current documentation-driven constant/formula/range target; L4 hardware-real timing is prohibited from implementation.

- **M5 Td S108 P1:** established Project A's Core specification-driven instruction and transaction timing design: L1--L4 vocabulary, admitted capability universe, explicit L2 exception ledger, and a neutral Core-to-VM timing-contract boundary; no runtime or Queue change.

- **M5 Td S107 P1:** retired the completed tiered-fallback foundation from the active Queue, promoted the DeskPro physical-cycle program, and ordered CPU-to-board timing, device phases, Compaq fixed disk, CECG, BYOB lifecycle, and independent L3 re-audit with retained evidence tiers.

- **M5 Td S106 P1:** corrected the retained PCjs DeskPro HDC configuration fact to two generic AT Type-1 10.16 MB drives; the Compaq WD 40 MB non-qualification and all physical receivers remain unchanged.
