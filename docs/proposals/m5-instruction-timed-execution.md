# Instruction-Timed Execution

## Purpose

Create the first physical-L3 prerequisite: a profile-aware instruction-cost
owner that advances guest time from an explicit instruction/form contract.
Today the executor supplies elapsed ticks without an Intel timing table;
clock-domain ratios can distribute those ticks but cannot make them
architectural.  This candidate must make that boundary explicit without
pretending to be cycle exact.

## Required scope

Use the applicable Intel 8086/80186/80286/80386 timing references and the
accepted CPU form ledger to allocate cost classes for a selected finite form
corpus: base decode/execute, taken/not-taken control flow, prefix effects,
memory/port accesses, and fault/interrupt boundaries.  Map every admitted
class through the single CPU-executor elapsed-tick publication point and prove
that reset, stop/fault, DMA/PIT/RTC ratios, and current selected L3 ordering
remain deterministic.

The work must inventory all CPU elapsed-tick writers/readers, validate timing
before state publication where a form can fault, and establish a stable
unknown-form disposition.  It must not add timings incrementally from guest
success or make the CPU executor a second device scheduler.

## Non-goals and stop conditions

No wait-state table, physical bus ownership, prefetch queue, pin waveform,
cache model, x87 timing, host wall-clock coupling, or Windows compatibility
claim.  Stop and split if the timing contract requires a chipset/device bus
model rather than CPU form cost, or if a reference does not support the chosen
profile/form corpus.

## Evidence standard

Require a source-to-form timing ledger, one CPU time-owner, focused profile
and fault/publication proof, reset/timeline regression, actual current-gate
result, and explicit transfer of all unallocated forms.  It is a prerequisite
for bus-timed PC/AT operation, not its substitute.
