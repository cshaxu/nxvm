# IBM 5160 8088 Board, Memory, ROM And ISA Phase Contract

## Purpose

Establish the IBM 5160-specific 8088 oscillator, memory/ROM availability and
ISA transaction contract after the CPU T; this is one board unit, not a CPU or
controller substitute.

## Required sequence

S1 validates the IBM original board references. S2 freezes List 1 of the full
admitted board surface: clock, normal/five-clock cycle, ROM/RAM, reset, ISA
and NMI-facing rules. S3 maps List 2 to the sole Core transaction/time owner,
its production entry and its consumer paths. The implementation S consumes the
complete batch with trace/reset proof; it does not add a parallel board path.

## Boundaries

No CPU opcode timing, device controller state, guessed waits, host pacing or
new scheduler. Every controller timing fact belongs to its own unit T.
