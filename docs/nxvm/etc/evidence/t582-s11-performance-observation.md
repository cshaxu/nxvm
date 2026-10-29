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

## Earlier plain-RAM route candidate

A task-owned candidate moves the existing plain-span geometry proof before the initial memory resolver for multi-byte reads and writes. Only ranges with no address-line mask, mappings, wrapping or intersecting providers bypass the resolver. Remaining routes use the original resolver; the test candidate is not production and has not proved provider, parity, fault, observer or concurrent transition behavior.

The directly compiled comparison retained the Release `X86_CORE_MACHINE_RUNTIME_TRACE_ENABLED=0` definition. Every 10,000,000-instruction run reached 79,964,294 guest ticks and PC 0003dbc5:

| Host | Order | Baseline ms | Candidate ms |
| --- | --- | ---: | ---: |
| x64 | baseline, candidate | 22196 | 20231 |
| x64 | candidate, baseline | 23388 | 23069 |
| x86 | baseline, candidate | 23478 | 26134 |
| x86 | candidate, baseline | 25799 | 27336 |

The candidate was slower on x86 in both measured pairs; the x64 improvement varied considerably. Equal final checkpoints do not prove memory-route correctness. The candidate is rejected rather than added as another path; no production or Shared code changed. Continue investigating the larger execution cost.

## Completed-observation reset candidate

CPU execution invalidates and clears the completed observation at the start of every refresh. Before a valid observation is published, the copy operation clears and fills it again. A private candidate replaces the first full clear with a reset of `next_lexeme_components`, the only field accessed before the subsequent valid copy; it retains the valid flag and normal construction. Source search found no other production reads of the invalid observation. This is a provisional single-line candidate, not proof of interrupt, debug, fault, halt or early-return equivalence.

At the identical 10,000,000-instruction checkpoint, x64 baseline/candidate took 23303/22644 ms, while x86 took 25751/27821 ms. Each is one ordered pair; this does not show a stable benefit, and the x86 regression disfavors landing it. No production code was changed. Further improvements must be supported by stronger measurements and the complete observation contract.

## Sampled inclusive execution costs

A private x64 fixed-work probe wrapped the actual CPU refresh, timing selection and Board deadline calls and measured one call in every 2048 with the host performance counter. It completed at 10000000 retired instructions, 79964294 guest ticks and PC 0003dbc5 in 19478 ms. Counters also include the brief interval between checkpoint and stopped worker. The sampling is deterministic, perturbs execution, measures inclusive functions with nested calls, and has finite timer resolution; it is not exclusive CPU attribution or a whole-boot result.

The performance counter frequency was 10000000 Hz. CPU refresh: 10008701 calls, 4887 samples, 74120 measured ticks (about 1.52 microseconds per sampled call). Timing selection: 10008697 calls, 4887 samples, 23917 ticks (about 0.49 microseconds). Board deadline: 10304511 calls, 5031 samples, 12941 ticks (about 0.26 microseconds). CPU refresh contains timing selection, so their costs must not be added. Results prioritize investigating CPU instruction execution and timing rather than further isolated Board deadline branches; they do not establish which part can be safely bypassed. Log SHA-256: 798C812C4CBB9EA1ACD2EB7C2A9DB4ABEEA975F68CB8B9C6B7683E59C0F5E035.
A second, more intrusive x64 probe directly instrumented the existing ExecIns call as well as the same outer functions; it required a task-owned CPU source copy and was not a production build. The fixed checkpoint remained 10000000 instructions, 79964294 guest ticks, PC 0003dbc5; wall time was 22488 ms. At 10000000 counter ticks per second, CPU refresh accumulated 91961 ticks over 4883 samples, instruction execution 48432 over 4883, timing selection 27680 over 4883, and Board deadline 17272 over 5027. Approximate sampled inclusive costs are respectively 1.88, 0.99, 0.57 and 0.34 microseconds. CPU refresh contains both execution and timing; calls and sampling are nested, not additive exclusive totals. Compared with the first measurement, the probe adds two calls per instruction and changes compilation/layout; it is useful to rank investigation paths, not as a product speed or gain measurement. Log SHA-256: C96BE1709C7A19BC1C2A7B6C3CE8A50783242A903DD39710C6AFF2D668B9492F.

## Disabled hardware breakpoint candidate

Instruction and data breakpoint matchers currently scan four slots even when all DR7 local/global enable bits are clear. A private candidate checks those existing enable bits and returns no enabled breakpoint after initializing the output. The sole callers use the matched mask only when at least one breakpoint was enabled and matched; TF, RF, software Debug and enabled/mixed slots still require their existing paths. This caller reasoning is not a replacement for the Debug regression suite.

At 10000000 instructions, 79964294 guest ticks and PC 0003dbc5, x64 baseline/candidate took 18687/23183 ms and x86 took 24698/23830 ms. Each is one sequential pair. Opposite directions and large host variation do not show a reliable benefit; no Shared production change has been made. Retain the existing matcher and investigate instruction execution/timing before considering this candidate further.

## Early instruction distribution

A separate task-owned x64 CPU source probe counted each successful primary opcode/prefix fetch during the first fixed 10000000 retired instructions, with private CMOS/media and the existing execution path. It reached 79964294 guest ticks and PC 0003dbc5 in 21030 ms. Counts include a brief post-checkpoint interval before the worker stopped and are not cycle costs. The 21012348 observed bytes included REP prefix F3 5595754 times, operand-size prefix 66 5338818 times and opcode AB (STOS word/dword) 5272818 times. This strongly identifies repeated string stores and repeated prefix classification as startup-specific investigation targets, but does not characterize the later Personaware workload or permit removing per-iteration device, timing, fault, interrupt or debug boundaries. Instrumentation adds a call per fetched byte and changes CPU compilation/layout. Log SHA-256: 4366BBC55F3939552DD320DA0D63D0CC288A9B264628ADB86E38604F59D9D4C5.

## Prefix-classifier inline candidate

A private candidate copied the existing prefix classifier into the timing translation unit as a static inline helper solely for an upper-bound experiment. A clean production design would instead move its one authoritative implementation to a CPU-private header; it must not retain duplicated classifiers. Every pair reached the fixed 10000000-instruction, 79964294-tick, PC 0003dbc5 checkpoint. x64 baseline/candidate were 21827/22271 ms and x86 25113/25981 ms, one sequential pair per architecture. The candidate was slower in both pairs and was not retained. These results do not establish behavior across all prefix forms or later guest workloads.

## Inline prefix count and classifier candidate

A second private candidate moved the unchanged prefix classifier and prefix counter to a CPU-private inline header, then directly compiled the three affected CPU translation units. This preserves one authoritative implementation in the proposed form, but correctness across profiles, V30 emulation, preview and debug was not validated. At 10000000 instructions, 79964294 guest ticks and PC 0003dbc5, x64 baseline/candidate took 24037/23067 ms and x86 took 28105/29520 ms. Probe EXE sizes grew from 3103750 to 3173263 bytes (x64) and from 3261827 to 3338506 bytes (x86). These are single sequential pairs; x86 regressed and x64's apparent gain is within observed host variation. The code-size cost and lack of consistent improvement do not justify a production change.

## Repeated store workload details

A separate task-owned CPU probe counted STOS word/dword handler entries, their repeat/operand-size flags, accumulator value and destination linear addresses. It completed the same 10000000-instruction, 79964294-tick checkpoint at PC 0003dbc5 in 24731 ms, with per-handler instrumentation and private CMOS/media. Of 5272818 STOS entries, 5271896 were repeated; 5160960 were repeated 32-bit zero-value stores. Their observed destination linear-address extrema were 00000000 and 013ffffc, covering the configured 20 MiB address span; the first/last observed instruction locations were 000f618d and 000f603e. These are entry counts and extrema, not proof that every address was uniquely written, every attempted write completed, or that this explains the post-startup slow clock. Do not change the owner's memory configuration or bypass guest writes on this evidence. Log SHA-256: 00C165A0185040B1E9A9F273FE11EEFC36D70671C9C49ADD0167AAE8668A5692.

## Repeated-instruction prefix reuse candidate

A private candidate computes the prefix count once within the existing repeat-timing recording and passes it to the existing continuation check, reusing it for the stored opcode and byte length. V30 keeps its existing NEC path and no persistent state is added. This is distinct from S5's propagation through nominal/memory/decode helpers. Both fixed-work pairs reached 10000000 instructions, 79964294 guest ticks and PC 0003dbc5. x64 baseline/candidate took 20833/21235 ms; x86 took 27032/28397 ms. One sequential pair per architecture is insufficient for a general performance claim, but neither supports landing this candidate. No production code was changed.

## Repeat identity candidate

A second private candidate checks the existing repeat-continuation identity normally, but avoids reassigning identity fields and recopying the already compared opcode bytes when they are identical. It continues updating the 486 data-miss count each iteration, and uses the existing paths whenever identity changes. It adds no persistent state or API. The first fixed-work pairs reached 10000000 instructions, 79964294 guest ticks and PC 0003dbc5. x64 baseline/candidate were 24924/22574 ms; x86 were 26958/28831 ms. These results conflict by host architecture and do not prove Debug, V30, self-modifying code, fault or interruption equivalence. In reverse order, x64 candidate/baseline took 22830/20881 ms and x86 took 25867/25399 ms at the same checkpoint. The x64 direction reversed and the candidate was slower in both x86 pairs. These measurements do not justify production admission; no production code was changed.