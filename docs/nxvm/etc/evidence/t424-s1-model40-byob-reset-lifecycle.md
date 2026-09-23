# T424 S1: Model-40 BYOB Reset Lifecycle

`M5:T424:S1:MODEL40-BYOB-RESET-LIFECYCLE:OK`

## Evidence Tier And Scope

Tier: `original` for the lawful user-supplied ROM interface boundary, not for
vendor firmware behavior. This S consumes the existing T386 S20 two-slot
manifest, exact-size SHA-256 validation, session-owned copy, immutable mapping
and aliases. It adds the missing executable boundary proof: a validated
project-owned synthetic pair is visible at the 80386 reset vector, one reset
instruction retires through the existing Core execution owner, and a cold reset
restores the same VM-owned reset observation.

`vm_session_get_reset_vector` now declares `type_status`, matching its existing
status-return implementation. It returns only the copied CS:IP observation; it
does not expose ROM bytes, paths, digests, Core storage or a firmware handle.

## Lifecycle Contract

1. VM accepts the transient owner-supplied manifest and validates both 16 KiB
   chips before session publication.
2. The Model-40 profile owns copied chip lifetime and binds the existing Core
   immutable-ROM provider and reset callback.
3. The existing VM observation reports `F000:FFF0` after construction.
4. A one-instruction bounded run reaches the mapped reset instruction through
   the sole Core execution path. A cold reset then returns the same observation.
5. A digest mismatch still publishes no session, and the test deletes its
   project-owned synthetic files.

The generic PC/AT profile uses its own profile-firmware provider. The Model-40
path remains the only BYOB manifest consumer; no alternate mapping or execution
route is introduced.

## Verification

- `current.vm-model40-byob-s20-smoke` passes and emits the retained T386
  markers plus the T424 lifecycle marker.
- The retained BYOB retirement-capture smoke is rerun with the serial current
  gate; it remains the separate normalized firmware-corpus observer.
- Documentation governance and the serial current gate are required at closure.

## Transfer

This proves a lawful, reset-visible execution lifecycle with synthetic chips. It
does not validate a vendor ROM's POST effects, device-service behavior,
cancellation behavior, boot checkpoint or timing. Those require a separately
owner-supplied external corpus under T390 S2 containment and remain a
non-physical, non-L3 receiver.
