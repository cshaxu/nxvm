# Complete CPU And Bus Execution

Uses [shared context](../etc/m2-execution-management-context.md). Entry: accepted runnable
[vertical execution](M2-T5-app-core-vertical-execution-proposal.md) with its
original fixtures.

## Outcome

The same MyNes executable runs full original CPU programs covering arithmetic,
all addressing forms, branches, stack, subroutines and interrupt entry semantics.
All 151 documented opcode forms work; all 105 other bytes trap explicitly.

## Coverage And Implementation

Consume the entire 151-row opcode ledger in one package-level coverage universe,
not a sequence selected by whichever ROM fails first. Complete the same CPU
microcycle mechanism, official operation semantics, addressing/wrapping, flags,
binary RP2A03 ADC/SBC, stack/control instructions, bus map and cold/warm reset
hardware operations. IRQ/NMI functional vector/stack behavior is included; exact
cycle-by-cycle polling qualification belongs to cycle refinement. No second CPU.

All ordinary base/conditional instruction cycle totals are implemented and tested
here. Timing refinements concern observable transfer order and interrupt sampling,
not permission to claim arbitrary cycle totals or omit instructions. Preserve
explicit traps at unsupported device addresses and genuine errors at host faults.

## Acceptance And Transfer

For every opcode assert decode/length/result/flags and cycle totals. Cover all used
address modes and functional wrap/cross cases; exhaust ADC/SBC A/M/C combinations
for both D values. Reject all 105 excluded bytes. Original programs exercise
JSR/RTS, stack, arithmetic and BRK/RTI through the production bus and reach named
PC/register/RAM checkpoints. No private register setters or mirrored emulator.
All first-package monitor/media tests and x64/x86 deliverables remain valid.

Exit closes functional CPU-ALL/ADDR/ALU and functional CPU-CTRL/MAP obligations.
Cycle refinement receives their ordered bus/poll timing dimensions explicitly;
the ledger must not mark those dimensions passed prematurely. Debugger workflow
receives the completed instruction metadata. Full product failure matrices go to
reliability. No display/audio/game compatibility claim; shared stop rules apply.
