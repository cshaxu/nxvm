# T593 S1: MyOB300 execution-cost baseline

## Controlled ROM DOS replay

The x64 and x86 probes ran serially against the cached GCC Release archives
used by the 0.5.0592 delivery. There was no production change after
`597aad29d`. The probe replaces the Product entry with a small driver of the
real Emulator Machine. It observes published guest text, responds to the
actual F1 warning, then requires the original DOS prompt and the CONFIG.SYS
entry followed by a fresh prompt. The System 1.0 ABA Card and owner C image
were verified before launch. The read-only DOS test medium derives from the
same backup, retains CONFIG.SYS and ends AUTOEXEC before Windows. Its SHA-256
is `34d5597e7b040ea7a8b09d84234eea118d7f2cd8934cc4fc6c5eceaadf90b67f`.
It matches the [published-Console fixture](t592-s3-ob300-local-setup.md#published-dos-performance-handoff)
and retained its SHA-256. The owned Card copy and generated disk were
removed after both probes exited. The instrumented route uses the production
worker and device paths, but does not exercise native keyboard or UI work.

| Observation | x64 | x86 |
| --- | ---: | ---: |
| F1 checkpoint | 44.091 s | 51.591 s |
| First DOS prompt | 87.421 s | 105.072 s |
| DIR CONFIG.SYS from Enter to next prompt | 5.693 s | 8.819 s |
| Core execution within DIR, directly timed | 5.677 s | 8.787 s |
| DIR retired instructions | 3.283 M | 3.274 M |
| DIR guest source ticks | 14.138 M | 14.091 M |

At the board's declared 20 MHz macro-proportional rate those DIR tick totals
represent about 0.71 guest seconds. The instrumented worker delivered about
2.48 M source ticks per host second in x64 and 1.60 M in x86 during DIR.
These are macro-model throughput measurements, not physical chip timing.
The same command is already slow with native presentation removed. The
published pair's comparison remains 6.015 s and 7.344 s in the earlier
Console replay. Neither result meets the one-second gate. Earlier T591
input measurements retain the ten neutral keys within 250 ms; the new
replay did not remeasure the ten individual echo latencies.

| Region | x64 cold | x86 cold | x64 DIR | x86 DIR |
| --- | ---: | ---: | ---: | ---: |
| Core run, directly timed | 89.472 s | 106.966 s | 5.677 s | 8.787 s |
| CPU refresh | 67.843 s | 79.144 s | 4.315 s | 6.858 s |
| Guest-time publication | 24.534 s | 32.997 s | 1.479 s | 2.518 s |
| Board deadline observation | 10.904 s | 13.060 s | 0.608 s | 0.981 s |
| Board advance before internal devices | 7.206 s | 10.709 s | 0.396 s | 0.778 s |
| Board advance after internal devices | 5.554 s | 9.475 s | 0.348 s | 0.741 s |

Cold counters include typing DIR up to Enter. Only Core totals are directly
timed. The nested rows are inclusive one-in-1,021 QPC estimates. Deadline
and device-advance rows fall within publication. Instrumentation,
extrapolation and boundaries make the estimates unsuitable for addition.
The board checked deadlines 48.33 M and 47.70 M times in the cold phase and
3.43 M and 3.42 M during DIR. Board advance callbacks ran at practically
the same frequency. This matches Core's ordinary per-retirement publication
with further subdivision at device deadlines.

## Ranked work

1. **CPU execution, preparation and timing selection (X86 Core).** The
   largest broad region includes fetch, decode, execution, restart state,
   bus access and timing. Earlier equal-media sampling showed timing
   selection alone at about 23–28 s during boot and branch-target lexical
   preview at 12–15 s within timing. The original diagnostic counted 8.6 M
   previews for roughly 45 M retired cold instructions and no additional
   previews for retirement observation. Shared timing actually uses the
   following instruction's lexical components. Keeping the existing fetch
   and exact timing is mandatory. A lighter preview scratch pair reduced
   cold sample cost and made DIR slower. It is not a selected patch.
   Fetch and memory-routing cost remain inside this aggregate.
2. **Scheduler deadline scans and device advancement (X86 Core and X86
   NXVM).** A single advancement queries PIT, RTC, DMA, storage, keyboard,
   PIC and applicable profile/device deadlines. Many calls occur between
   register writes and effective edges. A batching or cache proposal must
   show correct invalidation for I/O and asynchronous events and identical
   IRQ/DMA/deadline ordering. A cache without such proof is out of scope.
   Count repeated pure queries and unused advancement work before proposing
   a scheduler change.
3. **Exact fractional clock conversion (X86 Core).** Reuse the T592
   scratch differential result as a narrow S2 candidate. The existing
   implementation already specializes equal-ratio domains and narrows
   inverse conversion. Forward fractional conversion still uses the
   general division sequence. A range-proven native-width quotient and
   remainder can preserve guest tick/phase delivery. T592 verified 280,000
   deterministic cases on each width and sampled an x86 DOS decrease of
   about 2 s in the conversion region. One stock/candidate DIR pair was
   8.936/8.513 s. Unchanged CPU samples also moved, so the full boot
   difference cannot be credited to this change. Compare equal work first,
   with no new public state and all existing edge and overflow behavior.
4. **Host pacing and display.** The runner has no Turbo wall-clock
   throttle. It still does a time-observation call before checking the
   selected mode once per quantum, versus millions of board observations
   in Core. In the DIR sample Core consumes nearly the entire elapsed
   interval even without native presentation. That bounds the role of the
   host Window in this DOS workload only. Windows graphics and firmware
   waits require their own samples.

No timing model, test threshold or release was changed to obtain these
results. The measurements justify first evaluating the exact arithmetic
candidate while isolating preview and repeated deadline work. Even
eliminating the entire measured board-deadline region alone cannot meet
the DIR target.

## Measurement identities

| Transcript | SHA-256 |
| --- | --- |
| Current x64 DOS worker and guest output | `f0e2b48957e6891282d3d4ef169a48b330ff15798c04dd685b52032e30063d13` |
| Current x86 DOS worker and guest output | `b9ace8804d0d9b003d0551fcc27a32f19afd3fb062b7022c7155789f3c4044b2` |

Source anchors are [Core scheduler](../../../../src/x86-core/machine/machine_scheduler.c),
[board deadline query](../../../../src/x86-nxvm/board-base/board_deadline.c),
[forward clock conversion](../../../../src/x86-core/machine/clock.c), and
[CPU timing model](../../../../src/x86-core/cpu/cpu_timing_model.c).

## Original Windows and power switch, native Window x64

One separate run used the original restored C image without disabling
Windows startup, a disposable overlay, a private copy of a known-good
CMOS seed and a private unswitched Windows desktop. The probe links the
production App and cached Release archives with diagnostic observers.
It directs Win32 key messages and a private Ctrl/Alt keyboard state to
the actual Window. This does not run through a physical keyboard or RDP.

| Checkpoint from private process launch | Elapsed host time |
| --- | ---: |
| First identified original Program Manager frame | 163.406 s |
| First observed File selection after four F10/Down attempts | 208.610 s |
| Directed Ctrl+Alt+O completed | 210.063 s |
| Firmware at F000:A337h, waiting for the next switch press | 229.047 s |

The 45.204 s from first draw to first observed response is a checkpoint
interval, not an individual input event's latency. The chord's logged
make and break both reached the board with modifiers 03h. The make
asserted one SMI at tick 528,700,280. SMM entered immediately, returned
to the guest, and the original wake poll was observed at tick
575,048,653. The intervening 46.35 M guest ticks represent about 2.32 s
on the nominal 20 MHz time axis but took about 19 host seconds.
Measured Core work between the switch and wait was 19.328 s, including
one-in-1,021 estimates of 13.833 s for CPU refresh and 6.961 s for
publication. Board deadline scans within publication accounted for an
estimated 4.140 s across 8.48 M calls. The startup through switch phase
counted about 95 M retirements in the Core observer and about 101 M board
deadline calls. During boot the product's busiest host thread had
accumulated 174.39 CPU seconds while the next used 0.44 s. This supports
a saturated execution thread. It does not isolate Windows rendering
work done synchronously on that thread.

By the 270 s observation bound the instrument did not see a different
graphics size or top four graphics rows published after the switch. It
did not compare the whole displayed client or observe the panel's actual
power-control signal. The guest can be waiting for wake with its last
frame still visible. Treat visible power feedback as a distinct question
for the App's board/display path and current presentation contract. Do
not infer a hotkey failure or a confirmed LCD-model bug from the unchanged
sample. The native product exited normally after the check. Windows x86
and the owner's RDP path were not exercised in this pass.

| Transcript | SHA-256 |
| --- | --- |
| Native Window and firmware checkpoints | `a99fa798706afd13c87e5f4b4afd205b21a78247ee4b6be209ff8a6ab0790491` |
| Captured worker sample in product Console | `34d6d039e2631dd94022d774c84d0f519c6f9ffc3c87b2cd88a0eb65bbdb468e` |

## Verification boundary

The existing cached Release repository-only unit selection passed 717/717 on
each width (x64 89.61 s, x86 115.25 s). Documentation governance, LF,
relative links and diff checks passed. This evidence delivery modifies no
production code, build input, public interface, manifest, or published EXE.
The owner ROM and C master hashes and both deployed MyOB300 hashes still
match T592. The full integration matrix was not rerun for the evidence-only
delivery. The three controlled guest runs above are additional targeted
observations. Generated media, temporary EXEs and copied configurations
were removed. Probe source and bounded transcripts remain under the active
S1 scratch for inspection and handoff.
