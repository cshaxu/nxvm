# M5 Legacy Dynamic Arithmetic Timing Authority

## Purpose

Resolve the timing-authority boundary for the 8086 and 80186 arithmetic forms
whose Intel tables specify a range but no operand-to-clock rule. The task must
produce either a primary-source formula or an owner-approved, reproducible
hardware-observation contract. Until then these forms remain explicit
successful-retirement transfers and do not receive a guessed clock value.

## Scope

The candidate covers only 8086 `F6`/`F7` `MUL`/`IMUL`, and 80186
`F6`/`F7` `MUL`/`IMUL`/`DIV`/`IDIV` plus `69`/`6B` immediate `IMUL`, including
their profile-local operand width, register/memory, prefix, overflow/fault,
rollback, and source-consumer conditions. It inventories each primary table
statement, searches for a contemporary Intel formula, and defines the only
permitted observation alternative if the primary record stays incomplete.

## Boundaries

Do not choose a minimum, maximum, midpoint, later-processor formula, host
benchmark, or Bochs/PCjs value. Do not implement bus waits, device timing,
prefetch, exception delivery, or a general arithmetic refactor. A formula
must be captured before retirement without publishing on fault; an observation
contract must name the hardware/stepping, inputs, capture method, uncertainty,
reproducibility, and acceptance rule before it can allocate a value.

## Completion standard

Every listed form has either an exact profile-local authority and focused
consumer proof, or a precise retained transfer with an owner-approved next
admission condition. The task sweeps all timing consumers and preserves one
post-refresh publisher. It is a prerequisite for the complete instruction
timing corpus, not a cycle-exact claim.
