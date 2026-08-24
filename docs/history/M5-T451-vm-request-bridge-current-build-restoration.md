# M5 T451: VM Request-Bridge Current-Build Restoration

T451 is the owner-approved first repair candidate. It restores the configured
full build through the one VM request-bridge contract, without reviving an
obsolete production request interface or hiding its smoke target.

## Planned Subtasks

- S1 freezes the request contract, caller ledger and build-failure baseline.
- S2 makes any required repair only in the owner-local smoke fixture or its
  registration, retaining copied-payload and lifecycle coverage.
- S3 verifies the complete configured build, relevant target control and
  finite obsolete-contract sweep, then closes or explicitly transfers an
  independently proven unrelated failure.

## Constraints

The bridge remains the single request-queue owner. VM composition remains the
consumer boundary for host-originated requests. No compatibility alias, second
request authority, Console semantic change or concealed target is permitted.
