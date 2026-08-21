# Project Status

## Current Work

**M5 T435 S4 is active.** This continuation implements and verifies the
complete 8086 timing-manifest batch; T435 remains open for S5--S8.

| Field | Required record |
| --- | --- |
| Identifier Mode | `Continuation`; M5 T435 S4, following accepted S3. P numbering restarts at P1 for executor implementation; the later coordinator acceptance is a governance-only P. |
| Admission And Approval | Owner: user. Approval: implement S4, 2026-08-21. The owner clarified that S4 must close the complete manual-specified 8086 instruction behavior and timing, not timing alone. Scope approved: actual 8086 L3/L2 timing implementation, Core-private `WAIT`/TEST iteration input and successful-wait semantics, and exhaustive closure of the frozen 649-key S2 manifest. No exception is approved. |
| Objective | Make every `t435-s2-8086-timing-manifest.json` base, legal-context, and combination key select its ledger-defined L3 timing or named `L2:G3` model before successful retirement; implement each admitted 8086 instruction's required functional input/behavior together with timing; emit a result that proves key, inputs, origin and no unallocated success. |
| Non-goals | No 80186/80286/80386 timing changes; no board/transaction timing, DMA, prefetch, INTA or physical-cycle modelling; no new public ABI. The Core-private 8086 `WAIT`/TEST iteration contract is in scope, but 8087 operation-duration modelling remains outside it. |
| Reference Baseline | Accepted T435 S3 (`0567d134`): one Core-private selector/publication seam and compiled 3,295-key catalog. S4 consumes the 8086 S1 ledger, S2 manifest, legality contract, audit/tracker, and S3 result contract. |
| Candidate Proposal | [M5 Core CPU Instruction Timing Programs](../proposals/m5-core-cpu-instruction-timing-program.md), S4 row: 8086 L3 plus 16 named L2:G3 Group-3 keys, EA, odd-word, prefix, repeat and partition coverage. |
| Files And ABI Surface | Primary: `src/core/machine/cpu_timing.c/.h`, `src/core/machine/machine.c`, owned 8086 timing tests and verifier/evidence under `tests/machine`, `cmake`, `tools`, and `docs/etc/{cpu-timing,evidence}`. Core-private structures may change only where required; public/cross-module ABI remains unchanged. |
| Applicable Rules | Architecture: one Core CPU-timing owner/selection path; no board timing in this S; no second legacy truth source. Coding: C11, narrow cohesive helpers, remove obsolete successful path/tests, report tracked source/test delta. Execution: manifest is the coverage ledger, actual-diff review, similar-issue sweep, immediate P push, documentation gate. Source policy: no source import; existing accepted research/ledger is reference-only. |
| Verification | Run all four manifest-contract verifiers; add/run an exhaustive 8086 manifest-result verifier plus focused 8086 timing/fault/retirement regressions; run relevant CTest set, S3/T359/T360/T388 static seams, documentation governance, configured build and current `vm-0-5-0434` artifact verification. |
| Expected Markers | 649 unique canonical 8086 results; each has a known manifest key, `passed=true`, no duplicate/missing result, `unallocated=false`, correct L3 or named `L2:G3` classification, required input/origin proof; no successful 8086 one-tick/source-unallocated fallback; no legacy 8086 selector survives as a competing production truth. |
| Asset Needs | No new asset. Use committed S1 manual/research citations and existing generated manifest/catalog only; do not import emulator code or vendor material. |
| Reporting Requirements | Record implementation mapping and exhaustive result artifact in indexed `docs/etc/`; report exact test/static/build outcomes, source/test added/removed/net lines, obsolete-path disposition, manual/L2 origin proof, and any out-of-scope finding/transfer. |
| Stop Conditions | Stop and return to coordinator if a manifest key needs a non-ledger timing rule, a manual/reference conflict, a public ABI/semantic change, board input other than the owner-approved Core-private 8086 `WAIT`/TEST iteration contract, a source/license import, or a failure that cannot be fixed inside 8086 timing selection. Do not substitute fallback, a missing result, or a future S transfer for a scoped key. **Observed before P:** `XLAT` (`D7`) is a successful 8086 form in the retained source but is absent from the S1 ledger, S2 manifest and 649-key universe; expanding that frozen universe needs renewed owner approval. |
| Exit Criteria | All 649 S2 8086 keys are actual passing results under the shared contract; 228 base/166 context/223 combination L3 and 16 base/12 context/4 combination named L2 records retain their ledger provenance; 8086 successful retirement is never unallocated; obsolete selector path is removed/disposed; focused and required gates pass; complete P is pushed and independently accepted. |
| Original Owner Request | Implement S4 after requiring full 8086 closure rather than smoke-only coverage or a temporary fallback. |
| Similar-Issue Sweep | Defect class: successful 8086 instruction timing that is unallocated, selects a non-ledger rule, or lacks key/context proof. Scope: tracked Core machine timing source, 8086 timing tests, CMake/static verifiers, S2 manifests/contracts/tracker. Search `rg -n "source_timing_unallocated|8086_source_instruction_cost|CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_8086_FALLBACK|I86-" src tests cmake tools docs/etc/cpu-timing`; every production hit is fixed, proven non-8086/out-of-scope, or recorded as a governed transfer. |
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
| T435 S3 | Closed: [S3 pipeline evidence](../etc/evidence/t435-s3-cpu-timing-pipeline.md) proves one Core-private selector and one successful-retirement publication seam, preserves external board timing ownership, publishes form/key/input/origin/unallocated results, and compiles the 3,295-key S2 catalog. No key is claimed conforming; S4/8086 is the next owner-admitted batch. |
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
