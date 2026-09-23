# Core Memory-Device Route Registry Scalability

## Purpose

Replace Core's fixed memory-device-provider slot assumption with a bounded,
growable registration mechanism. This removes an artificial composition
limit when a real machine needs multiple immutable-ROM windows, aliases, RAM
replacement ranges, video apertures, or other independently owned routes.

## Required Scope

Core remains the sole owner of route registration, validation, lookup order,
overlay behavior, freezing, and rollback. The implementation must:

- admit additional providers only while machine configuration is open;
- grow storage under an explicit finite limit and report deterministic failure
  before partial publication when that limit or allocation fails;
- preserve the current registration order and the provider-decline/overlay
  semantics exactly; and
- keep mapping storage private to Core, without exposing a mutable registry or
  a new public profile-facing container.

Add owned Core regressions for growth beyond the present fixed capacity,
failure-atomic rollback, frozen-registration rejection, and route priority.
Re-run representative multi-route Model-40 composition plus non-Model-40
profiles to prove that this mechanism change does not select or alter a
profile's address decode.

## Non-goals And Stop Conditions

This proposal does not add any machine ROM, alias, device, timing, firmware,
media, or board contract. It does not import third-party source or firmware,
and it does not change public ABI merely to expose Core internals.

Stop and transfer if preserving the current route order requires a new public
contract, an unbounded allocation policy, or a change to existing route
conflict semantics. A profile-specific mapping defect remains with that
profile's own task.

## Evidence And Completion Standard

Require an owner-local capacity/growth ledger, direct Core regressions for the
four mechanism invariants, caller and similar-issue sweeps for every fixed
route-capacity user, focused profile regressions, full current-gate coverage,
and a statement of the retained finite safety limit. Completion records any
profile still constrained by a separate semantic contract; capacity alone is
not a hardware-completeness or L3 claim.