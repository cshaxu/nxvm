# T582 S11 — preliminary delivered-version performance observation

Using Shared 96eab1131 and the current public PC110 factory, each run used private copies of CMOS and disks with overlay access. The probe enters the selected DSTN/TFT mode and runs for three ten-second samples. It uses the existing execution and RTC observation paths, with a link-time quantum reporting wrapper. These are sequential single runs, not a variance study, a native Window/RDP test or a complete startup measurement.

| Host | Display | Wall seconds | Last sampled guest ticks | Last sampled instructions | Initial/final RTC seconds |
| --- | --- | ---: | ---: | ---: | --- |
| x64 | DSTN | 32.41 | 104440275 | 14482041 | 00 / 03 |
| x64 | TFT | 32.27 | 105942608 | 14666087 | 00 / 03 |
| x86 | DSTN | 31.85 | 93355548 | 12583784 | 00 / 02 |
| x86 | TFT | 31.81 | 93322651 | 12578630 | 00 / 02 |

All four probes exited zero and sampled 80x25 text. Last quantum samples are not exact end-of-run totals. At the configured 33 MHz guest axis, the sampled totals represent approximately 2.8–3.2 guest seconds; the RTC observations are consistent with slow guest progress. They do not prove a specific RTC bug, exact performance gain relative to S1 (different instrumentation), or final Personaware timing. The S10 cached x64/x86 full qualification and fourteen artifact hashes remain recorded in [S10](t582-s10-pit-deadline.md).

A separate 30-second x64 DSTN thread-IP sample exited zero, with 3231 in-image and 13964 outside-image observations. It sampled multiple threads and is not a CPU-time attribution. Leading in-image buckets were CPU execution refresh 193, memory route resolution 159, Core run 94, CPU timing selection 89, elapsed-tick publication 86 and 486 memory penalty 86. These are investigation leads, not proof that any removal is correct or worthwhile. Profile log SHA-256: FF9434DF878537C36900596612CCA9D27D6265BD20269C74C4D4D7A99449433A.

The requested usable-performance target remains unmet. Continue profiling and validate any equivalent optimization with matched measurements; do not mark T582 complete or claim interactive acceptance from these observations.
## Clock-domain no-carry candidate

A task-owned candidate returns zero before the general division when a sub-denominator source step plus existing phase cannot yet reach one device tick. It does not change phase ownership or introduce a second clock. A 500,000-case deterministic mathematical comparison covered random ratios and boundary values, but is not an execution proof of the C implementation. Production code was not changed.

All four fixed-work paired probes used the same cached Release libraries and private inputs, and ended at 10,000,000 instructions, 79,964,294 guest ticks and PC 0003dbc5:

| Host | Order | Baseline ms | Candidate ms |
| --- | --- | ---: | ---: |
| x64 | baseline, candidate | 20115 | 19585 |
| x64 | candidate, baseline | 21493 | 19427 |
| x86 | baseline, candidate | 23623 | 24783 |
| x86 | candidate, baseline | 25251 | 24047 |

The x64 pairs favour the candidate, but x86 changes direction between pairs. This small, early-workload experiment does not demonstrate stable improvement on both architectures or resolve the order-of-magnitude usability deficit. Do not land the additional branch on these data; seek larger costs and measure complete workloads. Equal terminal PC and ticks do not prove complete timing or side-effect equivalence.