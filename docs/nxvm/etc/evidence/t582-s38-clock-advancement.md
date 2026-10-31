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

### Periodic-refresh removal upper bound

A scratch-only candidate removed the board's periodic `pic_refresh` call but
left source-edge, register and acknowledgement refreshes intact. This is not
an eligible fix: it changes cascade and level-trigger observation boundaries.
Its purpose was to estimate whether even eliminating the entire periodic call
might justify the proof and complexity of a correct event-based design.

With baseline/candidate/candidate/baseline order, fixed startup times in
milliseconds were 19,441 / 17,773 / 21,156 / 18,861 on x64 and 22,128 /
21,642 / 23,236 / 24,295 on x86. Every run reported 10,000,000 instructions,
79,964,294 ticks and PC `0003dbc5`; this does not prove equivalent PIC or
guest state. The x64 direction reverses, and x86 favors the altered version
by a small, noisy amount. The samples do not establish a substantial reliable
benefit, characterize Game, or justify a more complex invalidation mechanism.
No production code or released artifact was modified by the ablation.

Log SHA-256:
`CE90A9FB17DA091B19EFE29C8F526E519EAC02C53B2A3626D683A2480BB3C3C3`.

## Corrected absent-FPU scheduler upper bound

The earlier S26 absent-FPU experiment compiled the private Machine layout
without the Release trace definition and is invalid for performance comparison.
This replacement compiled both scheduler variants with
`X86_CORE_MACHINE_RUNTIME_TRACE_ENABLED=0` and the same cached Release
libraries. PC110 selects `X86_FPU_PROFILE_NONE`. The scratch candidate removes
both scheduler calls to the FPU unconditionally; it is invalid for machines
with an active FPU and is not a proposed production implementation.

At the fixed startup checkpoint, baseline/candidate times were 19,148 / 18,312
ms x64 and 23,541 / 23,564 ms x86. Each reported 10,000,000 instructions,
79,964,294 ticks and PC `0003dbc5`. One ordered pair per width is sensitive to
host and compilation-layout noise and does not establish complete state or
Game equivalence. It provides no evidence of a large cross-width speedup from
removing those calls; do not add a mirrored FPU-presence state or another
execution path on these results. No production code or release artifact was
changed. Log SHA-256:
`A3C161ECC0A86F7FC9050F6C00A1937B7E515B6E3B46BFDF2125084BF90B580B`.

## Zero-device-tick call candidate

Board advancement converts source ticks before invoking PIT and RTC. Both
existing device functions immediately return for zero elapsed ticks. A
task-only x64 link counter found 7,516,756 zero-tick PIT calls out of
10,294,762, and 10,215,303 zero-tick RTC calls out of 10,294,762, at the
fixed ten-million-instruction startup checkpoint. The counter instruments
every observed call and counts include the stopping interval; it measures
frequency, not exclusive cost or Game behavior. Log SHA-256:
`A8C674B07682802BEC9F1F8C2C3129553F7088DDC69B0209DB9E8C211FD9F1E4`.

A scratch candidate guarded the existing PIT and RTC calls when their already
calculated device ticks were zero; clock conversion, callback order for
nonzero ticks, trace and advancement of configured devices remained in place.
It was not qualified across all device configurations. Fixed startup
baseline/candidate/candidate/baseline milliseconds were 19,061 / 19,033 /
20,624 / 20,186 x64 and 23,878 / 25,376 / 25,109 / 23,218 x86. Each run
reported 10,000,000 instructions, 79,964,294 ticks and PC `0003dbc5`. The x86
candidate was slower in both comparisons, so frequent empty calls alone do not
justify the additional branches. These observations are noisy and do not prove
complete guest-state equivalence or Game performance. The candidate was not
applied to production. Log SHA-256:
`7CF14036366D248371341D530E1FABD6B75425235E9FE0C1D654F4467EFE1993`.

## CPU bus advancement idle case

A task-only link wrapper counted the existing CPU bus advancement callback at
the fixed PC110 startup checkpoint. It classified calls with zero write-buffer
count, zero service credit and zero invalidation cycles, and sampled one in
256 call intervals. x64 recorded 10,294,318 calls, 2,237,289 idle cases,
40,212 samples and 2,462,780 accumulated TSC units; x86 recorded 10,290,046,
2,235,549, 40,195 and 2,853,210 respectively. Both completed 10,000,000
instructions at 79,964,294 ticks and PC `0003dbc5`.

Only about 22% of these calls met the simple idle condition. The sampled
intervals include timer overhead and deterministic sampling may alias the
guest; they are not exclusive whole-process measurements or Game evidence.
An additional branch for this bounded case would not remove the per-instruction
arbitration or establish a substantial product gain, so it is not adopted on
these observations. No guest timing, production source or artifact changed.
Log SHA-256:
`EF6894B2BFFF575CEA3309EEFD5C7091586F1935F13452ED28286D7162DC0C34`.
