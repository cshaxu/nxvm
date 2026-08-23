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

## Non-goals And Stop Conditions

Do not broaden CPU emulation, alter legal instruction semantics beyond the
proved guard, change guest memory topology, or refactor the memory subsystem.
Stop if a discovered hit changes a separately owned execution contract.
