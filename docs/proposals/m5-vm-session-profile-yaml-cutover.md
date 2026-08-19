# M5 VM Session Profile YAML Cutover

## Purpose

Replace hard-coded profile selection with the custom `nxvm-session` grammar
after all three current profiles use the VM resolver. YAML selects an approved
built-in profile and its permitted session choices; it never authors a machine.
This candidate applies the session boundary in
[VM Profile Resolution And Core Machine-Plan Design](../etc/architecture/vm-profile-resolution-design.md).

## Required Scope

Accept exactly one built-in `profile` selector and profile-declared session
choices such as permitted memory, boot order, BYOB manifest, and session media.
Validate YAML before resolution; create an immutable session request; resolve
the selected built-in profile and build its frozen Core plan.

Remove the legacy profile enum/parser and profile-specific selection branches.
Add positive and negative parser coverage plus startup regressions for
`pc-at-5170`, `default-at`, and DeskPro.

## Dependencies And Completion

Consumes the accepted VM AT profile-resolution foundation. Completion requires
the `nxvm-session` parser, rejection of undeclared choices, one production
selection route, all-profile regressions, current gates, and documentation
governance.

## Non-goals And Stop Conditions

There is no v1/v2 compatibility layer, variant, parent/topology/CPU/port/IRQ/
DRQ/ROM/timing/chip-graph YAML field, user profile authoring, XT admission,
asset import, or L4 work. Stop if a required session choice lacks a
profile-declared policy or requires a second resolver/composition path.
