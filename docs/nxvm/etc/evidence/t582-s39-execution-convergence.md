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
The reviewed Shared source/test change is commit `d5415b5b9`. The NXVM
documentation-governance gate also passed. The paired artifact inventory
records that source revision and the exact Shared manifest hashes.

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
