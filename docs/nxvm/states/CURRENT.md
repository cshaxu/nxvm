# Project Status

## Current Work

M5 Td S181 follow-up P5 onward is delivered for owner review after removal of
audited duplicate tooling operations and isolation of cache-test statistics.
The previous delivery was not accepted/closed. This standalone Shared task does not advance
T564, MyNES or MyRaidenII T2. Existing product work and artifacts stay intact.

| Field | Required record |
| --- | --- |
| Identifier Mode | Governance: M5 Td S181 follows latest NXVM Td S180; one session performs coordinator/executor roles. No implementation T/S or artifact revision is allocated. |
| Admission And Approval | Owner's 2026-10-18 original request and subsequent audit approval authorize this same Td follow-up: four tooling cleanups and related proof/records. All three products retain the same policy; no sibling repository import. |
| Objective | Retain the delivered single CMake/cleanup owners; remove duplicate filesystem checks, inherited preset directories and MyNES standalone options. Replace global cache-hit deltas with invocation-local checked statistics, preserving real compile and safety proof. |
| Non-goals | No emulator behavior, Shared corpus source/API/manifest, ROM/media/INI mutation, global-cache deletion, product binary refresh, device qualification, new parser/service/manager or copied SoftPC implementation. |
| Reference Baseline | Follow-up clean b85336f39, after the recorded initial 17af5609a cleanup. Preserve both stable compiler graphs and all existing product inputs. Recheck task processes before deleting follow-up outputs. |
| Candidate Proposal | ../proposals/m5-build-cache-governance.md; audit/evidence ../etc/evidence/td-s181-build-governance.md. |
| Files And ABI Surface | Follow-up: CMakePresets.json, tools/shared/Clear-Build.ps1, test/tools/build_environment.ps1, tools/mynes/Verify-SharedStandalone.ps1 and this task's records. Estimate scripts/checks +15--30/-20--40; actual +27/-31/net-4. No source corpus, ABI, rule or product input change. Prior delivery +383/-47/net336 remains historical. |
| Applicable Rules | Guides and Execution/Architecture/Coding/Document authorities; architecture/coding governance skills. CMake owns configuration, cleanup tool owns safety, tests call actual owners, documentation gate stays documentation-only. Owner explicitly replaces old cross-task build-tree retention policy. |
| Verification | Real x64/x86 stable Ninja builds, optional launcher off/on, direct build under hostile cache environment, no-op increment, actual local cache stats; reject outside Ninja trees; preview/apply/traversal/link/Git/cache/protected-file/all-before-delete safety tests. Applicable three product doc gates, actual diff and artifact hashes unchanged. No physical/native-device tests required. |
| Expected Markers | Stable .ninja trees, fixed three CCACHE variables, no repeated compile on unchanged build, default preview does not delete, dangerous targets refuse before any deletion; final build contains only .ccache/.ninja or explicitly reported protected exceptions. |
| Asset Needs | Existing external inputs and ten artifacts unchanged. Temporary owner: this Td session, build/td-s181; total2 hours, no progress5 minutes, diagnostic output128 MiB. Cleanup fixtures build/cleanup-contract-PID and build/build-contract-width-PID have180-second and8 MiB budgets and self-clean. Stable build metadata/objects stay only in .ninja. |
| Reporting Requirements | Initial brief and diff estimate before implementation; concrete cleanup list before Apply; actual code/docs counts, bytes freed, retained files, cache/increment/safety proof, failures and unexecuted checks; pushed complete target-scoped delivery awaiting owner review. |
| Stop Conditions | Live unrelated process, unchecked Git data, ambiguous/protected input without preservation/identity, path/link escape or source/behavior scope expansion. Resolve or retain explicitly; never recursively delete build root. |
| Exit Criteria | Follow-up actual diff review complete; each width passes2/2 governance tests with2 invocation-local cache hits and no-op increments. Four standalone configurations, three doc gates,1398 Types inputs and ten unchanged artifact identities pass. Remove only owned16 temporary files /171030 bytes; only both cache roots remain. Shared P5 f15f5a77b and MyNES P6 31c1c3ca1 are pushed; this NXVM P7 receipt awaits acceptance. Prior P1--P4 evidence remains intact. |
| Original Owner Request | Uniform optional ccache/Ninja and temporary-output lifecycle with safe existing build cleanup, minimal single-owner tooling and no unnecessary EXE refresh. |
| Similar-Issue Sweep | Search tracked presets/tools/current operational docs for old tree paths/launchers, task-keyed build roots and duplicated cleanup ownership. Historical evidence remains historical; stale product-only script contracts are repaired or explicitly retired. |

## Retained Numeric Task

M5 T564 remains open. The owner accepted S1 on 2026-10-17 and explicitly
requested its closure. S1 is closed after coordinator actual-change review
of Shared `d30b69b2a`, MyNES `906e61b0c` and NXVM `b9433b31d`.
There is no active S packet and no successor task is admitted.

| Task | Progress |
| --- | --- |
| T564 | S1 accepted and closed: committed SoftPC eight-tree import, sole PC/NES RGB/text receiving paths, ten qualified releases. T-level closure has not been requested. |

The complete S1 packet is retained in `b9433b31d`. The
[proposal](../proposals/m5-softpc-rgb-frame-import.md),
[task history](../history/M5-T564-softpc-rgb-frame-import.md) and
[review evidence](../etc/evidence/t564-s1-shared-frame-import.md) retain the
frozen scope, similar-issue sweep, initial failures and final proof.
Each width passed 666 units, 48 product integrations, four shared native
integrations, specialized gates and 498 strict rows. The accepted
[results](../etc/evidence/t564-s1-results.json) and
[ten artifact hashes](../etc/evidence/t564-s1-artifacts.json) remain unchanged:
eight PC 0564 and both MyNES 0044 EXEs.

Closure rechecked all ten deployed hashes and the 297 canonical imported files
against the frozen export. The eight source/test roots match the reviewed
SoftPC dfc85aac bytes, also unchanged at its 5e76dd49 closure baseline.
No source, test, build, configuration or artifact input changes in this
governance-only acceptance; no rebuild is required. Manual RDP, physical
audio/controller and whole guest GUI proof remain unclaimed, with prior
MyNES TODOs unchanged. The task queue is not advanced.

The owner's subsequent selection of former queue entry 10 is governed by
[MyRaidenII M0](../../myraidenii/states/CURRENT.md), not an admitted NXVM T.
The [NXVM Queue](QUEUE.md) records the transfer; other candidates remain pending.

## Receiving Release Update From MyRaidenII

MyRaidenII M1 T2 S47 refreshes the eight0564 PC EXEs after Shared P1
`6d4ed7fe8` semantic/form/prefix/lock-lifecycle delivery. No NXVM source, configuration,
firmware or media is changed, and no NXVM task/S is allocated. Both MyNES
0044 EXEs and all13 protected asset files remain unchanged.

Current receiving proof passes682 exact repository unit names per width,
498 retained strict rows and1398 Types inputs. Four PC receivers each pass
their selected configuration/boot cases:14 integrations per width, followed
by restore identity. Default restore relinks the final EXE, so all five default
receiving cases additionally pass on that final deployment. Eight PE widths,
stripped sections and final hashes are verified; profile switches/restores
compile zero CPU objects. Both cached Ninja graphs finish at default; six
selected external media identities retain before/after size/SHA/access mode.

The [S47 evidence](../../myraidenii/etc/evidence/t2-s47-residual-semantics.md),
[results](../../myraidenii/etc/evidence/t2-s47-results.json) and
[final ten release identities](../../myraidenii/etc/evidence/t2-s47-artifacts.json)
supersede only the prior receiving PC hashes. They do not rewrite T564 or Td
S181 status, qualify the whole external suite/desktop/RDP, or complete V30
timing/Machine admission or MyRaidenII gameplay. The one sandbox cleanup
failure and unchanged native-permission replacement proof are recorded.

### Prior S46 Receiving Update

MyRaidenII M1 T2 S46 refreshes the eight current 0564 PC EXEs after
Shared V30 escape/bus and instruction-entry mode P1 `128204d91`.
NXVM production source, configuration, firmware and media are unchanged;
one receiving static assertion now checks the CPU-private prefix helper's
actual context-bearing signature. No assertion is removed or weakened.
This receiving update allocates no NXVM task/S.
MyNES 0044 hashes remain unchanged. This receiving publication supersedes
only the PC hashes in the historical T564 snapshot above; it does not rewrite
T564's accepted evidence or open task status. See the
[current receiving evidence](../../myraidenii/etc/evidence/t2-s46-time-accounting.md)
and [ten release identities](../../myraidenii/etc/evidence/t2-s46-artifacts.json).
Each width passes680 units (193 X86 Core),498 strict rows,1391 Types inputs
and14 selected PC receiving integrations plus a restored-default identity,
not a new full integration-suite claim. Both original Ninja/ccache Release
graphs finish at default. Profile switches/restores recompile zero CPU
objects, verified with both Ninja path-separator spellings. INIs and MyNES
assets are unchanged; six selected external media originals retain actual
before/after SHA-256/size/access-mode identities, not an entire archive audit.
Earlier receiving proof remains in MyRaidenII history, not rewritten.
S46 is a bounded prerequisite stage, not complete V30 timing, Machine
admission or MyRaidenII gameplay qualification.

## Latest Task Closure

M5 T563 is closed after actual-change review of S10 Shared **a936e554b**,
MyNES **621b98d15** and NXVM **80bc1ac92**, plus the complete CPU closure audit.

| Task | Closure |
| --- | --- |
| T563 | S1--S10 accepted. Original C01--C17, F01--F20 and R1--R10 are reconciled: selected 486SX functionality, complete runtime/preview timing and real Core/Board production admission are implemented and qualified. Both widths pass 655 units (166 X86 Core), 48 product integrations, four shared native integrations and 498 strict rows. Ten current releases are verified. |

The [closure audit](../etc/evidence/t563-final-closure.md),
[task history](../history/M5-T563-486sx-timing-production-completion.md) and
[archived proposal](../history/M5-T563-486sx-timing-production-completion-proposal.md)
retain the requirement-to-proof mapping, source/model grades and acceptance.
S8's sole transferred native MIDI obligation is resolved by actual successful
S9/S10 opens/releases; reclassification alone is not treated as its resolution.

[S10 results](../etc/evidence/t563-s10-results.json),
[canonical inventory](../etc/evidence/t563-s10-canonical.json) and
[ten artifact hashes](../etc/evidence/t563-s10-artifacts.json) are the accepted pre-T564
receiving baseline: PC 0563 and MyNES 0044 pairs. Initial x64 timeout/native
Console failures and unchanged successful full reruns remain explicit in
[S10 evidence](../etc/evidence/t563-s10-shared-import.md) and existing TODOs.

Closure does not promote approved L2 models to silicon-exact timing or claim
broader x87, every 486 variant, PC110 board/firmware, manual RDP, physical audio
or full guest GUI qualification. The next queued candidate is PC110 Board;
no successor T is admitted automatically. This governance-only closure changes
no source/test/build/configuration/EXE input and needs no artifact rebuild.

## Previous Task

M5 T562 is closed after S4 coordinator review of pushed delivery `131e7d65c`.
Its complete final packet is retained in that commit;
[archived proposal](../history/M5-T562-shared-audio-import-quality-proposal.md)
and [task history](../history/M5-T562-shared-audio-import-quality.md) retain scope.

| Task | Closure |
| --- | --- |
| T562 | S1--S4 accepted: pinned import, approved shared repairs and equivalent-path consolidation, final 631/631 units and 48/48 integration executions per width, published CLI checks and specialized gates. Eight PC 0562 artifacts refreshed; MyNES 0044 pair remains current. |

[Final evidence](../etc/evidence/t562-s4-final-qualification.md),
[integration inventory](../etc/evidence/t562-s4-integration-inventory.tsv) and
[ten artifact hashes](../etc/evidence/t562-s4-artifacts.json) record results.
S4 changes no source/test code. Earlier native-console intermittency remains
in MyNES TODO; manual RDP, physical MIDI/audio and complete guest GUI proof are
not claimed. Remaining 486SX C16/C17/final qualification is now admitted
separately as T563 above; it was not completed by T562 closure.

## T562 S2 — Accepted Shared Quality Cleanup

S2 is closed after one-session coordinator actual-commit review of Shared
`e5b346309` and NXVM `8f3280726`. S3 is also accepted as recorded above.
The complete S2 implementation packet is retained in P2.

The five admitted findings are repaired: obsolete port dispatch, empty
composition wrapper, test ownership, stale names and checker blind spots.
[Evidence](../etc/evidence/t562-s2-shared-boundary-cleanup.md) and
[eight artifact hashes](../etc/evidence/t562-s2-artifacts.json) record 630/630
units per width, 15 selected integration executions per width, eight final
published CLI checks, all specialized gates and the reviewed actual diff.

Eight PC 0562 EXEs are rebuilt and published. MyNES, owner INI/media/ROM/
snapshots and sibling repositories are unchanged. Automated desktop-labelled
tests pass; manual RDP/physical audio and fresh full external qualification
are not claimed. Queue order and the unresolved 486SX successor are unchanged.

## Completed M5 Td S180 Shared Eight-Tree Audit

Owner-requested audit is complete; no source/test/build/artifact was changed.
[Report](../etc/evidence/td-s180-softpc-eight-tree-audit.md) and
[hash inventory](../etc/evidence/td-s180-eight-tree-hashes.tsv) pin NXVM
12e79911b versus SoftPC 44cad548: only committed Lib source/test differ;
the remaining six roots match. Upstream's subsequently edited Emulator
heartbeat work is explicitly separate, not a fixed import baseline.
Manifest and static boundary/Types checks pass for the frozen pair;
no fresh dynamic or desktop qualification is claimed.
Confirmed shared findings and receiver implications were repaired in T562 S1.
Standalone governance actual-change review and documentation checks pass.

M5 T561 is closed after S9 coordinator actual-change review of pushed Shared
`ee7fe06ab` and NXVM `034f3eb77`. No S is active. The
[archived proposal](../history/M5-T561-core-package-split-proposal.md) and
[convergence ledger](../history/M5-T561-core-package-split.md) retain the
complete I0--I5 scope and acceptance.

| Task | Closure |
| --- | --- |
| T561 S1--S9 | Accepted: Core split into X86 Core and X86 NXVM, mirrored source/test owners, real non-PC consumer and all four PC receivers. Both widths pass independent Core 165/NXVM 215 tests, root 626 units and 48 profile integration executions, specialized gates and eight published artifact checks. |

[Final evidence](../etc/evidence/t561-s9-final-qualification.md),
[679-path disposition](../etc/evidence/t561-s9-disposition.tsv) and
[eight artifact hashes](../etc/evidence/t561-s9-artifacts.json) record the
qualified 0561 releases. All eight PC EXEs were rebuilt and published.
INI/media/ROM/snapshots and MyNES artifacts are unchanged.

The final review verifies actual checker changes, all disposition destination
hashes, eight artifact hashes, preserved source owners and independent link
graphs. S9 production delta is zero; root test enforcement is +10/-4.
Its complete execution packet remains in delivery `034f3eb77`.
Existing App tests' NXVM adapter-private coupling is explicitly retained in
TODO; it is not neutral X86 Core private-layout borrowing.

Manual physical/RDP and fresh diagnostic-ROM qualification are not claimed.
The next queued 486SX successor retains C16/C17/final qualification and awaits
owner admission; this structural completion does not claim those CPU goals.

## Previous Staged CPU Closure

| Task | Closure |
| --- | --- |
| T560 | Closed as an owner-approved bounded stage. S1--S8 remain accepted; S9 implementation `b906c7138` passes coordinator actual-change review. Complete C16 timing/runtime/preview, C17 production admission and final CPU qualification remain unmet and transfer intact to the CPU successor after T561. |

## Retained T560 Stage Baseline

[Final S9 evidence](../etc/evidence/t560-s9-selected-timing.md#final-frozen-stage-qualification):
607/607 complete repository units per width; complete specialized gates;
48 profile integration executions plus restored-default identity per width;
eight rebuilt, deployed, PE/hash-verified and debug-info-stripped 0560 PC EXEs.
Both original Release Ninja/ccache graphs were restored to default. The
PC artifact target at that delivery was `vm-0-5-0560`; its source/artifact baseline is
implementation `b906c7138`. Shared-six, MyNES 0044 and owner INI/ROM/media/
snapshot inputs are unchanged. Automated desktop-labelled tests pass; manual
RDP, full Windows GUI and PC110 qualification are not claimed.

The [T560 history](../history/M5-T560-complete-486sx-cpu.md) and
[original convergence ledger](../etc/evidence/t560-s1-cpu-convergence.md)
preserve accepted mechanisms, source/model restrictions and all residuals.
The sole selected-SX instruction recipe remains explicitly compatibility/
unallocated, and production 486SX construction remains closed.

The former [Queue](QUEUE.md) placed
[Core refactoring](../history/M5-T561-core-package-split-proposal.md) first and
[complete remaining 486SX qualification](../history/M5-T563-486sx-timing-production-completion-proposal.md)
second. Core refactoring is now admitted as T561; CPU work
stays paused until the refactor is accepted and its successor is admitted.

## Retained Historical Packets (Not Active)

### Accepted Delivery — M5 T559 S6 CPUID And Flags

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation T559 S6 after coordinator acceptance of S5 P1 8e6159de4. |
| Admission And Approval | Owner-admitted T559 and automatic sequential Core-only implementation; group I7/I8 together. No Shared or MyNES changes. |
| Objective | Freeze source-qualified selected SX CPUID/ID behavior and implement it with the existing flags owner; reconcile PUSHF/POPF, IRET and task-load masks without duplicated profile state. |
| Non-goals | No production 486 admission, invented zero-feature silicon, unproved VME advertisement, SMM/cache/TLB hardware, PC110 board, Shared or MyNES changes. Real AC/VIF/VIP hardware effects remain mandatory system gates. |
| Reference Baseline | 8e6159de4; S1 I7/I8, pinned selected-SX data sheets and original Intel manuals. Audit exact stepping applicability and current flags paths before implementation. |
| Candidate Proposal | [Selected 486SX instructions](../proposals/m5-pc110-486-instructions.md), [finite ledger](../etc/evidence/t559-s1-instruction-inventory.md), and [system receiver](../history/M5-T560-complete-486sx-cpu-proposal.md). |
| Files And ABI Surface | Core CPU existing instructions, flags definitions and profile reset only where source-qualified, directly affected CPU tests/registration, Core manifests and NXVM evidence. Preliminary production +60--160/-10--60 and tests +200--400/-0--30; refine after source/form audit. No planned public API or persistent-state addition. Rebuild eight PC EXEs if executable inputs change. |
| Applicable Rules | NXVM guide; Execution lifecycle/coverage/artifacts; Document; Architecture/Coding and their skills; NXVM architecture/layout and source/research policy. |
| Verification | Full I7 leaf/GPR/flags/privilege/LOCK/fetch/old-profile matrix; I8 PUSHF/POPF, real/protected/VM86 IRET and task-load masks, 16-bit upper-half preservation and privilege rules. Focused whole batch, actual-change review, then full x64/x86 repository units and affected artifact/INI identity checks. |
| Expected Markers | CPUID feature and ID behavior match the named candidate source contract; one flags-mask owner, retained earlier-family semantics and no production admission before system qualification. |
| Asset Needs | Existing original manuals and admitted external build inputs only; preserve owner INI/media/ROM/snapshot and MyNES. |
| Reporting Requirements | Before edits report source/stepping contract, existing flags owners, complete form matrix and refined diff. Delivery records actual diff, direct proof, system gates, artifacts and whole-batch qualification. |
| Stop Conditions | Required Shared/new cross-owner API, unresolved source contradiction or absent evidence needed to choose feature values. Record exact prerequisite and continue independent scoped work; do not guess silicon capability. |
| Exit Criteria | I7/I8 applicable instruction contracts have direct proof and retained-family regression, system-only effects remain explicit, full dual-width units and applicable eight artifacts pass, actual diff reviewed and complete P pushed. No full CPU/PC110 qualification claim. |
| Original Owner Request | Implement 486SX instruction semantics under governance; implement related groups together and avoid per-opcode full validation. |
| Similar-Issue Sweep | Audit every reserved/writable flags-mask consumer, CPUID dispatch/metadata/preview and profile-specific identity path; classify reset/control/VME effects at their real owner. No global enum widening or CPU rewrite. |

S5 is accepted and closed after actual review of pushed 8e6159de4. Its
[evidence](../etc/evidence/t559-s5-system-instruction-boundaries.md) records
retained TR transfer/form/privilege/fetch proof and 596/596 units per width.
Complete I4--I6 unavailable instruction/effect contracts remain with the system
receiver, including the existing passive 486 TR6/TR7 limitation. No production
source changed; eight S4 0559 EXEs remain current. Shared-six, MyNES and owner
inputs are unchanged. S6 is admitted for the complete CPUID/flags batch; source
review precedes edits. T559 and production 486 qualification remain open.

The proposal's batch-efficiency order applies: complete family implementation,
focused CPU validation and actual-change review before final whole-suite
qualification. External guest boot remains a T-level exit requirement.

S6 executor delivery is complete, pending coordinator acceptance. The candidate
CPUID handler and one profile-aware flags mask have focused x64/x86 proof.
The new CPU-owned test covers leaf outputs, independent code-size/prefix
combinations, old-profile rejection (without confusing 8086 POP CS with a
two-byte CPUID), truncated preview, failed instruction fetch without register
mutation, reset ID/signature, PUSHF/POPF and real/VM86 IRET AC/ID transfer.
Existing protected IRET and five task-switch completion paths now include
486 AC/ID patterns; their updated tests pass on both host widths. Existing
IRET/POPF/task regressions also passed in the preceding focused batch.
The [S6 evidence](../etc/evidence/t559-s6-cpuid-flags.md) records source-contract
reconciliation, direct proof and actual-diff review. All 597 unit cases per
width now have passing evidence: x64 first passed 594 and then passed its three
affected rechecks after correcting one old metadata assertion and its manifest;
x86 passed 597/597 in one complete run. Eight PC EXEs are rebuilt/deployed and
pass real-product INI identity, PE-width and version checks. Core manifests,
documentation and whitespace gates pass. Shared-six, MyNES and owner inputs
remain unchanged. No production 486 admission, VME hardware qualification,
manual RDP or external firmware boot result is claimed. Whole-T S7 remains.

## Closed Task — M5 T558

Owner accepted S2 delivery 7ea9084fe and approved T closure on 2026-10-13.
The nine source/checklist receivers and their bounded unresolved decisions are
accepted as research delivery. This does not assert fully measured hardware:
CPU feature/source decisions transfer to T559 S1 and the system candidate;
board, input/power, display, storage, audio and asset decisions remain mandatory
gates in their named queued receivers. Production and ten EXEs are unchanged.
The retained [history](../history/M5-T558-pc110-evidence-and-implementation.md)
records the closure review and accepted limits.

### Accepted Delivery — M5 T558 S2 PC110 Implementation Evidence And Contract Freeze

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation T558 S2, after accepted S1 research/planning delivery feadb8235. |
| Admission And Approval | Owner directs continued PC110 audit/design so every later implementation task has evidence, a checklist and original manuals or reference code. Owner subsequently selects original DSTN and upgraded TFT as two compiled variants, with mypc110_x86.exe, mypc110_x64.exe, mypc110_tft_x86.exe and mypc110_tft_x64.exe; this supersedes the briefly proposed INI model/BIOS selection. All ROMs remain external build inputs embedded in EXEs. This admits design/research, not production implementation or source import. |
| Objective | Establish per-receiver implementation evidence, finite capability/checkpoint inventories, hardware/firmware baseline and explicit unresolved decisions; audit completion against all nine planned receivers. |
| Non-goals | No production/test/Shared changes, EXE refresh, external ROM acquisition/import, BIOS bypass, guessed hardware qualification or silently excluded device. |
| Reference Baseline | NXVM feadb8235; three external pinned references in S1 ledger, revalidated unchanged at S2 admission. |
| Candidate Proposal | [PC110 programme](../proposals/m5-pc110-evidence-and-implementation.md), its nine linked receivers and S1 ownership audit. |
| Files And ABI Surface | NXVM planning, evidence, status and history only; no ABI. |
| Applicable Rules | Guide, Execution coverage/closure rules, Document rules, architecture ownership and source/research policy; architecture-governance skill. |
| Verification | Inspect source bytes and reference code, original manual identity and relevant sections; reconcile every receiver to sources, current implementation, finite checklist and proof criteria; run documentation/link/diff checks. |
| Expected Markers | No receiver relies solely on a repository name or boot screenshot; unavailable raw evidence and conflicts are explicitly classified; every unresolved decision has an owner and admission condition. |
| Asset Needs | Read existing external candidates and record roles/size/hash; no copies into source or release admission inferred from availability. |
| Reporting Requirements | Report concrete evidence changes, exact unresolved facts, completeness per receiver and unexecuted hardware/runtime verification. |
| Stop Conditions | Incompatible source derivation, unapproved acquisition, production/Shared changes or essential hardware decision lacking defensible evidence; continue independent research. |
| Exit Criteria | All nine receivers have reviewed evidence/checklists/proof gates; exact baseline choices are justified or explicitly blocked with bounded resolution, without claiming unknown behavior implemented. T closure separately reviews the programme's hardware/contract freeze requirements. |
| Original Owner Request | Execute PC110 audit/design so each later implementation task has evidence, a checklist and original manuals or reference code. |
| Similar-Issue Sweep | Verify referenced raw evidence exists; distinguish original versus reconstructed manuals, implemented versus placeholder models, firmware dumps versus runtime shadows, CPU variant claims and actual bytes. |

S1's inventory, queue plan and ownership review are accepted as design delivery,
not complete hardware freeze. S2 completed its research goal. Current ten artifacts remain
unchanged because no executable inputs change.

S2 research delivery: all nine proposals consume the reviewed
[source record](../etc/evidence/t558-s2-source-availability.md) and
[receiver reconciliation](../etc/evidence/t558-s2-receiver-checklists.md#final-research-reconciliation).
Their source/checklist/ownership/proof coverage is reviewed; exact unresolved
hardware and asset choices retain named admission gates. No implementation,
firmware preparation, runtime qualification or new artifact is claimed.
The owner accepted this bounded delivery; remaining hardware decisions transfer
explicitly to the corresponding receivers rather than being counted as proven.

### Accepted Delivery — M5 T558 S1 PC110 Research And Gap Inventory

| Field | Required record |
| --- | --- |
| Identifier Mode | New T558 S1 after T557 closure; next NXVM numeric task. |
| Admission And Approval | Owner selected PC110 on 2026-10-13, assigns all current work to M5, then requests a multi-T queue plan. T558 is revised to research/contract freeze; nine explicit queued receivers retain its implementation/product goal. Owner also authorizes external reference checkout updates and external ROM embedding into committed PC110 EXEs. |
| Objective | Establish a source-tiered PC110 hardware, asset and current-code gap ledger, current ownership map and dependency-ordered multi-T implementation plan. |
| Non-goals | No production/test code, Shared-six changes, external source/ROM import, BIOS interception, asset edits or claims of a working PC110. |
| Reference Baseline | NXVM 8f00b1bce; external research revisions and dirty-source distinction recorded in the ledger. |
| Candidate Proposal | [PC110 evidence and implementation](../proposals/m5-pc110-evidence-and-implementation.md). |
| Files And ABI Surface | NXVM proposal, status, queue, roadmap, history and research evidence only; no ABI or executable input changes. |
| Applicable Rules | Guide, Execution lifecycle/coverage rules, Document rules, architecture ownership and source/research policy; architecture-governance skill. External sources are read-only research after the explicitly authorized checkout refresh; no code import. |
| Verification | Inspect current CPU/profile/device interfaces and external source provenance; distinguish original authority from reconstruction; check documentation structure, links, actual diff and clean scope. Design-only work does not manufacture runtime tests. |
| Expected Markers | Every inventoried domain has source status, current owner, gap, next receiver and proof gate; no M6 allocation among current candidates. |
| Asset Needs | Inventory external filenames and references only; lawful profile asset set is not yet admitted. Do not copy firmware/media or treat research dumps as release inputs. |
| Reporting Requirements | State facts versus hypotheses, exact CPU absence, external-reference limitations, proposed batches, unknown revision/assets and unexecuted hardware/runtime validation. |
| Stop Conditions | Source licensing/import, undocumented hardware behavior requiring an owner choice, Shared change or expanding S1 into implementation. Continue safe research on unresolved sources. |
| Exit Criteria | Reviewed initial source/gap ledger and bounded next research/implementation gates; each implementation receiver has a proposal, dependency, owner, proof and stop condition; M5 queue reconciled, documentation checks passed and complete documentation P pushed. T558 still needs hardware/contract freeze; no PC110 runtime qualification claimed. |
| Original Owner Request | Admit entry 3: IBM PC110 research and implementation; all currently queued work belongs to M5, not M6; begin research. |
| Similar-Issue Sweep | CPU enum versus historical 486 claims; stale Common/x86/chips/ibmpc paths in PC110 plan; roadmap/queue milestone allocations; reconstructed/emulator evidence versus original sources; owner-requested four-App/Core boundary audit and PC110 ownership receivers. |

The [initial ledger](../etc/evidence/t558-s1-pc110-source-and-gap-ledger.md)
is the T convergence inventory, not a complete register specification. Original
manual, board-revision and lawful asset qualification remain research gates.
Current artifacts remain the verified T557 PC pairs and MyNES 0044 pair;
S1 changes no executable input and creates no PC110 artifact.
The [ownership audit](../etc/evidence/t558-s1-core-app-ownership.md) adds actual
four-App/Core inspection and the proposed PC110 split without admitting code
changes; current manifest and dependency checks pass, with no dynamic retest.

The owner accepts T556 S11 and requests the new PC publication/INI task.
T556's NXVM work is closed. The owner subsequently closed S10's diagnostic
delivery with its unresolved MyNES input repair transferred to
[MyNES TODO](../../mynes/states/TODO.md#rdp-short-input-repair).
`3dc750be2` is not a production fix. No MyNES implementation is
part of T557's original S1; S2's approved receiver exception is recorded below.
Prior S9/S11 packets below are historical delivery evidence.

## Closed Task — M5 T557

| Task | Status |
| --- | --- |
| T557 | Closed after accepted S2 and task-level evidence review, before owner-selected T558 admission on 2026-10-13. |

Accepted delivery: Shared `5f5f43c8c`, MyNES `08bdaf06d`, NXVM `97d3cf948`.
Both widths passed 592 repository units, 36 PC integrations and 12 MyNES
integrations, with all ten EXEs deployed. The [S2 evidence](../etc/evidence/t557-s2-cursor-ownership.md)
retains the contract, review, hashes and verification limits. This closure only
updates governance; production and executable inputs are unchanged, so the
verified binaries remain current without a rebuild. The [proposal](../history/M5-T557-pc-publication-and-cursor-proposal.md)
is archived; both original naming scope and cursor extension are covered.

## Delivered Task — M5 T557 S1 PC Publication And INI Identity

| Field | Required record |
| --- | --- |
| Identifier Mode | New T557 S1, next numeric NXVM task after accepted T556 S11 and explicit MyNES residual transfer. |
| Admission And Approval | Owner requests four product-specific lowercase EXE/INI identities, explicit App INI injection and full affected qualification. This naming contract supersedes the old machine-key artifact naming in Execution for these four Apps only. |
| Objective | Publish my5160, my5170, mydeskpro386 and nxvm release pairs and load each App's own adjacent lowercase INI through the existing Core parser. |
| Non-goals | No SoftPC access/import, Shared-six changes, MyNES changes, configuration-content/media/ROM/snapshot mutation, fallback names or machine-key renaming. |
| Reference Baseline | ed75cf572, clean after separate MyNES diagnostic publication; accepted PC implementation 6b0cb13e8. |
| Candidate Proposal | [PC publication and INI identity](../history/M5-T557-pc-publication-and-cursor-proposal.md). |
| Files And ABI Surface | Four App build/main declarations; Core Product startup signature and version; NXVM build/deployment/gates; affected Core/App tests, current documents and eight assets. Root PC preset target version changes use a separate Shared commit, with no MyNES preset change. |
| Applicable Rules | NXVM guide and Architecture/Coding/UI/source policy; shared Execution/Architecture/Coding/Document rules; architecture-governance and coding-governance skills. |
| Verification | Existing x64/x86 Ninja/ccache graphs; affected startup/config/Console tests, all repository units, four-PC integrations and static/manifest/docs gates. Prove explicit INI selection, no wrong-name fallback, INI-relative media paths, lowercase release identities, PE widths and deployment hashes. |
| Expected Markers | Eight version 0.5.0557 release files with product-specific prefixes; four matching lowercase INIs, byte-identical contents; unchanged display names and internal machine keys. |
| Asset Needs | Read existing external inputs only; create and verify new eight artifacts before removing replaced PC EXEs; preserve MyNES and owner media/configuration bytes. |
| Reporting Requirements | Per-file scope/diff and naming map, actual dual-width results, release identity proof, missing manual/RDP verification; never infer desktop acceptance from unit tests. |
| Stop Conditions | Required Shared implementation, format/behavior change beyond naming, unexplained input mutation or conflict with other work. |
| Exit Criteria | Source/build/runtime/test/docs identities agree, required tests and eight deployments pass, old duplicate releases removed, reviewed target-scoped commits pushed; await owner review. |
| Original Owner Request | Close current S and admit a new T for four PC product-named EXE/INI publication and explicit runtime configuration loading. |
| Similar-Issue Sweep | All four App declarations/callers, integration/setup fixtures, release gates/scripts/presets and current normative documents; historical text stays historical. |

### S1 Delivery

Implementation and qualification are complete: 592/592 repository units and
36/36 four-PC integration executions per width. Eight 0.5.0557 product-named
EXEs are deployed and the replaced eight filenames removed. Four renamed INIs
retain their original hashes. Both specialized gate aggregates, manifests,
registration, INI/publication and documentation checks pass. The
[delivery evidence](../etc/evidence/t557-s1-publication-ini-identity.md) records
actual diff, final hashes and limits. Shared-six and MyNES are unchanged.
S1 is delivered for owner verification; T557 remains open.

## Accepted Task — M5 T556 S11 Default Text Raster

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: next unused S11 after the MyNES S10 admission; do not execute or modify that product's pending work. |
| Admission And Approval | Owner requests default NXVM 80x25 full 8x16 text, DeskPro verification only unless a defect is proven, dual-width qualification and deployment. |
| Objective | Repair the default guest BIOS text-mode register/font geometry so boot and graphics-to-mode03 return produce 640x400, with correct cursor and raster timing; establish whether DeskPro 640x350 is correct. |
| Non-goals | No Shared change, forced Window height, host BIOS interception, external ROM or INI/media mutation; preserve MyNES and other pending changes. |
| Reference Baseline | Current master 7bfefb2fc, plus preserved S10 documentation/diagnostic work; review b2ee99b1c and b25e2ca8d and subsequent firmware changes. |
| Candidate Proposal | [T556 proposal](../proposals/m5-app-component-layout.md), owner-added S11. |
| Files And ABI Surface | Default firmware/video tests and native size diagnostic; owner-added Core CPU retirement selection and existing capture boundary, timing tests/gates and Core manifests. Timing selector becomes CPU-private; no Shared API change. Docs and affected four-PC dual EXEs. |
| Applicable Rules | NXVM guide, architecture, coding, UI and source policy; shared Execution/Architecture/Coding/Document rules; architecture-governance and coding-governance skills. |
| Verification | Real guest execution at boot and return from graphics; complete font halves, cursor, vertical active/retrace/exit; snapshot-to-Window 640x400; actual client dimensions. Both existing Ninja/ccache graphs, full repository units, all four PC profile integrations including default DOS/Windows and required static/manifest/doc gates. Actual RDP DPI proof is separately reported, never inferred from a synthetic frame. |
| Expected Markers | Default 80x25, glyph height 16 and 640x400; coherent CRTC overflow/total/display/blank/retrace and cursor registers. DeskPro retains 14/350 if sources and selected board agree. |
| Asset Needs | Existing external manuals and build inputs read-only; refresh affected four-PC 0556 x64/x86 pairs and validate PE identity after the Core repair. Preserve MyNES, adjacent owner inputs and unrelated EXEs. |
| Reporting Requirements | Record sources and timing grade, before/after reproduction, actual diff, dual-width results and release hashes; distinguish native client, outer frame and untested RDP scaling. |
| Stop Conditions | Required Shared/API change, unsupported EGA register semantics, unproven DeskPro defect, or overlap with unrelated work. Report before extending scope. |
| Exit Criteria | Correct default guest BIOS and regression proof, required qualification/deployment complete, actual-diff review and scoped commits pushed; await owner desktop acceptance, T remains open. |
| Original Owner Request | Investigate/fix default Window text geometry from guest BIOS and verify rather than normalize DeskPro 640x350. |
| Similar-Issue Sweep | Check all default text-mode initialization/restore paths, glyph upload, BDA geometry, cursor and raster counters; inspect selected DeskPro BIOS/board without changing external ROM. |

S11 implementation and final automatic qualification are complete: 592/592
units and 32/32 four-PC profile integration executions per width. All eight
PC EXEs are rebuilt and deployed; MyNES and public six remain unchanged.
The default BIOS now presents all 16 font rows. Its exposed Core TIME fault
is repaired by selecting completed-instruction timing before interrupt entry
reuses decoder state. The actual-Core regression fails against admission
objects and passes with the repair. DeskPro retains its correct 14/350 mode.
Native remote-session observations no longer show the TIME failure, but the
diagnostic pause acknowledgement and owner phone-client DPI/interaction are
not qualified by automation. The owner has accepted this scoped delivery;
S11 is closed. Preserve the separately retained MyNES S10 pending work. See the
[S11 evidence](../etc/evidence/t556-s11-default-text-raster.md).

## Delivered Task — M5 T556 S9 Audit Repairs And Qualification

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: S9 follows completed S8 audit and accepted S1--S7. |
| Admission And Approval | Owner's 2026-10-13 request assigns necessary audit repairs to S9. Targets are NXVM and MyNES, with separate commits. The proposal's existing Shared root-build receiver covers only three PC preset target names changing from 0555 to 0556; public six and shared rules remain excluded. |
| Objective | Flatten DeskPro source and matching unit tests under the owner's S9 amendment; repair A1--A5 under that revised ownership, retain A6's separate Td receiver and complete five-App qualification. |
| Non-goals | No Shared-six or shared-rule edit, unrelated hardware rewrite, new runtime manager, persisted-format change or owner-input mutation. |
| Reference Baseline | S8 audit `a28cac691`; accepted S7 implementation `307cf1bd6`. |
| Candidate Proposal | [Five-App component layout](../proposals/m5-app-component-layout.md). |
| Files And ABI Surface | Core test registration/owner gates; all DeskPro source moved to its App root and units to its unit root, with includes/guards/symbols/build/gates/docs repaired; moved MyNES Machine symbols/callers; corresponding product version/artifact receivers. No Shared API change. S9 actual diff is recorded against the amended flat file map. |
| Applicable Rules | Product guides; shared Execution, Architecture, Coding and Document rules; both product architecture/layout authorities; source policy; architecture-governance and coding-governance skills. |
| Verification | Correct labels/names without reducing cases; reject incomplete D4 attachment with rollback proof; run dual-width full units and every admitted profile/MyNES integration in original Ninja/ccache trees; verify manifests, dependency/doc gates and affected release pairs. |
| Expected Markers | No absolute-path or mislabeled Core tests; no DeskPro chips/machine/product source directories or matching unit nesting; one D4 state owner and one Core attachment owner; missing alias rejects atomically; documents match current source; complete per-route results and ten qualified artifacts. |
| Asset Needs | Existing BYOB inputs read-only; update affected executable pairs only, preserving INIs, media, ROM originals and snapshots. |
| Reporting Requirements | Report per-target actual added/deleted/net lines, retained owners, exact test coverage, artifact identities/hashes, failures and unexecuted desktop checks. |
| Stop Conditions | Required Shared-six/rule change, incompatible saved data, unexplained behavior change or newly expanded hardware scope. Report rather than patch outside this packet. |
| Exit Criteria | A1--A5 repaired with evidence; A6 explicitly transferred; required dual-width builds/tests and artifact checks complete; actual changes reviewed, per-target P commits pushed, clean worktree; T closure still requires final acceptance. |
| Original Owner Request | Admit S8 for comprehensive governance, documentation and code-quality audit; admit S9 for necessary repairs. |
| Similar-Issue Sweep | Search all five App owners and Core plus current authorities for the same stale ownership, registration, vocabulary and contract defects; retain historical terminology as history. |

### S8 Audit Delivery

The [audit ledger](../etc/evidence/t556-s8-quality-audit.md) records six
confirmed classes and their S9 disposition. Both widths pass the 18 selected
static gates; both documentation checks and the production dependency DAG pass.
No new full dynamic qualification is claimed. Production, tests and assets
remain unchanged. Shared-rule vocabulary retains a separate Td receiver.

| Task | Progress |
| --- | --- |
| T556 S9 | Delivered for owner review: 592/592 units and 44 profile/MyNES integration executions per width, both specialized gate targets, documentation/dependency/manifest checks and ten release artifacts. T556 remains open. |

## Retained S9 Delivery

The [S9 delivery report](../etc/evidence/t556-s9-quality-repair-delivery.md)
records exact scope, actual diff, logs, artifact hashes and unperformed manual
acceptance. Four PC Apps retain 0.5.0556 under their flat asset roots; MyNES
retains its independent 0.0.0044 pair. The PC target is `vm-0-5-0556`.
MyNES receiving commits are `7fcd7b1d9` and `c5ca06e85`; the NXVM commit
containing this packet/report identifies its delivered source and eight EXEs.
Root preset names use a separate Shared-target commit; the public six are unchanged.

## Historical Evidence

Prior packets remain in the [retained status archive](../history/M5-T556-retained-status.md).
They are not concurrent active tasks. The [T556 history](../history/M5-T556-app-component-layout.md)
records accepted layout work, S11 closure and the explicit MyNES residual.
