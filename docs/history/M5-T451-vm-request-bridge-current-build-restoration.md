# M5 T451: VM Request-Bridge Current-Build Restoration

T451 is the owner-approved first repair candidate. It restores the configured
full build through the one VM request-bridge contract, without reviving an
obsolete production request interface or hiding its smoke target.

## Accepted Subtasks

- S1 freezes the request contract, caller ledger and configured-build baseline
  in [evidence](../etc/evidence/t451-s1-request-bridge-build-baseline.md). It
  proves the reported source defect is already absent and isolates the stale
  build instruction and debt record as the sole remaining in-scope repair.
- S2 retires that stale documentation/debt path and closes T451 without a
  source, CMake, ABI or compatibility-layer change.

## Constraints

The bridge remains the single request-queue owner. VM composition remains the
consumer boundary for host-originated requests. No compatibility alias, second
request authority, Console semantic change or concealed target is permitted.
