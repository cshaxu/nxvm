# M5 VM PC Profile Foundation

## Purpose

Normalize all built-in PC machine profiles and their single-parent inheritance
relationships on the VM side, using the committed VM Profile Resolution And
Core Machine-Plan Design. This is Project B's foundation: VM composes a frozen
Core plan; Core owns all timing mechanics and controller state.

## Required Scope

Implement the finite profile-field/dependency ledger and the immutable typed
profile/resolution path for the two roots `ibm-5160-xt` and `ibm-5170-at`, the
`generic-386dx-at` child, and the `compaq-deskpro-386-model-40` delta. The
resolver validates ownership, provenance, allowed patching, routes, windows,
media/firmware policy and available neutral Core contracts before publication.
It returns an immutable resolved profile and copied Core machine plan.

The committed supporting design defines the staged internal breakdown and
migration order. Every migration first proves parity with its existing
composition, then becomes the sole production path before the next one begins.
YAML remains a session selector with one built-in profile field, not profile
or hardware authoring.

## Dependencies

Consumes Core L3 contract export and the runnable 5170-derived DeskPro result.
It precedes the 5160-268 profile sequence and every later Windows candidate.

## Evidence And Completion Standard

Require a complete field/dependency ledger for all four in-scope built-ins,
resolver conflict/provenance/immutability tests, resolved-profile and
Core-plan snapshots, root and child composition regressions, migration parity
for 5170, 5160, generic 386DX AT and DeskPro, and proof that no profile name
or timing algorithm entered Core.

## Non-goals And Stop Conditions

Do not admit 80486/P5, arbitrary YAML topology, multiple inheritance,
variants, a compatibility-version layer, L4, external assets, or a new
machine family. Stop a field that lacks an available Core contract or changes
a frozen composition without a named predecessor receiver.
