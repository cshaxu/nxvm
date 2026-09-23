# T506 S2: DMA Deadline Batch

## Decision

The selected 8237A service relation has one qualified deadline only when the
already copied Core plan declares both a source rational DMA clock and
source-backed DMA service phases.  This is the existing IBM 5170 Model 339
`3/8` conversion from the nominal eight-MHz Core axis to the documented
three-MHz DMA clock, together with the documented logical service phases.

The scheduler now asks its existing clock domain for the source ticks until
the next single DMA phase whenever an eligible request is pending.  It then
uses the ordinary Core deadline publication path.  At reset phase zero the
first phase is due at source tick three; every later phase is recalculated from
the same clock-domain remainder.  This is a controller deadline, not a claim
that the macro axis is host wall time or that an entire command has one fixed
duration.

## Complete D1--D10 disposition

| Rows | After disposition |
| --- | --- |
| D1--D5 | Unchanged Manual-L3 request, arbitration, logical phase, mode and completion ownership in the sole DMA service path. |
| D6--D8 | Unchanged Manual-L3 board routes; their PIT/FDC/Xebec producers retain their own deadline responsibility. |
| D9 | Unchanged Manual-L3 reset and request-withdrawal lifecycle; the deadline predicate observes existing pending state only. |
| D10 | Model-339's existing source-qualified rational clock plus source DMA-phase rule is now an L3 next-phase deadline through the existing scheduler/clock owner. Default PC/AT, XT and Model-40 retain their existing lower-tier rules and do not take this route. |

The Model-40 wait/BUSRDY contract is deliberately excluded: it is not the IBM
five-clock service term and cannot be used as a DMA deadline.  No provider
timer, profile callback, new public API, service-duration constant, or second
DMA state was added.

## Regression and simplicity record

`core-machine-plan-smoke` now constructs the qualified plan, binds one opaque
channel, asserts DREQ, observes deadline tick three, and advances through the
normal Core deadline API.  The focused test passes, the complete
repository-only unit suite passes 312/312 with `ctest -L unit -j 8`, and the
documentation-governance target passes.

Tracked source/test delta: `+68/-1` lines across
`machine_scheduler.c` and `core_machine_plan_smoke.c` (documentation excluded).
The retained production path is `Core observation -> existing clock domain ->
existing scheduler arbitration -> existing DMA service`; the added predicate
only admits that path for the already validated plan declaration.
