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
| S2 IBM AT | IBM AT definitions inside app-nxvm/profiles/default_profile, IBM AT assertions | app-my5170 and test/app-my5170 | Admitted sequentially after S1; reconcile shared mechanisms rather than copy a peer App. |
| S3 DeskPro | app-nxvm/profiles/model40, D4, ROM and copied observations, Model40 assertions | app-mydeskpro386 and test/app-mydeskpro386 | Planned; retain actual Compaq-specific ownership. |
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
