# M6 T43 Six-Component Types Boundary Audit

## Goal

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
