# T546 S7 Build And Test Efficiency

## Owner-Directed Admission

The owner inserts this batch before the paused uncommitted FLAGS work.
Former S7 becomes S8, former S8-S19 become S9-S20; no committed S7 P is
renumbered. S1-S6 remain accepted. Preserve MyNES work and all owner inputs.

## Baseline And Complete Audit

Accepted S6 complete units: x64 541/541 in 248.83 seconds, x86 541/541 in
233.31 seconds, with 300-second containment and j4. These are historical
qualified runs, not controlled new benchmark measurements. Eight PC products
and original 58 external contexts are already qualified against S6.

Actual current default cache is MinGW Makefiles. Its generated top Makefile
declares .NOTPARALLEL. Sending hundreds of /fast goals together therefore
serializes top-level target recipes; j4 does not make those independent links
parallel. S6 relink inventory contained 407 current CPU archive consumers
per default width. Generated stale recipes are not active targets and must not
be scheduled merely because a build.make survives.

CTest cost records also contain source-boundary self-tests that repeatedly
copy and inspect a whole corpus. Audit their owned violation predicates and
registration overlap before changing scheduling or removing a case.
Performance alone does not make a correctness test invalid.

Audit units: dependency-valid no-op/incremental compiler cache; sole archive
and independent consumer links; selected current registrations versus stale
build files; transient probes/focused sets; complete source-owner units;
external integration resources; static positive/negative checks; artifacts
and failure propagation. Required dispositions are optimized with retained
proof, removed with exact duplicate/obsolete assertion mapping, unchanged
with a distinct ownership reason, or explicitly excluded.

## Planned Execution Boundary

Keep existing CMake compile/link/strip/deploy recipes. Prefer the native
dependency graph and persistent caches; if the existing Make generator
requires independent leaf scheduling, only schedule after shared prerequisite
completion, with disjoint output ownership and real failure propagation.
Do not invent another C source list or link command, suppress rebuilds through
timestamps, or treat compilation as runtime proof.

During exploration build changed targets and run a transient focused choice.
Final S acceptance runs complete required units once against the final code.
External integration is needed for changed execution/input or integration
mechanics, not for a documentation-only or unrelated tool probe. Keep all
original 58 contexts and their source-owned checkpoints. No fixed focused
suite is added to source or permanent configuration.

At admission no implementation or speedup is delivered. No executable input
change is planned, so do not manufacture new PC or MyNES binaries.

## Deferred S8 Preservation

Save the exact uncommitted test bodies, prior manifest and Git patch under
ignored build/t546-s8-deferred before restoring those three tracked files to
accepted HEAD. The restored test tree has no Git diff; no CPU source changes
exist. Saved SHA-256 identities:

- cpu_pushf_popf_smoke.c: 08CAF7A473AA300518A37BD09765CCE544C872AE088231A93341D04C93FEF2DE.
- cpu_debug_state_smoke.c: 465F70957A6C0EEF289B84C7CAAE1FDAACCEAA369ED65BDEC728AC2D106FA56A.
- MANIFEST.sha256: 3279CCA1FF676C60897A4E46572BFEEBD5AA42E16D3720EAB5BE75DCD6F37596.
- reproducers.patch: EEC80AD1035D9548444CC702D77EDE39AC62F5FC333D12CFCCAF787EAA4566D8.

Retain these local files and original research for S8. Restore their corpus
revision as S8 and refresh the resulting manifest on actual admission. Their
intentional failures are neither discarded nor counted as passing S7 proof.

## Initial Implementation And Measurements

CTest's accepted-cache JSON inventory identifies 36 duplicate command pairs:
unit.x86-test-cpu_* aliases and their canonical x86.cpu_* registrations execute
the identical chip-owned binary with identical arguments. Retire only the
NXVM aliases. Keep all C assertions and canonical owner registration, 30-second
timeout and actual test directory. The registration guard now recognizes the
canonical command's executable target instead of requiring its product alias.
Its exact expected-target and duplicate-route checks remain. The final Ninja
configure reports 505 unit registrations and 22 default integration rows;
the registration gate passes with the retained 334 target identities. Runtime
command equality is checked only after binaries exist: CTest omits command
fields for missing executables, so grouping empty fields is not a duplicate
test finding.

Add build-unit-tests as the one dependency-only entry; run-unit-tests depends
on it and executes the existing bounded aggregate. Existing dependency evidence
reads that entry's explicit dependency property. Separate unit j8 from external
integration j1 defaults, retaining the 300-second containment. The unit build
preset now selects optimized Ninja/ccache rather than the separate Debug tree;
add the symmetric x86 cache/build preset, without altering MyNES presets.
Keep CTest logs rather than deleting a guessed LastTest.log.tmp filename; the
bounded runner still propagates failures and terminates its owned job tree.

Measured Make reconfiguration in this round takes 134.14 seconds, including
129.3 seconds generating its many Makefiles. A subsequent stable x64 Ninja
configuration takes 5.78 seconds (1.3 configure, 3.7 generate reported by CMake).
These are actual local configuration measurements, not a full-build speedup
claim. Compiler-cache stats already show 6,787 hits/12,988 cacheable calls;
the accepted Make caches had no launcher, so their object-only incremental
behavior did not establish persistent cross-directory compiler-cache reuse.

Two sandboxed Ninja ABI probes remained alive without compiler descendants.
Read-only parent/command inspection distinguishes them from the sibling MySMB
build. Only those two verified owned probe trees are stopped; identical x64
configuration outside the sandbox completes real ABI detection. Initial x86
ABI compilation then fails when invoking its assembler. Prepending the selected
MSYS2 compiler's own bin to this invocation's PATH permits genuine ABI detection
to complete; no pointer-size cache value or compiler-success bypass is injected.
Configuration failure and environment repair remain evidence, not passes.

The coherent x86 Ninja configuration completes in 10.58 seconds, including its
fresh genuine ABI probe; the strict registration gate also passes.

## Build Cache And Final Verification Progress

Both dependency-only cold builds finish: x64 473.73 seconds, x86 234.56 seconds,
each with 1,318 native graph steps. This is one-time cache population, not an
incremental-build measurement. Both no-op builds report no work: x64 1.37
seconds, x86 1.05 seconds. The CPU instruction objects exactly match accepted
S6: x64 528A50BB1FBADBFE0A15F8ECAA310F88042D44CADAABD1054C2234E14B2B2C65;
x86 58B4B98820F45085DF6CBA3303BDAB55701B209EB21A43381D53D34E1E5D140D.

For a controlled cache-recovery/CPU-dependency measurement, remove only each
validated build-local CPU instruction object and let the original graph restore
it. Do not edit source, flags or archive/link commands. Per-invocation ccache
logs prove one direct hit and zero misses on each width; restored objects match
the original hashes. Each unit-only dependency graph executes 378 steps:
x64 109.47 seconds, x86 52.57 seconds. This is not a claim that all 407 previous
product/integration consumers were rebuilt: only the actual unit build graph
is requested. Those no-longer-required build outputs are deferred to their
real execution stage, not removed from integration/product coverage.

All 505 actual unit commands exist and are unique after the build. A copied,
ignored registration fixture passes the positive guard; adding back one retired
alias produces the expected duplicate-route rejection. The exact 334 required
target identities still pass. Both route-partition/registration gates and the
bounded aggregate's real child-timeout/cleanup regression pass. All eight
unchanged manifests, Types and documentation checks pass. All ten PC/MyNES
EXE hashes still match accepted baseline, with no CPU or other runtime input
change; no new binary is needed for this build/test-only work.

Final x64 complete units pass 505/505 once in 250.11 seconds, within unchanged
300-second containment. Do not claim the single full execution became faster:
its current measured runtime is not lower than S6's historical 248.83 seconds.
The proven savings are configuration, cached dependency builds, removed duplicate
executions, and avoiding full runs during exploratory edits. Final x86 complete
units pass 505/505 once in 233.17 seconds, also within unchanged containment.

## Verification Stage Policy And Final Executor Review

The appropriate execution stages follow the existing rules, not a new fixed
focused suite:

1. Configure a persistent optimized cache only when its toolchain, flags or
   selected composition needs updating. Keep the selected compiler's runtime
   bin available; on Windows, do not mix x86 assembler DLL lookup with x64 PATH.
2. During development build actual changed targets and execute the current
   packet's transient focused selection. Use bounded, ignored task-local probes
   for diagnosis, not an additional permanent test path or a success claim.
3. At final S acceptance build-unit-tests establishes the complete dependency
   graph, then the existing aggregate executes all unit cases once per required
   width. A changed or failed final candidate needs scoped diagnosis and the
   necessary new verification, not a blind repeated whole-suite loop.
4. Execute external integration when guest/runtime inputs, fixture/checkpoint or
   effective runner behavior changes, and at T qualification. Unchanged binary,
   input and command identity permits retaining its accepted proof. Integration
   never substitutes for a code-owned unit; compilation never substitutes for
   execution. Applicable source/build/governance gates still run.

This S changes no runtime input, integration fixture/checkpoint or actual
qualified integration command: the new j1 default matches S6's explicitly j1
58-context qualification. The bounded invocation and deadline are unchanged;
only a guessed diagnostic-temp cleanup is removed. Its real timeout/tree
cleanup regression passes. Preserve the accepted S6 58/58 proof with exact
unchanged ten EXEs, four INIs and five media masters rather than spend another
full boot replay on identical inputs. This is an unchanged-input determination,
not an integration failure waiver or a smaller T-level corpus. Future CPU
changes still require their affected receiving proof and T's original matrix.

Affected build-ownership, artifact-root and integration-INI boundary gates pass;
registration/route and negative/timeout checks pass; all eight manifests, Types,
documentation and whitespace checks pass. No Lib/Common/MyNES production,
shared-corpus source/test body, owner input or runtime public API changes.
The retained 334 target identities and all canonical assertion bodies survive;
only 36 duplicate product registrations are retired. Keep distinct chip/Core/
board assertions and positive/negative source gates rather than call them
invalid merely because they take time. There is no second compiler/link list,
test dispatcher or resource owner.

Counted tracked build/tool paths are four files, git diff --numstat against
741d00417: +56/-20, net +36. The increase adds the symmetric cache preset,
dependency-only stage, separate integration scheduling and canonical-route
recognition; runtime/test-body code adds zero lines. MyNES preset content and
its exact executable pair remain unchanged. Deferred S8 patches/research and
warm caches are needed by the immediate successor; do not clean them away.

Executor review accepts the complete efficiency batch for target-separated P
delivery. Immutable-commit coordinator review and governance closure still
precede S7 acceptance. CPU semantic correctness is not claimed by this S;
S8-S20 retain the full original repair universe.

Shared P1 0c04f2c24 is pushed to origin/master, changing only root build presets.
NXVM's following delivery contains its existing build/test owner changes and
complete source/input/verification/reallocation evidence. No product executable
input changes in either target; verified S6 binaries remain the current delivery.
