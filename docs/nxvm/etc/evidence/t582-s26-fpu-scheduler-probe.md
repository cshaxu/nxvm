# T582 S26 — absent-FPU scheduler probe

**Method qualification:** The temporary scheduler was compiled without the
Release build's `X86_CORE_MACHINE_RUNTIME_TRACE_ENABLED=0` definition. That
translation unit includes the private Machine layout, which is conditional on
the definition. Therefore the measurements below are not a valid comparison
with the Release libraries, even though the recorded checkpoint matched.
They must not be used to infer Release performance or semantic equivalence.
No production change was based on them.

PC110 selects the no-FPU profile, but the existing scheduler still calls the
FPU deadline and advancement functions. A task-only build removed both calls
unconditionally to estimate their contribution for that specific machine.
The candidate is not a valid general implementation: it would break machines
with an active FPU. It adds no production path and does not establish that
changing FPU ownership or adding a profile flag would be correct.

Each run used fresh private media/CMOS copies and retired 10,000,000
instructions at 79,964,294 guest ticks and PC `0003dbc5`:

| Width | Order | Baseline ms | Candidate ms |
| --- | --- | ---: | ---: |
| x64 | baseline then candidate | 20,251 | 20,374 |
| x64 | candidate then baseline | 18,112 | 19,353 |
| x86 | baseline then candidate | 23,105 | 22,420 |
| x86 | candidate then baseline | 24,899 | 22,758 |

The x64 candidate did not improve either pair; the x86 pairs suggest a
possible small benefit but do not isolate host variation, compiler layout or
other workloads. This is not an exact upper bound on call cost, a complete
state-equivalence test, or a PC110 usability measurement. Adding a second
FPU-presence state or special scheduler path is not justified by these results.
The original scheduler and all FPU behavior remain unchanged. The receiving
suite was not rerun because no production source or executable changed.
