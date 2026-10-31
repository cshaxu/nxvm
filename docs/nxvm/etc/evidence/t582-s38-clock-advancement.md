# T582 S38 — clock and device advancement

## Core timeline checks at startup

The scheduler queries the Core timeline for its next event and advances it at
each published interval. A task-only link wrapper counted and sampled both
existing functions with private PC110 media at the fixed ten-million-instruction
startup checkpoint. It did not remove either call or alter the returned value.

| Width | Elapsed | Next-due calls / empty / samples / TSC | Advance calls / empty / samples / TSC |
| --- | ---: | --- | --- |
| x64 | 20,191 ms | 10,297,651 / 10,297,651 / 40,225 / 2,045,805 | 10,289,535 / 10,289,535 / 40,193 / 2,035,820 |
| x86 | 24,196 ms | 10,298,956 / 10,298,956 / 40,230 / 2,145,202 | 10,290,835 / 10,290,835 / 40,198 / 2,123,260 |

Both reached 79,964,294 guest ticks and PC `0003dbc5`. Sampling every 256th
call with nonserialized TSC adds timing overhead and may alias the workload;
counts include the stopping interval. These are inclusive call intervals, not
whole-program percentages. The result is limited to startup, not Game or RDP.
The observed empty queue does not permit dropping advancement: it updates the
timeline's current tick even when no event is scheduled, which determines
valid future scheduling. The queue can also receive future events. A fast
path that bypasses its owner based on this one workload is not justified.

Together with prior deadline measurements, this rules out treating the timeline
as a proven explanation for the Machine-versus-CPU gap. Continue examining
the complete clock/device route and mutations before selecting a candidate;
no production code, API, user media or released executable changed by this
probe. Log SHA-256:
`45B06EBBC2087AC538FD0BB400BE870133513D2CA5FD9853AB1C27616D92A495`.

## Board PIC refresh at startup

A second task-only linker wrapper counted board-to-PIC refresh calls and, one
in 1,024 calls, compared both PICs' `irr`, `cascade_irr` and input levels
before and after the real refresh. In the same fixed startup checkpoint, x64
recorded 10,297,007 calls, 10,055 samples, zero sampled changes and 1,318,000
accumulated TSC units; x86 recorded 10,292,810 calls, 10,051 samples, zero
sampled changes and 1,579,060 units. Both reached 10,000,000 instructions,
79,964,294 ticks and PC `0003dbc5`. Link wrapping observes calls made across
object boundaries, not necessarily calls within the PIC bus object; the
sampling is deterministic, nonserialized and includes timer overhead.
This is startup evidence, not Game-stage or full-state equivalence.

The sampled refreshes need not change the three compared fields, but this does
not make them universally dispensable. The existing refresh recomputes slave
selection and propagates the cascade to the master; an unmask delay can expire
during PIC advance, and level-triggered requests can need relatching after
acknowledgement. Source edges and register writes also invoke refresh at their
own boundaries. Moving or omitting the periodic refresh without preserving
the ordering of these transitions can change when an interrupt becomes
visible. Investigate an event-bound alternative only with a complete mutation
and observation proof and representative timing; do not infer safety from
zero sampled changes. No production path was changed.

Log SHA-256:
`1255B9DD4DC005CC75F85AF3EA64088F16C1CB339659AFFDE86195834015077D`.
