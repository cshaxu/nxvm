# Project Status

## Current Work

## Current Task — M5 T548 (S33 Active: Shared Boundary And Manifest Canonicalization)

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation T548 S33. |
| Admission And Approval | Owner approved importing SoftPC's latest six shared trees into NXVM, repairing banner ownership locally, then correcting the discovered component-boundary and manifest-generation defects. |
| Objective | Keep each shared component's verifier within its own source/test corpus; make all six shared manifests deterministic and byte-identical for identical corpus content; retain `emulator/product` as the sole banner formatter. |
| Non-goals | No SoftPC edit; no cross-component verifier; no product-specific branch in Emulator; no second banner path; no external asset or firmware change; no unapproved public contract expansion. |
| Reference Baseline | NXVM `b2f4328a1`; SoftPC `f357b291831d7aee49e8e9f86650b23b10e455fb` plus its owner-visible working-tree six-tree Console update. |
| Candidate Proposal | `proposals/m5-unit-test-ownership-consolidation.md`, together with the approved shared Product banner ownership decision. |
| Files And ABI Surface | Shared six source/test trees and their manifests, the three existing manifest verifiers, Lib's naming verifier, and necessary App identity bindings. No public runtime API changes. |
| Applicable Rules | NXVM guide; Execution, Architecture, Coding, Documentation and source policy rules; SoftPC is read-only. |
| Verification | Verify every manifest header, lexical path order and hash; prove Lib's naming verifier no longer visits Emulator/Product/App paths; run the six shared suites on x64/x86. |
| Expected Markers | Every verifier has one component-local corpus; equal shared content yields equal manifest text; Product has one opening formatter in `emulator/product`. |
| Asset Needs | Existing authorized build-time profile assets only. |
| Reporting Requirements | Report imported/repaired paths, interface delta, exact build/test results, artifact hashes and desktop/external tests not run. |
| Stop Conditions | A required upstream input cannot be read, an import requires provenance review, a public contract conflict lacks an owner decision, or a test failure identifies a distinct behavioral defect. |
| Exit Criteria | Six trees are adopted, banner ownership is correct, manifests and gates pass, x64/x86 qualification passes, ten artifacts are rebuilt and verified, and committed/pushed evidence records the bounded result. |
| Original Owner Request | Import SoftPC's latest six shared trees; repair banner owner after import; then correct component-local verifier ownership and manifest generation after the diff audit. |
| Similar-Issue Sweep | Scan all shared manifest verifiers and all Lib-owned gates so no component scans peer code or task-specific metadata prevents byte-identical imports. |

| Work | Progress |
| --- | --- |
| T548 S33 | Complete pending owner review: adopted the selected SoftPC Console and shared Product repairs, then imported SoftPC's three strengthened shared tests (Console metadata/viewport failure, Emulator monitor fault/lifecycle, Product Surface snapshot fault). Lib's naming verifier now scans only Lib source/test corpus; all six manifests use the stable, lexically sorted `sha256-manifest-v1` format. Shared x64/x86 suites pass 66/66 each. This test/governance-only follow-up does not change runtime binaries; no desktop or external runtime qualification was run. |
| T547 | Closed at owner direction. Final Shared/NXVM/MyNES commits are `5b1d07412`, `08a6a19f4` and `6be3ce8ee`. Public suites pass 66/66, Core 221/221, NXVM 511/511 and MyNES App 45/45 on x64/x86; all ten deployed artifacts are current. No new manual desktop or external integration qualification is claimed. |
| T548 S1 | Complete: committed baseline `006a7592d` records the 541-unit-entry/45-integration-C universe, registration owners and first semantic candidate groups. It confirmed the Default-App factory duplicate and protected selected-profile increments from mechanical relocation. |
| T548 S26 | Complete: Core `c47e7c848`, MyDeskPro386 `41cc1779b`, My5170 `b67332ae1`, My5160 `113514a57`, NXVM `1209e0f35` and Shared registration `2de340bf3` retire or narrow Default-PC duplicates, assign generic predicates to Core and split every outward Core-to-App test edge to its real App receiver. The complete repository-only unit selection passes 515/515 on x64 and x86; Core manifests pass on both widths. No production, firmware, media, INI, snapshot or deployed executable input changed. |
| T548 S27 | Complete pending commit: all two My5160 and six My5170 unit entries are classified in `etc/evidence/t548-s27-my5160-my5170-ownership-ledger.md`. Every assertion selects a fixed profile, firmware, topology, clock or composed route that Core does not own; none is rehomed or retired. Two raw-CRT/duplicate-include test-boundary cleanups compile and the complete eight-entry App batch passes on x64/x86. No production, asset, firmware, media, INI, snapshot or executable input changed. |
| T548 S28 | Complete pending commit: all 25 Model 40 unit entries are classified in `etc/evidence/t548-s28-mydeskpro386-ownership-ledger.md`. Every assertion selects D4, CMOS, ROM, media geometry, CECG or a Model 40 composed route that Core does not own; none is rehomed or retired. Twenty-five raw-CRT test boundary cleanups compile and all 28 MyDeskPro386 routes pass on x64/x86. No production, asset, firmware, media, INI, snapshot or executable input changed. |
| T548 S29 | Complete pending commit: `etc/evidence/t548-s29-shared-monitor-ownership-ledger.md` assigns the shared monitor behavior by contract. Lib retains host-console mechanics; Emulator Session retains reader/prompt/result delivery; Emulator Product retains fixed grammar/help/lifecycle formatting; Product Surface retains IBM-PC adapter/debug/hotkey binding. MyNES retains its NES execution and provider increments. No equal-or-stronger duplicate exists, so no C/H, CMake, manifest, asset or executable input changes. |
| T548 S30 | Complete pending commit: `etc/evidence/t548-s30-mynes-unit-ownership-ledger.md` classifies all 44 MyNES unit entries. The 40 Core entries retain 6502/NES peripheral, cartridge/mapper, media/snapshot and complete-machine contracts; the four Product entries retain MyNES configuration, concrete composition, battery-failure and provider binding. No equal-or-stronger Shared receiver exists, so no C/H, CMake, manifest, asset or executable input changes. |
| T548 S2 | Complete: retired `nxvm_ini_smoke.c` and both `vm-app-ini-smoke` registrations. Reconfigured x64/x86 CTest graphs contain only `core.factory`; it passes on both widths. Core ownership and manifest gates pass. This is test/CMake-only, so no executable input or artifact changed. |
| T548 S3 | Complete: pushed as `ac7fbc5dc`. The unmodified eight-by-five Core CPU/PIC negative matrix now has one `core.cpu-bus-boundary-negative` registration, runs on x64/x86, and the old App identity is absent. The FDC assembly gate remains App-owned. Core ownership and both Core manifests pass. No production or artifact input changed. |
| T548 S4 | Complete: the mixed Default-App plan test is split into one Default-PC, one My5170 and one MyDeskPro386 receiver; the 5170 clock contract also moves to My5170. All four targets pass on x64/x86, and no old mixed path/target remains in active test/CMake sources. This is test/CMake-only, so no executable input or artifact changed. |
| T548 S5 | Complete: committed ledger `etc/evidence/t548-s5-test-identity-ledger.md` distinguishes 26 real active source-path markers, 67 explicit success-marker files and broad CMake lexical matches. It allocates separate target-scoped follow-up batches and leaves historical evidence untouched. |
| T548 S6 | Complete: pushed as `6624da0af`. Twenty MyDeskPro386 Model 40/D4 task-suffixed source paths, registrations and task-formatted markers now use behavior identities only. All 20 affected tests pass on x64 and x86; Core boundary verification passes. No production, artifact, firmware, media or assertion changed. |
| T548 S7 | Complete pending commit: six Default-PC source paths and their CTest registrations now use behavior identities only; all 37 current Default-PC test files have task provenance removed from active output. The six replacement targets pass on x64 and x86; all changed source files compile on both widths. An unrelated full-build failure in `integration-session-ini-support` still references retired `app_composed_machine.machine` and is retained for a later receiver. |
| T548 S8 | Complete pending commit: four My5170 unit sources now emit behavior-only success or diagnostic markers. The three success-bearing targets and the composition diagnostic receiver pass on x64 and x86. |
| T548 S9 | Complete pending commit: the My5160 profile test's eight task-formatted success markers now use XT behavior identities. Its target passes on x64 and x86. |
| T548 S10 | Complete: the evidence ledger classifies live unit/corpus verification helpers as semantic receivers, retains historical evidence/provenance and excludes T515/T533 integration helpers. S11 owns the finite behavior-name migration. |
| T548 S11 | Complete: strict CPU compilation, fixed-width vocabulary and fixture-lifecycle verification now have behavior identities. The prior lifecycle verifier's five retired paths and mismatched chip-local comparison key are corrected; all three targets pass on x64/x86. |
| T548 S12 | Complete: undefined-opcode source discovery and delivery-disposition verification uses behavior identities and passes in both configuration graphs. Its exact owner/disposition predicate is unchanged. |
| T548 S13 | Complete: unit-registration verification uses behavior identities, no longer requires the nonexistent `ibmpc-build-smoke`, and no longer reads the same Core CTest file twice. It reports 334 real routes on x64/x86. |
| T548 S14 | Complete: fixture-shape verification now has behavior identities, resolves only current test paths and passes on x64/x86. It preserves the 101 historical identities, removes duplicate Glob/explicit inputs, and classifies the two later Core direct constructors, for 135 current constructors. No test assertion, integration route, production code or artifact input changed. |
| T548 S15 | Complete: strict-declaration uniqueness verification now has behavior identities for its file, target, variables and generated matrix. Its exact 532 target-option declaration predicate passes on x64/x86; direct-compilation/T345 consumers remain untouched. No production code, artifact input or CTest route changed. |
| T548 S16 | Complete: repaired the stale NXVM integration session helper to initialize `app_composed_machine` and read its current `composition.machine` member rather than the retired direct field. The existing 500-row direct strict-compilation matrix passes on x64 and x86 (497 retained strict, 3 deferred). No production code, CTest route, asset input or executable artifact is retained by this test-only S. |
| T548 S17 | Complete: direct-compilation and deferred-ownership verifier filenames, targets, variables, generated matrices and diagnostics now use behavior identities. The formerly unpopulated residual ledger now declares the three actual deferred Core Product source rows, allowing the existing exact-ownership predicate to run. The matrix now retains the generated firmware source even in a fresh build tree: fresh x64/x86 CMake graphs prove 500 matrix rows (497 strict, 3 deferred), 151 ownership rows (148 owner tests, 3 residual product entries), the duplicate-row negative self-test and 151-command warning audit. No production or C test source changed; generated executable changes were discarded. |
| T548 S18 | Complete pending commit: CPU timing inventory/source/seam verifier filenames, targets, variables and diagnostics now use behavior identities. Their historical T359/T360/T435 evidence paths and required provenance anchors remain unchanged. Fresh x64/x86 Ninja graphs prove all three exact predicates and the old target names are absent. No production or C test source changed; no executable artifact is retained. |
| T548 S19 | Complete pending commit: successful-sentinel, physical-timebase, physical-eligibility and residual-form verifier filenames, targets, variables and diagnostics now use behavior identities. Their T388 evidence files and required evidence markers remain unchanged. Fresh x64/x86 Ninja graphs prove all four exact predicates and the old target names are absent. No production or C test source changed; no executable artifact is retained. |
| T548 S20 | Complete pending commit: the Jcc target-lexeme verifier filename, target, local variables and diagnostics now use a behavior identity. Its T388 evidence file and required success-marker anchor remain unchanged. Fresh x64/x86 Ninja graphs prove the unchanged predicate. No production or C test source changed; no executable artifact is retained. |
| T548 S21 | Complete pending commit: the 80286 Appendix-B, 80286 LSL reconciliation and 80386 LSL granularity verifier filenames, targets, variables and diagnostics now use behavior identities. Their T388 evidence files and required success-marker anchors remain unchanged. Fresh x64/x86 Ninja graphs prove all three unchanged predicates and the old target names are absent. No production or C test source changed; no executable artifact is retained. |
| T548 S23 | Complete pending commit: generic fixtures now have narrow Core owners, App-private profile/ROM helpers have local owners, and six multi-profile routes are visibly Core Machine qualification. Four App test-support boundary checks reject peer-App support includes. Fresh x64/x86 complete Ninja graphs pass (1,098/1,337 steps); each full unit suite passes 512/512. Core manifest and whitespace checks pass. No production assertion, source, artifact input or integration route changed. S24 remains the semantic receiver for mixed Core/D4 assertions. |
| T548 S24 | Complete pending commit: the three Default-App mixed board tests now have one owner per assertion. Generic machine time and DMA/hold competition proof is in Core; the distinct Model 40 D4 refresh-deadline and Port-B rollback proofs are in MyDeskPro386. Existing Core Port-B rollback and Model 40 D4 platform receivers retain the assertions that were already canonical, so no duplicate generic receiver remains. The four split routes, Core manifests and fixture-shape gate pass on x64/x86. No production, artifact or integration input changed. |
| T548 S25 | Complete: pushed as `5b9aaca83`. Four pure CPU timing-manifest runners, their generated-result consumers, three CPU decoder inventory runners and the FDC boundary negative check now have Core ownership. The runners link only their actual Core libraries, not Model 40/D4 as a historical aggregator. Their twelve exact routes pass on x64/x86; Core manifests, fixture-shape and registration gates pass, retaining 335 registered unit routes. No production, artifact or integration input changed. |
| T547 S5 | Closed at owner direction. P1 `9131545d8` replaces the live Product with the SoftPC command/keyboard base and App extensions; P2 `09cbfcf93` removes the SoftPC-branded fault message; P3 `3ff1e89f8` moves NXVM identity out of shared Product; P4 `7a2f23574` reduces the shared opening contract to App-provided text. Eight 0546 App artifacts were rebuilt. Product entry/command/manifest focused checks passed on x64; full dual-width Product and complete T547 qualification remain open and are not claimed by this S closure. |
| T547 S6 | Complete: `bf469c879` adopts SoftPC S16's ownership correction without importing SoftPC runtime code. NXVM-family configuration/factory/extension support and direct tests now live in `src/core/product` and `test/core/product`; the old `ibmpc/nxvm` member is gone. IBM PC and core gates plus 8 focused tests pass on x64 and x86; all eight 0546 product artifacts were rebuilt against the relocated link input. |
| T547 S7 | Superseded into S8 before P delivery: its uncommitted canonical Core rehome is retained as S8's required receiver baseline rather than split into an unbuildable partial commit. |
| T547 S8 | Active: owner-directed raw SoftPC public six-component import from `288d9319`, including its two stale Common manifest repairs and complete Ninja-first dual-width qualification/artifact delivery. |
| T547 S9 | Active: owner-directed raw SoftPC `e6001412` six-component import. Canonical shared `emulator` replaces retired `common`; all NXVM receivers, CMake, manifests and tests must migrate with no alias. |
| T547 S10 | Superseded into S11 before P delivery: its uncommitted neutral composition extraction is retained as S11's required baseline rather than split into an unbuildable partial commit. |
| T547 S11 | Complete pending owner review: Emulator owns one generic composition/teardown and fixed monitor shell; PC/MyNES retain only execution, extension rows, keyboard-help rows and App-private configuration. MyNES now accepts `display=window|console` and strict `console_control=0|1`. x64/x86 repository-only units pass 511/511 per width; MyNES owned tests pass 57/57 per width; all ten runnable artifacts were rebuilt and PE-verified. Desktop/manual and PC external integration were not rerun for S11. T547 remains open for owner review. |
| T547 S12 | Complete pending owner review: Emulator rejects arguments for eight fixed non-snapshot commands while retaining App-owned `save/load` file arguments; its help order is machine control, snapshots, then general commands. Runner-only getters are private, direct configuration receives the bounded Machine borrow, and MyNES owns an independent DOS-style `-` Debug console with `q` return. Full repository unit suites pass 511/511 on x64 and x86; MyNES suites pass 57/57 on x64 and x86; Emulator/Product manifests and corpus gates pass on both widths; all ten artifacts were rebuilt and deployment roots verified. The persistent Ninja ccache route now uses `build/.ccache`, with 360 verified cache hits after a cold/repeat build. No external PC integration or manual desktop validation was run for S12. T547 remains open for owner review. |
| T547 S13 | Accepted: `e75523d92`/`79fa1c9f7`/`9a00afb80`/`0d418bb6f` establish the corrected startup baseline and primary `> ` prompt; `de240d01c` makes Session the sole blank-result-separator owner; `102ca1f37` restores complete App-owned PC banners with shared PC version/copyright constants in `core/product/version.h`; and `7de400fc5` restores MyNES's App-owned banner. All ten x64/x86 artifacts were rebuilt and the owner accepted the real launch behavior. Emulator-focused x64/x86 tests pass. Full x64/x86 unit runs each retain one unrelated, unmodified `library.console_broker_display` desktop failure; this S does not claim complete unit qualification or external/desktop coverage. T547 remains open. |
| T547 S14 | Complete pending owner review: NXVM extension output now re-arms the one Session-owned monitor reader after successful formatting. This fixes `info` (and identically routed `speed`/`floppy`) printing then leaving no prompt, which looked like a hang. A Core factory callback regression covers the actual `info` route. Repository-only units pass 511/511 on x64 and x86; all eight 0546 PC artifacts were rebuilt. No MyNES source or artifact changed. External integration and manual desktop validation were not run. |
| T547 S15 | Complete pending owner review: one Session-owned result separator now appends exactly one CRLF whenever text re-arms a prompt. Normal monitor owners terminate text with one CRLF; PC and MyNES Debug owners publish terminal-newline-free text for their `-` continuation. This removes text inspection and no mode flag/API is introduced. The same S removes lossy Emulator composition status remapping, rejects UI composition before control exists, and removes the test-only public monitor capacity constant. Emulator/Product/Core focused tests pass 38/38 on each width; MyNES owned App plus Debug integration tests pass 5/5 on each width. All ten artifacts were rebuilt. Desktop/manual and external integration were not run; T547 remains open for owner review. |
| T547 S16 | Complete pending owner review: test-only zero-stage composition destruction coverage proves `create` then `destroy` releases the App opaque machine exactly once without composing Emulator Machine, Session or UI. `emulator.composition` and the Emulator test manifest pass on x64 and x86. No production code, artifact or desktop/external route changed. |
| T547 S17 | Complete pending owner review: private-machine unbinding now precedes wrapper destruction, and a failed unbind preserves the wrapper/private machine for retry. The existing fixture proves both ordering and retry behavior; the command-result contract documents Monitor versus Debug framing. Emulator composition/manifest tests pass on x64 and x86; all ten artifacts were rebuilt and deployed. Desktop/manual and external integration were not run. |
| T547 S18 | Closed: fixed lifecycle ownership and final composition/display repairs are included in the T547 closure commits named above; its former in-progress qualification is superseded by the recorded full dual-width unit results and current ten artifacts. |
| T547 S1 | Implementation P complete and pushed as `26c013bba`: current SoftPC differences were reconciled, native-test isolation imported, NXVM's later x86/IBM PC repairs retained, and the stale x86 negative fixture fixed. Its incomplete aggregate/stability evidence is explicitly continued by S2. |
| T547 S2 | Complete: pushed as `70bb50312`. The aggregate default is reduced from 8 to the evidence-backed safe 4 jobs, without changing individual budgets or assertions. One complete x64/x86 run of each shared package passes: Lib 51/51, Common 20/20, x86 182/182 and IBM PC 182/182; complete repository-only units pass 506/506 per width. See `etc/evidence/t547-s2-shared-test-stability.md`. |
| T547 S3 | Complete: pushed as `8d7022ea8`. All four shared test roots were audited; current identities are behavior-derived, registrations/manifests match, static identity and focused dual-width checks pass. S4 separately owns full-project qualification. |
| T547 S4 | Complete pending owner review: owner-authorized stale-reference repair `16e008415` and current-source MyNES artifact publication `9ab7ed6b9` are pushed. Complete NXVM repository-only units pass 506/506 on x64 and x86; the full available NXVM external integration suite passes 22/22 on each width; MyNES product tests pass 57/57 on each width. See `etc/evidence/t547-s4-full-project-qualification.md`. T547 remains open pending owner review. |
| T544 | Closed as CPU audit; complete repair/proof findings transferred to the first queued proposal, not claimed repaired. |
| M5 Td S177 | Complete: CPU audit closure and full repair transfer, archive/queue/reference reconciliation. |
| T545 | Closed after S7 actual-change acceptance: fixed eight-corpus import, preserved receivers, four owner-local test packages and full receiving qualification. No active S packet. |
| T546 S4 | Accepted after coordinator actual-change review of pushed Shared P1 2d9929742 and NXVM P2 ca016635d: complete source/caller/receiver proof, 537/537 units per width, original 58/58 external contexts and eight qualified 0546 PC artifacts. S4 closed; no active S packet. |
| T546 S5 | Accepted after coordinator actual-change review of pushed Shared P1 bdfe3b938 and NXVM P2 04f0278ad: full host/count/reference batch, final 539/539 units per width, original 58/58 integration and eight qualified 0546 products are proven. S5 closed; no active packet. T546 remains open for S6-S19. |
| T546 S6 | Accepted after coordinator actual-change review of pushed Shared P1 1db282a10 and NXVM P2 19945b2b9: complete ordinary-stack/source/caller proof, final units 541/541 per width, original integration 58/58 once, supplemental 33/33 per width, both gates/eight manifests and eight qualified stripped 0546 products. S6 is closed with no active packet; T546 remains open for S8-S20. |
| T546 S7 | Accepted after coordinator actual-diff review of pushed Shared P1 0c04f2c24 and NXVM P2 0f86b1ce0: 36 duplicate aliases retired without removed assertions, 505/505 final units per width, dependency/cache and affected gate proof complete. No-op builds 1.37/1.05 s; cached CPU unit-dependency recovery 109.47/52.57 s. Unchanged binaries/inputs retain S6 integration proof. S7 closed; no active packet. T546 remains open for S8-S20. |
| T546 S8 | Accepted after coordinator actual-change review of Shared 9d5e475f7 and NXVM 626d504f5: complete FLAGS/privilege/IRET/RF/prior-TF batch, dual units 505/505, supplemental 33/33, both gates/eight manifests, original integration 58/58 once and eight stripped 0546 products. S8 closed; no active packet. T546 remains open for S9-S20. |
| T546 S9 | Accepted after coordinator actual-change review of pushed Shared 746e3e214 and NXVM 04bd419e1: complete seven-member arbiter/shadow/NMI/comparator/HLT/REP/receiver batch, final units 505/505 per width, original integration 58/58 once, supplemental 33/33, both gates/eight manifests and eight stripped 0546 products. S9 closed; no active packet. T546 remains open for S10-S20. |
| T546 S10 | Closed at owner direction: finalizer, resident shutdown/Core wait, approved query and real new-CS entry repair are separated for target-scoped delivery. The 78-owner CPU scan and 82/418 specialized gates pass on both widths; the one unrelated Lib Console viewport failure is explicitly moved to S11, not used as CPU evidence. |
| T546 S11 | Closed at owner direction after pushed Shared Console broker repair/test P commits `2e1c59660` and `89ae694ef`, plus receiving product P commits. Focused dual-width Lib evidence passes: 36/36 Lib units per width, including the native desktop smoke and Console contract. The broader all-App test/artifact exit was not rerun or claimed; it remains available for a later explicitly scoped qualification S. T546 remains open. |
| T546 S12 | Implementation P commits `6200790b4`, `8e2f71d41`, and `c23b67abb` repair the admitted IBM PC/Audio failure contracts. Its required complete qualification remains retained for later coordinator reconciliation; it is not claimed closed by this status transition. |
| T546 S13 | Implementation P commits `320fdc385`, `c498244b7`, `5c5b64725`, and `4b76caa9f` replace the IBM PC adapter's legacy Win32 key interpretation with neutral KVM events. Its receiving qualification remains retained for later coordinator reconciliation; it is not claimed closed by this status transition. |
| T546 S14 | Implementation P commits `1b85c58d8`, `2b65d095c`, and `5a9d9e54d` repair descriptor-query/table correctness. Its complete dual-width unit qualification remains retained for coordinator reconciliation; it is not claimed closed by this transition. |
| T546 S15 | Accepted: `be1010714`, `bdc1f62fd`, `a20237d96`, `aed31ad7d`, and the final evidence closure repair source-derived outer RETF/IRET conforming/nonconforming CS predicates plus 286/386 post-return segment-cache cleanup without a second path. Focused gates and manifests pass; complete repository-only units pass 506/506 on each width through bounded CTest partitions. Eight 0546 PC artifacts were rebuilt/verified; MyNES artifacts and user snapshot remain untouched. T546 remains open for S16-S23. |
| T546 S16 | Accepted: `5e0388e8c` unifies the protected task-transition owner; `be3023e26` through `b4f0e3159` add direct 286/386 Ring-3 and task-gate evidence; `2a33fc378` corrects the target paging receiver; and `e83aa8a2d` records the disposition. Complete repository-only units pass 506/506 on x64 and x86. The four receiving Apps' eight 0546 artifacts were rebuilt by the preceding S16 product commits. T546 remains open for S17-S23. |
| T546 S17 | Accepted: Shared `de4185367` restores Intel-defined physical GDTR/IDTR table cycles, retains logical LDT/TSS references, and publishes PDE Accessed at the actual present-PDE walk. Direct owner regressions, manifests and complete repository-only units pass 506/506 per width; eight rebuilt 0546 PC artifacts and SHA-256 identities are recorded in S17 evidence. T546 remains open for S18-S23. |
| T546 S18 | Accepted: Shared `5d6a73dec` makes early-family string handlers check each element before publishing repeat state, while the sourced 8086/8088 multi-prefix interrupt return rule is applied only at an accepted NMI/INTR boundary. Existing INS/OUTS commit ownership remains unchanged. Direct callback regression, x86 gates and complete repository-only units pass 506/506 per width; eight rebuilt 0546 PC artifacts are recorded in S18 evidence. T546 remains open for S19-S23. |
| T546 S19 | Complete implementation and verification are recorded by Shared `5773243d4` and NXVM `3458e01c8`: Core owns one interruptible external NPX wait path; `WAIT`/TEST/BUSY, ESC restart/trap and accepted IRQ/NMI wake use the guest-time owner with no synthetic FPU completion. Complete repository-only units pass 506/506 per width and eight 0546 artifacts are rebuilt. T546 remains open for S20-S23. |
| T546 S20 | Accepted after actual-change review of Shared `2af784585` and NXVM `8aef7bea4`: the sole CPU timing selector now classifies transfer outcomes from pre-execution state, restores 80286 LEAVE and 80386 VM86 segment-POP source rows, and retains exact delayed retirement accounting. Focused owner gates, all family manifest runners, manifests and complete repository-only units pass 506/506 per width; eight rebuilt 0546 artifacts and identities are recorded in S20 evidence. T546 remains open for S21-S23. |
| T546 S21 | Complete pending review: one private Core retirement-wait owner now consumes qualified external work for both successful retirement and fault delivery without publishing a synthetic retirement. Direct successful/faulted evidence, manifests and complete repository-only units pass 506/506 per width; eight rebuilt 0546 artifacts are recorded in S21 evidence. T546 remains open for S22-S23. |
| T546 S22 | Complete pending review: all eighteen transferred receivers now map to S1-S21 owners or source-proven non-applicability. Five-family current timing corpora contain 4,842 rows with zero selected source-unallocated and zero failed entries; dual-width manifest runners pass. T546 remains open for final S23 qualification. |
| T546 S24 | Closed by owner-approved disposition: shutdown arbitration is narrowed to fault completion and the redundant successful-wait loop turn is removed. The current and isolated S20 Model40 x86 routes both exceed the unchanged 180-second host budget, while current reaches at least as much guest progress. The non-differentiating external-throughput result is retained in TODO, not claimed as a CPU repair failure. |

## T546 Owner-Directed Closure

T546 is closed at the owner's direction. The owner-approved retained item is
the Model40 x86 Turbo DOS-installation terminal's fixed 180-second
host-throughput qualification: current source and the isolated accepted S20
baseline both exceed the same budget with the same read-only inputs. It is a
separate TODO, not a passing qualification row, CPU semantic failure, or an
authorization to change the timeout, profile, firmware, media, or guest time.

S24's narrowly scoped Core shutdown-fault repair has direct x64/x86 evidence.
The interrupted all-unit run and unrerun final external/artifact matrix are
not claimed as passed by this closure. Historical S1-S23 packets remain below
as records; no T546 packet is active.

## Closed Packet — M5 T546 S22 Cross-Family Convergence

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; T546 remains the only open NXVM implementation task. |
| Admission And Approval | Owner's standing automatic S admission and timing-upgrade approval; this is S22 of the accepted M5 T546 proposal. |
| Objective | Reconcile every imported family/source/oracle finding with its final S1-S21 owner disposition; rescan legal successful timing routes for source-unallocated publication and identify any remaining unfixable L1 or source conflict. |
| Non-goals | No unrelated code cleanup, new timing model, source-value averaging, Lib/Common/MyNES change, profile workaround, firmware/media/INI change or unapproved timing downgrade. |
| Reference Baseline | Accepted S1-S20 and S21 commits `c7332bab5`/`7bde37ab2`; T544 convergence and full T546 ledger. |
| Candidate Proposal | `proposals/m5-cpu-audit-gap-repair.md`, S22. |
| Files And ABI Surface | Owner-local x86 timing/CPU/Core tests and NXVM evidence/state only; no public API or new state unless a confirmed remaining defect requires its existing owner. |
| Applicable Rules | NXVM guide; Execution, Architecture, Coding, Documentation and source policies; T544/T546 source evidence. |
| Verification | Five family timing-manifest runners on x64/x86; direct route/source-unallocated scan; relevant owner regressions and manifest/document checks. |
| Expected Markers | Every imported receiver has an S1-S21 or source-proven non-applicable disposition; L2 models retain explicit evidence; no legal successful selected route silently publishes L1/source-unallocated time. |
| Asset Needs | Existing build caches only; no external firmware/media input changes. |
| Reporting Requirements | Record all eighteen receiver dispositions, any retained source conflicts and their no-invention rule, actual diff, verification and S23's remaining final-qualification boundary. |
| Stop Conditions | Stop and report an unfixable L1, necessary downgrade, unresolved authority requiring policy, required non-x86 owner change or protected-asset need. |
| Exit Criteria | Complete receiver map and current-source sweep distinguish repaired, explicit L2 and source-proven non-applicable outcomes; focused dual-width proof, manifests/evidence and scoped P commits complete. T546 remains open for S23. |
| Original Owner Request | Repair the complete CPU semantic/timing gap universe before moving to unrelated work. |
| Similar-Issue Sweep | Cross-reference T544's eighteen receivers, source-unallocated publishers, timing origins, test manifests and S1-S21 evidence; reject historical status as current evidence without a current route/result check. |

## Closed Packet — M5 T546 S21 Retirement/External Wait Accounting

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; T546 remains the only open NXVM implementation task. |
| Admission And Approval | Owner's standing approval for timing-accuracy upgrades and automatic S admission; scope is S21 of the accepted M5 T546 proposal. |
| Objective | Reconcile successful, faulted and asynchronous CPU execution with the one Core time publisher: preserve completed instruction context, publish qualified external wait/overlap once, and keep instruction, delivery and compatibility progress distinct. |
| Non-goals | No Lib/Common/MyNES changes, board clock redesign, host pacing, parallel executor/time path, synthetic device duration, user INI/media/snapshot changes or unapproved timing downgrade. |
| Reference Baseline | Accepted S20 commits `2af784585` and `8aef7bea4`; S19 external WAIT/NPX contract and T544 S2/S7 retirement-boundary evidence. |
| Candidate Proposal | `proposals/m5-cpu-audit-gap-repair.md`, S21. |
| Files And ABI Surface | Existing x86 Core/CPU retirement and scheduler owners plus owner-local tests/evidence; no new public API, second clock, lifecycle manager or board-specific exception. |
| Applicable Rules | NXVM guide; Execution, Architecture, Coding and Document rules; architecture/coding authorities; source policy; T544 S2/S7 and T546 S19 retirement/external-wait evidence. |
| Verification | Direct successful, faulted, bus-not-ready, reset/pause/stop and accepted asynchronous-delivery regressions; x86/IBM PC owner gates; complete repository-only dual-width unit suite; manifests, evidence and affected artifact checks. |
| Expected Markers | One retirement observation per completed instruction; no observation for faulted delivery; exact partition of CPU-retirement versus external-wait time; pending state cleared only at completed delivery/reset; no new L1 or synthetic physical publication. |
| Asset Needs | Existing build caches and eight deployed 0546 PC artifact destinations only; no external firmware/media input changes. |
| Reporting Requirements | Record every pending-retirement exit and its disposition, similar-path sweep, actual diff, separate Shared/NXVM P commits, verification and remaining source/time boundary. |
| Stop Conditions | Stop and report any timing downgrade, unfixable L1, source conflict requiring a new model, protected-asset need, required Lib/Common change or non-CPU owner change. |
| Exit Criteria | Every admitted successful/faulted/asynchronous pending-retirement variant has direct owner proof; no duplicated or synthetic time publication; actual-diff review, required tests, manifests/evidence and pushed scoped P commits complete. T546 remains open for S22-S23. |
| Original Owner Request | Resume the NXVM CPU repair T after closing the preceding task; repair CPU semantics/timing thoroughly rather than switching work. |
| Similar-Issue Sweep | Scan all Core paths touching `cpu_retirement_wait_*`, `external_cycle_round_ticks`, `publish_elapsed_ticks`, reset/stop/pause and CPU refresh/delivery outcomes; classify every wait/publication origin and each asynchronous entry before changing an owner. |

## Accepted S9 Asynchronous Arbiter

Coordinator actual-change review accepts Shared `746e3e214` and NXVM
`04bd419e1` against the complete seven-member packet and
[source/caller/receiving proof](../etc/evidence/t546-s9-asynchronous-arbiter-design.md).
One original arbiter and private shadow/input/service owner replace the
retired conflated flags; original handler tables remain. No public API,
instruction clock allocation or excluded component changes. The retained 186
predicates and HLT/step combination are explicitly reference-model L2,
not new physical L3 or a downgrade of existing exact instruction clocks.

Final complete units pass 505/505 per width (206.77/194.28 s), original
58/58 external contexts pass once, supplemental 33/33 per width, both full
gates/eight manifests and eight optimized stripped 0546 artifacts pass.
Production is net +69; twelve source/test paths are +752/-51, net +701,
mostly independent source-conditioned regressions. INIs, five masters and
Lib/Common/MyNES remain unchanged. S9 closes with no active packet; T546
retains complete S10-S20 fault/gate/task/page/NPX/time/source receivers.

## Accepted S8 FLAGS Privilege And Return

Coordinator actual-change review accepts pushed Shared `9d5e475f7` and NXVM
`626d504f5` against the complete eight-member packet and
[direct source/caller/receiving proof](../etc/evidence/t546-s8-flags-return-design.md).
POPF-local privilege masks, original IRET/scalar paths and one actual-task
outcome repair RF images versus rollback and prior-TF completion without a
new public API, timing value or parallel owner. Original table style remains.

Final complete units pass 505/505 per width (120.05/247.58 s), original 58/58
external contexts pass once, supplemental checks pass 33/33 per width, both
full gates/eight manifests and all eight optimized stripped 0546 products
are verified. Earlier failed or contained candidates remain explicit evidence.
Production is net +33; nineteen source/test/build paths are +802/-55, net +747,
primarily independent source-derived matrices. Lib/Common/MyNES, four owner
INIs and five media masters are unchanged.

S8 is accepted and closed with no active packet. T546 remains open for S9-S20;
full arbiter, gate/task/page/delivery/NPX/retirement and remaining timing/source
contracts keep their original receivers. Validated receiving caches and
task-local research/final logs are retained for the immediate successor.

## Accepted S7 Build And Verification Efficiency

Coordinator actual-change review accepts Shared `0c04f2c24` and NXVM
`0f86b1ce0` against the owner-directed insertion and
[complete efficiency evidence](../etc/evidence/t546-s7-build-test-efficiency.md).
One native graph and compiler cache replace repeated serial leaf scheduling;
one dependency-only unit build separates compilation from execution. Thirty-six
duplicate product aliases are removed, preserving all canonical bodies and
334 target identities. Positive/negative registration and timeout/tree cleanup,
affected build/artifact/INI gates, eight manifests, Types and docs pass.

Both final complete unit executions pass 505/505 once, 250.11/233.17 seconds,
without relaxing 300-second containment. Full-suite runtime is not claimed
faster; measured build/configuration and avoided-work savings are in evidence.
Exact S6 CPU objects, ten EXEs, four INIs and five masters are unchanged.
The qualified integration command is unchanged; its accepted S6 58/58 proof
is retained rather than repeated. No runtime source/API or Shared corpus body
changes. Counted build/tool code is +56/-20, net +36.

S7 is accepted and closed with no active packet. The owner moved former
uncommitted FLAGS S7 to deferred S8 and former S8-S19 to S9-S20.
Exact local reproducer patches/hashes, research and warm caches are retained
for that next admission. T546's complete CPU repair remains open; source and
runnable CPU/artifact baseline remain accepted S6, not a new CPU qualification.

## Accepted S6 Stack And Frame Publication

Coordinator actual-change review accepts Shared P1 `1db282a10` and NXVM P2
`19945b2b9` against the complete original S6 batch and
[source/caller/verification evidence](../etc/evidence/t546-s6-stack-frame-design.md).
One logical ordinary-frame owner precedes original scalar transfers; independent
ENTER attributes/full ESP, early wrap, POP aliases and sourced explicit SP
admission retain the original table style. The candidate's overbroad GP rule
is corrected by original 386 14-6, not by weakening the accepted SS oracle.
Core/chip assertions use independent actual-memory images; no MMIO undo or
new public API/timing value is introduced.

Final units pass 541/541 per width (248.83 s/233.31 s), original external
integration 58/58 once, supplemental 33/33 per width, both gates/eight manifests
and eight stripped 0546 artifact checks. Production net +19; ten code/test/build
paths net +826 supply the missing direct matrices and guards. INIs, five masters,
Lib/Common and MyNES are unchanged; unrelated MyNES documentation is preserved.

S6 remains accepted and closed; subsequent efficiency S7 acceptance is above.
T546 remains open for S8-S20;
full FLAGS/arbiter/gate/task/page/delivery/NPX/retirement and remaining source
 timing contracts retain their original receivers.

## Accepted S5 Host Arithmetic And Counts

Coordinator actual-change review accepts pushed Shared P1 `bdfe3b938` and
NXVM P2 `04f0278ad` against the complete S5 packet and
[direct proof](../etc/evidence/t546-s5-host-arithmetic-count-design.md).
Pre-widening, unsigned masks, portable SAR, guest count/definedness and
initialized undefined double shifts retain the original handler tables.
Trusted typed references use one existing copy path; no public API, timing
constant, new framework or App-specific CPU implementation is added.

Final complete units pass 539/539 per width (116.97 s/118.33 s), original
integration 58/58 once, supplemental 33/33 per width, both full gates and
eight manifests/qualified stripped 0546 artifacts. Earlier incomplete x86
containment and corrected early oracles remain explicit evidence, not passing
results. Production net -8; nine code/test/build paths net +437 supplies the
missing independent 2,004-context matrix proof and two guard registrations.
Owner INIs, five media masters, Lib/Common and exact MyNES EXEs are unchanged.

S5 remains accepted and closed; subsequent S6 acceptance is recorded above.
T546 remains open for S8-S20. Complete fault/task/paging/delivery/NPX/retirement and
remaining timing/source conflicts retain their original receivers.

## Accepted S4 Admission Versus Next Fetch

Coordinator actual-change review accepts pushed Shared P1 `2d9929742` and
NXVM P2 `ca016635d` against the complete S4 packet and
[direct proof](../etc/evidence/t546-s4-admission-next-fetch-design.md).
Ten target checks retain logical admission without speculative page faults;
one preview model distinguishes missing-input L2 from exact m/ts source rows.
Original handlers, real frame/descriptor checks and single retirement/time
owners remain. The only public extension is the explicitly approved enum append.

Final complete units pass 537/537 per width (117.63 s/221.41 s), original
integration 58/58 once, supplemental 33/33 per width, both full gates, final
manifests/Types and eight optimized stripped 0546 artifact checks. Production
net +23 lines; source/test/build net +627 supplies the missing 121-case chip
matrix, Core publication proof and source-derived receiving oracle/gate fixes.
INIs, five master images, Lib/Common and exact MyNES EXEs remain unchanged.
Unrelated MyNES documentation is preserved outside this task.

S4 is closed with no active packet. T546 remains open for S6-S19, including
full frame/gate/task/paging/delivery/NPX/RMW and remaining timing contexts.

## Accepted S3 Effective Address And Segment Spans

Shared P1 `15a2399d0` and NXVM P2 `d875a72fe` are pushed. Coordinator
actual-change review accepts the complete S3 source/caller and receiver batch
against the original request and [direct proof](../etc/evidence/t546-s3-address-span-implementation.md):
word/bit EA, cached and empty expand-down limits, all fifteen composite
readers, scalar bus phases and ordinary real SS delivery. The original GP
mask policy and original table handlers remain; no public API or second owner.

Final units pass 536/536 per width (127.02 s/118.02 s), original integration
58/58, supplemental 33/33 per width, both specialized aggregates, final
manifests/Types/documentation and eight artifact checks. Production net -7
lines; counted source/test/build net +483 supplies the missing matrix proof.
INIs, external masters and MyNES remain unchanged. S3 remains accepted; S4's subsequent closure is recorded above.
T546 remains open for S6-S19. Complete frame/task/paging/delivery/RMW
and the two inherited host-reference guards retain their explicit later owners.

## Accepted S2 Decode/Admission And Model40 Inputs

Shared P1 `34867b960` and NXVM P2 `01709c418` are pushed. Coordinator
actual-change review accepts the approved batch and
[direct proof](../etc/evidence/t546-s2-decode-admission-implementation.md):
complete units 534/534 per width, all 58 original external contexts, eight
manifests, applicable static/supplemental/specialized and documentation gates,
and eight verified optimized stripped 0546 PC artifacts. INIs, external masters
and the exact MyNES pair are unchanged. No public API or parallel owner is added;
production net +1 line. The Model40 conversion is L2, not physical L3.

S1-S6 are closed; T546 remains open for S8-S20. The owner-authorized
automatic boundary remains unchanged for the next admission. S1's
accepted reset/image proof remains in the [task history](../history/M5-T546-cpu-audit-gap-repair.md).

## Accepted T545 Baseline

Active T546 developer target is vm-0-5-0546 (0.5.0546), optimized stripped
x64/x86 pairs for all four fixed PC Apps. They are rebuilt and verified;
the former accepted 0545 baseline is retained in T545 history, not assets.
MyNES remains 0043 and is not rebuilt for CPU-only changes.

S7 implementation P1 is `90e91d721`, pushed to origin/master. Coordinator
actual-change review accepts the entire original request, S1-S6 ledgers and
original S7 verification, not merely a passing unit summary. See
[final proof](../etc/evidence/t545-s7-final-qualification.md),
[task history](../history/M5-T545-softpc-eight-corpus-refresh.md) and
[archived proposal](../history/M5-T545-softpc-eight-corpus-refresh-proposal.md).

All 58 original PC integration contexts pass once with unchanged checkpoints:
default 22/22, AT 3/3, Model40 3/3 and XT 1/1 per width. Complete PC/shared
units pass 532/532 per width; MyNES receiver units pass 43/43 and integration
12/12 per width. Eight manifests, 33 supplemental checks per width, both
specialized aggregates, artifact-root/INI and documentation checks pass.
S7 corrects only two NXVM static fixture checkers after S5 relocation; their
44-owner/133-constructor inventories remain intact. No Shared production,
API, App source, owner INI, media or executable input is changed by S7.

S2 imports exact committed SoftPC
`8124e551e841ccdec2ceb7f6a0f6ae5b513a7951` eight-root bytes. S3-S6 subsequently
complete Lib/Common/x86/IBMPC owner-local tests with refreshed test manifests;
current test bytes are not claimed identical to that historical upstream pin.
All preserved/relocated assertions and removed-file dispositions remain in
the S1/S2 inventory and S3-S6 evidence linked by task history.

## Current Technical And Runnable Baseline

Accepted implementation baseline is S9: Shared CPU/tests `746e3e214` and
NXVM evidence/artifacts `04bd419e1`, with production hash BEBC79F8… and final
artifact identities in S9 evidence. Earlier candidate identities are superseded;
this governance acceptance adds no executable input.

- Independent chips and sole CPU implementation live in core/chips; neutral
  execution, memory/ports and guest time live in core/x86.
- IBMPC board-common/AT/XT own PC wiring; Machine owns one Emulator driver and
  pacing/media adaptation; Product owns command/Debug/UX/entry while each App
  owns its INI/request-loader policy.
- Four fixed Apps own immutable compositions and firmware bindings. Model40
  alone owns D4. No App production graph links a peer App.
- Shared tests follow their owners; actual model assertions stay App-local.
  The one external family harness executes each real fixed binding and its INI.
- Eight PC 0.5.0546 EXEs remain directly in assets/my5160,
  assets/my5170, assets/mydeskpro386 and assets/nxvm, with unchanged adjacent
  owner INIs. All eight S9 optimized stripped pairs are verified. MyNES now retains
  its separately accepted 0.0.0044 pair; its own [Current](../../mynes/states/CURRENT.md)
  owns those identities. CPU-only work must not rebuild or alter them. Qualified PC hashes
  are in [S9 evidence](../etc/evidence/t546-s9-asynchronous-arbiter-design.md),
  historical S1 MyNES hashes remain in [S1 evidence](../etc/evidence/t546-s1-reset-images.md); PE widths,
  current identity and absence of compiler debug sections are verified.
  Runtime Debug remains; only CPU-dependent PC products are rebuilt.
- External media masters are verified unchanged after final S9 overlay integration.
  All S9 builds/tests/gates are terminal; complete units, original 58 integration
  and supplemental routes pass. Coordinator acceptance is recorded above.
  Ignored receiving caches
  remain needed for the next CPU repair batch's incremental regression checks.

## Next Work And Qualification Boundary

The owner-requested CPU instruction/function/timing repair remains open as
T546 after accepted S9, with S10-S20 mechanism/proof batches and final
qualification pending. Remaining candidates stay in [Queue](QUEUE.md). Concrete
Shared changes follow the automatic-approval boundary recorded in the
[proposal](../proposals/m5-cpu-audit-gap-repair.md).

T544's [complete audit](../etc/evidence/t544-s7-five-family-convergence.md)
and family findings remain intact. T545 imports/tests/boot qualification do
not resolve them or establish complete CPU correctness, new L3 timing or a
physical-time axis. Existing Common wake/Console rollback, portability and
other debt remain explicitly in [TODO](TODO.md), not claimed fixed here.
