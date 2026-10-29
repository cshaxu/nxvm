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

At 10000000 instructions, 79964294 guest ticks and PC 0003dbc5, x64 baseline/candidate took 18687/23183 ms and x86 took 24698/23830 ms. In a reverse-order pair, x64 candidate/baseline took 20281/20136 ms and x86 took 25765/25381 ms at the same checkpoint. Both reverse-order candidates were slightly slower; the two ordered pairs do not show a reliable benefit. No Shared production change has been made. Retain the existing matcher.

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

## Delivered-version late workload

A separate current-library x64 DSTN public-factory probe with private copied CMOS/overlay media ran 30 ten-second samples and exited zero. Its last captured 640x480 frame SHA-256 `85a35680232ca0ad623db7dd18d23a79396931acb133791581d40c37048aef41` matches the previously accepted Personaware frame. RTC moved from 00 to 24 seconds while approximately 300 host seconds elapsed. Thread IP samples from host seconds 5–25 and 220–240 contained 6341 in-image observations, 18112 outside-image observations and 10752 failed reads; they span multiple threads, are perturbed and do not represent exclusive CPU time. CPU refresh and memory route resolution remained leading individual in-image buckets in both windows.

An independent, instrumented 250-second run counted successful primary opcode/prefix fetches. The 0–25 second window included 22544384 fetches, of which F3=5595754, 66=5338818 and AB=5272818. The 220–240 second window included 10616832 fetches, led by E2 (LOOP)=2452564, F3=747853, D1=626158, 74=547441 and A4=441319. Window changes are selected from host time checked every 65536 fetches. This sampling changes execution cost and is not a direct proof of guest time, exact instruction retirements, continuous steady state, causality or interactive acceptance. RTC moved from 00 to 20 seconds in approximately 253 host seconds. Output log SHA-256 `AE971862E97495DA5D17B420A11A2ED9CB4D5757A88B3DE3EBC8738E00B88DD8`.

## Fixed-size RAM copy candidate

A task-owned candidate replaced the existing RAM copy call in both read and write with one shared private dispatcher, using constant-size `lib_memory_copy` for 1, 2, 4 and 8 bytes and the original call for other lengths. This retained the existing route, parity, snoop and observer paths in the prototype; their full correctness was not separately proven. The same 10000000-instruction, 79964294-tick, PC 0003dbc5 fixed-work checkpoint took 21020/21560 ms baseline/candidate on x64 and 24325/25337 ms on x86. These are single sequential pairs, but neither showed a benefit. The extra dispatch is not admitted; production remained unchanged.

## CPU translation-unit link-time optimization experiment

A task-owned probe compiled the same three CPU translation units (`cpu_instructions.c`, `cpu_timing_model.c`, `cpu_timing.c`) plus the fixed-work harness with GCC Release flags, comparing ordinary linking to `-flto=auto`; all other libraries remained the same non-LTO cached Release inputs. This is not a production configuration or full-program LTO. Each pair retired 10000000 instructions at 79964294 guest ticks and PC 0003dbc5.

| Host | Order | Baseline ms | LTO ms |
| --- | --- | ---: | ---: |
| x64 | baseline, LTO | 21026 | 19292 |
| x64 | LTO, baseline | 19952 | 18626 |
| x86 | baseline, LTO | 23623 | 22770 |
| x86 | LTO, baseline | 24399 | 23568 |

These two small sequential pairs suggest a bounded benefit, not real-time qualification or a whole-product result. Probe EXE sizes grew from 3103750 to 3158169 bytes on x64 and from 3261827 to 3272060 bytes on x86. The linker reported 11 serial LTRANS jobs; a production decision must measure ccache compatibility, incremental and clean build costs, toolchain portability, semantics and full receiver tests. Fixed-work checkpoint equality is not proof of full behavioral equivalence. No production build configuration changed.
A separate cache probe compiled the same task-owned LTO source twice with the repository cache. Both x64 and x86 recorded one miss then one direct hit; this proves only that a small LTO object can be cached, not that relinking CPU products is cheap. The linker reported 11 serial LTRANS jobs even when the test requested eight LTO workers.

A broader x64 private comparison added five X86 Core Machine translation units (`clock.c`, `cpu_bus.c`, `machine.c`, `machine_scheduler.c`, `memory.c`) to the three CPU units. On the same fixed 10000000-instruction, 79964294-tick, PC 0003dbc5 checkpoint, baseline/LTO took 20848/21287 ms. This single sequential pair does not demonstrate a benefit and does not validate x86 or other workloads. Do not generalize the smaller CPU-only result to the whole Machine or enable a global flag on this evidence.
A further x64 private comparison included all five CPU translation units, matching the component boundary rather than the initial three-file experiment. The fixed checkpoint remained 10000000 instructions, 79964294 guest ticks and PC 0003dbc5; baseline/LTO took 19961/20806 ms. The linker again reported serial LTRANS. This single pair shows no improvement at the proposed production target boundary, so the earlier three-file result does not justify changing the CPU target or enabling a broader configuration. No x86 target-boundary comparison was run, and no production configuration or binaries changed.

## Bounded single-division clock candidate

For source steps no larger than UINT32_MAX, a private candidate computes `elapsed * numerator + phase` once and divides once, preserving the identity-domain path and retaining the original overflow-safe general formula for larger steps. With 32-bit numerator and phase, the short-step product and sum fit in 64 bits. A 500000-case deterministic mathematical comparison covered output ticks, phase and delivered counter, including zero, 32-bit boundaries, UINT64_MAX and random valid ratios; this is not a substitute for actual clock tests. At the identical 10000000-instruction, 79964294-tick, PC 0003dbc5 checkpoint, x64 baseline/candidate were 22765/24740 ms and x86 24954/25430 ms. These are single ordered pairs, but neither supports the extra branch. The candidate was not admitted; no production code or public contract changed.

## CPU optimization-level comparison

A task-owned fixed-work probe directly compiled all five X86 Core CPU translation units and its harness with either the current GCC Release `-O3` or `-O2`, retaining the same remaining cached Release libraries and private CMOS/media. At 10000000 instructions, 79964294 guest ticks and PC 0003dbc5, x64 O3/O2 took 20198/23992 ms and x86 took 24953/25019 ms. Each is one sequential pair; O2 showed no benefit in this workload and was substantially slower on x64. No build flag, production component or released EXE changed.

## Single-byte instruction fetch candidate

A private CPU-source candidate used direct one-byte copies within the existing code-fetch function when the requested size was one, leaving the prefetch, code-length, fault and multi-byte paths otherwise intact. It retired the same 10000000 instructions at 79964294 guest ticks and PC 0003dbc5. x64 baseline/candidate took 19524/19942 ms; x86 took 23933/24536 ms. Each is one ordered pair, insufficient to establish a stable regression, but neither supports adding the special branches. No production code changed, and instruction-fetch fault, self-modification and debug semantics were not qualified by this probe.

## Clang CPU object comparison

A task-owned x64 probe compiled the same five CPU translation units with Clang 22.1.8 at `-O3`, targeting `x86_64-w64-windows-gnu` and the installed MinGW headers, then linked those objects using the existing GCC driver, harness and non-CPU Release libraries. A directly compiled GCC CPU baseline used the same source and fixed workload. At 10000000 instructions, 79964294 guest ticks and PC 0003dbc5, baseline/Clang took 20477/20617 ms. This single ordered pair does not demonstrate a runtime gain or qualify the ABI, optimizer behavior, x86, product builds or full tests. No compiler configuration, production code or released artifact changed.

## LOOP address sampling

A further private x64 CPU probe sampled one in 256 successful E2 LOOP handler entries, recording current instruction linear address, calculated branch target and whether the decremented count requested a taken branch. It used a 250-second product-path run with private CMOS/media; the guest RTC moved from 00 to 22 seconds and the process exited zero. Sampling windows use host time refreshed every 65536 LOOP entries, beginning with the first LOOP, so their exact start/end and workload phase are approximate and should not be directly compared to the earlier opcode-fetch histogram.

During the approximate late 220–240 second window, all 1021 samples fit the 128-entry table and none targeted their own LOOP instruction. Leading sampled instruction/target pairs were 00031089/00031051 (344/344 taken), 00031007/00030fcd (248/319 taken), 00030d34/00030d2a (81/85 taken) and 00030ced/00030cd7 (62/83 taken). The instruction bodies between those addresses may have architectural effects; this evidence cannot authorize collapsing or skipping them. The run is not a complete enumeration, a proof of all ROMs, or an interactive acceptance. Output SHA-256 `65B443E2C7F86E1CE828C2CB8FFF3A1F5ED6FC4D140C108ECD50CFF64B569D1C`.

## Prefix dispatch upper-bound probe

A private, deliberately non-production experiment directly dispatched the two frequently observed F3 and 66 prefix handlers instead of their normal instruction-table lookup. It retained the original handlers and all other instruction paths, so this experiment temporarily duplicates dispatch and is not an acceptable final design. At 10000000 instructions, 79964294 guest ticks and PC 0003dbc5, x64 baseline/candidate took 23116/21754 ms and x86 took 23364/25106 ms. These are one ordered pair per architecture and do not establish full prefix, V30, exception, debug or interrupt equivalence. The opposite directions do not justify replacing the canonical dispatcher, and no production change was made.

## REP continuation prefetch probe

The current instruction loop rewinds EIP for a continuing repeat, whereas the fetch expectation was advanced during prefix/opcode decoding. A private CPU-source candidate treated that rewind as expected when prefetch remained valid. It kept each instruction iteration and the existing event path, but altered real fetch/bus/timing behavior and was never eligible for production without independent hardware-model and self-modification, fault, interrupt and debug proof.

At 10000000 retired instructions, x64 baseline/candidate took 23802/20375 ms and reached 79964294/63009739 guest ticks at PC 0003dbc5/0003db16. x86 took 23910/22718 ms with the same respective tick and PC differences. In this single ordered pair, approximate guest-tick throughput fell from 3.36 to 3.09 million ticks/host-second on x64 and from 3.34 to 2.77 million on x86. Wall-time improvement for a fixed instruction count is not a guest-clock improvement here; differing checkpoints also preclude claiming the same guest workload. Neither result establishes which prefetch behavior is hardware-accurate. The candidate was rejected for this performance task and no production code changed.

## Worker CPU-time and scheduling check

A task-owned fixed-work wrapper read Windows thread user+kernel processor time and QueryThreadCycleTime on the actual Machine worker immediately before its first and after its last Core quantum. It used the existing cached libraries, private CMOS/media and the same 10000000-instruction checkpoint. Both measurements returned success, and both reached 79964294 guest ticks at PC 0003dbc5.

| Host | Wall time ms | Worker processor time ms | Worker CPU/wall | Reported thread cycles |
| --- | ---: | ---: | ---: | ---: |
| x64 | 20167 | 19859.375 | 98.5% | 39550542493 |
| x86 | 23762 | 23375.000 | 98.4% | 46593268620 |

On this host and early workload, the worker was scheduled for nearly the whole measured wall interval. This rules out long worker sleeping or ordinary descheduling as the dominant explanation for these specific runs; it does not establish CPU availability on other hosts or later guest workloads. The Windows cycle counter is frequency/implementation dependent and is not a host retired-instruction count. Instrumentation uses only two thread-time reads per run and does not modify the guest. Log SHA-256: 98F15C290324A8C54FA3F7ABDA626CDCC5BC4D9F0081AE6201C7C1CB2C8D76B6.

## Debug-memory recording upper bound and combined prefix scan

A task-owned upper-bound probe removed only the copies of memory values into the instruction debug record while preserving the remaining access bookkeeping. This knowingly breaks the paused Debug observation contract and is not a valid production candidate. At the same 10,000,000-instruction checkpoint (79,964,294 guest ticks, PC 0003dbc5), baseline/candidate times were 20,952/22,305 ms x64 and 24,207/24,865 ms x86. These single sequential pairs do not quantify the actual recording cost, but give no evidence to justify a design that could compromise the snapshot.

The current timing-input capture scans the same instruction prefixes twice: once for segment/lock inputs and again for the opcode index. A separate task-owned candidate combined those scans while retaining their original prefix classifications, then linked it with the existing cached libraries and private media. At the same checkpoint, baseline/candidate took 22,608/21,974 ms x64 and 26,105/29,266 ms x86. This single sequential pair does not establish a stable performance difference; its opposite architecture directions do not support a production change. Checkpoint equality alone does not qualify all profiles, malformed prefixes, Debug observations or exceptions. No production file or released executable changed.

## 486 target-lexeme preview cost

A separate task-owned x64 product-path probe wrapped the existing target-lexeme preview and sampled one call in 2048 using the Windows performance counter. It ran for 253.02 host seconds with private CMOS/media, exited zero and ended at RTC second 19 with a 640x475 graphics frame. The windows start at the first preview, with the time classification updated only on sampled calls; their boundary and call totals are approximate. Early 0-25 seconds had 639 samples and an estimated 1,308,672 calls; late 220-240 seconds had 687 samples and an estimated 1,406,976 calls. Both accrued approximately 1.49 microseconds per sampled call. A rough extrapolation suggests preview consumed about 2.1 seconds in the late 20-second window, subject to sampling bias and instrumentation overhead. This establishes a possible bounded hotspot, not a real-time solution or validated speedup.

The 486 preview copies the CPU execution context, including its cache and TLB, along with CPU and instruction state. Its fetch path consults those cached lines and translations; reading guest backing memory instead is not equivalent. Any change must retain cache, paging, self-modification, faults, interrupt observation and paused Debug semantics, and must avoid a second owner or persistent mirrored state. The probe did not modify production code or qualify x86. Log SHA-256: 389E9B487C9FFFD864DC16606078DD501AD103BD630FD3C19A96816EAF3A015E.

## Borrowed-cache preview experiment

A task-owned non-production candidate omitted the 486 preview's cache/TLB copy and read the original execution context's cache and TLB while the same worker performed its preview. For this experiment only, it reused a diagnostic context pointer as a temporary source marker; this is not an acceptable production contract and has not been qualified for unrelated preview users, reentrancy or all CPU/fault paths. The existing owning CPU remains unmodified in these checks, and guest instructions and clocks are not intentionally skipped.

At the same 10,000,000-instruction, 79,964,294-tick, PC 0003dbc5 checkpoint, sequential baseline/candidate results were 24,208/21,108 ms on x64 and 24,254/23,133 ms on x86. A reverse-order candidate/baseline pair measured 19,685/20,314 ms on x64 and 23,875/24,926 ms on x86. The reverse-order benefit is about 3.1% and 4.2% respectively; this limited startup workload and host variation do not establish a steady-state or interactive benefit.

The candidate was separately linked with the existing cached CPU cache, TLB, Debug state and Machine instruction-preview tests: all four passed once on each architecture. This targeted evidence does not prove that the pointer reuse is safe, does not replace full qualification, and does not qualify an implementation. A clean design would require an explicit, bounded immutable borrow for preview only; it must account for debug patch preflight, any nested preview, page walks, cache hits, self-modification, errors and object lifetime, without repurposing diagnostics, introducing global state or changing the normal live-state owner. Evaluate the actual pointer/lookup cost and late workload before admitting such a change. No production source or product was modified. Logs SHA-256: E3539C8CCAF0F5DBBD650735ADC1899D890AB10B986E69C0F9EF04963CACF3D4, 444F38B2960965AA942C78604FBE3A258518DA83A6680F6FE69FBC12C9319D86, B7997810A6888F7851236DB2B9CD8C8FB6F932C34A44459159EEFC8465A708B4.

## Borrowed-cache late workload and access-log-copy candidate

A sequential x64 product-path comparison used the same private copies and a 25-sample run for each directly compiled variant. Baseline and experimental cache-borrow candidate both exited zero after 252.58 and 252.57 host seconds. The last published quantum reported 680,640,960 and 708,000,735 guest ticks respectively; RTC showed 20 and 21 seconds. Both last frames were 640x480, but their hashes differed (baseline 4321B893EA63AE4B8390D300A88FB9EA1831BE956CE58C0A0BAE63ED3727B721, candidate 85A35680232CA0AD623DB7DD18D23A79396931ACB133791581D40C37048AEF41), as expected from distinct guest progress but not independently proven equivalent. This one ordered comparison suggests roughly 4% greater reported tick progress and does not establish identical guest state, x86 performance, real-time usability or interactive acceptance. The candidate still uses an unacceptable diagnostic-pointer shortcut; no production change is justified without a real borrow contract and direct semantic evidence. Logs SHA-256: 2FBD0C7499DC88E7B309F6E1BB3E92F4CA3C84D49298BE3644A0A100C6D4B563 and B2FB2A2711435F52B8CD63A16157785AFB097E74552D66A69765B3A7266A1DCC.

A second task-owned candidate avoided copying the old instruction memory-access array into the preview while preserving the surrounding fields. Preview resets the access count and uses forced instruction fetches; nevertheless, uninitialized-array safety on every failure and callback path remains unproven. At the fixed 10,000,000-instruction, 79,964,294-tick, PC 0003dbc5 checkpoint, x64 baseline/candidate took 21,032/20,916 ms and x86 took 27,857/28,833 ms. A single ordered pair does not prove a regression, but shows no convincing benefit. The candidate is not admitted. Log SHA-256: 7AE8A64C9E64FC2485F387173B0A2463AC9CC620CF7923AF2F910FCC20435029. No production code, public contract or published EXE changed.
