# M5 PC/AT 5170 Root Profile Normalization

## Purpose

Make the current runnable IBM 5170 Model 339 composition the sole AT root profile,
named `pc-at-5170`, without changing its selected behavior. This is the first
implementation candidate in the VM profile program described by
[VM Profile Resolution And Core Machine-Plan Design](../etc/architecture/vm-profile-resolution-design.md).

## Required Scope

Freeze a field/dependency ledger for the three current VM profiles:
`default-at`, `5170-339`, and `compaq-deskpro-386-model-40`. The ledger
distinguishes the future AT-root field, each child delta, Core contract ID,
provenance, and L2 fallback.

Express the current selected Model 339 through an immutable `pc-at-5170` root
description and its copied Core machine plan. Prove equivalent reset, ROM,
address/port mapping, route, device, firmware-policy, and startup behavior
against the frozen current path.

## Dependencies And Completion

Consumes the accepted Core L3 integration and VM contract export. It precedes
every child-profile and YAML migration. Completion requires the finite ledger,
root/resolved-plan snapshots, focused parity regressions, current gates, and
one removed source of duplicate 5170 configuration.

## Non-goals And Stop Conditions

Do not add a child profile, general resolver, YAML change, DeskPro behavior,
XT profile, CPU/device/timing change, firmware/media asset, or L4 work. Stop
if a required root field has no exported Core contract or preserving Model 339
behavior requires a machine-named Core branch.
