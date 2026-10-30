# T582 S26 — SMM write-path lookup audit

The prior World Clock observation recorded 27,652 SMM entries in about 60
seconds. The current SMM entry writes the 512-byte save image in 128 ordered
four-byte physical operations; RSM reads 128 ordered operations. If each entry
has a successful matching RSM, this implies about 7.08 million physical
operations over that interval. The recorded entry counter does not itself
prove every RSM succeeded or measure host time spent in these operations.

On an 80486 write, `_kma_write_physical` looks up a cache way before calling
`_kma_write_bus`; the latter performs another lookup after the write and
external-cycle callbacks, to publish the write-buffer cache-hit fact. It is
tempting to pass the first result directly. That is not established equivalent:
the callback boundary permits changes to the memory view, and existing
`x86_core_cpu_execution_invalidate_memory_view` invalidates cached lines.
Machine device writes can also notify the CPU through the snoop path. The
normal CPU-owned transaction filters its own write from that snoop, but this
alone does not prove that every provider or callback preserves the cache view.

## Instrumented World Clock observation

A task-only x64 build counted SMM writes and compared the pre-write cache way
with a fresh lookup immediately after the bus callback. It used private media
copies and the existing machine input probe; the final frame was inspected and
shows World Clock. The program exited successfully after 372.89 seconds.
Between the nearest logged observations to samples 30 and 36, host time
advanced 60.764 seconds and guest ticks advanced 190,274,122 (about 5.77
seconds at 33 MHz). The SMM write counter increased by 6,165,077; its
pre-write cache-hit and changed-way counters did not increase. Across the
whole run, the last sampled totals were 29,489,259 writes, 56 pre-write hits
and zero changed ways. These are lookup observations, not a measurement of
lookup CPU time; the extra comparison, frame output and sampling perturb
throughput. This path bypasses native Window/RDP input.

The observation identifies a frequently executed path but does not establish
that reusing a result across arbitrary callbacks is correct or that it would
materially improve throughput. The candidate remains unadopted. Any follow-up
must establish callback mutation behavior and preserve the individual bus
operations, faults, write-buffer accounting and SMM timing. No CPU, timing
contract, public interface, test or executable changed for this probe.

SHA-256 (guest log, final PPM):
`8320D55A2640619BEAE0602B580B42CC377AA9C7A3B6CBE9D26C90DDF642807E`,
`54E10943DB0E7DCEF309B9973AB2BB667128781D9C04E7E502BA7B43A5E2D9A6`.

## Scratch upper-bound comparison

A task-only candidate skipped the post-write cache lookup when SMM was active
and reported a miss instead. This is deliberately not a generally correct
implementation: even the instrumented run observed SMM pre-write hits, and a
callback may change the view. It is an upper-bound performance experiment,
not a proposed product special case. At a fixed 10,000,000-instruction private
startup checkpoint, baseline/candidate/candidate/baseline elapsed times were
18,071/19,228/19,332/19,187 ms on x64 and 25,277/23,019/22,800/24,121 ms on
x86. Every run reported 79,964,294 guest ticks and PC `0003dbc5`. Checkpoint
equality does not establish complete-state equality. These sequential samples
suggest a possible x86 effect but no reliable x64 improvement; they do not
measure World Clock or prove gameplay acceleration. This result does not
justify an incorrect SMM special case or a new cache state mechanism. No
production code or artifact changed.
