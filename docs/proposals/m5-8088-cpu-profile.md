# 8088 CPU-Profile Admission And Timing Closure

## Purpose

Add an explicit 8088 CPU profile while retaining the existing 8086 profile.
The 8088 shares the 8086 instruction-set semantics, but it owns a distinct
external 8-bit data-bus, instruction-fetch/prefetch, and physical-transaction
timing boundary required by the IBM PC/XT 5160-268 baseline. This candidate
follows the DeskPro 386 Model 40 L3 audit, precedes the 5160-268 profile audit,
and does not
redefine the completed 8086 instruction ledger.

## Required scope

Start with an Intel-primary 8086/8088 difference ledger and current caller
sweep: profile enum/validation, configuration and product parsing, instruction
acceptance/rejection equivalence, reset state, timing dispatch, fetch/prefetch,
memory and port transaction width, DMA availability interaction, trace, and
all affected tests. Implement only the shared-profile boundary and the
source-backed 8088 differences required by the selected 5160-268 corpus. The
later 5160-268 tasks own profile selection, functional devices, board-specific
waits, adapters, DMA/PIC/device timing,
and physical closure.

The ledger must also consume the retained 8086 dynamic-arithmetic timing
transfers that affect the 8088 semantic profile. Each form must receive an
Intel-exact, source-labelled model, or explicit reference-exhausted disposition
before the 5160-268 closure; it may not inherit the closed T362 proposal as a
live receiver. The separate 80186-only rows remain CPU debt because this M5
program selects no 80186 machine.

## Non-goals and stop conditions

No replacement or deletion of 8086, no invented instruction semantic
difference, no generic PC/XT machine, no board wait-state value without the
selected 5160-268 contract, no copied emulator code, and no Windows claim. Stop
if a purported difference is only board/chipset behavior or lacks an
Intel-primary or explicitly bounded probe contract; transfer it to the machine
candidate instead.

## Evidence standard

Require an 8086-versus-8088 source matrix, caller/write/ABI inventory,
profile-selection and retained-8086 regressions, focused fetch/bus/transaction
traces, reset and fault nonpublication proof, full current-gate evidence, and
an explicit handoff to the 5160-268 profile audit. Where primary material gives only a
range or no timing value, named 86Box/MAME/PCjs probes may cross-check the
selected physical contract without becoming implementation authority.
