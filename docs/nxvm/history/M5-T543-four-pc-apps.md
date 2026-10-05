# M5 T543 Four PC Apps

## Admission And Frozen Coverage

Owner admission on 2026-10-04: close T542 and split the four fixed PC Apps as
app-my5160, app-my5170, app-mydeskpro386 and app-nxvm. The additional owner
direction requires exactly one App per S, in that order. Reference commit
4c0c2db16 is the clean, pushed T542 S20 acceptance baseline.

The owner additionally requires parallel src/test/assets ownership, while
docs/nxvm, tools/nxvm, shared version and the NXVM MTSP sequence remain unified.
The target artifact roots are assets/my5160, assets/my5170,
assets/mydeskpro386 and assets/nxvm. No new per-App governance queue or counter
is created; the existing NXVM commit target continues to own these four Apps.

The [proposal](../proposals/m5-independent-pc-apps.md) owns the sequential
delivery design. [Current](../states/CURRENT.md) owns the one active packet.
This record is evidence and convergence inventory, not another current status.

| Batch | Existing source owner | Receiving source/test owner | Admission disposition |
| --- | --- | --- | --- |
| S1 XT | app-nxvm/profiles/xt and XT binding/firmware composition assertions | app-my5160 and test/app-my5160 | Accepted after pushed cd173acb7 and fed7049b3, full verification and actual-change review. |
| S2 IBM AT | IBM AT definitions inside app-nxvm/profiles/default_profile, IBM AT assertions | app-my5170 and test/app-my5170 | Accepted after pushed e9bb0dc55 and fe4ca8107, complete verification and actual-change coordinator review. |
| S3 DeskPro | app-nxvm/profiles/model40, D4, ROM and copied observations, Model40 assertions | app-mydeskpro386 and test/app-mydeskpro386 | Admitted after accepted S2; migrate the whole original owner without hardware/timing changes. |
| S4 NXVM | app-nxvm/profiles/default_profile, project-owned firmware and remaining shell | app-nxvm and test/app-nxvm | Planned; retains original default hardware, not a new board. |

Each row consumes source, public/private includes, fixed composition/binding,
CMake selection/link graph, repository-only tests, external integration
scenarios, documentation/tool references and its two deployed artifacts.
Common PC mechanisms and their tests remain in ibmpc. Lib/Common/x86 and MyNES
are excluded because this is a PC product-ownership cutover, not their upgrade.

Required proof per accepted row: no peer-App production dependency; one shared
Product/Machine implementation; preserved board/ROM/CMOS/media values; full
repository-only units on x64 and x86; original affected boot checkpoint;
optimized stripped pair retaining runtime Debug; SHA-256 and architecture;
source/test line accounting and reviewed, pushed one-target P commits.

Whole-T closure requires all four rows accepted, independent build entries,
the retained 58 original integration contexts passing once with unchanged
predicates, eight verified current EXEs, complete manifests/dependency/docs
checks, owner INI/media-path preservation and no obsolete live selection or
unclassified migration residue. New hardware, timing upgrades, PC110 and
MyArcade are non-applicable with their existing separate receivers.

Accepted Td S175 reconciles artifact/commit-target mapping in the execution
rule before the migration: assets/<app>/<profile> retains the original INI
relative-path depth; the existing NXVM scope, PC version and MTSP stay unified.
It creates no chip/Product/Machine mechanism or per-App governance hierarchy.

## S1 Admission Review

Confirmed the source inventory contains a real XT composition and firmware
provider, while the thin main/binding and CMake-generated binding currently
remain under the four-profile app-nxvm shell. S1 moves rather than forks the XT
owner and consumes the accepted shared PC runtime. Later App deliveries are
not admitted simultaneously. Planning documents create no 0543 artifact or
claim a completed source migration.

## S1 Delivery Review

The XT constructor, plan, ROM provider and declarations move to
src/app-my5160/profiles; their C bodies change only include paths. The new
14-line process entry and immutable XT binding consume ibmpc/product, not a
copied command loop. src/app-my5160/CMakeLists.txt owns the fixed source list,
BYOB roles and manifest checks. The family build selects that entry; this is
not a claim of a standalone `cmake -S src/app-my5160` project. The selected
transitive production graph rejects peer-App sources/includes, and the old
app-nxvm/profiles/xt owner is removed rather than retained as a forwarding path.

The original XT profile smoke moves to test/app-my5160/unit/profiles and links
only the XT composition plus declared shared fixtures. All original predicates
and markers remain; its local construction helper replaces the multi-profile
test selector. test/app-my5160/integration/register.cmake owns the original XT
boot case. The existing cross-profile boot observer and INI fixture remain
single family test-only implementations during cutover under test/app-nxvm;
they do not enter any production graph or create another acceptance predicate.
The timing qualification matrix remains a multi-App test and follows the moved
XT header. Final family test-owner reconciliation belongs to S4's already
admitted complete cutover, not a deferred broken XT test.

assets/my5160/ibm-5160-model-268-360k preserves the original INI contents and
directory depth. Its Git blob remains 946ca0c1774ef7d6a52be6ef7bbc689da024802b;
the relative floppy reference resolves to the same external 368640-byte master.
No external master or other App configuration is edited. Only the XT pair is
rebuilt at 0.5.0543; other six PC EXEs and MyNES remain at their accepted source
identity. Shared .gitignore admits the parallel PC roots, without changing a
shared executable input or requiring a MyNES rebuild.

No chip, timing formula, Product/Machine runtime owner, Lib/Common/x86 source
or test changes. Configuration carries the same CPU/FPU, topology and firmware
values. New App code declares its own fixed composition root, not a new parser,
worker, lifecycle queue, registry or platform wrapper.

The old current-artifact verifier counted literal CMake calls, so the two
mutually exclusive revision branches during App cutover caused a false
two-target failure. It now checks the actual selected target graph for exactly
one versioned EXE and its declaration. The GCC preset assertion still applies
to the default PC configuration that those presets select; its retained 0542
graph passes the same verifier. Custom XT configuration must not be required
to masquerade as the repository's default preset.

Repository-only unit aggregates completed without skips: x64 506/506 in
264.93 seconds, x86 506/506 in 270.91 seconds, using the existing four-job
bounded runner and separate build-tree outputs. These runs include all retained
cross-profile assertions, not just the relocated XT smoke. x64 XT integration
passes the unchanged `installer-running` predicate in 18.46 seconds; x86
passes the same original case in 24.77 seconds. Six shared
corpus and ibmpc manifest/corpus/DAG/negative checks pass 17/17. Specialized
aggregates pass on both widths after the build-graph correction. The gate-only
correction does not change the
compiled source, registered unit set or tested runtime predicates.

Initial cache-generation/build overlap attempts failed before test execution;
waiting for complete Make generation repaired the stale dependency graph
without modifying Core or its observation object. The two completed full unit
runs supersede those build-only failures. No failed runtime predicate is being
hidden or relabelled as a pass.

Source accounting uses rename-aware `git diff --cached --numstat` against the
admission baseline, without counting whole moved files as additions. The five
XT C/H files retain 349 original lines; only four include lines are replaced.
Production C/H: 39 added, 4 removed, net +35 (the 14-line App entry, 12-line
fixed binding and 9-line binding header). Test C/H: 22 added, 11 removed, net
+11 for the direct XT fixture and include migration. New App/test CMake entries
are counted separately, not disguised as C code reduction. This positive
increase purchases a real independent composition root while leaving all
runtime behavior in its one shared owner.

| My5160 artifact | SHA-256 |
| --- | --- |
| nxvm_xt_0_5_0543_x64.exe | 89FEDEDA7851F10688EEB0E30806B019A4FBF11B9D52955AADD7A2508C6CD8A4 |
| nxvm_xt_0_5_0543_x86.exe | 4BF917EEC2DB9B3AE6E5E4673F71E5043948607CBC6CD5C347E1F282D9D68972 |

Both are deployed beside the unchanged owner INI in assets/my5160. PE machine
types are x86-64 and i386; compile flags are -O3 -DNDEBUG with strict warnings,
linking uses --strip-debug and neither artifact contains debug/stab sections.
The selected graph retains x86-debug; production Core trace is explicitly 0.
The superseded XT 0542 pair is removed only after replacement build and boot
verification; it remains recoverable from Git history. There are exactly ten
current repository EXEs again: two MyNES, two My5160 and six remaining PC Apps.

Executor review confirms every S1 ledger member: five relocated XT files,
fixed App binding/entry/build, retained unit/boot assertions, include and gate
receivers, family documentation/tools and verified artifact/INI mapping. No
Lib/Common/x86/ibmpc corpus or MyNES diff; no raw vendor asset is added. Both
specialized aggregates exit 0; documentation governance and diff checks pass.
The six remaining PC EXEs keep their baseline identities. No active owned build
or test process remains; reused build caches are retained for later App rows.
Shared delivery cd173acb7 changes only the artifact-root ignore exceptions.
NXVM delivery fed7049b3 contains the complete App cutover. Both are pushed;
the worktree is clean. Separate coordinator actual-change review confirms
unchanged XT algorithms, four mechanical include substitutions in the moved
349-line corpus, original test predicates, independent selected production
graph, unchanged INI identity and unaffected peer artifacts. S1 is accepted;
this does not claim whole-T closure.

## S2 Admission And Source Inventory

S2 follows accepted S1 under the original owner-approved sequence. The IBM AT
definitions are actually inside profiles/default_profile, not the empty
profiles/at directory. Two fixed descriptors and their constructors share
pc_at_profile.c, machine_plan.c and the external PC/AT ROM provider. Copying
these files into app-my5170 would duplicate validation, snapshot construction,
ROM preparation and board materialization, or retain a peer-App dependency.

S2 must separate immutable 5170 values and model constraints into My5170,
retain default-specific choices in NXVM, and give genuine shared PC mechanisms
one ibmpc owner. The existing AT grammar and materializer remain authoritative;
no chip, timing, firmware or media behavior is redesigned. The receiving map
must account for the descriptor declarations, plan helpers, ROM provider,
test-only matrices, build gates and exact original IBM AT boot checks before
source changes. Necessary ibmpc changes receive a separate Shared delivery and
the affected PC consumers' rebuild/verification; Lib/Common/x86 and MyNES
remain out of scope. No S2 implementation or artifact is claimed by admission.

Owner packaging correction after S1: assets/<app> directly holds each fixed
App's EXEs and INI, with no profile child directory. Td S176 reconciles this
with shared rule delivery f65685e35 and product authorities. The owner's
My5160 relocation keeps both EXE bytes intact and rebases its floppy reference
by one directory level. S2 includes build/test receiving-path verification;
prior S1 path and INI hashes above remain historical evidence, not the new
deployment contract. My5170, MyDeskPro386 and NXVM use the same flat layout
at their respective sequential cutovers.

S2 packaging receiver verified: the owner's My5160 directory move is kept;
the build now receives one explicit artifact directory, and both original INI
integration helpers load NXVM.ini from that same directory instead of deriving
a child directory from a test-case name. The My5160 boundary checks the flat
root. The INI boundary still requires exactly four canonical configurations,
and EXE deployment still cannot write an INI. Unmigrated products retain their
existing computed artifact directories; their source cutovers remain separate.

Both 0543 XT product targets rebuild and deploy directly to assets/my5160,
without recreating a profile child. The original XT integration case passes
once per width at the flat receiving path: x64 19.66 seconds, x86 23.91 seconds.
The external master remains DE271368874209C07A2FC25C81C17529D4BDD7718B2731B86C49A2DA923A256E,
368640 bytes; only the relative path loses one ../ level. Re-linked artifact
hashes are x64 5B5A4FE62563D2BF5DACF6AD328A0770216AC59B3BB31FDBA5B11083A452EF2A
and x86 8F47004386F6526C3FDC03D97B24120333C68A12D64045E15D5AD586EE1E5E93.
These packaging checks are in-progress S2 evidence, not acceptance of the
unimplemented My5170 source cutover or a substitute for its complete units.

The first S2 source move puts the shared immutable PC/AT ROM provider in
ibmpc/board-common/pc_at_rom.c and pc_at_rom_interface.h. Both existing consumers
and repository-only fixtures follow the new header; the App source list drops
the old provider instead of compiling it again. The shared Board's production
and observable variants use the same source list. Region addresses, bytes,
validation and reset behavior are unchanged; only include/guard ownership and
LF normalization change. The existing default-ROM test remains App-owned
because it verifies default machine construction, not an independent provider
fixture. Static immutable-ROM gates follow the shared implementation and still
reject runtime asset-file loading. ibmpc manifest and component boundaries pass.
This move is work in progress; receiving product rebuilds, complete units and
the remaining descriptor/construction split are still required before S2 push.
After that move, x64 strict compilation of the shared provider, XT product and
both default/5170 construction tests succeeds; the two original ROM mapping
and IBM AT composition unit cases pass. The x64 re-link is now in-progress
source output, not the earlier packaging-only hash above. Complete dual-width
and receiving-product evidence will replace this partial proof at S2 exit.

S2 implementation continues: My5170 now owns its immutable descriptor,
memory/media constraints, preparation callback, fixed binding and process entry.
The shared AT descriptor/materialization lives in board-common/pc_at_profile;
the single allocated candidate, ROM preparation, controller projection and
finish/rollback live in machine/pc_at_preparation. AUX presence is an explicit
copied board input; the immutable App validator retains model constraints.
There are no mixed default/5170 build guards or compatibility headers.

The original 5170 composition, CGA topology and firmware/FDC unit sources move
to test/app-my5170 with their assertions intact. Cross-profile comparisons and
the one family fixture remain test-only until S4 reconciles the final shared
test receiver. Original AT console/CMOS/boot registration now belongs to that
App's integration/register.cmake. The INI moves to assets/my5170/NXVM.ini;
only one ../ is removed. Its SHA-256 is
F4D85EE2E7BFF403F8C19588DA8416CC96FE5AC72FE7A1764817EDDDC9B76F19;
the unchanged 1.2MB external media hash is
0F51D92B482253FC468A2B470FFAB82DB43898D1C8B44E504808B7A3EF3D4BDE.

Both selected AT 0543 product targets compile with strict warnings and deploy
at the flat root, with PE width checks passing. The x64 three migrated unit
cases pass 3/3 (0.96 seconds). Twelve affected owner/clock/ROM/INI static gates,
ibmpc manifest and component boundaries, and documentation governance pass.
One attempted build named an absent vm-default-composition-smoke target;
that command is not a successful aggregate proof. Complete run-unit-tests
is now being rebuilt instead. None of these partial checks accepts S2:
full units on both widths, original integrations, receiving product rebuilds,
source accounting and actual-diff review are still outstanding. Old AT 0542
EXEs remain until their verified replacement is accepted. No P is formed yet.

The x86 migrated trio also passes 3/3 (2.82 seconds). Its 0543 product hash is
C66A2739964B82C86CDBF4F94975ABF720B3BD5EA301EF9CDF5185857DB85BBB;
x64 is 343D0BDC5A3723FD4CD9CAB180CADCC8D256C6832509941B7B8B8A33D86CA2C5.
These identify in-progress outputs, not an accepted delivery. Source review
compares the original mixed translation unit with all three receivers: fourteen
function bodies are unchanged apart from removal of static where shared;
the remaining changes replace the model-based AUX decision by its copied input,
keep the existing single contract id 1, rename the unchanged memory option bit,
and move fixed validator branches to their App owners. Memory eligibility,
clock values, ROM bytes/mappings and controller algorithms are not upgraded.
The x64 full-unit aggregate passes 506/506 in 258.58 seconds. The x86
per-target dependency build was deliberately stopped after verifying its
owned process tree: repeated Make dependency traversal was unnecessarily
expensive. One default build reuses its objects and visits the graph once;
this changes neither the registered test set nor any assertion. Native test
execution remains serialized across widths. The x86 full suite has not yet
run; original AT integrations, specialized gates and receiving-product builds
are still in progress, so S2 remains unaccepted and no P is formed.

The x64 specialized aggregate passes. Original AT boot reaches the installer
in 38.57 seconds and its Console lifecycle passes in 0.98 seconds. The CMOS
integration initially fails before observing any register: its expected-row
selection compares the old profile-directory filename against NXVM.ini at
the new flat root. The family fixture now selects its original expected row
from the already compiled immutable machine binding, not an artifact path.
Both Model40 and AT predicates and all CMOS assertions are retained. A sweep
of integration source finds these two comparisons as the complete instance
set; no firmware, seed or production behavior is changed. The focused rerun
passes in 0.14 seconds after strict recompilation. The original failed attempt
remains recorded above; it is not counted as a passing result. All three
original AT integration predicates now have direct x64 passing evidence.

Both specialized aggregates pass; the additional 19 manifest/corpus/DAG,
layout and controlled-negative cases pass on x64 (177.26 seconds under build
load). Shared PC manifest/corpus and documentation governance also pass.
The AT link commands use -O3 -DNDEBUG and --strip-debug on both widths;
objdump finds no .debug sections in either deployed 0543 AT EXE. The x64
receiving product job completes XT/default/Model40 builds with PE checks;
their final boot observations and the x86 receiving job remain pending.

Current source-size review uses git diff HEAD --numstat for src/test C/H
paths with rename detection, plus every untracked C/H receiver's line count;
documentation, manifests, build scripts, binaries and generated files are
excluded. It counts 1031 added, 949 removed, net +82 lines. The additional
surface is the independent App entry/binding and public owner contracts;
there is no second parser, executor, ROM provider or AT candidate lifetime.
Including CMake/build verifier changes gives +1166/-1022/net +144. These
numbers are pre-commit inventory, to be reconciled with the final staged diff.

The x86 complete unit aggregate passes 506/506 in 217.91 seconds; both widths
now retain the full original suite. The rebased CMOS seed integration also
passes on x86 (0.07 seconds). The remaining AT boot/Console checks and receiving
machine checkpoints are run once per changed context, not in repeated rounds.

Both receiving-product jobs complete: XT at 0543 and default/Model40 at their
retained 0542 identities, on x64 and x86, with architecture checks passing.
The original x86 AT Console and boot checks pass (0.37 and 43.35 seconds).
Final-source x64 XT/default boot checks pass in 19.95/3.77 seconds. Model40
and x86 receiving boot checks remain pending. All configurations still use
the same external masters; unaccepted builds do not establish a new baseline.

The remaining x64 Model40 boot and CMOS checks pass (65.33 and 0.17 seconds).
The x86 19 manifest/corpus/DAG/layout/negative cases pass in 86.55 seconds.
Final-source x86 XT/default boot checks pass in 23.53/2.74 seconds; Model40's
last x86 boot/CMOS checks are running. Documentation and diff checks pass after
the source-layout/provenance receiver correction. No six-corpus or MyNES
source, test, configuration or artifact changes appear in Git status.

Final receiving verification passes: Model40 x86 boot reaches its installer
in 63.21 seconds, and the unchanged CMOS predicates pass in 0.07 seconds.
S2's full original unit set passes 506/506 on each width; original AT
Console/CMOS/boot predicates pass on each width; every affected PC receiving
product is built with a passing original boot checkpoint on both widths.
Both specialized aggregates and all 19 additional corpus/layout checks pass
per width. No new timing grade, controller behavior or external master is
claimed. All eight PC EXEs and the two unchanged MyNES EXEs are retained;
My5170's superseded 0542 pair is removed, recoverable from Git history.

Executor actual-change review covers the added/removed/relocated C/H owners,
fixed bindings, selected graph, build registration, original test predicates,
INI rebase, manifest and documentation changes. The App definitions contain
only model values and constraints. Shared ROM/candidate projection retains
the sole resource owner and cleanup path. No peer-App production include or
Lib/Common/x86/MyNES diff remains. Family test support and cross-profile
assertions are live test-only receivers explicitly assigned to S4, not a
second production route. S2 is ready for separate Shared and NXVM P delivery;
coordinator acceptance follows the pushed actual diff, not this report alone.

Shared implementation P1 is pushed at e9bb0dc55. Final deployed SHA-256:

| App / width | SHA-256 |
| --- | --- |
| My5160 x64 0543 | CE4B0E79B07DA5C44AB5E347BEC879BEEBCA2FCD972F89C8E343AA473354FDCA |
| My5160 x86 0543 | AFD22833FD248022313566E0709DFE30D81433B7DFBEDCE57A3154DE50A57C8E |
| My5170 x64 0543 | 343D0BDC5A3723FD4CD9CAB180CADCC8D256C6832509941B7B8B8A33D86CA2C5 |
| My5170 x86 0543 | C66A2739964B82C86CDBF4F94975ABF720B3BD5EA301EF9CDF5185857DB85BBB |
| NXVM default x64 0542 | 67B47DB1444A1126677961FECA359EFCAC9743428E19F4B5EA9411BC4444E0B4 |
| NXVM default x86 0542 | 41445E21DCB9D9FDF9F6B32C733BF5D658B17A85DB5070C214BBC7F39CB98145 |
| DeskPro386 x64 0542 | 4F68B2ADB6FDEEE59FAE22284AF0FA716C46B5D6853D9C99B96A200D46E0F089 |
| DeskPro386 x86 0542 | 03756414CEDC6DE800C59FCE6728A85ACEEC2EA87AF46F7AB572B7D1BBEDF77E |

My5160 and My5170 deploy directly in assets/my5160 and assets/my5170.
Their INI hashes are EC2AFB0E89421ED4BDA95D8B1797864D19EC360A3AD57237D2256DCE78950338
and F4D85EE2E7BFF403F8C19588DA8416CC96FE5AC72FE7A1764817EDDDC9B76F19,
respectively. The INI diff only removes one relative path level; original
media bytes, access modes and other settings are preserved. These hashes
supersede intermediate S2 output hashes, not the historical S1 evidence.

## S2 Coordinator Acceptance

Reviewed the pushed Shared/NXVM changes against the original request, S2 packet,
coverage row and architecture/coding/source rules. Common AT projection and
candidate teardown have one shared owner; Apps retain values and validators.
The ROM algorithm is relocated, not forked. My5170 uses the shared Product
runtime without peer-App production dependencies. Moved tests preserve their
assertions; the CMOS fixture now uses compiled identity rather than a retired
directory name, retaining both expected rows. Live test-only family support
and cross-App matrices have S4 as their explicit receiver.

Matched deployed hashes to the final records and reviewed scoped commits,
INI rebase, artifact retirement and absence of six-corpus/MyNES changes.
The complete unit, affected integration and gate results satisfy S2, not T exit.
S2 is accepted and closed; S3/S4 remain unaccepted. App-boundary and flat-root
checks guard the new fixed My5170 route against regression.

## S3 Admission Inventory

Reference 1eb905a85 is clean and pushed. Model40 has 15 tracked source/header
files: composition, construction, machine_plan, model40 definition, D4 memory,
D4 platform, copied observation and ROM mapping. They form one existing owner,
not a new shared mechanism. The family CMake currently lists its D4 in runtime
and observation graphs and its composition as the selected Model40 library;
all those lists must follow the same moved implementation. Model40-specific
unit/ROM/refresh fixtures and external integration source/registration follow
the new App; mixed-family support remains a single test-only S4 receiver.
The immutable entry and firmware roles become App-owned. No chip algorithm,
Compaq personality, CMOS seed, original assertion or external master changes.

S3 executor confirms the packet. The 15 source files and 30 Model40-specific
test/fixture files move with git mv; all direct includes and family build paths
are repaired. The App owns a thin entry/fixed binding, composition source list,
D4 source list and original Compaq BYOB declarations. The first selected
Model40 configure passes the independent-App and transitive production graph
checks; generation/build are not yet complete and no runtime pass is claimed.
The original INI moves to assets/mydeskpro386 with only a one-level path rebase;
the previous 0542 pair remains until replacement verification. Mixed-family
test support remains a single live S4 receiver, not copied into the new App.

The x64 selected product configure and build finish successfully, including
the MyDeskPro386 source-only composition library, firmware embedding, thin
binding/entry and PE x64 verification. Its 0543 EXE is deployed directly under
assets/mydeskpro386. This is build proof only: full unit and integration runs
and the x86 product remain outstanding. A post-move include sweep corrects
relative fixture references, including the moved model40.h's bare profile.h;
the original multi-family helper remains single-owned until S4.

The resumed S3 actual-source check compares all 15 relocated source/header
files against 1eb905a85 after normalizing only the moved include prefix: no
other difference remains. Tracked source/test changes likewise contain only
include repairs; the new fixed entry/binding does not copy Product behavior.
All relative includes resolve. The family INI gate follows each relocated
integration/unit tree and the direct MyDeskPro386 INI; it passes without
weakening predicates. The obsolete unreachable Model40 selected-library
branch is removed. The two INI path changes still resolve to the original
1,228,800-byte floppy and 40,256,000-byte hard-disk masters with the same
overlay mode. Full x64/x86 test builds are in progress; runtime acceptance is
not yet established.

Both complete test builds and the selected 0543 products now compile on x64
and x86. The first x64 test build exposed one remaining bare fixture include
in the relocated FDD test; its include now points to the existing single
family fixture, and the complete retry passes. No assertion or fixture bytes
changed. Both product flags are -O3/-DNDEBUG, runtime trace is disabled and
the linker strips compiler debug information; objdump confirms PE x64/x86
and no debug sections. Runtime Debug remains linked through x86-debug.
The current product hashes are ED2267E129A56ED64278878C4C6E8A54798B1AF3847AC6DE059A3B67F0EAD86A
(x64) and 5F63FE5B853623E41C339E58EFCB8B4BF5C80AAEC526E59BAA3F004DD3D1D72D
(x86). Full unit and original integration execution remains outstanding;
these are build identities, not acceptance evidence.

The supplemental manifest/corpus/DAG/layout/negative CTest selection passes
19/19 on x64 (138.50 seconds) and x86 (112.52 seconds). Its expression selects
only those checks, not the shared runtime units, which remain in the complete
unit aggregate. Documentation governance passes for NXVM. The six shared
source/test corpora, ibmpc and MyNES paths have no S3 diff. Flat deployment's
owner INI hash is C812A0C99D258C19CA8BB4BED584617DF2F4D25F17372583EB58BBD71D378599.

The x64 complete run-unit-tests target passes 506/506 in 216.96 seconds;
its dependency build and bounded aggregate both exit successfully. The x86
unit and original Model40 integration executions are still required.

The x86 complete bounded unit aggregate passes 506/506 in 213.04 seconds,
reusing the successful full test build with the identical run-unit-tests
command. A relative TestDirectory attempt failed before starting any test;
the successful invocation uses its resolved absolute build path. x64's three
original Model40 integration checks pass (55.25 seconds total, boot 54.84
seconds). Both specialized gate aggregates now pass. Their first attempts
exposed two migration omissions: the historical constructor glob lost the
two moved D4 tests, and the fixed-constructor verifier did not read the new
App CMake. Both retain their original requirements; the 133-constructor
predicate is unchanged. The same scan repairs Model40 coverage in firmware,
media, controller, display, CPU/PIC, D4 and machine-owner verifiers. Direct
checks of those repaired gates pass. Expected negative-probe diagnostics
remain part of a passing self-test, not unreported gate failures.

Final source comparison: all 15 moved C/H bodies match the baseline after
only include-prefix normalization. No tracked test content changes beyond
include directives remain. Runtime publication, D4 lifetime, Compaq protocol,
CMOS and media configuration are unchanged; the new thin entry/binding uses
the sole shared Product path. The x86 integration run remains outstanding.

## S3 Executor Delivery

x86's original integration checks pass 3/3 in 68.33 seconds, with boot reaching
the installer in 67.81 seconds. Each original context ran once per width;
there is no repeated-success qualification or changed terminal predicate.
The replacement identities above match both deployed files. The previous
Model40 0542 pair is retired after verification, recoverable from Git history;
assets retains ten EXEs: eight PC-family and two unchanged MyNES artifacts.
MyDeskPro386 now deploys its pair and unchanged-policy INI directly at its root.

Code accounting uses rename-aware git diff HEAD --numstat under src/test/cmake,
plus complete line counts of newly added files, excluding docs and artifacts.
Production C/H: +51/-16, net +35; test C/H: +42/-42, net zero; build/registration
and verifiers: +165/-95, net +70. The 35 production lines are the independent
thin entry, fixed binding and public declarations, not another runtime.
The build increase relocates model values and extends existing owner checks
to the receiving App; it adds no device, parser or execution framework.

Executor self-review covers actual source/test relocations, fixed source graph,
BYOB declarations, D4 ownership and lifetime, every repaired gate, original
test predicates, artifact/INI identities, documentation and excluded targets.
All S3 criteria are proven; coordinator acceptance must still review the
pushed actual change. Family-wide fixture/registration reconciliation and
the complete 58-context replay remain the original S4 receiver, not S3 proof.
