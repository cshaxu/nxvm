# T582 S32 — PC110 execution-cost investigation

**S34 setup correction:** The S31/S32 fixed-work probes used empty CMOS next
to their executables, not the copied configured seed in their working
directories. See [the correction](t582-s34-cmos-probe-correction.md). The S20
and other independently described earlier product-path runs are not reclassified.

This investigation continues the product usability objective after S31; it is
not a claim that PC110 is playable. The baseline is Shared `b2c17ba68` and
NXVM `178dc86a9`.

## Existing attribution and boundaries

The [S27 envelope](t582-s27-performance-envelope.md) compares ten million
synthetic NOP/JMP instructions. CPU-only and Machine have different bus,
initialization and time-publication contracts, so their wall-time difference
is not an exclusive Machine cost. [S20](t582-s20-execution-cost.md) estimated
CPU refresh at roughly 65% of a sampled World Clock Machine-run interval;
the remainder includes Machine timing, device and retirement work as well as
instrumentation and scheduling effects. Nested term samples are not additive
whole-program shares.

The [S26 synthetic ablations](t582-s26-pc110-emu-measurement.md#synthetic-loop-timing-and-publication-ablation)
measured 8603 ms for normal Machine execution, 7863 ms without time
publication and 4993 ms with a constant timing selector. Both modifications
deliberately break correctness and cannot be shipped; their individual
samples are diagnostic upper bounds, not exclusive timing or comparable
complete machine state. Together with the earlier [World Clock timing-term
samples](t582-s26-486-term-sampling.md) and
[board samples](t582-s13-scheduler-observation.md), they prioritize the CPU timing and
observation chain alongside Machine memory and advancement, without assuming
that any single term explains the product slowdown.

## Machine deadline capture on the current baseline

A task-only copy of `machine_scheduler.c` sampled the existing private
deadline capture once per 2,048 calls with the Windows performance counter;
it did not change the production scheduler. Private firmware/media copies and
the existing fixed-work runner reached 10,000,000 instructions, 79,822,202
guest ticks and PC `00039119`. The x64 run recorded 10,298,411 capture calls,
5,028 samples and 18,770 counter ticks in 23,397 ms; x86 recorded 10,295,928
calls, 5,027 samples and 23,108 ticks in 27,578 ms. With a 10 MHz counter this
is approximately 0.373 and 0.460 microseconds per sample, inclusive of the
board deadline callback. Deterministic sampling, timer cost, compiler layout,
host scheduling and the startup-only checkpoint limit the estimate.

A separate run sampled the board callback inside the same sampled capture.
The x64 totals were 26,055 capture and 15,618 board ticks across 5,030 samples;
x86 totals were 31,052 and 16,834 ticks across 5,027 samples. Thus board time
was approximately 60% and 54% of the respective instrumented capture interval;
the nested timer also increases the outer interval. The remainder includes
Core timeline, FPU, CPU wait and observation handling, not a proven exclusive
cost for any one function. Board and total are nested and must not be added.
Both runs reached the same fixed checkpoint, which does not prove complete
state equivalence or late-game behavior. These measurements prioritize the
existing board deadline queries and their actual invalidation/observation
contracts; they do not justify caching or skipping callbacks on their own.

Log SHA-256: first capture
`6AC632A0577DEB6F8453DEC036D1D1670FAE87A85632DF89DC0DD14E0E1CA6EF`;
nested capture
`116967B71F264312BC8CF9146FCD124D859BAEEAD81CCF6D7754FCF2D6F1CB8C`.

The same task-only instrumentation also counted whether the board's nearest
nonzero deadline was after the current scheduler advance target. On x64,
9,969,324 of 10,290,494 advancement steps met that condition; on x86,
9,970,847 of 10,292,067 did. This is approximately 97% in this startup
checkpoint. It suggests investigating a bounded deadline query that can prove
an event is beyond the current advance without calculating its exact distant
time. This count alone does not prove a speedup or correctness: public time
observations still require exact deadlines, all device queries and immediate
conditions remain visible, and overflow, event ordering and blocked/L1
progress semantics must remain unchanged. Log SHA-256:
`445388FFB61A87F692BFD8DC20C19C795C3B7661CD90C76CA6753B1D4924F68A`.

An exploratory prototype used the current scheduler target as a temporary
bound, preserving the device deadline queries but avoiding the exact clock
division when overflow-checked integer comparisons proved the event lay
beyond that bound. Public observations must still return the exact deadline.
The prototype used a process-global scratch variable only to estimate the
idea's cost; it is not safe as a production interface and was not qualified
for concurrency, reentrancy, all event ordering or all failure semantics.
All fixed-work runs reached the same instruction/tick/PC checkpoint:

| Architecture | Baseline | Candidate | Candidate | Baseline |
| --- | ---: | ---: | ---: | ---: |
| x64, ms | 21424 | 21262 | 22480 | 20951 |
| x86, ms | 26990 | 26311 | 27760 | 25241 |

The first pair on each width slightly favored the candidate, while the reverse
pair favored the baseline. These noisy startup timings neither prove a gain nor
complete-state equivalence. A correct design would require a bounded query
contract across Core and Board while retaining exact unbounded observations,
immediate events and blocked/L1 behavior. The measured result does not
justify that interface expansion; the prototype was rejected without
production changes. Log SHA-256:
`04EE7E718D432D3B65C5ACDCE03EACD92D377CA290123AEC6DD2A5136974D429`.

### Rejected earlier-deadline conversion shortcut

One possible shortcut was to retain every device deadline query, but avoid
its clock division when a previously found earlier deadline mathematically
proves that the new result cannot win. A scratch-only instrument of the
existing board function counted such opportunities without actually skipping
any calculation. It excluded immediate zero-device-tick cases, checked ratios
and bounded multiplication, and reached the same fixed startup checkpoint.
x64 recorded 10,298,186 conversions, 1,323 with a prior nonzero candidate,
and 1,263 meeting the tested bound. x86 recorded 10,296,252, 1,323 and 1,263.
The opportunity is about 0.012% of these calls on this startup workload, so
adding comparison and overflow logic to every call is not justified. This is
not a measurement of late gameplay or proof of a production implementation;
the idea was rejected without changing production. Log SHA-256:
`F1033D24EFA772AD211AD299F614811779094C82A882DA791E860FDF3E06AB8A`.

## Rejected lexical-preview scan hint

The existing next-instruction preview fetches bytes individually and calls
the same lexical scanner after each fetch. Once the scanner has read the
complete header and determined that displacement/immediate bytes are missing,
a scratch-only candidate returned the required byte count and deferred the
next scan until that count was reached. The byte reads remained sequential;
invalid headers retained the existing scan behavior. This preliminary
prototype changed 11 lines in and 6 out and added one local hint. It was not
qualified for production or subjected to full semantic regressions.

In sequential baseline/candidate/candidate/baseline fixed-work runs, all
reported 10,000,000 instructions, 79,822,202 guest ticks and PC `00039119`:

| Architecture | Baseline | Candidate | Candidate | Baseline |
| --- | ---: | ---: | ---: | ---: |
| x64, ms | 20447 | 21578 | 21024 | 20347 |
| x86, ms | 23759 | 23819 | 23917 | 24874 |

The x64 samples both favor the baseline and x86 is mixed. These measurements
do not establish complete state equivalence or interactive performance; they
do not show a benefit sufficient to justify extra parser control state. The
candidate was rejected and did not change production, tests or APIs.
Task-owned log SHA-256:
`F456A6CF35CDE930F45DC2A21320BA994C2ADC2ADC4560E47B9A831F56AE0556`.

## Candidate: one inlinable prefix classifier

The timing model repeatedly calls the existing pure
`x86_core_cpu_instruction_is_prefix()` defined in a different CPU translation
unit. The classifier is CPU-private, has no caller outside the CPU source, and
already owns classification for execution and lexical paths. A scratch
candidate moved the same implementation to the existing CPU-private header as
`static inline`, removing its former out-of-line definition. This creates no
second classifier, table, state or public interface. All profile checks, V30
prefix behavior and invalid-input handling remain subject to real execution
regression, not just source comparison.

Sequential baseline/candidate/candidate/baseline fixed-work runs reached
10,000,000 instructions, 79,822,202 ticks and PC `00039119`:

| Architecture | Baseline | Candidate | Candidate | Baseline |
| --- | ---: | ---: | ---: | ---: |
| x64, ms | 23006 | 22052 | 22249 | 22763 |
| x86, ms | 26397 | 24695 | 24596 | 25581 |

Both orderings favor the candidate in this startup workload. Scratch EXEs grew
from 3,103,857 to 3,119,146 bytes on x64 and from 3,258,876 to 3,275,188 on
x86. This is approximately 15–16 KiB, not a production artifact measurement.
Equal fixed checkpoints are not full-state equivalence; late gameplay, other
profiles, native UI and receiver qualification remain unproven. No production
source was modified in S32. Log SHA-256:
`AC6DCF47AC1FC090015CAE0F843DA16ED4A400CF8BA86784348DEA83D66D2726`.

S33 may implement the same move and retain it only after actual CPU prefix,
V30, interrupt/fault, timing and preview tests, source/code-size review,
representative workload comparison and the applicable full receiver gate.
Do not change timing accuracy or remove an externally consumed symbol without
first verifying its callers and ownership. This bounded candidate does not
close the PC110 usability objective.
