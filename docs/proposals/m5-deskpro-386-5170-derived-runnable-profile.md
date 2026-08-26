# M5 DeskPro 386 5170-Derived Runnable Profile

## Purpose

Make the existing Compaq DeskPro 386 Model 40 a runnable explicit child of
`pc-at-5170`. This is the production vertical slice of the VM profile design,
not an interim direct derivation and not a fresh DeskPro physical/L3 program.
See [VM Profile Resolution And Core Machine-Plan Design](../etc/architecture/vm-profile-resolution-design.md).

## Required Scope

Use the accepted reusable resolver kernel: typed parent patch to immutable
`vm_resolved_profile`, then copied Core machine plan. Register only the
DeskPro child against the existing AT root; do not add a DeskPro branch to the
kernel.

The DeskPro delta declares only 80386DX, memory/ROM map, Compaq controllers and
display, firmware policy, and board-route differences already in source or
accepted evidence. Every remaining field is an explicit inheritance, L2
fallback, or transferred receiver. Compare old and new reset, ROM, routes,
devices, selected timing contracts, and startup behavior; then remove the old
Model-40-special production composition path.

## Dependencies And Completion

Consumes the accepted Core boundary, resolver kernel and `pc-at-5170` root
candidate. It precedes the `default-at` child and YAML candidates. Completion
requires the inherited-versus-delta ledger, immutable configuration proof,
parity/current regressions, and no DeskPro-only Core mechanism.

## Non-goals And Stop Conditions

Do not claim DeskPro physical or complete L3, reopen device/firmware research,
add assets, create an interim inheritance route, migrate `default-at`, or
implement YAML. Stop if an inherited fact changes frozen behavior, a Core
contract is unavailable, or a new difference lacks evidence.
