# T582 S26 — Personaware Game baseline

## Event-driven navigation experiment

### Follow-up with visible selection checks

A subsequent x64 run waited for the blue label in each expected menu row
before queuing the next Down make/break pair, and saved each observed frame.
Selections 0 through 4 were seen at one-second samples 237, 248, 252, 256 and
261. Enter was sent only after selection 4 appeared. Game-specific pixel
features matched at sample 294, 33 samples after Enter; the run exited zero
after 298.56 host seconds. Visual inspection confirms that the saved frame
contains the Game board but its surrounding interface is still being drawn.
This is evidence of progress to Game, not a measurement of complete drawing
or of input delivery time. The samples suggest why the previous blind one-
second navigation was inadequate, but do not identify which of its inputs
were consumed. Both probes use direct Machine input, not native Window/RDP.
No production code or owner media changed. Log SHA-256:
`8938014321E8A25370CE2869B991617516FF69A9A7DABB05A3BC7B3DC0D5B6AC`;
saved frame SHA-256:
`58F9ACE8C250698FE995A2A0D37C551CE07DB4C0C4E6D8970C44430232C67956`.

A later x64 probe used the committed implementation and private media, polled
the published frame once per host second, and avoided per-pixel screenshot
writes. The same two menu pixel features used by the menu-timing probe first
matched at sample 208. It then queued four Down make/break pairs at samples
209–212 and Enter at sample 213. No two-pixel Game-frame match was observed
before the 500-sample limit; the probe cleaned up and returned failure after
506.04 host seconds.

This does not establish a product input failure: the probe did not capture a
frame after navigation or observe device delivery, and the game detector was
not independently validated against the actual final frame. Rapidly sending
keys also did not wait for each visible menu transition. The earlier slower
navigation and adjacent-pair experiment remains the positive direct-Machine
evidence. Any repeat must observe each selected menu state and the actual
frame, rather than blindly enqueueing keys or extending the timeout. This
experiment changed no production code, owner media or published binary. Log
SHA-256: `80739D6EC51FA78D3A5F0F2B87ABF65E2AAC9F1EED2BD30592CAE9B1E29009C9`.

## Single-click response observation

### Adjacent pair action after compact preview

An x64 private-media run using the committed compact-preview implementation
entered the same Personaware Game through the ordinary Machine queue. After
the complete board appeared, it moved the cursor to a pair of adjacent
matching tiles and submitted a left-button press and release. Inspection of
the next sampled frame shows both tiles removed, the matching tile count
falling from 32 to 30, and the displayed score changing from -171 to -169.
The changed frame was captured at the next approximately one-second sampling
step after the press was queued; frame capture and polling overhead mean this
is not a precise input-latency measurement. A subsequent move and click were
also sent, without asserting another scoring action.

The run exited zero after 372.43 seconds. The complete board was first captured
at sample 33, with ten-second startup samples; this is not a precise startup
duration. The log SHA-256 is
`818C143FFC962E341E41DFBE3CFB06FF4488CC726248167E71C97D3432D36D26`;
the frame showing the removed pair hashes to
`EC25C3ACE5C4BAD07C77422B2F6E23039D6D1307B4A01E806696569BDFE9A113`.
This establishes one successful gameplay action in the instrumented direct
Machine-input path, not whole-game qualification, x86, native Window/RDP or
acceptable startup/clock speed. Owner media and product source were unchanged.

### Device delivery follow-up

A separate x64 private-media run instrumented the existing Machine delivery,
8042 AUX packet receipt and data-read functions at link time. For the same
initial-position click, Machine delivery returned OK for both edges, and the
controller accepted `09 00 00` and `08 00 00`; its subsequent data reads
returned those six bytes in order. The captured board remained unchanged.
This demonstrates delivery and controller reads in this run, not the guest
software's interpretation or a successful game action. It exited zero in
364.22 seconds; log SHA-256:
`12481F8A5B3A401B27673B2867C6929642CCA6589683CAB012320D61215BCB1F`.

Another run sent two relative moves and two press/release pairs at one-second
sample intervals. All six events returned OK, all six AUX packets were
accepted, and the controller reads reproduced the submitted bytes. Frame
inspection showed cursor movement, but no removed tiles or score change.
The intended pair and the game's connection rules were not established, so
this is not a successful-gameplay test or a proven guest input defect. The
run exited zero in 374.73 seconds; log SHA-256:
`F407C88BBCC04ED44DA8D255F4511994A4195C74037120A0E0F62F25B0FD231D`.
Both probes use direct Machine input, private media and instrumented code;
they do not validate native Window/RDP or measure actual response latency.

A private-media x64 run entered the same Game using the existing Machine
queue. Once the complete board was visible, it queued a left-button press and
release at the initial cursor position, then sampled frames approximately
once per host second for 15 seconds. Both submissions succeeded and the run
exited zero after 369.41 seconds. The complete-board image was unchanged at
every sample, including the image before submission. The log SHA-256 is
`DEFE562E33224E4089585273DC78DDA59FE2BFDDF0BF8608A4586E44F16D335B`;
each of those frames matched
`E5C1AB80C00F84C1B80C1069EFA58FD01D761FDE7AC07F7982F0B0B34AB0DE99`.

The queue accepts events before device delivery; the driver discards the
delivery status, and this probe did not instrument PS/2 packet acceptance,
guest consumption, the selected tile or the game's selection rules. It cannot
therefore distinguish no visual response by design from rejected or ignored
input, nor measure click latency. It used direct Machine events rather than
native Window/RDP input, and does not establish playable interaction. No
production source, published binary or owner media changed.

## Read-modify-write precheck frequency

A scratch-only x64 CPU build counted calls to the existing `_m_read_rmw`
path, separating register from memory operands. At the fixed startup
checkpoint of 10,000,000 retired instructions it counted 183,246 calls,
including 12,678 memory operands; the checkpoint remained 79,964,294 guest
ticks and PC `0003dbc5`. The counter changes the compiled code and does not
measure the precheck's exclusive cost.

An independent private-media Game run used the same stage markers and input
route as the opcode probe below. Between the sample-30 and sample-34 markers
it counted 3,410,291 register and 120,104 memory operands on this path. The
sample-34 frame has the same complete Game hash as the prior probe. Marker
changes are asynchronous to guest input and frame delivery; the counts are
not an exact draw interval or a complete count of every memory write. The run
exited zero after 352.89 seconds. Its log SHA-256 is
`01A2FFA04DD28330A64DC5A6A4A4482D12666E52159277B4F6070507143E9ECF`.

Memory read-modify-write prechecks are too infrequent in these observed
workloads to explain the large throughput gap by themselves. They also check
write permission before the initial data read, preserving fault and device
side-effect order. This evidence does not justify deleting the precheck or
introducing translation caching; it does not measure x86 or native desktop
performance. No production code, owner media or release binary changed.

The existing PC110 Personaware menu contains a Game entry even though the prior extension-based disk inventory did not identify a separately named game executable. A hidden x64 probe used the ordinary product factory, Machine input queue, current release libraries, and private copies of the configured firmware inputs, CMOS and disk images. At samples 26–29, ten seconds apart, it submitted four Down make/break pairs; at sample 30 it submitted Enter. The sampled menu changed from World Clock to Calculator after the first Down, and the Game screen was visibly present by sample 34. This demonstrates a real game workload is available without acquiring additional media; it does not demonstrate successful gameplay or native Window/RDP input.

The nearest logged quantum before Enter reported host millisecond 86,977,963 and 909,024,041 guest ticks. The nearest logged quantum before sample 34 reported host millisecond 87,018,056 and 983,517,525 guest ticks: approximately 40 host seconds and 2.26 seconds on the declared 33 MHz guest axis. The frame at sample 32 showed a partially drawn Game screen; samples 34–39 had identical image hashes and showed the complete screen. Screen capture and log instrumentation add overhead. This is a single run at ten-second sampling resolution, not precise input-to-frame latency or a claim that an unchanged screen is frozen.

The guest process exited successfully after 403.1 host seconds. A second private-media run repeated the navigation and, at sample 35, submitted a left-button press/release with absolute input coordinates (12, 35). The samples captured at 35, 36 and 37 had the same complete-screen hash; the run exited zero after 383.22 host seconds. The queued input is not proof of guest acceptance: the PC110 adapter forwards absolute input to its pen path only when that controller is enabled, while the common mouse route is separate; calibration, intended tile and delivery/acknowledgement were not observed. This is an inconclusive interaction probe, not evidence that the game is frozen or that a performance change is needed in the input path.

No owner media, public source, executable or configuration changed. Native presentation and gameplay speed remain unverified.

A third private-media x64 run sent a relative mouse movement `(60, 30)` at sample 35, left-button down at sample 36 and release at sample 37, each ten seconds apart. Relative movement is the type emitted by the Win32 Window path and consumed by the PC mouse mapper; these events were still injected directly into the Machine queue, not through a native desktop. The next sampled frame showed the game's pointer moved from the upper left into the board. Frames at samples 36–39 were identical after that movement, including after the press/release; no board change was observed. The attempted tile and the game's click rules were not established, so that result does not prove failure to click or gameplay success. The log's nearest quantum to movement was host millisecond 88,101,441 and to the next frame was 88,111,940; ten-second sampling only bounds observed response, not actual latency. The process exited zero after 403.21 seconds.

Scratch evidence SHA-256 (guest log, menu after Down, complete Game frame): `4246F84BEDA167CD6DFE9B568FDF431A92A38D8A0397908A37D408132E50F78D`, `54941C36AD92B6E439C1E64E2BE165EBFC16276C38E4AAD45585849F6DDF2A84`, `E5C1AB80C00F84C1B80C1069EFA58FD01D761FDE7AC07F7982F0B0B34AB0DE99`.

Second guest log SHA-256: `BAAEDD9D0BFDF7E69C7147607ED75B8D990ABDF9F2457ECC70688EA00F7B5B55`.

Relative-input guest log SHA-256: `7A18F9D3884A3E0B317B252C779BC0E5E5F561E4BDA5BE88245A240D93CBB64E`; frame after motion SHA-256: `63EB3A5FBD4085B9B15AE3441B10F19E219A2443F48099F287870EEF285AB157`.

## Game drawing workload

A separate private-media x64 run used the same input sequence and counted successfully fetched primary opcode/prefix bytes in a scratch-compiled copy of the current CPU source. The producer switched counters at the sample-30 Enter and at sample 34, and the guest worker observed the stage through an atomic value. A visual check confirmed sample 34 displayed the complete Game screen. Instrumentation on every opcode fetch changes execution cost and stage switches occur before input/frames are consumed, so the windows are approximate; these are byte-fetch counts, not retired-instruction counts or exclusive CPU time.

The approximate sample-30–34 drawing interval contained 28,927,504 counted opcode/prefix bytes. Its largest categories were `74` (1,975,044), `75` (1,827,504), `D0` (1,769,027), `80` (1,756,133) and `8A` (1,679,695). In contrast, the earlier startup-to-menu window included `F3` 8,112,014 and `AB` 5,688,521 among 202,499,438 bytes, consistent with the separately measured startup REP-store workload. These counts suggest that optimizing startup's repeated clearing alone would not address the game's drawing workload; they do not identify inclusive costs or prove that any specific opcode is safe or worthwhile to change.

The nearest logged quanta at Enter and sample 34 were host milliseconds 88,613,834 and 88,653,773, and guest ticks 894,861,059 and 985,036,129. This instrumented run took 373.07 host seconds overall and exited zero. Its four-sample draw interval and frame capture include instrumentation and are not a release-performance measurement. No production source, media or executable changed. Guest log SHA-256: `580386652AC35D8D078D90285FC8ADA974552B4FBB4DBAA6AF4F07395BA81925`; complete Game frame SHA-256: `E5C1AB80C00F84C1B80C1069EFA58FD01D761FDE7AC07F7982F0B0B34AB0DE99`.

## Display access during drawing

A separate private-media x64 run wrapped existing F65535 functions without replacing them. The same stage markers and input route were used; the final sample-34 frame hash again matched the complete Game image. Between the sample-30 and sample-34 markers it counted 770,345 aperture queries, 78,841 reads, 266,911 writes and 2,280 snapshot calls. The calls are not unique guest instructions: routing and the PC110 adapter both query the aperture and chip read/write also validate it. The stable-frame stage subsequently counted no aperture reads or writes and 1,253 snapshot calls.

One call in 128 was timed with non-serialized host `rdtsc`: drawing-stage query, read and write sampled totals were 471,760/6,018, 58,580/615 and 514,020/2,085 TSC units/samples, respectively; 17 snapshot samples totaled 2,644,700 units. Each wrapper interval includes its own overhead and native calls; deterministic sampling can alias with loops, clocks may not be invariant, and snapshot samples are too few to establish reliable cost. Counts are limited to references intercepted at the link boundary. The sample window is based on host-time frame markers and includes input and capture work; it is not an exact guest-instruction interval. These data identify duplicate query work but do not show that removing it can resolve the roughly 40-second draw delay. No new interface, production change or performance claim is justified solely from this probe.

The probe exited zero after 372.96 seconds; log SHA-256: `7FFFD20F657CECB71B4380B10DFC6A3C4CF585CB0119B3CD35CB8B4E048F7407`; sample-34 frame SHA-256: `E5C1AB80C00F84C1B80C1069EFA58FD01D761FDE7AC07F7982F0B0B34AB0DE99`.

## Game drawing execution and scheduler

A separate x64 private-media run wrapped the existing CPU refresh, board-deadline, time-publication and Machine-run calls, with one host `rdtsc` sample per 256 calls. It used the same input and stage markers; the sample-34 frame hash matched the complete Game image, and the run exited zero after 373.38 seconds. Stage transitions occur asynchronously at host frame samples, and CPU refreshes are classified by SMM state **at entry**: a refresh that enters SMM is counted as ordinary. Counters include instrumentation and the brief stop interval; the result is not a native Window or real-time measurement.

During the approximate drawing stage there were 22,658,617 ordinary-entry and 1,599,547 SMM-entry refreshes, so about 6.6% began in SMM. Their sampled intervals totaled 166,010,710/88,510 and 15,443,301/6,248 TSC units/samples, respectively. Time publication totaled 107,347,617/94,784, board deadline 34,897,880/95,679, and enclosing Machine run 324,678,779/76. In the subsequent approximate stable-frame stage, 3,791,291 of 10,596,030 refreshes began in SMM, about 35.8%.

The intervals are inclusive and nested: board-deadline is inside time-publication, both are inside Machine run, and they must not be added. Only 76 Machine-run samples occurred during drawing; deterministic sampling, asynchronous stage switching, different call counts, non-serialized TSC, preemption and instrumentation prevent precise CPU percentages or a proof of optimization equivalence. The difference in SMM-entry counts supports investigating ordinary execution and time advancement for **drawing**, rather than assuming the World Clock idle-service mix applies to this phase. It does not justify skipping SMM, instructions or device events. Log SHA-256: `5025BC55B9A9262317DC070FFA68484CDC9B68126FF986AB8BA37EB51D74C94B`; frame SHA-256: `E5C1AB80C00F84C1B80C1069EFA58FD01D761FDE7AC07F7982F0B0B34AB0DE99`.

## Timing-prefix reuse in the Game workload

The prior [S5 prototype](t582-s5-cpu-timing-performance.md) passes one local prefix count through the existing 486 nominal, L2, memory and decode calculations. Its early-startup comparisons were inconsistent across widths and it was not admitted. The same scratch prototype differs from current `cpu_timing_model.c` only in that local reuse and associated temporary helper names; it does not retain the standalone helper symbols used by focused tests and is not a production implementation.

One new x64 private-media Game run linked that prototype ahead of existing release archives using the original game probe. Both baseline and prototype entered the game and displayed the same complete frame hash. The baseline first sampled the complete frame at sample 34, whereas the prototype did so at sample 33; each frame was sampled only every 10 seconds. From the nearest quantum at Enter to sample 34, baseline progressed 74,493,484 ticks over 40.093 host seconds and the prototype 82,549,870 ticks over 40.103 seconds. Different entry ticks, instruction counts and sampled states, separate sequential execution, host variability and coarse frame cadence prevent attributing this difference solely to the prototype or claiming a validated speedup. Both remain far below the declared guest clock, and x86, full CPU state, exceptions, debugging and other profiles were not tested by this run.

The prototype exited zero after 372.95 seconds; no production source, interface, artifact or owner media changed. Any subsequent production proposal must first restore a single correct owner for the focused helper contracts and establish representative dual-width gains and semantic equivalence; the original x86 S5 result remains contrary evidence. Prototype log SHA-256: `B616CAB4622A11D0A1F659B7D76E945F3A9283188D154253E0F0E5694A6A69D4`; baseline log and complete-frame identities are recorded above.

A subsequent x86 comparison directly compiled the unchanged timing source for the baseline and the same S5 prototype for the candidate using identical Release flags and cached libraries. Each used private copies of the same inputs, sent Down at samples 40–43 and Enter at 44, and exited zero after approximately 513 seconds. Both first sampled the same complete Game image at sample 48. From the nearest quantum to Enter through sample 48, baseline advanced 53,692,037 guest ticks over 39.949 host seconds; the candidate advanced 53,218,998 ticks over 41.083 seconds. The sequential runs started at different guest states; 10-second frame intervals, host variation and probe overhead limit the comparison, so this does not establish a general regression or equivalence. It provides no supporting evidence for a cross-width improvement, and both results remain far below the declared guest clock. No production change or further full qualification was made.

x86 guest log SHA-256 (baseline, candidate): `636DC2E2451E3324AE92702A70B72242A821F6BF638E8BD790CB9C30808FC913`, `FE46F9C5E255D5B861686497BA99F4A3C9D653DABAED484B6DADD77C5F49DD95`; both complete frames match `E5C1AB80C00F84C1B80C1069EFA58FD01D761FDE7AC07F7982F0B0B34AB0DE99`.
