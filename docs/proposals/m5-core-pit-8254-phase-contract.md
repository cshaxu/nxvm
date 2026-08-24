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

## Boundary

Unselected oscillator phase and electrical waveform remain L2. It does not
change speaker presentation or create a second event scheduler.
