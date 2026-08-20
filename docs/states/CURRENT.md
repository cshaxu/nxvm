# Project Status

## Current Work

**Active: M5 T435 S1, block 1 of 4 (8086 evidence and model ledger).**

## M5 T435 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New; T434 is task-level closed. S1 has four owner review blocks: 8086, 80186, 80286, and 80386. This admitted first delivery is 8086 only; S1 remains active until the owner accepts each block and the complete four-profile ledger is reconciled. |
| Admission And Approval | Owner approval in this conversation on 2026-08-20: collect and read authoritative manuals in the approved external archive, account every instruction, research simulator models where L3 is incomplete, and use the stated L3/L2/L1 ladder. The owner explicitly requires review after block 1 before block 2. |
| Objective | Create the complete 8086 successful-retirement manual/reference/model ledger for every legal current 8086 instruction form and context; classify each rule as manual exact/formula/range/principle-only/no timing, then select a labelled L3/L2/L1 disposition under Td S121 without changing runtime code. |
| Non-goals | Do not begin 80186/80286/80386 ledger blocks, change an instruction implementation or timing value, treat emulator code as L3 authority, import third-party source, infer board/device waits, model L4, or close S1/T435 after the 8086 review. |
| Reference Baseline | [CPU timing proposal](../proposals/m5-core-cpu-instruction-timing-program.md); [Td S121](../etc/evidence/td-s121-cpu-timing-evidence-first-governance.md); [T359 8086 inventory](../etc/evidence/t359-s1-four-profile-instruction-timing-inventory.md); [T360 source closure](../etc/evidence/t360-s5-four-profile-timing-source-closure-audit.md); and the user-approved external historical-manual archive. |
| Candidate Proposal | [Core CPU instruction timing programs](../proposals/m5-core-cpu-instruction-timing-program.md). |
| Files And ABI Surface | `docs/etc/cpu-timing/` 8086 research/ledger documents, `docs/etc/README.md`, `docs/states/CURRENT.md`, and accepted Td S121 governance material only. No `src/`, public ABI, build target, artifact or runtime behavior change. |
| Applicable Rules | Task Reading Set; Execution coverage-ledger/P-review lifecycle and code-size reporting (documentation-only: no counted code paths); Documentation supporting-index authority; Source/research policy; Architecture source/dependency boundary; Td S121 L3/L2/L1 ladder. |
| Verification | Archive permitted manual research inputs in the approved external archive; record publisher/edition/URL/hash and exact loci; reconcile every legal 8086 current metadata/dispatch form against the manual; inspect version-pinned 86Box, MAME, PCjs and Bochs timing implementations as non-imported L2 candidates; prove every form/context has one disposition and no anonymous one-tick outcome; documentation governance and diff checks pass. |
| Expected Markers | `M5:T435:S1:8086-MANUALS:OK`, `M5:T435:S1:8086-FORM-LEDGER:OK`, and `M5:T435:S1:8086-L2-MODEL-REVIEW:OK` only when the corresponding complete 8086 records are present. |
| Asset Needs | Owner-authorized external research archive for historical manuals only; do not commit copyrighted manuals, third-party source, firmware, guest media, local paths or raw copied code into the repository. Repository evidence records neutral citations, hashes and conclusions only. |
| Reporting Requirements | Report manual archive identity, each complete 8086 family disposition, every consulted simulator revision/model, manual conflict/correction, L1 exception if any, and the owner review gate before 80186. Commit/push every completed P by default. |
| Stop Conditions | Stop before advancing to 80186 if an 8086 form cannot be mapped to manual/research evidence, a manual cannot be lawfully/reproducibly archived, a model conflicts with manual semantics without an explainable correction, or a purported L2 collapses to anonymous one tick. Record the exact blocker in the 8086 ledger and request owner review; do not paper over it with L1. |
| Exit Criteria | One owner-reviewable pushed 8086 evidence/model ledger covers every current legal 8086 successful-retirement form/context, records manual status and exact loci, evaluates available reference models, gives each row a proposed labelled L3/L2/L1 disposition with rationale, and leaves T435 S1 active awaiting owner review. |
| Original Owner Request | Collect authoritative manuals and account every instruction; for a row without exact/formula timing, establish L2 from manual-conforming emulator modelling or a fixed range midpoint, and use L1 only when neither model nor range exists. T435 S1 has four blocks; implement and review 8086 before 80186. |
| Similar-Issue Sweep | Compare 8086 metadata, primary dispatch, prefix legality, Group opcode extensions, current timing classifiers and tests; sweep 86Box, MAME, PCjs and Bochs for every 8086 timing table/model rather than relying on one implementation or one dynamic-arithmetic example. |
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
