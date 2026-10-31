# T582 S35 — cost attribution in progress

The current owner goal is interactive PC110 operation, not a smaller CPU
structure alone. The qualified S34 dispatch change reduced code and instance
space but its x64 Game comparison was 268.05/268.06 seconds: it has not
demonstrated a throughput improvement.

Existing observations give the following investigation order, not mutually
exclusive costs or an exact decomposition of the present build:

1. **CPU instruction execution and timing.** The S20 World Clock probe found
   refresh represented roughly 65% of measured Machine-run intervals; inside
   sampled refresh, instruction execution was about 55.6% and timing plus
   observation 29.1%. These nested estimates were collected in different
   runs with instrumentation and cannot be summed or treated as exact.
2. **Instruction initialization and fetch.** The S34 Game-marker probe
   estimated roughly 12.4% of Machine run for `ExecInit`, including about
   7.4% in its logical prefetch reads. The latter is nested and may include
   recursive reads. Fetch still owns paging, bus, timing and faults; it is not
   a plain memory copy. These are x64 samples with few enclosing run samples.
3. **Machine clock and board work.** S13 saw about ten million board before
   and after advances each at a ten-million-instruction startup checkpoint,
   with approximately 61 million domain-advance calls. Published time and
   board deadlines were sampled separately on both widths; their nested times
   cannot be added. A stable previous deadline does not prove safe batching.
4. **Memory routing and device activity.** S34 Game-marker cache lookups
   observed both hits and misses frequently. These are counts, not elapsed
   time. S34 sampling identified memory route and PIC leaves in different
   phases but does not explain the complete gap to PC110-EMU.

The synthetic CPU-only versus full-Machine comparison is a useful direction,
not a subtractable cost breakdown: the two use different bus, initialization,
cache and timing contracts. Likewise, the reference emulator does not provide
an equivalent instruction-timing and device-observation contract. Neither
comparison proves a single culprit or a safe shortcut.

After the S34 dispatch consolidation, the existing synthetic loop was
relinked against the current cached graph and rerun sequentially on the same
host. For ten million NOP/JMP instructions the x64 CPU-only, full Machine and
PC110-EMU samples took 3,838, 8,582 and 818 ms respectively; x86 took 6,731,
11,821 and 789 ms. The Machine reported 40,000,014 ticks and PC `000f0001`;
the standalone fixture and reference have the different contracts described
above. This is one sample per width, not an end-to-end PC110 benchmark, and
does not isolate Machine overhead by subtraction. It confirms the broad
performance gap persists after S34 and makes a large dispatch-only improvement
unlikely on this workload. Synthetic output SHA-256, x64 then x86:
`656261D671DFC9E9965ADBF69FAAFBEBEED49B8272146782730739DC33AC9469`,
`7DC7D44113152DF98DB4990E293F3A3AE6B3BA0DD96B8B751C94BDC49AE4E653`.

The existing [synthetic ablation](t582-s26-pc110-emu-measurement.md#synthetic-loop-timing-and-publication-ablation)
offers a more controlled, but deliberately non-equivalent, comparison within
the same Machine harness. On x64 its ten-million-instruction baseline was
8,603 ms; replacing the timing selector with a constant gave 4,993 ms, while
suppressing time publication gave 7,863 ms. The former changed guest tick
totals and the latter eliminated them: neither is a valid implementation or
an exact exclusive cost measurement. One ordered sample and a NOP/JMP loop
also do not predict Game speed. The result nevertheless prioritizes examining
the existing timing selector over assuming board time publication explains
the entire synthetic Machine overhead. The previously tested prefix-sharing
variant did not establish a reproducible cross-width or Game gain; it should
not be repeated unchanged.

The next investigation must find a shared calculation or observation whose
work can be reduced at its existing owner, then compare equivalent guest work
on both architectures without simultaneous test load. Previous negative
experiments in the [S26 index](t582-s26-investigation-index.md) must not be
repeated without a new hypothesis. Desktop/RDP responsiveness, complete game
state equivalence and an order-of-magnitude improvement remain unverified.

## Prefix-count inlining experiment

The S33 prefix-byte classifier is already inline; the surrounding prefix-count
function still has callers across three CPU translation units. A scratch-only
candidate moved that single function body to the existing private header as
`static inline`, without changing its algorithm, profile conditions or call
sites. Both variants directly compiled the same three units and used private
CMOS/media copies, with baseline/candidate/candidate/baseline order. At the
10,000,000-instruction / 79,964,294-tick / `0003dbc5` checkpoint, x64 times
were 19,224 / 17,936 / 20,275 / 16,979 ms; x86 times were 23,749 / 22,312 /
23,233 / 24,230 ms. The executable increased from 3,092,997 to 3,147,707
bytes on x64 and from 3,256,457 to 3,317,822 bytes on x86.

The x86 ordering favors the candidate, but x64 changes direction and the code
size grows. These sequential fixed-startup samples are subject to host noise;
matching PC and ticks does not establish complete state or Game behavior.
There is insufficient evidence of a cross-width benefit to justify the extra
machine code, so the candidate was not applied or submitted for the full
receiver. Production source and published EXEs are unchanged by this probe.
The combined log SHA-256 is
`7140129A697F2D18C9FAAB9E98F1B95299F28912CC85F7361225A278F04DE551`.

## Same-workload CPU and time-publication samples

A task-only link wrapper measured the existing CPU refresh and Machine elapsed
time publication in the same fixed-work PC110 startup, one call in 256, without
changing either operation or guest inputs. The x64 run completed in 18,387 ms
at 10,000,000 retired instructions, 79,964,294 guest ticks and PC `0003dbc5`;
x86 completed the same checkpoint in 22,890 ms. Calls / samples / accumulated
host TSC units were:

| Width | CPU refresh | Time publication |
| --- | --- | --- |
| x64 | 10,003,583 / 39,076 / 91,651,994 | 10,009,700 / 39,100 / 42,969,673 |
| x86 | 10,006,398 / 39,087 / 111,106,180 | 10,012,524 / 39,111 / 54,433,005 |

Mean sampled CPU intervals are approximately 2.13 times publication on x64
and 2.04 times on x86. These are inclusive intervals at wrapped link
boundaries; they may nest and cannot be added or directly converted to a
whole-machine percentage. Deterministic sampling may alias guest loops,
non-serialized TSC and scheduling can distort results, and the startup
checkpoint is not Game or full-state equivalence. This reinforces examining
both CPU work and publication rather than attributing the entire gap to just
one. No runtime production code or asset changed. Combined log SHA-256:
`D4820C086D2A1A2DC53DD30736469D6E8C744A35B3FFDD4B3EC1530C3EDBF59F`.

## Timing-only LTO experiment

A scratch compiler experiment applies GCC fat LTO to only `cpu_timing.c` and
`cpu_timing_model.c`, without changing the runtime executor, guest timing or
production build. The same two files were separately compiled without LTO for
the baseline. At the fixed ten-million-instruction PC110 startup checkpoint,
baseline/candidate/candidate/baseline milliseconds were 20,866 / 18,895 /
19,740 / 20,400 on x64 and 22,085 / 21,711 / 23,174 / 22,833 on x86. All
reported 79,964,294 guest ticks and PC `0003dbc5`; this does not establish
complete state equivalence. x64 favors the candidate in both orders, while
x86 changes direction. Diagnostic link time was 1.20 versus 6.40 seconds x64
and 1.08 versus 7.28 seconds x86; the candidate executables were slightly
smaller. Harness compilation also used the selected LTO flag, so the result
does not isolate only the two timing files. The full-frame Game pair follows
below. These are exploratory measurements, not an adopted build
policy or evidence of interactive usability. Fixed-work log SHA-256:
`7D95E3A4B40D81E10F82EC1F1827A22E64EC403C1092D1AADC6DFF97C366736C`.

The x64 full-frame Game candidate and then baseline exited normally in 259.02
and 269.93 seconds. Each produced the same stable frame SHA-256
`E5C1AB80C00F84C1B80C1069EFA58FD01D761FDE7AC07F7982F0B0B34AB0DE99`;
the first complete frame appeared 32 and 31 one-second samples after Enter.
This single ordered pair suggests a possible roughly four-percent startup
improvement, not a proven repeatable or interactive-play improvement; the
matching frame is not a complete guest-state comparison. x86 Game and native
desktop/RDP input were not tested. Guest logs SHA-256, candidate then baseline:
`AAEF56C6ECE29D0FEED213362621E4FF42EB9A2D31DDC29C435FE653502E1F62`,
`6E03163F5D8921BDA92A65D69ABC22ED88585DC6C41BD2D1B9E77ECAB9756BA7`.

The existing CPU archive contains both timing and other CPU objects; enabling
LTO on these two objects would affect all consumers that pull them, including
the many independent CPU tests, unless the build also introduces a separate
native archive or consumer-specific opt-out. The scratch product link was
five to six seconds slower and the x86 fixed-work samples disagreed. Given
this added build cost and complexity, the limited single-pair evidence and the
still roughly 259-second startup, this is not adopted as the usability fix.
No production build setting, source, user media or released executable was
changed.

## Idle FDC seek scan experiment

The FDC advance performs a readiness pass over four seek slots followed by a
due-seek pass even when none was pending. A scratch candidate tracked whether
the first pass saw any pending seek and skipped the second only when it saw
none. If a seek was pending, the original readiness, callbacks and due pass
remained; the ordinary absolute-time update was not skipped. This candidate
was not subjected to complete reentrancy or FDC correctness qualification.

In the private-media fixed-startup checkpoint, ordered baseline/candidate/
candidate/baseline times were 18,683 / 19,336 / 18,477 / 18,019 ms x64 and
23,594 / 23,940 / 22,110 / 22,600 ms x86. All reached ten million
instructions, 79,964,294 ticks and PC `0003dbc5`. The x64 result favors the
baseline and x86 changes direction; these samples show no reliable product
benefit and matching checkpoints do not prove semantic equivalence. Native
disassembly confirmed that the candidate changed the linked FDC function.
Given the extra branch and no demonstrated gain, the candidate was discarded.
This startup probe does not characterize Game or desktop use. No production
code, owner media or published artifact changed. Combined log SHA-256:
`7810303FCCFD3A656E8E9D6A398F6A0D076BCD2BB274361B9396F12A952F142C`.
