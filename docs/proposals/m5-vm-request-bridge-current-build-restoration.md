# M5 VM Request-Bridge Current-Build Restoration

## Purpose

Reconcile the historical request-bridge build-debt report with the current VM
request contract and the one supported CMake build entry. Retire the debt when
the current source and configured complete build prove that no repair remains.

## Boundary

The task changes only obsolete task/build documentation and the stale debt
record, unless direct inspection proves that the registered smoke or its
registration is in fact stale. It must not restore removed production request
aliases, add a compatibility facade, alter Console request semantics, or change
current-gate membership.

## Approach And Acceptance

Freeze the current public VM request types and every request-bridge caller.
Prove the registered smoke, its copied-payload/lifecycle semantics and the
configured complete build. If they already agree, delete the obsolete debt and
replace the stale build instruction with the one declared preset route. The
task closes only when the full build succeeds or a remaining unrelated failure
is individually evidenced and transferred; it may not hide the target.

## Planned Subtasks

1. **S1 - Contract and build baseline.** Freeze the present request data
   contract, producers and consumer boundary; prove the registered bridge and
   transport smokes, `current-gcc`, and the configured `all` build. The actual
   baseline already finds no obsolete request field in source and no build
   failure; its evidence determines the documentation-only S2 boundary.
2. **S2 - Stale-debt retirement and build-entry convergence.** Delete the
   obsolete TODO and this consumed Queue candidate, move this proposal to task
   history at closure, and correct the stale supporting toolchain command to
   the current CMake preset route. No source, CMake target, request contract or
   compatibility layer is added.

## Stop Condition

Stop for a required product request-semantics decision, a second production
request authority, or a source-level failure that contradicts S1; do not
recreate removed fields merely to compile.
