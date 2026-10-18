# MyRaidenII Status

## Current Work

M1 T2 S28 implementation is delivered at Shared P1 `2a51bee0e` and
NXVM receiving P2 `67b32dab5`; the complete evidence delivery precedes
coordinator actual-change review. S28 remains active until that review.
T2 and the full playable objective remain open.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, MyRaidenII M1 T2 S28; next unused S after accepted S27. |
| Admission And Approval | Owner requests continuation implementing MyRaidenII's V30 and facilities; standing V30 implementation and enum approval, no Lib/Emulator modification. Coordinator admits the complete software-entry arbitration batch on 2026-10-18. Targets Shared CPU/tests, NXVM eight receiving EXEs/evidence, MyRaidenII governance/evidence. MyNES is unchanged. |
| Objective | Consume V11 software-entry versus simultaneous NMI/INT/BRK arbitration: a successfully committed software entry wins this completion boundary, retains external requests, and admits them only after subsequent guest execution. Include BRK3/BRK/BRKV, DE/CHKIND and native/emulation mode calls at their existing entry owner. |
| Non-goals | No new public API/enums/layout, Lib/Emulator changes, timing model, loaded-BRK sampling upgrade, Intel policy change, board/game execution or T closure. DE/BR retirement/timing classification remains V15; this batch fixes arbitration only. |
| Reference Baseline | Clean pushed a50905530; original NEC October 1986 manual SHA FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C, Table 6-1 and mode-call flag tables; S22/S23/S27 entry proofs and complete T2 convergence ledger. |
| Candidate Proposal | proposals/m1-v30.md; V10/V11 successful software-entry priority subfamily. Existing timing transfer metadata describes protected transfers, not software-entry acceptance; use one CPU-private per-refresh completion fact written only by successful real entry while an instruction is executing. No opcode shadow classifier. |
| Files And ABI Surface | src/x86-core/cpu/cpu_instructions.c/.h, test/x86-core/cpu software arbitration unit and registration, both manifests; product-scoped receiving artifact/evidence and MyRaidenII records. Estimate production +15..25/-2..5, tests +250..400/-0..5. Private context bookkeeping only; public opaque interface unchanged. |
| Applicable Rules | Read product guide/architecture/coding/roadmap/source policy, shared Execution/Architecture/Coding/Document, local architecture/coding skills and receiving NXVM guide/source/artifact authorities. One entry owner, Lib vocabulary, source-backed priority, synthetic units, explicit partial-write failure semantics, one-target P, reusable graphs/cache, actual-diff coordinator acceptance. |
| Verification | Before-fix actual CPU witness; both original Release Ninja/ccache graphs -j8, affected units then complete repository unit suites -j4, specialized Types/dependency/manifest/strict gates, fourteen selected PC receiving integrations plus restored-default identity per width, all ten PE/hash identities; MyNES/INI/media/ROM unchanged. Documentation and diff checks. |
| Expected Markers | V30:SOFTWARE-ARBITRATION:OK; complete units with zero failures/skips per width; existing strict rows retained; all receiving cases and identity checks pass. No timeout retry or partial-run pass claim. |
| Asset Needs | No game input; pinned ignored primary manual only. Rebuild/deploy eight current PC 0564 EXEs; all ten identities, MyNES pair unchanged. No runnable MyRaidenII artifact exists yet. |
| Reporting Requirements | Confirm mechanism/estimate, report reproduction and focused proof, then full qualification and actual added/removed/net source/test lines, helpers/state/API necessity, remaining timing/board/game obligations and unexecuted desktop/RDP/full external tests. |
| Stop Conditions | Public contract expansion beyond approved boundary, new source/license/model necessity, unrelated dirty tree, regression or failed required release/gate prevents acceptance; diagnose in scope, never weaken assertion or silently transfer failure. |
| Exit Criteria | Frozen software-entry mechanism and failure batch proven, external requests preserved, no same-boundary double entry, old Intel tests pass, full units/gates/receiving releases pass, manifests and evidence current, target-separated P commits pushed; coordinator actual-change review then pure governance closure. T remains open. |
| Original Owner Request | Switch to MyRaidenII context and continue the next S implementing NEC V30 and required facilities; automatic sequential S execution under governance. |
| Similar-Issue Sweep | Inspect every _ser_int_real caller, debug completion and ExecInt admission, exception/mode entry and halted/reset refresh. Fix at successful entry commit, retain distinct asynchronous/non-V30/failure contracts, document DE/BR retirement receiver rather than misusing fault flags or fabricating timing. |

## Task Progress

| Task progress | Accepted result / remaining work |
| --- | --- |
| T2 S28 | Software-entry arbitration delivered for coordinator review; original remaining CPU/Machine/board/game obligations remain open. |

[S28 evidence](../etc/evidence/t2-s28-software-arbitration.md),
[results](../etc/evidence/t2-s28-results.json) and
[ten artifact identities](../etc/evidence/t2-s28-artifacts.json) preserve proof.
[T2 history](../history/M1-T2-v30.md) and
[ledger](../etc/t2-v30-ledger.md) retain the complete original domain.

## Final Verification

Both original all-target Release Ninja/ccache graphs and whole-repository
units pass 676/676 per width, including 189 X86 Core tests, in
317.96/118.01 seconds. The new 3,457-input software-arbitration unit plus
refusal routes takes 0.0507752/0.0629108 seconds. Both seven-unit focused
groups, all 498 strict rows, Types/dependency/ownership/negative/manifest,
documentation and whitespace gates pass. Existing instruction warning
exceptions remain explicit. No final suite failure/skip/retry or increased
budget is hidden.

Eight PC 0564 EXEs are rebuilt/deployed, with fourteen selected receiving
integrations plus restored-default identity per width. Both original graphs
are default; profile switches recompile zero CPU objects. All ten PE/name/
width/size/hash identities pass. MyNES 0044 and INIs are unchanged; no
external firmware/media or snapshot source is edited. All owned handles exit.

Production +9/-2/net +7; tests/registration +287/-0/net +287; code total
+296/-2/net +294. Manifests separately +4/-3/net +1, records/artifacts excluded.
One private per-refresh completion fact at successful real entry governs
both step completion and asynchronous admission. No public API/layout,
production helper, clock, worker, queue or second entry path is added.
Failure preserves the existing checkpoint and accepted write prefix.

## Remaining T2 Boundary

Loaded-BRK/other prefix sampling, general arrival/nesting, packed-adjust/non-
ten conversion and CHKIND source/model questions, complete native slot/form
qualification, physical bus/prefetch/LOCK and complete V30 timing retain
their original receivers. DE/BR delivered-fault retirement classification
remains V15, not silently qualified by this arbitration proof. Neutral
Machine admission and actual board/game execution remain open. Existing
L2 model grades are not promoted. No fresh complete external/native
integration suite, manual desktop/RDP/audio or gameplay claim is made.
There is still no runnable MyRaidenII EXE.

## Previous Work

S27 closed at `a50905530`; [its evidence](../etc/evidence/t2-s27-interrupt.md)
retains the earlier asynchronous-arbitration qualification.
T1 source/board/set and integration-contract freeze closed at `7f7d23938`;
[T1 history](../history/M1-T1-board-and-source-freeze.md) retains thirteen
runtime image identities and F01--F13, not game execution.
M0 Td S1 closed at `f21a22997`; M1 remains open toward the full playable goal.
Accepted NXVM T564 supplies RGB32/four-slot publication; actual RAIDEN II
palette/frame/rotation integration remains unqualified.

## Queue

[Queue](QUEUE.md) contains five unadmitted receivers after open V30 T2.
