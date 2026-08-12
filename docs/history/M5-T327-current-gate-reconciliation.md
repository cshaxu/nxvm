# M5 T327: Current Gate Reconciliation Closure

## Scope And Result

T327 completes the Queue's refreshed current-test and specialized-gate
separation prerequisite. T324 had already established the two-root layout, but
the later audit found that the fast root still built media-classified targets
from the broad base list even though its CTest command excluded the `media`
label.

The fast root now depends on a list derived by subtracting the authoritative
media target set. The current-gate verifier reads generated Ninja and CTest
state to prove both developer roots exclude specialized verifiers, full and
fast target membership matches the CTest labels, media is a subset of current,
and the aggregate contains exactly the smoke and specialized roots.

## Verification And Acceptance

The generated selection at acceptance is 210 current tests, partitioned into
15 media and 195 non-media tests. The full smoke root, fast smoke root,
specialized verifier root, and `current-gates-gcc` aggregate all passed after
a fresh GCC configuration. The specialized verifier reports 48 mechanical
gates. Documentation governance and `git diff --check` passed.

No CTest registration, command, assertion, timeout, media classification,
runtime artifact, product source, or verifier semantic changed. The observed
elapsed baselines and complete graph evidence are retained in the indexed
[S1 evidence](../etc/evidence/t327-s1-current-gate-separation.md).

## Transfers

This closure changes no Intel instruction behavior. The remaining 80386DX
architecture-state packages continue in the Queue; future test admissions are
mechanically checked by the strengthened gate separation verifier.
