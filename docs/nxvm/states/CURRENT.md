# Project Status

## Current Work

M5 T561 remains open. S1 is accepted after coordinator actual-change review of
pushed Shared `15d86fc8e` and NXVM `6c95c2da3`. S2 is accepted after coordinator
actual-change review of pushed `82e3257cc`. S3 CPU/FPU extraction is accepted
after actual-change review of pushed Shared `8bb5bcaa5` and NXVM `039d946ab`.
S4 PC metadata/default/trace separation is accepted after actual-change review
of pushed `f136911be`. Neutral Machine extraction is next; CPU qualification
does not resume. S5 is admitted for neutral advancement and interrupt wiring,
before address-decode separation and the actual Machine move.
The [proposal](../proposals/m5-core-x86-core-nxvm-split.md) and
[convergence ledger](../history/M5-T561-core-package-split.md) retain I1--I5.

## Active Packet — M5 T561 S5

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation T561 S5 after accepted S4 `91046e0f8`; one-session coordinator/executor. |
| Admission And Approval | Owner-approved complete Core split and automatic sequential S admission; NXVM source/test/checker/docs/eight artifacts only. No Lib/Emulator, imported package, CPU/FPU or MyNES changes in this prerequisite. |
| Objective | Consume I3 P4/P6: remove PIT/PIC, media and RTC progression slots from neutral execution; Board owns its device order through two explicit advancement boundaries around internal CPU/FPU readiness. CPU consumes neutral interrupt pending/acknowledgement rather than PIC-named slots. |
| Non-goals | No directory move yet, A20/decode implementation (next S), CPU algorithm/timing grade, C16/C17, device waveform, second scheduler, callback registry, persisted temporary clock state, Lib/Emulator edit or sibling import. |
| Reference Baseline | Clean accepted `91046e0f8`; S2 P4/P6 and S4's exact original timing/trace policies and eight 0561 artifacts. |
| Candidate Proposal | [Core split](../proposals/m5-core-x86-core-nxvm-split.md), [S2 contract review](../etc/evidence/t561-s2/review.md), [T561 ledger](../history/M5-T561-core-package-split.md). |
| Files And ABI Surface | Core attachment/scheduler/CPU bus, Board advance/binding/declarations, directly affected tests and ownership checkers, Core manifests/docs/eight PC EXEs. Two bounded before/after-internal advance callbacks replace named PIT/media/RTC/peripheral slots; Board privately combines contiguous device work. Remove the PIT-pair public type. Neutral interrupt slot names replace PIC names without aliases. Preliminary production +110--200/-150--260, tests +120--240/-70--160, checker/registration +20--70/-20--70; refine from actual changes. No new state or thread. |
| Applicable Rules | Execution/Architecture/Coding/Document, NXVM design Architecture/Coding and source policy; architecture-governance then coding-governance. Preserve one timeline/bus/executor, inward dependency, Types vocabulary, mirrored test owners and existing closed CPU eligibility. |
| Verification | Original dual-width Ninja/ccache graphs, build -j8/test -j4. Focused complete advancement/clock/interrupt/arbitration/deadline/attachment batch first, actual diff review, then complete repository units/specialized gates and required four-profile integrations; rebuild/deploy eight 0561 artifacts and verify real published CLI/INI/PE/version/hash. No alternate graph or reduced timeout/assertion. |
| Expected Markers | Core names no PIT/PIC/media/RTC progression slot or PIT-pair representation. Timeline then bus/refresh/DMA then Board PIT/PIC/media then internal FPU/WAIT then Board RTC/input/video remains the causal order. A readiness source cannot enter earlier same-tick PIC arbitration. Bus HOLD/READY/prefetch ownership stays Core. IRQ sampling/ack is one path. |
| Asset Needs | Existing approved external firmware/media inputs only; preserve owner INI/ROM/disks/snapshots and MyNES assets. Rebuild eight PC EXEs (four dual-width pairs), not unrelated MyNES artifacts. |
| Reporting Requirements | Before implementation record real owners, clock-conversion placement and call-order proof, file scope and estimates; report actual added/deleted/net lines, retained and deleted mechanisms, full qualification and unexecuted manual boundaries. Complete implementation P pushed before separate coordinator acceptance. |
| Stop Conditions | Changed guest ordering/timing, observable difference from local clock conversion relocation, lost refresh/DMA ownership, outward dependency, new state/registry/thread, necessary Lib/Emulator/CPU change or unrelated conflict. Revise design rather than mask failure. |
| Exit Criteria | P4/P6 whole batch has explicit neutral contracts, Board device order and regression proof on both widths; applicable complete qualification/artifacts/manifests/docs pass, actual diff reviewed and delivery/acceptance pushed. I3 extraction and P7/P8, I4/I5 remain open. |
| Original Owner Request | Execute complete Core-to-X86-Core/X86-NXVM split, keep Product separate, and mirror all source/test component hierarchies. |
| Similar-Issue Sweep | All attachment initializers/callers, reset/deadline/idle/run paths, clock conversions and readiness/trace order, PIC pending/ack and CPU interrupt sampling; checkers and tests must enforce the real owner, not obsolete spelling. Deferred A20 and cross-package private fixtures remain explicit later ledger items. |

## Delivered S5 Advancement Boundary

[Frozen S5 proof](../etc/evidence/t561-s5-neutral-advance.md) and its
[21-file review hashes](../etc/evidence/t561-s5-file-review.tsv) record the
two neutral advancement callbacks, Board-owned device order and neutral IRQ
sampling/acknowledgement. Production C/H is +64/-111 (net -47); no new state,
thread, scheduler, CPU algorithm or timing grade is introduced.

Both original graphs pass 627/627 repository units, specialized gates and
48 receiving integrations per width plus restored-default identity. All eight
rebuilt/deployed 0561 PC EXEs pass actual CLI/INI/PE/version/hash checks.
Lib/Emulator, imported packages, CPU/FPU, MyNES and owner inputs are unchanged.
Manual RDP/physical/full Windows GUI qualification is not claimed. Coordinator
actual-change acceptance remains required; T561 and I3 extraction/I4/I5 stay open.

## Accepted S4 PC Policy Separation

[Frozen S4 evidence](../etc/evidence/t561-s4-pc-policy-separation.md) and
[170-file hash review](../etc/evidence/t561-s4-file-review.tsv) record the moved
30-row Board timing contract, seven producer-owned trace IDs and PC CPU/RAM
defaults. One Board table replaces the executor table; no duplicate state,
thread, scheduler, import or CPU algorithm change is introduced. Original
table values, plan rules, deadline calculations and emission order are checked
against accepted S3 after the declared owner substitutions.

Both original Ninja/ccache graphs pass 627/627 repository units, all specialized
gates, 48 receiving integration executions per width plus restored-default
identity, and all eight published CLI/INI/PE/version/hash checks. The eight
0561 PC EXEs are rebuilt and deployed; both graphs return to default. Imported
packages, X86 Core CPU/FPU, Lib/Emulator, MyNES and owner inputs are unchanged.
Manual RDP/physical/full Windows GUI qualification is not claimed. Coordinator
actual-change review accepts pushed `f136911be` against the packet, original
request, moved policy bodies, tests, frozen hashes and published assets. The
full S4 packet remains in that implementation revision. I3 extraction, I4 and
I5 remain open; T561 stays open and no implementation runs without the next packet.

## Accepted S3 CPU/FPU Extraction

[Actual-change review](../etc/evidence/t561-s3/review.md) verifies 12 moved
implementation/header files and 106 test files, six CPU-local checkers, all
438 changed source/test/build/tool hashes and the 511-name ownership map.
Production C/H is mechanically equivalent, net zero lines; two test-only
utility dependency removals yield net -3 test C/H lines. No CPU algorithm,
table, layout, grade, new profile, worker or scheduler changes. The original
CPU/FPU paths and targets are retired; new src/test hierarchies match.

Both original graphs pass 627/627 complete repository units, including all
112 X86 Core owner cases, and all specialized gates. Four-profile integrations
pass 48 executions per width plus restored-default identity. All eight
optimized/stripped 0561 PC Assets EXEs pass PE/version/hash and published
INFO/SPEED/HELP/EXIT/INI checks. Both graphs return to default; persistent
ccache is preserved. Imported ten trees, Lib/Emulator, MyNES and owner inputs
are unchanged. Manual RDP/physical/full Windows GUI qualification is not claimed.

The coordinator reviewed both pushed implementation commits and actual frozen
bytes, not only the test report. I2 is discharged as CPU/FPU extraction. The
recorded execution/board white-box test receivers remain explicit S5/S6 work;
they are not a claim of final package-boundary completion. I3--I5, planned
S4--S7 and T560 C16/C17/final qualification remain open. T561 remains open.
The complete S3 admission packet is retained in `039d946ab`'s CURRENT revision.

## Accepted S2 Inventory

[Disposition evidence](../etc/evidence/t561-s2/review.md) covers all 679 tracked
Core source/test paths, declaration/include records, affected registrations and
both original configured graph witnesses. Reproduction, Core source/test
manifest, corpus and documentation checks pass. The actual P1 review confirms
14 NXVM documentation files only (+25,912/-9 lines, chiefly generated records);
source/test/build/artifact delta is zero. No dynamic tests or rebuild are
claimed for this design prerequisite; S1 remains the runnable baseline.

The two governance skills made ownership and real dependency evidence precede
renaming. PC qualification/default/trace and device progression remain explicit
separation work, with no new public API implemented. Proposed S3--S7 retain
five bounded extraction/qualification batches. I1 planning is discharged;
I2--I5 and CPU C16/C17/final qualification are not. Full S2 admission is retained
in `82e3257cc`'s CURRENT revision. T561 remains open.

## Accepted S1 Import And Runnable Baseline

[S1 evidence](../etc/evidence/t561-s1-softpc-ten-tree-import.md): all 264 files
in the five source/test pairs match pinned SoftPC `3af1955932fcb9e61ceec3456493580e316a12c9`.
Old src/test/x86 is retired and Core uses the split contracts, with no alias
or parallel runtime. Lib, Emulator and CPU implementations remain unchanged.

Both widths pass 621/621 complete repository units and all specialized gates,
48 profile integration executions plus restored-default identity, and the
strengthened INFO/SPEED/INI checks against every published PC EXE. Final Core
manifest/corpus/factory/startup checks pass 5/5 per width. Automated desktop
cases pass; manual RDP, full Windows GUI and PC110 acceptance are not claimed.

Eight optimized, stripped, PE/hash-verified 0.5.0561 PC EXEs are deployed in
the four existing Assets roots. Source/artifact commit is `6c95c2da3`; current
target is `vm-0-5-0561`. Both original Release Ninja/ccache graphs are restored
to default and the persistent cache is retained. MyNES 0044, INI, ROM, media
and snapshot inputs are unchanged. The full admission packet remains in the
P2 revision of this file. T560's CPU residual successor remains paused.

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
[Core refactoring](../proposals/m5-core-x86-core-nxvm-split.md) first and
[complete remaining 486SX qualification](../proposals/m5-486sx-timing-production-completion.md)
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
