# T593 S3 exact RTC and shared clock microbenchmarks

## RTC arithmetic

The candidate shares elapsed quotient and remainder across the second and
periodic accumulators. Short advances below one second require no elapsed
division. Each normalized phase is a sum of two values below the divisor,
so one subtraction is sufficient. A latched PF bit suppresses only edge
detection work. Periodic phase still advances and the original IRQ
refresh and calendar/alarm update sequence remains in place.

The same scratch harness linked original and candidate chip sources
separately with GCC Release flags. Its deterministic operation stream
applies 25,000 steps at each of four tick rates. It varies divider and
mode writes, resets, C-register observation/clears, short and multi-second
advances. Intermediate calendar fields, both phases, C flags, square wave,
queried deadlines and ordered IRQ edges matched. Both existing RTC chip
smokes passed per architecture, including the SET-mode maximum elapsed
case in the contract test.

Seven alternating baseline/candidate pairs supplied the following medians.
Each timed sample advances four million times with an identical loop and
periodic status reads. Final phases and IRQ checksums matched.

| Width | RTC ticks/s | Baseline | Candidate |
| --- | ---: | ---: | ---: |
| x64 | 32,768 | 71.641 ms | 34.490 ms |
| x64 | 1,000,000 | 71.750 ms | 33.245 ms |
| x86 | 32,768 | 168.684 ms | 43.542 ms |
| x86 | 1,000,000 | 167.083 ms | 55.632 ms |

The timings apply to advancement of the chip. An actual machine divides
source ticks into RTC ticks, so machine-wide acceleration is smaller.

## Board and Core regression

The Core publication benchmark links real PIC, PIT, RTC and Core scheduling.
Each run takes two million source-tick advances per row. Both rows use
the MyOB300 primary PIT and RTC ratios. One row also enables an auxiliary
PIT to preserve an exercised installed-device path. The original and
candidate objects use identical Release trace configuration. Checksums of
elapsed time, next deadline, status flags and PIT outputs matched.

| Width | Auxiliary PIT | Baseline | RTC only | Board only | Combined |
| --- | --- | ---: | ---: | ---: | ---: |
| x64 | Absent | 487.673 ms | 462.313 ms | 458.016 ms | 479.653 ms |
| x64 | Installed | 657.407 ms | 668.893 ms | 634.970 ms | 656.264 ms |
| x86 | Absent | 843.713 ms | 841.283 ms | 807.367 ms | 865.921 ms |
| x86 | Installed | 1061.127 ms | 1128.056 ms | 1116.787 ms | 1104.862 ms |

Five alternating passes give these medians. A second x86 control with nine
passes confirmed a combined slowdown with the auxiliary PIT installed.
The median went from 1.055 s to 1.158 s. RTC-only was 1.029 s and
board-only 1.038 s. The optional-device filtering and PIT minimum
short-circuit remain scratch candidates. Their semantics matched, and the
combined cost is unresolved. An earlier board run with asymmetric trace
configuration is outside these results.

Turbo returns before computing the unused host pacing observation. Core
reports timeline pending count from the active heap size. Existing scheduling and
trace tests still exercise the enabled tracing path. The audit's suggested
trace argument guard was dropped because the Release macro already avoids
evaluating the argument.

Both cached Release trees built the initial combined files and passed the same 15
targeted cases covering RTC, optional PIT wiring, timing, timeline, trace,
disk topology, pacing, Core execution and MyOB300 board assembly. Both
changed source manifests verify. After withdrawing the optional-device
prototype, the reduced worktree again passed all 15 cases per width.
The [release qualification](t593-s3-release-qualification.md) records the
receiving suite, product publication and two DOS replays. No new CTest wall-time gate was introduced.

## Core one-tick advancement

An attachment deadline is an observation. Its flags and next positive
deadline can split a step only when part of a multi-tick interval remains.
For one remaining source tick, Core still advances the one timeline,
arbitrates refresh and DMA, and invokes the same internal and attachment
callbacks. It omits the query whose result cannot affect that tick.
All production deadline callbacks were reviewed for observational behavior.
The existing scheduler test now checks that the scheduled event and device
callback still arrive at the expected tick with immediate, L1 and blocked
flags asserted. Explicit deadline observations remain unchanged.

Five alternating Release comparisons of two million advances per row
compare the scheduler objects with the same compiler flags and the same
RTC, PIT and PIC logic. Each original/candidate pair produced identical
elapsed time, deadline, peripheral status and output checksums.

| Width | Advances | Auxiliary PIT | Original | Candidate |
| --- | --- | --- | ---: | ---: |
| x64 | One tick | Absent | 422.055 ms | 235.493 ms |
| x64 | One tick | Installed | 503.288 ms | 279.035 ms |
| x64 | 1–8 ticks | Absent | 477.453 ms | 451.363 ms |
| x64 | 1–8 ticks | Installed | 690.336 ms | 650.135 ms |
| x86 | One tick | Absent | 756.020 ms | 478.635 ms |
| x86 | One tick | Installed | 810.492 ms | 502.597 ms |
| x86 | 1–8 ticks | Absent | 780.241 ms | 752.002 ms |
| x86 | 1–8 ticks | Installed | 1119.399 ms | 995.150 ms |

These results are from source-tick publication, not the whole CPU
instruction loop. The machine-wide impact depends on the frequency of
one-tick steps or one-tick tails after longer advances.

## Core memory routing controls

A private one-pass provider prototype collected machine-decoded geometry
while retaining raw callbacks first, selected RAM aliases second and other
provider queries third. It removed one geometry pass and two net source
lines, and passed seven existing contracts per width. It was rejected
because the isolated Am386SXLV preview became slower in x64 and was
effectively unchanged in x86.

A separate geometry proof avoids multibyte lane resolution for ordinary
RAM below the lowest bit cleared by the active mask and disjoint from all
RAM mappings and device ranges. It uses no persistent cache. Eight existing
contracts passed on each width, including A20 crossings and 486 cache fill
and provider checks. Nine in-process alternating control pairs showed Am386SXLV with both a
mask and a disjoint mapping improving from 30.369 to 14.307 ms in x64.
The same x64 test with neither went from 11.243 to 12.829 ms. The x64
unmasked/no-mapping slowdown appeared in all five tested profiles, from
9.4 to 15.8 percent. This narrows the useful proof but rejects the current
implementation. Two follow-up proofs also left a measurable x64 cost
on some already-plain profiles. No memory route change is retained in this
batch. The plain case describes a contiguous guest RAM span and is available
to both host executable widths.

| Transcript | SHA-256 |
| --- | --- |
| RTC states, IRQ traces and timings | `6ae484b477241d66654823b2f7d503a552236c34a4420782db20d3cca50f3a1d` |
| Single-tick scheduler state and timings | `16ea8eeaf4b3d7db8488b387cea7fb0ff92eef424c8822b7127fe3512e238c15` |
| Board publication states and timings | `a568e474f9ab8cd8b36dd8d9031e7e41e883fe1f07ece4f5f6b3675c90bb8a00` |
| Nine-pass x86 board control | `b820f9d06fcea249060acc1a47d9d6bac699c3b4ae8e1ac0441de845189f932e` |
| In-process Core routing control | `b0f7bf94789394214a528abad00ce1d77e308d583cb110876bee133aaa974d6c` |
| x64 reduced batch CTest | `7647400525dd9c25593d303ada77079a8d6d2e82f9f1e88403bcd64b9fa6fd3b` |
| x86 reduced batch CTest | `7206833332250be1e21330b8a37336220e06c2f184a487deddfedaabe15679e9` |
