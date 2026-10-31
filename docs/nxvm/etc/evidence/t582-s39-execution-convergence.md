# T582 S39 — execution and timing convergence

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
