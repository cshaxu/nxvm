# M5 T438: Core Reset And Firmware Failure Atomicity

T438 is active at S1. Its approved [candidate proposal](../proposals/m5-core-reset-firmware-failure-atomicity.md)
owns truthful VM firmware-reset failure propagation through the existing Core
reset boundary and one defined non-runnable failure state. Detailed active
requirements are in [CURRENT.md](../states/CURRENT.md).

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
