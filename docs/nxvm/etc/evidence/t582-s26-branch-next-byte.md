# T582 S26 — timing branch next-byte candidate

The generic timing-input collector calls the existing branch classifier for
every instruction. That classifier currently reads the next byte before
checking whether the opcode needs it. A task-only candidate deferred this read
to the `0F` and `FF` forms, retaining the existing short-branch and direct
branch rules. This adds a branch and source lines; it is not an asymptotic
improvement. The temporary candidate was not qualified for production.

For the private-media 10,000,000-instruction startup checkpoint, sequential
baseline/candidate/candidate/baseline times were 21,341/23,754/23,036/24,013
ms on x64 and 28,150/28,074/24,711/25,229 ms on x86. All runs reported
79,964,294 ticks and PC `0003dbc5`. The order-dependent timings do not show a
reliable gain, and the endpoint does not prove full CPU-state equivalence.
No World Clock or interactive measurement was performed for this candidate.
It was not adopted; no production source, test, executable or media changed.
The next investigation must address a larger component of execution cost,
rather than assume this single conditional fixes PC110 usability.
