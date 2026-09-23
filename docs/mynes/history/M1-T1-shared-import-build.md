# M1 T1: Shared Import And Independent Build

## Request And Scope

Owner approved the first M1 package following planning 45f9724 and authorized
MIT reuse of their own shared code. Source pin is SoftPC
58ac3943170e2e68879d83b49d184a8a84f6c471. One session executes and coordinates.
No NES engine, App, ROM, sibling mutation or next package is admitted.

## Frozen Convergence Ledger

S1 consumes the entire finite input universe: 89 Lib source, 32 Common source,
44 Lib test and 22 Common test files; 187 in total, including build/docs/manifests.
Each per-unit row records source/destination hashes, effective license and delta.
New local build files are additional owned units, not hidden replacements.

| Batch | Required proof | Disposition |
| --- | --- | --- |
| Rights and provenance | Owner grant, headers/history review, exact pin and per-unit ledger | Passed; evidence below |
| Import completeness | 187 source and destination identities, no external guest inputs | Passed; evidence below |
| Build portability | C11, extensions off, warnings, assertions, explicit roots; all four entry points | Passed; evidence below |
| x64 aggregate | Full configure/build and every registered CTest case | Passed; evidence below |
| x86 aggregate | Full configure/build and every registered CTest case | Passed; evidence below |
| Independent roots | Production Lib/Common and separate test suites build/run without product integration | Passed; evidence below |
| Integrity and dependencies | Four manifests and source/test static/negative gates | Passed; evidence below |
| Governance and actual review | Active/final documentation gate, actual import delta, cleanup and source-line count | Passed; evidence below |

Accept only with all eligible rows passed. A failed case remains blocked until
fixed within scope; platform-inapplicable paths need a named reason. Runtime
design gaps remain explicitly assigned to the later M1 candidates and are not
certified by this build/import task. No imported case is silently discarded.

## Verification Contract

Use shared-x64/shared-x86 configure/build/test presets; record exact compiler,
CMake and generator versions. Test assertions remain active in Release and
each case has a finite timeout; timeout is failure. Run suites serially to avoid
native Console resource collisions. Independently configure source Lib/Common
and test Lib/Common with explicit source roots. Static negatives must fail for
the expected reason. Native-test limitations remain named and do not become
gameplay or terminal compatibility claims.

## Similar-Issue Sweep And Transfer

Inspect every CMake root, test source path, C standard, warning/assertion option,
and relocated README/verifier path. Preserve one implementation per component;
no product-name conditionals or sibling build paths. x86 debug/xasm code and
its tests are explicitly retained under the approved staged import; the next
neutral-contract package owns removal and replacement of neutral test proof.
Lifecycle and equal-presentation gaps belong to their existing queued owners.

## Delivery

Accepted implementation P1 `1911af9` after coordinator actual-commit review; closure delivery is `M1 T1 S1 P2`. Both are local commits because no remote is configured. No product artifact applies.


## Import And Portability Results

All 187 pinned units imported: 168 byte-identical, 19 modified. The
[provenance](../etc/provenance/softpc-shared.md) and its per-unit CSV retain source
and destination hashes and the owner MIT grant. No external notice conflicts or
recovered engine/ROM dependencies were found in the selected corpus. No files
were taken from SoftPC's working tree; later concurrent changes there are outside
this task and were not modified.

Four roots retain existing component implementations. Test entries now accept
explicit Lib/Common roots rather than fixed test-relative paths. Common/tests
select C11 instead of C17; all inspected compile commands use -std=c11 and
-Werror, with -UNDEBUG for runtime tests. The eight modified C files only make
initialization explicit: five session event constructors, two debug trace counts,
decoder SIB, native wait handles and four test initializers. Existing regressions
exercise these paths; no public ABI or successful runtime policy is changed.

Strict Common/test compilation exposed inherited unused parameters/functions,
intentional table fallthrough and empty trace-macro branches in two xasm source
files. Four named diagnostic suppressions are limited to aasm32.c and dasm32.c;
other diagnostics remain errors. This approved import-specific treatment preserves
the old implementation until the queued neutral-contract removal, instead of
rewriting its decoder during a build task. There is no global warning suppression.

The first x86 configure failed because its compiler helper DLL directory was not
on the process PATH. The verification runner now selects that directory locally
and restores PATH; no system setting or machine path is committed. Presets check
requested pointer width. A deliberate x64-compiler/x86-width configure failed
with the expected architecture diagnostic.

The exhaustive static types-layout self-test exceeded the original 30-second
default on x86. It launches hundreds of finite CMake dependency probes; its CTest
timeout is now explicitly 180 seconds. Runtime cases retain their 30-second bound.
The full affected suite was rerun after this build-only budget change and passed;
no failure was waived and no test was removed.

## Executed Verification

Host toolchain: CMake 4.3.3, Ninja 1.13.2, MinGW GCC x64 16.1.0 and i686 GCC
16.2.0. Both builds are Release, C11, extensions off with static runtime linkage.
The 51 runtime executables per architecture were checked for PE machine type
(AMD64 and I386 respectively). These are tests, not emulator developer artifacts.

| Entry | x64 | x86 | Evidence class |
| --- | --- | --- | --- |
| Root shared preset | 65/65 passed | 65/65 passed | 51 runtime + 14 static cases per architecture |
| Standalone src/lib | Built | Built | Production library build |
| Standalone src/common | Built | Built | Production library build with explicit Lib root |
| Standalone src/test/lib | 43/43 passed | 43/43 passed | Independent suite, no root project |
| Standalone src/test/common | 22/22 passed | 22/22 passed | Independent suite with fake machine drivers |

[Per-case results](../etc/evidence/m1-t1-tests.csv) record all 65 inherited cases
in both aggregate and independent runs, with no skipped/disabled result.
Commands: shared-x64/shared-x86 configure/build/CTest presets and
tools/Verify-SharedStandalone.ps1 for each architecture. Manifest, DAG, naming,
type-layout and negative fixtures are registered CTest cases. Static results are
not runtime claims. Linux algorithm fakes do not certify a Linux native host.
Native probes retain their original finite coverage; they do not prove NES
gameplay, all Console hosts, ASCII rendering or real downstream adapter integration.

## Actual-Change Review And Cleanup

Coordinator reviews the imported identity ledger, every source/build delta, new
root presets/verification script, notices and documentation against the original
request. Dependency owners and runtime paths are preserved. No alternate state,
dispatcher or product dependency was added. All compiler-reported initializer
variants were inspected; complete suites and strict compilation guard the changes.
The root architecture rejection was separately exercised. Final documentation,
staged whitespace/scope and imported-hash checks pass before implementation commit.

Counted code/build paths: 175 files under the four roots with .c/.h/.cmake
or CMakeLists.txt, plus root CMakeLists.txt and the new verification PowerShell
script. Relative to the source-free MyNes baseline: +39702/-0, net +39702 lines.
Documentation, manifests, JSON presets and generated files are excluded from this
code count. The increase is primarily the approved reusable import, not a second
implementation. Against the pinned source, C/header changes are +24/-14,
net +10 lines. Existing x86 and platform variants remain for their
distinct admitted import semantics, with future neutralization explicitly queued.

Cleanup completed after evidence reconciliation: no owned test executable remained
running; all 11 explicitly named generated build/configuration trees and the
negative-configure log were removed after absolute-path containment checks.
Only committed source, manifests and compact test results are retained; no emulator
artifact exists and no sibling worktree was cleaned or modified.


## Closure And Successor

All frozen T1 batches passed. S1 and T1 close; M1 remains open. The retained
[proposal](M1-T1-shared-import-build-proposal.md) records the original scope.
The next queued neutral-contract package owns x86 separation and paused transport;
it is not admitted by this closure. Lifecycle, input and transfer qualification
remain later M1 work. No new unplanned debt or sibling mutation is introduced.


## S2 Refresh Admission And Brief

Owner requested an updated import and explicitly reopened T1. The latest closed
numeric task receives S2 under the Corrective allocation path; this in-scope
refresh does not reject or rewrite S1 acceptance. No successor was admitted or
withdrawn, so the specialized Owner-Reopen allocation mode does not apply.
One session performs implementation and then coordinator review sequentially.

Freeze SoftPC 8cb23e914a00ad607e12c89d7b2c65b87fcecdca, whose parent is the
S1 source pin. The input universe is 188 units: 89 Lib source, 32 Common source,
45 Lib test and 22 Common test files. Upstream changes 29 paths, including one
new native Linux storage-lock test. Read fixed Git blobs, never mutable source
worktree files. Three-way reconcile the previous source, MyNes baseline be133c4
and new source; preserve the accepted C11, assertions, warning and root-path
adaptations. Regenerate local manifests and the current per-unit provenance.
The original S1 ledger is retained in its committed revision and S1 test CSV.

| S2 batch | Required proof | Disposition |
| --- | --- | --- |
| Rights and completeness | Existing owner grant, new headers, 188 pinned units and updated hashes | Passed; S2 review/results below |
| Delta and ABI review | All 29 upstream paths and local adaptations; failure-retention callers and platform variants | Passed; S2 review/results below |
| Aggregate verification | Strict C11 x64/x86 full builds and 65 applicable tests each | Passed; S2 review/results below |
| Independent verification | Four roots built on each architecture; Lib 43 and Common 22 tests each | Passed; S2 review/results below |
| Platform boundary | New native Linux lock test retained, Linux host execution explicitly not claimed | Passed; S2 review/results below |
| Governance and delivery | Documentation gate, actual P1 review, code delta, cleanup, local commits and reclosure | Passed; actual P1 accepted and S2 closure recorded below |

S2 uses the original verification contract and scoped transfer boundaries above.
The current provenance ledger represents this new source revision; historical
S1 source hashes remain retrievable at be133c4. The future lifecycle candidate
still owns broader neutral lifecycle qualification; importing upstream fixes does
not accept that candidate or certify all terminal hosts.


## S2 Import And Actual-Delta Review

All 188 pinned units are present: 169 byte-identical and 19 carrying existing
MyNes adaptations or regenerated local manifests. All source/destination SHA-256
rows and all four upstream plus four local manifests were independently checked
against the fixed Git blobs and filesystem inventory. Three-way merges were
needed only for Common README, Win32 Base sync and Lib test CMake; they were
conflict-free. The original eight C initializer fixes and independent-root,
C11, warning, assertion and timeout settings remain. No source was read from
the mutable SoftPC worktree. The added lock test is owner-authored source;
header/history/dependency review found no new independent license or dependency.

The complete 29-path upstream delta was reviewed, including the added native
Linux test. The imported failure contract returns lib_status from Base task
cancel/join/destroy and Common machine shutdown/destroy, retaining resources
when worker exit cannot be proven. The Common destruction chain propagates
failure before clearing/freeing the worker. The internal create-failure cleanup
has no successfully created worker and therefore safely completes. Remaining
callers in this corpus are normal-path tests; updated failure-injection tests
check cancellation/join failure, retained resources and successful retry on both
platform algorithms. There is no MyNes product adapter yet. Downstream receivers
must recompile and handle failure before releasing borrowed callback contexts.

Window changes consolidate startup cleanup without replacing the original error;
new title/notify failures are tested. Geometry uses upward integer rounding with
stable refit probes, including narrow/extreme aspect ratios. Linux storage changes
from process locks to advisory open-file-description locks; fake tests cover lock
selection and failed-open cleanup, while the new native test covers independent
opens and hardlink aliases. Windows runs do not claim that native Linux proof.
These upstream fixes do not close the queued neutral/lifecycle/presentation work.

Against MyNes be133c4, the precommit line-based comparison counted 23 changed
code/build files with +245/-112,
net +133 lines, including the new 40-line native Linux test. Count includes
.c/.h/.cmake and CMakeLists.txt, excluding docs, manifests, CSV and generated
files. This is the upstream refresh only; no duplicate implementation, product
branch or new local runtime adaptation was introduced.


## S2 Executed Verification And Delivery Preparation

The [S2 case ledger](../etc/evidence/m1-t1-s2-tests.csv) was generated from all
six CTest JUnit reports and reconciled by exact case identity against the S1
65-case inventory. Both aggregate reports have 65 passing cases; standalone
reports contain Lib 43 and Common 22 for each architecture. No applicable case
was skipped, disabled or removed. The extra Linux-native lock case is registered
only on Linux and is explicitly marked not applicable in all Windows columns.
Each architecture therefore executes 51 runtime and 14 static cases, with the
same scoped claims and limits as S1. Linux algorithm fakes are executable tests,
but are not native Linux host acceptance.

| Entry | x64 | x86 |
| --- | --- | --- |
| Root shared preset | 65/65 passed | 65/65 passed |
| Standalone src/lib | Built | Built |
| Standalone src/common | Built | Built |
| Standalone src/test/lib | 43/43 passed | 43/43 passed |
| Standalone src/test/common | 22/22 passed | 22/22 passed |

Commands: cmake --preset shared-x64/shared-x86; cmake --build --preset for each;
ctest --preset for each with absolute JUnit destinations; then
Verify-SharedStandalone.ps1 -Architecture x64/x86. Runs were serial. Toolchain
remains CMake 4.3.3, Ninja 1.13.2, GCC x64 16.1.0 and i686 GCC 16.2.0.
All ten build-entry compile databases enforce C11 and warnings as errors;
test compilation retains assertions. The 51 aggregate test executables in each
architecture were inspected for AMD64/I386 PE machine type. These are shared
component tests, not an emulator artifact. Root presets and standalone runner
are unchanged; their original S1 negative-width proof remains historical.

The initial admission-document checks caught CRLF output and missing active
proposal/Queue indexing. Those were corrected before source import; the normal
governance gate then passed without changing its rules or checker. Runtime and
build verification passed on the first S2 runs. Full pinned/hash/manifest audit,
source delta self-review and whitespace checks passed. No new unplanned debt or
sibling edit was introduced; broader contract work retains its queued owners.

All ten owned build directories and eight task logs were removed after reconciling
results, checking no owned test process remained, verifying absolute containment
under build, excluding build/output and rejecting reparse paths. Only compact
committed evidence is retained. No product artifact is applicable.

Implementation delivery is M1 T1 S2 P1 (`5a59835`), local-only because no remote
is configured. The implementation record left acceptance pending; the following
coordinator review completes it.


## S2 Coordinator Acceptance And Reclosure

Accepted actual implementation commit 5a59835 after reviewing its source/build
changes, ABI boundaries, provenance, test evidence and scope against the owner's
reimport/reopen request. All 188 destination hashes were rechecked against the
committed blobs, not merely the worktree. Source roots and retained adaptations
match the tested revision; no omitted unit, unresolved failure or added product
scope was found. The post-commit active-packet governance gate also passed.

The actual-commit line-count check initially differed from the precommit Python
line-based diff because Git groups matching lines differently. Reconciled Git
numstat is 23 code/build files, +240/-107, net +133; the earlier +245/-112 has the
same net and uses the precommit comparison method. This is a counting-method
clarification, not a source change or failed runtime check.

All frozen S2 batches are accepted. This pure-governance delivery is M1 T1 S2 P2;
both P commits are local because no remote is configured. The
[refresh supplement](M1-T1-S2-shared-import-refresh-proposal.md) is archived
alongside the unchanged S1 proposal and evidence. S2 and T1 close again; M1
remains open. The four successor candidates retain their existing order and
boundaries and none is admitted. Native Linux execution and downstream adapter
integration remain outside this Windows import/build acceptance.
