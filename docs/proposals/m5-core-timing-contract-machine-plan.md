# M5 Core Timing Contract And Machine Plan

## Purpose

Introduce the single Core-owned, immutable timing-plan boundary required to
apply registered L3 rules and explicit L2 fallbacks without machine-name logic
in Core. It converts the reconciled ledger into validated configuration, while
preserving current behavior through an explicit default plan.

## Shared Admission Baseline

This candidate implements the plan boundary in the
[Core specification-driven timing design](../etc/architecture/specification-driven-l3-timing.md)
and consumes the earliest T433 S6 receiver batch:
`TIME-CLOCK`, `TIME-LIFECYCLE`, `CPU-RETIRE`, `MEM-ROM-FIRMWARE`, and
`MACHINE-CONFIG` from the
[Core L3 admission ledger](../etc/evidence/t433-s6-core-l3-admission-feasibility-ledger.md).
The [T433 S7 source-sufficiency ledger](../etc/evidence/t433-s7-core-source-sufficiency-ledger.md)
is a mandatory admission qualifier: the plan may represent and reject partial
or absent rules, but may not manufacture a profile timing value or Core rule.
It creates registration, validation, copied lifetime and explicit L2-exception
support only; later candidates implement their assigned timing rules through
this boundary and may not redefine the shared L3 or Core/VM contract.

## Required Scope

Define stable neutral capability IDs and a copied `core_machine_plan` (or
approved equivalent) containing clock ratios/reset phase, instruction timing
program selection, transaction classification/waits/arbitration, topology and
routes, lifecycle ownership, and trace declarations. Core validates complete
range, type and topology consistency before publication and holds copied data
thereafter.

Existing `core_machine_config` material must converge toward this one path;
there must not be any second production composition or publication route,
including a temporary compatibility route. Existing configuration structures
may remain only as private input material to construct the plan; they may not
independently publish a machine. The default plan preserves current runnable
behavior through explicit, named dispositions until later candidates replace
individual L2 rules with L3 rules.

## Mandatory Boundary Completion

This task completes the shared Core timing boundary itself. It is not a
plan-only shell followed by a later audit or a delegation of consumer wiring to
the CPU, transaction, or device candidates. Before this task can close, the
single copied plan has a usable, validated consumer seam for every current
Core timing domain:

- CPU form/program selection and successful-retirement observation;
- memory and port transaction classification, availability, BUSRDY and
  arbitration declarations;
- clock domains, reset phase and deterministic event lifecycle;
- RAM/A20/parity and ROM/firmware mapping declarations;
- controller/device topology, port/memory decode, IRQ/DRQ/HOLD routes and
  phase-rule registration; and
- reset, cancellation, trace and copied observation declarations.

These seams do not pre-implement a CPU opcode formula, a board wait scalar or
a controller phase. Later candidates implement those sourced rules *through*
the registered seams. They may neither create a parallel Core-to-device timing
path nor redefine plan ownership, validation, publication or fallback policy.

### Default Plan, L2 And Rejection Policy

Every currently public capability selected by a runnable default machine must
have exactly one disposition in its validated default plan:

1. a registered L3 rule; or
2. a named `l2-fallback` record identifying the current deterministic
   behavior, Core owner, missing L3 fact, evidence tier, regression and earliest
   receiver; or
3. a non-guest-time declaration with its isolation/lifetime proof.

An L2 fallback is not an omitted field or a guessed timing constant. It retains
the current deterministic functional/proportional behavior, including its
existing virtual-clock, retirement, transaction, device-state and lifecycle
contract, while making no L3 claim. The default plan must therefore preserve
existing runnable machines rather than rejecting them because a future L3 rule
has not yet been admitted.

Default-plan preservation is a behavioral-equivalence requirement, not a boot
smoke. Its evidence compares the pre-plan baseline and the validated default
plan for deterministic event order, reset and cancellation, IRQ/DRQ assertion
and withdrawal, trace/observation publication, and every construction or
validation failure rollback. A difference requires an explicit approved rule
change; it cannot be called migration equivalence or moved to a later task.

Machine construction rejects atomically only when the plan is missing or
invalid (range, type, topology, lifecycle or copied-lifetime failure), or when
a profile explicitly requires an unavailable L3 contract. A profile may select
an approved L2 fallback; it may not silently substitute one when it requested
L3. No invalid/partial plan may publish a machine or fail later during guest
execution.

## Dependencies

Consumes the master-ledger and conformance dispositions. It precedes CPU
instruction timing, CPU-to-board transaction contracts, controller/device
phase contracts, and Core-to-VM contract export.

## Evidence And Completion Standard

Require validation/rejection tests, immutability/lifetime tests, default-plan
behavior regressions, explicit L2-disposition tests, and a ledger mapping every
selected plan field and current Core consumer seam to one neutral Core owner.
Demonstrate that Core contains no profile or machine name, VM is not required
at runtime to drive a controller state machine, and no legacy configuration
route can independently create or publish a machine. Test that a valid default
plan remains runnable; invalid/missing plans and unavailable required-L3
profiles fail atomically before publication; and an approved L2 selection
remains runnable and visibly labelled.

Task closure requires the complete consumer-seam inventory above, one
production plan path, and a disposition for every current public capability.
It is not sufficient to publish an interface, perform a later integration
audit, or transfer unconnected consumers to a downstream task.

T434 is ineligible to close if any of these conditions remains true:

1. a current public capability lacks exactly one L3, named L2, non-guest-time
   or construction-rejection disposition;
2. any production machine publication route bypasses the validated copied plan;
   or
3. a required plan meaning conflicts with the established Core/VM ownership
   boundary and has not received a separately approved architecture decision.

These are task-local closure blockers. They may not be reclassified as a final
audit observation or transferred to a CPU, transaction, controller/device or
integration candidate.

## Non-goals And Stop Conditions

Do not implement VM inheritance/YAML resolution, migrate every timing rule in
one change, add a profile-local callback into Core, or model L4. Stop a field
whose required capability has not been reconciled by the first candidate.

`M5:Td:S115:TIMING-PLAN-CLOSURE:OK`
`M5:Td:S116:TIMING-PLAN-BLOCKERS:OK`
