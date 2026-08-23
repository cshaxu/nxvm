# M5 Core Reset And Firmware Failure Atomicity

## Purpose

Make Core reset report firmware/reset failures truthfully and leave the
machine at a defined, non-runnable state on failure. This is a direct repair
of owner-local lifecycle semantics, not a machine-profile or timing change.

## Required Scope

Trace every Core firmware reset callback and cold-reset mutation through
`core_machine_reset`. Preserve the first meaningful failure status, prevent
post-failure success publication, and either restore the pre-reset valid state
or establish the documented stopped/faulted state before return. Cover all
current firmware-capability variants and reset callers.

## Dependencies And Completion

This is the first repair candidate. Completion requires a finite callback and
mutation ledger, focused firmware-reset failure and retry regressions, a
similar-issue sweep of Core reset producers/callers, and current gates.

## Non-goals And Stop Conditions

Do not change firmware bytes, reset-vector behavior on success, profile
selection, timing, or introduce a generic rollback framework. Stop if an
owner-visible reset state needs a new architecture decision; transfer that
decision rather than inventing one locally.
