# T582 S39 — execution and timing convergence

## Host logical-processor comparison

A task-only x64 diagnostic pinned its own execution thread to logical
processors 0, 8, 16 and 20 in that order and ran the same ten-million-refresh
neutral-Machine CPU loop on each; a separate run reversed the order. Forward
wall times were 6,313 / 7,422 / 7,277 / 7,856 ms; reversed times were 6,655 /
8,017 / 8,902 / 10,648 ms. Corresponding thread CPU times were 5,485 / 6,609 /
6,719 / 7,484 ms and 6,328 / 7,437 / 7,453 / 5,828 ms. Each reached the same
reported PC, without Machine clock advancement or complete state verification.

The substantial order and scheduling variation prevents a reliable ranking
of logical processors: for example, processor 0's second wall time differs
greatly from its thread CPU time. The probe does not establish PC110 Game
speed, physical-core type or a safe product affinity policy. Affinity was set
only on each disposable diagnostic thread; no system power or scheduler setting
was changed, and no production behavior was modified.

## Steady-game SMI signal origins

A task-only x64 link wrapper observed rising-edge calls to the existing CPU
SMI setter during the 30 seconds after the Game marker, with private media.
The run exited normally in 299.57 seconds; the Game marker appeared at
sample 265, 29 samples after Enter. The wrapper counted 16,517 rising-edge
calls, all successful and all observing linear PC `000ff353`; its 32-entry
histogram did not overflow. The PC is the CPU's current CS base plus EIP at
the setter, not a disassembly or proof of the instruction that caused it.
It measures setter calls, not completed SMM entries, and it does not capture
chipset source registers or prove how the guest reached that firmware PC.
The instrumented stage is selected asynchronously and is not evidence for
the drawing interval, x86, native input, or real hardware frequency.

The nearest logged quanta before the Game marker and after its 30-second
observation differ by 85,694,356 guest ticks and 31.203 host seconds. At the
configured 33 MHz this is about 2.60 guest seconds. Dividing the 16,517
observed edges by that approximate guest interval gives roughly 6,360 edges per
guest second; the endpoints and observation window do not coincide exactly.
This quantifies the workload but does not establish that the rate is wrong.

The previously recorded [S24 ASMI audit](t582-s24-asmi-audit.md) already
identified `FF353h` with APM CPU-idle requests in a World Clock run and traced
one request to its SMM return. This new run repeats that source observation
for a different Game steady-state workload; it did not capture request
registers, so it cannot independently classify all Game requests as idle.
The earlier record should have been consulted before launching this probe.
Neither observation justifies suppressing SMI or altering the guest-visible
protocol. No production code changed. Guest log SHA-256:
`9FDCFE9691EC1AFB6A78663DBB51CD05B86CCEB2D750DBD00E5C9039DED3AD4F`.

## SMM classification of non-filled reads

A further x64 task-only private-media run counted non-preview unfilled cache
misses by CPU SMM state and decoded physical region during 30 seconds after
the Game marker. The run exited successfully in 304.76 s; the marker appeared
at sample 270, 26 samples after Enter. All observed unfilled misses in
`30000h–3FFFFh` occurred with CPU SMM active: 3,811,142 data and 2,492,515
fetch lookups. A further 15,561 data misses in `A0000h–AFFFFh` occurred outside
SMM. Counts are lookup iterations, not CPU-time shares; instrumentation and
asynchronous host markers perturb the workload. This does not classify the
draw interval, x86, the chipset memory-control value, complete state or native
Window/RDP behavior.

The existing S19 experiment already enabled the provider cache hook for the
SMRAM region under a proposed chipset condition and retained decode-change
invalidation. Its single World Clock comparison did not demonstrate a gain
and changed cache/timing behavior; exact chipset cache eligibility was not
established. This new Game observation identifies workload frequency but does
not resolve that contract or justify repeating the candidate unchanged.
No production code changed. Log SHA-256:
`7F64C9B3690EEC4CA38B2CE407DDC8042EE7FF13B4C15F95E4908A084DB98166`.

## Physical ranges of steady-game non-filled reads

A separate task-only x64 private-media probe grouped non-preview cache misses
whose fill was disabled by decoded 64 KiB physical region during 30 seconds
after the Game marker. The run exited successfully in 315.47 s; the marker was
sample 281, 22 samples after Enter. Data misses counted 3,524,891 in
`30000h–3FFFFh` and 15,348 in `A0000h–AFFFFh`; instruction-fetch misses
counted 2,305,363 in `30000h–3FFFFh`, with no other populated region in this
observation. These are lookup iterations, not time or retired instructions,
and per-call instrumentation perturbs the workload. This is steady-game, not
draw-time, x86, native Window or RDP evidence.

PC110 registers `30000h–3FFFFh` as a dynamically selected CPU-only SMRAM
overlay; the firmware cache probe currently rejects addresses below `C0000h`.
The same physical region can instead route to ordinary RAM when the overlay
is not selected. The count does not distinguish those states and does not
prove that the rejection is wrong. The next required check is the selected
route and SMM/memory-control state for these misses, then the original cache
coherency and overlay-switch contract. Do not force fills or change guest
chipset settings based on this frequency alone. No production code changed.
Log SHA-256:
`049407C87C0DD528C8D1ABE2E243CB3BEBC07FDB0AE398A60EEBF150C29BC708`.

## Steady-game 486 cache miss classification

A task-only x64 counter classified 486 read-cache outcomes during 30 seconds
after the Game marker, using private media. The run completed in 306.12 s;
the marker was sample 271, 28 samples after Enter. For non-preview data reads,
5,757,250 lookups hit and 3,972,551 missed; 138,710 of the misses had a fill
enabled. For non-preview instruction fetches, 6,198,926 hit and 2,785,817
missed; 287,644 misses had a fill enabled. Across both, 6,332,014 of 6,758,368
misses had fill disabled. In the observed buckets CR0.CD and page PCD were
clear; this does not explain the fill rejection or show that it is incorrect.
Preview hit/miss counts were 176,687 / 103 for data and 2,466,841 / 684,227
for fetch. Counts are lookup iterations, not retired instructions; per-call
instrumentation perturbs execution. This is not an x86, draw-time, native
Window or RDP result, and cannot justify relaxing cache coherency or device
mapping rules. The next specific question is which physical ranges and
existing Machine fill-eligibility checks account for non-preview misses.
No production source changed. Log SHA-256:
`DABB25874BCAD0AC0C82446EA1386ECB5F8D6FB8E13505F32ABB5520729D165F`.

## Double-precision shift candidate

The existing 16- and 32-bit SHLD/SHRD implementations construct the result
one bit at a time. A task-only candidate replaces only those result loops with
bounded unsigned shifts and masking; the existing flag and undefined-result
paths remain. A deterministic model compared 235,000 operand/count pairs in
both directions, including the 16-bit count-equals-width boundary. Separately,
actual CPU CL-form register instructions ran 262,144 cases per build on each
host width, covering 386/486, 16/32 bits, both directions, all 256 count byte
values, eight values per operand and two initial flag patterns. Baseline and
candidate produced identical aggregate hash `3514befe56732c83` for registers,
flags, undefined markers, recorded operands/result and captured timing.

This is not proof of memory-destination, immediate-form, fault, debug,
complete machine-state or Game equivalence, nor evidence that these opcodes
are a PC110 hotspot. It reduces per-operation bit-loop work but does not by
itself establish playability. Production code and published assets remain
unchanged; a candidate receiver requires an explicit active-packet scope,
the existing instruction tests and complete Shared receiving before adoption.

A subsequent task-only differential compared 131,072 memory-destination cases
per build on each host width, including both CL and immediate counts, all 256
count bytes, both widths, both directions, 386/486, four values for each
operand and two initial flag patterns. Both original and candidate produced
aggregate hash `139ddbf1df0db103` for memory value and write count, register
and flag state, undefined markers, recorded operands/result and timing. This
adds actual memory-path evidence, but does not prove protected-mode fault,
watchpoint or complete-machine equivalence. No production source changed.

The bounded loop replacement was subsequently admitted and applied: production
diff +8/-44 lines. Existing double-shift, 486 timing, debug, cache, 80386
secondary timing and board integration targeted tests passed 6/6 per width;
the existing double-shift test covers memory/register and CL/immediate forms.
Cached full units passed 712/712 and default integrations 48/48 on each width.
Profile-specific integrations passed 3/3 (5160), 5/5 (5170), and 6/6
(DeskPro) per width. An initial 5160 profile run failed two tests because the
test executables had not been relinked after profile selection; after rebuilding
the profile test targets both passed. MyRaidenII external selected-set startup
and product-console checks passed on both widths, without a gameplay claim.
Fourteen published files passed PE-width checks; MyNES, MyPC110 and MyRaidenII
deployments match their built outputs on both widths. X86 Core manifest and
NXVM documentation gates passed. The user's unrelated queue edit is retained.

The private-media x64 Game run reached the stable complete frame at sample 281,
36 samples after Enter, and exited in 284.83 seconds. Its frame SHA-256 is
`E5C1AB80C00F84C1B80C1069EFA58FD01D761FDE7AC07F7982F0B0B34AB0DE99`,
matching the prior frame. The prior carry-rotate run took 266.69 seconds and
30 samples after Enter. These single sequential runs do not control host or
complete guest state and do not establish the magnitude or direction of a
performance change. Input was injected through Machine, not native Window or
RDP; neither run establishes playable PC110 performance.

## Synthetic CPU versus neutral Machine fixture audit

An additional scratch-only experiment constructed the same neutral Machine
with the same reset alias and memory for each run, but compared its ordinary
Machine run with ten million direct calls to its own CPU refresh. In
Machine/direct/direct/Machine order, x64 times were 7,915 / 7,747 / 7,016 /
8,409 ms, and x86 times were 12,305 / 8,765 / 8,557 / 12,447 ms. Each
reported linear PC 000f0001; the ordinary Machine run reported 40,000,014
ticks. Direct refresh did not advance Machine time or execute the normal
retirement/scheduler path and did not verify the entire final CPU state, so
this is an intentionally incomplete diagnostic, not an equivalent execution
or a candidate implementation. It uses the neutral Machine bus rather than
the simple CPU-test bus. Results suggest the outer Machine loop is not the
only large difference on x64, and is a larger cost on x86; sequential samples,
layout and differing execution semantics prevent exact subtraction. The
observed direct CPU run includes the Machine's bus callbacks and setup, unlike
the original CPU-only fixture. No production code or public API changed.

The S27 loop's Machine is created with `x86_core_machine_neutral_create`,
not the PC110 board. It supplies 2 MiB of RAM and a reset alias, but does not
assemble PC110 devices. Thus its CPU/Machine difference cannot be attributed
to PC110 PIC, KBC or board-device scheduling. The CPU-only fixture instead
loads CS at zero, uses a direct 512 KiB byte-array bus with no cache-query or
external-cycle callbacks, and binds a one-tick timing fixture. The neutral
Machine starts through reset and a far jump and uses its own memory routing,
CPU bus and Machine timing/retirement machinery. Both execute the same loop
after entry, but they do not have identical memory, reset or timing contracts.
These facts narrow the comparison; they still do not quantify exclusive
Machine overhead or prove that any of those operations is redundant. The S27
results are one earlier sample per width, not current PC110 gameplay evidence.

To isolate the mere presence of cache-query dispatch, a task-only x64 CPU
fixture ran the same NOP/JMP loop for ten million refreshes in off/on/on/off
order. The added callback returns the input address and disables filling, as
does the default initialized result; no Machine or device is constructed.
Times were 3,554 / 3,632 / 3,875 / 3,976 ms and each run ended at EIP 1
without a reported fault. The opposing ordered comparisons and host variation
do not establish a measurable difference. This trivial callback is not the
real Machine address decoder, so the result cannot quantify the full Machine
cache or bus cost, prove complete CPU-state equivalence, or characterize
PC110 Game. The same x86 order took 6,135 / 6,421 / 6,106 / 5,883 ms,
again with EIP 1 and no reported fault. Both x86 pairs favor the no-callback
fixture, but two ordered samples do not establish the cost of the real
Machine callback or complete-state equivalence. No production code was changed.

A further scratch-only x64 diagnostic used the neutral Machine's own CPU
and bus, alternately disabling its cache-query callback or external-cycle
provider. Both omissions change the declared guest contract and are not
candidate fixes. With baseline/query-off/cycle-off/baseline/cycle-off/
query-off/baseline order, times in milliseconds were 5,999 / 7,379 / 7,251 /
7,691 / 7,711 / 7,551 / 8,066; all reported PC 000f0001, but did not advance
Machine time or verify complete state. The large baseline drift and changed
semantics prevent meaningful attribution of either callback; this is not a
Game, x86 or correctness result. No callback was disabled in production.

To check whether wall-time drift is solely descheduling, a follow-up x64
scratch run measured the same executing thread's Windows user plus kernel
CPU time as well as wall time. In baseline/query-off/query-off/baseline
order, wall milliseconds were 6,816 / 6,783 / 6,322 / 6,157 and thread CPU
milliseconds were 6,578 / 6,609 / 6,157 / 5,937. Every run reported the same
PC, but query-off remains intentionally semantically incomplete. CPU time
also changed substantially, so this particular drift is not explained solely
by wall time spent off CPU. Thread-time granularity, host frequency, power
management and other host contention were not independently measured; the
result cannot identify their cause or a reliable callback speedup. Host
processor inventory was unavailable through the sandbox CIM query.

## Steady-game prefetch cache and bus scopes

A separate x64 private-media probe sampled prefetch-provenance cache-way
lookup, bus read and enclosing physical read after the complete Game marker,
for 30 seconds without further input. It exited successfully after 321.33 s;
the marker appeared at sample 286, 31 samples after Enter. Calls / samples /
TSC units were respectively 10,901,154 / 42,572 / 4,178,480;
2,899,718 / 11,431 / 5,828,120; and 6,149,237 / 23,870 / 14,166,552.
Machine run recorded 35,318 / 149 / 241,282,845. Scaled estimates are 1.87%,
2.59% and 6.38% of Machine run. They are inclusive, overlap where nested,
and do not partition all physical-read work; provenance also covers nested
operations. Instrumentation, compiler layout, unsynchronised TSC and only
149 Machine samples limit the figures. This does not prove a removable
operation, a complete bottleneck, x86 behaviour, or desktop usability.
No production code changed. Log SHA-256:
`E89957AD909E1B0BACD0DEFC843011FB5AAA93666915DA5691DC4A4D59FDA5EB`.

## Steady-game prefetch read stages

An x64 task-only probe sampled instruction-prefetch reads after the complete
Game marker, during 30 seconds without further input. It used private media and
finished successfully in 312.15 seconds; the marker appeared at sample 277,
28 samples after Enter. Logical, linear and physical read scopes recorded
respectively 5,618,787 / 22,023 / 18,945,509, 5,618,787 / 22,102 /
17,642,252, and 6,341,117 / 24,611 / 14,534,660 calls / samples / TSC
units. Machine run recorded 37,221 / 155 / 238,169,716. Scaling by calls per
sample gives approximately 8.45%, 7.84%, and 6.55% of Machine time. These
scopes are nested, not additive; physical reads may occur more than once per
logical read. Instrumentation, unsynchronised TSC, compiler layout and only
155 Machine samples limit accuracy. This is steady-game, not game-drawing,
x86, native Window or RDP acceptance evidence.

Most sampled prefetch time lies within physical access and its callees, not
the outer logical-to-linear checks alone. This directs investigation to the
existing physical cache and bus path; it does not prove that any check can be
removed or that this cost accounts for the overall throughput gap. No
production code changed. Probe log SHA-256:
`1E053782C402F59272B57C10E63CBA6BE92C6B95BA53B4C13E377355FB91DA05`.

## Repeated short-branch outcome

The 486 nominal timing evaluator evaluates the old flags for a short
conditional branch. The later generic formula-input capture identifies the
same branch and evaluates its old flags again to publish the branch outcome.
Both consumers are required by their current contracts; removing the second
result would lose observable timing metadata. A shared same-instruction fact
could eliminate duplicate work, but its scope must also account for other CPU
profiles, zero-displacement branches, faults, preview and interrupt-entry
observation. The current code does not expose such a local fact between those
owners without changing their interfaces or adding state.

A scratch-only x64 build counted calls in the original helper and its two
short-branch paths at the private PC110 fixed startup checkpoint. It recorded
911,584 total condition evaluations, 417,301 in 486 nominal timing and
417,301 in the short-branch helper used by generic branch observation. The
run reached 10,000,000 instructions, 79,964,294 ticks and PC `0003dbc5` in
20,730 ms. Counts include the stopping interval, and instrumentation changes
the generated code. This is startup-only; it does not measure the cost of
each evaluation or the later Game workload.

The duplicate is real but applies to about 4.2% of this instruction sample;
its removal alone cannot plausibly establish the order-of-magnitude speedup
needed for the stated PC110 goal. Do not add persistent mirrored branch state
or a second timing path on this evidence. No production source, public API,
owner media or artifact changed. Log SHA-256:
`F41A492DA5BBFF513FE73A5336D584201F333C57923E29D4780BDA72C9A2991C`.

## 486 timing-profile selection order

The shared timing selector tests three Intel profiles before selecting 486.
A scratch-only candidate moved the existing 486 branch ahead of those three,
leaving the V30 route, actual timing evaluator and all formulas unchanged.
No product-specific predicate, second selector or persistent state was added.
The candidate was not qualified for all profile and failure paths.

For baseline/candidate/candidate/baseline at the private fixed startup
checkpoint, x64 took 18,259 / 20,526 / 18,381 / 20,330 ms and x86 took
23,449 / 24,826 / 21,679 / 22,000 ms. Every run reported 10,000,000
instructions, 79,964,294 ticks and PC `0003dbc5`. Both widths change direction
between ordered comparisons, and checkpoint equality is not full-state or
profile equivalence. These noisy samples do not justify adopting the reorder
or imply a product speedup. No production source or artifact changed.
Log SHA-256:
`7161638B4A31F04A1212199778BA41E32D8B93B843608457517EF4C41D5A7C55`.

## Disabled instruction-breakpoint scan

The instruction breakpoint matcher, like the already optimized data matcher,
scans four slots even when all DR7 enable bits are clear. Its caller uses the
matched-status mask only when at least one enabled comparator caused a trap.
A scratch candidate returned after initializing that mask when all enables
were clear, while preserving the entire original scan if any was enabled.
The existing comparator test covers all enabled and matched masks for both
instruction and data cases on 386 and 486, but the scratch candidate was not
run through that test or the full receiving gate.

At the fixed startup checkpoint, baseline/candidate/candidate/baseline times
were 19,209 / 19,611 / 18,973 / 18,213 ms x64 and 22,525 / 23,351 /
23,487 / 25,012 ms x86. All runs reported 10,000,000 instructions,
79,964,294 ticks and PC `0003dbc5`. The candidate was slower in both x64
pairs and x86 changed direction. Host variation is significant, but the
observed potential regression does not satisfy the owner's criterion, so the
candidate was not applied. This is not full-state equivalence or Game testing.
Log SHA-256:
`4A985876116EAD883F8748865E4886D2AF375BF68CD34361157BE3F3AD93B7FB`.

## Test-trained PGO recheck and compilation-unit isolation

The earlier test-trained PGO experiment suggested a startup gain but also
produced an x86 `stringop-overread` warning. Repeating its 12 existing
training tests on current source produced five profile files and reproduced
the warning as an error under `-Werror=stringop-overread`: the shared IDT read
contains an eight-byte protected-mode path while `_ser_int_real` supplies a
four-byte vector. Ordinary entry callers select real mode before this call;
this static observation does not prove every possible reentrant state safe or
establish a runtime overread. Warning-log SHA-256:
`4E7A2543014CC7DF7DE0E0450C7ED369E98EB6EBC303E413BD17A0DADB0F5C3D`.

A scratch vector widened to eight bytes and initialized to zero compiled
without that warning after retraining the same twelve tests against the same
scratch source. An initial attempt to reuse the original source profile for
the different scratch path failed missing-profile checks and is not counted
as a successful build. The widened vector is not a production fix: normal,
virtual-8086, fault and timing paths and all CPU profiles have not been
qualified. Successful compilation alone does not authorize PGO.
Training-log SHA-256:
`588FFEEE90FE4AF3A7DC165112FA21DEB57FB21C1EDC3F4F1CA8CDD88ED69B35`.

Separately, current unmodified source was compiled both normally and with the
fresh test-trained x86 profiles, and linked into the same fixed-work harness.
In one sequential order, ordinary / PGO timing selector only / PGO timing
model only / PGO instruction unit only / all five PGO units took 23,059 /
25,954 / 23,669 / 24,220 / 23,480 ms. All reached 10,000,000 instructions,
79,822,202 ticks and PC `00039119`. The unmodified PGO build retained the
compiler warning; no warning was suppressed in a production build. A single
order and different host conditions are not comparable proof of the earlier
speedup or a late-Game result, and matching the checkpoint is not full state
equivalence. These results do not support identifying one profitable unit or
adopting PGO; no production source, build setting or artifact was changed.
Isolation-log SHA-256:
`8C1B5CE8A23777CCDCE0DA5195226225FB2E4BD5B88EA841B0E626E9B7959571`.

## Fixed nominal timing table prototype

A scratch-only 486 nominal-timing prototype moved opcode groups whose
existing nominal cost is always one tick into one 256-byte lookup table,
removing their original range and switch cases rather than duplicating timing
ownership. Dynamic opcode cases, prefix handling, memory penalties and bus
timing remained on the existing path. The temporary source was not qualified
against all CPU profiles, faults or timing observations.

At the private fixed startup checkpoint, ordered baseline / candidate /
candidate / baseline times were 18,628 / 18,342 / 18,588 / 19,803 ms x64 and
24,141 / 25,221 / 22,245 / 24,123 ms x86. Each reported 10,000,000
instructions, 79,964,294 ticks and PC `0003dbc5`; this is not complete-state
equivalence. x64 favored the candidate in both pairs, while x86 changed
direction. The candidate adds a data lookup and table; the samples do not
prove a stable cross-width benefit or Game improvement. It was not applied to
production, and no full receiving gate was claimed. Log SHA-256:
`8A40DF8E43D46FE210EF13C19AF07DD2CEC00010980A115D26D1170E58E7AA94`.

The same already-built x86 executables were then launched suspended, pinned to
one host logical processor, and resumed, in baseline / candidate / candidate /
baseline order with fresh private media copies. Fixed-work times were 22,653 /
22,209 / 21,724 / 21,562 ms, all at the same instruction, tick and PC
checkpoint. The direction still reverses between the two pairs. Pinning avoids
CPU migration but not frequency, thermal or background-load variation; it
does not prove equivalent guest state or Game performance. The added table and
lookup are therefore still not justified. No production code or artifact was
changed. Log SHA-256:
`287CD73EAC10C9087878EC5B0B16DFD9ED3DE42BA100A0D6382F83A40722E2E7`.

## Non-486 timing-dispatch outlining prototype

A task-only candidate moved the existing non-486 Intel timing dispatch into
one private non-inlined helper, leaving V30 selection and the 486 evaluator on
the original single selector. This explores code layout, not a new timing
formula or executor. The temporary GCC-specific `noinline` marker is not a
production proposal, and the prototype was not qualified for all profiles.

At the fixed PC110 startup checkpoint, baseline / candidate / candidate /
baseline took 19,649 / 22,650 / 18,637 / 19,786 ms x64 and 23,209 / 21,065 /
20,894 / 21,444 ms x86. Every run reported 10,000,000 instructions,
79,964,294 ticks and PC `0003dbc5`. x86 favored the candidate, while x64
changed direction. A further x64 comparison pinned to one host logical CPU
took 18,093 / 16,804 / 16,521 / 16,113 ms; the direction still changes and
pinning does not control host frequency or load. Neither set establishes
complete guest-state equivalence, Game performance or a stable cross-width
gain. The added helper and compiler-specific annotation are not justified by
these results. No production code or release artifact changed.

Log SHA-256 (unrestricted, pinned):
`72AA2069CC6ECED5D42D20B19F0F2BC5F0EFF2E18C4075014DABD474DD715092`,
`942CFC427D3D774CCB2827217CEF682F62673E2635FBB94EA3B0E20C9781ACA9`.

## RCL/RCR carry-ring consolidation

The old RCL/RCR implementation had six width/direction-specific loops. The
candidate retains the existing per-profile effective count and the same flag,
undefined-flag and instruction-record semantics, using one private carry-ring
helper. Count one uses 32-bit arithmetic; other nonzero counts rotate a
9/17/33-bit ring in fixed work. Count zero leaves flags untouched. The actual
production change is 56 lines added and 169 removed in the CPU source, with no
new interface, state, clock or execution owner. The count is bounded to eight
bits, so this is a fixed-work reduction rather than an unbounded complexity
claim. It does not itself establish PC110 playability.

Task-only actual-CPU comparisons covered 262,144 cases per executable:
8086, 8088, 80186, 80286, 80386, 80486SX and V30; applicable 8/16/32-bit
register forms; both directions; all 0–255 counts; eight operand samples;
and four independent CF/OF initial states. Baseline and candidate on both
host widths produced digest `e8284f2137602a53` for status, registers, flags,
decoder observations and timing fields. This is sampled input coverage, not
proof of all state or memory-operand behavior. Existing `cpu_rotate` adds
memory operands and carry-ring boundary coverage; `cpu_v30_native`,
`cpu_486_nominal_timing`, `cpu_486_repeat_timing` and `cpu_debug_state` also
passed with it, 5/5 on each host width. Differential log SHA-256:
`34DE02CD76F754E6C354812E05360BEABCD973E1022D4A2492731463F756BBE2`.

An earlier candidate used 64-bit ring arithmetic even for count one and was
measurably slower on an x86-only synthetic one-count workload; it was not
adopted. The revised candidate's same workload measured baseline / candidate /
candidate / baseline 7,191 / 7,047 / 6,944 / 6,962 ms x86. This synthetic
CPU-only test does not establish Game performance. Its log SHA-256:
`0E0C8A1615CCFA2456F800744AF9761C4C931F2814416F150E8A6AE007F52D93`.

The revised candidate's PC110 fixed-startup comparisons in that order took
18,081 / 18,397 / 18,216 / 17,276 ms x64 and 21,983 / 21,743 / 22,515 /
21,967 ms x86. Every run reported 10,000,000 instructions, 79,964,294 ticks
and PC `0003dbc5`. A separate x64 pinned-core comparison took 17,333 / 18,082 /
18,006 / 18,890 ms. These samples are noisy and do not prove an end-to-end
speedup or full guest-state equivalence. The change is selected for the
owner-approved source and algorithm simplification, not as the usability fix.
Log SHA-256 (unrestricted, pinned):
`C5F3A75B1623FA327A523C476E55B5E14420E95B0975419D42D99B8C96560796`,
`5FDBF2D2F5E1DABA4E42E9D7358F15C6FC26CA2DD53B9ACE69205B5CE301B80D`.
The cached Shared receiver completed on both host widths: each ran 712/712
unit tests and 48/48 default integration tests, followed by 3/3, 5/5 and 6/6
integration tests for the 5160, 5170 and DeskPro profiles respectively.
The x86-core manifest and documentation governance checks passed. All fourteen
product artifacts had their PE widths verified; the default NXVM, PC110,
MyNES and MyRaidenII deployments matched their respective build outputs.
The MyRaidenII selected-set startup and product-console checks passed on both
widths; these checks do not establish MyRaidenII gameplay.

The x64 PC110 private-media Game probe reached its complete-frame marker in
266.69 seconds, with image SHA-256
`E5C1AB80C00F84C1B80C1069EFA58FD01D761FDE7AC07F7982F0B0B34AB0DE99`,
matching the earlier frame. The marker was sample 263, 30 samples after
Enter. This single run is close to the earlier 267.78-second baseline and
does not show a material improvement; it is neither full-state equivalence
nor desktop/RDP responsiveness. PC110 usability remains unresolved.

## Zero access-penalty candidate

This probe substantially repeats the earlier S28 zero-access candidate in
`t582-s28-execution-chain.md`. The investigation index was consulted but its
link to that candidate was missed. The repetition was unnecessary; the result
is retained as a repeat, not presented as an independent new direction.

The 486 access-penalty routine validates translation and cache counters and
then calculates their terms on every invocation. A scratch candidate returned
zero early when all counters contributing to those checks and terms were zero,
except for the task-gate special case. All other inputs kept the original
path. This exploratory source change did not receive correctness qualification.

For baseline/candidate/candidate/baseline, the private-media fixed startup
checkpoint took 18,606 / 19,607 / 18,003 / 17,832 ms on x64 and 22,739 /
23,335 / 21,396 / 21,801 ms on x86. All runs reached 10,000,000 instructions,
79,964,294 ticks and PC `0003dbc5`. Both x64 comparisons favor the baseline;
the x86 comparisons disagree. These noisy samples are not full state or Game
verification. Given the additional condition and no demonstrated gain, the
candidate is rejected without changing production. Log SHA-256:
`2485A41290EEC1E286D6E139DC2F36C55117B68ADC2F7799F2BD6E066927FF0B`.

## Current Game host-instruction samples

An x64 task-only native sampler observed the current committed guest probe
using private media. The guest selected Game at sample 215 and reached the
stable Game frame at sample 245; the sampler's late window began approximately
225 host seconds after process start and ended on guest exit. The resulting
frame SHA-256 was `E5C1AB80C00F84C1B80C1069EFA58FD01D761FDE7AC07F7982F0B0B34AB0DE99`.
Input was injected through Machine, not the desktop. Sampling suspended worker
threads and therefore perturbed the runtime; sample time and guest sample
markers use different clocks and their alignment is approximate.

The worker had 4,105 late samples mapped to the nearest executable symbol or
module. Largest counts were CPU refresh 286, Machine run 171, PIC selection
125, keyboard-input callback 117, elapsed-time publication 112, clock-domain
advance 109, physical memory read 109, timing selection 104, CRT module 104,
and board deadline observation 99. These are sampled instruction pointers,
not exclusive function timings or exact whole-program percentages; optimized
code and inlining may change attribution. They show distributed work in the
observed drawing interval rather than identify a single proven culprit.
Keyboard-input callback activity merits source inspection, but the earlier S8
repeat-deadline change is already present: do not repeat it or suppress input,
device callbacks or deadlines without a separate validity proof. The sample
does not establish PC110 responsiveness or complete guest-state equivalence.
Sampler log SHA-256:
`A26A0C61E37D45D17478BBD7B62B87FD2EC593CA04BA3B8A13C1BC585684C8F0`.
Guest log SHA-256:
`969396C3CC2D78C2912A63A4DB4831EF85A5B924B29B8EAF91B099CF1F30874A`.

### Keyboard callback ownership follow-up

The Board callback obtains the keyboard endpoint's scan set, scanning and BAT
state and, when repeat bytes exist, its remaining repeat time. Controller
deadline calculation needs that snapshot for immediate BAT and repeat events;
controller advancement takes BAT and can publish a repeat after advancing
other state. These are separate observations around possible mutation, so the
host instruction-pointer count alone does not justify reusing a snapshot across
them. When keyboard serial bytes are pending, deadline calculation can query
the callback more than once through its serial and response predicates; source
inspection has not established how frequently that special state occurs in
Game, nor whether removing those few repeated calls improves the workload.
The existing S8 change already avoids the repeat-time query when no repeat
bytes exist. Do not add cached keyboard state or skip BAT/repeat checks on the
basis of this sampling. No production code or additional test was changed.

### Current stable-frame workload split

The completed S39 x64 private-media Game run recorded the first menu selection
at sample 211, with the nearest preceding quantum at 128,899,070 instructions
and 668,714,019 ticks. The Game selection and Enter were queued at sample 233,
with the nearest quantum at 145,011,720 instructions and 707,228,588 ticks.
The stable Game frame appeared at sample 263, with the nearest quantum at
167,411,113 instructions and 762,934,862 ticks. Thus the recorded drawing
interval spans approximately 22.4 million instructions and 55.7 million guest
ticks over 30 one-second samples. These are asynchronously logged quantum and
frame markers, not exact input or drawing boundaries.

The long pre-menu period and the later drawing period are both material to the
observed 266.69-second run. A startup-only memory-clear optimization cannot
by itself establish an acceptable Game experience; similarly, the previously
observed one successful direct-Machine click does not establish native Window
or RDP responsiveness. No new timed run, product change or usability claim is
implied by this analysis. The earlier Game frame and limits are recorded in
the carry-rotation entry above.

## PIC request-check ordering

The current priority loop loads each line's unmask delay before testing whether
that line is pending. A scratch-only candidate computed the existing pending
mask once and checked its bit before loading that line's delay, retaining the
same priority loop and later selection tests. It adds no state or callback.
This differs from the S26 empty-mask early-return experiment. The source-level
reasoning assumes PIC state cannot mutate during this synchronous selection;
the candidate was not qualified against every cascade and priority case.

At the private-media fixed startup checkpoint, baseline/candidate/candidate/
baseline took 19,286 / 19,947 / 18,173 / 17,894 ms on x64 and 24,816 /
22,356 / 22,352 / 22,650 ms on x86. All reported 10,000,000 instructions,
79,964,294 ticks and PC `0003dbc5`. x64 favors the baseline in both ordered
comparisons, while x86 favors the candidate; the magnitude is subject to host
noise and the checkpoint does not establish full-state equivalence or Game
performance. Since this neither reduces algorithmic complexity nor demonstrates
a reliable cross-width gain, it was rejected without production changes.
Log SHA-256:
`D22E84771F3AB7974898898222FC2CC0AB9CFAAD25FEB272575048ADF2F3CC38`.

## Full-size CPU snapshot copy sampling

The prior S26 probe counted 380-byte `memcpy` calls at the instruction
boundary but did not time the copy. A new task-only link wrapper timed one in
256 calls of that size, leaving the real `memcpy` in place. On the private
10,000,000-instruction startup checkpoint, x64 counted 11,967,532 matching
calls and 46,749 samples totaling 2,622,720 TSC units; x86 counted 11,970,123
calls and 46,759 samples totaling 3,964,240 units. Both reached 79,964,294
guest ticks and PC `0003dbc5`, in 18,885 and 22,438 host milliseconds with
instrumentation. The sample means are approximately 56 and 85 TSC units.

This groups all linked copies of that size, not just the previously identified
instruction-boundary call; inlined copies are absent. The nonserialized clock,
sampling, call wrapper, host scheduling and instrumented build affect the
result, which is neither an exact CPU-time percentage nor a valid removal
ablation. The startup-only result does not establish Game performance. It does
not justify replacing the full snapshot with a complex copy-on-write model or
omitting the fault/debug state. No production code or artifact changed.
Log SHA-256:
`4FA120212300556859269DA5A8FAC9D62E924EA2A8182B1D0C862123F1E478AD`.

## Runner rendezvous quantum

The product runner caps Turbo at 4,096 instructions per Core call, with no
tick cap. Common control, input and frame handoff occur at the runner boundary;
Core still advances guest devices at individual retirement boundaries. A
scratch candidate raised the Turbo instruction cap to 16,384, without changing
the Core loop or guest timing formulas. This may delay control, input and frame
handoff; its responsiveness and device correctness were not qualified.

At the private PC110 fixed-startup checkpoint, baseline/candidate/candidate/
baseline elapsed milliseconds were 20,137 / 20,247 / 21,405 / 17,458 on x64
and 21,938 / 23,632 / 23,701 / 26,237 on x86. Each reported 10,000,000
instructions, 79,964,294 ticks and PC `0003dbc5`. Both x64 comparisons favor
the baseline; the x86 direction changes. Host variation is visible and the
checkpoint does not prove full-state or Game equivalence. These measurements
do not justify delaying host rendezvous or adopting the candidate.

An initial probe changed the Standard tick cap instead, but PC110's product
binding initializes Turbo, so it did not test the candidate branch and is not
PC110 performance evidence. The corrected Turbo probe changed only task scratch;
production and released artifacts remained unchanged. Corrected log SHA-256:
`86DAF7B486E70D19D8DE0760DB30D03C5DF8CA8F576EDE81E1A81CD1CA438DF9`.

## Preview's overwritten CPU snapshot

The existing lexical preview copies the beginning of its instruction record,
including `oldcpu`, before `ExecInit` calls `ExecBoundary` and overwrites that
snapshot from the preview CPU. A scratch candidate split that existing copy
around `oldcpu`, retaining the remainder and the existing preview/decoder path.
This removes bytes subsequently overwritten but adds a copy call. It has not
been poisoned or qualified for all profiles, faults, preview callbacks or
debug observation; it is not a proven equivalent implementation.

At the private PC110 startup checkpoint, baseline/candidate/candidate/baseline
took 18,696 / 18,285 / 17,947 / 18,312 ms x64 and 23,442 / 23,754 / 24,838 /
23,097 ms x86. All reported 10,000,000 instructions, 79,964,294 ticks and
PC `0003dbc5`. x64 favors the candidate while both x86 pairs favor baseline.
These are noisy startup samples, not full guest-state or Game equivalence.
The reduced byte count does not compensate for a demonstrated risk of slower
x86 execution and added copy complexity, so no production change is admitted.
Log SHA-256:
`C3AC098BEC4F4C5106863A533BD98B93C63E3C78990CB5660668E9BFF4106E1A`.

## Retirement timing capture and second loop

On the private 10,000,000-instruction PC110 startup checkpoint, a task-only
wrapper counted 20,002,370 CPU timing captures on x64 and 20,008,794 on x86.
Sampling one call in 256 recorded 7,084,180 TSC units across 78,135 samples
and 3,272,980 across 78,160 respectively. Both runs reached 79,964,294 guest
ticks and PC `0003dbc5`. These nonserialized, instrumented intervals are not
exclusive whole-machine costs and include wrapper and return-value overhead.

A separate x64 call-site counter recorded 10,008,549 calls after CPU refresh
and 10,008,541 in the pending-retirement path. The latter reads event ticks
after the loop's stop, refresh and bus-hold checks. A partial tick budget can
carry pending retirement across Core calls, so a previously captured local
value is not automatically available or safe there. Count and sample logs
SHA-256 respectively:
`B5D4E833D65FB0516E8DB4599EF5E3C6E4E682FE3B9CC0417217C0C1571E51C5`,
`AC6D9E7C0BBB61F5A3C0C7A9CE57C697ACC17D549EFBC5F7D2CCA916EDCD0640`.

To bound the possible gain, an intentionally invalid scratch candidate took
the immediate retirement route for an unbounded tick budget when no external
or event ticks were present. This skips the second loop's checks and is not a
production proposal. Baseline/candidate/candidate/baseline were 17,023 /
16,416 / 16,804 / 16,821 ms x64 and 20,897 / 22,580 / 22,480 / 21,749 ms
x86. At the same instruction count, baseline reported 79,964,294 ticks and
PC `0003dbc5`, whereas the altered route reported 79,964,327 ticks and PC
`0003e3b2`. The divergent endpoint proves these runs are not equivalent and
does not identify the exact mutation causing divergence. They do not show a
stable gain worth bypassing the boundary; no production code was changed.
A correct future design must retain all stop, reset, hold, event and partial
budget boundaries, with explicit fault/debug and asynchronous regression.
Invalid-ablation log SHA-256:
`2F3F1D782F189134EAF772F56BC758B226232AA2A04888ABEEFE79280D2243FB`.

### First observed divergence in the invalid ablation

A separate x64 pair logged every Core quantum for the original and invalid
immediate-retirement variants, using private copies of the same CMOS and disk
inputs. Their first 1,877 recorded boundaries matched. The next boundary was
a hard CPU reset request: baseline had retired 7,691,597 instructions at
62,079,603 guest ticks, while the ablation had retired 7,691,598 at 62,079,614;
both returned the reset vector `fffffff0`. A second pair instrumented the two
existing reset branches and confirmed `X86_CORE_CPU_RESET_HARD` was consumed
through the reset-request branch in each run, rather than the shutdown-reset
branch. It does not establish which device raised the request.

The ablation has already been rejected. This trace explains why preserving the
pre-retirement reset check is necessary; it does not prove that reset handling
alone accounts for every later state difference, or qualify a fast path. Output
logging perturbs timing and these are product-run quantum boundaries, not a
complete per-instruction comparison. No production code or user media changed.
Instrumented baseline/candidate log SHA-256:
`3BA725043870E05188D7F9FA5F38F2772B9AE60A8DBA0269D752134C8AC3CA06`,
`AAAE5C9656E21126C4E7E849CB6C565CBBD6EFF3E2D3ADD690FF33DF20840332`.

### Returning timing with CPU refresh

A separate scratch candidate changed the existing CPU refresh return value to
the already computed timing result and used it at its sole production Machine
caller, avoiding the immediate post-refresh capture. The later pending-retirement
capture and all event boundaries remained. This changes the existing CPU
function's return ABI and its null-result contract; the numerous test callers
and all consumers were not rebuilt or qualified, so it is not an admitted API
change. No persistent state or additional execution path was introduced.

Baseline/candidate/candidate/baseline at the private PC110 startup checkpoint
took 18,111 / 19,304 / 19,047 / 18,512 ms on x64 and 23,685 / 25,062 /
23,449 / 24,819 ms on x86. All reported 10,000,000 instructions, 79,964,294
ticks and PC `0003dbc5`. Both x64 comparisons favor baseline, while x86 changes
direction. The single ordered sequences are noisy; matching the checkpoint is
not full CPU, fault or debug equivalence, nor a Game result. The extra ABI
change is not justified by these observations. No production code changed.
Log SHA-256:
`3F85282FEC7E377A622F2D49AB24EE6BC2985B2FCD5DF6896F1F27A97B731B6C`.

### Borrowing timing from CPU refresh

A further scratch candidate returned a pointer to the CPU's existing timing
result from the current refresh call and used it in the Machine caller,
avoiding the immediate struct capture without adding mirrored state. This
would change the public return ABI and add a borrowed-pointer lifetime contract;
callbacks, asynchronous mutation and all CPU consumers were not qualified.
The pending-retirement capture and its existing boundaries remained unchanged.

Baseline/candidate/candidate/baseline on the private startup checkpoint took
18,919 / 18,108 / 17,670 / 16,232 ms x64 and 21,438 / 22,601 / 22,230 /
21,915 ms x86. All reported 10,000,000 instructions, 79,964,294 ticks and PC
`0003dbc5`. The x64 direction reverses, and both x86 comparisons favor the
baseline. Host variation is substantial; matching the checkpoint does not
prove full-state or Game equivalence. These results do not justify the ABI and
borrow-contract change, so it was rejected without changing production.
Log SHA-256:
`866C8354CD6A6845689C4E010CF8FF377482F145C9817C90679EBE82AA773D25`.

### Local pending-event reuse probe

A scratch-only Machine candidate retained the event-tick value captured after
CPU refresh in a local variable and used it for the next pending-retirement
iteration; a pending retirement entering a new Core call used the original
capture. It retained the loop and its stop/refresh/hold checks. This prototype
does not invalidate the local value when an intervening attachment or time
callback changes CPU timing, so equivalence has not been established and it
is not an admissible implementation.

Baseline/candidate/candidate/baseline at the private PC110 startup checkpoint
took 17,047 / 17,506 / 17,801 / 18,088 ms x64 and 21,465 / 23,121 / 22,217 /
22,884 ms x86. All reported 10,000,000 instructions, 79,964,294 ticks and PC
`0003dbc5`. Both widths change direction across ordered pairs; these noisy
samples do not establish state or Game equivalence. They do not justify adding
local cached state and its invalidation proof. No production change was made.
Log SHA-256:
`88F19DF96519710DD5CE1BC7186262E1938EFCF0ED444F1CEF61A7B61683CF29`.

## Lexical preview using an already retained prefetch

An instrumented x64 startup probe found 971,468 lexical previews at the
10,000,000-instruction / 79,964,294-tick / `0003dbc5` checkpoint. Of these,
373,320 had a retained prefetch range at the target and 360,766 could be
decoded from that range by the existing lexical scanner. Counting and extra
scanning perturb execution; the probe covers startup, not Game. It does not
prove that bypassing preview preserves side effects or fault semantics.
Count log SHA-256:
`827E760E05BC4A4E243F14F91CEE56F44148CC99C22110061FCACC5901AC2CF0`.

A scratch candidate tried that same scanner directly on the retained range,
falling back to the existing preview for incomplete or NEC emulation forms.
It did not change the instruction executor, but introduced an additional
preview branch whose behavior across all profiles, page faults, code mutation
and asynchronous entry was not qualified. Baseline/candidate/candidate/
baseline times at the same fixed endpoint were 17,428 / 17,633 / 17,406 /
17,921 ms on x64 and 21,284 / 23,115 / 22,749 / 25,856 ms on x86. Both
widths reverse direction across the two ordered pairs. These noisy samples do
not demonstrate a gain or full-state or Game equivalence, so the additional
branch is not admitted. No production source or artifact changed. Comparison
log SHA-256:
`CFBB0EC8C7B3C5DF037F6227654BCFDFC0CD005215B4487C52615CAC9E9F2D0C`.

## Contiguous instruction-timing reset

This substantially repeats the S34 candidate documented in
`t582-s34-current-profile.md`. The earlier comparison was missed by the
targeted search; this rerun was unnecessary and is retained as a repeat, not a
new optimization direction.

`ExecBoundary` clears a contiguous group of timing counters individually. A
scratch candidate replaced those assignments with a bounded block clear ending
before `memory_lock_active`; previous-instruction facts and lock state remained
outside the range. This reduces source repetition but also clears padding, adds
a layout-dependent range and was not qualified for every CPU, fault, debug or
preview path.

At the private startup checkpoint, baseline/candidate/candidate/baseline took
19,395 / 17,494 / 18,492 / 20,539 ms on x64 and 23,243 / 22,902 / 23,459 /
20,972 ms on x86. All reported 10,000,000 instructions, 79,964,294 ticks and
PC `0003dbc5`. A separate x86 run pinned to one host core gave 21,908 / 22,469 /
25,190 / 21,837 ms in the same order. Both pinned pairs favor baseline. These
measurements are noisy startup observations, not complete state or Game tests.
The smaller source is not worth the observed x86 regression and added layout
coupling, so no production code was changed. Log SHA-256, unpinned then pinned:
`3F285651E4AE8737A9945C74A426B524A4A3B8ECF7C326FBB57A38E5C3956AB6`,
`E8A50019DF3526DD26FE8E1F0083F7E35C2A1F8CE49487DF5D4CD37E521DB380`.

## CPU compilation inlining policy

A scratch comparison compiled only the existing CPU instruction translation
unit with GCC `-O3 -fno-inline-functions`, versus the normal `-O3`, and linked
the same cached remaining libraries. It changed no source or project build
settings. Baseline/candidate/candidate/baseline at the private PC110 fixed
startup checkpoint took 19,295 / 19,613 / 19,257 / 18,630 ms x64 and 22,451 /
22,103 / 22,905 / 23,113 ms x86. All reported 10,000,000 instructions,
79,964,294 ticks and PC `0003dbc5`.

The scratch executables shrank from 3,088,881 to 3,027,172 bytes on x64 and
3,249,782 to 3,188,999 bytes on x86. Both x64 comparisons favor baseline; x86
differences are small. These noisy startup samples do not prove full-state or
Game equivalence, nor a portable compiler-policy gain. The size reduction is
insufficient reason to risk the observed x64 slowdown; the flag is not adopted.
Log SHA-256:
`518798EFA2A678AED5B326BED936631E2C4FE426AAFF6557CD8CBE509FB7A8A0`.

## PIC rotating-priority bit search

A scratch-only candidate replaced the existing scan of a nonzero rotated PIC
register with three fixed binary bit-group checks. Rotation and the original
callers remained. Eight lines are a fixed hardware width: the change reduces
the worst number of bit checks but increases the common first-bit-hit case,
and is not an asymptotic whole-machine improvement. It has not been qualified
for all PIC priorities and reset/cascade states.

At the private startup checkpoint, baseline/candidate/candidate/baseline
took 22,552 / 21,871 / 22,054 / 21,649 ms x64 and 27,589 / 27,202 / 26,046 /
27,078 ms x86. All reported 10,000,000 instructions, 79,964,294 ticks and PC
`0003dbc5`. The x64 direction changes and x86 differences are small against
host variation; this is not complete-state or Game equivalence. The extra
checks in the best case and lack of clear cross-width gain do not justify the
alternative, so no production change was made. Log SHA-256:
`A2CB739056ACB4604FFDDF60FCDA1925F5717E137121FE4FE477138A45361CB0`.

### Keyboard deadline duplicate-callback boundary

Source inspection narrows the previously noted duplicate observation. In
`x86_kbc8042_ticks_until_event`, the serial-delivery predicate only calls
`keyboard_inputs` when `keyboard_serial_count` is nonzero. The response
predicate only reaches that same serial-delivery predicate when a delayed
response exists and its status-poll count is zero. The unconditional snapshot
in between also supplies BAT and repeat decisions. Thus an idle controller
with no queued serial bytes performs one snapshot in this deadline calculation,
not three. Reusing one snapshot could reduce callback calls in a queued-input
case, but its frequency and benefit in the actual Game workload are unmeasured;
the separate advance operation may mutate keyboard state. No shortcut across
that boundary or production change is justified by the existing sample.

## SMM memory-view invalidation cost

The PC110 firmware changes its 64 KiB SMRAM memory view on SMM entry and
exit. The existing CPU invalidation scans 128 sets by four ways for such a
range and also invalidates prefetch. A task-only link wrapper timed the real
`x86_core_cpu_execution_invalidate_memory_view` call for that exact range,
without changing it; private firmware and media copies were used. At the fixed
10,000,000-instruction startup checkpoint, x64 recorded 12,274 view calls,
12,218 SMRAM calls and 15,555,668 host TSC units in those SMRAM calls, versus
x86 12,264, 12,208 and 25,943,167 respectively. Elapsed times were 21,055 and
26,127 ms; both ended at 79,964,294 guest ticks and PC `0003dbc5`.

All SMRAM calls were timed, so instrumentation and nonserialized TSC affect
the figures; they are not exclusive CPU time or a Game/RDP measurement. The
counts and measured intervals do not support treating the 512-slot scan as the
large Machine-path bottleneck. Nor do they prove it may be removed: the same
range decodes differently across the transition and the invalidation includes
prefetch. No production code or binary changed. Log SHA-256: `155052965FDBC7FA50435EBDD6B3F1D48A277F849EDD5DCE8CB49AAB081F721B`,
`0C470AEC656D6069CFA995C45624087D34D460CF02F3A80E8B67D48CA10678B0`.

## Repeated 486 timing validation: scratch upper bound

The standalone nominal, fallback, memory and decode timing helpers each check
their public inputs and instruction state. A scratch-only upper-bound candidate
removed these checks from all four helpers; this breaks their standalone error
contract and is not an admissible implementation. It retained the timing
calculations and did not modify the production source or published binaries.

At the private 10,000,000-instruction startup checkpoint, in
baseline/candidate/candidate/baseline order, x64 times were
20,802 / 21,438 / 21,098 / 20,633 ms, and x86 times were
28,775 / 25,626 / 25,447 / 25,495 ms. All reached 79,964,294 guest ticks and
PC `0003dbc5`. These two sequential pairs show no x64 benefit and an x86
result sensitive to ordering; they do not prove complete state equivalence,
Game or RDP performance. Even this invalid removal provides no convincing
cross-width basis for adding validated-inner wrappers and increased interface
complexity. Public checks must remain. Log concatenation SHA-256: `0F85E01C8F5558DDFB50478D36D56820C2A12943C84D6EA849EC47B019E40F9F`.

## Startup-first acceptance correction

The owner clarified that usability means booting into DOS/Personaware and
ordinary operation promptly; the game is not the objective. In the existing
private-media, Machine-input run, the first recognizable Personaware menu
was sampled at second 211. This is a one-second frame-sampling marker rather
than an exact production or native desktop/RDP boot time. The complete Game
marker at sample 263 is not the startup acceptance endpoint.

To identify where to investigate next without another long guest run, the
existing QUANTUM records were grouped by the final sampled PC's 4 KiB region.
Intervals of at most five seconds, from the first recorded quantum through
211 seconds, total 210,461 ms. The largest endpoint groups were `00038xxx`
47,044 ms, `000FAxxx` 32,729 ms, `000FExxx` 31,003 ms, `000F6xxx`
24,915 ms, and `000FCxxx` 16,926 ms. These are endpoint classifications, not
continuous PC sampling or exclusive cost attribution: execution may move
between regions during an interval. They indicate that startup work is
distributed, so neither the later Game-drawing opcode mix nor one short REP
startup checkpoint is a sufficient sole acceptance workload. Inspect these
phases and guest progress using the existing source records before selecting
a semantically valid optimization. Log SHA-256: `0444C1460B5D1D5F0300D11CA9E24ED7F9F52BED1AA3FC1A755DC7C875339DB5`.

## Existing boot-stage observations

The earlier S19 private-media run captured text or graphics approximately
every ten seconds. Its text output at samples 1 through 14 (approximately
20--150 seconds after the capture loop started) showed `20096 KB OK`; sample
15 (approximately 160 seconds) first showed `Starting PC DOS...`. The sample
19 image, approximately 200 seconds, showed DOS startup and PC DOS Power
Manager text; the image was visually checked again. Sampling times are coarse
and include capture, logging, media and older-build effects; a persistent
message does not prove the CPU was stuck in a single firmware routine. This
run and the newer 211-second Personaware observation are different builds and
must not be subtracted as a precise stage breakdown.

In the earlier run, QUANTUM endpoints from 20--60, 60--100 and 100--150 seconds
advanced 127,505,232, 126,076,034 and 161,246,275 guest ticks respectively,
while retiring roughly 22.44, 22.06 and 28.18 million instructions. Their
most common sampled endpoint was `00038000` (12/29, 11/29, 16/38 samples).
An endpoint is not an instruction trace and cannot by itself establish what
percentage of time was spent in a routine or why the firmware waited. The
practical conclusion is narrower but useful: prioritize the firmware/POST to
DOS transition, not only Personaware drawing. The next candidate must preserve
the firmware and device event contract and be evaluated against the actual
boot endpoint and both host architectures. Log SHA-256: `FA3E2794D404D8CC1745D4556B804B711258A6A3B65BD683B09743300E904ECF`; checked image
SHA-256: `0D9C178FA01458D66B9B6AE0C76218315977BA487E6B72399D43A1EA31BD84A8`.

## Startup firmware wait classification

### Machine-return endpoint sampling bias

A task-owned x64 counter examined every existing Machine run return during
the first 30,000,000 retired instructions, using the same private media and
unchanged production libraries. It completed in 60,745 ms at 195,624,840
guest ticks and PC `000f6440`. Of 32,561 returns, 27,155 ended at `00038000`;
none returned zero retired instructions. Those `00038000` returns accounted
for 8,052,934 retired instructions, about 26.8% of this sample, despite
being 83.4% of all returns. Sampling every 128th return yielded 213 such
endpoints out of 254, also about 83.9%. The earlier logger additionally applied
a wall-time filter, so this is not an exact reproduction of its selection.

This demonstrates that return endpoints are strongly weighted by how often
the Machine returns: they cannot be interpreted as a time or instruction
distribution. Even the 26.8% is the instruction count in entire calls ending
at that address, not a count executed in SMM; this observation cannot by
itself attribute 26.8% to the entry handler. The result directs the next
investigation toward actual per-instruction or sampled cost, not a presumed
83% SMM hotspot. It covers only early boot, x64 and private media; no guest
semantics, source, user media or released EXE was changed. Diagnostic output
SHA-256: `92C6AF178D01603C110631A346FABC86F4EA82B68EF743C12CA248F2D098804D`.

An independent counter wrapped the existing CPU refresh in the same x64
30-million-instruction private-media checkpoint. It observed 23,504,229
refresh calls beginning outside SMM and 6,500,982 beginning inside SMM,
plus 27,155 entries and 27,155 exits. The run took 62,311 ms and reached the
same 195,624,840 ticks and `000f6440` endpoint. About 21.7% of observed
refresh calls began in SMM; this is not an exclusive time share, and refresh
calls include asynchronous entry and the brief stop interval. Instrumentation
also changes host cost. The entry count equals the preceding run's `38000h`
endpoint count, consistent with early Machine returns at SMM entry, but the
separate runs do not establish a one-to-one causal trace. This observation
narrows the possible role of SMM in this early phase and rules out treating
endpoint frequency as executed-instruction frequency; it does not establish
the cause of the later DOS/Personaware delay or a sufficient optimization.
Diagnostic output SHA-256:
`AB539AB0E67C20A1228380CD37780E41D80889F520FA3969560DA031685AA4D0`.

A second x64 run at that checkpoint sampled one CPU refresh in 256 using
nonserialized host TSC reads. Samples / accumulated TSC units were normal
91,692 / 230,329,096, SMM-internal 25,260 / 59,984,950, entry 118 /
4,689,380 and exit 119 / 5,384,424. Entry and exit together account for about
3.4% of the sampled CPU-refresh intervals, while SMM-internal accounts for
about 20.0%. This does not include the rest of Machine or prove exact time
shares: fixed-interval sampling may alias guest loops, TSC is not serialized,
and instrumentation can alter code layout. It nevertheless gives no basis to
expect eliminating SMM entry/exit alone to remove most of the startup delay.
The 59,631 ms diagnostic ended at the same 30-million-instruction checkpoint;
it does not measure the complete boot, x86, or native desktop. Guest log
SHA-256:
`3DD90461168C449832AD2BDDF21E063A167C7C4F8C8FCE2A929F6C2618F8A8EC`.

The earlier 20--150 second quantum-endpoint sample included addresses in two
firmware delay routines that poll port 61h refresh status and one routine
that polls the floppy controller status port. The first delay's successful
return is already established by T580 S7. An endpoint is not a time profile,
so those locations alone do not prove that either device is defective or
account for a specific share of startup time.

A new task-only x64 wrapper counted actual FDC main-status reads without
changing the return value or event scheduling, using the existing PC110
Machine path and private copies of media/CMOS. At 30,000,000 retired
instructions it reported 56,245 ms, 195,624,840 guest ticks and PC `000f6440`;
55 status reads returned 30 instances of `80h` and 25 of `d0h`. The probe
exited zero. These reads are intercepted calls, not a proof that every FDC
access was intercepted; it covers this early startup checkpoint only, not
the entire 150-second phase, later waits, x86 or desktop/RDP behavior. It
does not support diagnosing an FDC status storm or altering FDC status to
accelerate this early phase. Continue assessing the broader startup path and
actual device ordering; no production code or release asset changed. Guest
log SHA-256: `A56BFA293BD2CF2B4553853AE4D1A9933399930CD4181B609B476B90D446E4F1`.

## Refresh-status division at the boot wait

PC110's existing port-61 status derives its refresh bit from elapsed guest
ticks divided by the configured period. A task-only candidate used a 32-bit
division when ticks fit in `lib_u32`, falling back to the original 64-bit
division otherwise. The quotient and source clock are mathematically the
same, but complete edge cases and runtime side effects were not qualified.

At the x86 private-media 30,000,000-instruction checkpoint, baseline and
candidate took 72,496 and 73,454 ms and each reported 195,624,840 ticks and
PC `000f6440`. One ordered pair is subject to host noise and does not establish
full-state equivalence or Personaware/desktop performance; it supplies no
evidence to justify the extra branch. No x64 run, production change or
artifact publication followed. Concatenated logs SHA-256: `6FE7A6D2EB8AE6D4E927CBE52BA5C27F9F073F31D61726F810E95541D72C06FE`.

### Extended FDC-status observation

The same x64 interception extended to 70,000,000 retired instructions,
completed in 138,893 ms at 420,747,119 guest ticks and PC `000fc84b`. It
recorded 1,478,619 intercepted main-status reads: 43 returned `80h` and
1,478,576 returned `d0h`. The previous 30-million checkpoint recorded only
55 reads, so the high-volume condition is later in boot; the earlier count
must not be generalized to the full firmware phase. The current FDC model
returns `d0h` for its result phase, and the observed BIOS routine waits for
the upper bits corresponding to `80h`. This establishes a concrete pending-
result/wait interaction, not whether the model or guest is wrong: the command,
result reads, interrupts, device ordering and eventual exit need a trace.
No guest timing or status may be forced from these counts alone. This is one
instrumented x64 private-media run, not x86 or a desktop test. Log SHA-256:
`A5C2F3B27B725C9AFEE337AB2FDE0A5A9F48D8E7DC575C6231972D9D7E869093`.

## SMC enhanced FDC command handshake

A private-media x64 trace at the 70-million-instruction boot checkpoint shows
that firmware writes `12h`, after which the current 8272 model produces an
invalid-command result. The firmware expects a parameter and polls command
readiness while the result remains unread. The same trap repeats; the trace
recorded 1,478,706 status reads. T580 S49 identifies the physical PC110 chip as
SMC FDC37C665IR; the already archived original manual, printed p. 62, describes
`12h` as the two-byte Perpendicular Mode command with no result phase.

A scratch implementation that merely consumes `12h` and one parameter
then exposed the next absent enhanced command, `13h`; it too was rejected by
the old model and caused another result/wait. A second scratch implementation
consumed `12h` plus its parameter and `13h` plus three parameters. In its
70-million-instruction run, status reads fell to 160, and the guest proceeded
through subsequent ordinary FDC commands. The two runs ended at different
guest states: baseline 135,774 ms / 420,747,119 ticks / `000fc84b`, candidate
137,764 ms / 432,495,511 ticks / `000fa5ab`. These numbers do not establish
end-to-end startup speed, equivalent state or a time gain at equal work.

The observed guest writes `12 00` and `13 00 30 00`. The manual defines
Perpendicular Mode's drive and reset behavior and CONFIGURE's FIFO, polling,
implied-seek and precompensation fields. Simply consuming those commands as
no-ops is a diagnostic shortcut, not an acceptable implementation. A correct
production design needs a chipset-specific command capability, proper supplied
field semantics/reset and coverage for existing 8272 machines, plus actual
boot and x86 qualification. No firmware, media, public API, production source
or published binary changed. Log SHA-256 baseline / one-command / two-command:
`3A9E3454559B1BD3DC630EBFD6993F7C86B7F52FEF3419C736CB9FD94DE48593`, `B03B4BD939FE2BFCEBEB95EEEB8883FC1457D164F8BEB4B54540C36C035CBC70`, `F8D2DAB69A515513C11DD5ED5EC19B803D2EEB466BA6BFD10362BA84E03F2D48`.

### End-to-end menu check rejects handshake-only shortcut

The temporary two-command handshake was linked into the existing x64
private-media `s26-menu-time/guest.c` monitor, with the same CMOS seed and
unchanged one-second menu criterion. It exited unsuccessfully after 405.69
host seconds without `MENU-READY`; its last recorded quantum reached
238,558,871 retired instructions and 1,462,783,007 guest ticks. The earlier
run using that same menu detector recorded `MENU-READY sample=216`, but used
an older build and separate execution, so these are not a controlled paired
benchmark or proof of a specific regression. They do prove the scratch
handshake does not qualify as an end-to-end boot improvement.

The 70-million trace shows that the handshake allows the guest to continue
into further floppy seek/read activity with no attached floppy, including an
unsuccessful read and subsequent controller reset. It intentionally discarded
CONFIGURE and perpendicular parameters, so no conclusion may be drawn that a
correct SMC controller has the same behavior. In particular `13 00 30 00`
disables polling and FIFO in the real device; the scratch no-op left the old
polling policy unchanged. This is not an admissible production implementation.
No source, assets or owner media changed. Candidate log SHA-256: `F6E789A2B8CBF57C385EBC51A50A9812E6042524EEFC8356F4058DE326ABA34F`.

### Polling and unlock scratch follow-up

A subsequent task-only candidate additionally applied the observed
`CONFIGURE` polling-disable bit and returned the unlocked `LOCK` status. It
retained a temporary sentinel in the existing polling field and still does
not implement the complete SMC commands, FIFO, lock/reset, or perpendicular
media semantics; it is not production code. At 70,000,000 instructions, it
recorded 150 intercepted FDC status reads, 432,496,591 guest ticks, PC
`000fa5b4`, and 123,391 ms. The different guest state and host variation
prevent comparison to the original 8272 baseline as equivalent work.
A private-media full menu check is running separately. No production
source or release artifact changed. Log SHA-256: `4926C928D948D1B261A007C71D2DB9241E8A0749DD9D6E495042592FC37A24D1`.

### Full menu check with polling-disable and unlock approximation

The second diagnostic additionally approximated the observed polling-disable
and unlock behaviors, but not FIFO, implied seek, locked-reset or full SMC
semantics. The same private-media x64 menu harness ran for 405.67 seconds and
exited with no menu marker. The last recorded quantum and guest state are in
the hashed log. As with the prior approximation, this is not a proper SMC
implementation or a matched controlled benchmark; no causal claim about any
one missing feature follows. It does show that these partial additions do not
qualify as a PC110 boot improvement, so none is installed in production.
The next investigation must identify the full SMC/firmware/device sequence
and distinguish whether startup time is caused by a specific device contract
or general execution throughput before constructing another candidate.
Log SHA-256: `E0B8BE22CD5C86F03627AC614C11ADA34D97228C7406D719D14E88381B29F9F6`.

### Menu-detector follow-up and CMOS scope

The polling/unlock scratch candidate was rerun with explicit frame capture
at 180, 220, 260, 300, 340 and 380 seconds. Captured text frames still
showed `20096 KB OK` throughout those samples; the run exited without its
menu marker after 405.37 seconds. Thus the previously missing menu marker
was not merely a graphics-pixel mismatch in these captured intervals. This
is one instrumented x64 private-media run of an incomplete SMC approximation;
it does not identify the missing device behavior or establish a real hardware
expectation. No production code or user media changed. Log SHA-256: `41C42B4B081B874BFF61EE69F694B409D568602EBB400FCA44399E71C7A59918`.

The private probe's copied DSTN CMOS seed has bytes `10h=40h` and `14h=25h`;
the current tracked product asset `mypc110-dstn.cmos` is empty. The existing
PC floppy mapping associates `40h` with a 1.44 MB drive, whereas the PC110
board declares no installed floppy drives. These facts identify a configuration
mismatch to test, not authority to rewrite user CMOS or proof that the BIOS
uses the standard meaning in every path. A follow-up must use private copies,
recompute the firmware-checked checksum if changing CMOS bytes, and compare
otherwise identical boot configurations.

### Private no-floppy CMOS comparison

For a bounded follow-up, the exact same incomplete SMC candidate executable
and the same private master/slave media were used. Only a private DSTN CMOS
copy changed: `10h` from `40h` to `00h`, equipment bit zero cleared in `14h`,
and the existing `10h..2dh` checksum recalculated to `037bh`. The modified
128-byte seed SHA-256 was
`D4C738483D0EAA369AE31253C2B7C1491CBA5A27278668586DD9CCC689968D32`.
No tracked or owner CMOS was modified.

The menu harness exited after 404.98 seconds without detecting the menu,
similar to the 405.67-second unmodified-seed run of the same approximate
candidate. This rules out the private seed's floppy declaration as a
sufficient standalone fix for this particular candidate and detector; it
does not prove how the PC110 BIOS interprets those bits, that the candidate
is correct, or how unmodified production boots with a different CMOS. No
production change or artifact publication followed. Log SHA-256: `06F7A819E39E44E722D89418F0615653275FB75832FF53E3603C090F318DDCC8`.

### Current-production CMOS comparison

The previous no-floppy test used the incomplete SMC candidate and could not
represent current production. A new x64 pair used the identical executable
(SHA-256 `AD6DDFEDAF3E818FE4C20BF48791AB96CF0AC14FE276AF112A5143CFEB464C1D`), unmodified current production libraries, private copies
of the same media, and the same one-second Personaware menu detector. The
private no-floppy CMOS ran first and reached `MENU-READY sample=209` in
212.71 seconds; the original private CMOS ran second and reached sample 201
in 204.48 seconds. Both returned zero. The last preceding quantum recorded
683,316,533 ticks / 134,727,848 instructions and 684,593,002 ticks /
135,225,679 instructions respectively. These are separate guest executions
and one ordered pair; host drift, one-second sampling and possibly different
firmware behavior prevent a precise speedup or equivalence claim. This result
does not support editing user CMOS to solve startup performance. It also does
not establish the behavior of an empty first-run CMOS or native/RDP UX.
No production source, user configuration or release asset changed. Logs
SHA-256: `F7F8FFD460DB4878DFEAE4D48A7FAE1EAE22B3D78668338FD0369737220FAD30`, `0CEA7D46730220D44F4BFD3892B586365232941E8FE62C935DA33EC1C3D34133`.

## Port-61 route attribution during firmware startup

A task-only x64 build wrapped the existing CPU I/O read path and sampled the
existing exclusive-provider lookup independently, without removing either.
At the 70-million-instruction private-media startup checkpoint it reached
420,747,119 ticks and `000fc84b` in 110,677 ms. Port `61h` had 8,634,777
reads, 33,730 sampled read intervals and 60,211,026 accumulated TSC units.
The corresponding exclusive-provider lookup had 8,634,778 calls, 33,730
samples and 20,331,199 TSC units. The lookup interval is nested inside the
read interval and includes its own timer overhead; the roughly one-third
ratio is not a whole-machine percentage. Deterministic sampling, nonserialized
TSC, compiler layout and early-boot scope limit interpretation.

The read path subsequently traverses wired-OR contributors in its established
order; the registration rollback uses the current linked-list head as its
checkpoint. Any move-to-front, index, or one-pass design must preserve both
contracts, including registration failure and callback ordering. Counts alone
do not justify new mirrored lookup state or changing registration semantics.
No production code or release artifact changed. Log SHA-256: `30C206965960FF7A9E3DF797139B949C5546D2FBB4FD6B0AA6215CDBF4181024`.

### Exclusive-port move-to-front upper bound

A scratch-only x64 candidate moved an accessed exclusive read provider to
the head of the existing registration list; wired-OR entries were not moved.
This changes registration-list ordering and has not been proven safe for
registration rollback, reentrant callbacks or every wired-OR lifecycle; it is
not an admissible production implementation. It adds no index or new state.

At the private 30-million-instruction boot checkpoint, baseline/candidate
then candidate/baseline took 51,787 / 50,157 then 56,709 / 54,503 ms. All
reported 195,624,840 guest ticks and PC `000f6440`. The sign reverses and
the timings are sensitive to host drift. The result neither demonstrates a
stable gain nor validates full-state equivalence, later boot, x86 or desktop
behavior. It does not justify increasing route/rollback complexity; no
production change or artifact publication followed. Log SHA-256: `A19D8C75408F1991119CA904D6FEB78BE97D4AD551656A7903BDE47C8B89C83E`.

### Exclusive-provider stop is not a valid route invariant

Although wired-OR routes must be registered after an exclusive route, route
owners may be removed independently. A task-only x64 reproduction registered
an exclusive read and a wired-OR contributor on the same port, removed only
the exclusive owner, then installed a replacement exclusive route. The actual
production route returned `6` (replacement `4` OR contributor `2`). A
scratch variant that stopped wired-OR traversal at the new exclusive route
returned `4` and failed the same probe. Replacement registration inserts the
new exclusive route ahead of the surviving contributor. Thus a shortened
traversal based only on list position is observably wrong; it cannot be used
as the proposed no-state optimization. This is an internal port fixture, not
a PC110 end-to-end or all-callback proof. Neither production code nor release
assets changed. Probe SHA-256: `05DAF704C36D8D2AADEEC85853EA4D2EE8C6F7A24D311FC108FA9C1C06254663`.

### Wired-OR traversal upper bound at the later boot checkpoint

A task-only x64 candidate omitted the entire wired-OR read traversal, which
is incorrect for general machines and is not an implementation. This extends
the earlier ten-million-instruction upper-bound experiment into the firmware
port-polling interval. The private-media baseline/candidate reached 30 million
instructions in 50,625 / 49,504 ms, both at 195,624,840 ticks and PC
`000f6440`. One ordered pair cannot exclude host drift or establish complete
state equivalence. Even omitting the full traversal only changed this sample
by about 2.2%, insufficient evidence for adding a route index or state to
solve the roughly three-minute boot. The actual ordered wired-OR callbacks
remain required; no production code or release artifact changed. Concatenated
log SHA-256: `ED7910C15B37576DAFDBADD3B399E120BBB8DDB52D3772F1305A418CD9C5E15A`.

## Existing instruction-record prefix-count experiment

A task-only x64 candidate reused the existing NEC prefix counter field for
non-V30 decoded prefixes and returned the bounded count to the timing helper
when nonzero, retaining the scanner for zero. This tests a broader reuse than
passing a local count among the four 486 helpers, and does not add a field.
It is not an eligible implementation: prefetch can populate bytes before they
are consumed, and fault, preview, synthetic timing fixtures, mode transitions
and all CPU profiles have not been proven equivalent. The field's NEC-specific
meaning would also need a proper single-owner redesign rather than this reuse.

At the private ten-million-instruction startup checkpoint the directly
compiled baseline/candidate took 18,709 / 19,391 ms, both at 79,964,294 ticks
and PC `0003dbc5`. One ordered pair is noisy and not a complete-state test,
but it does not justify expanding this risky candidate into x86 or full
receiving work. No production code or artifacts changed. Log SHA-256: `AC37A93A37FBD05EEFD10331A73699EC62FE71A6F2E3D63E1FC6385F331D5D70`.

## Longer enhanced-FDC diagnostic trace

The incomplete polling/unlock approximation was extended to 100 million
instructions: 162,859 ms, 614,943,578 guest ticks, PC `000fa5ab`, and 252
intercepted status reads. It shows repeated recalibrate, seek and read
commands interspersed with CONFIGURE; `reset value=01` records a reset-line
write, not necessarily a reset transition. This is one instrumented x64 run,
not a valid controller implementation or an end-to-end startup test.

Comparison with the two-command scratch trace exposes a qualification:
at tick 65,423,277 the firmware reads the diagnostic UNLOCK result as `80h`
in the two-command approximation and `00h` in the polling/unlock variant.
At the later 284-million-tick point the former issues standalone `12h`
before CONFIGURE, while the latter proceeds directly to CONFIGURE. Thus the
earlier `12h` observation is conditional on controller responses, not an
invariant of the firmware path. Later log entries showing byte `12h` inside
an `e6h` command are parameters, not independent commands. These traces
do not isolate which differences explain the unsuccessful menu run; no
production behavior or user media was changed. Longer trace SHA-256:
`9A76522398A33412C05B6C201419014663DCE6A0D0747BD7B0E152F1D7696436`.

The locally available PC110-EMU reference is not an equivalent controller
comparison: its `pc110_core.c` describes its FDC as a placeholder and selects
MSR and data responses using the executing BIOS address. Its `F000:C960`
port-61 timing-loop handler also forces the loop counter to zero after a
bounded number of visits. These are source observations, not measurements of
their contribution to the startup gap or evidence that the reference's
ordinary execution engine is solely responsible for the gap. The current
S39 contract does not admit skipping guest work or BIOS-address shortcuts;
retain the actual controller and CPU semantics while investigating.

The production-model FDC trace narrows the first unsupported-command interval:
`12h` was written at tick 284,206,909 and its invalid result was finally
read at tick 350,929,703, a span of 66,722,794 ticks (about 2.02 seconds on
the declared 33 MHz axis). The result was still pending at two intervening
reset-line writes. This does not prove the guest spent the entire interval
polling or give the equivalent host duration, but confirms a guest-visible
controller mismatch spanning substantial guest time. The scratch command
approximations remove the pending result yet do not boot within their 405-second
budget, so a complete device sequence, not that single handshake, remains
necessary before claiming a valid startup improvement.

For scale, the current-production original-CMOS menu run reported
684,593,002 Core ticks before the menu at 204.48 host seconds. PC110 declares
33,000,000 ticks per guest second, so that tick count represents about 20.75
guest seconds, or roughly 0.10 guest seconds per host second over the whole
sample. The marker is sampled once a second and the last tick count precedes
the marker, so this is an approximate aggregate, not an instantaneous speed
or a decomposition of firmware waits. It does show why eliminating only
a small device poll cannot by itself establish acceptable startup speed.

The same current-production original-CMOS run contains 158 quantum endpoints
over 201,003 ms. Grouping intervals shorter than five seconds by the ending
PC's 4 KiB region gives `00038xxx` 40,622 ms (38 endpoints), `000FExxx`
37,393 ms (27), `000FAxxx` 32,585 ms (24), `000F6xxx` 22,263 ms (16),
and `000FCxxx` 16,760 ms (12). These are endpoint classifications, not
continuous instruction sampling: execution inside an interval can visit many
other regions. They reinforce that the complete boot is not represented by
one FDC polling point or the early fixed-instruction benchmark. This reused
the recorded run and did not execute a new guest, alter production or qualify
an optimization.

## Native host samples during current-production boot

Using the same current-production x64 private-media guest executable, the
existing Windows sampler collected worker instruction pointers during seconds
5--60. The task deliberately terminated its own guest at 65 seconds; exit 1
therefore is not a boot test failure or a menu result. Sampling suspends the
thread, perturbs execution, and maps each instruction pointer to its nearest
symbol. In seconds 5--30 the 4,309 worker samples included CPU refresh 294
(6.8%), memory route resolution 237 (5.5%), Machine run 157 (3.6%) and port
read 151 (3.5%). In seconds 30--60 the 5,155 worker samples included port
read 376 (7.3%), CPU refresh 315 (6.1%), Machine run 214 (4.2%) and memory
route resolution 170 (3.3%). These are sampled instruction-pointer shares,
not inclusive call costs, and are not precise product speedup bounds.

Port reads perform an exclusive lookup followed by an independently ordered
wired-OR traversal. The earlier same-work upper-bound probe removed the
entire latter traversal and showed only about 2.2% in one ordered pair; it
was invalid for real route behavior. The new sample therefore identifies a
real boot-phase hot path but does not justify removing the second traversal
or adding an unproven route cache. It does not establish which part of the
first lookup can be simplified safely. No production code or release assets
changed. Sampler log SHA-256:
`DD73229C94BDB1E1E2745DAC5FFC3C69A71C6D97366D11EC544E7AACD4F69598`.

## 486 access-penalty algebra candidate

A task-only candidate eliminated the intermediate saved-transfer arithmetic
in the existing 486 access-penalty calculation. The existing validation
proves both subtractions nonnegative; algebraically distributing the terms
and checking each component for the modeled flag gives the same result.
An independent seeded model checked 500,000 valid uint32 input combinations;
this is not a test of the compiled CPU, failure paths or complete state.
The scratch source diff is +7/-7 lines, with no reduction in asymptotic work.

At the private-media 10-million-instruction checkpoint, baseline/candidate
took 22,073/19,871 ms on x64 and 25,105/27,397 ms on x86. Reversing the x86
order gave candidate/baseline 27,232/26,714 ms. All reached 79,964,294 ticks
and PC `0003dbc5`. These sequential runs have host variance and do not prove
a precise regression or complete equivalence, but both x86 pairs favor the
baseline. Given the lack of a net source reduction or reliable dual-width
benefit, the candidate was not applied; no x64 reverse, full receiver, media
or release change followed. Forward/reverse log SHA-256:
`EB73F9841C80C55A07999343A23A6E473F3FFA21B0F9738273EF84038FD4F701`,
`5C0E8B9B049152E5F8E31B2A9417E6C4B0320A9E011DD210CB622222062A6D93`.

## Firmware HLT-wait upper-bound exploration

A disposable, signature-checked firmware-copy variant replaced the observed
BIOS delay's refresh polling with HLT and a branch to the existing completion
flag check. The existing guest clock, RTC, interrupts and Machine path remained
in use. This intentionally changes the guest instruction stream and observable
intermediate state; it violates S39's no-skipped-work boundary and is not a
proposed production patch. No external firmware file or owner media was edited.

The private-media x64 product-path probe reached the Personaware menu at sample
213 and exited in 216.93 host seconds. Its last pre-menu quantum recorded
686,232,500 guest ticks and 111,295,264 retired instructions; the earlier
unmodified-production run reached sample 201 in 204.48 seconds, at a preceding
684,593,002 ticks and 135,225,679 instructions. These are separate ordered runs
with coarse menu sampling, host drift, altered guest code and no full-state
equivalence. The candidate executed fewer observed instructions but did not
demonstrate faster startup. It was not tested on x86, on native Window/RDP,
or after menu interaction and was not adopted. Log SHA-256:
`97D95DA7510677A62081A9535FF0EB93F0A429703F27FF92BE513AD944BD18E5`.

The recorded quantum endpoints between approximately 230 million and 350
million guest ticks took 30.48 seconds and 20.33 million instructions in the
baseline, versus 21.35 seconds and 10.53 million in the candidate. The sampled
tick endpoints differ, and this is only one portion of two sequential runs;
it does not prove an end-to-end speedup or that HLT waiting has the same
debug, interrupt, register or device-observation behavior. It does identify
the firmware wait phase as benefiting in this exploratory comparison even
though the complete menu comparison did not show an improvement.

A follow-up on the same disposable variant sampled the existing Machine
deadline-advance call once per 256 invocations through the 50-million-instruction
checkpoint. It counted 263,424 successful deadline advances, 1,029 sampled
calls totaling 9,423 host performance-counter units at 10 MHz and 240,687
sampled guest ticks; the L1 bounded-advance path was not called. The run
finished in 93,674 ms at 385,145,067 guest ticks and PC `0003e3af`. Scaling
these deterministic samples suggests about 0.24 seconds inside the sampled
deadline-advance function, excluding runner work and instrumentation. Sampling
may alias with repeated deadlines, and the different guest instructions make
this neither a comparison to the production firmware nor a full attribution
of the unchanged menu time. It does not support blaming that function alone
for the probe's roughly 217-second startup. No production change resulted.
Guest log SHA-256:
`7D76CF6A3261853E6641B6757576A26EAEB802D09BECE4CE88A01239B77D19CF`.

In another same-variant run at that checkpoint, instrumentation in a disposable
runner copy sampled the two actual executor callback sites and display-publish
call once per 256 invocations. It counted 660,936 callbacks / 2,581 samples /
65,439 counter units and 330,467 display calls / 1,290 samples / 24,645 units,
at 10 MHz. Separately wrapped deadline advance reported 263,424 calls / 1,029
samples / 12,732 units. Scaling suggests about 1.68, 0.63 and 0.33 seconds
respectively in those sampled functions within a 98.71-second instrumented
run. These values exclude surrounding runner work; deterministic sampling,
clock-call overhead and source instrumentation limit accuracy. The stages are
not evidence that all waiting-related work is measured, and they do not qualify
the guest-code change. They do not support attributing tens of seconds solely
to these callbacks, display publication or deadline advancement. The run
reached 50,000,000 instructions, 385,145,067 ticks and PC `0003e3af`; no
production source or owner media changed. Guest log SHA-256:
`DB6770D2A70C6AB46B677D8AC05CE2433B2E1F6162E82F720F49BE62126DD245`.

The existing full-menu baseline and HLT logs were additionally aligned by
interpolating their quantum endpoints to common guest-tick boundaries. For
20–230 million ticks, baseline/candidate took about 53.3/68.0 host seconds
and retired about 33.06/33.03 million instructions. For 230–500 million ticks,
they took 71.6/53.0 seconds and retired 47.77/26.14 million instructions.
For 600–680 million ticks, they took 30.6/38.4 seconds and retired about
21.35/21.33 million instructions. Both the early and late similar-work intervals
were slower in the candidate run, so comparing whole-run wall times alone
conceals substantial between-run variation. Interpolation, changed firmware,
different device state and two sequential executions prevent a causal
speedup claim or general normalization. These records warrant preserving the
phase-level observation but do not qualify the HLT patch or change S39's
no-skipped-work boundary. No guest was rerun for this calculation.

## CPU host-tuning exploration

A task-only x64 comparison compiled the same CPU instruction translation
unit at `-O3`, once with the default GCC tuning and once with `-mtune=znver4`;
it retained the same cached libraries and private-media startup fixture. The
option tunes code layout without requesting new ISA instructions. Baseline and
candidate took 21,152 and 20,427 ms respectively at 10,000,000 instructions,
79,964,294 ticks and PC `0003dbc5`. The object sizes were 361,068 and 362,430
bytes. This single ordered pair does not establish a repeatable improvement,
full guest equivalence, other host performance or usability. It cannot justify
a machine-specific release setting; no x86, reverse-order or complete-boot
test was run. Production source, compiler flags and artifacts remain unchanged.
Log SHA-256:
`4F1DA45CC12E54EA7C3E5E79DA636477CF4627ADC2C89AEA2E2DB1EBDC9A219F`.

## Memory-route address-index upper bound

A task-only x64 candidate indexed registered provider address windows by
64 KiB region and enumerated only matching candidate bits in registration
order. Actual provider query, tier precedence, aliases and callbacks remained
in the existing route. The prototype used a 512 KiB process-global index;
it is deliberately unsafe for multiple Machines, removal and route changes,
so it is not a production design or an equivalence proof.

For the private-media early-startup checkpoint, baseline and candidate took
19,586 and 20,011 ms respectively, both reaching 10,000,000 instructions,
79,964,294 ticks and PC `0003dbc5`. This single ordered pair is subject to
host variation and does not prove a regression or full-state equivalence.
It provides no evidence of a gain sufficient to justify the index, additional
memory and ownership/invalidation machinery. No x86 or complete-boot run was
made, and no production source or artifact changed. Log SHA-256:
`1B63C0DEDD43F2ACEF8A92862CF26BF5F5732F01CEC5A5B0D4A2EC41E60F41EF`.

## Last-exclusive-port lookup upper bound

A task-only upper-bound candidate used one process-global pointer to reuse
the last successful exclusive-port lookup. This is deliberately unsafe for
multiple Machines, route removal, registration rollback and callbacks that
mutate routes; it is not a production design. It retained the independently
ordered wired-OR pass. Each run used private copies of the same input media.

At the 30-million-instruction startup checkpoint, ordered baseline/candidate/
candidate/baseline x64 times were 56,738 / 51,275 / 54,661 / 53,299 ms,
all reporting 195,624,840 guest ticks and PC `000f6440`. The first pair
favors the candidate by about 9.6%, while the reverse pair favors baseline
by about 2.6%. The candidate is not guest-state or route-mutation equivalent;
the differing direction and host variation do not establish a reproducible
benefit. This does not justify adding persistent cache state and invalidation
to the public production route owner. No x86 or complete boot was measured,
and no production code, user media or product binary was changed. Log SHA-256
in the same order:
`37907F19691C2E080234471A6EAA0F815F46737F6EA844164F3D56BE97230A86`,
`55721489BA35E26F4F3E2FC3CF469EE5185668E7219E6051779365F9B090D75F`.

### Route callback mutation boundary

A short task-owned x64 fixture registered an exclusive read and a wired-OR
read on the same address, then performed a paused Machine bus read. The
exclusive callback used the existing Machine removal operation to remove the
wired-OR owner's route. Removal returned success and the read returned only
the exclusive value `4`; a pre-callback snapshot that still invoked the
removed contributor would return `6` or access freed state. The fixture uses
the existing private stack-Machine setup with its lifecycle set to paused;
it is not a full PC110 or concurrent-control test. Separately, installing
new routes requires the initialized, unfrozen configuration state, so the
private port helper's ability to register in a callback must not be generalized
to a paused public Machine. Any one-pass optimization must preserve the
existing paused-read removal behavior as well as registration rollback. No
production source changed. Probe output SHA-256:
`463B0742C6551F6BA9D1AC08EFF1819270DA8E8633FDF52AD7775FCBEEA1D7BD`.

### Stable read/write partition at freeze

A task-only candidate stably partitioned the existing port-provider list into
reads followed by writes immediately after successful Machine configuration
freeze. It added no persistent index or route cache; relative order among all
read entries, including wired-OR contributors, was retained. A separate
50,000-instruction witness printed that the partition was actually applied.
This is not yet an admissible implementation: registration rollback, callback
mutation, independent owner removal, all consumers, and write-heavy workloads
were not fully qualified, and moving writes later can worsen their lookup.

At the private ten-million-instruction startup checkpoint, ordered
baseline/candidate/candidate/baseline x64 times were 18,352 / 17,864 / 17,551
/ 18,445 ms; x86 times were 22,704 / 23,290 / 28,010 / 27,519 ms. All
reported 79,964,294 ticks and PC `0003dbc5`. Both x64 comparisons favored the
candidate while both x86 comparisons favored baseline. Host drift is visible,
and one endpoint is not complete guest-state or end-to-end proof. Given the
possible write regression and no consistent dual-width benefit, the candidate
is not adopted; no production code or artifact changed. Comparison log hashes:
`7CC78F4FF22F78E16B7EB7EE8A2E81E8EB13ABA30C4E0F721A79EB54BB619A6D`,
`43CB314D8B5605038D3C0F4A7D09FAF648C1128F169C31D9B917381F87198A66`,
`79D787EF74FFF7ED91D347BD612B613C0BCB408AC76D9A73B1D2E85433B6C2CA`,
`A4F23954040878BB917D519EE6FBBCF0E4C5A45FA391209292F418D6A2FB6D7E`.

### Separate read/write registry experiment

A task-only upper-bound variant partitioned the list at freeze and used a
process-global boundary pointer to avoid visiting the other direction. The
pointer was deliberately unsafe for multiple Machines and route removal;
it was never eligible for production. At 30 million startup instructions,
ordered baseline/candidate/candidate/baseline times were 60,830 / 56,281 /
56,106 / 57,531 ms x64 and 67,890 / 62,954 / 62,441 / 66,445 ms x86.
Each reached 195,624,840 ticks and `000f6440`. These are fixed-work samples,
not complete-state or menu measurements. Upper-bound log hashes:
`C87815CD7AC9CAC39616D547FB79BC64C372834AED7FED65A8CD61055823217E`,
`8E65759753E2E2B6BF709F17F5C92CD649D0F9932D93964356CD4AA09064F874`,
`03B4086404905D11084D5B2E8B8DCAB483543CBF1C04ADEAA272FA977A546614`,
`0445F8A2A71B41F207A0B619D22E2BA8E0167B6FDA64E7BC40F5E55ACC8BD83F`.

The actual trial replaced the one owner-local list with distinct read and
write lists, kept each route in exactly one list, and extended the private
registration checkpoint to both heads. Existing mixed registration rollback
coverage and an added callback-removes-wired-route test passed on x64 and x86.
At the same fixed checkpoint the actual baseline/candidate/candidate/baseline
times were 47,508 / 48,486 / 65,012 / 68,859 ms x64 and 64,210 / 74,483 /
78,310 / 69,518 ms x86. Baseline binaries were separately linked from the
prior source while candidates linked the rebuilt archive, so layout and host
drift limit attribution; the endpoint does not prove full-state equivalence.
Both x86 pairs nonetheless show a material potential regression, unlike the
upper bound. The trial's production, test and manifest changes were withdrawn;
no release artifact was produced from them. Full receiver, end-to-end menu and
write-heavy workload qualification were not run. Comparison-log hashes:
`7BBB48CC81660AB88AA6FA64680A5CDD8F7454F60EA0A30124CABF32020D54BD`,
`1109F2B0A712C564B85FAC402B597167DBB447F36F74E6CAEE33FA8DEFA399D6`.

After withdrawal, the existing port-registration and RAM/port-context targets
were rebuilt from restored source in each cached Release tree. The two focused
tests passed on x64 and x86. This is a restoration check, not a full repository
build, integration run or PC110 usability qualification.

## Zero-or-one clock-conversion candidate

A task-only clock candidate handles a source advance of at most `UINT32_MAX`
with an exact 64-bit multiply and phase addition when the result yields zero
or one device tick; larger results retain the original division path. This
extends the earlier no-carry-only candidate, without a new clock, state or
device path. An actual C comparison of baseline and candidate covered 600,000
seeded boundary/random inputs on each architecture, including representative
PIT ratios, and compared result, phase and accumulated ticks. Both passed;
this does not replace full clock, device or product tests.

At ten million startup instructions, baseline/candidate/candidate/baseline
times were 18,896 / 17,788 / 17,767 / 17,625 ms x64 and 21,964 / 21,398 /
21,326 / 22,904 ms x86. At thirty million, x64 times in the same order were
57,808 / 61,816 / 58,341 / 56,308 ms; an x86 baseline/candidate pair was
74,556 / 71,207 ms. Endpoints were respectively 79,964,294 ticks at
`0003dbc5` and 195,624,840 ticks at `000f6440`. Separate runs and host drift
limit interpretation, and endpoints do not establish complete state. Both
later x64 comparisons favored baseline by several percent, so the added
branches are not adopted despite the x86 samples. No x86 reverse at thirty
million, complete boot, production edit or release followed. Log hashes:
`3D8717E88EA73C1ED4D37384B22AD9F838ED0AEC456EFE72F3DE605BC9D235FE`,
`34511E493ACB9A2FF2BA16713571D1CFCC39E1B4EF159207CB65728362B3F8C0`,
`D4B6BA67C02ED531BD516D7A79AC3BA0D540F1557CA9F6B94890850B5DC3DE91`,
`822B0FD7CC68FD9CE81BB0F86204132B7C7E7AADF3EE07A894BEDB086E7FCA58`,
`CC7FCBA74B5CA6F4ADDC8837E1F0260FA9887909F58D820E2C69F9331600A559`,
`16D8ED1E42A81D374AF0752616A39F862C0286775E5382C458284B1179568B36`.

## Instruction-byte reset boundary

`ExecBoundary` clears the 15-byte instruction buffer at each boundary. A
potential shortcut would leave stale bytes beyond the new instruction length,
but `x86_core_cpu_debug_capture_instruction` currently copies the entire
buffer into the externally visible debug observation, not only the used
prefix. The timing and decoder sources also contain many indexed buffer
accesses. Therefore deleting the reset solely on the assumption that every
consumer ignores unused bytes would change the existing observation and is not
justified by the current contract. No speedup has been measured for this
candidate, and no production code or test was changed.

## Reusing conditional-branch timing facts

A task-only x64 candidate used the existing timing-result flags to carry the
outcome already calculated for 486 short and near conditional branches into
the generic timing-input collector. It avoided a second condition evaluation
without a new persistent field or alternate execution path. It also changes
what the internal source-cost helper publishes when called directly, and its
fault, standalone-helper and other-profile behavior was not qualified; it is
not a production implementation.

At the private ten-million-instruction startup checkpoint, ordered
baseline/candidate/candidate/baseline times were 17,140 / 17,417 / 18,073 /
17,376 ms; each reported 79,964,294 ticks and PC `0003dbc5`. Both pairs favor
baseline, while the same endpoint does not prove equal complete state. This
does not justify extra conditionals or expanding the candidate to x86 and the
full receiver. No production code, public interface or release asset changed.
Comparison-log SHA-256:
`ED8155257D8DABE780BB0B73383B37C1F696BDA8C64638466EEAB99B0E742244`.

## Firmware-wait endpoint classification

The private extended FDC approximation ended its 100-million-instruction
sample at physical `000fa5ab`, and the longer unsuccessful menu run repeatedly
sampled `000fa5a9` as well as the previously identified RTC interrupt entry
`000fe98a`. An offline inspection of the already validated DSTN firmware's
bank-3 bytes places `a5a9` and `a5ab` inside a port-61 refresh-transition wait
that also has a counter and an interrupt-updated termination condition. The
reported addresses are instruction endpoints, not evidence that one call
never returns or that RTC interrupts are malfunctioning. This explains why
port-61 lookups appear frequently even after the scratch FDC status-polling
storm disappears; it does not establish that the wait duration is wrong.
Firmware bytes remain external; no firmware, machine state or user media was
modified. A correct next investigation must distinguish repeated legitimate
wait calls from an incorrectly long single wait before changing the timer or
firmware behavior.

### Wait-counter follow-up

An x64 private-media run of the same incomplete enhanced-FDC approximation
sampled CPU registers only at Machine quantum boundaries within that firmware
wait. It completed its 100-million-instruction diagnostic in 171,297 ms at
614,943,578 ticks and `000fa5ab`; 77 wait samples were recorded. In one
observed sequence the counter decreased from `0000f3fd` to `0000e093`, and
later sequences repeatedly decreased from approximately `0000f3xx` to
`00007fxx` before increasing again after intervening execution. Ten such
sample-to-sample increases were observed. Different stack pointers also occur
between samples. These are evidence of repeated wait intervals rather than
one counter remaining fixed; sparse quantum sampling cannot prove every
entry/exit or determine which caller requested each delay. The approximation
is not a correct SMC implementation, and this diagnostic does not measure the
unmodified product or qualify an optimization. No production source, owner
media or release binary changed. Guest log SHA-256:
`2FCF51C5683FF7D479F0879CBA605408957A6D6B3220ED4CC2551D0EFF90D426`.

### Caller of the repeated waits

A second x64 private-media run of the same incomplete FDC approximation
captured CPU state and a small stack window at selected wait quantum
boundaries. It completed 60 million instructions in 94,743 ms at
375,626,854 ticks and PC `0003a7c5`. The sampled stack consistently contained
a return into `F000:C7CA`; independent offline analysis of the validated
DSTN ROM shows this return follows a BIOS wait service invocation. The same
samples contained `C9CF`; the immediately preceding firmware routine calls
the wait helper and subsequently reads port `3F7h` and tests its disk-change
bit. These facts connect the sampled wait to the firmware floppy path; the
sampled words alone do not prove a complete call chain for every wait or that
the chip's disk-change bit is wrong. The incomplete candidate still cannot be
used as a correct controller or product benchmark. No firmware bytes are
copied into this record, and no production code or user media changed. Guest
log SHA-256:
`DAE4BB157F2B12235AAD94823E0996A046951FAFC8AD6E7A85652CA2205F3D79`.

The recorded stack also preserves the wait service's original high word
`0007h`, while the wait samples show low word `a120h`: together they specify
500,000 microseconds. In one sampled wait, ECX fell from 62,461 at tick
302,181,640 to 32,576 at tick 317,064,247. The difference of 14,882,607
ticks is about 451 ms on the 33 MHz Core axis; 29,885 refresh transitions at
the configured 498 ticks each predict 14,882,730 ticks. The 123-tick
difference is consistent with sampling at different instructions in the wait
loop, not proof of exact real-hardware refresh timing. This sampled interval
is consistent with the guest's requested half-second service rather than a
counter stuck or a 20-second guest wait. The host still spends much longer
executing that guest work; the incomplete SMC model prevents attributing the
number of calls to an authentic PC110 boot.

The same boundary-and-stack probe was then linked against the unmodified
production FDC, using private media and the same CMOS seed. At 60 million
instructions it reported 365,365,343 ticks, PC `00038887`, and 1,376,335 FDC
status reads; elapsed host time was 112,454 ms with tracing enabled. In its
first sampled wait, ECX decreased from 62,457 at tick 233,307,414 to 42,655
at tick 243,168,741: 19,802 transitions at 498 ticks predict 9,861,396 ticks,
versus the observed 9,861,327. The saved high word and return addresses match
the earlier sampled BIOS wait path. This establishes that the observed
refresh wait progresses on the configured guest clock in the actual current
FDC path too; it does not validate physical timing, identify every wait's
caller, or establish its exclusive host cost. Probe log SHA-256:
`B2504BBDDC5F68DCA0294A16311B51FE15FE90D639DC3358190BA6BEA0B9796F`.

#### RTC completion versus refresh-loop fallback

The firmware wait has two exit conditions: an RTC interrupt marks the
requested delay complete, or the refresh-transition countdown reaches zero.
The earlier comparison of countdown deltas with refresh ticks confirms loop
progress but does not itself prove which condition ends a wait.

A denser private-media x64 probe retained the production FDC and capped the
existing Machine run budget at 65,536 instructions for observation. Two
sampled intervals covered ticks 233,307,414–249,742,978 with ECX 62,457–29,454,
and ticks 267,757,622–284,193,164 with ECX 62,433–29,432. Each spans about
498 ms of guest time. The next traced FDC operations occurred only 15,888 and
13,745 ticks after those last samples, respectively, far less than the time
needed for approximately 29,000 remaining refresh transitions. Together with
the decoded dual-exit control flow, this supports RTC completion for these
waits rather than countdown exhaustion. It is sampled evidence, not a direct
capture of the completion flag or proof for every wait. The probe finished 50
million instructions at 306,212,464 ticks; it is not a complete boot or a
performance comparison. Production source and owner media were unchanged.
Guest log SHA-256:
`8A7CA21EF8E3B98721CC179D3EB4AE0E220D6098603A07883B1CFE839A2F4B4C`.

### Forced disk-change diagnostic

A task-only x64 candidate additionally forced bit 7 of every FDC direction
register read high; this is not an admissible hardware model. At 100 million
instructions it reached 600,682,902 ticks and PC `000fadb5` in 150,728 ms;
the earlier incomplete CONFIGURE/UNLOCK candidate reached 614,943,578 ticks
and `000fa5ab` in 162,859 ms. Traces recorded zero versus two READ DATA
commands, ten versus eight recalibrates, and one versus seven CONFIGURE
commands. Thus the forced bit changes the BIOS path, but at this fixed-work
endpoint it has not established a normal boot or comparable guest state.
The SMC manual states that DIR bit 7 is the inverse of the external disk-change
pin; it does not establish PC110's undriven-pin wiring. The PC110-EMU FDC
placeholder returning `ffh` for this port likewise is not proof of that
wiring. Do not force this bit in production based on this probe. No user
media or production source changed. Guest log SHA-256:
`0ACB0B3B377324DF0D6F0BA68096BFF7BF05B2E94485A3548BEC3D74B34C7471`.

The same unconditional-high, incomplete-SMC candidate was subsequently run
with the private-media full-menu harness: it reached `MENU-READY` at sample
242 and exited successfully in 245.95 seconds, versus 204.48 seconds in the
earlier separately run unmodified-production original-CMOS sample. These are
not an ordered controlled pair; the firmware paths and final guest states
are different, and the detector has one-second granularity. The observation
does establish that a disk-change response can change the outcome of the
partial-SMC experiment, whose earlier unchanged-DIR run had no menu by 405
seconds. It does not establish a production speedup or a correct electrical
model. Log SHA-256:
`6346A3C9CA5851B545BEF523CF5C69E6B932EDD5D5506BBCAD791996DCCE2E79`.

The local 86Box reference at commit `4fef696a4eead1d55a28d6ac0e5bd2864e5454da`
uses an asserted disk-change bit for an empty drive when its motor is enabled
in its ordinary PC/AT FDC model. This is an L2 implementation reference,
not PC110 electrical evidence. A motor-conditional, uninstalled-drive scratch
candidate is being tested separately with the same incomplete SMC model; the
reference does not justify asserting the bit unconditionally or changing
other machine profiles without separate verification.

The motor-conditional uninstalled-drive scratch candidate reached the menu at
sample 252 and exited successfully in 256.56 seconds, still with the incomplete
SMC approximation. The preceding unconditional candidate had reached the
same logged 140,958,003-instruction / 746,391,858-tick quantum, but complete
guest-state equivalence was not checked. The two separate runs were not a
controlled performance comparison, and neither is a qualified hardware model
or demonstrated speedup over current production. A further scratch run now
tests the motor-conditional board behavior with the unmodified production FDC
chip, so the effects of the two changes are not conflated. No source or release
artifact has been changed. Motor-conditional guest log SHA-256:
`A73F120D65A29FC7A5ECF2525BEF5150A24C23D3FD6B6F7A622EEB6013B7F63D`.

The separate motor-conditional board candidate with the unchanged production
FDC chip reached `MENU-READY` at sample 224 and exited in 228.56 seconds. Its
last preceding quantum was 143,232,673 instructions and 756,835,906 ticks;
the earlier production original-CMOS run reached sample 201 in 204.48 seconds
with 135,225,679 instructions and 684,593,002 ticks at its preceding quantum.
They are sequential separate runs with coarse menu sampling and are not a
complete-state or controlled latency comparison. Nonetheless, the candidate
performs more recorded guest work and does not demonstrate a startup
improvement. The 86Box reference is not PC110-specific wiring evidence; no
production status behavior, ROM, user media or release binary was changed.
Guest log SHA-256:
`3A4AB856DA1A5082BD8566C772982D993B0B6BACB9930FBB5B830EE6A6853D69`.

### Requested-memory experiment is not a memory comparison

A private product-path probe requested 4 MiB instead of 20 MiB, with the
unmodified production device code and the same media/CMOS seed. It reached
the menu at sample 202 in 206.34 seconds; its preceding quantum had exactly
135,225,679 instructions and 684,593,002 ticks, the same as the preceding
20 MiB run's recorded quantum. This prompted a source audit: the common
factory copies the requested RAM size into the machine config, but the PC110
construction initializes `core_config` from its fixed 20 MiB plan and the
construction-finish routine does not apply the requested size. The Machine
constructs its board from that plan. Therefore this was not a valid 4 MiB
versus 20 MiB comparison and cannot establish a relationship between RAM
size and startup speed. `memory_reconfigurable = false` separately disables
runtime resizing; that flag alone would not explain startup configuration.

The product INI currently exposes `memory_kib` and the PC110 programme
proposal describes runtime RAM settings, so the silent disregard merits a
separate size-contract and supported-hardware review. It is not a reason to
change the user's INI, assert arbitrary sizes are valid, or call reduced RAM
a performance repair. No production, user configuration or media changed.
Guest log SHA-256:
`E19AAEB7B68A5BCAD31E841019C42D0395BDF34D847CA33BEDCAAB78A769EF4D`.

A short follow-up queried the constructed product Machine directly before
starting it: the normal PC110 plan returned 20,971,520 bytes after the caller
requested 4,194,304 bytes, with successful information status. The diagnostic
then intentionally exited nonzero on the mismatch; this is not a startup test
failure. This dynamically confirms the source audit's silent fixed-size
behavior without inferring it from matching execution checkpoints. Output
SHA-256: `E08C9693D7B556B5243A7F1831ED0E9002E12E74388A272700148248227D8220`.

For a genuine size comparison, a task-only PC110 plan set the Core backing
to 4 MiB and moved its 64 KiB open-bus probe window to the 4 MiB boundary;
the private request also asked for 4 MiB. The published Machine information
reported exactly 4,194,304 bytes. With otherwise copied media and the same
CMOS seed, the guest reached `MENU-READY` at sample 207 in 211.33 seconds;
the preceding quantum had 130,660,836 instructions and 645,813,096 ticks.
The earlier normal 20 MiB run reached sample 201 in 204.48 seconds, with
135,225,679 instructions and 684,593,002 ticks. This is one separate ordered
comparison with coarse sampling, different guest memory and no complete-state
equivalence; the copied CMOS and remaining chipset setup were not independently
qualified for a physical 4 MiB configuration. It does not demonstrate faster
startup from reduced RAM, nor qualify a product memory-size change. Both
samples still require about 200 host seconds, so shrinking RAM alone is not
a satisfactory repair. No product source, user INI or owner media changed.
Guest log SHA-256:
`BA919A3D28E4FEDCAEF056A8BCEB02FD0CA4AB70749D08E221DE6C033D16B5F3`.

### Refresh request observations at the arbitration boundary

Core arbitration obtains `refresh_request` and then calls the refresh-hold
advance routine, which can query the same callback again. PC110's current
profile callback always returns false. However, the existing attachment
contract does not promise that refresh callbacks are pure or that successive
observations return the same value: callbacks execute on the Core owner and
may use bounded Core operations. Other boards can also provide different
callbacks. Therefore reusing the first answer would change an existing
cross-component contract unless all owners and intermediate mutations are
accounted for. In particular, the later prefetch-reservation check follows
DMA and refresh advancement, and cannot reuse the earlier answer merely because
PC110's callback currently returns false. No elapsed-time measurement or
complete-callback audit has yet established the value and safety of this
candidate. No production code was changed by this inspection.

A subsequent task-only x64 upper-bound probe replaced the first arbitration
query with false while retaining the refresh-hold query. This is only valid
for the observed PC110 no-op callback, not for the general attachment contract.
Using the existing private-media fixed-startup harness, baseline and candidate
ran 10,000,000 instructions in 20,695 and 20,404 ms respectively, each ending
at 79,964,294 ticks and PC `0003dbc5`. One sequential pair with a 1.4% gap is
within the observed host variation and is not a full-state equivalence test.
It provides no evidence that this small call elimination can solve the roughly
204-second startup or justify weakening the callback contract. No x86 or
end-to-end startup comparison was run and no production source was changed.

### FDC enhanced-command settings

For the firmware-observed `13 00 30 00` sequence, the manual's CONFIGURE
table specifically selects no implied seek, disabled FIFO, disabled polling,
a one-byte threshold and precompensation starting at track zero. Thus the
existing byte-at-a-time transfer behavior is compatible with the selected
FIFO setting, but the polling-disable effect and software-reset/LOCK behavior
still require their own implementation and tests. The observed `12 00` selects
conventional mode. The production trace also shows `14h` returning the 8272
invalid-command byte; on the enhanced controller it is an unlock operation
with a lock-status result. These conclusions concern the observed settings,
not general FIFO, perpendicular recording or complete SMC compatibility.
The earlier partial implementation already explored the observed polling and
unlock settings, and its long boot prevents presenting this narrower model as
an established performance repair.
Reference: local `5-FDC37C665IR.PDF`, SHA-256
`2105112CD5EE41E377BF667D408796573F244180A4F3E47B9B7C714A07BE3258`,
printed pages 55 and 68–72.

Reanalysis of that same production-FDC trace found long gaps before the
unsupported `12h` transaction as well: between consecutive non-status FDC
events, the largest was 61,801,989 ticks (about 1.873 guest seconds) and a
later gap was 52,146,882 ticks (about 1.580 guest seconds). The event log does
not establish what the CPU did throughout either interval. These observations
rule out attributing the whole long boot solely to the `12h` pending result;
they do not estimate what a correct controller would save. This analysis reused
an existing trace and did not run another guest or change production.

The recorded production menu run was also grouped by host-time intervals,
using the first and last existing quantum endpoints inside each interval.
From 0–30, 30–60, 60–120, 120–180 and 180–210 host seconds, the respective
measured guest/host time ratios were 0.132, 0.109, 0.113, 0.088 and 0.065;
the corresponding retired rates were approximately 591k, 660k, 659k, 707k
and 720k instructions/host second. The final interval ends at the last quantum
before the menu rather than 210 seconds. These are coarse asynchronous endpoint
measurements, not CPU profiles, and do not prove which instructions or devices
dominate. They show that slow guest-time progress persists through multiple
boot phases while host instruction throughput stays in the same broad range;
the changing guest/host ratio alone is not evidence of a single stuck wait.

## Scheduler-only time-observation fields

The scheduler calls the same private deadline observation as the external
time-observation API, but does not read its pacing and physical-rate fields.
A task-only candidate omitted those fields when the scheduler supplies its
board result, leaving deadline computation and callback order unchanged. The
external caller still received all fields. Its use of the board-result pointer
as a discriminator was an experiment, not an admitted production contract.

In one x64 private-media pair, baseline and candidate took 20,409 and
20,702 ms respectively to retire 10,000,000 instructions, both reaching
79,964,294 guest ticks and PC `0003dbc5`. This single ordered comparison is
subject to host noise and does not establish a regression or complete state
equivalence; it also does not show the required benefit. The candidate adds a
branch and risks obscuring the output contract, so it is not retained. The
private observation function is inlined in the Release object, but that alone
does not prove which stores the compiler removed. No production source,
public API, media or released executable changed; no x86 or whole-boot run
was warranted on this evidence. Guest-log SHA-256, baseline then candidate:
`43E9ED740814A004BA42E96C43AF1E193353962F82AA8FDA283D282DFC436AA1`,
`651E94E7A2EC466072713063999CE73FB81A77ECA9FD3900F753C3E7F9F677CD`.

## Port bucket lookup upper bound

A task-only candidate grouped the existing port entries by the low eight
address bits at machine freeze, then searched only the selected group for
both exclusive and wired-OR reads. Its process-global index deliberately does
not handle multiple machines or route mutations and is unsafe for production.
Within a group it retained the original list order. At the private 30-million
instruction checkpoint, baseline/candidate/candidate/baseline x64 times were
56,557 / 54,463 / 57,879 / 61,283 ms; one x86 baseline/candidate pair was
81,150 / 74,298 ms. All reported 195,624,840 ticks and PC `000f6440`. Host
drift and ordering can affect these samples, and the same checkpoint does not
prove complete state, callback, or full-boot equivalence. The repeated x64
direction and the x86 observation justify testing a real owner-local design,
not claiming this percentage as product startup improvement.

An admissible candidate makes per-Machine buckets the sole route storage;
preserves registration checkpoints, exact same-port ordering, wired-OR
contribution and callback-time removals; and adds no global cache or duplicate
executor. It costs a bounded bucket array per Machine and must be compared
against current source on both widths, including write-heavy workloads and
actual DOS/Personaware boot. Full Shared receiving and fourteen artifacts are
required if retained. Scratch upper-bound log SHA-256 in x64 forward, x64
reverse, x86 forward order:
`03E406F7973A3B2293502B643BB526D54163C4B8FB5C2F73F9790156D77EAA5D`,
`5FC3B1F4C9D915B2762E193658F110FBB33F4B68A062EEDED32F80583876E386`,
`2C3DB2E7097E75341F9D1F94DB4F9B15AFBAA46299723D2D7E525782485F1AF6`,
`5E4FFF6146ECA0AE12B991F79FD6101B477C7EC4D07F8702AB61588781417622`,
`81DE3E35046CAB4679EE60DEC60FFD1B7E905C29753E688FA1FF7B3849F4A9AE`,
`175C83747F412F6C61D305DE5B94406C4ED15E8A0C83EACCFACEF368BC480424`.

### Owner-local implementation and baseline correction

The actual candidate makes 256 owner-local buckets the sole route storage.
Same-port entries retain insertion order. A private checkpoint copies bucket
heads for registration rollback; owner removal and destruction visit all
buckets. No public route API or process-global index is added. The existing
registration test was initially extended to exercise 120 colliding addresses, callback
removal of a wired-OR contributor and independent port owners. The registration,
RAM/port-context and neutral-link tests passed on x64 and x86.

An initial actual-candidate comparison used the previously linked upper-bound
baseline and produced different 30-million endpoints: candidate 195,603,273
ticks / `000f6442`, older baseline 195,624,840 / `000f6440`. Thus those times
are not an equivalent-work comparison. To investigate, a separate x64 pair
used the same new Machine layout, with one build routing all ports through a
single bucket and the other using the actual bucket selection. All 32,582
recorded Machine run boundaries matched through 30 million instructions.
An independently rebuilt old-layout baseline, compiling the 16 current Machine
translation units against the original port header and sources, also matched
all 32,582 candidate boundaries and ended at 195,603,273 ticks / `000f6442`.
This isolates the earlier endpoint difference to the older linked baseline or
its build inputs rather than demonstrating a bucket routing difference. It
does not prove every callback, fault, full guest state or later boot equivalent.

A new-layout single-bucket/bucketed x64 pair without boundary logging took
59,004 / 57,727 ms, respectively, at the matched new endpoint. This one
candidate-first pair does not establish a stable gain or measure the original
smaller Machine layout. Compare the rebuilt old-layout baseline with the
actual candidate on both widths before retaining the production change, and
still measure full boot if it qualifies. No release artifact has been built
from this candidate. Trace log SHA-256, old layout, new-layout single bucket,
new-layout buckets respectively:
`2DB9876D575CB015436A9D75EF3B4933C26C8C163A24D8FFFA830A40C6555F7E`,
`74CC3025A56DF94F5309B2DA447F5B31F2F01CE54FFAA9BF8E2E6834634F121D`,
`2B423D55830D5EDEB53D64AE81FF700ACFE70DAC18A32C656699072EC99B057F`.

The rebuilt original-layout and actual owner-local candidate then ran the
unlogged fixed-work probe against the same private media. At the common
30-million endpoint (195,603,273 ticks / `000f6442`), baseline/candidate
measured 58,715 / 55,180 ms x64 and 71,733 / 69,612 ms x86. These are one
ordered pair per width, not complete-state equivalence or full-boot results.

The x64 candidate's real product-path menu probe did not reach `MENU-READY`
within 405.36 seconds; its final recorded quantum was 268,293,479 instructions
at 1,056,960,406 ticks. The freshly rebuilt original-layout baseline did not
reach the marker within a separately imposed 300-second bound; its last quantum
was 206,555,455 instructions at 862,799,857 ticks. Comparing the asynchronous
logs by equal instruction count found 176 shared logged checkpoints with no
tick or PC difference, through 206,555,455 instructions. These samples do not
prove full-state equivalence, but they do not establish that the bucket
candidate caused this failure. Both runs failed the intended startup
qualification; no full-menu speedup or usable product is claimed. Their
private guest logs hash, baseline then candidate:
`18019F2CEF8CB841E44A2C961EDC2195C9174474240B9D2DD941D953571188D3`,
`7959642B45CF1C7D3FE85AE630C7BFCBBC8112B762A1770B509072A761C68850`.
The preserved earlier known-successful probe binary was then run with copied
private media and the same menu detector. The subsequent adjacent-CMOS audit
below establishes that this was not the same CMOS input as the failed runs. It reached
`MENU-READY sample=209` and exited successfully in 212.89 seconds; its final
recorded quantum was 135,787,759 instructions and 687,009,029 ticks. This
confirms the successful result can still be reproduced with those inputs and
the detector. The recent code/build baseline differs from that binary in
several CPU, timing, board and display changes, so this alone does not name a
particular defect. The later controlled input comparison below identifies a
CMOS mismatch in this comparison; it does not prove a CPU regression. Full
receiver qualification remains required before release.
Historical guest-log SHA-256:
`64142787EAB9413DF719377D6ABACA63DF06C5E7BDA1C921ED327F1247CC1861`;
historical executable SHA-256:
`E3C6E60670ED56820C097286F1184C7B3205196C1D3C39C11E48BF9EB7A193D9`.

The saved pre-double-shift trace and historical binary both reported 3,932,346
ticks at 524,288 instructions, versus 3,932,418 in the current rebuild. A
scratch old-`SHLD/SHRD` CPU instruction source alone still reported 3,932,418;
the early difference therefore cannot be attributed to that one change. A
scratch build of all five CPU translation units from repository commit
`c39610901` with the current board also reported 3,932,418. Separately, the
historical clock, memory, board-deadline, PIC and PC110-display source files,
individually or together with the current CPU, still reported 3,932,418.
Combining all five historical CPU translation units and those five historical
non-CPU sources reported 3,932,346 at 524,288 instructions and 79,964,294 at
10 million, matching the old checkpoints. These short probes did not control
the executable-adjacent CMOS and therefore do not isolate a source/build
combination. The controlled same-binary CMOS comparison below reproduces the
early difference without changing code. No historical source is installed into
production by these probes.

## Current boot investigation: firmware identity and observation limits

Correction: the earlier assertion that the menu probes all used the same
CMOS seed was wrong. PC110 resolves CMOS adjacent to the executable, not from
the process working directory. The failed candidate binary was located in
`s39-port-bucket-menu/`, whose adjacent DSTN CMOS was an empty first-run file;
the 128-byte seed copied into its `candidate/` working directory was not the
CMOS that the App loaded. The preserved earlier executable and the current
frame-capture executable each had the 128-byte seed alongside the executable.
The DSTN CMOS currently adjacent to the published PC110 App is also empty.
These cases therefore have different guest configuration: their differing boot
times do not establish a CPU or port-routing regression. The fixed-work probe
reports remain useful only with their actual adjacent-CMOS condition recorded;
working-directory hashes alone do not prove CMOS identity. No owner CMOS or
media was modified by this discovery. A further controlled pair used the exact
same frame-capture executable, copied alongside either the 128-byte seed or
an empty CMOS: at 524,288 retired instructions it reported respectively
3,932,346 and 3,932,418 guest ticks, both at PC `000c5e1f`. Thus the early
72-tick difference previously attributed to binary combinations is reproduced
solely by the CMOS condition. This does not by itself explain every later
divergence or prove that boot should skip firmware initialization. The empty
CMOS screen and a same-input end-to-end comparison remain under investigation.
This repeats the already documented S34 probe-setup mistake; the S34 record
was available in the investigation index and should have been checked before
claiming identical inputs. Earlier T580 empty-CMOS evidence also recorded an
interactive firmware ERROR page rather than an automatic DOS boot. The new
screen probe confirms that at 180 seconds current code presents the same
`ERROR 00173 / 00163` page with OK and Cancel; its 640×480 PPM SHA-256 is
`B9EA224DFEB96227BC89CF2B1FD9C5147324F85767C21B9FE8259DBAD9159B96`,
identical to the earlier T580 captured page. The sample at 220 seconds has
the same hash; the bounded probe ended without its automatic menu marker at
229.46 seconds. With no input, rerunning the
previously failed executable with its empty adjacent CMOS again failed its
automatic menu check after 406.06 seconds. This is consistent with waiting for
first-run configuration, not proof that the executor is stalled. A configured
CMOS still takes over three minutes to reach the menu, so the performance goal
remains open independently. The user-owned CMOS was only inspected, not edited.
The adjacent-CMOS condition of other S39 scratch experiments has not yet been
systematically rechecked; any claim there that a copied working-directory
seed was consumed must be treated as unverified until its executable directory
is checked. Equal-input comparisons among executables sharing the same empty
adjacent state can still describe that first-run workload, subject to their
other recorded limitations.

For the port-grouping candidate, a corrected x64 full-boot comparison used
the original-routing and candidate executables, each with a configured CMOS
alongside the executable and private overlay media. The original reached
`MENU-READY` at sample 230 in 234.64 seconds; the candidate reached it at
sample 212 in 216.23 seconds. Their adjacent DSTN CMOS SHA-256 was
`608df3abc3cdd9bb3e35015d2d7aa9d8756567e080643b01e593ff99d1616034`.
This one ordered pair favors the candidate by approximately 7.8%; host drift,
one-second screen polling, and a concurrent short manifest-hash check during
the candidate limit the comparison. It does not establish a statistically
stable gain, full architectural equivalence, x86 boot time, native desktop or
RDP behavior. Both times remain far from practical usability.

The existing registration rollback test was corrected to exercise a mixture
of colliding and distinct buckets within each partial-failure prefix. After
this change, the cached x64 and x86 builds each passed the source and test
manifest checks plus port-registration, RAM-port-context and neutral-link
tests (5/5 each). This is targeted evidence, not full repository validation.
Subsequent cached Release default-configuration builds succeeded on x64 and x86. The
default-configuration unit suites passed 712/712 on each width, and the
default-configuration integration suites passed 48/48 on each width,
including the registered desktop and MIDI tests. The additional My5160,
My5170 and MyDeskPro386 profile receiver sets passed 3/3, 5/5 and 6/6
respectively on each width. MyRaidenII's existing external-ROM selected-set
startup and deployed-console receivers passed on each width; these are not
gameplay or audio qualification. All 14 product files were checked for PE
architecture and hashed in [the artifact inventory](t582-s39-port-artifacts.json).
MyNES's two cached products matched their deployed files and did not change;
the rebuilt MyRaidenII receiver artifacts were stripped of compiler debug
sections. These checks do not qualify PC110 desktop/RDP performance.

The production port change adds 58 lines and removes 38 (net +20) across
three C/H files; the reused registration fixture adds 71 and removes 2 (net
+69). Manifest hash updates are separate. The candidate replaces the global
linear route owner with per-Machine bucket heads and a private registration
checkpoint; it adds no public API or second routing path. It uses approximately
2 KiB of bucket heads per x64 Machine and a temporary checkpoint of the same
size during registration. The measured speed gain does not meet the T582
usability objective, so this task remains active after receiving validation.
The reviewed Shared source/test change is commit `d5415b5b9`; MyRaidenII
receivers are `1093a09ce` and the PC receivers and evidence are `81ea72cbc`.
The NXVM documentation-governance gate also passed. The paired artifact
inventory records the Shared source revision and exact manifest hashes.

The preserved earlier menu-reaching x64 probe and the current x64 screen
probe both contain byte-identical DSTN BIOS (262,144 bytes; SHA-256
`aa2ab27a8cc4fee059238fdbd4bd869f209f5f1c3a67096fc8704ca795857121`)
and font (1,048,576 bytes; SHA-256
`9829fdb8281c12022dc3b77686044ed1a5213ab526ce4329f2841cd64171784c`).
This excludes a different embedded copy of those inputs as an explanation;
it does not establish equivalence of the complete binaries or media paths.
The PC110 runner is configured for Turbo: its pacing function does not wait
for host wall-clock time in that mode. Core still advances guest time and
device deadlines, so this is not evidence that device processing is free.

Different probes used different executable-adjacent CMOS inputs and did not
all perform the same frame and RTC observations. Their instruction counts
therefore cannot be treated as a controlled comparison of production paths.
A new current-build probe used
private media and the same menu detector as the preserved earlier probe. At
sample 180 its 640×475 screen displayed `Starting PC DOS` and the DOS Power
Manager; it reached the menu detector at sample 214 (218.69 seconds total),
versus sample 209 (212.89 seconds) for the earlier binary. This confirms
that this build can boot but remains very slow; a single ordered comparison
does not demonstrate a speed improvement. The earlier current-build run that
did not reach the marker used empty CMOS and the current empty-CMOS frame
probe confirms the firmware is awaiting setup; it was not a matching-input
regression comparison. No desktop or RDP validation is implied.

## Keyboard signal snapshot compiled check

The x64 startup sample attributed 126 of 5,155 late samples to the board's
`kbc_keyboard_inputs` symbol. In `x86_keyboard_get_signals`, the source calls
the pure scanning predicate twice, once for the scanning flag and once for the
repeat-byte count. Inspection of the current GCC Release keyboard object shows
one shared scanning/pending decision and conditional repeat-count load; the
compiler already removes this apparent repetition. Changing the source to
reuse that boolean would therefore not remove this particular machine-level
work. The snapshot and callback also carry BAT and repeat deadline facts, so
the callback cannot simply be dropped. No code, API or runtime test changed;
this x64 object inspection does not establish x86 code generation or total
keyboard cost.

## Startup middle-phase instruction samples

An x64 private-media probe sampled the existing CPU instruction record once
per 128 refresh calls over the first 100 million retired instructions. It
completed in 180,085 ms at 557,401,592 ticks and PC `000faf0d`; it was not a
complete DOS or Personaware boot. Stage boundaries are refresh-call counts,
not exact retired counts: 0–30m, 30–60m, 60–90m and 90–100m calls. The
sample records the existing prefix-stripped primary opcode after refresh and
the SMM state before refresh. Entry and other non-retiring refresh calls can
therefore sample a previous instruction; 236, 241 and 166 samples in the
first three non-SMM stages had no available opcode and were counted separately.

In the 30–60m non-SMM samples, `74` (conditional jump) occurred 38,489 times,
`24` (AND immediate) 36,436, `e4` (immediate-port IN) 31,361, and `3a`
(CMP) 25,167, out of 181,579 samples. The same four opcodes dominated 60–90m
non-SMM samples, at 34,138 / 33,257 / 29,466 / 28,802 out of 192,795. These
are sampling frequencies, not exclusive host-time measurements or proof that
all instances belong to one loop. They are consistent with the separately
traced port-61 firmware polling, but do not identify the port or establish
which individual wait is incorrect. The early 0–30m samples also contain
41,191 `ab` string stores, matching the separately measured RAM-clear phase.
SMM samples had a more distributed opcode mix. Deterministic sampling may
alias periodic loops and instrumentation affects code layout and cost; no
x86 or native desktop observation was made.

This establishes that a startup optimization targeted only at the initial
RAM clear would miss a large later workload. The next candidate should target
the existing general input/conditional-branch execution and timing costs,
while retaining each guest read, event, clock and interrupt boundary. Do not
replace these loops with a BIOS-specific skip based on the sample alone. No
production source, public API, media or released executable changed. Guest
log SHA-256:
`598A4B3519AD287D74C6530589EFB4E7CB33ED049526F5B238CF09716C02F984`.

### Conditional-branch compiler upper-bound probe

The middle-phase sample makes the shared conditional-jump helper relevant to
investigate. A disposable x64 candidate applied GCC's `always_inline` to the
existing `_e_jcc` helper, retaining the same instruction, checks and timing
paths. This compiler-specific experiment is not proposed as production code.
At the early private-media 10-million-instruction checkpoint, baseline and
candidate took 20,481 and 20,494 ms, both at 79,964,294 ticks and `0003dbc5`.
The candidate executable was 2,621 bytes larger. One ordered pair is noisy
and covers an earlier workload than the middle-phase sample; it cannot prove
equivalence or rule out every branch optimization. It gives no reason to add
a compiler-specific policy, so the candidate was discarded before x86 or
complete-boot qualification. No production source or assets changed. Guest
log SHA-256, baseline then candidate:
`E0CACEA787421633C6DA8FF6A7B919DFB82B36433D5700CBDBF164DE0AE497FC`,
`E00A4F37873DB4EF4A205750733256D0A8581C05CFE675A6566FEA2ED25E55C3`.

## Post-port startup host sample

A hidden x64 sample of the committed port-bucket build used a private executable,
media overlays and configured CMOS beside the executable (CMOS SHA-256
`608df3abc3cdd9bb3e35015d2d7aa9d8756567e080643b01e593ff99d1616034`).
Sampling covered approximately 5–60 seconds of startup; the task-owned guest
was terminated and reaped after 65 seconds. It was not a complete boot or
interactive acceptance.

The worker had 4,345 early and 5,182 late sample opportunities. In the late
interval the sampled instruction pointers included CPU refresh 308 (5.9%),
memory route resolution 186 (3.6%), board deadline observation 152 (2.9%) and
port read routing 26 (0.5%). The earlier pre-bucket sample had 376 port read
routing observations in 5,155 late samples (7.3%). These are sampled leaf
locations, not inclusive function costs; differing samples and instrumentation
make the comparison directional, not a precise end-to-end speedup. The new
sample indicates that the remaining startup cost is distributed; it does not
establish that any single remaining function explains the slow boot.

### CPU refresh address attribution

A second hidden x64 sampling run used the same private input arrangement and
recorded guest-executable instruction-pointer offsets in 16-byte buckets. The
worker had 4,254 early and 5,066 late sampling opportunities; the task-owned
process was stopped and reaped after 65 seconds. Native sampling reported no
errors. The most frequent individual late CPU-refresh bucket was 18 samples
(0.36%); the `+0x7d0` bucket had 28 early and 17 late samples and disassembly
places it among compiler-generated stack loads following the inlined execution
path. The `+0x1280` bucket had 18 late samples near instruction metadata
handling. No single sampled bucket accounts for the overall boot slowdown.
Bucketed leaf instruction pointers are not inclusive costs and do not exclude
an expensive chain distributed over multiple functions. This is startup-only
sampling, not a complete boot, x86 result, or native UI qualification. No
production change follows from it by itself.

### Keyboard input snapshots at separate scheduler boundaries

The post-bucket x64 sample identifies `kbc_keyboard_inputs` as a recurring leaf.
Static call-chain review found two owners of separate observations:
`x86_kbc8042_ticks_until_event` obtains inputs during board deadline capture,
while `x86_kbc8042_advance` obtains them after Core's timeline, bus arbitration,
and internal advancement. `advance` can drain serial output, take the keyboard
BAT and publish a delayed response before querying repeat inputs; its repeat
callback can further mutate serial state. Input can also be delivered between
separate scheduler calls. A snapshot from deadline capture is therefore not a
proven equivalent replacement for the later query. Caching it would require a
complete invalidation contract, not a local removal of a duplicate read.

The existing S8 change already avoids querying the typematic deadline when
repeat bytes are absent. No production change is justified by this static
review; no claim is made that all callback mutation combinations were tested.

### Startup sample categories and performance implication

For the preceding post-port 16-byte-bucket sample, grouping the 5,066 late
worker-thread instruction pointers by their nearest symbol name gives 2,205
CPU-labelled samples (43.5%), 984 Core-Machine-labelled (19.4%), 1,445
board/device-labelled (28.5%), 159 DLL (3.1%) and 273 other (5.4%). These
are disjoint leaf-location groups made from symbol names, not inclusive costs
or a component dependency accounting; inlined code, nearest-symbol naming,
DLL calls without stacks, sampling bias and the sampled startup interval limit
interpretation. The process was intentionally stopped before complete boot.

For scale only, reducing a 216-second boot to 60 seconds would require about
72% less elapsed time. Even the impossible elimination of every CPU-labelled
leaf sample, if those proportions held throughout boot and host time scaled
linearly, would yield about 122 seconds, not 60. Neither assumption establishes
a reachable bound or a product measurement. It does show why optimizing one
small sampled function is unlikely to be a sufficient plan: evaluate the
whole instruction-to-bus-to-device chain and any guest-work changes on their
actual semantics and end-to-end startup evidence. No timing or behavioral
contract is relaxed by this estimate.

### Flush-pending guard split experiment

CPU boundary and interrupt entry both use the existing flush-pending check.
A scratch-only candidate split its unchanged initial idle guard into a private
inline function and left the actual flush work in a private helper. The goal
was to avoid the full out-of-line call when no flush is pending; it did not
change the conditions, add state, or omit a guest operation. This is not a
qualified production patch: asynchronous assertions and full CPU behavior
were not tested.

With private media and CMOS, one x64 fixed-work baseline/candidate pair
reached 10,000,000 instructions, 79,964,294 guest ticks and PC `0003dbc5`.
The inner measurement reported 19,168/20,542 ms, respectively. The candidate
executable was 28 bytes larger. Host drift and one ordered pair prevent a
reliable regression or full-state equivalence conclusion; they do not show
the expected improvement, so the candidate was discarded before x86 or
full-boot testing. No production, interface, owner media or EXE changed.
Job log SHA-256: `FD09F285BAD9DE9E79F15FAE151DE5FEC63F3DDE3655155B6419CB7EDBFFDB56`.

### 486 decode-prefix single-scan experiment

The decode-penalty term first counts instruction prefixes and then iterates
the same prefix bytes to calculate their costs. A scratch-only candidate
combined those operations in one pass using the existing prefix classifier.
It did not change timing formulas or add production state. Review found that
the first candidate omitted the original prefix counter's explicit opcode
buffer bound, so it is not semantically qualified for malformed instruction
records and cannot be used as production code.

In one x64 fixed-work baseline/candidate pair using private media and CMOS,
the inner measurements were 18,804/20,695 ms; both reported 10,000,000
instructions, 79,964,294 guest ticks and PC `0003dbc5`. Host drift and a
single ordered pair prevent a reliable regression or complete-state claim.
The result does not establish a benefit, so the candidate was discarded
without x86 or full-boot testing. No production, public interface, owner media
or deployed executable changed. Job log SHA-256:
`D4937C0F8301CD43E0F011108609087E33F12C02DAAB30113266720F829F9248`.

The scratch candidate was then amended to retain the original opcode-array
bound and run once on x86 with the same private inputs. Baseline/candidate
inner measurements were 22,294/24,548 ms at the same 10,000,000-instruction,
79,964,294-tick and `0003dbc5` checkpoint. One ordered pair is not a full
semantic proof or a stable regression measurement; together with the earlier
x64 result it still provides no reason to adopt the change. The corrected
candidate also lacks full prefix, debug, interrupt and fault qualification.
No production source or released EXE changed. Job log SHA-256:
`09669DFA68BD404E1F5E01F9B548F66269C26307DC4E8F76DC3B345810DFDF2E`.

### Firmware callback cost after provider-cache and port changes

The address-bucket sample above can also distinguish PC110 firmware callback
leaf symbols. In 4,254 early worker samples, `mypc110_firmware_query` and
`mypc110_firmware_read` accounted for 46 and 39; in 5,066 late samples they
accounted for 27 and 28. The existing provider-cache probe accounted for one
early and seven late samples. Decode may be inlined into these symbols, while
called routines and DLL copy time are attributed elsewhere; these counts are
not inclusive time or a precise estimate of all firmware-route cost.

The query and read both perform decode because query owns route selection and
may decline, while read owns the actual transfer; a shared cached resolution
would require correct invalidation across chipset configuration, SMRAM,
access mode and aliases. The current sample does not support adding that
state and invalidation complexity as a likely solution to the startup gap.
The existing S18 provider-cache change remains in place. No extra workload,
production change or complete-boot claim follows from this analysis.

### Internal bus transaction inlining candidate

A scratch-only candidate inlined the existing transaction begin and commit
checks at CPU bus call sites, retaining owner and hold checks, trace callback
arguments and counters. It deliberately retained the old functions for
other scratch-linked objects and is not the proposed production structure.
One private-media fixed-work baseline/candidate pair per width reached the same
10,000,000 instructions, 79,964,294 guest ticks and PC `0003dbc5`. Inner
measurements were 20,958/19,974 ms on x64 (4.7% lower) and 26,267/24,720 ms
on x86 (5.9% lower). Both candidate probe executables were 2,048 bytes larger.
These single ordered pairs and matching endpoints do not prove complete state
equivalence or an end-to-end improvement. Logs SHA-256:
`53872CCF5EF770C013E67DFC9E6C2A8193949C7EE2691B26827057286BB44633`,
`AD72C73D90369BA4DF8370B0F0A86EB0E188E5E355366EC0F420DCFD74B0C6DD`.

The admitted production candidate moves begin and commit into the existing
private transaction header, with the original trace-record operation as its
single out-of-line owner; it removes the old out-of-line begin/commit bodies.
There is no new public interface, second transaction implementation or state.
The affected x86-core source manifest is updated. Existing transaction,
executor, memory-inspection and prefetch tests passed 6/6 on each width;
these focused tests do not qualify all receivers or prove startup usability.
The same-input full-menu x64 comparison and receiving results follow below.

The subsequently built production candidate was compared to the prior
port-bucket build on the x64 product-path menu probe. The executable and
128-byte CMOS seed were placed beside each other in each private run; both
CMOS hashes were `608df3abc3cdd9bb3e35015d2d7aa9d8756567e080643b01e593ff99d1616034`.
Baseline/candidate reached the same two-pixel menu detector after 222.50/216.38
seconds, with preceding common checkpoint 135,225,679 instructions and
684,593,002 ticks. The detector is not full interactive acceptance; these are
one sequential pair, affected by host noise and one-second sampling, and do
not prove a stable 2.8% product gain or complete-state equivalence. Both remain
far from usable. Native Window/RDP and x86 whole boot were not observed.
Log SHA-256 `A2774AFE0E116108C61755179A82ECB40EC32111EA438DE5CC4352BD68EC717C`.
The cached receiver and artifact results follow below.

The cached receiving run rebuilt the affected products. Both architectures
passed 712/712 unit tests. The initial default integration run passed 47/48
on each width; `lib.integration/native_midi_output` failed the same output
creation assertion. A later no-playback MIDI mapper open succeeded, and the
two failed tests were rerun once and passed individually. The transient cause
was not established, so the first failures are retained here rather than
presenting the initial full runs as uninterrupted passes. My5160, My5170 and
DeskPro 386 profile integrations passed 3/3, 5/5 and 6/6 respectively on each
width. MyRaidenII selected-set startup and deployed Console receivers passed
both widths; this is not gameplay qualification. The public source and test
manifest checks, Core dependency check, documentation gate and diff whitespace
check passed. The standalone test manifest is verified with the Core verifier
and an explicit corpus root; an earlier attempted nonexistent test-local
verifier path was an invocation error, not a product test result.

All fourteen published PE widths were validated and hashed in
[t582-s39-transaction-artifacts.json](t582-s39-transaction-artifacts.json).
MyPC110 published files matched their builds; MyNES builds were byte-identical
to its existing two published files. No PC110 native Window/RDP usability
acceptance or x86 full-menu timing was performed. Full logs SHA-256:
`040C1652745D6E70960403FAB3592A0F47802B23658B0175EFAA5551F8143EDE`,
`C3F6F30CDC9415C5E97632A5FA64DE34991555120B46737BABD2B5402A683009`,
`55BAC90C03821319D239AD7DAEB2E00094ECF6890EEADDE3E8E5B90F3D0B5802`,
`967DA35EAB5E644EC43CE85C27514053F9C5755F2BF774CC74AE742ACA119573`.

## Reusing 486 short-branch outcome through timing metadata

The nominal 486 timing calculation and generic observation both evaluate the
same short conditional branch using the instruction-entry flags. A task-only
candidate published the branch metadata during nominal calculation and reused
the existing metadata in the generic observation; it added no persistent
field. It inferred the outcome from the nominal 1/3-clock result. That inference
would silently couple branch metadata to a timing constant, so it is not a
clean or independently qualified contract even if it were faster.

At the private-media 10,000,000-instruction startup checkpoint, ordered
baseline/candidate/candidate/baseline times were 18,331 / 19,011 / 18,660 /
19,305 ms x64 and 23,821 / 21,974 / 23,539 / 23,101 ms x86. All reported
79,964,294 ticks and `0003dbc5`. Both widths reverse direction across the
pairs; sequential host variation and the early checkpoint limit interpretation.
No complete-state, later-boot or interactive equivalence is established. The
candidate was rejected for both its fragile inference and lack of consistent
benefit; production code and published artifacts were not changed. A future
design would need to pass the actual branch fact without coupling it to the
numeric timing result, and justify its interface and complexity against a
representative workload.

Diagnostic log SHA-256:
`09F73199EAE972FCBE91D2C75DEB4092F2A9E86B0AB4B8FD5C9990BCBD4E898C`.

### Direct branch-result handoff

A second task-only candidate removed the fragile clock-value inference: the
nominal 486 evaluator returned the actual short-branch decision through an
optional private output, and the source-cost path populated the already
existing formula-input flags for the generic observer. It added no persistent
field but did change a private helper signature; callers and exceptional paths
were not fully migrated or qualified.

At the same private-media 10,000,000-instruction checkpoint, ordered
baseline/candidate/candidate/baseline times were 19,473 / 20,154 / 19,752 /
18,388 ms x64 and 23,194 / 23,849 / 23,903 / 22,481 ms x86. All reported
79,964,294 ticks and `0003dbc5`. The candidate was slower in both adjacent
pairs on both widths, though sequential host variation prevents an exact
regression claim. These are not complete-state or late-boot comparisons. The
additional private contract and test-call migration are not justified on this
evidence; no production source, interface, test or published artifact changed.
The PC110 performance objective remains open. Diagnostic log SHA-256:
`42C50D759EC27B5EE3AECA9D4227CD3A8C282F89B5C4C25323AE64C796B6ED6D`.

## Inverse-pair condition evaluation

The timing helper's sixteen x86 condition codes form eight adjacent inverse
pairs. The same private helper now computes one of eight positive predicates
and applies the low condition bit as its inversion. Its flag sources, null
output behavior and callers remain unchanged; there is no added state or
alternate executor. The production source diff is +13/-18 lines, net -5.
An independent logical check compared all 32 flag combinations and 256
condition byte values; this is not a compiled-CPU or device test.

Private-media startup baseline/candidate/candidate/baseline fixed-work times
were 19,687 / 19,235 / 18,506 / 18,682 ms x64 and 23,151 / 23,311 / 25,072 /
25,304 ms x86. Every run reported 10,000,000 instructions, 79,964,294 ticks
and `0003dbc5`. Differences are small and affected by host variation; this
does not establish full-state equivalence or user-visible startup improvement.
The local source simplification does not close the PC110 usability objective.
Comparison log SHA-256:
`40AF6FB2CE2624D18F0550022C0100B2E0FD7AAB6B410639227B79E802DD6122`.

Existing branch, V30 conditional repeat, 486 nominal timing, Machine timing
preview and board control-stack timing tests passed 5/5 on both architectures;
no new test fixture was added. Cached full unit suites passed 712/712 per
architecture. Each default integration run passed 47/48: native MIDI output
could not open the host device. The other tests passed or were registered
skips. Additional My5160, My5170 and DeskPro integration sets passed 3/3,
5/5 and 6/6 on each width. MyRaidenII selected-set startup and Console
receivers passed on both widths. Fourteen product PE architectures were
checked; both PC110 deployed files matched their build outputs. See
`t582-s39-branch-pairs-artifacts.json`. No desktop/RDP performance or complete
post-change boot time was measured; PC110 usability remains unproven.

Logs SHA-256, focused / full / MyRaidenII:
`789655F7E3D379D5AD3E66C1B1E7BF4B909EB3F0788D69B6A4F270B03CDFEE53`,
`C2AA06C2BAF0C74CA7899AD417D17AC85DB43B49EAB641F33A33D3B4F69F5355`,
`967DA35EAB5E644EC43CE85C27514053F9C5755F2BF774CC74AE742ACA119573`.

### Port transaction value setter inlining probe

After the accepted begin/commit change, an independent scratch-only x64
candidate placed the existing `transaction_set_value` guard and assignment
in the existing private header and compiled CPU bus against it. Disassembly
confirmed the port-read call to the setter disappeared; other scratch-linked
objects retained the original function, so this is not a production design.
One private-media fixed-work baseline/candidate pair reported 20,272/21,046 ms
and the same 10,000,000 instructions, 79,964,294 guest ticks and PC
`0003dbc5`. Both probe EXEs had the same byte length. One ordered pair cannot
prove a regression or semantic equivalence, but shows no benefit worth a
production change, x86 qualification or full-boot run. No product source or
released binary changed. Job log SHA-256:
`4330B3287C396571C81D262A2C80EF696675B87E6A1C618D328ED6BCF73DDA59`.

### Repeated raw-provider query frequency

The raw overlay path queries a matching provider before address decoding.
If that observational query declines, the later ordinary-tier selection can
query the same raw provider at the same address when address decoding does not
change it. Skipping that second query would require relying on the exact
observational and mutation contract; it is not automatically justified merely
by identical arguments.

A scratch-only x64 counter in the actual later query location recorded 81,920
such same-address raw-provider queries during the private-media startup run
ending at 10,000,000 instructions, 79,964,294 guest ticks and PC `0003dbc5`.
It included the shutdown interval and its counter changes code layout and
runtime. The count is fewer than one per 100 retired instructions, not a
host-time measurement or a bound on later phases. It does not justify adding
a new lookup rule as a likely solution to a multi-minute boot. No query was
skipped, and no production source or product artifact changed. Job log
SHA-256 `5EBF4F816CFC0FFB886C85E397BAEAF3283EBAA5154AC73A930354EEFE032D40`.

### PIC zero-delay idle advancement

The PIC has exactly eight hardware interrupt inputs. When all eight pending
unmask-delay counters are zero, its existing advancement loop only stores
zeroes. The admitted X86 NXVM candidate checks their combined value and skips
that loop only in this case; it adds no state or interface and retains the
original path when any counter is nonzero. Existing PIC contract coverage
checked a delayed IRQ but not every index after an idle advance, so its
existing fixture now tests all eight positions, including partial and complete
delay. Source and test manifests are updated. PIC and board interrupt focused
tests passed 8/8 on each architecture after this fixture change.

One private-media fixed-work baseline/candidate pair per width reached
10,000,000 instructions, 79,964,294 ticks and PC `0003dbc5`. Inner times were
22,553/21,843 ms x64 and 26,924/25,711 ms x86. A separate x64 full-menu
comparison with the same adjacent 128-byte CMOS seed and private media
reached the existing two-pixel menu detector in 213.47/203.26 seconds. These
are single ordered comparisons subject to host drift and do not prove stable
gains, complete guest-state equivalence or native desktop usability. Both
startup times remain far too long. Job log hashes, x64 fixed / x86 fixed /
menu: `688461FE8A315BB3C8F7722672854FE2EE95D7BA642B6CDFF0B7D30F53743B81`,
`A5AFD0B0565AD331E99C7F848CCCBCFB2A716C1212AB98B4499A3271870F0009`,
`B04C3B695D651DA2DA1E9F213C8171213DD9592D1BB1E967BE1C93B89A1ED221`.
The cached Shared receiving and fourteen-artifact results follow below.

The complete cached receiver run passed 712/712 unit tests on each width.
Each initial 48-test integration run had 47 passes and the same native MIDI
output-creation failure; both MIDI tests passed when rerun individually. The
cause of the initial failures was not established, and they are not described
as uninterrupted full-run passes. The three extra PC profile integrations
passed 3/3, 5/5 and 6/6 on both widths. MyRaidenII selected-set startup and
deployed Console receivers passed on both widths; this does not qualify its
gameplay. X86 NXVM source/test manifests, dependency checks, documentation
governance and `git diff --check` passed.

All fourteen released PE widths were checked and hashed in
[t582-s39-pic-select-empty-artifacts.json](t582-s39-pic-select-empty-artifacts.json). The
MyPC110 assets matched the current builds, and MyNES assets were byte-identical
to their current builds. The complete-menu result is x64-only and the native
PC110 Window/RDP experience was not tested. Qualification, isolated MIDI and
MyRaidenII logs SHA-256 respectively:
`53D1E963A18A7C413236A142D84412B51EB6307E1E395BEB3C79E2E3216BF9A0`,
`9502CAAF1501A07A3B4F64B7F36A55ADCA50162E6123FF95337B51111CB756FB`,
`967DA35EAB5E644EC43CE85C27514053F9C5755F2BF774CC74AE742ACA119573`.

### Empty timeline advance upper-bound probe

A scratch-only scheduler candidate directly advanced the timeline's `now`
when its heap was empty and the target was not earlier than the current time;
nonempty or invalid-time cases retained the original timeline operation. This
bypasses the timeline owner and is not a production design. It was used only
to estimate the value of eliminating the empty-queue call.

One x64 private-media baseline/candidate fixed-work pair measured
18,629/18,524 ms, with 10,000,000 instructions, 79,964,294 ticks and PC
`0003dbc5` in both cases. The 0.6% difference is smaller than observed host
variation and matching endpoints do not prove event or full-state equivalence.
The experiment does not justify adding a fast path or changing component
boundaries; no x86 or full-boot test followed and no production source changed.
Job log SHA-256: `9B5997BD08DAC4E969FF84AA0ED992EAF69B0D6F32AAF8AF75447CFC7C603B9B`.

### 486SX absent-FPU scheduler candidate

An older absent-FPU ablation had incompatible compile definitions and is not
usable performance evidence. This new scratch comparison compiled the
scheduler with the same `X86_CORE_MACHINE_RUNTIME_TRACE_ENABLED=0` setting as
the linked Release libraries. It avoids the FPU deadline and advance calls
only for the already-admitted 486SX machine profile: neutral configuration
rejects any FPU other than NONE for that profile, and the FPU owner cannot
set busy when NONE. All other profiles follow the original FPU path. There is
no new API or mirrored presence state. The existing profile-gate fixture now
checks that 486SX plus 80387 is rejected.

At the same 10,000,000 instructions, 79,964,294 ticks and PC `0003dbc5`,
one baseline/candidate pair reported 19,544/17,792 ms x64 and 22,988/22,406
ms x86. These are one ordered pair per width, not stable gains or full-state
equivalence. After applying the candidate, existing FPU, timeline, executor
and profile tests passed 6/6 on both widths. Full-menu results and the final
disposition follow below. Fixed-work log hashes x64/x86:
`13B1FBFF050CAED5322CB0B504CEE24615AF575F5CE0DDF7EBB335A59893E033`,
`BECE9D779FCEF3DECFCD4A148FED4F1F43426BC14985B65F7CC1E1982FCEA327`.

The full-menu x64 private-media baseline/candidate comparison reached the
existing menu detector in 190.38/205.48 seconds with identical adjacent CMOS
hash `608df3abc3cdd9bb3e35015d2d7aa9d8756567e080643b01e593ff99d1616034`.
This is one sequential pair and host drift prevents attributing the difference
with confidence; it does not establish a real boot improvement. The candidate
also hard-codes a scheduler dependency on the CPU/FPU pairing invariant at two
sites. Given the slower observed menu run and the additional coupling, the
candidate and its temporary pairing test were reverted. The original
production sources and manifests were restored, the existing six FPU/profile/
executor/timeline tests passed again on both widths, and the two MyPC110
published EXEs were restored to their committed bytes. No whole-repository
test or interactive acceptance was claimed for the rejected candidate. Menu
log SHA-256: `0A40EFED68778DF39A0A49F0B56F24032033B93196C60A8757EBE9F7E1E82F7C`.

### Post-optimization Personaware input response

A hidden x64 product-factory probe linked the current PIC/transaction build
and used private copies of the disks and the configured 128-byte CMOS beside
its executable. The existing two-pixel menu detector matched at sample 242.
The saved menu frame was visually inspected and showed the selected World Clock
entry. The probe submitted one Enter make/break pair through the existing
Machine input queue, waited 15 host seconds, then saved the published frame.
Visual inspection of that frame shows the World Clock window with Tokyo and
New York clock fields, demonstrating a response through this direct Machine
input path. The process cleaned up successfully after 261.53 seconds; final
RTC seconds read 21, after 20 in the displayed menu frame. The delay includes
the explicit 15-second observation interval and is not an input latency
measurement. This was a single x64 probe, not native Window/RDP input, an x86
run, a complete application suite or proof of real-time usability. It changes
neither owner media nor production code. Menu, response frame and guest-log
SHA-256 respectively:
`85A35680232CA0AD623DB7DD18D23A79396931ACB133791581D40C37048AEF41`,
`2758C0B659C28139981D9B3ABFCFDAB070E088EB3B604BEC68D3C7DF16875B6C`,
`B7AAAC0D54CE5BDCD3C36AA0AB3157F00D5B45A16886BE7CC459BEEA7B7F7AE9`.

### PIT-only zero-tick call guard

S38 had tested PIT and RTC zero-tick guards together; this distinct scratch
candidate guarded only the existing primary PIT call, leaving RTC and all
clock conversion and trace ordering unchanged. At the same private-media
10,000,000-instruction, 79,964,294-tick and `0003dbc5` checkpoint, one
baseline/candidate pair reported 23,786/23,089 ms x64 and 25,851/26,109 ms
x86. These ordered pairs are subject to host noise and do not prove complete
state equivalence. They do not establish a cross-width benefit; a board-side
guard would also repeat the PIT owner's existing zero-tick behavior. The
candidate is not retained, and no production source, API or artifact changed.
Logs SHA-256 x64/x86:
`000F1A0C94F51D760EB7CBA7811DC4ABE8E4393BDAABA26523BD992FDCB4EF3F`,
`321D0D4DFCDFAC905AA1C3802D02D4E5FE3F52A2C698E74F10A50F372D1E6476`.

### Host power-policy check

A read-only host query reported the Windows Balanced power scheme and a
configured maximum processor state of 100% for both AC and battery. The
processor registry reported Ryzen AI 9 HX 370 and a nominal `~MHz` value of
1996; that registry value is not a live execution-frequency measurement.
Windows CIM processor counters were inaccessible in the execution sandbox.
These observations do not establish active power source, actual frequency,
thermal behavior, core placement or the cause of the measured startup
variation; no host power policy or system setting was changed. Prior S39
processor-affinity probes remain the evidence for placement variation.

### PIC source-mask expression probe

The board converts eight per-IRQ asserted-source counts into one level mask
on each refresh. A scratch candidate expressed the existing conditional OR
as a Boolean shift and OR, retaining all eight counts, ownership, callbacks
and refresh ordering; it added no state or public API. Disassembly confirms
the compiler emitted a different mask calculation, not a removed refresh.

At the fixed private-media checkpoint (10,000,000 instructions, 79,964,294
ticks and PC `0003dbc5`), one baseline/candidate pair reported 17,585/17,859
ms x64 and 26,252/27,059 ms x86. Single ordered pairs are noisy and do not
prove a regression or full state equivalence, but neither shows a reason to
change the production expression. No full-menu comparison or receiver gate
was run and no production file changed. Log hashes, x64 then x86:
`EC1EE9B1EB675F1AB91DA3FB69AFDDD3315F815D42D698BD4E8227E40109B011`,
`CDBEF02503E1F55445285FFDD6E5D06C1F8E0CB3F2C4206DD44F9D9E1FD5E53A`.

## Post-bucket startup leaf distribution

Re-aggregating the existing 5,066 late x64 worker instruction-pointer samples
by nearest symbol gives 206 symbols: the largest one accounts for 328 samples
(6.5%), the five largest 19.1%, the ten largest 31.3%, and the twenty largest
49.5%. The largest named buckets include CPU refresh (328), Machine run (201),
memory-route resolution (144), instruction boundary (138), elapsed-time
publication (136), keyboard-input collection (129), board-deadline observation
(127), DMA advancement (116), timing selection (110), and 486 source timing
(110). The complete ordering also contains system-library samples.

These are exclusive sampled instruction addresses, not inclusive function
costs: inlined code is charged to its enclosing symbol, sampling can alias,
and the data is one x64 startup run rather than a controlled comparison.
They show why another isolated one-function micro-optimization is unlikely by
itself to make the three-to-four-minute startup usable. The next investigation
should follow duplicated work across the whole retirement/timing/device chain,
measure the inclusive candidate on both host widths, and verify the actual
DOS/Personaware endpoint. No new production change or speed claim follows from
this aggregation.

## Already-minimal board deadline conversion count

The board's deadline aggregation can in principle avoid converting a positive
device deadline when its current earliest positive source deadline is already
one: a successful conversion cannot produce a smaller positive value. A zero
device deadline must still report immediate, and conversion failures must retain
their existing outcome. This observation does not authorize skipping deadline
callbacks or changing scheduler ordering.

A task-only x64 build counted calls to the existing clock-conversion aggregation
in the private-media startup fixed-work harness. Of 10,298,529 calls, only four
had a positive device deadline and an already selected one-tick source deadline;
zero calls had a zero device deadline. The run reached 10,000,000 instructions,
79,964,294 guest ticks, PC `0003dbc5`, in 22,105 ms reported by the harness.
Instrumentation affects timing and this covers only the early x64 workload;
nevertheless, four eligible calls cannot materially improve this checkpoint.
No production optimization, public interface or release artifact was changed,
and no full-product performance or test claim is made.

## Startup 486 timing-term sampling

A task-only x64 build sampled the existing 486 source evaluator once per 256
calls during the private-CMOS and private-media startup checkpoint. It logged
10,002,313 calls and 39,071 samples. Accumulated unserialized TSC deltas were:
nominal 3,573,651; L2 fallback 5,500; memory 3,500,411; decode 2,600,866;
and bus 2,016,780. The four common terms therefore account for approximately
30.6%, 29.9%, 22.3% and 17.2% of their own sampled term intervals. These
numbers exclude the surrounding evaluator and Machine, and cannot be treated
as total CPU percentages. The fallback is called conditionally, so its small
total should not be interpreted as its cost per call.

The probe reached 10,000,000 instructions, 79,964,294 guest ticks and PC
`0003dbc5`; the harness reported 19,520 ms. Deterministic sampling can alias
the instruction stream; added timers and rewritten scratch control flow alter
compiler layout, and TSC reads are not serialized. It is an early startup x64
observation, not a full-boot speedup or correctness qualification. Together
with the earlier World Clock term sample, it supports investigating the
nominal and memory term implementations first, rather than assuming decode or
bus conversion dominates. The previous prefix-sharing and zero-access
candidates remain rejected for the reasons already recorded. No production
source, interface, release artifact or owner media was changed.

## Deferred complex-instruction timing fields

Disassembly of the current x64 Release nominal 486 timing evaluator shows
that it reads the ModR/M extension and computes protected-mode eligibility
before classifying simple arithmetic, register and conditional-branch opcodes.
A task-only candidate moved those two calculations into the existing complex
opcode switch, with the same formulas and no persistent state or new executor.

With private copied media and the configured adjacent CMOS, ordered
baseline/candidate/candidate/baseline fixed-work measurements in milliseconds
were 17,886 / 20,740 / 20,740 / 21,019 on x64 and 25,417 / 26,505 / 26,799 /
26,379 on x86. Every run reported 10,000,000 instructions, 79,964,294 ticks
and PC `0003dbc5`. These are sequential samples, subject to host drift and
compiler-layout effects; the checkpoint is not full-state equivalence.
The candidate did not demonstrate a consistent benefit and adds a nested
scope, so it is rejected. No production source, interface, test, owner media
or executable was changed. Full-boot and desktop/RDP tests were not run for
this rejected scratch candidate.

## Deferred operand-cache timing fields

A scratch candidate moved operand width and missed-read arithmetic after the
existing zero-miss return, and evaluated the existing call-gate eligibility
only for the applicable opcodes. The special REP, task-switch, gate and
interrupt paths retained their order and formulas. No public interface, new
state or production code was changed.

At the configured private-media startup checkpoint, ordered baseline /
candidate / candidate / baseline measurements were 21,361 / 21,960 / 21,476 /
20,973 ms on x64 and 25,652 / 22,044 / 22,312 / 22,691 ms on x86. All runs
reported 10,000,000 instructions, 79,964,294 ticks and PC `0003dbc5`. Host
drift and compiler layout affect these sequential samples; the endpoint is
not complete state equivalence. The x64 pairs both favor baseline, while x86
favors the candidate, so the observations do not establish a cross-width
benefit without an apparent x64 cost. The candidate is not adopted, and no
product boot, desktop/RDP or full regression qualification is claimed.

## Split nominal timing scratch candidate

A scratch candidate factored complex nominal-486 cases into one private
function, leaving the existing simple cases and each timing formula with a
single owner. The x64 Release compiler inlined the extracted function back
into its caller and still emitted the full register-save prologue, so this is
not evidence that function-frame work was eliminated. It did change the
generated code layout. The candidate is not yet admitted or qualified.

At the private-media startup checkpoint, baseline / candidate / candidate /
baseline times were 18,747 / 18,100 / 15,881 / 18,287 ms on x64 and 23,388 /
22,692 / 21,864 / 23,722 ms on x86. All reported 10,000,000 instructions,
79,964,294 guest ticks and PC `0003dbc5`. These sequential measurements can
include host drift and do not prove complete-state equivalence. A matching
configured-CMOS full Personaware menu comparison completed: baseline and
candidate reached the menu in 205.77 and 216.09 seconds respectively, using
the same adjacent CMOS hash and private copies of the same media. This is one
sequential pair and does not establish a precise regression, but it does not
show an end-to-end improvement. Both still take more than three minutes.
The candidate adds a private function and does not reduce the observed prologue
or code complexity, so it is rejected without modifying production source.
No x86 full-boot or native desktop/RDP result is claimed.

## Idle floppy head-unload guard

The existing floppy advance checks all four head-unload slots even when its
head-unload mask is zero. A temporary candidate returned immediately from that
one helper in the zero-mask case; it retained the FDC time update and all
other advancement and event paths. No production source was changed.

At the private-media fixed-startup checkpoint, ordered baseline / candidate /
candidate / baseline results were 19,861 / 19,606 / 19,831 / 22,442 ms x64
and 27,406 / 26,755 / 23,772 / 22,549 ms x86. Every run reported 10,000,000
instructions, 79,964,294 ticks and PC `0003dbc5`. The x86 direction reverses,
and host drift limits all four pairs. The candidate adds a branch, affects a
fixed four-slot scan, and does not establish a cross-width gain or a useful
end-to-end speedup. It was rejected without additional qualification. No
desktop, RDP or full-boot comparison was performed.
## Startup operand-cache timing populations

A task-only x64 build sampled the existing operand-cache penalty entry once in
256 calls during the private-media startup checkpoint. Among 39,070 samples
from 10,002,116 calls, 36,145 had zero non-descriptor misses and zero locked
reads, while 21,860 had a REP prefix; no sampled task switches, eligible call
gates or software-interrupt opcodes appeared. These categories overlap: in
particular, zero misses does not authorize returning before REP processing,
because its previous-miss handoff and modeled flag may still matter. The run
reached 10,000,000 instructions, 79,964,294 ticks and PC `0003dbc5`.

This is a deterministic early-start x64 population sample, not a timing or
full-boot result; aliasing and instrumentation can affect it. It identifies
the zero-miss and REP intersection as worth inspecting, while ruling out
assuming that an unconditional zero-miss shortcut preserves the existing
contract. No production source, release artifact or owner media changed.

A second independently instrumented run sampled 39,087 entries; 20,890 had a
REP prefix and zero data misses, locked reads, permission-field misses,
missed permission fields and previous-iteration misses. This is an overlapping
population, not proof that every such record is valid or that all REP opcodes
have the same semantics. It reached the same fixed instruction/tick/PC
checkpoint with private media and CMOS.

A scratch-only early return for that zero-field case was then tested. Its
correctness still depends on the preceding access validation and on the form
having come from the existing string decoder; neither complete error-path nor
all-profile equivalence was proven. Ordered baseline/candidate/candidate/
baseline fixed-work times were 19,723 / 18,841 / 18,723 / 18,105 ms x64 and
22,368 / 23,084 / 24,135 / 27,361 ms x86. Both widths reverse direction and
show substantial host drift; the measurements do not establish a benefit.
The extra special branch is rejected without a full boot or receiver run.
No guest operation was intentionally omitted and no production file changed.


## Combined raw and decoded provider scan on current routing

The current memory resolver walks the provider list once for raw decode and
again to collect decoded-address candidates. A scratch candidate combined
these traversals while retaining raw-query priority, selected-alias priority,
and the subsequent replacement/ordinary/fallback order. It relies on the
declared observational query contract and has not been qualified against all
provider and reentrancy cases. It adds no persistent index.

Using configured private CMOS and private media, ordered baseline/candidate/
candidate/baseline fixed-work results were 19,720 / 17,728 / 16,763 / 17,502
ms x64 and 21,967 / 22,474 / 22,273 / 22,127 ms x86, all at 10,000,000
instructions, 79,964,294 guest ticks and PC `0003dbc5`. A separate x64
baseline-then-candidate full-menu pair reached Personaware in 196.78 and
223.06 seconds. These are sequential runs with host drift, not proof of a
precise regression, but they do not establish an end-to-end gain; x86 short
measurements also favored baseline. This revisits the earlier S15 idea on
the current provider and port-routing implementation. The candidate is
rejected without production changes or full receiver qualification.

## Early-start instruction and STOS attribution

A task-only x64 CPU build sampled one in 256 `ExecIns` calls, attributing the
recorded primary opcode after the call. At the fixed private-media startup
checkpoint, it recorded 39,080 samples and 59,113,494 accumulated
nonserialized TSC units; `AB` (STOS word/dword) accounted for 20,597 samples
and 39,970,702 units, or approximately 67.6% of these sampled execution
intervals. This is the first ten million instructions, not the whole boot;
the later firmware polling phase has a different opcode mix. Instruction
execution excludes the separately measured timing selection and Machine
scheduler. Deterministic sampling, interrupts, source instrumentation and
compiler layout limit attribution.

A separate scratch build sampled the existing STOS memory-write and index
advance calls, one in 256 STOS entries. It recorded 5,348,088 STOS calls,
20,890 samples, 10,991,170 TSC units in `_s_write_es`, and 1,175,020 in
`_kas_move_index`. These are inclusive intervals with instrumentation, not
whole-program percentages. The result makes the existing write route a more
productive next investigation than simplifying index arithmetic. Both probes
reached 10,000,000 instructions, 79,964,294 ticks and PC `0003dbc5`; neither
proves complete state equivalence or desktop usability. No production source,
public interface or released executable was changed.

Two further task-only x64 samples narrowed that write. Of 5,853,109 logical
write calls, one-in-256 sampling recorded 22,863 intervals: segment-to-linear
conversion accumulated 1,069,000 TSC units, alignment checks 1,107,790, and
the existing linear write 9,353,698. A separate linear-write probe recorded
5,853,393 calls and 22,864 samples: single-page physical translation accounted
for 1,584,520 TSC units and the subsequent physical write for 10,916,719;
no sampled access crossed a page. A third probe sampled 25,915 of 6,634,352
486 physical-write calls: the cache-route query accounted for 2,055,460 TSC
units, initial cache-way lookup 1,656,900, and bus write 10,582,080. All
sampled calls reached that bus write. Each run reached the same fixed
10-million-instruction / 79,964,294-tick / `0003dbc5` checkpoint.

These are separate instrumented runs with nested, nonserialized intervals;
their counts and times cannot be summed or treated as total-program cost.
They direct the next investigation toward the existing CPU-to-Machine bus
write route. In particular, the cache-way lookup after the bus callback cannot
simply reuse the pre-write result: the callback may invalidate the memory
view, as documented in `t582-s26-smm-cache-lookup.md`. The unchanged instruction
checkpoint does not prove full guest equivalence or interactive performance.

## Startup bus-write components and combined span proof

Two task-only x64 samples followed the existing write chain at the private
startup checkpoint. CPU bus-write sampling counted 6,634,612 calls and 25,916
samples: sampled TSC totals for write-buffer preparation, external-cycle
begin, Machine write, external-cycle commit and write completion were
1,557,380 / 1,595,150 / 6,913,576 / 1,355,600 / 2,335,900. A separate
Machine-write run counted 6,634,295 calls and 25,915 samples: transaction
begin, board cache-control callback, physical write and transaction finish
totaled 2,246,640 / 1,010,600 / 5,450,982 / 1,601,708 TSC units. Intervals
are nested across the runs, nonserialized and affected by instrumentation;
they cannot be added or interpreted as whole-program percentages. Both runs
reached 10,000,000 instructions, 79,964,294 guest ticks and PC `0003dbc5`.

The ordinary multibyte RAM path separately scans providers to prove that no
provider partially intersects the access after its route was resolved. A
scratch candidate collected that same conservative proof during the existing
raw-provider scan, retaining the no-mapping, no-address-mask and no-wrap
requirements and existing bytewise fallback. Other resolver calls passed no
proof output. The candidate has not been qualified for all overlapping,
failure or callback cases and introduced a private output argument.

Baseline/candidate/candidate/baseline fixed-work results were 16,781 / 17,114 /
16,465 / 16,769 ms x64 and 20,610 / 21,945 / 21,537 / 20,619 ms x86. All
reached the same recorded checkpoint. The x64 direction changed, while both
x86 pairs favored baseline; these sequential samples are sensitive to host
variation, but do not support adopting the more complex route. No full-menu,
desktop or receiver qualification was run and no production file changed.

A later task-only x64 sample followed the existing physical-write routine:
6,633,878 calls, 25,913 samples, of which 20,659 reached the ordinary RAM
copy. Sampled TSC totals were 2,695,180 for route resolution and 1,959,400
for the subsequent split/span decision across sampled calls; RAM-copy,
CPU-snoop and remaining parity/observer work totaled 1,016,560 / 1,027,340 /
789,840 on the sampled RAM path. The run reached the same 10,000,000
instruction / 79,964,294 tick / `0003dbc5` checkpoint. Substeps have
different populations, timer overhead and nonserialized TSC, so the totals
cannot be treated as an exact time budget. They confirm that the earlier
combined span-proof candidate targeted real repeated work; its measured x86
regression nevertheless does not justify adopting it. No production or
artifact change followed.

## Coarse empty-provider-page upper bound

An instrumented private-media startup run recorded the eight registered
provider windows present when its first plain-span proof occurred. The windows
include `30000h..3ffffh`, `a0000h..fffffh`, `1400000h..140ffffh` and the high
reset alias; sampled spans also covered RAM from 1 to 20 MiB. This is one
initial snapshot, not a guarantee that providers cannot subsequently change.

A task-only 8 KiB bitmap prototype used empty 64 KiB pages to bypass both the
provider traversal and plain-span scan only with no address mask or mappings.
Its process-global storage and invalidation by owner pointer and provider
count are unsafe for general multiple-Machine use or same-count replacement;
it is solely a cost upper bound, not a product design. Baseline / candidate /
candidate / baseline fixed-work times were 18,348 / 17,257 / 17,172 / 16,590
ms x64 and 21,317 / 21,711 / 23,082 / 24,170 ms x86. All reported 10,000,000
instructions, 79,964,294 ticks and PC `0003dbc5`. Both widths reverse
direction between pairs, with visible host drift. The samples do not establish
an improvement sufficient to justify per-Machine index storage, invalidation
and semantic qualification. No production code, media or executable changed;
no full boot or desktop validation was performed for this unsafe prototype.
## Memory-route end arithmetic consolidation

The existing route resolver calculated `(address - provider_start) + bytes`
for every matching raw/decoded provider and mapping. It now computes each
address plus bytes once, then subtracts the provider/mapping start. Both
expressions use unsigned 64-bit arithmetic, so associativity modulo 2^64
preserves the existing result even on overflow. Query order, route precedence,
fallback, failure handling, public interfaces and state are unchanged. The
production change is +6/-3 lines in `memory.c`; the source manifest is updated.

The directly compiled private-media fixed-work baseline/candidate/candidate/
baseline timings were 23,662 / 21,767 / 21,066 / 21,250 ms x64 and 25,682 /
25,237 / 26,849 / 27,530 ms x86. Each reached 10,000,000 instructions,
79,964,294 ticks and PC `0003dbc5`. The configured-CMOS x64 full-menu pair
reached Personaware in 229.55 / 223.42 seconds. These sequential comparisons
are subject to host drift and one-second menu sampling, not a precise gain;
the product still needs several minutes and is not considered usable yet.

Five directly affected memory/ROM routing tests passed on each host width.
The cached x64 full unit run passed 712/712. The x86 full unit run had
711 passes and one `emulator.verifier-negative` timeout; that test passed
independently in 15.03 seconds. The x64 default integration run passed 47/48;
native MIDI output failed while opening the device, then passed its independent
retry. The x86 default integration run passed 48/48 including MIDI. This
records the failures and reruns rather than describing either original run
as entirely green. Additional My5160/My5170/DeskPro receiver integration
passed 3/3, 5/5 and 6/6 respectively on each host width. MyRaidenII's
selected-set startup and product-console receivers passed on each width;
these do not establish arcade gameplay. Fourteen published executables were
checked for PE width; both MyPC110 assets match their current build outputs,
and both MyNES artifacts were unchanged. See the route-end artifact inventory.
No new native PC110 Window or RDP responsiveness qualification was performed.

Logs SHA-256, targeted / full / x86 continuation / external receiver:
`07DF03A8312B50CF2EDD372D968FE3983560A851B3A145E730245D38DE4946B7`,
`7938AF2A9913EDC6E15238E9754674FC40AE73A4EB57CE23BEDAF7420F99506B`,
`F444BF5A97E359F3C89190089543F807FF80CB6F206EEB93C415710B35A3E079`,
`147575917C39B79B44488CF5B05F74D4E5C7009E2E0918A35122B90F2F2C6238`.
## Write-buffer preparation and duplicate observation

An x64 task-only startup sample observed 6,634,054 eligible write preparations.
Of 25,914 samples, 22,424 had an empty buffer; 21,430 had cache-disable set,
all with an empty buffer; and 2,966 had a full buffer. The first ten million
instructions reached 79,964,294 ticks and `0003dbc5`. Deterministic sampling
can alias and this does not characterize a later workload. In the empty,
cache-disabled case the existing code calls the drain function, which checks
the same observation predicate before seeing the empty buffer.

One scratch candidate returned immediately for an empty buffer. Ordered
baseline/candidate/candidate/baseline times were 21,281 / 22,768 / 21,283 /
21,418 ms x64 and 25,483 / 25,587 / 26,567 / 26,621 ms x86. A second scratch
candidate factored the existing drain loop into a private helper, allowing the
already-validated write preparation to avoid repeating the predicate; the
public drain still performed its validation. Its fixed-work results were
21,996 / 20,166 / 20,794 / 21,304 ms x64 and 27,228 / 26,072 / 25,322 /
25,541 ms x86. All runs reported the same instruction/tick/PC checkpoint.
These sequential measurements are subject to host variation and do not prove
full state equivalence.

The helper candidate's configured-CMOS x64 full-menu comparison took 220.79 /
222.77 seconds for baseline/candidate. This single pair does not prove a
regression, but it does not demonstrate a meaningful product improvement; the
helper adds code and a second entry into the drain loop. Neither candidate was
adopted. No production source, owner media or released artifact was changed,
and no x86 full boot or desktop/RDP result is claimed.

## FDC pending-seek representation

The 8272 controller previously kept four byte-sized pending flags and scanned
them to form the low status bits, including when no seek was pending. The
candidate stores those same four bits in one private mask and skips idle
seek/deadline scans. It changes no public API or guest timing contract. On both
host architectures the private data structure decreased from 256 to 248 bytes.

Scratch fixed-work baseline/candidate/candidate/baseline measurements were
18,587 / 18,108 / 18,839 / 19,057 ms on x64 and 22,733 / 22,119 / 22,773 /
24,130 ms on x86, at the same instruction/tick/PC checkpoint. One x64
configured-CMOS full-menu pair measured 219.63 seconds baseline and 209.59
seconds candidate. These sequential samples are noisy and do not establish a
general speedup or full-state equivalence. More than three minutes to reach the
menu remains a usability problem; this improvement does not close T582.

The production change was +12/-15 lines across the two private FDC files;
the existing causes test was +14/-6, including all four drive bits and reset.
The targeted FDC tests passed 7/7 on each architecture. Cached unit tests passed
712/712 on each architecture. The x64 integration run passed 47/48: native
MIDI output could not open the host device, and an isolated retry also failed.
The x86 integration run included successful real-firmware cold and resume
handoff tests; two default-profile tests failed after two qualification scripts
accidentally used the same build tree concurrently. With exclusive use restored,
both passed on retry. The other x86 integration results and x86 My5160, My5170
and DeskPro profile runs passed; profile counts were 3, 5 and 6 respectively.
The x64 profile runs passed with the same respective counts. MyRaidenII receiver
probes passed for both architectures. All 14 published binaries were checked
for their PE architecture; see `t582-s39-fdc-mask-artifacts.json`. Native PC110
desktop and RDP responsiveness were not verified in this batch.

## Current deployed first-run configuration check

On 2026-11-01, a read-only inspection found the deployed DSTN CMOS adjacent to
the executable still has zero bytes; it is not tracked by Git. The App's CMOS
loader treats an empty file as absent, and the DSTN firmware definition has no
embedded CMOS seed. Consequently the configured-CMOS menu timings above do not
describe this current first-run input. The earlier empty-CMOS screen capture
documented an interactive firmware ERROR page, not an automatic DOS boot.
That earlier capture was not rerun here and cannot establish the current
desktop state. Neither the user CMOS nor media was modified. Even with the
separate configured-CMOS input the measured menu time remains approximately
210 seconds, so the first-run setup distinction does not resolve the
performance problem.

## Current x86 Personaware menu observation

A task-only x86 build of the existing menu probe used the current release
libraries and FDC source, private copies of both media and the same adjacent
configured CMOS seed used for the x64 FDC comparison (SHA-256
`608df3abc3cdd9bb3e35015d2d7aa9d8756567e080643b01e593ff99d1616034`).
It exited successfully after 274.15 host seconds with `MENU-READY` at
sample 270. Its last recorded quantum before the marker had retired
135,225,679 instructions at 684,593,002 guest ticks. The previous x64
candidate reached the same two-pixel marker in 209.59 seconds, with the last
recorded quantum at 135,529,605 instructions and 685,553,826 ticks. The
samples are separate runs, not an interleaved host-width benchmark; the
sampled quantum does not establish identical complete state. Both results
show multi-minute startup, not acceptable responsiveness.

The menu detector checks two graphics pixels and samples at one-second
intervals; it neither independently proves the entire menu frame nor reports
the DOS arrival time or native desktop/RDP behavior. User CMOS and media were
not touched. x86 guest-log SHA-256:
`61D1BF95E50D6096DB0C98684C79EE3874D7BB96731B00068E67A0AFD8600EC7`.

## Replacement-provider scan population

A task-only x64 counter in the current memory resolver counted the early
10-million-instruction private-CMOS startup checkpoint. Of the decoded-route
collections it observed 5,407,726 with no geometric match, 1,925,516 with
one, and 12,330,151 with two. Every two-match sample had an ordinary route
registered before a replacement route. The replacement tier's query returned
success 13,944,733 times and a non-success zero times in this run. Counts
include setup and the stopping interval; instrumentation changes code layout
and timing, and these are not whole-boot, exclusive-cost or x86 counts.

A scratch candidate queried replacement routes as they were found in the
existing geometric scan, retaining ordinary and fallback selection afterward.
It relies on the declared observational-query contract and was not qualified
for all callback/reentrancy cases. One ordered baseline/candidate pair at the
same 10,000,000 instructions, 79,964,294 ticks and `0003dbc5` checkpoint took
18,586/20,275 ms on x64 and 24,106/23,945 ms on x86. The x64 sample was
slower; the x86 difference is small compared with run-to-run variation. These
are not full-state or end-to-end comparisons. The candidate duplicates query
and failure handling and does not demonstrate a useful benefit, so it was not
adopted or subjected to product qualification. No production code, user media
or published executable changed.

Population log SHA-256:
`FBD3FAFC617C6229596BDA50B7670BE243EC7771C393217935C21EB334B3F9A7`;
comparison log SHA-256:
`F3A242438563BA3C8E38D7A914DAD4EB1D7E58D8624DB5BADA17F93E0157CB89`.

## Invalid completed-observation initialization

At each CPU refresh, the previous completed observation is invalidated.
Source inspection found that `next_lexeme_components` is the only member of
that invalid record read before the existing reconstruction; the public copy
path fully initializes its output when no completed observation is valid.
The previous whole-record clear was therefore replaced by clearing this one
member, with the validity flag and full successful reconstruction unchanged.
The reset path still clears the complete record. This is a reduction in stores,
not a new execution path or change to the published observation contract.

A current-source scratch comparison used the same private CMOS and media
copies and baseline/candidate/candidate/baseline order. At 10,000,000 retired
instructions, 79,964,294 guest ticks and `0003dbc5`, x64 times were 18,199 /
18,175 / 18,956 / 19,121 ms; x86 times were 23,580 / 22,795 / 23,435 /
23,524 ms. Matching checkpoints do not prove complete state equivalence and
these differences are small relative to host variation. In particular they
do not establish a useful whole-boot improvement or PC110 usability.
The x64 generated refresh code was inspected and the candidate removes the
prior whole-observation stores. Diagnostic log SHA-256:
`09A46E6013B509B32457017CCCBE9CCC7A4AE4EE88FBE753B3D937C43E515FB2`.

Production diff: CPU source +3/-1 and the existing completion fixture +5/-0,
plus the two manifest hash replacements. The fixture poisons the only field
used before reconstruction and checks it and the copied output after interrupt
or failure. Focused completion, 486 timing, execution-bus, fault-event and
Machine-preview tests passed 5/5 on both widths. Cached full unit tests passed
712/712 on both. In the initial default integration runs, native MIDI output
failed to open the host device on both widths; x86 MyNES native-window also
failed its screen-pixel assertion once and passed an isolated retry. All other
default integration results passed or were registered skips. The three PC
profile integrations then passed 3/3, 5/5 and 6/6 on each width; MyRaidenII
startup and product-Console receivers passed on both. All 14 artifact PE
widths were checked and both PC110 deployments matched their build outputs;
see `t582-s39-observation-artifacts.json`. No native PC110 desktop/RDP or
post-change complete-menu timing was measured. T582 remains open.

Logs SHA-256, focused / full / x86 continuation / MyRaidenII:
`6BEB508CBD2672DFAB989D86F3BA8ED45D2FE5F9BC5C4957E0F10BBEA2E7005A`,
`DC353636111277A0A5BF10A112335930D593466CCBECFDEBB13B7A631104A5C7`,
`CE7DA85D3FA8513766BFDB5C064107BE267CDE06D36D3761C31AD1589F213D1C`,
`967DA35EAB5E644EC43CE85C27514053F9C5755F2BF774CC74AE742ACA119573`.

## Inactive external-cycle observer

For a frozen machine contract without page timing or any nonzero wait window,
the external-cycle callback cannot add guest wait ticks. Release builds also
compile out its runtime tracing. Binding it in that configuration nevertheless
called it on each external cycle. The candidate avoids the callback only for
that combination; configured timing and trace-enabled builds retain it. V30 bus
accounting remains in the CPU and does not depend on this callback.

On the same host with the same private CMOS and disk copies, the baseline
reached the sampled menu marker at 215 seconds and the candidate at 198
seconds. This is one sequential x64 comparison, sampled at one-second
intervals; it is not a desktop or RDP measurement, and a menu marker is not
proof of all guest state. The 17-second reduction does not make the observed
startup delay acceptable. Independent fixed-work ABBA runs reached the same
instruction count, guest ticks and PC; x64 was approximately 8% faster, while
the x86 results ranged from approximately 4% faster to indistinguishable from
noise. Full-state equivalence and practical interaction remain separate gates.

The production change is +11/-3 lines in Machine; the existing prefetch
fixture is +7/-3, plus two manifest hash replacements. The fixture now
checks the first and last wait-window slots; existing page-timing and
observable-transaction tests cover the other enabled contracts. The cached
dual-width unit suites each passed 712/712. Default integration passed 47/48
on both widths: only native MIDI output failed to open the host device. The
PC110 real-firmware cold/resume tests passed; the 5160, 5170 and DeskPro
receiver groups passed 3/3, 5/5 and 6/6 on each width. MyRaidenII's startup
probe and product Console receiver passed on both widths; these are receiver
checks, not game acceptance. All fourteen PE widths were checked; each PC110
artifact matched its build. MyNES artifacts remained byte-identical. See
[artifact hashes](t582-s39-idle-observer-artifacts.json).

Qualification, receiver, menu comparison and fixed-work logs have respective
SHA-256 values `4F2D34BD402BD1CEFC6AECCE8804F85155C32B45352869DC632E0FCC815D69C4`,
`967DA35EAB5E644EC43CE85C27514053F9C5755F2BF774CC74AE742ACA119573`,
`BC860B1AC3B645DF4949BB154B4851EA0D689D31B18627D6EA783CD52B430782`, and
`9DC6DD6481DC3A9E18CF663DB33D1F3D4760D422D5E33233BD4A17AD2B6DABF5`.
No native PC110 Window/RDP startup or interaction timing was performed.

## Current synthetic CPU/Machine comparison

After the inactive-observer change, the existing same-instruction synthetic
fixture was relinked to the current cached x64 and x86 release graphs and run
sequentially. For ten million NOP/JMP instructions, the standalone CPU,
neutral Machine and PC110-EMU reference respectively took 3,683 / 10,529 /
1,130 ms on x64 and 5,995 / 13,561 / 1,027 ms on x86. The Machine ended at
`000f0001` and 40,000,014 ticks on both widths; the other two fixtures do not
share its bus and timing contracts. The reference reported no BIOS-loop
escapes in this synthetic workload. Each is a single ordered sample on a
loaded host, not a controlled paired regression comparison or a PC110 boot.
The measurements confirm that the broad gap remains, but do not justify
subtracting the CPU time from Machine time as an exclusive Machine cost.

The previously observed 1.5-million FDC-status poll storm is not a proven
main boot cause: inspection of the old diagnostic script and retained output
confirms that the incomplete SMC CONFIGURE candidate had the same configured
CMOS adjacent to its executable, yet did not reach the menu after 405 seconds.
That candidate did not implement complete device semantics, so neither this
result nor the boot baseline excludes a correct controller improvement. The
next implementation candidate still needs a coherent contract and evidence
across CPU execution, timing and the device path, rather than repeated local
optimizations presented as a usable PC110.

The synthetic logs' SHA-256 values, x64 then x86, are
`6C57F5CA4F4A7FAE047F69CBD0E1D4275C72757AA56502AC0F615E56A213A44F` and
`6A398D7B0D6E4F93A266C1B0536B8F3C86116A0CA465F3F9D5F2B02AA997B5E1`.

### External-cycle invalidation call upper bound

The Machine still calls its private external-cycle invalidation on each CPU
refresh. A task-only candidate replaced that particular call with the exact
three false stores in its existing implementation. The Machine is non-null at
that call site. This was an upper-bound probe for moving the single helper to
a private inline definition, not an approved duplicate production path.

With private media and configured CMOS, baseline/candidate/candidate/baseline
at ten million instructions took 19,850 / 18,584 / 19,399 / 18,861 ms on x64
and 23,758 / 24,140 / 23,925 / 23,168 ms on x86. Every run reported
79,964,294 ticks and `0003dbc5`. One sequential ABBA series remains noisy
and matching these counters does not prove complete state equivalence, but
the x64 order crosses and both x86 candidate samples are slower than their
adjacent baselines. It does not justify expanding inlining across the other
call sites or another complete receiver build; no production change was made.
No menu or RDP timing was measured for this candidate. Log SHA-256:
`CABB0D9EFF45853D740039E7879A8207DCBD5B7B7FACE925336EBB2771C639ED`.

## Current startup host samples after observer removal

A hidden x64 private-media harness linked the current cached Release graph
after `d6eca2814`; its executable SHA-256 was
`36F5CBF53492E07C6E32EC194115855399B10541F0AA52A283185F7F518743B1`.
The configured adjacent CMOS SHA-256 was
`608DF3ABC3CDD9BB3E35015D2D7AA9D8756567E080643B01E593FF99D1616034`.
The existing native sampler observed the guest worker at 5–60 seconds and
intentionally stopped and reaped its own guest at 65 seconds. It is not a
complete startup or a desktop/RDP measurement. Suspending the worker during
sampling perturbs execution; nearest-symbol attribution records leaf addresses,
including inlined work, rather than inclusive timings.

Among 5,146 late-interval worker samples, CPU refresh had 404 (7.9%),
Machine run 206 (4.0%), CPU instruction boundary 179 (3.5%), elapsed-time
publication 167 (3.2%), 486 memory penalty 166 (3.2%), memory routing 158
(3.1%), and board deadline observation 138 (2.7%). Other work is distributed
across many symbols. The disabled external-cycle observer had no recorded
samples; the invalidation helper had 11. A prior pre-removal sample recorded
31 observer and 3 invalidation samples, but different runs, code layout and
host behavior prevent interpreting the difference as a measured speedup.
No single sampled leaf explains the approximately 198-second menu wait.
Continue investigating cross-boundary duplication without removing the
clock, instruction, fault or device event contracts.

Sampler and guest logs have SHA-256
`2A8E45B1A02C2D71E95AB5F17D96AF3205620463EF6D10FE71D36584FBAC291E` and
`2C151DB8B5E2C5EC50094B055B21FE2291D563D9F9DE52504ED223695BD06BAB`.

## Same-Machine CPU and bus upper bounds

The earlier standalone-CPU comparison also changed the bus and Machine
initialization. A task-owned variant instead constructs the same neutral
486SX Machine for each run, with the same synthetic reset jump and NOP/JMP
loop, then calls its existing CPU refresh directly. This preserves that
Machine's CPU and bus configuration, but intentionally omits Machine device
time advancement, scheduler and retirement boundaries. It is not an eligible
execution path, nor an equivalent guest-state comparison.

For ten million calls, the first ordered x64 run reported 9,632 ms for
Machine run and 7,995 ms direct; x86 reported 11,814 and 9,322 ms. Each
reported PC `000f0001` and 40,000,014 CPU or elapsed ticks. A second ordered
series reported x64 9,059 / 7,679 ms and x86 12,460 / 8,817 ms. The direct
loops explicitly capture timing each refresh, while the normal runner does
its own captures; matching PC/ticks does not prove complete guest state.

The second diagnostic also replaced only the direct CPU's memory read/write
callbacks with a private flat buffer containing the same synthetic code and
reset alias. It measured 5,272 ms x64 and 6,468 ms x86, with the same reported
PC and ticks. This bypasses Machine transaction admission, memory routing,
providers, callback effects and other real behavior, so it is an invalid
product path and only a rough upper bound on that group of costs. Samples
are single fixed-order runs affected by host drift, and the synthetic loop is
not PC110 boot. They nevertheless direct the next investigation to the
existing CPU-to-Machine memory path as well as timing and device publication;
they do not prove which memory substep is responsible or justify bypassing it.

First-series logs, x64/x86, SHA-256:
`331AE9CA67610DE330A0C1D38C8195FCFBA8FA0EF4F97A378BB4D5415100B99B`,
`FE118BAA4F7CE5F5EC8328FBB13AD769254888C5642671E496D08B04B3D368F9`.
Second-series logs, x64/x86, SHA-256:
`E55D01AD357E66C1F3B89DCDEAC2C29FB590616643CCF73E0B038C7AF9CBACD8`,
`057190B36B320DCD2E0FECAA0750758A3341916A5C4CC5AB151045B5C2054E02`.

### Separating transaction admission from memory routing

Using the same neutral Machine direct-refresh synthetic loop, a task-only read
callback retained the original memory inspection, reset fetch, instruction
fetch, ordinary read, fallback and optional cache-control calls, but deliberately
omitted transaction begin/commit/cancel. It is an invalid production path:
transactions also protect lock, failure, observation and reentrancy semantics.
The NOP/JMP fixture has no board attachment and is not a PC110 boot.

In baseline / bypass / bypass / baseline order, x64 reported
6,249 / 7,019 / 7,018 / 7,715 ms and x86 reported
8,531 / 8,888 / 9,385 / 9,416 ms. Each run reported ten million refreshes,
40,000,014 CPU ticks and PC `000f0001`. Each ordered sequence shows host drift,
and the endpoints do not establish complete state equivalence. Neither width
demonstrates a stable benefit from bypassing transaction admission alone, so
the previous flat-buffer upper bound cannot be attributed to that one owner.
No production code was changed.

The synthetic reset alias does make its mapping list nonempty, whereas a
previously instrumented real-PC110 startup cache-fill sample observed no
mapping list or address mask in 2,566,175 probes (S37). Therefore its
conservative alias-span behavior must not be generalized to real PC110.
The remaining productive investigation is the real startup memory path,
including provider queries, access sizes and failure/alias semantics, rather
than optimizing the synthetic fixture's alias case.

Log SHA-256, x64 then x86:
`7D80B66065D387C344D8EDEE1E0A19B202A908191BE6D41C79D68AA6CC08B974`,
`D1102E88AF4E74B84C07EF93E3EB45D0AF61E9E039CBED885E490556C6526551`.

## Current dual-width CPU stage and 486 term samples

Task-only copies of the current CPU instruction and timing-model sources
sampled one in 256 entries on the private configured-CMOS startup path.
Each run stopped at 30,000,000 retired instructions, 195,624,840 guest ticks
and PC `000f6440`. Stage 0 covers the first ten million refresh or timing
source entries, and stage 1 covers the remainder; these are not exact retired
instruction boundaries. The guest was not run to DOS or the menu. TSC reads
are nonserialized, fixed-interval sampling may alias loops, and instrumentation
can affect code layout. The separate stage and term runs are not additive or
exact host-time accounting.

For stage 0 / stage 1, x64 sampled instruction execution totals were
65,354,640 / 78,131,692 TSC units and timing-selection totals were
23,736,334 / 51,005,690, over 39,063 / 78,136 refresh samples. On x86 the
corresponding execution totals were 80,174,528 / 95,970,945 and timing totals
36,357,163 / 71,100,478, over 39,063 / 78,144 samples. The later phase has a
higher timing-to-execution ratio in both instrumented runs; this does not
measure the whole boot or prove the cost of the enclosing Machine path.

In the separate 486 source-term run, among late-stage x64 samples the nominal,
memory, decode and bus intervals totaled 7,722,760 / 7,087,734 / 10,486,492 /
4,071,200 TSC units; x86 totals were 7,915,287 / 9,450,520 / 6,206,050 /
5,183,285. Each main term had about 78,000 samples. Nested timer overhead is
included, and the fallback L2 term was invoked only 54 times in each late
stage. These observations suggest examining the shared 486 timing data flow,
not simply optimizing the RAM-clear instruction or assuming the same term
dominates both architectures. Previously rejected prefix and PUSH/POP probes
remain rejected; no production or public interface was changed.

Stage and term logs have SHA-256 respectively
`3FCC5BC4CE05B52CB57E28CC92BCDEB083D272AAE013316BCEF73FD45232CDA6` and
`54CAF31B81D80B72BF9081D21C20D6B87AB1315B625B1BA7DE1174C88469922D`.

## Direct timing-result upper bound

The current timing selector has one production caller. On non-V30 profiles it
collects source metadata in the CPU context, builds a local result, then copies
the result back. A task-only candidate wrote the local result fields directly
into that existing context and removed the final copy; V30 kept its original
path. This adds no persistent object, but it is not a qualified replacement.
In particular, the 8086-family LOCK helper originally reads the zero-initialized
local result's allocation flag before the context flag is copied into it; the
candidate instead exposes the context flag earlier. Failure, preview callback,
debug and all-profile equivalence were not established.

At the private configured-CMOS startup checkpoint, ordered baseline /
candidate / candidate / baseline times were 19,124 / 19,534 / 18,076 /
18,963 ms x64 and 22,100 / 22,347 / 24,208 / 24,783 ms x86. Every run
reported 10,000,000 retired instructions, 79,964,294 ticks and PC `0003dbc5`.
Both widths reverse direction between pairs; host drift and the common endpoint
do not establish a reliable speedup or complete-state equivalence. The
possible behavior difference and lack of stable benefit do not justify this
design. No production source or released artifact changed, and no full boot
or RDP result was measured. Log SHA-256:
`6008E7244A137E3AA06DAB641E3FE915301B47CAB1D4C19277717B85FCC0FE45`.

## Startup branch timing and lexical-preview attribution

A task-only x64 build sampled instruction execution and timing selection by
decoded primary opcode at one in 256 CPU refresh entries over a 60-million
retired-instruction private-media startup. It ended at 365,365,343 guest ticks
and PC `00038887` in 91,987 ms. The first and second stages are split at
30 million refresh entries, not exact retirements. Fixed-interval sampling,
nonserialized TSC, possible interrupt/decoder-record reuse and code-layout
changes limit attribution; the run did not reach the menu or test RDP.

In the later stage, opcode `74h` accounted for 18,651 samples and 15,986,580
sampled timing TSC units, compared with 66,228,510 for timing selection across
the entire sampled stage. Execution of those samples totaled 7,580,196 units.
These are inclusive sampled intervals, not a precise percentage of whole boot.
Other common sampled instructions included `E4h`, `24h`, `3Ah` and `E2h`.

A separate link-wrapper run on the same configured CMOS and private media
split its stages by the last completed Machine quantum's retired count, at
30 million instructions. It counted 3,477,525 target lexical previews
attributed to `74h` in the later stage; all returned an available lexeme. Of these, 13,533 sampled preview
calls accumulated 10,214,868 nonserialized TSC units. The timing and preview
figures are from different runs and cannot be subtracted or added, but support
investigating the existing target-preview path rather than only the simple
branch-condition expression. The previews also feed the existing completed
CPU observation; removing them solely because this workload has no subscriber
would change that contract.

A further separate run sampled whether the retained prefetch range was
eligible under the previously tested guard. None of 13,533 later-stage `74h`
preview samples had an eligible retained range; across all later preview
opcodes, 2,988 of 24,517 samples had one and 2,750 were decodable from it.
This explains why the earlier retained-prefetch candidate's first-ten-million
coverage cannot be assumed for these common taken branches. It does not prove
that a different cache-based preview can preserve paging, permissions, memory
mutation, faults and all CPU profiles. The first attempt at this diagnostic
failed to compile because it referenced a translation-unit-private scanner;
the corrected version used the existing public scanner only when its options
matched. No production source, interface or release artifact changed.

Opcode-stage, preview-count and prefetch-sample logs have SHA-256 respectively
`6879F3A4F9AD6020DD536BB31A9896059B6F1E2B41E00B14D297E6E5C0ED4E1C`,
`8D426F5CBCDA6A4C03E16A565BEBBDD8CB1567547B25DC8D372F6F061EC4DF95`, and
`2F6C2F8053250A93867551A4A04BC991779E31497497C6F847F14CDE5928E177`.

### Target CPU-cache coverage in the boot branch workload

Two task-only x64 runs sampled one in 256 lexical-preview calls attributed
to `74h`, restricted to the 486SX, real mode with paging disabled, the default
80386 ModR/M policy and a target within the current CS limit. They queried
the existing Machine cache-address owner and compared a read-only decode from
resident CPU cache data with the ordinary preview. Both used configured CMOS,
private media and the 60-million-retirement endpoint; the 30-million-stage
split follows the most recent completed Machine quantum. The diagnostic adds
extra cache-address queries and decoding, so timings are not a performance
comparison. Neither run reaches the menu or tests x86/RDP.

In the later stage, 11,522 of 13,533 eligible `74h` samples had their first
target byte in a resident CPU cache line. Decoding from that first line matched
the ordinary result in 11,521 cases and could not finish in one. However,
the existing preview's complete bounded prefetch fit in that first line in
none of those samples. Looking up a second consecutive decoded cache line
covered all 11,522 hit samples; their complete-range decoded results matched
the ordinary preview. The early stage had no cache hits in 13,359 samples.

This is a promising population observation, not an equivalent implementation:
the diagnostic performs additional observational address queries, leaves the
real preview intact, and only compares lexeme length and component count.
Before any candidate can replace that path it must preserve complete prefetch
range semantics, CS checks, page and A20 boundaries, cache lookup/failure
ordering, callback effects, debug/fault behavior and mutation invalidation.
It must also retain the original route for all other cases without creating
an alternate CPU executor. A bounded scratch prototype and dual-width
end-to-end measurement are required before production admission.

Single-line and two-line diagnostic log SHA-256 respectively:
`2DA67FA06FE901716C8D03C9B02D814E7EBFDEFA36E2810128D7EE0F46EF6E5E`,
`F4B29B0F5FF05BB92A85FD8A9AA8D7705024539C977314677C9E99DD08DA8C8A`.

### Two-line cached-preview scratch candidate

A task-only CPU copy tried decoding the complete existing prefetch range from
resident cache lines when the CPU was a real-mode, non-paged 486SX and the
existing retained prefetch was unusable. The original preview remained the
fallback. The candidate did not change guest instruction timing formulas or
introduce a second executor, but adds a separate lexical-preview branch and
extra Machine cache-address queries. Its equivalence for callback effects,
failure ordering, address changes and all CPU/debug paths was not established;
it is not production code.

At the private configured-CMOS ten-million-instruction checkpoint, ordered
baseline / candidate / candidate / baseline times were 19,000 / 19,538 /
19,130 / 19,592 ms x64 and 24,012 / 25,639 / 24,806 / 24,721 ms x86.
Both widths reverse direction across adjacent pairs, and the x86 first pair
shows a possible regression. All reported 79,964,294 ticks and `0003dbc5`.
This early phase had no cache hits in the prior sampled `74h` population.

The relevant 30-to-60-million-instruction phase was then compared once on
x64 using the same private CMOS/media. Baseline / candidate took 42,077 /
43,304 ms in that interval, and 90,867 / 94,253 ms across the full first
60 million instructions; both reported 365,365,343 ticks and `00038887`.
The split is at a Machine-return boundary, and a single sequential pair is
susceptible to host drift. These numbers neither prove a regression nor full
CPU-state equivalence, but show no useful improvement to justify adding the
complex branch and its extensive semantic proof. No production code, tests,
media or released EXE changed, and full-menu/RDP timings were not measured.
Log SHA-256, early and later probes respectively:
`1B2A80CCDD59A0683D6CBC4D94BDD6BEDE9EE38FEFF1744EE88F9A0BBE1D79C6`,
`989A48CB7D9032DE5A53693C4C067A6F10D544400B6A19C11DB1692845588FD3`.

## Cache-fill query distribution during startup

An x64 link wrapper counted the real Machine cache-address queries across a
60-million-instruction private-media startup; it sampled one in 256 calls with
nonserialized TSC reads. Both runs reached 365,365,343 ticks and PC `00038887`.
The split at 30 million follows completed Machine quanta. Instrumentation,
fixed-period sampling, host scheduling and the early/middle boot scope limit
cost interpretation; neither run is a complete DOS/menu or RDP measurement.

In the first/later stages, the first run counted 45,866,294 / 36,325,760
ordinary address lookups and 15,408,679 / 9,879,845 fill probes. Fill was
allowed only 71 / 553 times. Among sampled fill probes, the first stage used
10,810,450 TSC units across 64,411 samples; the later used 4,195,847 across
38,131. These include wrapper and timer overhead and are not exclusive
whole-process times.

An independent region-counting run found 8,941,036 of the early fill probes
and 9,292,100 of the later probes in physical `30000h–3FFFFh`, with zero
fills admitted there. Early firmware `F0000h–FFFFFh` accounted for 6,462,944
probes with zero fills; later it accounted for 577,787 probes with 474 fills.
The sums are separate instrumented runs, not a complete state-equivalence
proof. The physical addresses alone do not prove the selected route at every
query. Current PC110 firmware policy explicitly rejects caching below its
video/firmware region, which includes the `30000h` SMRAM overlay; that region
can also resolve to ordinary RAM depending on chipset state. S17–S19 already
identify dynamic mapping, cache invalidation and chipset-evidence constraints;
an earlier unconditional SMRAM-cache experiment did not show a benefit.
Therefore these counts do not justify skipping provider resolution, caching a
negative result across mapping changes, or changing the guest cache model.
The next candidate must preserve those boundaries and demonstrate a net
benefit on the actual boot rather than merely remove a frequent callback.

Stage-query and region-count logs have respective SHA-256 values
`8C3BED0822EA2FCEE0FBA945809894E483E69E95D71F5BC7F35B6802ACFD5A1F` and
`8C35220E74B576AEF3B404C6F73A7F0D1FB0C85BFFECEEF6346DB372B6F16288`.

### Candidate-provider region index upper bound

A task-only resolver prototype kept the existing provider-query callbacks and
route ordering, but cached candidate provider indices for the last physical
and decoded 64-KiB regions. Its index is process-global and identifies a
provider set only by Machine pointer and count; it fails for multiple
Machines, same-count route replacement, callback mutation and concurrent use.
It is deliberately unsafe and is not an admissible product design. Crossing
region boundaries followed the original scan. The experiment asks whether
even this inexpensive shortcut merits designing a correct per-Machine index.

On x64 with the same configured private CMOS and disk inputs, one ordered
baseline/candidate pair reached 60 million retired instructions at
365,365,343 guest ticks and PC `00038887`. The first 60 million took
90,197 / 86,845 ms; the interval after the most recent Machine boundary at
30 million took 41,153 / 40,490 ms. These single sequential samples are
subject to host drift, and matching endpoints do not prove route, state or
fault equivalence. They do not establish a substantial benefit in the actual
polling phase, much less the roughly tenfold improvement needed. A correct
index would need explicit registration/removal and reentrancy handling plus
per-Machine memory; this evidence does not justify that complexity. No
production code, interface, media or EXE changed; x86 and full-menu/RDP
were not tested for this invalid prototype. Log SHA-256:
`0C4774DD99E3F3B74DAA107B2842D6E2BBF4EA0DB46342F73E72DBB951F4AE84`.

## Preview copy attribution and scalar-copy experiment

A task-only x64 `memcpy` wrapper counted calls originating on the Machine
worker during the first 60 million retired instructions, split by the last
completed quantum at 30 million. It does not observe inlined copies; counting
and caller attribution add overhead, and sizes/counts are not inclusive host
time. In the later stage it counted 36,308,124 calls of 380 bytes at the
instruction-boundary snapshot site, and 6,275,991 calls each of 380, 736 and
820 bytes at three lexical-preview copy sites. The associated guest reached
365,365,343 ticks and PC `00038887` in 99,675 instrumented milliseconds.
The instruction snapshot still owns fault/restart semantics; counts alone do
not justify removing it.

A separate task-only candidate split the preview's 820-byte scalar-context
copy around the completed observation and task checkpoint, which are not
obviously consumed by lexical fetch. It retained the original CPU, cache/TLB
borrow, decoder and preview execution paths, but increased the number of copy
calls. This is not qualified: indirect fault, callback, debug and all-profile
reads of omitted fields still need proof. With the same private CMOS/media,
one x64 baseline/candidate pair reached 60 million instructions at the same
ticks and PC. The total was 93,440 / 92,670 ms and the 30-to-60-million
interval was 42,165 / 44,511 ms. The split is at a Machine boundary; host
drift and fixed order prevent a precise regression claim or state-equivalence
conclusion. The extra copy segments and semantic proof are not justified by
this result. No production, test, media or released artifact changed; x86,
full boot and desktop/RDP were not measured for the candidate.

Copy-attribution and candidate logs have SHA-256 respectively
`AE5BAF5FE10177DDC52A05460185E02E49A84FBB6D627C59D96C3CEA53636BDC`,
`2EC590D1B1711180BFC85443F0B7641479C43479B250823B6A774D0FC304D777`.

## Current startup milestones for emulator usability

After the owner clarified that usable system startup and interaction, rather
than game performance, are the objective, a hidden x64 product-path probe used
the current cached Release libraries, private copies of the configured
20 MiB media and CMOS, and ten-second published-frame samples. It first showed
`20096 KB OK` at sample 20, retained that text through sample 130, and first
showed `Starting PC DOS...` at sample 140. Samples 140–160 showed that text;
samples 170–200 were graphics. The existing two-pixel Personaware detector
fired at sample 206 and the probe exited successfully after 210.25 seconds.
The detector does not establish complete menu correctness or interactive
responsiveness. These coarse samples include capture and logging overhead and
do not establish an exact phase boundary or imply the CPU was idle. There was
no native Window or RDP measurement, and owner media was not modified.

The early firmware query optimization was already investigated in S13 and did
not establish a gain; caching a decoded result also requires preserving mutable
chipset mappings and route precedence. Whole-Core LTO was investigated in S34
and did not establish a sufficiently simple build solution or acceptable boot
time. Neither is newly adopted. Further work must improve the real firmware to
DOS path and subsequent interaction, with actual end-to-end comparison; this
observation is not a performance fix or acceptance.

Probe executable SHA-256:
`07D68A9DEC9837A1BE54DC00D71DD6918B3E68F16C6B171F01C8FFB8B2F42B90`;
output SHA-256:
`607ED1651AF0B90C5AC4FC5FA561724C645AC98DFBBEA85C551EB1067FB56834`.

### Instruction-byte clearing upper bound

A scratch x64 candidate omitted the instruction-byte buffer clear in
`ExecBoundary` entirely. It is deliberately unsafe: bytes beyond the decoded
length can remain stale in externally visible debug observations, and other
indexed consumers have not been qualified. It cannot be used as a fix.
The baseline and candidate used current Release libraries and separate private
copies of the same input. In one ordered pair, the existing fixed-work probe
reported 19,143 and 19,305 ms respectively for 10,000,000 instructions; both
ended at 79,964,294 ticks and PC `0003dbc5`. The latter five-million interval
was 10,379 and 10,377 ms. These are noisy early-startup samples, not complete
state comparisons or evidence about the full boot, but they do not support
adding a more complicated clearing/observation protocol to remove this cost.
No production code, public contract or owner media changed. Baseline and
candidate output SHA-256 respectively:
`F66006E32A6FBE5D843543A07B3EFC37BA8B2FB27269B6BED2732C88D01B9CA6`,
`D2EE1F485C9AB6010A30FB7B866E1D7B5FEB64E3CEECE8FC6AFC5E9FA464D8D1`.

### Private event-access record layout

`_e_begin_access` saves and clears existing 486 timing counters before event
arbitration, and `_e_end_access` restores them. A scratch-only candidate
reordered the fields of its private temporary `cpu_entry_access` to follow
their source-record locality; field meanings, counter access and event
ordering were unchanged. Its compact layout remained 96 bytes on x64. GCC's
disassembly of `_e_begin_access` shrank from 114 instructions / 496 bytes to
94 / 432 on x64; x86 remained 180 / 816. This is compiler-specific evidence,
not proof of semantic equivalence across all events or profiles.

At the private configured-CMOS 10-million-instruction checkpoint, both widths
reached 79,964,294 ticks and PC `0003dbc5` in every run. In candidate/baseline
then baseline/candidate order, x64 reported 18,420 / 19,067 then 18,298 /
19,479 ms; x86 reported 22,875 / 22,253 then 24,133 / 23,188 ms. The
direction reverses on both widths, so these noisy fixed-work pairs do not
establish a stable speedup or full guest-state equality. The candidate changes
layout without reducing source or algorithmic complexity and is not adopted;
there was no production, API, media or binary change. The x64 candidate then
baseline output hashes were `9C176F853487BA228DA88D1ABC975C1ABFC686D4E4E8C5CA31466583A6CFFD56`
and `FA59C14E66230D907653D42AF87880DEE2050A3C54B79B8B11E810C628043369`;
the reverse-order outputs were `EABB763E5806F4584098F5C5D0D782E67CFF491E21DE1C927C07C5395B47B2A2`
and `3100E0781AEA3288EFF26BEFFCF2FE0D2878F0E9B469DEBE6B40CC138BB7E72C`.

### SMM admission stack-frame split

The x64 Release object enters `_smm_enter` at both existing arbitration
boundaries and saves eight registers plus reserves a 680-byte stack frame
before checking the profile and pending SMI. A scratch candidate moved the
unchanged admission predicate into a small caller and kept the actual image
and entry work in one private callee. It introduced no persistent state or
alternate execution path. GCC inlined the admission into its callers and
retained the large entry body out of line. Complete event, failure and other
profile equivalence has not been qualified.

In one ordered baseline/candidate pair using the existing private-media
10-million-instruction probe, x64 reported 17,969 / 18,298 ms and x86
22,658 / 22,604 ms. Every run reported 79,964,294 ticks and PC `0003dbc5`.
These are early-startup samples, subject to host drift and not complete
state or end-to-end comparisons. They do not demonstrate a worthwhile gain
from the extra source helper, so it was not adopted. No production source,
public interface, owner media or published executable changed. Output
SHA-256 in x64 baseline/candidate, x86 baseline/candidate order:
`ACDA2447D5D24B510BB80E0AC558ECF9DAAE40AA01DDAE408D6C45CADBC7CB3E`,
`A0FE86D861E1A02ED82A04DC857EB14E04219CE230CECC35BEE750B8B74356B5`,
`05396BF51C61DCD6917D5E1280CAC4B663D6628647809943A7CC9454D096633D`,
`325694966C6A1D8AD02D0CB3933FAF7CEEE9DEA1A43CA7D43E31C9204876E344`.

### Execution-provider clock without a time consumer

Core currently advances its private execution-provider clock even if the
frozen execution provider has no `advance_time` callback. The PC110's bound
provider has a reset callback and a null `advance_time`; it receives no such
clock value. The only production references to this clock are initialization,
reset and this scheduler call. Configuration freezes provider topology before
normal reset/run; the provider's callback lifetime and immutability remain a
contract assumption rather than a new enforcement mechanism.

A scratch candidate moved the existing clock advancement into the existing
non-null callback branch. It reduced the source by one line and left the CPU,
board and device clock routes unchanged, but stopped maintaining that private
clock while there was no consumer. That is an internal-state change, so it
would still require a complete provider-lifetime and reset review; it is not
established equivalent for a caller that mutates its borrowed callback table.

At the private configured-CMOS 10-million-instruction checkpoint, ordered
baseline/candidate/candidate/baseline times were 19,172 / 19,850 / 18,698 /
18,406 ms on x64 and 25,195 / 23,787 / 22,942 / 24,048 ms on x86. Each
reported 79,964,294 ticks and PC `0003dbc5`; this is not full-state or boot
equivalence. Host drift is visible, and both x64 pairs favor the baseline
while both x86 pairs favor the candidate. Given the apparent cross-width
tradeoff and the lifetime question, do not adopt the candidate based on this
test. No production code or EXE changed. The corresponding output SHA-256s
in table order, x64 then x86, are:
`A5EF76095315F63280B7C1CABD0C8C7EE5411000DA3FA14882D87CB7C4DE4C9C`,
`1A871B90CECCB769DDBA8E809AA68B0D7C251100AF4A4B51FC301C8C2ABAE928`,
`EA2DAF0BA680CBC1149311DAD7007748E329F075D2C4CD98FE8F885A5CD3B6F9`,
`20E515417D5F0393F912D3F8910253583F025AFABF1E74120D69C9A4EC7B33D6`,
`CEC7771CE0FDEFF22F9BF66CF0055BA8775368244BFB8DE10AC09474F66E750D`,
`A4E91BFEFC7D34376A88E8CA5C5867E851CF576C9A4E7E10A310B38EAAF4A0D0`,
`3092382E41706B65EBC0036C0841709037F66BA86858F0203CA04F15F1D1B936`,
`182BB99C3E23DB3C702E4A84D3948EDDF7355E4F83DED88614C143710B4315D8`.

### Host duration of the first unsupported FDC command

A current x64 private-media run added a host monotonic timestamp to the
existing FDC trace and stopped at 60 million retired instructions. The actual
8272 model remained unchanged. At guest tick 284,206,909, firmware wrote the
SMC `12h` command; at tick 284,206,953 the model exposed its invalid-command
result. The next traced read of that result occurred at tick 350,929,703.
The host timestamps on the write and read were 264,760,853 and 264,777,454 ms:
16.601 seconds elapsed, corresponding to approximately 2.02 seconds on the
33 MHz guest axis. The 60-million endpoint recorded 1,376,335 total FDC status
reads and completed in 90,441 ms at 365,365,343 ticks / PC `00038887`.

This confirms that the unsupported-command interval is material to the slow
boot, but the elapsed interval is not an exclusive measurement of CPU time
spent polling: firmware and other device work may also run within it. This
is one instrumented configured-CMOS run, not a complete boot, x86 or RDP
measurement. The earlier incomplete SMC handshake caused substantially longer
startup; correct `12h`/`13h` handling must therefore be evaluated with the
subsequent reset, status and no-drive behavior as a whole, not advertised as
a proven speedup or installed as a no-op. No owner media or production file
was changed. Trace SHA-256:
`649E7A7CEABA09FF9C7F8CBAFB29A1D75436EE141818CD558B54788D45843FC1`.

### Preview checkpoint copy reduction

Lexical preview copied the decoder's previous `oldcpu` checkpoint before
`ExecInit` immediately called `ExecBoundary`, which overwrote that checkpoint
from the preview CPU before using it. The only operation before that overwrite
which could inspect prior instruction facts examines the copied opcode bytes,
length and timing-write fields, not `oldcpu`; V30 lock completion is suppressed
in preview mode. The private `oldcpu` field is now adjacent to the already
omitted access array, allowing the existing two-segment preview copy to omit
both. It does not add a branch, cache, persistent state, public interface or
different execution path. The private x64 record remains 13,056 bytes; its
initial preview copy decreases from 736 to 356 bytes. On x86 the private
record remains 13,024 bytes and the copy decreases from 704 to 324 bytes.
Full profile, fault, debug and receiver qualification remains required.

Scratch builds compiled all five CPU translation units against either the
original or the reordered private header. One ordered x64 baseline/candidate
pair over 60 million retired instructions reported 89,970 / 87,691 ms total
and 41,710 / 39,887 ms for the final 30-million stage. The corresponding x86
pair reported 117,399 / 118,014 ms total and 55,674 / 55,190 ms for that
stage. Every run recorded 365,365,343 ticks and PC `00038887`. One sequential
pair per width, host drift and a common checkpoint do not prove complete
state equivalence or an end-to-end speedup; this is a fixed 380-byte copy
reduction per preview, not an asymptotic complexity claim. No DOS, desktop
or RDP usability claim follows. Logs, x64 then x86, baseline/candidate:
`97121AD6965E4245C0024E425B27FD87C8AC5445C60EEB38CD09ADF32EEA7242`,
`0A6F3FB9883CF9B8C575F9728076FE669BFE938F74AA86C1518BFCE25CD7E225`,
`21414FFB11FA0023BA2D6CECC5E3E7F30884153659FE0CA3E68273EBC263F504`,
`B7B0FBFEB2C462D4554D2B073AD538B142BFACD953DBCC4118000D29A1AC2C5D`.

The cached Release builds and the existing 40 focused CPU/preview tests passed
on both widths. The first x64 build had four output-open linker failures during
parallel linking; resuming the same tree at four jobs completed before testing.
The full unit suites passed 712/712 on both widths. Default integrations passed
47/48 on each: the only failure was the native MIDI output creation assertion.
A separate WinMM diagnostic found one output device but `midiOutOpen` on the
mapper returned native error 1; this does not establish why it failed or prove
playback. The owner has allowed this host-dependent failure to be recorded
separately. My5160, My5170 and DeskPro profile receivers passed 3/3, 5/5 and
6/6 per width. Both MyRaidenII selected-set startup and product-Console probes
passed on each width; they do not establish gameplay. All fourteen release
files were checked for PE width; deployed PC110 and MyNES files matched their
cached outputs. The other PC deployments were exercised by profile integration.

The same private-media x64 frame-sampling harness first observed
`Starting PC DOS...` at sample 130 and its two-pixel menu marker at sample 197;
it exited in 200.83 seconds. The earlier baseline first observed DOS at sample
140 and the marker at sample 206, exiting in 210.25 seconds. One ordered pair,
ten-second DOS sampling, host drift and the incomplete marker do not establish
a reproducible or sufficient startup improvement. Neither run measured native
Window/RDP interaction. The observed delay remains unacceptable, so T582 stays
open. Focused, full, external and boot log SHA-256 respectively:
`19EFADB6DD84116C2047FF377D1F026695CC6F6783DC87087CA48D279E4E746D`,
`DB34FEFB612FABBD1E21582354A7428EA892EE29A440654903DE0B79ED0C715B`,
`967DA35EAB5E644EC43CE85C27514053F9C5755F2BF774CC74AE742ACA119573`,
`5E9B861B876582CE3C7139184CB246459E18013B04183CCF887DD7E64CAE8F21`.

Reviewed Shared source commit `dfba2e6cb` changes two private CPU files by
+4/-2 lines, plus two manifest hash replacements; no test source or public
interface changed. Twelve PC/MyRaidenII receiver EXEs changed; both MyNES EXEs
were unchanged and matched their build outputs. The
[artifact inventory](t582-s39-preview-checkpoint-artifacts.json) records all
fourteen PE architectures and hashes, together with the Core manifest hashes.
The two MyRaidenII receiver artifacts are commit `f5b86bbc0`.

### Preview context snapshot grouping probe

A task-only candidate placed the private completed observation and task
checkpoint beside the already excluded preview cache/TLB fields, reducing
the x64 initial context-copy prefix from 820 to 380 bytes without adding a
copy segment. The x64 private-media baseline and candidate reached the same
60-million-instruction checkpoint, 365,365,343 ticks and PC `00038887`, in
90,267 and 91,681 ms respectively; the last 30-million interval took 41,609
and 42,202 ms. These single ordered samples do not prove a regression or full
state equivalence, but show no gain that justifies changing private layout.
The scratch candidate was not adopted; x86, complete boot and interactive
responsiveness were not tested. No production, test, media or EXE changed.
Baseline and candidate log SHA-256 respectively:
`E4576D4A8F1387981E5004AA6A133A7BF3D7FAC0225C4D6283EF41ECB78B22B6`,
`AF6673FB583342F9F29FBA4B554FB09E2A13213BA30DA06AA489886E3F805579`.

### System startup and command-response baseline

A hidden x64 product-factory/Machine probe used the current cached Release
libraries and private copies of the same configured CMOS and disk images.
It first detected the Personaware menu at the 202nd one-second sample. After
capturing that frame, it enqueued an Enter make/break pair and compared each
subsequent published frame to the saved menu. The first eight one-second
samples differed by 1,515 pixels; the ninth differed by 44,496 pixels. Visual
inspection confirms the captured ninth-sample frame shows the World Clock
window. This is a coarse sampling result for the complete visible window, not
an exact input-to-first-response latency: sampling and frame copying add work,
and the earlier pixel changes were not independently attributed. The test
process shut down successfully after 215.33 seconds.

This single run confirms that interactive system use, not only boot, remains
slow on this Machine-input path. It does not measure the native Window, RDP,
x86, other applications or repeatability. No production source, user media or
EXE was changed. The log, menu frame and response frame SHA-256 respectively:
`225FC0AC78C4AFA77086F380C83C03B1CDE825A87063E2CCA23320A4CE5FFA4B`,
`85A35680232CA0AD623DB7DD18D23A79396931ACB133791581D40C37048AEF41`,
`A06BECF44F5986D642FD972A06FCE2A150DF121154BCAB3E59EE43C564E80CE8`.

### System response host samples

The same private-media x64 Machine probe was repeated with a task-only native
instruction-pointer sampler activated after `INPUT-QUEUED`, then allowed to
exit normally after the large visible frame change. The menu appeared at
sample 200; the 5,000-pixel change threshold was reached at input sample 9.
That threshold alone does not establish that the window was fully drawn. The
worker had 1,441 sampled instruction pointers across 176 symbol labels.
The largest labelled buckets were CPU execution refresh 117 (8.1%), Machine
run 65 (4.5%), elapsed-time publication 59 (4.1%), CPU physical read 54
(3.7%), and instruction boundary 52 (3.6%); the ten largest totalled 35.1%.

These are nearest-symbol leaf samples, not inclusive time or a proof of the
full CPU/board cost. Inlining, debugger sampling, fixed intervals, attribution
and thread suspension affect the data and may perturb response. The sample
covers one x64 World Clock opening, not the native Window, RDP, other commands
or a full boot profile. It supports investigating the entire execution and
device path rather than treating input delivery or rendering as the sole
cause; it does not establish that any proposed optimization is safe or enough.
No production, owner media or EXE changed. Sampler and guest log SHA-256:
`AE1CE1463372F396249141F7B5CB762050B16FA4DE237B69EF8469BFC2AA348B`,
`5D66009E7AEA45EB42910AE8FC051E86DCFE3DDE817332967CFE8277C6209884`.

### Repeat-prefix computation

The repeat timing path separately determined the decoded prefix count for
continuation matching and again for recording the active instruction. A
single local count can serve both without persistent state or a second decoder;
V30 still uses its distinct NEC prefix identity. A first, narrower scratch
change only consolidated two record updates, but Release disassembly showed
GCC had already merged those calls in the 486 path, so it was not adopted.

The cross-stage scratch candidate reached the same 10,000,000-instruction,
79,964,294-tick, `0003dbc5` PC checkpoint in each variant. In ordered
baseline/candidate runs, x64 took 18,519/17,930 ms and x86 21,611/21,876 ms.
Each is one pair, not a reliable performance comparison or full-state proof.
Release disassembly shows fewer static prefix-count call sites in the repeat
record function; the change eliminates repeated classification on the active
486 continuation path. Product startup and response qualification is pending.
The existing 40 focused CPU, repeat, V30, fault and preview tests passed on
each host width.
The existing repeat fixture covers zero-count, first and continuation phases,
operand/address prefixes, interrupts and faults. The local count is computed
only when either the previous repeat is active or the current instruction
will continue; the only intervening matching operation observes the existing
instruction bytes and does not mutate them. V30 never uses the Intel count.
Both 712-test unit suites passed, as did the three PC profile receiver groups
(3/3, 5/5 and 6/6 per width). Default integrations passed 47/48 per width;
the native MIDI output creation assertion was the sole failure in each run.
Both MyRaidenII selected-set startup and Console probes passed per width;
this is not an arcade-playability claim. Fourteen PE widths were checked;
the deployed MyPC110 and MyNES files matched the cached build outputs.

The first post-change x64 system probe detected the menu at sample 200 and
the generic frame-change threshold at input sample 9, completing in 213.12
seconds. Visual inspection shows that this response frame is still partially
drawn, unlike the earlier baseline frame. The threshold cannot therefore be
used to compare complete response latency. A further x64 run required a pixel
in the right-hand World Clock pane as well as the change threshold. The menu
appeared at sample 202 and the completed frame at input sample 9; total time
was 215.33 seconds. The completed frame hash exactly matched the baseline
frame, and visual inspection confirmed both panes. One-second sampling, a
single baseline and one candidate cannot establish a speedup or whole-state
equivalence. Neither run measures native Window or RDP responsiveness.
Focused, full, external and partial-response log SHA-256 respectively:
`04E6F64878133E6D7222C39836DAF338968C78E7A416E61ABF715A15B0DD382C`,
`134ABD76DED0BF7FFEDD644EFDC0D404BB6F8E77E30D00D44771BE9939FE1BD5`,
`967DA35EAB5E644EC43CE85C27514053F9C5755F2BF774CC74AE742ACA119573`,
`86444356B677A2B9E511AD0CE36783EE535C5281FBE74B623E27441B826C565A`.
The completed-window log and frame SHA-256 respectively:
`7E82EA87F675E0C733224EA68C08B3AEB144FAC310B8A9577755ED07910F05A1`,
`A06BECF44F5986D642FD972A06FCE2A150DF121154BCAB3E59EE43C564E80CE8`.
The CPU source change is `b1b4f59b2`, and its MyRaidenII artifacts are
`5252aaf89`. The [artifact inventory](t582-s39-repeat-prefix-artifacts.json)
records the fourteen PE widths and hashes.

### Machine boundary request guard upper bound

A task-only candidate sampled the CPU's existing pause, stop and reset flags
once at the Machine boundary and called the existing consuming operations only
for set flags. It did not change the external atomic stop check or its ordering.
This is not a qualified production design: the temporary getter directly
inspects CPU-private state, and asynchronous mutation, reset/hold and callback
boundaries would need proof before any public contract change.

At the same private 10-million-instruction startup checkpoint, ordered
baseline/candidate times were 17,778/17,656 ms x64 and 22,262/22,142 ms x86;
each reported 79,964,294 ticks and PC `0003dbc5`. These single pairs are well
within ordinary run variation and are not whole-state or response evidence.
They do not justify a new CPU/Machine interface or the required concurrency
qualification. No production or EXE changed. The four log SHA-256s, x64 then
x86, baseline then candidate:
`8A6813872E2A80208A2099D95A6329AC9F8F3F3F6E9E5BC8A586F5F412BF9111`,
`7BE5899AC2DB6D8ADE3EFF4382383F7954FFBC3B9B7D7CA6C674D0F7A6EFC5A5`,
`BF8216EC7C57B3A8DF77AD94733F902A9B5E617B0CC933724BE8A61F64083C25`,
`08BCF3598DD2F5315F6B71E84D17A1B6A577F24B26EEF2F1FE7F7B3B0EFAA370`.

### Shared 486 timing prefix handoff

The selected 486 timing owner already computes the bounded opcode-prefix
count before calling its nominal, L2, memory and decode terms. Those terms
independently computed the same count again. A task-only candidate passed the
original local value directly; the instruction record and formulas were not
changed. The sole production aggregate has no intervening opcode-byte write;
repeat bookkeeping changes timing facts but not the decoded prefix bytes.

At the private 10-million-instruction startup checkpoint, baseline/candidate
times were 18,282/17,766 ms x64 and 22,683/22,030 ms x86. In reverse order,
candidate/baseline times were 17,408/18,276 ms x64 and 22,687/22,901 ms x86.
Every run reported 79,964,294 ticks and PC `0003dbc5`. These two ordered pairs
suggest reduced early-startup work but do not prove full-state equivalence,
complete startup speed, interactive response or native Window/RDP behavior.
The production candidate and its existing-test adaptations require full
qualification; no acceptance is implied by this probe. They were subsequently
reverted after the complete-system comparison below.

Forward log SHA-256, x64 baseline/candidate then x86 baseline/candidate:
`DBF830DC14BCD2B80943E43ED3023A28299DE7F866DBF35AF29861EE7B54FEB1`,
`254942BA403FBFA91AD44D11BB8519F1247A9DA93E99BDBA77B26480157A91DA`,
`233590F9F1D24B18BCC9B487C7FF2411614A313FD9131465FA8684AAC886349F`,
`64E590E050534C845353FDC527C88310DDB8597CE6F4C9BE694B0C3DC5607CFE`.
Reverse log SHA-256, x64 candidate/baseline then x86 candidate/baseline:
`145FFF857F3B412E6391BC84C9631EC3E0808B55954D1D8D37B0C01771F3337F`,
`7AB1302561AE4EE8C521D32C7AE6D2B3BFBCC5D99DF9E1563B553780D09C387D`,
`3FEFA0E6202ABC82B403DC37EBCF53DE2B2E1FA48F7497A0992C539C2994F3DA`,
`28953D0F2052B47631214475F6CDD9383E5FBCBBE0445B6A6BD68D9D03374723`.

The existing 40 focused CPU/preview tests and all 712 unit tests passed per
width. The x64 default integration run passed 47/48 with native MIDI output
creation failing. The x86 initial run passed 46/48: the same MIDI test failed
and MyNES native-window's nonblack-pixel assertion failed; that window test
passed on an isolated retry, with its initial failure cause undetermined.
All three PC profile receiver groups passed 3/3, 5/5 and 6/6 on each width;
MyRaidenII selected-set startup and Console probes passed on each width.
Fourteen PE widths were checked; MyPC110 and MyNES assets matched their builds.

The first x64 complete-window probe detected the menu at sample 230 and the
World Clock frame at input sample 10, compared with the earlier baseline's
samples 202 and 9. A subsequent adjacent, reverse-order candidate/baseline
pair reported menu samples 240/222, complete-response samples 11/10, and
total times 255.66/236.33 seconds. Guest quanta near the menu differed and
host scheduling varies; these pairs do not prove causation or complete-state
equivalence. Nonetheless, both orderings show an end-to-end slowdown risk,
not a demonstrated improvement. The candidate and its test-interface edits
were reverted; all PC and MyRaidenII assets were restored byte-for-byte to
the previously qualified `ab8095d87` baseline. The user requested all-profile
similar-path review, without PC110 or App special cases. V30's distinct NEC
prefix meaning must remain intact. No native Window/RDP timing was performed.

Focused, full, x86 profile continuation, isolated MyNES-window retry and
MyRaidenII probe log SHA-256 respectively:
`032DF06C709BF6B2DA01A2627930BCF1D95C0B2EA5879D954B7FD0847EA33F53`,
`4C4E907ABA4D4C1E88A67F276B4B7C20521BBB527D21D26CFCD2DDF9A7981200`,
`830001F6B8B1DD6A19055D49DDD0D369C973E0D597585D047D3A2DD2522FDBE7`,
`72EB43460AE00E8F457B4979B8DBCDE92DC6103A482699858243BF087A27D419`,
`967DA35EAB5E644EC43CE85C27514053F9C5755F2BF774CC74AE742ACA119573`.
Initial candidate and reverse candidate/baseline system-log SHA-256:
`A4D8750E1D4C5AE7CC773FF41540E5F0FCA0A7082C15A2A8A88476AFA83E10A2`,
`F8A18E578C38B0E2BB4E5663CB8396F710BBD46F04D4DD7154A04F7DF53225ED`,
`7DD3837368A3FEBFA7649C6408DA3433E6AA4418A885FB515FD22C6CA76A335D`.

### Cross-profile timing prefix handoff

Following the owner's request, the Intel selector now computes a bounded
prefix position once and passes it through its existing candidate evaluators,
string/repeat and LOCK terms and the 486 component terms. V30 retains its
NEC-specific native/emulation prefix representation. Formula-input
classification remains separate because its older-profile byte rules differ;
it is not safe to replace that logic mechanically. There is no App-specific
branch, persistent state, public interface or second executor. The existing
test assertions were compared after removing only the added count arguments
and whitespace; they were unchanged.

A scratch NOP/JMP loop ran 2,000,000 instructions on 8086, 8088, 80186,
80286, 80386, 80486SX and V30, on both host widths. Baseline and candidate
reported matching per-profile ticks, final EIP and no fault/stop. That loop
does not cover all opcodes, REP, faults, interrupts or protected mode and is
not a substitute for the full component tests. The existing 40 focused tests
and the separate prefix-attributes test passed per width. The initial strict
build rejected an unused private parameter; removing the parameter rather
than suppressing the warning resolved the build.

In adjacent private-media Machine probes, x64 baseline/candidate reached the
menu at one-second samples 214/207 and a two-pane World Clock marker at input
sample 10/10, with total times 228.68/221.50 seconds. On x86 the respective
menu samples were 272/272, input samples 13/12 and total times 289.23/288.40
seconds. Captured frames show the panes but still contain differing areas of
background drawing, so they do not prove complete visual or machine-state
equivalence. Single ordered pairs, coarse sampling and host variation do not
prove a stable speedup. Neither native Window nor RDP was measured; the system
remains far too slow. Full receiver qualification and release artifact checks are recorded below.

The x64 then x86 system log SHA-256s, baseline/candidate respectively:
`AE1BB33BE79A534BA81F96FED7A5382C275B7713E74F36E7AA329B214A1C94D9`,
`656DBD053121C68971FF2D7B4807B5E12675ED3CCA2153AC74E050611D685E97`,
`214F88D519E91CA76ECADC5FC4C54FAD44ABFAE6D66FCF63EE548DE83E239519`,
`35D93469337D203B2231E4B3D0855CC326FDF50CA693734B54CAB711D598AE3F`.
The seven-profile loop log SHA-256s in the same order:
`6470CE5CB5CFA32425A7A74125490AC8A7C2518A9961920C2ADBD630186BBF8B`,
`18E97B8F58F79C34F92A2E671437F98A03FF828F71FBA8AB04F27BD1F0CFDF3A`,
`27C2FF51E1E1EA81171F70FE27533B67D7B4F68FAE8078B497F184F77C5D3244`,
`2C6F6C8FD7AC115336B21372AE2EDF5555131506F6DF950649B728741668D103`.
Log SHA-256, x64 baseline/candidate then x86 baseline/candidate:
`F7A73DE287ABC6CAF062D284CEED8B20FB568D24C27C78480791B9B640F6F575`,
`744BF59F87A9C7BAA9A78DDF07145D20B779E280423F0CFD5DCB68B74697C329`,
`15ADF227D3B88190E601867F23BE10DD2F7AC1CE4438E12847F7564FA56D4385`,
`75D000C3F6D8AC948B82ED26D74D04CCD040F2F142BFE6CDC769383DF804765E`.


The cached x64 and x86 runs each passed 712 unit tests and 48 default
integration tests, including native MIDI; the 5160, 5170 and DeskPro receiver
groups also passed 3, 5 and 6 tests respectively on each width. The
MyRaidenII selected-set startup and product Console probes passed on both
widths; this does not establish game playability. Both X86 Core manifests,
documentation governance and diff whitespace checks passed. Fourteen release
PE widths were verified. The MyNES executables were unchanged by the rebuild.

The final x64 cached-build private-media PC110 probe observed its menu at
sample 194 and the target frame change at input sample 8, with 205.97 seconds
total. The observed change is not proof that all drawing has completed. This
is one run, not a controlled benchmark, and the real Window/RDP experience
was not measured. System startup and interaction remain unacceptably slow.

Full receiver, MyRaidenII, and final PC110 probe log SHA-256 respectively:
`6B47D8C25D7B0B274FE21A89B7E0277F8A393ADAF6491EB09CB3B1734AB4EB42`,
`967DA35EAB5E644EC43CE85C27514053F9C5755F2BF774CC74AE742ACA119573`,
`EF331A46F1533225F37A7F939B77E6C5FB0399FA437656447203C87EBD8363F7`.


### Current response-path re-sample

After the cross-profile prefix handoff at `7b8e39cae`, the private-media
x64 Machine probe reached the Personaware menu at sample 202. It enqueued
Enter and reached the existing frame-change threshold at input sample 9.
This is not proof of complete redraw or a native Window/RDP measurement.
The task-only sampler suspended threads at fixed intervals only after input;
it exited normally and reported 1,383 samples on the worker thread. Largest
leaf-symbol buckets were CPU execution refresh 96 (6.9%), elapsed-time
publication 53 (3.8%), Machine run 46 (3.3%), clock-domain advance 46
(3.3%), PIC selection 40 (2.9%), instruction boundary 38 (2.7%), physical
read and keyboard collection 35 each (2.5%). Inlined work is attributed
to its enclosing symbol; the numbers are not inclusive costs and fixed
intervals and thread suspension perturb execution. This is one x64 response
and not a controlled performance comparison.

The result independently confirms that response work is distributed across
CPU, scheduler, device and bus owners; it does not justify bypassing any of
them. The existing S11 and S26 experiments already tested narrower clock and
PIC shortcuts with inconsistent effects. The next investigation must
measure duplicate work across the CPU/board scheduling boundary under real
response, and establish mutation, event-order and input visibility invariants
before changing it. No production code or artifact changed in this sample.

Sampler and guest-log SHA-256 respectively:
`F6E929289309138336DBC590DBADF8ED37F051322663764B7E13D911EDBC0D5F`,
`1824025AB1FE5493B91740CBD6948C21009B1C9FA436FC09210E575E9D5DDB4F`.


### Shared PIC idle-selection reduction

The existing `x86_pic_select` scans all eight priority positions even if its
pure unmasked pending mask is zero. The ready-controller case now evaluates
the mask once and returns its existing INVALID_STATE without touching the
request output when empty; nonempty selection uses the same mask and the
existing delay, priority and cascade logic. Argument and initialization
checks remain first. This is a common PIC change, not a PC110 or App branch;
it introduces no new interface, persistent state or guest-time shortcut.

A task-only current-source comparison tested two million pseudorandom PIC
states on each host width, comparing status and every output field with the
original selector. Existing PIC and board interrupt tests also passed 14/14
on each width. This differential does not enumerate all reachable temporal
sequences; the existing lifecycle, level and delay tests cover actual paths.

An instrumented private-media x64 Machine run recorded 226,775,492 ready-PIC
selections before the input marker, of which 226,738,450 had no unmasked
request. After the marker it recorded 11,261,815 selections, of which
11,259,819 were empty. The counters add work, and the stage boundary follows
the queued input, not the guest's acceptance; the result establishes frequency
for this one run, not exclusive host cost. The frame-change criterion does
not prove complete redraw or native Window/RDP responsiveness.

A same-process 4-million-selection microfixture mostly containing idle PICs
reported original/candidate/candidate/original 91/27/25/96 ms x64 and
103/26/29/106 ms x86, with equal checksums. This isolates the selector and is
not an end-to-end speedup. At the ten-million-instruction PC110 checkpoint,
original/candidate and reverse candidate/original respectively measured
17,907/17,898 and 17,790/19,131 ms x64; x86 measured 22,882/24,015 and
21,735/23,262 ms. Each reached the same recorded 79,964,294 guest ticks and
PC `0003dbc5`; sequential host noise and endpoint-only comparison prevent
a stable whole-machine conclusion. Receiver qualification and the x64 system measurement are recorded below;
system usability remains unresolved.

Logs SHA-256 in order: fixed forward, fixed reverse, differential, micro,
real-load counter, focused tests:
`5CA3BB5B7E7F6D2AF0EFCEF3C6A9982DCF968D6D03A7AABAE03DAA4588A7336A`,
`5F9200E2C6158918458B8D3AD293A67680CF169379DE96DDC3D8543527197A11`,
`526139713F8FC13552E903907AEA58F5521BE09AFBA879391B1D992450328982`,
`269680132658928DA8F529C0FC33771F6AC620A56EBDDD3A392B37F5766368FA`,
`8743D9242C0140AC6CC5D75DA47205DAE10CDF616853F30510904D68917AF04B`,
`8201A860906765B2781C8543A53AC1D23929288D57102BC04490C136F151934D`.


The cached x64 unit run passed 711/712; `emulator.verifier-negative`
timed out at 66.66 seconds during parallel execution and passed separately
in 22.97 seconds. The x86 unit run passed 712/712. No timeout budget was
changed. On default integration, x64 passed 47/48 and native MIDI output
creation failed; x86 passed 48/48 including MIDI. The owner has already
accepted continuing past unavailable native MIDI. Each width passed the
5160, 5170 and DeskPro receiver groups (3/3, 5/5 and 6/6), and the MyRaidenII
selected-set startup and Console probes. These results do not validate native
desktop interaction or game playability. The X86 NXVM source manifest,
documentation governance and diff whitespace checks passed.

A final non-instrumented x64 private-media Machine run reached the menu at
sample 178 and the input frame-change threshold at sample 8 (190.12 seconds
total). The preceding handoff measurement was sample 194 and input sample 8;
these are single sequential samples susceptible to host drift, and the pixel
threshold does not prove complete redraw. They do not establish a stable
acceleration, and do not resolve system usability.

Initial x64 unit log, isolated retry, resumed receiver, external receiver,
and final x64 system log SHA-256 respectively:
`69B7B8FB8768F51423D839D91346CD70CC8DDC52DC563EB2EB2DADB790B93640`,
`B3226C295F1743286916BE66A4FE6F1DC6DE1AE3275A7EAEE56383ED367DF83D`,
`D99B170E94B6DE483899E921303021A6E9B3D60886C08E87A0A45EA76370DC79`,
`967DA35EAB5E644EC43CE85C27514053F9C5755F2BF774CC74AE742ACA119573`,
`97AF06232C407F5CD0185B612E2A6181AA3FC0A08BB2D3A86AD73EF6EAC2F235`.


The corresponding final x86 private-media Machine run reached menu sample
254 and the input frame-change threshold at sample 13, with 271.77 seconds
total. The previous handoff candidate recorded menu 272 and input 12; these
are single sequential runs with host and sampling variation and do not prove
complete visual equivalence or a stable performance gain. The x86 log
SHA-256 is `76A4806200F11806117DEC1FC52443A9F884B00F65F2A65BBD35F0167D5633DE`.


The [artifact inventory](t582-s39-pic-select-empty-artifacts.json) records all 14 PE
widths and SHA-256 identities against the committed Shared PIC source.
MyPC110 and MyNES deployed binaries matched their respective cached build
outputs on both widths; the PC profile receiver tests checked their selected
deployments and MyRaidenII selected-set probes passed. MyNES binaries were
unchanged and do not require a binary commit.


### PIC empty deadline and single owner

The previous change made empty interrupt selection constant-work. The adjacent
PIC deadline query had the same empty pending-mask scan and a private helper
with exactly one caller, which had already validated both arguments and
ignored the helper status. The public query now owns that loop directly; an
empty unmasked-pending mask returns the existing INVALID_STATE before scanning,
without writing the caller's output. Nonempty masks retain the same earliest
nonzero delay calculation. Production diff is +10/-20 lines. No persistent
state, interface, model-specific route or changed guest time was added.

Task-only differential programs checked two million pseudorandom state
combinations per host width for both zero/one and varied 32-bit delays;
status and sentinel output matched. This does not prove temporal ordering.
The existing PIC and board interrupt tests passed 14/14 per width.
Fixed-work baseline/candidate and reverse candidate/baseline measurements
were 19,858/17,967 and 19,394/20,646 ms x64; x86 was 22,637/22,680 and
26,370/26,113 ms. All ended at 10,000,000 instructions, 79,964,294 ticks and
PC `0003dbc5`. These sequential measurements are subject to host drift and
do not prove whole-guest equivalence or a stable end-to-end gain. The change
is admitted for removing a redundant owner and the empty eight-slot scan,
not as completion of the system usability objective.

Differential output, two differential source files, forward/reverse probe,
and focused tests SHA-256 respectively:
`1FBC85441326AAC4A9E43517226771424FB9E3A412092368A622E17EA243A1E0`,
`1D5B4BD7A987AEBC6CBCF3BF9C91DCC3CB9AF59A52F776089A7540ED863706DA`,
`277C5341E1D88EDF49C808E8B77BE9BCB543670FE80CB9EC99CDE9E7374B712A`,
`285062061DC8EE28C191C9827B3641B343F239E082ABF19BDB528910A6AB46B0`,
`B2EADB6327C88DA341D7D9D63FDAB5ECEFC1D037F30BFAED979D9FD07164C6D1`,
`61C199F9855883749F0A2F3CF49001DA810B30CAB0DB64A6E89183152E3F286F`.

Full Shared receiver and final system qualification are recorded below.


The cached x64 receiver passed 712/712 unit and 48/48 default integration
tests. x86 initially passed 710/712: `lib.types-layout-selftest` timed out
at 179.86 seconds and `emulator.verifier-negative` at 60.76 seconds. Both
passed in a subsequent serial run in 138.75 and 49.60 seconds, respectively;
no timeout was changed. Fixture timestamps advanced during the retry, but
the reason for the slow host execution was not established. The remaining
x86 default integration passed 48/48. The 5160, 5170 and DeskPro profile
receivers passed 3/3, 5/5 and 6/6 on each width. MyRaidenII selected-set
startup and Console probes passed on both widths.

An x64 private-media final-build run reached menu sample 216 and input
frame-change sample 8 in 228.26 seconds; the preceding accepted build had
recorded 178/8. Because that single pair showed a possible regression, a
reverse-order repeat used the preserved candidate and baseline binaries with
the same private inputs: candidate reached 183/11 in 197.96 seconds;
baseline reached 209/9 in 222.57 seconds. The sequential pair directions
reverse. Host variation, one-second sampling and a partial-draw frame marker
prevent a stable speed or full-state conclusion. No native Window or RDP
measurement is implied; PC110 remains too slow for the task objective.

Original full gate, isolated retry, x86 resumed receiver, external receiver,
final candidate, reverse candidate, reverse baseline and reverse summary
logs SHA-256 respectively:
`DC632FAD506C12E9AF0601AE1E14E41B0A1C4DF4CE656CDDD96CA43F3F351452`,
`3D13514292F3AD2F635A7A2C5AE9D6399368A87AB40E44A9DF39B834EE03F37A`,
`73C9E580D85B432E107219AA146AA715EECDCB6E2BBEC2F02FBC589A32E01B80`,
`967DA35EAB5E644EC43CE85C27514053F9C5755F2BF774CC74AE742ACA119573`,
`4047C00A2CE1021EAAEB7B929DE3F9A6F2127EE9A7C5AA3101849944EB3CFD1B`,
`8C87A6FCF5A570E5F51E39A46C19AE97BE4D0B80B2DA08EBD58D2BF6A2F5067A`,
`53548E6DCF706270721384C037C40DD3306CBD284380F54D5F5FE09BC0F22C87`,
`0207A05E8A161A59F92C10145E9677CB726FCAA59F46E0CE83152EA46C3125FB`.


The [artifact inventory](t582-s39-pic-deadline-consolidated-artifacts.json)
records all 14 PE widths and hashes against the committed Shared source.
MyPC110 and MyNES deployment bytes matched the respective cached build outputs
on both widths; PC profile receiver checks covered selected deployments.
MyNES binaries did not change.

### Timing formula prefix scan recheck

A task-only candidate reused the prefix count already computed by the Intel
timing selector to skip its later formula-input prefix scan when the count was
zero. It retained the original scan for the `64`–`67` byte range, where the
formula collector and older Intel profiles intentionally classify prefixes
differently. NEC timing follows a separate existing path. No production code
or public contract changed, and callback mutation equivalence was not proven.

At the private-media 10,000,000-instruction startup checkpoint, sequential
baseline/candidate elapsed times were 19,524/20,553 ms on x64 and
20,287/23,627 ms on x86. Each reached 79,964,294 guest ticks and PC
`0003dbc5`. These single pairs are sensitive to host load and do not establish
complete state equivalence or whole-system performance. Neither supports
adding the candidate's condition and fallback, so it was not retained. This
does not resolve PC110 startup or interaction performance.

Comparison log SHA-256:
`FBFB1AACC6AE55B73B048D0262E0E599658921263DEE6A721E79A8D0E2626366`.

### Post-classifier startup sample and DMA observation

After the common prefix-classifier change, a hidden task-only x64 probe sampled
the existing private-media startup worker from 5 to 60 seconds. Its CMOS hash
was `608DF3ABC3CDD9BB3E35015D2D7AA9D8756567E080643B01E593FF99D1616034`.
The sampler stopped its own guest at 65 seconds; this was not a complete boot.
There were 4,282 early and 4,949 late worker samples, with no reported native
sampling errors. Among late leaf samples were CPU refresh 273, Machine run 218,
KBC input observation 159, memory-route resolution 143, CPU 486 memory timing
134, clock-domain advancement 123, board deadline observation 119, DMA
transaction advancement 104, DMA signal capture 98 and DMA pending request
observation 93. These are individual leaf addresses, not inclusive costs; they
cannot be added as isolated costs or used to establish an end-to-end speedup.

The DMA path repeatedly captures controller signals, but its advancement can
grant or release a channel, execute a bus transfer and change cascade request
state before the later cascade observation. A prior idle-skip experiment had
not proved cascade coherence. Reusing a pre-advance snapshot without proving
the mutation cases would be incorrect; this sample alone does not justify a
new cache or production change. No x86 or interactive Window run was made.
The guest and sampling are task-owned; the host output reports termination,
not a successful guest exit. Sampler log SHA-256:
`B80EB740B64BD7453C369E2C5270E2D7374D215E4FDB566B03945EEBF3E72378`.

### DMA signal reuse scratch candidate

A task-only board DMA candidate reused the first controller's just-read
signals for cascade update only when the advancement path performed no
transfer or grant. Paths that could mutate controllers or invoke a transfer
re-read signals as before. This was an exploration, not a qualified change:
complete callback, cascade, controller topology and error equivalence was not
established, and the candidate added branching and signal passing.

At the private-media 10,000,000-instruction checkpoint, sequential
baseline/candidate times were 19,445/19,556 ms on x64 and 22,855/23,189 ms
on x86. Each reported 79,964,294 guest ticks and PC `0003dbc5`. The single
pairs are sensitive to host variation and do not prove full-state equivalence
or whole-system performance; neither supports retaining the extra complexity.
No production source, interface, test or product artifact was changed.
Comparison log SHA-256:
`4FC40D5D7B21F0EAFE3990FC8E97E995A14B9DB98AEF2F08B643D5D6BEA7B25B`.

### Empty FDC command-phase advance candidate

A task-only FDC candidate still updated elapsed guest time, then returned in
the command phase only when no head unload, seek, reset, DMA byte, non-DMA
byte or service deadline was pending. Other phases and pending events followed
the existing route. This was an exploratory guard, not a fully qualified
proof of callback or guest-device equivalence.

At the private-media 10,000,000-instruction checkpoint, baseline/candidate
elapsed times were 21,931/20,296 ms on x64 and 24,738/26,716 ms on x86. The
x86 reverse-order candidate/baseline pair was 25,947/24,368 ms. Every run
reported 79,964,294 guest ticks and PC `0003dbc5`. These short ordered samples
are sensitive to host variation and do not prove complete state equivalence or
full-system performance; nevertheless, the extra conditions and two adverse
x86 comparisons do not justify adoption. No production source, test, public
interface or artifact changed. Forward and reverse logs SHA-256:
`8D3FF6846C4977FF009F42EC00FBE1E479381689AFF8552B4A41C9884B26A704`,
`C03DB5EC095151D6DC4A0676EB7ACE6A08F7B5B52489D651B382123F8E35E685`.

### Pending retirement event subtotal reuse

A task-only Machine candidate retained the event subtotal from the CPU
timing result already read after refresh and reused it in the subsequent
pending-retirement iteration. A new Machine run with an existing pending
retirement captured the current subtotal as before. The existing second loop,
including stop, reset, bus hold, waiting and budget checks, was retained; this
is distinct from the rejected immediate-retirement ablation. The candidate
has not proven timing immutability across all attachment callbacks, debug,
fault and partial-budget paths and is not a production implementation.

At the private-media 10,000,000-instruction checkpoint, sequential
baseline/candidate elapsed times were 19,568/19,097 ms on x64 and
23,844/24,628 ms on x86. The x86 reverse-order candidate/baseline pair was
23,768/24,233 ms. All reported 79,964,294 guest ticks and PC `0003dbc5`.
The x86 direction changed with order, host variability is material, and the
checkpoint does not prove full-state equivalence or complete boot speed.
These samples do not justify the additional local state or qualification
burden. No production source, test, public contract or artifact changed.
Forward and reverse log SHA-256:
`AB776EFF17CE079FBC99BA6D8729D0A785426154B3656FB23CEE2A625C49ECD8`,
`4502716DD19A04239F0122BEB3B900164E161D79C0A9CEE30A929F6BE86157C6`.

### Private CPU segment-cache layout

The CPU owns ten private segment-cache records in each full CPU snapshot.
Reordering their existing fields and storing the bounded segment kind in a
byte reduces a record from 24 to 20 bytes and the CPU snapshot from 380 to
340 bytes on both tested host widths. The enum constants and all descriptor,
selector, control and general-register fields remain; a compile-time bound
guards the encoded kind. Debug snapshots are copied by field. The record is
private to X86 Core CPU, not a public ABI or a serialized guest structure.
This reduces fixed state and bytes copied at instruction boundaries; it does
not change asymptotic complexity or skip timing, interrupts, faults or guest
instructions. All profiles use the same layout.

At the configured private-media 10,000,000-instruction checkpoint, sequential
baseline/candidate samples were 18,671/18,886 ms on x64 and 22,194/22,969 ms
on x86; reverse candidate/baseline samples were 18,642/19,201 ms and
22,691/23,412 ms. Each reported 79,964,294 ticks and PC `0003dbc5`. The
mixed results and host variation do not establish a speedup or full-state
equivalence. In a separate candidate-then-baseline x64 run with private media
and configured CMOS, Personaware menu markers occurred at samples 207 and 211,
and the subsequent frame-change markers at samples 10 and 9. The runs exited
in 221.63 and 224.79 seconds. Menu frame hashes matched, but the two response
frame hashes differed; the marker is not proof of an identical completed
frame or machine state. These samples do not demonstrate usable performance
or native Window/RDP behavior.

Fixed-work forward, reverse, candidate system and baseline system log SHA-256:
`D05516E84BD1B131CBA341327F9087CD0A99E5D67C1644CB5BD6E2C2D030187F`,
`99E596AD4CF5DA529E927910FDBA6FBBEE87411D6A906F7F82CBDCDC5BA3DE4C`,
`CC737DB851152528A94E0FA2423CB85FF0707F6B62C05002431BFEAD70566E20`,
`983488C8B97E114D8B74BACAE5649FA4AD59AE5DF9D3209E80CB87736C063AA4`.

The production layout is Shared commit `be0f1e499`. Each width passed 41
focused CPU/preview tests, 712/712 full unit tests and 48/48 default
integration tests, including native MIDI. My5160, My5170 and DeskPro selected
profile integrations passed 3/3, 5/5 and 6/6 on each width. MyRaidenII's
selected-set startup and Console probes passed on each width; they do not
establish arcade gameplay. Source and test X86 Core manifests and whitespace
checks passed. The release inventory below records all fourteen PE widths and
hashes. MyPC110 and MyNES deployed bytes matched their final cached build
outputs on both widths; MyNES artifacts were unchanged.

The final production x86 private-media Machine probe reached the menu at
sample 259 and the input frame-change marker at sample 13, exiting in 276.00
seconds. Its captured response includes both clock panes; it does not prove
that every background element finished drawing. The earlier x86 samples were
240/11 and 254/13; host variation and one run per version prevent a causal
comparison. No native Window/RDP performance test was performed. The PC110
usability objective remains open.

Focused, full, external and final x86 system log SHA-256 respectively:
`9CF9CDD76AA700FA22AF70EDDB1E09A59A745AE0C0B81D4D62CCBD047B42DF27`,
`4E8B03EB6CA6F500B0FA4D1F1484B1AD450E3F2E64A8C052DA23D73F31CFE43A`,
`967DA35EAB5E644EC43CE85C27514053F9C5755F2BF774CC74AE742ACA119573`,
`A9ABEB982CCE95055301F64ACB65E39BEFA18ED2816F76233B1CEA6DAE5C1A79`.
The [artifact inventory](t582-s39-segment-layout-artifacts.json) records PE
widths and identities against the committed Shared source.

### Lexical preview task-checkpoint copy

The CPU context's existing task-switch checkpoint was copied into every
lexical preview even though preview executes no instruction handler or task
switch; `ExecBoundary` resets the copied task-switch flag before fetch. Moving
the checkpoint adjacent to the already excluded cache/TLB region allows the
existing context-copy boundary to omit it without a new path or state. On the
tested x64 build the initial context-copy prefix decreases from 780 to 436
bytes; on x86 it decreases from 724 to 384. The full private context decreases
by eight bytes on x64 and is unchanged on x86. This is a fixed-size and copy
reduction, not an asymptotic improvement or a different timing model.

Scratch baseline/candidate comparisons over 60 million retired instructions
reported 98,513/90,584 ms on x64 and 119,320/119,212 ms on x86; all reached
365,365,343 ticks and PC `00038887`. Last 30-million intervals were
44,151/41,408 and 55,440/56,091 ms respectively. Single ordered pairs and host
variation do not prove a stable gain or complete-state equivalence. Existing
Machine lexical-preview tests, built with the omitted checkpoint explicitly
filled with `A5`, passed on both widths; those tests cover the existing profile
matrix but are not proof of every possible callback or error path.

A private-media x64 candidate run reached the menu at sample 214 and a later
input frame-change marker at sample 9, exiting in 227.07 seconds. The immediately
preceding segment-layout baseline had reached menu sample 211 and input sample
9 in 224.79 seconds. These coarse, sequential samples do not show an
end-to-end speedup, complete state equivalence or native Window/RDP usability.
The PC110 performance goal remains open.

The initially started full test run was stopped before acceptance when it was
found to have built product targets but not relinked all test executables.
Its partial results are not counted. The replacement run first rebuilds both
complete test trees, then executes the full qualification.

Fixed-work x64, x86, poisoned preview, and candidate system log SHA-256:
`4500C707127065187E4A52593229A479B87F2FF51DF8D3CAC442487D51E2C7F9`,
`3D1AEB2DB520FB835F91A2E1876236C2F9FF017285F0B399C0EDBE76CC78FA96`,
`539967D28AD165710FB3BE6ACE3530FA51DE73AC21BD02C0EDD56233971E4A05`,
`D69589983B91892350F671ED1B0828C79784EF4036C832C7BFFC11B0EDDC4B2D`.

The production change is Shared commit `b57abda98`. After rebuilding the
test executables, each width passed 712/712 unit and 48/48 default integration
tests, including native MIDI; the three selected PC profile groups passed
3/3, 5/5 and 6/6 on each width. MyRaidenII selected-set startup and Console
probes passed on both widths, without establishing gameplay. X86 Core source
and test manifests and whitespace checks passed. The inventory below records
fourteen PE widths and hashes; MyPC110 and MyNES deployments matched the cached
outputs on both widths, and MyNES binaries were unchanged.

The final production x86 private-media Machine probe reached menu sample 256
and its frame-change threshold at input sample 11; it exited in 270.45 seconds.
Visual inspection shows a partially drawn World Clock, so that threshold does
not prove a completed application redraw. The prior sample was 259/13, but
one run each and host variability cannot establish a speedup. No native
Window/RDP performance run was performed. The usability goal remains open.

Qualified full, external and final x86 system log SHA-256 respectively:
`96712568735903C441A2944DFCF271B3888D5F20A637F9DAD22318CCDFF8492B`,
`967DA35EAB5E644EC43CE85C27514053F9C5755F2BF774CC74AE742ACA119573`,
`13095CD5090661CEBFCEDE54D7A0356232E1DC6544E674665C4780C1D636A2AE`.
The [artifact inventory](t582-s39-preview-task-checkpoint-artifacts.json)
records the fourteen PE widths and hashes against the committed Shared source.


### Current prefix-dispatch reuse recheck

After the PIC and cross-profile timing changes, a task-only version of the
current instruction executor calculated `instruction_is_prefix` once for each
dispatched opcode and reused it for V30 prefix charging and the loop decision.
It retained the existing execution, fetch, fault and timing paths and added
only an instruction-local Boolean. The serialized CPU bus contract allows
signal propagation but not reentrant execution; complete all-profile, fault
and callback equivalence was not demonstrated. No production code changed.

At the 10,000,000-instruction private-media checkpoint, x64 baseline/candidate
were 20,253/19,258 ms and reverse candidate/baseline were 17,630/19,454 ms.
x86 was 21,543/22,033 and 24,503/22,053 ms respectively. All runs reached
79,964,294 guest ticks and PC `0003dbc5`. A separate x86 baseline/candidate/
candidate/baseline series launched each copy pinned to the same logical
processor, with fresh private inputs, and measured 20,920/21,656/26,731/
25,701 ms. Pinning does not control clock frequency or background load and
these are early fixed-work checkpoints, not complete guest-state or system
response comparisons. The candidate was slower than its adjacent baseline
in both pinned pairs. It is not adopted because the cross-width evidence
does not meet the owner's no-obvious-regression criterion.

GCC emitted an x86 execution-refresh body of 0x3f60 bytes for the candidate
versus 0x40f0 baseline, and a 0x108-byte stack frame versus 0x100. This
shows different code generation, not the cause of the observed difference.
No x86 or x64 full-boot comparison was warranted after this check.
Forward, reverse and pinned log SHA-256 respectively:
`29F38124E56DBD6AB71214DF6EEC918E2DB2E62DAD2BCC04FEE819571B30ECD5`,
`0205BE0B66D2FF04EB00CF6F2319638627209E7004E7ACB7677AB5B958F43391`,
`349870741CAF83F1760DF112A936B8C8A4F89BBE13D8549231CD11AF94A2747A`.


### Common prefix classifier bit groups

The existing CPU-private prefix classifier had a switch with individual
segment and REP opcode cases. Its one owner now groups the four segment
overrides by their two differing bits and the two REP bytes by their one
differing bit; the `64`–`67` group retains 80386/80486 and V30 distinctions.
No generated table, per-machine state, profile shortcut, second classifier,
fetch route or timing formula is added. Source is +7/-19 lines. The existing
prefix-attributes test gains an independent literal-list check of all 256
opcodes for the eight declared profile values (+32/-1 lines).

A temporary baseline-versus-candidate direct classifier check matched all
5,888 byte/profile combinations (profiles -2 through 20 inclusive) on each
host width. The existing 41-test CPU/V30/exception/timing focused suite passed
on each width; the expanded prefix test was then rebuilt and passed on each.
These results do not by themselves establish whole-Machine equivalence.

At the 10,000,000-instruction private-media checkpoint, x64 baseline/
candidate were 18,282/16,743 ms and reverse candidate/baseline were
20,391/20,632 ms; x86 were 25,306/24,950 and 24,758/25,324 ms. Every
run recorded 79,964,294 guest ticks and PC `0003dbc5`. These sequential
fixed-work samples are subject to host variation, not proof of stable
end-to-end speedup or equivalent complete state. Full receiving and PC110
system validation are recorded below.

Classifier equivalence, forward/reverse probes, prior focused suite and
expanded classifier test logs SHA-256 respectively:
`6F2E24CA59DC288E7EC6578EEB1DEC2C086849EB9412B202B52817080C242124`,
`9E639D8D02EF81005177A6CFF131EA6AF0992BA8FE62B3D4DDEAD254CC736C56`,
`D47C6F6ADC0F6747DAE957F8A69A290859F02302BAE6F2D145880AAC9B756A22`,
`103BF7DE046AE178F1B44B56B82469DDE7E3BD58AB234A258031641391F1CC65`,
`E5B994D19722F537BB6C70B4E0F7AFAF519F15199D08ED2C71287F589BFA2F02`.


The cached receiver passed 712/712 unit and 48/48 default integration tests
on each host width, including native MIDI. The 5160, 5170 and DeskPro receiver
groups passed 3/3, 5/5 and 6/6 on each width; MyRaidenII selected-set startup
and Console probes passed on both. The full unit suite used four concurrent
tests instead of eight after earlier concurrent timeouts; timeouts were
not changed. X86 Core source/test manifests and whitespace checks passed.
These checks do not establish native Window/RDP interaction or arcade play.

The final x64 private-media PC110 Machine probe reached menu sample 179 and
the input frame-change threshold at sample 8, total 191.20 seconds. This
single host run and pixel marker do not prove complete redraw or a stable
speedup compared to earlier samples; system usability remains unresolved.

Full receiver, external receiver and x64 system log SHA-256 respectively:
`7925253B90FFEF97CF4B1B50C5037C5B0EE3A90F65239FCE493CBCAD5C252C51`,
`967DA35EAB5E644EC43CE85C27514053F9C5755F2BF774CC74AE742ACA119573`,
`FD209B234671080F3A5513016F40C1FC65C4398C826C7B02447C6D767434FB2E`.


The final x86 private-media Machine probe reached menu sample 240 and the
input frame-change threshold at sample 11, total 255.48 seconds. The preceding
PIC-only run recorded 254/13. These are single sequential runs with host and
frame-sampling variation; they do not prove a stable gain or complete redraw.
Native Window/RDP interaction remains unverified. The x86 guest log SHA-256
is `17ABC580C81406AFC37D321AAB906C3BFD6C87A1A02B787D26ED477562C6675B`.


The [artifact inventory](t582-s39-prefix-mask-artifacts.json) records all
14 PE widths and SHA-256 identities against the committed Shared source.
MyPC110 and MyNES deployment bytes matched their cached build outputs on
both widths; the PC profile receivers exercised their selected deployments.
MyNES binaries did not change.
