# T582 S26 — shared 486 timing-term prefix candidate

The 486 source-cost owner already calculates a prefix count, and its nominal,
L2, memory and decode terms calculate it again. A task-only candidate passed
the owner's count to those four existing private terms. There was still one
production calculation path, and the existing nine directly affected test
executables passed on both x86 and x64. Production source decreased by three
lines after normal formatting, but 125 existing test calls needed an explicit
derived argument and a fixture helper. This is a constant-factor reduction:
the instruction prefix sequence is bounded, so no asymptotic complexity
improvement is claimed.

Release builds on the same cached trees were compared using the real PC110
factory, copied CMOS and copied media, and a 10,000,000-instruction endpoint.
Both orders used the same linked libraries; the baseline linked the committed
timing-model source and the candidate linked the proposed model. The model
source and private header were directly compiled for the baseline, which may
affect layout. Results, in milliseconds:

| Host width | Baseline | Candidate | Candidate | Baseline |
| --- | ---: | ---: | ---: | ---: |
| x64 | 23,304 | 22,654 | 22,408 | 23,717 |
| x86 | 26,178 | 26,775 | 27,609 | 26,997 |

All runs reported 10,000,000 instructions, 79,822,202 elapsed ticks and PC
`00039119`. This is an early-start checkpoint, not proof of complete state
equivalence or real-game speed. The x64 sample was faster but x86 was slower
in both ordered comparisons; a previous prototype also had mixed x86 results.
Two pairs do not quantify a stable regression, but neither do they establish
the absence of one. Combined with the extra caller responsibility and test
churn, this candidate does not yet satisfy the owner's requirement that other
properties not visibly worsen. It was withdrawn; no Shared source, interface,
test or product EXE is changed by this experiment. No real-time or RDP
usability claim follows from this comparison. T582 remains active.
