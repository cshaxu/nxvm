# Four-Profile CPU Physical-Timebase Closure

## Purpose

Make the shared successful-retirement time axis eligible, where primary
evidence permits, to represent the selected processor's physical clock domain.
This is a prerequisite for a profile to convert CPU execution to PIT, DMA, or
other board-clock domains. It is distinct from CPU instruction semantics,
device service, bus arbitration, and final model-L3 acceptance.

The current publisher intentionally emits one-tick
`CORE_MACHINE_SOURCE_UNALLOCATED_TICKS` sentinels for some successful forms.
Those values preserve deterministic progress but cannot be described as CPU
oscillator cycles. A profile must not label or ratio that mixed axis as 8 MHz,
16 MHz, or any other physical clock.

## Required Scope

Start from the shared 8086, 80186, 80286, and 80386 successful-retirement
inventory and every source of elapsed-time publication. For each reachable
successful form/context in a selected profile, record one of the following:

1. an Intel-primary cycle value or primary-authorized formula that the sole
   publisher can emit;
2. a primary-underdetermined row retained outside the physical-time profile
   contract, with an explicit prohibition against advancing board clocks from
   that instruction; or
3. a bounded, owner-approved secondary-observation contract, only where the
   authoritative manual provides a range or no deterministic value, and only
   after the exact instruction/form context has been fixed by primary sources.

The task must then define and prove a single Core-owned conversion boundary:
the unit and frequency of every source which may advance a board clock, its
validation/publication order, reset/stop/fault behavior, overflow behavior,
and its interaction with the existing deterministic timeline. A VM profile may
select a documented frequency and rational clock plan, but cannot relabel a
mixed retirement counter or create a second scheduler.

The selected 80386DX-16 Model 40 and 80286 5170 consumers must each prove
their CPU-to-PIT conversion against this boundary before a physical board
clock claim. The later 8088/XT profile consumes the 8086 semantic result but
still owns its separate bus/prefetch contract.

## Non-goals And Stop Conditions

This is not a CPU semantic-completeness program, a device or board timing
task, a universal cycle-exact claim, a prefetch/cache/pipeline model, or a
host-time source. It does not make an unallocated form one cycle by default,
nor borrow a later-processor value. It does not convert host pacing while HLT
into a guest board-duration fact.

Stop and transfer a row when neither a primary value/formula nor an approved
secondary observation can identify a deterministic selected-profile value.
The final task decision must say whether the affected machine may make a
physical-clock claim; deterministic event ordering alone is not sufficient.

## Evidence And Completion Standard

Require a complete source/form/context-to-timebase matrix; a publisher and
clock-domain consumer sweep; focused profile/reset/fault/overflow regressions;
and current-gate verification. The closure audit must prove that no successful
unallocated path can silently advance a physical board clock, or retain every
such path as an explicit prohibition/receiver. It transfers CPU semantic
defects to the separate four-profile instruction-correctness program and
device/transaction rules to their owning board or device tasks.