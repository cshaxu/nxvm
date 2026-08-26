# T485 S3 8088 CPU And XT Bus Common-Axis Ledger

`M5:T485:S3:8088-COMMON-AXIS:ACCEPTED`

## Primary-source facts

Intel, *iAPX 86, 88 User's Manual* (1981), Table 2-21, printed page 2-56,
was visually checked from the original PDF.  It labels each listed execution
cost as `Clocks` and its footnote states that the 8088 adds four clocks for
each 16-bit word transfer.  This is a Manual-L3 CPU input, not an instruction
count conversion.  The pre-existing T484 S2 rendered-source record supplies
the same manual's four-byte 8088 queue rule.

IBM's 5160 Technical Reference, as retained in T483's source record,
supplies the board relation: 14.31818 MHz divided by three is 4.77 MHz;
ordinary memory read/write cycles are four 210 ns clocks, while selected I/O
cycles are five.  It also distinguishes five-clock DMA cycles.  These are
Manual-L3 board relations, but do not identify every current Core external
access as one of those cycles.

## Current owner reconciliation

| Relation | Current sole owner | Result |
| --- | --- | --- |
| Successful CPU retirement and `elapsed_ticks` publication | `core_machine_run()` through `core_machine_publish_elapsed_ticks()` | One Core owner exists; no VM/profile clock writer is needed or permitted. |
| 8088 fetch state | Core CPU execution context | The four-byte prefetch capacity is already one Core-owned state, but it has no source-backed cycle-cost publisher. |
| 8088 instruction clocks | `core_machine_cpu_timing_select()` | Blocked.  The 8088 branch selected the generic compatibility recipe, not a Table-2-21 source-form ledger. |
| External memory/I/O waits | Core external-cycle trace and copied transaction contract | Blocked.  Present page/wait-window values are generic profile inputs; they are not an IBM 5160 4/5-clock classifier.  Physical mode already rejects their `EXTERNAL_WAIT` publication. |
| PIT ratio | Core rational-clock plan | Still blocked by S2.  It can consume the axis after, but cannot create it. |

## Immediate correction

The retained compatibility recipe is not a source-form allocation.  S3 marks
it `SOURCE_UNALLOCATED` at its single owner.  The focused physical-8088
retirement regression proves that a physical configuration faults before the
first retirement and leaves both elapsed time and the timeline at zero.
This is deliberately global: no CPU profile may qualify a generic fallback as
physical time.

## Next finite receiver

S4 must add an Intel Table-2-21 8088 source ledger to the existing Core CPU
timing owner, including its variable transfer and control outcomes, then
produce its bounded retirement-qualification set.  S5 may only bind the
IBM-selected 4/5-clock cycle conditions at the existing Core external-cycle
owner after each current access class has a source rule.  Neither S may add a
profile clock, a second scheduler, or a host-time writer.  Until both are
accepted, the XT time axis, PIT/DMA ratios and physical pacing remain blocked.

## Gate reconciliation and verification

The focused physical-8088 retirement smoke passes.  The full configured
current gate then exposed only stale guard expectations, not a second runtime
path:

| Guard | Current single owner recorded by the repair |
| --- | --- |
| T344/T345 direct-source inventories | The two XT device-owner smokes remain direct by design; their 77-constructor and 58-residual inventory is explicit. |
| Public raw-borrow closure | `core_machine_pic_topology` moved to the minimal public `pic_interface.h`; the private PIC state remains private. |
| Display/DMA/RTC/ATA and EGA boundary guards | The immutable default-PC/AT profile resolver publishes copied Core topology/configuration; session composition does not regain a duplicate publication path. |

The regenerated full current gate passes: all static ownership checks pass and
all 297 current-gate tests pass.  No profile clock, host-time writer, guessed
8088 cost, or controller ratio was added.
