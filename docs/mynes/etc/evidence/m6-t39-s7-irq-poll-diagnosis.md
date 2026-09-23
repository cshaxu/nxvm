# M6 T39 S7 Corrected-Clock Pixel-Path Diagnosis

## Result

The black frame is downstream of CPU progress, not an unfilled palette or
background shifter.  An ignored corrected-clock probe observes a persistent
Cartridge IRQ, PC at the RAM IRQ-vector entry and repeated copies of the same
return address throughout the CPU stack.

`core_machine_service_interrupt` sets the CPU I flag but leaves
`irq_poll_i` at the preceding instruction's CLI-derived value.  The executor
therefore services the asserted level IRQ again before it executes the first
IRQ-handler instruction.  The handler cannot acknowledge `$E000`, the stack
overflows and controller polling/pixel preparation cannot progress.

## Transfer

S8 must set the IRQ polling state to masked when an interrupt is accepted and
retain an owned regression proving a level IRQ cannot re-enter before at least
one handler instruction executes.  It must repeat the ignored Start/frame
probe after the CPU repair.
