# T388 S3: Physical Eligibility Boundary

`M5:T388:S3:PHYSICAL-ELIGIBILITY-BOUNDARY:OK`

## Core Owner And Contract

`core_machine_instruction_cost()` clears the private
`source_timing_unallocated` classification before each successful-cost
selection. `core_machine_source_timing_mark_unallocated()` is the direct
successful-sentinel writer; `core_machine_source_timing_lookup()` marks a
missing immutable ledger row. The value `CORE_MACHINE_SOURCE_UNALLOCATED_TICKS`
remains the legacy deterministic-progress value, but the physical decision uses
the separate classification bit rather than numerical equality.

`core_machine_retirement_time_contract` has two typed configurations:
`CORE_MACHINE_RETIREMENT_TIME_DETERMINISTIC` preserves existing elapsed-time
and clock-plan behavior, while `CORE_MACHINE_RETIREMENT_TIME_PHYSICAL` refuses
a successful unallocated source cost at the sole post-refresh publication
boundary. The rejection occurs before `executed`, result ticks, elapsed ticks,
timeline, device clocks or the execution provider advance. External
`core_machine_advance_time()` is invalid in physical mode, so VM composition
cannot relabel host pacing or inject an unqualified source.

No current VM profile selects physical mode. This mechanism therefore creates
no 8 MHz or 16 MHz claim, profile ratio, board-clock enablement or artifact
revision.

## Regression

The owner smoke `core-machine-instruction-timing-smoke` constructs an 80286
physical contract. Its ledger-selected `NOP` retires and publishes three ticks.
A successfully executed prefixed `NOP` reaches the explicit sentinel branch and
is rejected before publication; the smoke requires zero executed/result/elapsed
ticks. It also requires `core_machine_advance_time(machine, 1u)` to return
`TYPE_STATUS_INVALID_STATE` in physical mode. The first and second cases prove
classification, not the numerical value, chooses the boundary.

## Transfers

This S3 safety boundary does not make a profile physically eligible. T388 S4
must add the source/form/context or approved-observation coverage necessary to
select physical mode for a concrete profile. The existing four-profile source
receivers, x87 receiver and DeskPro board-level timing task remain unchanged.