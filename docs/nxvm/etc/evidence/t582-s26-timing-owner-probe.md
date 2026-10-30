# T582 S26 — timing-result ownership probe

The private CPU timing selector has one production caller, which passes its
own `context->timing_result` as the output argument. On the ordinary path the
selector writes the same result both to the context and to that argument.
An earlier conditional attempt was rejected in S16. This separate scratch
candidate removes the duplicate write without adding that conditional; its V30
path writes the context result once. Its unchanged two-argument signature
does not preserve hypothetical separate-output callers, so it is not a valid
general implementation. A production cleanup would instead narrow the private
signature and must preserve the semantics of every CPU profile and failure.

Both variants were built with Release's trace-disable definition. Private
disk images were copied into each run directory and CMOS images were copied
beside the actual probe executable before each run. An earlier attempt did
not place the CMOS beside the executable and reached a different checkpoint;
those results are discarded. All runs below reached 10,000,000 instructions,
79,964,294 ticks and PC `0003dbc5`:

| Width | Order | Baseline ms | Candidate ms |
| --- | --- | ---: | ---: |
| x64 | baseline then candidate | 25,076 | 26,228 |
| x64 | candidate then baseline | 25,829 | 24,968 |
| x86 | baseline then candidate | 34,292 | 29,767 |
| x86 | candidate then baseline | 30,041 | 30,075 |

The order-dependent measurements do not establish a reproducible benefit.
Matching the checkpoint is not complete CPU-state equivalence. This small
cleanup cannot explain the PC110 usability gap, and it is not admitted as a
performance fix. No production code, media or published artifact changed;
no full receiving suite was run for this discarded scratch candidate.
