# M5 T593: OmniBook 300 execution and interaction performance

The owner admitted this implementation task on 2026-11-21 from the queue
head after [T592 product-stage closure](M5-T592-ob300-product-qualification.md).
The admission baseline is `597aad29d` and the published 0.5.0592 pair.
[The proposal](../proposals/m5-ob300-performance.md) owns scope and budget
requirements, and [CURRENT](../states/CURRENT.md) owns active status.

S1 starts with the [controlled ROM DOS results](../etc/evidence/t591-s4-ob300-dos-baseline.md)
and the [native DOS and Windows observations](../etc/evidence/t592-s3-ob300-local-setup.md).
The admission makes no new timing or hotkey claim. The retained T592 scratch
includes prior probe inputs and output. Review their ownership before cleanup
and copy only the needed diagnostic source into the S1 scratch. Use current
external masters and private disposable media for any new replay.

## S1 evidence

[Execution-cost baseline](../etc/evidence/t593-s1-ob300-execution-baseline.md)
records the repeated dual-width ROM DOS measurements and the x64 original
Windows power replay. CPU execution and timing dominate the DOS profile.
The next largest region is time publication, including the board scan and
device advance. The x64 probe separately observed the original BIOS wait
after Ctrl+Alt+O and left the actual panel/RDP feedback unresolved.

## S2 evidence

[Indexed timing and exact clock qualification](../etc/evidence/t593-s2-ob300-indexed-timing.md)
records the two retained changes, their isolated and guest timings, the
dual-width test matrix and all 16 artifact identities. The 0.5.0593 PC
releases are the S2 delivery. The original performance thresholds remain
open for the rest of T593.

## S3 evidence

[Clock and routing microbenchmarks](../etc/evidence/t593-s3-clock-microbench.md)
and [release qualification](../etc/evidence/t593-s3-release-qualification.md)
record the four retained simplifications, the rejected routing variants,
all sixteen deployed identities and the dual-width results. Functional
qualification passed. The MyOB300 probes still miss the original DIR gate
on both widths. S3 is delivered for review, with the usability target open.

## S4 evidence

[Timing dispatch proof](../etc/evidence/t593-s4-cpu-timing-dispatch.md)
and [release and usability qualification](../etc/evidence/t593-s4-release-qualification.md)
record the bounded call reduction and unchanged timing contracts, the full
dual-width receiving matrix and one original DOS qualification per width.
The remaining performance gates and scope decision stay with T593.
