# T357 S1: Instruction-Timed Execution Contract

## Decision

T357 starts from a verified but synthetic execution cadence. It must not turn
that cadence into a timing claim merely by adding opcode exceptions. S2
therefore receives a finite profile/form ledger, a single publication owner,
and explicit physical exclusions.

## Current Source Inventory

| Owner or consumer | Current route | Disposition |
| --- | --- | --- |
| CPU profile/configuration | `core_machine_create()` resolves `cpu_profile`, `ticks_per_instruction`, and `instruction_timing` in `src/core/machine/machine.c`. | S2 replaces the profile-agnostic synthetic configuration as the selected-form authority; compatibility input is translated once or rejected explicitly. |
| Cost calculation | `core_machine_instruction_cost()` recognizes leading prefixes, `REP MOVSB`, short conditional branches, moffs `A0`--`A3`, and ordinary `IN`/`OUT`; it otherwise emits `base_ticks`. | S2's one owner. Current recognition is not a form ledger and does not inspect profile timing. |
| Publication | `core_machine_run()` calls CPU refresh, rejects fault/overflow before incrementing `executed`, `ticks`, and `elapsed_ticks`, then records CPU retire and advances the scheduler. | Retain as the only guest-time publication point. S2 computes an approved cost after successful CPU refresh and before this publication; faults/stops publish no instruction cost. |
| Budget | `maximum_instruction_ticks` is a configuration-derived upper bound used before CPU refresh. | S2 derives a safe maximum from the selected immutable profile/form ledger; it must not preclude a legal selected form. |
| Device time | `core_machine_advance_scheduler()` advances the deterministic timeline at elapsed time, then advances provider clock; scheduled callbacks drive DMA/PIT/PIC, FDC/HDC/RTC, and KBC/VADP. | Preserve T354 ordering. S2 supplies only CPU elapsed ticks, never a second scheduler. |
| Clock conversion | `clock.c:core_machine_clock_domain_advance()` converts elapsed core ticks through exact rational domains. | Retain; device ratio conversion is not an instruction timing table. |
| Existing proof | `tests/machine/core_machine_instruction_timing_smoke.c` injects `{10,2,7,3,5,4}` and proves its synthetic surcharge paths, reset, fault, and stop behavior. | Retire or rewrite its synthetic assertions in S2; retain its fault/no-publication and reset split-run observations. |

No other production write to `machine->elapsed_ticks` was found by the S1
`rg` sweep of `src/`; test/configuration hits are consumers or setup only. VM
composition forwards profile `ticks_per_instruction`; it does not publish guest
time.

## Primary Reference Contract

Selected form costs are transcribed from Intel primary manuals, not from
Bochs/PCjs behavior. Those implementations may later be used only for bounded
differential experiments under the source policy.

| Profile | Primary reference | Relevant contract |
| --- | --- | --- |
| 8086 | [Intel 8086 Family User's Manual, Table 2-21](https://bitsavers.org/components/intel/8086/9800722-03_The_8086_Family_Users_Manual_Oct79.pdf) | Form clocks, effective-address additions, transfers, and documented odd-word adjustments. |
| 80186 | [Intel Embedded Microcontrollers and Processors, volume 2, 80186 execution timings](https://f6.erista.me/files/bitsavers/components/intel/_dataBooks/1993_Intel_Embedded_Microcontrollers_and_Processors_Volume_2.pdf) | The table's stated assumptions are prefetched opcode/data/displacement, no wait states or bus HOLD, and even word data; S2 records every selected table row/page before a value is admitted. |
| 80286 | [Intel 80286/80287 Programmer's Reference Manual](https://bitsavers.org/components/intel/80286/210498-005_80286_and_80287_Programmers_Reference_Manual_1987.pdf) | Form timing and protected-mode instruction semantics; S2 records the exact form/table location. |
| 80386 | [Intel 80386 PRM section 17.2.2.3](https://www.ardent-tool.com/CPU/docs/Intel/386/manuals/prref386/s17_02.htm) | Clock counts assume prefetched/decoded instructions, no wait states/HOLD, no exceptions, aligned memory; r/m tables distinguish register and memory forms. |

These assumptions are material: bus wait states, unaligned transfer penalties
where not covered by the selected manual row, HOLD/DMA ownership, prefetch, and
exception delivery timing are not CPU form-cost values. They transfer to
bus-timed PC/AT operation or the selected cycle-exact profile.

## S2 Selected Corpus

| Form family | Variants that S2 must allocate per profile | Evidence and boundary |
| --- | --- | --- |
| Baseline simple execution | `NOP`; immediate-to-register `MOV`; flag-only form. | Establish profile base values and an immutable table lookup. |
| Conditional control | short `Jcc`, taken and not-taken. | Prove post-refresh branch disposition selects the approved form cost. |
| Memory operand | selected register-versus-memory `MOV`, moffs read/write. | Form/EA category only; wait states and physical bus duration excluded. |
| Port operand | immediate and `DX` `IN`/`OUT`. | Form direction/width selection only; port-device latency excluded. |
| Prefixes | segment, `66h`, `67h`, `LOCK`, `REP` only where the reference defines a selected cost. | Preserve profile validity and prefix length; do not convert prefix count into a universal timing rule. |
| String repeat | `MOVSB` single and bounded `REP` iterations. | State per-instruction versus per-primitive accounting and preserve interrupt/restart semantics. |
| Failure/interrupt boundary | #UD/profile rejection, synchronous selected fault, and delivered external IRQ. | No successful-form cost is published before a fault; interrupt-delivery cost is explicitly classified, never guessed. |

All other implemented forms remain unallocated after S2 until their primary
source rows are admitted. Their disposition must be stable and observable: the
owner either uses the documented fallback policy selected by S2 or rejects
timing-mode activation before execution; it may not inherit an unrelated
opcode's number.

## S2 Mechanism Boundary

The implementation has one immutable, profile-indexed timing ledger and one
owner-local classification routine. It receives only decoded/current instruction
outcome valid after CPU refresh, then returns a bounded cost to
`core_machine_run()` before the existing increment/trace/scheduler publication.
It does not expose a mutable timing table as a public ABI, put device latency
into CPU cost, or bypass reset/overflow/budget handling.

The S2 similar-issue sweep covers every former `instruction_timing` field,
`ticks_per_instruction` composition path, test fixture, profile declaration,
and elapsed-tick writer/reader. Any compatibility configuration retained for
API reasons is translated at create time into the same private plan; it cannot
form a second timing truth source.

## Transfers

| Boundary | Receiver |
| --- | --- |
| Memory/I/O wait states, bus arbitration, HOLD, and DMA physical availability | T369 closes logical availability only; physical values transfer to selected-profile phase refinement. [Retained proposal](../../history/M5-T369-bus-timed-pcat-operation-proposal.md). |
| Prefetch, bus phases, exact interrupt/fault cycle costs, alignment/caching microstate, and pin waveforms | Closed T371 [cycle-exact selected profile](../../history/M5-T371-selected-profile-phase-refinement-proposal.md). |
| x87 instruction/timing | Explicitly excluded by owner request. |
| Unselected CPU forms | Later timing-corpus admission only if backed by exact primary rows; otherwise retain to the applicable future receiver. |
