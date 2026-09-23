# Cycle And Interrupt Refinement

Uses [shared context](../etc/m2-execution-management-context.md). Entry: full CPU/bus plus
[debugger workflow](../history/M2-T7-nes-debugger-workflow-proposal.md) on the
same engine.

## Outcome

The execution and stepping paths have verified ordered bus transfers and
cycle-sampled reset/IRQ/NMI behavior, providing the CPU timing foundation needed
by later PPU/APU integration. This delivers corrections and deterministic
regressions in the runnable product, not a timing research report alone.

## Complete Timing Universe

Qualify every official opcode's ordered fetch/read/write sequence; indexed and
branch dummy accesses, zero-page/page/stack wrap, RMW old/new writes, all stack
and control-flow sequences and unavailable-device effects. Qualify seven-cycle
reset, IRQ/NMI priority, status pushes, I-flag latency, branch polling and the
documented NMI recognition/vector takeover gates. Drive pin transitions at each
cycle of the named short/branch/entry sequences, including assertion/release.

Use MOS original bus tables and identified NES/Visual6502 research as evidence;
runtime traces compare against independently derived expectations. Apply fixes
at the existing microcycle owner; no emulator-comparison-only acceptance, global
fudge cycles or second timing engine. Sub-cycle silicon claims remain excluded.

## Acceptance And Transfer

Every CPU-ALL/ADDR/CTRL ordered-timing dimension and MAP dummy-access obligation
must have direct proof or a pre-existing explicit profile exclusion. Step/run
produce equivalent traces at equal checkpoints. Check counter limits, finite
slices and interrupt-entry versus retired-instruction accounting. Re-run all
functional/debug tests and ship both architecture artifacts.

M2 CPU/bus accuracy closes here; reliability receives end-to-end fault/lifecycle
qualification. M3/M4 device timing remains a future capability, not a requirement
to implement PPU/APU in this package. Do not pick isolated first-failing games as
the coverage universe. Missing authoritative behavior triggers shared stop rules.
