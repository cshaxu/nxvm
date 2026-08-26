# M5 Core Unified Guest-Time Axis And VM Boundary

## Purpose

Establish the machine-neutral Core capability that all later profiles consume:
one Core-owned guest-time axis integrates CPU retirement, bus transactions and
eligible controller deadlines, while VM receives only copied observations and
may limit completed progress in Standard mode. Turbo removes that host wait,
not any Core time semantics.

## Scope And Ownership

Core owns elapsed guest time, event order, deadline composition, CPU and bus
cost application, reset/cancellation effects, and every mutable device state.
It accepts one immutable, neutral timing plan whose values carry Manual/Other
L3 or explicit L2 provenance. VM and profiles may construct that plan and read
a copied observation; neither can advance time, query device internals, create
a second event queue, or name a machine inside Core.

The task freezes one convergence ledger covering the generic CPU-retirement,
transaction, controller-deadline and Core-to-VM observation routes. Every row
ends as an implemented source-backed rule, a validated L2 input, explicitly
ineligible/unsupported with a named receiver, or a stop condition. Exact
integer conversion does not upgrade an L2 premise to L3.

## Dependency And Consumers

This task is the prerequisite for every machine timing claim. It does not
select IBM 5170, PC/AT, DeskPro, XT, ROM, firmware, media, profile inheritance
or YAML policy. The queued [PC/AT 5170 root profile normalization](m5-pc-at-5170-root-profile-normalization.md)
is its first intended consumer: it supplies selected IBM Model-339 source facts
and retained L2 inputs through the validated plan, without creating any Core
timing mechanism.

The closed T469/T470/T471/T472 contracts are baseline material only. Their
existing copied observation, qualification and pacing behavior must be reused
or simplified; this task must not introduce a parallel Core/VM contract.

## Planned Batches

1. Freeze the machine-neutral convergence ledger and reconcile existing time,
   transaction, deadline and VM observation paths.
2. Consolidate the immutable timing-plan and copied Core observation boundary.
3. Apply generic CPU retirement and transaction costs on the sole Core axis.
4. Compose eligible controller deadlines and retain explicit ineligible/L2
   dispositions without fabricated time.
5. Prove Standard ahead-only pacing and Turbo no-wait on the same completed
   Core observations, including reset, HLT, pause and debugger boundaries.
6. Audit the whole ledger, remove superseded paths, run full gates and publish
   the task Release artifact.

## Non-goals And Stop Conditions

No L4 electrical timing, host-to-Core tick injection, profile-name branch in
Core, third-party source import, protected asset use, second scheduler, or
machine-profile implementation is allowed. Stop for owner direction if a
generic capability requires a machine-specific fact that cannot be represented
as immutable plan data, or if a selected input has neither primary evidence nor
a defensible explicit L2 disposition.
