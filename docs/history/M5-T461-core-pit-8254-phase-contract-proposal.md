# M5 Core PIT 8254 Phase Contract

## Purpose

Define selected 8254 counter command, count, gate, output and IRQ0 phases.

## Admission And Dependencies

Consumes PIC. Admit the original 8254 manual and selected board input-clock,
gate and IRQ0 wiring facts before an L3 frequency or phase selection.

## Scope And Completion

Freeze control-word, read-back/latch, reload, modes, gate transitions, output,
IRQ0 visibility, rational clock conversion and reset rows. Prove exact selected
counter transitions and reset phase without host-clock dependence.

## Planned Subtasks

1. **S1: source and gap reconciliation.** Freeze manual-first Lists 1--2 and
   distinguish chip semantics from board/electrical boundaries.
2. **S2: unified chip phase contract.** Complete control/count write,
   latch/read-back, null-count, delayed load and all mode 0--5 transitions in
   the existing PIT owner. These rules share the same CR-to-CE boundary and
   must not be split into an invalid intermediate state machine.
3. **S3: integration and closure.** Prove selected OUT-to-IRQ0 routing,
   scheduler order, reset/finalize and each retained L2 boundary.

## Boundary

Unselected oscillator phase and electrical waveform remain L2. It does not
change speaker presentation or create a second event scheduler.
