# Project Status

## Current Work

**Active: M5 T436 S6.** The owner-approved continuation closes the remaining
80286 call, return, jump and interrupt-transfer partition through real
retirement observations.

## M5 T436 S6 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; M5 T436 S6 follows accepted S5 in the latest open numeric task. |
| Admission And Approval | The owner directed continued complete 80286 implementation in single-person dual-role mode. S5's accepted legal-universe correction assigns S6 exactly 27 base and 193 canonical transfer keys. |
| Objective | Prove and, only where required, implement every S6 CALL, RET, JMP, INT/INT3 and IRET successful path, including direct/indirect, real/protected, privilege, gate, task and next-byte inputs. |
| Non-goals | Do not change S3--S5 or S7 ownership; do not implement protected segment/descriptor/system forms, fault/exception/event delivery, board timing, public ABI, source import, a new profile, or final 771-key result emission. |
| Reference Baseline | [S1 partition](../etc/cpu-timing/t436-s1-80286-key-partition.md), [80286 manual ledger](../etc/cpu-timing/t435-s1-80286-ledger.md), [manifest](../etc/cpu-timing/t435-s2-80286-timing-manifest.json), [S2 producer](../etc/cpu-timing/t436-s2-80286-result-producer.md), [S5 evidence](../etc/cpu-timing/t436-s5-80286-conditional-control-closure.md), and the current manifest runner. |
| Candidate Proposal | [M5 80286 instruction timing closure](../proposals/m5-80286-instruction-timing-closure.md). |
| Files And ABI Surface | Core-private control-stack timing/retirement path only where a real S6 recipe proves necessary; manifest runner, focused tests, evidence, history and CURRENT. No public header or external ABI change. |
| Applicable Rules | Execution: complete P lifecycle and dual-role acceptance. Architecture: one Core timing/publication route; no board/event policy. Coding: C11 cohesive owner-local repair. Documentation: active packet/evidence only. Source policy is not triggered: no import or research. |
| Verification | Run manifest, decoder-ledger and partition verifiers before and after changes; build/run the manifest runner; prove every S6 key has a successful classified real retirement with exact Appendix-B ticks/origin/path and required next-byte input. Run result-refusal and documentation gates plus actual diff review. |
| Expected Markers | Add `M5:T436:S6:I286-TRANSFER-COVERAGE:PASS:canonical=193` and `M5:T436:S6:I286-PROTECTED-PATHS:PASS`; retain incomplete final-result refusal. |
| Asset Needs | Existing owner-managed Intel manual evidence and repository fixtures only. No download, firmware, guest media, Microsoft component or third-party source. |
| Reporting Requirements | Report base/context ownership, before/after captures, real/protected path and next-byte proof, source/test delta and live/retired mechanism paths; transfer only S1-owned S7 residual keys. |
| Stop Conditions | Stop for manual/manifest/decoder contradiction; an unrepresentable protected path or next-byte input; required public ABI, board or event timing; source-policy issue; a defect outside S6; or verifier contradiction. Do not use a generic control fallback. |
| Exit Criteria | All 193 S6 canonical keys, and no other partition, have real classified L3 observations with correct ticks/origin/form/input/unallocated fields; functional path checks pass; no S6 record uses generic/unallocated fallback; focused and relevant regressions pass; dual-role review accepts actual changes. |
| Original Owner Request | Owner request, faithfully translated: continue complete correct 80286 instruction function and timing implementation through the reasonably split S tasks. |
| Similar-Issue Sweep | Defect class: a successful 80286 transfer retirement missing canonical observation, actual path/privilege outcome or next-byte formula input, Appendix-B timing or focused functional proof. Scope: all S6-owned keys, control-stack decode/timing, runner recipes and result evidence. |
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
| T436 S5 | Closed: [conditional-control evidence](../etc/cpu-timing/t436-s5-80286-conditional-control-closure.md) proves all 54 keys, both INTO outcome next-byte paths and 771-key manifest legality. S6 protected control transfer is next. |
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
