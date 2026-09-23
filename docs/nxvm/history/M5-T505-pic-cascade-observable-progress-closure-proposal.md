# M5 PIC Cascade Observable-Progress Closure

## Purpose

Remove the one T504 L1 candidate in which a slave PIC request exists but its
master cascade request has not yet been published. Determine from Intel 8259A
material and selected XT/AT board facts whether this is immediate ordered Core
work or has a source-qualified observable deadline. Do not invent a delay.

## Scope And Constraints

The sole PIC owner retains IRR/ISR/IMR, cascade, acknowledge, EOI and reset.
Core retains event order and CPU delivery; profiles contribute only immutable
topology at construction. VM and firmware receive no PIC state, clock, or
compatibility path. This task must not add a second interrupt dispatcher or
turn a causal publication into a guessed timing scalar.

## Required Sequence

1. Admit and quality-check Intel 8259A and selected board sources; cross-check
   the one route against 86Box, MAME, PCjs, Bochs and QEMU where available.
2. Freeze List 1 for slave request, master cascade publication, masking,
   acknowledge, EOI, reset and CPU-visible wake ordering.
3. Produce List 2 by tracing every List-1 row through the current PIC, Core
   scheduling and CPU interrupt path.
4. Implement the complete owner-local correction batch: publish it as
   immediate ordered work when that is the evidenced result, otherwise expose
   only a sourced deadline. Sweep all selected topologies and consumers.
5. Run owner tests, complete unit and affected integration regressions; record
   every unsupported timing relation with its next receiver.

## Exit Criteria

The cascade candidate no longer appears as an unclassified T504 L1 wait; its
source-backed immediate/deadline disposition, cancellation/reset semantics and
CPU-visible ordering are proven through the one PIC owner. No guessed delay,
profile-side PIC behavior, second scheduler or host-time path is introduced.
