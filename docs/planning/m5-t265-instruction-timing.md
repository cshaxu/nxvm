# M5 T265: Level 2 Instruction-Timing Contract

## Scope

T265 adds deterministic, profile-owned instruction-cost attribution to the
existing Level 1 rational device clocks. `core_machine_run()` remains the sole
guest executor and the sole writer of `elapsed_ticks`. It does not add x86
semantics, host-clock timing, wait states, DMA arbitration, prefetch, or a
cycle-accuracy claim.

## S1: Timing Model And Corpus

**Status:** complete.

The frozen `core_machine` timing configuration contains a base cost plus
bounded surcharges for prefix bytes, taken short conditional branches, selected
data-memory MOV forms, IN/OUT, and REP MOVSB iterations. A zero timing table
uses the existing `ticks_per_instruction` as its base and has no surcharges.
Default PC/AT is an explicit frozen consumer of the table.
It retains a base-only table in this task: calibration of nonzero PC/AT
surcharges needs its own hardware/profile corpus and is not inferred here.

Completed instructions alone commit time. Prefixes belong to their completed
non-REP instruction. The current executor already advances REP MOVSB one
iteration at a time, so each admitted iteration uses base plus its iteration
surcharge, without charging the repeated prefix separately. A fault, stop,
pause, or reset commits no extra cost. Before
execution, the core checks a deterministic maximum cost against a tick budget;
it may pause before an instruction rather than overshoot. REP cost is per
executor iteration, not a claim that all memory accesses have a generic timing
model. Cost arithmetic and elapsed-tick updates
must reject overflow deterministically.

The owned corpus will cover NOP, selected register and moffs MOV forms,
immediate/DX IN and OUT forms, short Jcc both outcomes, and REP MOVSB. It will
compare an identical program under single and split run budgets, cold reset,
pause/step boundaries, and fault/stop boundaries. It will also retain Level 1
rational-clock, PIT IRQ0, display, DMA/FDC, DOS boot, Console, debugger, and
current-gates evidence.

**Stop conditions:** a need for a VM-side tick path, host timing, a second CPU
loop, unbounded REP accounting, or changed Console/debugger/boot behavior
stops the task for redesign.

**S1 closure:** the audit confirmed one existing tick-commit point in
`core_machine_run()`, after CPU refresh and before the Level 1 scheduler. The
decoder already exposes copied current instruction bytes, prefix state, branch
outcome through EIP, and REP's one-iteration executor loop; no decoder or VM
execution path was needed.

## S2: Core-Owned Implementation

**Status:** complete.

`core_machine_instruction_timing` is frozen in `core_machine_config` and held
by each `core_machine`. The legacy `ticks_per_instruction` is retained as a
zero-surcharge base shorthand. The executor derives costs from copied decoder
observations for the bounded corpus and commits only that cost to the existing
Level 1 scheduler. The default PC/AT profile carries an explicit base-only
table; it does not claim an unmeasured calibration.

## S3: Evidence And Closure

**Status:** complete.

`core-machine-instruction-timing-smoke` proves the admitted base, prefix,
data-memory, I/O, short-branch, REP, reset/quantum, and fault paths. It is in
the current smoke matrix. `current-gates-gcc` passed **101/101** CTest after a
first deliberate correction: default PC/AT surcharge values were returned to
zero because an uncalibrated table changed the retained BIOS-timer regression.
The rebuilt `build/output/nxvm_0_5_0265.exe` SHA-256 is
`29DF9D6ACB8EA6E8EDB51F595E10908B80802A00DF15C0501DAB82DE509DC67A`.
