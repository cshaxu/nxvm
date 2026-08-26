# M5 VM Session Profile YAML Cutover

## Purpose

Replace hard-coded profile selection with one custom `nxvm-session` grammar
after all three current profiles use the VM resolver. YAML selects an approved
built-in profile and its permitted session choices; it never authors a machine.
The root `schema` and root `profile` fields are the sole grammar identity and
selection fields; the parser must not silently accept the currently divergent
`nxvm-session/v1` spelling or a nested `machine.profile` alternative. This
candidate applies the session boundary in
[VM Profile Resolution And Core Machine-Plan Design](../etc/architecture/vm-profile-resolution-design.md).

## Required Scope

Accept exactly one root built-in `profile` selector and profile-declared
session choices such as permitted memory, boot order, BYOB manifest, session
media, and display policy. Reject CPU/FPU/memory hardware-topology overrides
unless the resolved built-in profile explicitly declares that choice as session
policy; `--display` must become validated immutable session configuration or
be rejected, never parsed and discarded. Validate YAML before resolution;
create an immutable session request; resolve the selected built-in profile and
build its frozen Core plan.

Remove the legacy profile enum/parser and profile-specific selection branches.
Add positive and negative parser coverage for root-only identity/selection,
undeclared choice rejection, and retained choice propagation, plus startup
regressions for `pc-at-5170`, `default-at`, and DeskPro.

## Dependencies And Completion

Consumes the accepted `default-at` 5170-derived profile. Completion requires
the `nxvm-session` parser, rejection of undeclared choices, one production
selection route, all-profile regressions, current gates, and documentation
governance.

## Non-goals And Stop Conditions

There is no v1/v2 compatibility layer, variant, parent/topology/CPU/port/IRQ/
DRQ/ROM/timing/chip-graph YAML field, user profile authoring, XT admission,
asset import, or L4 work. Stop if a required session choice lacks a
profile-declared policy or requires a second resolver/composition path.
