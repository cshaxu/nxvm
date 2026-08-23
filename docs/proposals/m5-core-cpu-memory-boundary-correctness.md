# M5 Core CPU And Physical-Memory Boundary Correctness

## Purpose

Repair confirmed local CPU guard and physical-memory range validation defects
without changing instruction/profile scope or introducing a new memory layer.

## Required Scope

Replace the unreachable/contradictory CPU instruction guard with the intended
finite predicate, and make physical mapping arithmetic reject overflow and
invalid spans before mutation. Sweep equivalent arithmetic/guard shapes in
tracked Core production paths and classify every hit.

## Dependencies And Completion

Completion requires focused positive/negative instruction and mapping tests,
boundary/overflow probes, the recorded similar-issue sweep, and current gates.

## Minimalism Constraints

- The CPU lexical/decode owner contains the one intended finite instruction
  predicate; callers do not restate profile or opcode validity.
- The physical-memory mapping owner rejects invalid or overflowing spans before
  it appends or mutates mapping state; access callers consume its status rather
  than recalculate ranges.
- A shared owner-local helper is allowed only when it deletes duplicated range
  arithmetic. No facade, cached validity state, compatibility branch, or new
  memory layer is permitted.
- Acceptance records the net source/test line change and each removed duplicate
  guard. A net addition requires a named, non-duplicable responsibility.

## Non-goals And Stop Conditions

Do not broaden CPU emulation, alter legal instruction semantics beyond the
proved guard, change guest memory topology, or refactor the memory subsystem.
Stop if a discovered hit changes a separately owned execution contract.
