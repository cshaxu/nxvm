# M5 VM Request-Bridge Current-Build Restoration

## Purpose

Restore the configured full build by reconciling the dormant
`vm-request-bridge-smoke` with the current VM request-event contract.

## Boundary

The task changes only the stale smoke/support fixture and, if direct inspection
proves necessary, its target registration.  It must not restore removed
production request aliases, add a compatibility facade, alter Console request
semantics, or change current-gate membership without an explicit disposition.

## Approach And Acceptance

Freeze the current public VM request types and every request-bridge caller.
Make the smoke express that one current contract, prove copied payload and
lifecycle semantics through its registered target, then run the full configured
build and relevant current-gate control.  The task closes only when the full
build succeeds or a remaining unrelated failure is individually evidenced and
transferred; it may not hide the target.

## Stop Condition

Stop for a required product request-semantics decision or a second production
request authority; do not recreate removed fields merely to compile.
