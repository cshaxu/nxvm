# M5 T438: Core Reset And Firmware Failure Atomicity

T438 closed at S1. Its retained [candidate proposal](M5-T438-core-reset-firmware-failure-atomicity-proposal.md)
owned truthful VM firmware-reset failure propagation through the existing Core
reset boundary and one defined non-runnable failure state.

The task applies the owner's hard minimalism constraint: the existing Core
firmware context is the sole failure owner during reset; no VM-specific wrapper,
parallel reset route, or generic rollback framework is admitted. Configuration
flows down to the frozen Core machine and failure status returns upward through
the existing callback/result boundary.

## S1 Implementation

P1 retains the private Core firmware context as the only reset failure owner.
Its existing Core firmware-operation methods record their first failed status
during reset, and the existing reset invocation returns it if the callback
discarded that status. A failed reset returns to `CORE_MACHINE_INITIALIZED`;
it cannot run, and an ensuing repaired reset is valid. The focused proof,
artifact identity, similar-issue sweep and aggregate-gate T344 baseline
disposition are retained in
[T438 S1 evidence](../etc/evidence/t438-s1-core-reset-firmware-failure-atomicity.md).

## Coordinator Acceptance

The single-session coordinator reviewed pushed P1 `48ed865c` against the
approved packet and the actual nine-file implementation diff. The Core-private
context is the only added failure state, each existing firmware operation uses
the same first-failure path, and no VM wrapper, public ABI, second reset route
or generic rollback layer appears. The focused failure/retry smoke and
documentation-governance gate pass. The aggregate current gate is not accepted
as passing because its independent T344 historical-fixture assertion fails at
75 actual versus 71 expected constructors; P1 changes no constructor and the
evidence retains that bounded baseline transfer. The 0438 developer artifact
and its recorded hash are present. S1 and T438 are accepted and closed.
