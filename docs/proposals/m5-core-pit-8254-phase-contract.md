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
2. **S2: control and observation contract.** Complete control/count write,
   latch/read-back, null-count and initial-output semantics in the existing
   PIT owner.
3. **S3: modes 0--2.** Implement delayed load, terminal, one-shot,
   rate-generator and gate transitions through that same owner.
4. **S4: modes 3--5.** Implement square-wave odd/even phases and both
   strobe/retrigger contracts without a parallel timer.
5. **S5: integration and closure.** Prove selected OUT-to-IRQ0 routing,
   scheduler order, reset/finalize and each retained L2 boundary.

## Boundary

Unselected oscillator phase and electrical waveform remain L2. It does not
change speaker presentation or create a second event scheduler.
