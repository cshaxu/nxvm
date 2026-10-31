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
