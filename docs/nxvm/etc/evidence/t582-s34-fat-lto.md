# T582 S34 — fat LTO feasibility probe

The prior whole-Core LTO experiment slowed each Core-linked test link from
under one second to over twenty seconds. This scratch experiment compiled the
same CPU and Machine sources as GCC fat LTO objects, allowing existing test
links to use their native portions with explicit `-fno-lto`, while a selected
product link uses LTO. This is a compiler experiment, not a second runtime
executor. Nothing in the production CMake or product sources was changed.

An initial candidate accidentally compiled Machine with runtime tracing enabled
while the Release baseline disabled it. Its timing results are invalid and
excluded. The corrected objects explicitly disable runtime tracing like the
Release target. Two existing x64 and x86 CPU/Machine tests linked against the
fat archives with `-fno-lto` in roughly 0.4 seconds and passed. Omitting
`-fno-lto` caused an x64 test to invoke LTO and take 21.45 seconds. Thus the
fast-test behavior is not automatic and any real design must account for every
consumer without introducing a dependency on App names into Core.

At the fixed 10,000,000-instruction PC110 checkpoint, baseline/candidate/
candidate/baseline elapsed times were 17,656/15,799/15,505/16,436 ms on x64
and 22,287/22,807/22,334/24,042 ms on x86. All reported 79,964,294 ticks and
PC `0003dbc5`; this is not complete-state equivalence. Product-path links
were 1.10/24.49 seconds on x64 and 1.89/28.74 seconds on x86. The x64 result
is promising, while the x86 result and general build impact are inconclusive.
The current x64 complete-frame Game comparison used candidate then baseline,
with separate private copies of the same media. Both exited normally and
produced the same stable-frame SHA-256
`E5C1AB80C00F84C1B80C1069EFA58FD01D761FDE7AC07F7982F0B0B34AB0DE99`.
Candidate/baseline total times were 243.53/269.54 seconds, and the complete
frame appeared 29/30 one-second samples after Enter. This one ordered pair
suggests a bounded startup improvement; it does not establish equal complete
state, reproducibility, x86 Game performance, native Window/RDP input or
interactive playability. The prior S26 Game pair used a different temporary
build and is not a substitute for this result.

An x86 pair then ran baseline before candidate using the same frame
criterion. Baseline/candidate completed in 346.20/315.93 seconds; the stable
frame appeared 41/40 one-second samples after Enter. Both frame hashes were
`E5C1AB80C00F84C1B80C1069EFA58FD01D761FDE7AC07F7982F0B0B34AB0DE99`.
This is again one ordered pair, with separate guest runs and possible host
variation, not full-state equivalence or native desktop qualification. It
suggests that the startup benefit is not confined to x64, but both runs remain
several minutes long and do not establish acceptable usability.

Applying this globally would require every non-product consumer to opt out of
LTO or incur very slow links. An opt-out propagated by the Core archive could
also disable LTO requested by an unrelated consumer, while identifying a
particular App inside Core would violate dependency direction. A production
solution must resolve those tradeoffs without separate production executors,
unrelated component regressions, or app-specific knowledge in Core. This
experiment does not yet establish such a solution, so the formal build stays
unchanged and the PC110 performance goal remains open.

## Native archive extraction probe

An additional task-only experiment ran the matching GNU `objcopy` on each fat
archive, removing `.gnu.lto_*` sections without recompiling the source. On
x64 the CPU/Machine extractions took 0.31/0.37 seconds; on x86 0.25/0.33
seconds. Two existing CPU/Machine tests per width then linked against the
stripped archives without `-fno-lto` and passed; links took 0.47/0.80 seconds
on x64 and 0.76/0.70 on x86. These are two tests per width, not a full
standalone or receiver qualification.

A scratch x64 product-path link placed each fat archive before the respective
stripped native archive. Its linker map listed members from the fat archives
and only `LOAD` entries for the stripped archives; it completed in 27.12
seconds. This supports feasibility for that specific link order, but it does
not prove CMake will maintain that order for every product, linker or standalone
consumer, nor establish full correctness of the generated archives. It would
require additional generated archives and build-graph ownership, GNU-specific
tool discovery and fallback, dependency and incremental-rebuild checks, and
qualification of product LTO separately from normal unit-test code. Given the
limited single-pair evidence, approximately 26-second product-link penalty and the
still-slow measured Game startup, this is not yet a sufficiently simple,
qualified solution to the user's usability objective. The normal build remains
unchanged; no runtime fallback or second CPU execution path has been installed.
