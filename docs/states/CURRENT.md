# Project Status

## Current Work

**M5 T435 S5 is active.** This corrective reopens the 8086 coverage claim at
its boundary: reconcile every successful 8086 decoder form against S1/S2,
then close every discovered omission. The planned 80186 batch follows as S6.

| Field | Required record |
| --- | --- |
| Identifier Mode | `Corrective`; M5 T435 S5 follows accepted S4 after its whole-8086 claim was shown to omit successful opcode `XLAT` (`D7`). |
| Admission And Approval | Owner: user. Approval: repair S1/S2 and implementation as a reopened S4 P, 2026-08-21. Governance maps this to the next legal corrective S5. No exception is approved. |
| Objective | Mechanically reconcile the successful 8086 decoder corpus with the S1 ledger and S2 manifest; add every omitted instruction/form/context, implement its manual L3 or named L2 timing and required semantics, and publish an exhaustive actual result universe. |
| Non-goals | No 80186/80286/80386 timing implementation; no board, transaction, DMA, prefetch, INTA, device-service or physical-cycle timing; no public ABI. |
| Reference Baseline | Accepted S4 P1 `a2d66c19` and governance P2 `c4156333`; S1 8086 ledger, S2 manifest/context contract, and the Intel 1979 Table 2-21 primary manual. |
| Candidate Proposal | [M5 Core CPU Instruction Timing Programs](../proposals/m5-core-cpu-instruction-timing-program.md), corrective S5 row. |
| Files And ABI Surface | S1/S2 8086 ledger, manifest, tracker/audit and result artifact; Core-private 8086 timing/decoder-owned functional code only where the difference requires it; owned runner/verifier/tests, CMake and evidence. No public or cross-module ABI change. |
| Applicable Rules | One Core timing owner and no duplicate selector; C11 and cohesive owner-local changes; source material is reference-only and no external code/import; coverage ledger requires a finite, zero-difference completion predicate; report source/test delta and retired paths. |
| Verification | Add a reproducible decoder-to-ledger difference verifier; execute every expanded 8086 key on a real Core machine; verify result provenance/ticks/inputs and absence of unallocated success; run focused and full current-gate CTest, static timing seams, documentation governance, configured build and artifact verification. |
| Expected Markers | The existing `M5:T435:S5:I86-XLAT-CORRECTION-SLICE:PASS` proves only the repaired XLAT slice. S5 P2 must replace it with a decoder-derived zero-difference marker whose input is every accepted 8086 opcode/form/context; every expanded key is unique, passed, allocated and L3/L2-proven. |
| Asset Needs | Existing owner-authorized manual corpus only. No source, firmware, media or emulator-code import. |
| Reporting Requirements | Record the complete pre/post decoder-to-ledger difference, every newly admitted key/context and its manual locator, functional/timing implementation mapping, actual result count, test/static/build outcomes, delta and any remaining boundary. |
| Stop Conditions | Stop for a decoder form whose accepted/manual status cannot be determined, a required source/license import, public ABI change, board input, or a difference that changes the 8086 profile's admitted instruction boundary. Do not hide an unresolved difference by excluding it from the verifier. |
| Exit Criteria | The reproducible successful-decoder-to-ledger difference is empty; every newly discovered instruction/form and legal context has S1 source/L3-or-L2 disposition, S2 manifest key, functional proof and successful actual retirement result; prior S4 evidence is corrected to its 649-key scope; all required gates pass and the complete P is pushed and independently accepted. |
| Original Owner Request | Repair the omitted `XLAT` manual/timing entry and implementation, and re-close the same 8086 work without leaving any other unledgered successful instruction. |
| Similar-Issue Sweep | Sweep all 8086 decoder table registrations, prefix/context legality, S1/S2 key expansion, runner recipes, result verifier and historical S4 whole-coverage language. The mechanically detectable class is any successful 8086 opcode/form absent from the manifest-derived ledger. |
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
| T435 S4 | Closed: [S4 execution evidence](../etc/evidence/t435-s4-8086-manifest-execution.md) proves every frozen 8086 key through a real Core retirement: 617 L3 and 32 named L2:G3 results, no unallocated successful retirement, all 291 current-gate tests passed, and the current 0.5.0434 artifact rebuilt. S5/80186 remains the next owner-admitted batch. |
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
