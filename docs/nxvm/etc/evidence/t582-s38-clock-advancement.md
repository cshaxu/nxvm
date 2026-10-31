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
