# M5 T545 SoftPC Eight-Corpus Refresh

## Admission And Frozen Universe

Owner explicitly admits a new audit/conditional-import task after closed T544.
NXVM baseline is 826eccc93. The source is the read-only owner-authored SoftPC
sibling. Initial HEAD 3537e87a advanced to 03c979c7 and then governance-only
9291c8cc. The accepted eight-root snapshot is 03c979c7; the later committed
HEAD has identical corpus bytes. Subsequent dirty IBMPC work is excluded.
The [proposal](../proposals/m5-softpc-eight-corpus-refresh.md) owns scope.
CPU audit repairs remain first in the candidate Queue, not admitted here.

## S1 Audit Batch

All eight src/test roots of lib/common/x86/ibmpc, shared test-root dependencies,
and all NXVM/MyNES consuming source/build/test/artifact surfaces are the finite
coverage domain. Each changed file, removed test and public contract needs
an explicit identity, source/provenance review, actual diff disposition and
receiver proof before unchanged import. Current owns the active S packet.
Shared source/tests and receiving App inputs remain unmodified during S1.

The [audit](../etc/evidence/t545-s1-eight-corpus-audit.md) and
[complete receiver inventory](../etc/evidence/t545-s1-file-receivers.md)
accept the fixed committed snapshot with bounded receiving adaptations.
45 x86 paths migrate to IBMPC; 44 product-owned files must be retained under
NXVM test ownership. Six root helper files accompany the exact eight trees.
Baseline units pass 506/506 per width. Production/API review, eight manifests,
Types/inward ownership, source DAG/corpus checks and independent strict-warning
IBMPC build and 155/155 runtime aggregate pass (98.09s). The documentation
governance gate and diff check pass. The source/test change count in this S is
zero; no binary inputs changed and no product artifact is rebuilt for S1.
S1 P1 78b30bca7 is pushed. Coordinator actual-diff review accepts its complete
fixed identity, semantic review, removed-test map, verification and target-only
document changes. S1 closes after its governance P; S2 is admitted under the
owner's conditional import authorization. Latest uncommitted source is not
included in the accepted baseline.

## S2 Import And Receiving Batch

Import exact accepted eight trees and six explicit test-root helpers. Retain
44 product files at the mapped NXVM receivers, mechanically repair registrations
and includes, then verify all consumers and affected dual-width artifacts.
Shared, NXVM and MyNES deliveries have separate target-correct P commits.
Original 58 integration predicates remain T/S3 verification; CPU repairs and
owner INI/media/snapshot changes are excluded.

### Owner-Requested IBMPC Refresh

The owner explicitly requested the newer IBMPC src/test pair during S2.
The pinned source advances to `8124e551e841ccdec2ceb7f6a0f6ae5b513a7951`;
both IBMPC source paths are clean upstream. The six other roots have no Git
diff since `03c979c7`, so the refreshed eight-root set is one committed source,
not a mixed uncommitted snapshot. Old-source builds were stopped before copying;
their results do not qualify the new pair.

Actual review accepts owner-local embedded run/reset atomics, removal of
first-media-slot union aliases and failure-preserving Product teardown/binding.
Atomic operations and hardware algorithms remain unchanged. New composition
tests exercise failed UI destruction and failed Machine cleanup, including
retry without releasing still-borrowed callbacks.

The newer source removes 28 test files. Twenty-seven are retained at NXVM-only
receivers: both historical CPU timing generators remain under unit/board;
25 helper files move together under unit/support/ibmpc, preserving their local
relative topology. Registrations, includes and historical gates follow them.
The remaining unregistered vm_debug_authority_smoke used removed flagTrace/
traceCount APIs; it is retired, not represented as a lost passing regression.
Its current execution-plan behavior is covered by the imported
machine/debug_budget_smoke, with trace counts 1/10/4096, breakpoint/watchpoint
completion and reset assertions. All original registered product predicates
remain required. Receiver tests use slot-zero arrays and the address of the
embedded control state; no imported source is patched or shimmed.

Fresh raw path/hash comparison passes for all eight roots: src counts
109/23/95/125 and test counts 49/20/180/241 for Lib/Common/x86/IBMPC.
All eight complete manifests pass independently. Shared P1 fc3c73aa1 fixes
the exact imported Git blobs and root helpers; MyNES P2 f010812fd delivers only
its rebuilt 0043 pair. NXVM P3 delivers retained receivers, link/build paths,
all eight 0545 PC artifacts and this acceptance record, with no owner INI edit.

Fresh complete units pass 531/531 per width (223.46s x64, 147.44s x86), and
MyNES product units pass 43/43 per width (23.97s x64, 40.43s x86). Both widths
pass all 23 supplemental manifest/corpus/Types/DAG/layout/naming/negative
checks; the x64 specialized aggregate passes. All ten optimized stripped
artifacts pass PE-width and compiler-debug-section checks; hashes and source
provenance are in the [refresh evidence](../etc/evidence/t545-s2-ibmpc-refresh.md).
Superseded PC 0543 artifacts are retired only after verified replacement.

Coordinator actual-change review accepts the full S1 universe, latest 46-path
IBMPC diff, all 71 retained product files, mechanical receiver layout/include
changes and selected board/Core fixture archive closure. No Shared local patch,
new runtime/API, device algorithm or test predicate is introduced. One retired
unregistered obsolete Debug API test has an explicit current-coverage receiver.
Documentation governance and actual reference/status review pass. S2 closes
after target-correct P delivery/push; T stays open for S3. All 58 original PC
external integration contexts and the receiving MyNES integration suite remain
required, not represented as freshly passed by this import.

## Owner-Approved Test Completion Plan

S6 Shared P1 `f785e71a9` and NXVM receiver P2 `efc153d22` complete IBMPC's
generic Machine memory/media and shared AT preparation proof. The [S6 ledger](../etc/evidence/t545-s6-ibmpc-test-completion.md)
records the complete contract/assertion map and corrected fixture failures.
Standalone suites pass 181/181 and full units 532/532 per width, with six gates
per width. Coordinator review accepts original registrations/markers, generic
assertions moved to their owner and preserved actual App model choices. Three
Shared C tests add 207 lines; two App tests change +16/-50 (combined net +173).
No production/API/configuration/assets/EXE input changes. S6 closes with its
governance P; all four requested test batches are accepted. Original S7 remains
planned for external qualification; neither this acceptance nor unit green
claims T closure, new CPU manual qualification or fresh integration success.

S5 Shared P1 `d610343e2` and NXVM receiver P2 `302aa18f6` deliver owner-local
Core/chip/CPU tests and five preserved relocations. The [S5 ledger](../etc/evidence/t545-s5-x86-test-completion.md)
maps contract families, old assertions and discovered/corrected verification
failures. Independent x86 passes 173/173 and full units 532/532 per width;
twelve relevant gates pass per width. Decoder source bytes/count/output remain
identical across standalone and historical receivers. Actual review accepts
neutral dependencies, no lost predicates and no production/API/App asset change.
Shared tracked C/H/CMake: nineteen paths, +712/-107 (net +605), with direct
missing proof added to existing fixtures and one split timeline executable.
NXVM build receiver: four path replacements (+4/-4). Current EXEs need no
rebuild. S5 closes with its governance P; S6/S7 remain planned and T stays open.

S4 delivers Shared P1 `6955f7093`: public Session lifecycle/ingress and specific
lifecycle dispatch, UI failure cleanup/control/event mapping, publication copy
and run identity. The [S4 ledger](../etc/evidence/t545-s4-common-test-completion.md)
records the complete owner-local inventory, retained assertions and standalone
20-case proof per width, current full 531/531 units per width and six Common
gates per width. Coordinator review accepts four reused tests +313/-12,
README +3/-1 and manifest +6/-6, with no production/API or executable change.
S4 closes with NXVM acceptance; S5-S7 remain planned and T stays open.

S3 delivers Shared P1 `bd68089a8`: direct Lib contract tests in existing fixtures,
with no production/API change. The [S3 proof ledger](../etc/evidence/t545-s3-lib-test-completion.md)
maps all contract families and records the initial corrected manifest failure,
standalone 51-case proof per width, full 531/531 units per width and 19 gates
per width. Coordinator actual-change review accepts original assertion retention,
own-suite independence and failure/lifetime coverage. Test source +198/-3,
README +5/-3, manifest +8/-8; unchanged executable inputs need no artifact rebuild.
S3 closes with its NXVM acceptance P. S4-S7 remain planned and T stays open.

After S2 acceptance, the owner requests four additional bounded S tasks for
clean, self-sufficient Lib, Common, x86 and IBMPC testing. Read-only review
found owner-local gaps in Lib medium replacement, Common public Session
construction/ingress, neutral x86 Core contracts and generic IBMPC Machine
contracts currently demonstrated by App fixtures. API reference counts are
diagnostic only, not branch-coverage proof.

Coordinator plans S3 Lib, S4 Common, S5 x86 and S6 IBMPC in that order. The
original S3 final qualification was never admitted and moves to S7 without
discarding any original external predicate. The proposal owns the bounded
briefs and common gates; no implementation packet is active yet. S2 stays
accepted and its exact SoftPC baseline is unchanged. Later owner-approved test
revisions require new manifests and truthful identity, not an unchanged-import
claim. Production repairs require further concrete review; sibling repositories
remain read-only.

This planning-only follow-up changes NXVM task records, not source, tests,
configuration or executable inputs. Current verified EXEs need no rebuild.
