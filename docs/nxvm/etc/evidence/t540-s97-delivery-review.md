# T540 S97 Delivery Review

Owner: NXVM. This record reconciles the complete pending extraction with
S94-S96, records final verification and explains the owner's build/test cost
request. Current remains the admission/acceptance authority. No partial P has
been delivered; this progress record is not T closure.

## Reviewed Input Reconciliation

Rehash all 626 logical paths in S94's frozen review inventory. The only changed
listed source/build path on admission is NxvmProduct.cmake, explained by S96's
deployment repair. All other listed live/deleted paths match. S95's two x86
manifest revision comments and S96's deployment/caller/gate identities are
separate recorded follow-ups, not silently rewritten S94 proof.

S94's opening completed-review section supersedes the chronological pending
notes retained later in that record and in its frozen TSV. In particular,
deleted protected timing/private CPU fixtures are reconciled by the complete
CPU/board receiver and fixture reviews; KBC's retained final-verification note
is discharged by S94's complete units/gates and S95-S96's independent/boot
proof. Those historical notes are not current unreviewed implementation rows.

S97 subsequently changes NXVM build registration and its existing INI
boundary gate, plus product documents. Final staging also removes one extra
EOF blank lines from the two scheduler test receivers and updates their
test-manifest hashes; this is byte normalization, not assertion/behavior change.
Runtime behavior, shared source/test assertions,
compiler flags and generated S96 product inputs remain unchanged. The complete source
review, 492/492 root units per width and four independent tools-on/off suites
remain valid for their execution inputs. Re-run the affected build/registration
and boundary checks rather than replay those unchanged runtime suites.

Final diff checking also removes one EOF blank line after Model40's last
function and two evidence-file trailing blank lines. There is no changed C
token, source line before that function end, compilation option or generated
product input; do not relink timestamp-bearing products merely for EOF cleanup.
The deployed S96 identities remain the executable qualification inputs.

## Build/Test Cost

S96 removes deployment's same-path INI write-back, which unnecessarily caused
another 59-80-second CMake generation after product linking. Its existing
artifact-roots gate prevents the write from returning. Retain the measured
strict-matrix batching (36.14 to 4.91 seconds) and manifest indexing (4.388 to
2.012 seconds); these are not new S97 benchmarks.

For remaining integration, use width-local Release Ninja trees, eight build
jobs and no MyNES configuration. Initial configuration/build of all 21 default
targets takes 36.8417788 seconds on x64 and 33.9126344 seconds on x86, including
210 build steps each. This is a cold integration-target observation, not an
equivalent comparison with an old complete Make build. Reuse these objects for
the two family configurations; those changes require only 13 and 11 steps.
No result cache, skipped dependency, weakened assertion or blanket timeout
change is introduced. Native suites run sequentially across trees.

## Complete External Integration

Default's actual registered non-boot suite passes 21/21 on x64 in 54.21 seconds
and 21/21 on x86 in 67.14 seconds. This includes DOS keyboard/video/media and
Windows 3.1 checkpoints. Commands use `ctest -L integration -R
'^integration.vm-' -E vm-profile-floppy-boot-matrix -j 1 --output-on-failure`.
The exact default target inventory is read from generated CTest registrations,
not the obsolete 19-target S93 helper. No MyNES case is selected.

The first AT x64 Console case fails before the monitor prompt; CMOS passes.
The registration launches a build-tree EXE next to a verbatim copied owner
INI. Its relative media path is valid only in the deployed profile directory,
so the copied path points to absent media. This is a registration/input defect,
not evidence of a guest hardware or CPU failure.

Consume the complete two-family Console class: delete the build-tree INI copy,
compute the existing product path at the sole artifact-name owner, and register
the tests against that deployed EXE and its adjacent unchanged owner INI.
Move the artifact declaration before registration so existing exact registration
counts are still checked. No duplicate product-name formula or new API exists.
The existing INI-boundary gate now rejects a copied build-tree INI and checks
the deployed-pair registration. Its current positive passes; an isolated copied
authority restoring the old configure_file fails with that exact diagnostic.

Corrected AT x64/x86 suites pass 2/2 in 0.70/0.54 seconds. Model40 x64/x86
suites pass 2/2 in 0.60/0.48 seconds. They inspect the real deployed Console
and compiled CMOS seed, retaining the original predicates. Each corrected row
executes once. S96 already proves the eight boot rows once each; do not replay
them. Together the current 50 non-boot cases and eight boot rows cover all
58 NXVM integration rows across the four profiles and two widths.

## Artifacts And Boundaries

All eight deployed EXE hashes still equal S96. All four owner INI hashes and
both MyNES artifact hashes remain unchanged. Git reports no MyNES, root README
or shared-rule change. Raw ROM/media originals stay external; tests retain
readonly/overlay access. The ignored Ninja trees/logs are retained for immediate
delivery audit; no executing build/probe is restarted on a observation timeout.

Both current specialized aggregates finish successfully, including exact T344
registration, 518 strict/deferred rows, fixture/owner boundaries and the new
Console registration predicate. All six actual manifest cases pass on x64
(1.99 seconds) and x86 (2.37 seconds). Negative-control failure output inside
the T345 self-test is expected and the enclosing control exits successfully;
it is not a failed production gate. Documentation governance passes.

The changed product build identities are NxvmProduct.cmake
`23C7449BA4CA3709F29F59467B5A864F3F4521FC5E14E5860630AD542723022D`
and verify_t533_integration_ini_boundary.cmake
`C52422946E3A766924B5921DB811E81C6E7A69449B1D521E185BC12C487F4FE9`.
The S96 deployment/gate identities are retained. Current is reduced from its
5,500-plus-line accumulated history to a compact active packet/baseline;
480,638 historical characters (LF normalized) remain in the existing receiver
work record with relative links rebased, not discarded or promoted to status.

Complete scoped delivery and actual pushed-tree audit remain pending. Current
and the T history must record those outcomes before acceptance; the tests above
alone cannot close T540.

## Final Change And Closure Audit

Count the complete staged tree against accepted S92 using Git numstat with
rename detection, including receiving files, excluding documents/artifacts and
restricting source/tests to C/header paths. Production: 57 paths, +1603/-818,
net +785. Tests: 374 paths, +17687/-13173, net +4514. Moves are counted by their
paired diff, not advertised as deleted functionality. The added test code is
the CPU/board assertion split, checked synthetic fixtures and setup-failure
coverage documented in S94; it is not another production implementation.
Build/tool groups separately: 43 CMake paths +904/-1038, net -134; four tools
+24/-31, net -7. These figures describe the complete pending S93-S97 delivery,
not merely the small performance repair.

The original finite ledger maps to one neutral x86/core owner, independent
chips, flat common/AT/XT public board receivers and Model40-owned D4. S94
reconciles all 626 logical original/receiving paths, 99 private readers and
146 migrated registrations; S95 proves independent linkage and exact optional
tool exclusions; S96 proves all eight sole-Core optimized products and boots;
S97 closes the remaining external integration and current registration gates.
Core/board observable test builds compile the same sources with observation
enabled; products select only their production variant, not a parallel engine.

App-split handoff: src/app-nxvm/app and product retain product shell/config/CLI;
machine retains Common binding, debug/media/host adaptation; profiles retain
four frozen compositions, with genuine Model40 D4; firmware retains project
firmware construction. Shared chip/Core/board tests stay in test/x86, while
product composition/firmware and external integration remain test/app-nxvm.
The queued App-split proposal owns the next four-product cutover. No hardware,
timing-grade upgrade or new guest qualification is claimed by this extraction.

Final pre-delivery review includes all newly staged paths, not only already
tracked paths: both specialized aggregates pass again; six manifests pass
6/6 per width in 1.48/1.87 seconds. Test/x86's EOF-normalized manifest hash is
`F8A836BFD79CCA6593B08D5222FEED6EF5E090E2FEE5992ECBC4B0DDA6CBAC86`.
All other S95 manifest identities remain exact. Documentation governance,
staged diff checks and all changed-document relative file links pass. The
eight product and two MyNES hashes still match S96; no INI is staged.

Coordinator-role actual-change review compares the complete staged path set
with S94's source/assertion dispositions and S95-S96 proof; reviews the final
deployment/test-registration/gate changes and current architecture/layout;
and reconciles the finite board/private-reader owners, optional linkage,
failure cleanup, external inputs, queue handoff and actual CMake target graph.
No unresolved member of the structural extraction scope remains. Existing
hardware/guest/native-test debt stays in its named Queue/TODO receiver; it is
not falsely claimed repaired by this source relocation.
