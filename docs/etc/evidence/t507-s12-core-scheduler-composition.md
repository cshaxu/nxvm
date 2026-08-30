# T507 S12 Core Scheduler Composition

`M5:T507:S12:CORE-SCHEDULER:P1`

## Frozen Composition

The accepted S1 matrix remains complete: timeline, PIT/auxiliary PIT, RTC,
qualified DMA, FDC and keyboard provide an earliest real deadline; PIC settles
on producer mutation; VADP has no wake deadline; unqualified DMA, HDC and D4
are explicit L1 owners.  `machine_scheduler.c` remains the only elapsed-tick
consumer and retains the fixed same-tick order: timeline, arbitration,
readiness, peripheral, provider completion.

## Defect And Repair

Before this S, an active L1 HDC/D4/unqualified-DMA owner was ignored whenever
PIT, RTC, timeline or another real deadline existed.  The observation then
published that unrelated deadline and the scheduler could cross the L1 owner
without giving it its required bounded Core step.  That was neither a genuine
deadline nor correct event ordering.

The composition now has three ordered outcomes:

1. an already-due owner remains immediate;
2. an active L1 owner publishes the existing opaque L1 disposition and Core
   advances it one normal scheduler tick at a time, never crossing a real
   deadline; and
3. otherwise Core publishes the minimum qualified deadline and advances once
   to it.

Both Standard and Turbo request the same opaque Core L1 operation.  Standard
may first wait against completed Core progress; Turbo omits only that host
wait.  VM supplies no tick count, device deadline or fast-forward distance.

## Proof And Minimality

`core_machine_time_smoke` now arms a D4 L1 hold and a later timeline event.
It proves the first Core compatibility step services D4 at tick 1 and leaves
the timeline event for tick 4, then proves ordinary deadline advancement
delivers that event.  The speed-policy, D4 and scheduler focused cohort passes
4/4.  Complete repository-only unit passes 312/312 in 15.40 seconds.

The production change is two scheduler ordering branches and one VM waiting
condition; it reuses the existing observation, bounded compatibility API and
pacing helper.  It adds no controller field, callback, deadline value, public
ABI, profile path, host-to-guest tick, second scheduler or renderer/firmware
workaround.
