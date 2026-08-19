# M5 Core Timing Contract And Machine Plan

## Purpose

Introduce the single Core-owned, immutable timing-plan boundary required to
apply registered L3 rules and explicit L2 fallbacks without machine-name logic
in Core. It converts the reconciled ledger into validated configuration, while
preserving current behavior through an explicit default plan.

## Required Scope

Define stable neutral capability IDs and a copied `core_machine_plan` (or
approved equivalent) containing clock ratios/reset phase, instruction timing
program selection, transaction classification/waits/arbitration, topology and
routes, lifecycle ownership, and trace declarations. Core validates complete
range, type and topology consistency before publication and holds copied data
thereafter.

Existing `core_machine_config` material must converge toward this one path;
there must not be a parallel long-term composition route. The default plan
reproduces current behavior until later candidates migrate each rule.

## Dependencies

Consumes the master-ledger and conformance dispositions. It precedes CPU
instruction timing, CPU-to-board transaction contracts, controller/device
phase contracts, and Core-to-VM contract export.

## Evidence And Completion Standard

Require validation/rejection tests, immutability/lifetime tests, default-plan
behavior regressions, and a ledger mapping each selected plan field to one
neutral Core owner. Demonstrate that Core contains no profile or machine name,
and VM is not required at runtime to drive a controller state machine.

## Non-goals And Stop Conditions

Do not implement VM inheritance/YAML resolution, migrate every timing rule in
one change, add a profile-local callback into Core, or model L4. Stop a field
whose required capability has not been reconciled by the first candidate.
