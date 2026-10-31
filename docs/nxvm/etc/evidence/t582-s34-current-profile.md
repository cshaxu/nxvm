# T582 S34 — current PC110 profile

At Shared `c1b6f95b4`, a task-only x64 product-path executable sampled thread
instruction pointers during host seconds 5–25 and 220–240. The executable and
configured CMOS were in the same private directory, with private disk copies.
The worker recorded 3,440 early and 3,432 late samples; asynchronous thread
suspension and symbol lookup perturb execution and do not supply inclusive
function costs or cross-architecture results. Other threads had context-read
errors and are not used in the ranking. The sampled run identified a game
frame, but the diagnostic uses the earlier insufficient two-pixel frame check;
this is not a fully rendered-frame or native/RDP qualification.

The named CPU (`x86_core_cpu_*`, `Exec*`) buckets account for 1,067 early and
1,124 late samples; named Core Machine buckets 873/486; named NXVM board/chip
buckets 600/859; `msvcrt.dll` 76/85; all other labels 824/878. These are
exclusive instruction-pointer labels, not complete component attribution:
helpers, inlining, other modules and unknown labels remain in the last group.
Early `memory_route_resolve` accounted for 189 samples; late `pic_select`
accounted for 117. No single named leaf is a demonstrated explanation for the
whole order-of-magnitude product gap. Sampler / guest log SHA-256:
`7BCFAB089BB619923CED851199C748540E49CCB783B3D6BCBD0287B1DC1039AC` /
`2F9C4513AC1E1ADEE315D97F61EA2B25AA54669B798C193E69F7994AE7A18A9C`.

## Rechecked one-pass memory route candidate

The [S15 experiment](t582-s15-cpu-line-audit.md) merged raw-provider inspection
and ordinary-window matching into one pass while retaining raw, selected alias,
replacement, ordinary and fallback query priority. S34 revisited it because
current sampling still identifies memory routing, and S31 has since changed
the existing decoded-address handling. The old candidate's route was adapted
to current source without changing the new fallback decode; it is scratch-only.
It removes five net source lines and a provider traversal, but performs some
matching earlier, including on paths where a selected alias later wins. The
observational-query contract and all error, overlap and mapping semantics
would require direct qualification before any implementation.

With each executable and configured CMOS seed in the same private directory,
all sequential baseline/candidate/candidate/baseline runs reached 10,000,000
instructions, 79,964,294 ticks and PC `0003dbc5`:

| Architecture | Baseline | Candidate | Candidate | Baseline |
| --- | ---: | ---: | ---: | ---: |
| x64, ms | 20859 | 21270 | 21521 | 21088 |
| x86, ms | 24454 | 24944 | 22012 | 21908 |

Both orderings slightly favored the baseline, despite host variation. Combined
with the earlier late-workload result, this does not justify changing the
production route. It is not a proof of complete state equivalence, a precise
regression estimate, or a claim about other machine configurations. Log
SHA-256:
`722DB9EF3C58456AD134F8C01233BD9211FA15ABDA38B46AA7C07B6A6DA237A2`.

A follow-up scratch candidate computed the decoded-window range match once
per provider and reused it for a raw query when raw and decoded addresses were
equal. Other raw addresses still received their own range check, and query
ordering remained unchanged. It changed 23 lines in and 26 out compared with
the current source, but made the loop's conditions more complex and still did
matching before a possible selected alias. Configured-seed sequential
baseline/candidate/candidate/baseline samples were 22621/22294/23101/22154 ms
on x64 and 25509/26364/27407/27616 ms on x86; each reached the same fixed
instruction/tick/PC checkpoint. Both widths changed direction between pairs,
so no stable gain is established. The small net line reduction does not justify
the increased conditional complexity and potential extra work on alias paths;
this candidate was also rejected without production changes. Log SHA-256:
`8766087A1B2EA853CC106E87A484B20831DE61970FB138F45F82BD2344D76A5F`.

## PIC input-level zero check

Board PIC refresh derives each controller's input bitmask from eight existing
source counts. A configured-CMOS, task-only fixed-work counter observed
20,579,338 derivations on x64, of which 11,674,883 had all eight counts zero;
x86 observed 20,591,174 and 11,680,801. This is approximately 57% on this
startup checkpoint, not a late-game frequency or exclusive cost estimate.

A scratch candidate used a byte-safe copy into an eight-byte scalar solely to
test for all zeros before the existing loop; zero detection does not depend on
byte order. It introduced no mirrored mask or other persistent state. At the
same 10-million-instruction / 79,964,294-tick / `0003dbc5` checkpoint,
baseline/candidate/candidate/baseline timings were 19219/18878/19734/21341 ms
on x64 and 24393/25074/26774/26637 ms on x86. The x64 samples favored the
candidate with substantial baseline variation, while both x86 samples favored
the baseline. The additional branch and copy have no proven dual-width or
product benefit, so no production change or semantic qualification was made.
Counts and comparisons remain startup-only observations, not complete-state or
interactive evidence. Log SHA-256, count and comparison respectively:
`8B2BD9F1CA4CE53A5A9D5C5B7228A6EE200E643C0754DB8FBE316ECE1065B552` /
`A0DD4D27FDA5B83783FCECC3BF149A8F6C376E8E467A2803BDED5EF368B9CEEB`.

## Instruction timing-fact reset

`ExecBoundary` individually resets a contiguous group of per-instruction
timing facts. A scratch candidate replaced those assignments with one bounded
zero operation from `timing_fetch_bus_ticks` up to, but not including,
`memory_lock_active`; the preceding previous-instruction fields also remained
outside the interval. The scratch diff was 5 lines in and 26 out. It was not
qualified across exception, preview, V30 or debug paths, and would make reset
semantics depend on future structure field placement.

Configured-CMOS baseline/candidate/candidate/baseline fixed-work times were
19377/20807/19279/18549 ms on x64 and 21062/24505/21268/22447 ms on x86;
all reached 10,000,000 instructions, 79,964,294 ticks and PC `0003dbc5`.
Both x64 comparisons favored the original; x86 changed direction and displayed
substantial host variation. Although the candidate reduces source lines,
the timings do not demonstrate an improvement and its implicit layout
coupling weakens maintainability. It was rejected without modifying production.
The fixed checkpoint does not prove complete state equivalence or gameplay
performance. Log SHA-256:
`E1E5F6249C671A3387C0AE331918612AE221A74D1BE054E678A414F53EF677BB`.

## 486 access-penalty zero case

A configured-CMOS, task-only counter checked whether every input relevant to
the 486 translation/alignment/descriptor access-penalty calculation was zero.
It observed 9,293,112 zero cases in 10,003,020 calls on x64 and 9,295,787 in
10,006,312 calls on x86, approximately 93% at the startup checkpoint. The
counter is diagnostic and does not characterize late gameplay or nonzero/error
cases.

A scratch candidate returned the same zero ticks and unmodeled result when all
those inputs were zero, retaining the original validation and computation for
every other case. Its fixed-work baseline/candidate/candidate/baseline times
were 23589/21068/20400/19628 ms on x64 and 27041/24141/24834/26541 ms on x86;
each reached 10,000,000 instructions, 79,964,294 ticks and PC `0003dbc5`.
The x86 pairs favor the candidate, while x64 changes direction. This does not
establish a stable dual-width or gameplay gain or complete-state equivalence.
The prototype repeats a long list of fields ahead of the existing validation,
adding a separate fast-case condition and making maintenance of the error
contract more fragile. It was not subjected to semantic qualification or
merged. Counts and comparison log SHA-256 respectively:
`584C1E60D69FDC49FCB91CED218F07EBB8BD6831BF5359E2A204A8211C24D609` /
`05A2EC0525BCFEF628D1CF4A6D1DCAA84509263F62BDB64E33BCE76F3B748A2A`.

A second scratch candidate retained the existing validation first and then
returned early when its validated inputs implied zero access penalty. It
avoided duplicating the full validation, but introduced another set of
conditions and repeated part of the result classification. The configured
baseline/candidate/candidate/baseline times were 22780/22633/22292/19532 ms
on x64 and 25663/24639/26000/26494 ms on x86, with the same fixed checkpoint.
The x64 comparisons again changed direction and the small x86 differences do
not establish an interactive gain. This version was not semantically
qualified or merged; matching the checkpoint is not proof of equivalence.
Log SHA-256:
`8F5567BB54A9BAB9CA39800A35BE41D07F291C8C037F75D6026EEC4E7FF5F115`.

### Algebraic access-term candidate

For the three page-walk counts `a`, `b`, `c`, the existing validation bounds
the missed table reads by `2 * (a + b + c)` and updates by `b + 2 * c`.
Expanding the existing saved-transfer term gives
`9*a + 15*b + 20*c + 2*(table_misses + updates)` without changing the
validation. Both subtractions making up the saved-transfer count are
nonnegative, so it is nonzero exactly when either bounded value differs from
its bound. A task-only arithmetic comparison checked 73,434 small and maximum
`lib_u32` boundary combinations. This is not a test of the production C
implementation, the remaining terms, or guest execution.

A scratch implementation tested that substitution using the configured CMOS
and the same ten-million-instruction checkpoint. Sequential
baseline/candidate/candidate/baseline milliseconds were
20298/22071/23297/24634 on x64 and 32108/28134/23521/22885 on x86. Both
widths changed direction, with pronounced host drift. This reduces neither
asymptotic complexity nor a demonstrated production bottleneck, and obscures
the original source-row coefficients unless further explanation is added. It
was not semantically qualified or merged. Log SHA-256:
`D8BE15882EFAD82C51B1F3C30E77729D04E2745120511D89DD4064EDAE2B0D7B`.

The same 79,964,294 guest-clock checkpoint corresponds to approximately
2.42 seconds at the configured 33 MHz PC110 clock, versus 20--32 seconds of
host time in these individual startup probes. This is a workload-specific
throughput comparison, not a complete interactive or real-time measurement;
it illustrates why a small isolated arithmetic saving cannot alone account
for the remaining usability gap.

## Device-advance and keyboard observation boundaries

The late worker sample includes `x86_fdc_advance_at` (66 of 3,432 samples),
`x86_fdc_next_due_tick` (46), and `kbc_keyboard_inputs` (80). These are
exclusive instruction-pointer labels from the same perturbed x64 sampling
run, not inclusive costs and not independent device-time estimates. Even
removing a whole named leaf would not explain the approximately eightfold
fixed-work gap; source audit is needed before interpreting their frequency as
redundancy.

The FDC advance stores the current absolute guest tick before processing head
unload, readiness, command, seek, reset, DMA and non-DMA events. Its deadline
query separately considers pending events, including events due at the stored
tick. Consequently an idle-looking advance cannot simply be dropped: later
commands and externally changed readiness can observe that tick or change the
next action. A safe reduction would have to retain the time update and the
existing event/readiness boundaries, rather than assuming absence of media
activity from the previous observation.

The KBC deadline query samples its endpoint, while the subsequent advance may
drain serial input, consume BAT, publish replies, call endpoint advancement
and deliver output. The link explicitly samples endpoint state and does not
retain it. Reusing the earlier snapshot across these stages would change the
observation boundary and can miss changes caused by those callbacks. The PIC
refresh is likewise not a pure repeated assignment: in level mode setting
unchanged input levels can re-latch an asserted interrupt after an
acknowledgment. These findings do not preclude smaller local simplifications,
but rule out treating unchanged-looking device input as permission to skip
an entire device callback. No new runtime test, source change or performance
claim is made by this inspection.

## 486 decode dependency classification

The 486 decode penalty checks whether consecutive instructions are
PUSH/POP before applying its exception for stack-pointer dependencies. If the
previous write mask and the current address-base mask do not overlap, the
exception cannot affect the final penalty; if they overlap on a non-stack
register, that overlap remains chargeable regardless of PUSH/POP. The opcode
classifier is a pure local function. A scratch candidate moved these tests
behind the mask intersection and skipped them for non-stack overlap; no
instruction timing rule or runtime interface was changed.

An exhaustive 262,144-case Boolean comparison covered all two eight-bit masks
and the two opcode-classification results. This validates that expression,
not the compiled decoder or full CPU behavior. The configured-CMOS fixed-work
baseline/candidate/candidate/baseline times were 20823/19660/23547/18825 ms
on x64 and 23576/23789/23649/23957 ms on x86; all reached the same
10,000,000-instruction, 79,964,294-clock and `0003dbc5` checkpoint. The x64
result changed direction and x86 differences are small. The candidate adds
one net source line and a more complicated condition, so the measurements do
not justify merging it. It was not qualified against complete CPU state or
interactive gameplay. Comparison log SHA-256:
`601A91660D41542B54734FF93D4D1E49A590AB9788E27593B318000A6455E37B`.

## Post-board game-stage worker sample

At Shared `02b91c96a` and NXVM receiver `d8a8e95b3`, a new task-only x64
product-path harness used the configured CMOS and private media copies. It
sent the existing menu inputs, waited for the board/score-area marker, and
kept the machine running for 30 seconds. The sampler began on observing the
flushed marker and sampled thread instruction pointers for the following 20
seconds. The marker appeared at 268.38 host seconds; the process exited zero.
The saved frame shows the game board and score area but the top window is not
fully drawn. No input was sent during the sampled interval, so this represents
post-board idle/redraw work, not interaction latency or a complete game.

The worker yielded 3,595 samples. Its largest named exclusive instruction
pointer buckets were CPU refresh 238, CPU physical read 140, Machine run 128,
Machine elapsed-time publication 94, instruction boundary 88, timing selection
87, keyboard input collection 82, CRT 74, 486 source timing 70, memory route
resolution 70, 486 memory penalty 68, FDC advance 66, clock-domain advance 66
and PIC selection 65. Inlining and helpers can hide work under another symbol;
these labels are not inclusive function costs or a proof of total component
percentages. Thread suspension, deterministic timing and symbol attribution
perturb execution. This one x64 sample neither demonstrates x86 behavior nor
establishes native Window/RDP responsiveness. It supports prioritizing the
combined CPU execution, memory and per-instruction advancement chain over a
single isolated device leaf; no one named bucket explains the large gap.

SHA-256 (sampler log, guest log, captured PPM, task-only harness source):
`5BBB3BFF222DDE3EFB8DB305B0A12A4FE9B08DA884DDE0AAFEBC2133F5D72A81`,
`6CF18CD327591B2B8B65DCB22496978FABB6FE5307EB6278A8B5A5E83A2A8F4B`,
`E7543730687E8AB543B42EC141737ABE0D711E9B798CE13AC1D8DD49DF1B9F09`,
`D0771DD9848B41C99A09FF798585A3BEEC98DCA731E378BE806003428E776EB2`.

## Game-stage physical-read inclusive sampling

An independent x64 private-media run used the same configured CMOS, entered
the game through the existing Machine input path and collected a 30-second
post-board interval without injected input. A separately compiled scratch CPU
source sampled one in 256 calls, selected by independent pseudorandom streams,
to CPU refresh and physical read; a link wrapper similarly sampled Machine
run. The physical-read probe uses a scoped cleanup hook so normal and early
returns close the same measurement. After shutdown, the worker-owned counters
reported refresh 17,102,243 calls / 66,846 samples / 143,690,117 TSC units,
physical read 18,013,261 / 70,200 / 28,285,580, and Machine run 41,842 / 155 /
238,672,153. Scaling each interval by its calls/sample count gives roughly
57% for CPU refresh within Machine run and 11% for physical read within
Machine run. Physical reads are ordinarily nested inside refresh; these
figures are not additive. Only 155 Machine-run samples, non-serialized TSC,
host preemption, instrumentation and compiler-layout changes materially limit
precision. They are not a rigorous upper bound or an x86 result.

The first and last logged quanta after the game marker advanced 93,952,525
guest clocks in 29.51 host seconds, approximately 3.18 MHz on the configured
33 MHz axis. The captured image shows the Game title, board and score, but a
single image plus a stationary 30-second interval does not establish input
response, real-time native Window/RDP behavior or complete usability. The
result supports addressing the whole CPU/Machine chain rather than assuming
that removing one physical-read subroutine alone can produce the roughly
tenfold improvement needed here. Production source and owner media were not
modified.

SHA-256 (summary, guest log, captured PPM, sampling source, instrumented CPU
source):
`8B4347A48B6E67456B1BB987C9184A49517558ADAA60EA3CF18787DD714BB782`,
`7FD268871692EB329E1AD85047AE3FD73826395B92FC0CE8C53C77DE0D5CF3AF`,
`48EF7BA99E57CE861A09C2B65B8E6A2E0BBC71AA75374E38074FDC2A130E4538`,
`9196A921ECD548A6E3DD9B971356F81E0A3B8BF37C51BA6113936220B53B4D43`,
`B7A9C7FDA3F3374BEB7E64D01F685A5D44BEC71A1E42678BB6B45C23FB5B97ED`.

The [CMOS probe correction](t582-s34-cmos-probe-correction.md) separately
records why S31/S32 fixed-work results used the empty-CMOS path and supplies a
new configured-seed comparison of S33. No production code or public contract
changed in this audit.

## Interpretation for the next performance investigation

The game-stage estimate above places physical reads inside CPU refresh. As a
thought experiment, if the sampled 11% were representative and all of that
work could be eliminated without affecting anything else, the maximum implied
speedup would be about 1.12 times. Even eliminating the entire estimated 57%
refresh interval would imply only about 2.33 times. Neither elimination is a
valid implementation: guest execution and reads must still occur. These
illustrative calculations inherit the sample's substantial uncertainty and
are not measured speedups or hard bounds. The configured 33 MHz is a guest
clock reference, not by itself a proven interactive acceptance threshold.

The separate World Clock profile breaks refresh into execution, timing and
interrupt stages, but it is a different workload and cannot be multiplied by
the Game-stage percentages to claim a precise Game cost. The independent
NOP/JMP comparison shows a large execution-chain difference even without
firmware shortcuts, while its different CPU, memory and timing contracts
prevent attributing that difference to Machine scheduling alone. Together,
these observations point to a larger end-to-end execution-chain investigation,
including CPU and Machine work, rather than treating one small physical-read
or timing helper as a demonstrated solution. No further candidate or product
speedup is established by this analysis.

## Refresh-request observation candidate

The current Machine arbitration first calls the attachment's
`refresh_request`, then calls `x86_core_machine_refresh_hold_advance`, which
calls the same provider again when no refresh cycle is active. These calls
are adjacent apart from reading the local remaining-cycle state. PC110's
current profile provider always returns false, so its actual board binding
performs two equivalent negative observations in that case. A third observation
is conditional on prefetch-reservation admission and occurs after DMA and CPU
bus advancement; it cannot generally reuse the earlier value because those
operations may change board state.

The Core attachment declaration does not currently state that the provider
is observational or stable across calls, and the helper has two additional
call sites outside arbitration. Therefore reusing the first result would
change the observable callback sequence for a generic attachment and is not
yet established safe. The two calls' frequency and exclusive cost on PC110
have not been measured; this is a candidate for focused measurement and
contract review, not evidence of an interactive speedup. No implementation,
API, executable or test was changed for this observation.

The attachment contract permits callbacks to use bounded Core operations and
does not require repeated requests to return the same value. For example, an
attachment that reports false on the first call and true on the second can
start a refresh through the existing helper; substituting the first result
would not start it. This is a contract-level counterexample, not a claim that
the PC110 provider behaves that way. Consequently a generic reuse patch needs
an independently justified contract change or a different design; the current
PC110 implementation alone does not authorize one. No such patch is made.

### Game-marker timing probe

A task-only x64 scheduler copy sampled one in 256 calls using the existing
pseudorandom TSC instrumenter. It timed the complete arbitration function,
the nested refresh-hold helper and Machine run separately. The private-media
probe reported its Game marker and measured the following 30 seconds before
normal exit (274.69 seconds total). It did not exercise native Window or RDP.

Arbitration: 19,185,845 calls, 74,849 samples, 20,003,406 TSC units;
refresh-hold helper: 19,185,845 calls, 74,780 samples, 3,277,080 TSC units;
Machine run: 46,474 calls, 168 samples, 230,282,829 TSC units. Scaling by
calls per sample gives approximately 8.0% for arbitration within Machine run
and 1.3% for the helper within Machine run. The latter is nested in the former,
not additive. The helper includes more than the second provider call, so it is
not a measurement of that callback alone. Only 168 Machine samples, unsynchronized
TSC, instrumentation, scheduling and compilation layout limit these estimates.
The Game marker and captured frame do not establish interactive playability.
The result does not justify changing the generic refresh callback contract or
claiming this candidate resolves the throughput gap.

SHA-256 (guest log, instrumented scheduler, Game frame):
`B614424D68437CA7B0DFC42DE44B464DD040DEC166E78B8253247C278ECCC3D0`,
`FD14EFFD1914102390C276C0DBB56C1A8E4DDF7D0C939D253CE4D190B78CECBC`,
`6FA8701C11BC23AEA11AF9E43BAED4EFCF92A47976680199CC43491114F69925`.

### Game-marker scheduler phase sampling

Another private-media x64 run used the same Game-marker and 30-second window,
with independently sampled nested timers around the existing scheduler phases.
It exited normally after 267.48 seconds. Scaling sampled TSC intervals by
calls per sample against the sampled Machine-run interval gives approximately
12.2% for Core time/deadline capture, 8.5% for the board before-internal
callback, 1.4% for internal FPU/WAIT advancement and 4.4% for the board
after-internal callback. Machine run had 47,795 calls, 175 samples and
226,103,151 TSC units. Phase calls were about 19.34 million each, with
75,338–75,625 samples. These are approximate inclusive intervals, not an
exact accounting: additional timer checks and compiler layout change runtime,
TSC is not serialized, and the denominator has only 175 samples. The marker
and frame do not establish native Window/RDP responsiveness.

The deadline capture asks the board, timeline, FPU and CPU WAIT owners for
their current deadlines; the board in turn includes video, PIT, RTC, DMA,
media and input. The before phase advances PIT and interrupt readiness before
media, while the after phase advances RTC, keyboard, PIC and video. Both have
observable ordering and cannot simply be suppressed. Previous S13 results and
S28's deadline-removal upper bound remain applicable; this measurement is a
new Game-stage prioritization, not a safe deadline cache design or an achieved
speedup. Investigate repeated calculations inside the 12.2% deadline path
while preserving all mutation and observation boundaries.

SHA-256 (guest log, instrumented scheduler, Game frame):
`80963E164A813E203964E7000DAEFF02EB94BCFAA0AE2407BF87FEDE61075D20`,
`3F5CD57F8A9507F43496F9AE751DCBA35CDC1A136ADE16098D4564E91E957C80`,
`40DF145BD2B6EF004D835A6E2CD309D88DE40606657584ED347FF892B1993E9B`.

### Game-marker board deadline breakdown

A separate task-only x64 board-deadline copy used the same private-media Game
probe and 30-second measurement window; it exited normally after 273.73
seconds. Board deadline observation had 17,359,675 calls / 67,814 samples /
30,720,370 TSC units; its PIT consideration had 17,359,675 / 67,559 /
8,650,660, and its clock-conversion consideration had 17,359,675 / 67,921 /
3,400,700. The Machine-run denominator had 41,061 calls / 153 samples /
243,512,824 units. Scaling by calls per sample gives approximately 12.0%,
3.4% and 1.3% of Machine-run time respectively. PIT and clock considerations
are nested within board deadline; the clock path can itself be nested in PIT,
so none of these figures should be summed. Instrumentation, non-serialized
TSC and the small denominator sample make the percentages approximate.

This workload suggests the deadline cost is distributed: the PIT and clock
subpaths alone do not explain the whole board observation interval. Prior
S13 conversion experiments showed no reliable cross-width benefit, and the
existing deadline-cache mutation analysis remains applicable. No production
optimization follows solely from this probe, and Game-marker capture is not
native Window/RDP or interactive-acceptance evidence.

SHA-256 (guest log, instrumented board source):
`4D015E8A4B6236086AF846833A86B1D517B952B7699719B105D7DEDB6CE400D0`,
`AB51D92709B46EC93DAEE97A8A412266065B98C3CEA6C8C25E176DE78FFCFFD8`.

### Game-marker CPU execution phases

A task-only x64 CPU-instruction source instrumented the existing execution,
timing-selection plus observation, and interrupt-dispatch phases during the
same private-media Game-marker window. The guest exited normally after 280.85
seconds. Execution had 17,633,270 calls / 68,827 samples / 89,194,189 TSC
units; timing plus observation 17,610,986 / 68,539 / 45,186,494; interrupt
dispatch 17,633,269 / 68,945 / 10,470,560. Machine run had 42,821 calls /
160 samples / 246,018,134 units. Scaling each by calls per sample gives
approximately 34.7%, 17.6% and 4.1% of Machine-run intervals respectively.
These phases occur inside CPU refresh; they must not be added to the earlier
inclusive refresh percentage. The short, nonsynchronized TSC samples, altered
compiler layout, added checks and small denominator sample limit precision.
The marker and frame do not establish interactive Window/RDP performance.

This specific Game workload supports prioritizing execution before the timing
selector or interrupt dispatch, while confirming that no one of these phases
alone explains the entire Machine-to-reference difference. Earlier execution,
prefix, preview, SMM and timing candidates remain subject to their existing
correctness and measured-performance limitations; this sample does not make
any previously rejected shortcut safe. No production code, interface or
artifact changed for this probe.

SHA-256 (guest log, instrumented CPU source):
`57A3E15E1457A8769CCDF48BC5458FCE87BBACD74D5B552AE2C38A1A0A81CD40`,
`ED9A8F7EBCBCBEAADE7A696F24DD791C0ADFBEEBC76CF3A6F44CEE298E06CB85`.

### Game-marker instruction admission cost

An independent private-media x64 Game-marker probe sampled the existing
profile/form admission function inside instruction execution. It exited
normally after 288.94 seconds. Admission had 19,903,248 calls / 77,651
samples / 6,996,320 TSC units; enclosing instruction execution had
17,620,835 / 68,575 / 92,454,154; Machine run had 42,057 / 157 /
262,345,939. Scaling by calls per sample gives roughly 2.6% of Machine run
for admission and 33.8% for instruction execution; the former is nested in
the latter. It is about 7.5% of that execution interval. The 157 denominator
samples, unsynchronized TSC, profiling checks, host scheduling and compiler
layout limit accuracy. This is not an x86 result or native Window/RDP gameplay
qualification.

Even eliminating the entire measured admission interval would not plausibly
resolve the PC110 throughput gap. The result does not justify introducing a
second instruction table or a new per-context cache. Continue looking inside
the execution and fetch paths, preserving the single existing source of
instruction eligibility. No production code or artifact changed.

SHA-256 (guest log, instrumented CPU source):
`B368AAA13BA91DAA235D200E0F1A86FBF2DAC621CB58B35E6A6EB40CFF669A46`,
`02D81156AF000BF4F56FE5B656D8D947186FBE49129F93686AB55F959CE6D272`.

### Game-marker instruction preparation and code reads

A private-media x64 Game-marker run sampled the existing `ExecInit`,
`_s_read_cs`, `ExecFinal`, `ExecIns` and Machine-run scopes independently.
It exited normally after 301.11 seconds. Calls / samples / TSC totals were:
`ExecInit` 20,841,696 / 81,238 / 32,019,370; `_s_read_cs` 39,917,303 /
155,889 / 10,499,669; `ExecFinal` 18,362,594 / 71,800 / 4,204,985;
`ExecIns` 18,362,538 / 71,595 / 92,491,701; Machine run 44,297 / 163 /
247,401,034. Scaling by calls per sample gives approximately 12.2%, 4.0%,
1.6% and 35.3% of Machine-run intervals respectively. These scopes overlap:
initialization and code reads can occur inside execution, and initialization
can also occur for preview. They cannot be added. Timer overhead, instrumented
compiler layout, host scheduling and the 163-sample denominator materially
limit these estimates; the marker and frame are not desktop usability proof.

The initialization result identifies a substantial execution-chain candidate
for a new correctness-preserving investigation. It does not establish that
the old CPU snapshot, prefetch refill or any individual suboperation owns the
whole interval. The existing S23 initialization and REP-prefetch findings
still apply: do not silently reuse a stale fetch or change guest clocks. No
production code or artifact changed for this measurement.

SHA-256 (guest log, instrumented CPU source):
`8340E3D88ED2F1E4E8FBB61403FA97A5524D7AA827F965A3D03CD8BF01A7B19B`,
`88E7CF14F84F3FF6E256D73A0A13DE533D364FDD6C207DC0BB128405DB760ACF`.

### Game-marker initialization boundary breakdown

A further task-only x64 run measured `ExecBoundary` and its enclosing
`ExecInit` during the same Game-marker window, using private media. It exited
normally after 290.86 seconds. Boundary had 17,182,716 calls / 67,172 samples /
12,600,206 TSC units; initialization had 17,166,101 / 66,781 / 32,519,081;
Machine run had 36,224 / 133 / 225,352,246. Scaled estimates put boundary at
about 5.3% of Machine run and initialization at 13.6%; boundary is nested in
initialization and the estimates cannot be added. The differing call totals
also include asynchronous boundary calls. Sampling overhead, compiler layout,
non-serialized TSC, host scheduling and only 133 Machine samples materially
limit accuracy. This is not a native Window/RDP or x86 qualification.

The result directs attention to both the boundary and the remaining fetch and
prefetch work. It does not measure the 380-byte snapshot independently, prove
that removing it is correct, or authorize the previously rejected REP shortcut.
No production code or artifact changed.

SHA-256 (guest log, instrumented CPU source):
`E53635D9F7FAA26BAA0FE1E08F8D946ABECFE34FD7EF9D01D90E73843CFCB045`,
`DADFD70E43062E30312E30E11B41982F6257057F94A00DB67C84AFDC97377030`.

### Game-marker prefetch logical-read cost

A task-only x64 Game-marker run sampled logical reads tagged as instruction
prefetch and the enclosing `ExecInit`, alongside Machine run. It used private
media and exited normally after 298.30 seconds. Prefetch reads had 6,109,071
calls / 23,920 samples / 19,269,054 TSC units; initialization had 19,280,707 /
75,144 / 31,987,965; Machine run had 41,527 / 154 / 245,133,919. Scaling by
calls per sample yields approximately 7.4% and 12.4% of Machine-run time,
respectively. Prefetch is inside initialization; the estimates cannot be
added. The instrumented logical-read function may recurse on legacy segment
wrap, so nested calls may inflate its inclusive total. Added provenance checks,
non-serialized TSC, compilation layout, scheduling and only 154 Machine
samples further limit the precision. This is not x86 or native Window/RDP
usability evidence.

The logical prefetch path includes segmentation and paging, cache/bus access,
fault and timing effects, rather than a single replaceable memory copy. Its
measured cost reinforces investigating shared access calculations, but does not
justify bypassing those effects or the previously rejected REP shortcut. No
production code or artifact changed for this probe.

SHA-256 (guest log, instrumented CPU source):
`C8F46582EFD6C6DB384A962AB32C4AE811F637965341662792C9778348D7379A`,
`2F4087F64F1D2E0406D973D3959D18981D24AEB21D7362CB7F60ACA0CB57A4A5`.

### Game-marker arithmetic-flag cost

An x64 private-media Game-marker run sampled `_kaf_set_flags`, enclosing
`ExecIns` and Machine run independently and exited normally after 292.69 s.
Calls / samples / TSC totals were 4,097,409 / 16,109 / 1,629,920;
19,001,461 / 74,028 / 90,029,079; and 46,293 / 167 / 225,367,762,
respectively. Scaling by calls per sample gives about 0.7% of Machine run
for this flag setter and 37.0% for execution; the former is nested in the
latter. This does not include flags updated by paths that do not call the
setter. Timer overhead, altered layout, unsynchronized TSC and the small
Machine sample limit precision. The result does not justify a large flags
rewrite as the PC110 performance solution; it does not prove x86 or desktop
behavior. Production code and artifacts did not change.

SHA-256 (guest log, instrumented CPU source):
`730D9E78A289E1457924F5B15FE57B5A707C1B2C225D05CAA07EECEB716C1E0F`,
`179DF376E79B7676A2EA5FC3CD567F07CCCF7C261701C35FD9639CCF1FC486EE`.

### Game-marker 486 read-cache outcomes

A private-media x64 Game-marker run counted each 486 physical-read cache
lookup by preview and fetch provenance. It exited normally after 305.29 s.
Non-preview fetch lookups recorded 6,302,829 hits and 2,738,220 misses
(about 69.7% hits); other non-preview reads 5,758,072 hits and 3,904,275
misses (about 59.6% hits). Preview fetch lookups recorded 2,502,781 hits and
672,510 misses; other preview reads 173,465 hits and 115 misses. These are
lookup iterations, not bytes, retired instructions or measured elapsed-time
shares. All lookups are counted and the added callback perturbs execution;
the marker window is asynchronous and this is neither x86 nor native
Window/RDP qualification. The counts do not classify why misses occurred.

Both hit and miss paths are materially exercised, so an optimization should
not assume either outcome or unconditionally change provider cacheability.
This observation does not validate skipping cache, bus or timing effects.
No production code or artifact changed.

SHA-256 (guest log, instrumented CPU source):
`7BF3FA80CC878D117D0A803DEBEBB2773247C910A5780A0565C622978C222D4E`,
`D2F261009D46ED4196D42CCFC6D7FCA27D04CAA9A40E9A644176F67AFB77E4AF`.
