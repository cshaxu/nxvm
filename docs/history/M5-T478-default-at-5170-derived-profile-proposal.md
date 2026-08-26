# M5 Default-AT 5170-Derived Profile

## Purpose

Migrate `default-at` as the second explicit `pc-at-5170` child using the
already accepted VM profile resolver kernel. This is the final AT-tree profile
connection before YAML selection in
[VM Profile Resolution And Core Machine-Plan Design](../etc/architecture/vm-profile-resolution-design.md).

## Required Scope

Register `default-at` through the existing profile registry, one-parent typed
patch rules, field-owner/provenance records, immutable resolved-profile
snapshot and copied Core-plan route. Migrate it from current direct composition
to an evidence-led explicit delta; do not assume it is identical to Model 339.

All three current profiles must use the same production route:
`pc-at-5170` root, child delta where applicable, resolved profile, and frozen
Core plan. Reject route/window conflicts, unavailable contracts,
disabled-but-bound devices, invalid firmware/media policy, and post-freeze
mutation.

## Dependencies And Completion

Consumes the accepted Core boundary, resolver kernel, `pc-at-5170` root and
DeskPro vertical slice. It precedes the session-YAML cutover and the XT profile
program. Completion requires root/child snapshots, parity regressions for all
three current profiles, current gates, and removal of superseded composition
branches.

## Non-goals And Stop Conditions

Do not alter the resolver kernel, admit an XT profile, a generic 386
intermediate profile, 80486/P5, multiple inheritance, variants, user hardware
graphs, YAML grammar changes, assets, timing algorithms, or L4 work. Stop if a
requested field lacks a neutral Core contract or cannot preserve its frozen
profile behavior.
