# Project Status

## Current Work

**M5 T435 retains accepted S8 progress; no subtask is active.** S8 closed the
owner-requested 80386DX manual/decoder/S1/S2 audit before timing
implementation. S9 may be admitted only when the owner starts the 80186
implementation receiver defined in the approved T435 proposal.

| Field | Required record |
| --- | --- |
| Latest Accepted Subtask | `M5 T435 S8`, accepted after single-person dual-role review, 2026-08-21. |
| Closure Result | 450 source-located L3 base forms, 961 legal contexts and 1,411 canonical nonconforming keys are mechanically reconciled against 253 primary and 66 `0F` decoder families. |
| Evidence | [decoder audit](../etc/cpu-timing/t435-s8-80386-decoder-ledger-audit.md), [reconciliation contract](../etc/cpu-timing/t435-s8-80386-decoder-reconciliation-contract.md), `Verify-80386DecoderLedger.ps1`, profile inventory runner and manifest contract. |
| Transfer | S11 receives the entire 1,411-key 80386DX implementation universe; S9 is the ordered 80186 implementation receiver. No runtime behavior or ABI changed in S8. |
## Current Technical Baseline

- **Current developer artifact:** target `vm-0-5-0434`; T434 S2 `nxvm_0_5_0434.exe` / `build/output/nxvm_0_5_0434.exe`, SHA-256 `0252F8FDA17BEC2131606F19E3547B46894AC6B56DD37EC3B16BD302494FAFDC`. T434 has one copied Core timing-plan publication route for default PC/AT, IBM 5170 Model 339 and Model-40 BYOB session composition.
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
| T435 S8 | Closed: [S8 audit](../etc/cpu-timing/t435-s8-80386-decoder-ledger-audit.md) and its executable verifier reconcile every 80386DX S1/S2 key with the current decoder universe: 450 L3 base forms, 961 legal contexts and 1,411 canonical nonconforming dispositions. S11 receives the complete 80386DX implementation batch. |
| T434 | Closed: all 30 copied plan declarations/dispositions, every current VM materializer and atomic Core publication route are reconciled; full task closure proof is retained in the [S3 audit](../etc/evidence/t434-s3-task-closure-audit.md) and [task history](../history/M5-T434-core-timing-contract-machine-plan.md). |
| T433 | Closed after corrective S7: 30 frozen Core capabilities have a source-sufficiency and ownership disposition; chip/manual semantics are separated from VM-profile inputs, host/product boundaries and the explicit ATA/HDC source block. [S7 ledger](../etc/evidence/t433-s7-core-source-sufficiency-ledger.md). |
| T432 | Closed: separate Core-private Set-2 translation and typematic break states prevent released keys from restarting typematic; focused regression, Model-339 contract and owner Console/Window verification pass. [Record](../history/M5-T432-keyboard-keyup-reliability-repair.md). |
| T431 | Closed: Model-40 selects a reference-derived unready-drive READ `ST0=48h`/IRQ/result completion through one Core policy; generic PC/AT stays generic, and physical FDC/L3 timing remains transferred. [Evidence](../etc/evidence/t431-s1-deskpro-fdc-not-ready-reference.md). |
| T430 | Closed: original/reference-backed Compaq WD 40 MB C:/D: selection now uses frozen Core media slots with optional Model-40 secondary backing; generic ATA stays unchanged, and physical/L3 timing remains transferred. [Evidence](../etc/evidence/t430-s1-deskpro-dual-fixed-disk.md). |
| T429 | Closed: generic-AT CECG port/aperture wait skeleton and D4-memory classification; physical CECG and monitor behavior remain TODO. [S1](../etc/evidence/t429-s1-cecg-8bit-bus-wait.md), [S2](../etc/evidence/t429-s2-d4-cecg-memory-class.md), [S3](../etc/evidence/t429-s3-cecg-aperture-wait.md).
| T428 | Closed: Model-40 selects a generic-AT Core prefetch reservation lifecycle with reset/HOLD/refresh priority; it publishes no physical cycle, page hit or timing result. [Evidence](../etc/evidence/t428-s1-generic-prefetch-reservation.md). |
| T427 | Closed: original-source CPU BUSRDY gate holds the existing Core external completion wait without retirement; Model-40 selects it and 5170 is isolated. [Evidence](../etc/evidence/t427-s1-deskpro-cpu-busready-gate.md). |


## Recent Governance

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
