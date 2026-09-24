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
