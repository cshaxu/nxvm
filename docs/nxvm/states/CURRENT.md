# Project Status

## Current Work

T560 remains open after accepted S7 implementation 0059bfd51. S7 is closed
after one-session coordinator review of the actual pushed source, tests,
interfaces, artifacts and source/model limits. Its C11/C12 functional batch
has direct CPU and Core proof: SRESET, physical SMM/RSM, special-cycle RDY,
HOLD/LOCK ordering, Stop Grant/qualified CLK and held/deferred FLUSH.

Final full repository units pass 604/604 per width in the original
Ninja/ccache graphs (45.70 s / 49.56 s). The subsequent fixture-only output
cleanup is separately rebuilt/rechecked, 2/2 per width; it is not another
full-suite execution. Both complete specialized aggregates and all eight
profile/width integration/release groups pass. Eight verified stripped 0560
PC EXEs are deployed. MyNES remains at 0044; public-six, MyNES source/binaries
and owner INI/media/ROM/snapshot inputs are unchanged.

The [S7 evidence](../etc/evidence/t560-s7-smm-system.md) records the complete
event matrix, actual diff, source/model qualifications, failures and final
artifact identities. Reserved save-image encoding, ambiguous DR7 and relocated
selector behavior retain explicit L2 limits. Physical pin/PLL timing stays
with S9 and board qualification; no fitted PC110 silicon claim is inferred.
No manual RDP or Windows 3.1 GUI qualification is claimed.

There is no active S packet between acceptance and the next admission.
S8 full inherited semantics, S9 timing and S10 final Core admission remain
mandatory. Production 486 construction stays closed. The persistent complete
486SX implementation objective is not yet achieved.

The prior accepted cache/bus baseline remains supporting evidence:
The [S5 source/route investigation](../etc/evidence/t560-s5-cache-bus.md)
records the original cache/test-register contracts and concrete A20,
per-access paging-attribute and external-write coherence gaps.
Ordinary CPU accesses and TR3--5 commands now share actual cache state,
with mode/privilege/fetch/failure, replacement, PCD/PWT, A20, route and snoop
proof on both widths. INVD/WBINVD deliver ordered Core cache operations.
Page-table A/D updates use authoritative second reads and a single memory
LOCK boundary shared with explicit RMW and implicit memory XCHG. Real Core
tests prove DMA exclusion, changed-entry targets, cached-entry reconciliation,
split-page failures and retained accepted effects. All accepted LOCK handlers
have been reviewed against their common RMW entry; no second arbiter exists.
The accepted S5 snapshot passes 601/601 full repository units per width in the
original Ninja/ccache graphs. All eight final profile/width integration groups
pass and eight verified 0560 PC EXEs are rebuilt and deployed. C16 pin/clock
fidelity remains in the timing batch; the current remaining obligations are
summarized above. No complete CPU or production 486 admission is claimed.
S4 whole units passed 600/600 per width, all eight final profile integration
groups pass and eight verified 0560 EXEs are deployed.
The [complete CPU proposal](../proposals/m5-pc110-486-system.md) and
[convergence ledger](../etc/evidence/t560-s1-cpu-convergence.md) retain all
remaining CPU obligations. Production 486 admission remains closed.

The owner permanently approved routine task-version synchronization of build
targets, presets and release configuration on 2026-10-14. This is not approval
for unrelated Shared functionality or APIs; the [task history](../history/M5-T560-complete-486sx-cpu.md)
records the exact boundary. Do not ask again for equivalent version updates.


| Task | Progress |
| --- | --- |
| T560 S7 | Accepted and closed: 0059bfd51. C11/C12 source-qualified SMM/RSM and system/power proof; final 604/604 units per width plus fixture-only rechecks, both specialized aggregates, eight passing integration groups and eight verified deployed 0560 PC EXEs. Exact timing, full inherited semantics and final admission remain S8--S10. [Evidence](../etc/evidence/t560-s7-smm-system.md). |
| T560 S6 | Accepted and closed: 926bfe185. Selected feature/VME/debug/exception/SX-FPU and carried C05 entry proof at documented grades; no new public API or persistent field. Final 602/602 units per width, both complete specialized aggregates, eight passing integration groups and eight verified deployed 0560 PC EXEs. S7--S10 and production admission remain open. [Evidence](../etc/evidence/t560-s6-feature-exceptions.md). |
| T560 S5 | Accepted and closed: 33cb2efe1. Actual cache/TR/operations/snoop/page-control, second-read/locked A/D and implicit/explicit LOCK proof; 601/601 full units per width, eight passing integration groups and eight verified deployed 0560 PC EXEs. [Evidence](../etc/evidence/t560-s5-cache-bus.md). Remaining CPU obligations stay in T560. |
| T560 S4 | Accepted and closed: c32103a24. Translation/test-command and invalidation proof, 600/600 full units per width plus final affected-test recheck, eight passing integration groups and eight verified 0560 artifacts. [Evidence](../etc/evidence/t560-s4-translation-cache.md). Internal L2 grades and remaining C04/C16 bus/CPU obligations remain explicit; production 486 admission stays closed. |
| T560 S3 | Accepted and closed: cb52586a9. Full units pass 599/599 per width, all eight final PC integration groups pass and eight verified 0560 EXEs are deployed. [Batch evidence](../etc/evidence/t560-s3-alignment-rmw.md). S4--S10 and C13-linked exception-entry qualification remain in T560; production 486 admission stays closed. |
| T560 S2 | Accepted and closed: e7c3f4e78 / ddb260095. Base system tests pass 597/597 per width; all eight integration groups pass and eight 0560 EXEs are published. See the [S2 evidence](../etc/evidence/t560-s2-base-system.md). Complete CPU qualification and planned S3--S10 remain open in T560. |
| T559 | Closed by owner approval: instruction-delta qualification, 597/597 units per width, eight final external groups and eight verified 0559 PC EXEs. Remaining CPU system semantics transfer to the approved next task. |

The [delivery evidence](../etc/evidence/t559-s7-instruction-qualification.md)
and [task history](../history/M5-T559-486sx-instruction-semantics.md) record
actual-diff review, sources, hashes and verification. Shared-six, MyNES and
owner inputs are unchanged. No manual RDP qualification is claimed.
The [instruction proposal](../proposals/m5-pc110-486-instructions.md) and
[finite ledger](../etc/evidence/t559-s1-instruction-inventory.md) retain the
bounded T contract; the [system receiver](../proposals/m5-pc110-486-system.md)
still owns real cache/TLB/test commands, reset/control, AC/VME/SMM and timing.
Production 486 admission stays closed. The persistent full implementation
objective is not complete.


### Accepted Delivery — M5 T559 S6 CPUID And Flags

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation T559 S6 after coordinator acceptance of S5 P1 8e6159de4. |
| Admission And Approval | Owner-admitted T559 and automatic sequential Core-only implementation; group I7/I8 together. No Shared or MyNES changes. |
| Objective | Freeze source-qualified selected SX CPUID/ID behavior and implement it with the existing flags owner; reconcile PUSHF/POPF, IRET and task-load masks without duplicated profile state. |
| Non-goals | No production 486 admission, invented zero-feature silicon, unproved VME advertisement, SMM/cache/TLB hardware, PC110 board, Shared or MyNES changes. Real AC/VIF/VIP hardware effects remain mandatory system gates. |
| Reference Baseline | 8e6159de4; S1 I7/I8, pinned selected-SX data sheets and original Intel manuals. Audit exact stepping applicability and current flags paths before implementation. |
| Candidate Proposal | [Selected 486SX instructions](../proposals/m5-pc110-486-instructions.md), [finite ledger](../etc/evidence/t559-s1-instruction-inventory.md), and [system receiver](../proposals/m5-pc110-486-system.md). |
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
