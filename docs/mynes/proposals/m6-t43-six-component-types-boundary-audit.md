# M6 T43 Six-Component Types Boundary Audit

## S10 Shared Registration And Byte Boolean

Owner accepts S9 and admits this bounded Shared change, with NXVM and MyNES
receiving deliveries. Move the one registration helper to `test/register.cmake`;
all three standalone suites retain separate selection, targets and execution.
Document this common transfer input instead of retaining a forwarding copy.
Change the Types owner to `typedef lib_u8 lib_bool`, preserving canonical
LIB_TRUE/LIB_FALSE behavior. Audit callbacks, native return normalization,
copied layouts and persistence; native ABI, atomic and wire types stay unchanged.
Update exact layout assertions and six manifests. No product policy change,
new API or extra framework is authorized.

Exit: actual-diff review; independent Lib/Common/x86 suites; complete configured
MyNES and NXVM unit suites on x64/x86; rebuilt and verified current two MyNES
and eight NXVM stripped EXEs; target-separated commits/push. Preserve INIs and
owner title changes. Record findings, counts, hashes and transfer requirements,
then await manual acceptance with T43 open.

## S9 Snapshot Resume Correction

Owner admits this receiving-product defect after accepting S8. Reproduce the
Running-but-frozen condition through the real snapshot/lifecycle path before
repair. MyNES owns guest snapshot state and its driver execution latches;
Common remains the single executor/lifecycle owner. Inspect save/resume and
stop/load/resume, repeated restores, reset/media restart and pacing/input/frame
state. Fix the owning mechanism without a second run flag, thread or snapshot
format. Shared edits require owner review before implementation.

Exit proof requires actual CPU/frame/controller progress after resume, bounded
pause/stop, failed-load state preservation, both full configured MyNES suites
and rebuilt stripped 0043 x64/x86 artifacts. Commit/push the reviewed MyNES
delivery and await owner gameplay verification; do not close T43.

## S7 Shared Quality Repairs

Owner approves the post-S6 audit repair batch: preserve assembler tables and
existing label loops, track successful materialization in one temporary flag,
and reject unresolved references before output. Audio preserves every accepted
prefix and consumes clear cancellation before acknowledgement; tests synchronize
at actual FIFO completion, not a platform callback midpoint. Common request
admission must propagate event failures without leaving an accepted request.
The owner also approves completion-failure termination through the existing
task cancellation and join, with admission/claim serialized by the request lock.
Dispatch reset failures must terminate rather than spin. Concurrent loss of
both native notification paths or failed join is not claimed recoverable.
Correct the README audio dependency and duplicate test include. No public API
expansion or polling workaround is admitted. Verify all six corpora and both
receiving Apps on both widths; preserve configuration and keep T43 open.

## Goal

S8 extends the owner's quality review to boolean vocabulary, unused references,
comment accuracy and owner-local flattening/duplication across the six roots.
Freeze a complete file inventory and classify findings before implementation.
Native/serialized ABI, atomics, counters, bitsets and deliberately invalid test
fixtures are not automatic boolean violations. Preserve original xasm tables.
Report concrete proposed changes for individual owner discussion; no source
repair is authorized by a regex hit. Approved repairs later require all affected
receiving products, dual-width tests/artifacts and refreshed manifests.

Owner subsequently approves all reported S8 repairs, including the copied-value
boolean layout changes, local flattening and shared test-registration cleanup.
Review and deliver the owner's additional MyNES composition/native-test and
NXVM.ini changes with target-separated commits. Preserve T43 open after S8.

Owner resolves the final INI finding: all INI boolean values accept only 0/1;
true/false are rejected, not aliases. Update the existing NXVM parser, four
supplied configurations, repository-only regression and UX contract. MyNES has
no boolean INI key and requires no parser change. Rebuild eight NXVM receivers,
run complete dual-width units and retain the already verified MyNES artifacts.

Prove that the Shared six-component corpus has one external-vocabulary owner:
`src/lib/types/`. The audit covers `src/{lib,common,x86}` and
`test/{lib,common,x86}` equally.

## Boundary Model

This does not forbid each component's own public API. A call to `lib_base_*`,
`common_*`, or `x86_*` is component-owned, not an external vocabulary leak.
The audited boundary is every ISO C, compiler, Win32 or Linux type, constant,
function, macro and declaration that would otherwise bypass Types. Each ledger
row has exactly one disposition:

1. Types-owned: declared by `src/lib/types/` and used through that contract.
2. Component-owned: defined by the same Shared component and not external.
3. Test-harness boundary: a deliberately native probe that is isolated and
   named by the test policy.
4. Violation: direct external vocabulary, with file/line and a proposed owner
   receiver; no repair is made until owner review.

## S1 Completion Standard

S1 freezes the file inventory, produces the exhaustive line-level ledger,
reviews every non-Types row, runs the six-root gates and full MyNES x64/x86
unit suites, and delivers the versioned 0043 x64/x86 pair. It changes no Shared
source/test code. Any violation becomes the finite input for a later,
owner-approved repair S; T43 cannot close while such a receiver remains open.

## S3 Corrective Batch

The owner admitted the post-S2 residual batch: ten Win32 pointer spellings in
four Lib tests, plus the verifier's incomplete rejection set. Use existing
Types records with explicit pointers, preserving const and callback signatures.
Prove every reported spelling is rejected by an isolated negative test and
that valid Types pointers pass. Normalize relative scan roots and reject empty
inputs so a zero-file scan cannot claim compliance. Update the Lib test
manifest, run both complete configured architectures and refresh the 0043 pair.
The pointer batch needs no new aliases. The same-class sweep also covers raw
scalar/constants/functions: add only their missing one-to-one Types vocabulary,
preserve behavior, and include each newly found spelling in the rejection tests.

## S4 Unchanged-Import Audit

Compare all six roots against a pinned, clean SoftPC revision. Review every
changed file, retained Types fixes, manifests, public layout changes and both
MyNES/NXVM consumers. Record a complete difference ledger and whether unchanged
Shared source is eligible for import, including any product adaptation needed.
This audit changes only MyNES task records; importing or repairing the candidate
is a subsequent admitted action. Audit-only work produces no new executable.

Owner follow-up on 2026-09-25 admits that import within the same S4, overriding
the usual next-S allocation. Continue from P3 with the pinned six-root bytes,
code-quality/manifest audit and full Shared dual-width testing; rebuild and
regress MyNES and verify NXVM's receiving path. Refresh the MyNES 0043 pair.
Preserve the earlier audit as history. T43 remains open after S4 delivery.

After reviewing the native-audio failure, the owner directs commit/push of the
unchanged import, evidence and rebuilt pair without adding audio repair. Keep
the failure visible; delivery is not a claim of all-green qualification or T
closure.

## S5 Native Audio Correction

Owner admits repair of S4's native-audio failure. Diagnose the render and
loopback paths before changing semantics; correct the responsible shared owner
or test and cover repeated delivery. Verify exact submitted PCM bytes; do not
alter host settings. Verify x64/x86 Shared/MyNES suites and NXVM units, update
manifests, rebuild the 0043 pair and deliver target-separated commits. T43 stays
open. A genuine external host prerequisite must be reported, not hidden.

Owner clarification: unit acceptance must not depend on host mute or physical
signals. Exercise the production WASAPI adapter with deterministic OS-boundary
doubles; remove real loopback entirely, without an opt-in switch, as the owner
further directs. Do not modify host volume or production audio merely to make
a loopback assertion pass.

## S6 Receiving Artifact Completion

Owner requests executable-delivery governance and the missing NXVM receiving
artifacts. Test-only changes need no binary refresh; however S4 changed production
console/frame/UI inputs after the deployed NXVM baseline. Verify/rebuild the
four current NXVM profiles on both widths and verify the existing MyNES pair.
Keep live 0535/0043 identities, INI contents and external BYOB assets unchanged.
Shared rules, NXVM artifacts/evidence and MyNES task records use separate P
commits. Run full unit suites and documentation gates; T43 stays open.
