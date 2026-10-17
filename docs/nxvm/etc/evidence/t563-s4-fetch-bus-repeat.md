# T563 S4 Fetch, Bus, Decode And REP

## Baseline And Frozen Universe

Admitted from clean ed59d3182 after accepted S3. Original C01--C17 and
[S1 R5/R6](t563-s1-rebase.md) remain the receiving obligations, not a moving
list of first failures. [S3](t563-s3-memory-translation.md) allocates ordinary
access/translation, not repeated iteration distribution or final runtime.

| ID | Complete cohort | Initial evidence and required proof |
| --- | --- | --- |
| F1 | Instruction fetch: resident/missing/ineligible, line/page crossing, queue/refill, branch taken/untaken and target association | Actual `_kma_read_bus` owns successful fetch baseline; cache/queue and failure matrices exist. Reconcile decoded instruction versus fetched queue/target and prove no fictional read or duplicate DATA charge. |
| F2 | Physical read/burst/write baseline, cache fills/invalidations, overlap and write-buffer assumptions | Source tables assume fastest 2-1-2 bus and ready write buffers. Inspect actual synchronous provider and Core pending owner before allocating any additional term; explicit L2 assumptions are not hardware pipeline claims. |
| F3 | Extra waits, LOCK/HOLD/DMA, cancellation and reset | Existing Core external-cycle trace and pending completion own waits and the sole timeline; prove real successful-prefix, HOLD interlock and reset/invalidation behavior without a CPU-side second wait subtotal. |
| D1 | Every prefix spelling/count, ordinary versus repeated string applicability, EA/index/displacement+immediate and AGI | Existing decode facts cover all ModRM/SIB/address/code widths; redundant REP and ignored REP still return unallocated. Reconcile original rows and exact register-write identity, same-value writes, stack exemptions and failure lifetime. |
| R1 | MOVS/CMPS/STOS/LODS/SCAS, INS/OUTS; byte/word/dword, modes/IOPL, operand/address widths, both directions and prefix choices | Existing nominal setup/iteration rows and source_repeat_active state; complete numerical distribution from actual memory/permission accesses, independent cache residency and line splits. |
| R2 | Zero, one, many, early compare stop, final iteration and failed later iteration | No repeated setup/prefix cost; no data access for zero count; accepted physical prefixes retained on failure without false successful cost. |
| R3 | Reset, debug-entry/register/memory edits, interrupt/task/SMI transitions, self-modification, preview and query purity | Inspect every actual repeat/dependency invalidation owner and prove same-byte versus changed-byte identity. Events are S5, final selector/preview publication S6, but their lifecycle boundary must not retain a stale continuation. |

The initial census is read-only. Numerical models require original-page
review and a recorded decision before implementation. Current source keeps
selected-SX final timing unallocated and C17 closed. Complete unit results
from S3 are a baseline, not fresh S4 qualification.

## Owners And Invariants

CPU owns real cache/TLB and prefetch state, decoded/access/dependency facts and
the existing per-macroinstruction repeat identity. Core `cpu_bus.c` owns
external wait windows/page-policy accounting; `machine.c` owns HOLD and pending
completion/time publication. Neither will duplicate the other's waiting or
create a speculative target read merely to compute timing. Failed/event-only
elapsed aggregation remains S5. The final selected aggregate and safe preview
bound remain S6, not a second selector in this S.

Architecture-governance and coding-governance skills apply to the source owner
and real failure/publication boundary. Preserve original instruction style,
Types vocabulary, inward dependencies, explicit L2 versus source-backed rows,
and all earlier CPU families. Admission initially proposed no public API;
the later owner-approved service expansion is recorded below. Initial
code estimates are in the active packet; source/route review must refine them
before editing the corresponding cohort. No Lib/Emulator/MyNES, owner INI,
firmware, disk or snapshot change is authorized here.

## REP And Prefix Source Decision

Re-read the original 1992 Intel486 PRM, SHA-256
`D86207AAEBD9B10464F2058EFC61559E6141D3B072DB100E89BFE17C8187E143`,
Appendix E-10/E-11/E-14/E-15 (PDF 822/823/826/827), including the printed
table layout and notes 16--20. The retained external manual is read-only;
no source code is imported. E-10 assigns MOVS one-move miss cost 1; note 19
distributes a new missing line as 1 on its first operation and 3 on the next.
SCAS note 20 distributes 2 then 2; OUTS E-15 note 3 places 2 on the second.
CMPS's full missing pair is 6 on the first compare, and LODS is 2 on its first
load. STOS/INS have no ordinary read-miss addition. Zero-count performs no
access; the nominal setup owner remains unchanged.

Extend the existing repeat owner with the previous iteration's actual
non-TSS physical data-miss count, and copy it to the current instruction only
for a validated continuation. This is a timing observation with the existing
macroinstruction lifetime, not another cache or a second repeat state machine.
Pure memory-cost queries consume current and previous facts; only successful
retirement records the next observation. Existing reset/debug/entry/event/
changed-byte invalidation remains authoritative. A failed iteration neither
publishes a successful cost nor advances that identity. Permission reads keep
S3's explicit L2 two-clock actual-miss differential, separately identified by
the existing TSS-field observation.

Role review also covers an unaligned TSS base: its two-byte I/O-map pointer
can cross a cache line. Add the completed logical TSS-field miss count at the
same `_s_read_tss` owner, beside its existing physical fragments; permission
cost is S3's two-clock logical term plus the qualified extra-fragment bound.
Do not assume one physical TSS fragment always means one completed logical
field or subtract it from the wrong counter. This is one private observation,
not a new permission route; add split-TSS real port/repeat regression.

For CMPS partial residency retain the symmetric three-clock logical-read
share; split/uncached layouts use the existing explicit L2 extra-fragment
bound. MOVS/SCAS/OUTS carry the source second-operation term across the real
primitive boundary, even when the next read hits. Nonreference sharing,
splits or uncached distribution are marked L2, not claimed as Intel's exact
cache-line pipeline. Do not invent future data reads or charge an unexecuted
second operation after early/final termination.

E-11 lists address, operand, LOCK and segment prefixes at one clock. It does
not specify redundant/ignored REP timing. For already implemented selected-SX
semantics, retain the effective string REP in nominal setup, and model each
additional/ignored REP byte as one decode clock, explicitly L2. F2 on a
noncompare string is already executed as repetition; preserve that behavior,
use the same selected alias model and validate its actual continuation rather
than inheriting older-family repeat-prefix admission. The private decode
query gains an optional model-grade output; no public ABI changes. All
remaining EA/AGI source terms stay in its existing sole owner.

Expected increment for this cohesive REP/prefix batch: production
+100--170/-15--35 across CPU private observations/timing, tests
+220--400/-20--45 across existing repeat/cache/nominal files. This is within
S4, not completion of F1--F3; their complete source/route proof remains open.

## Fetch Association And Core Wait Review

The existing completed-read observation is already at the correct owner.
Add actual near JMP/CALL/RET/Jcc vectors with warm/cold targets, one/fifteen
decoded bytes and ordinary line versus 4-KiB page crossings. No target read
occurs during the branch merely to manufacture a penalty; the next real fetch
owns its successful physical work. This staged serialized queue-fill value
remains explicitly L2, not the E.2 best-case two-clock target access. A
page-clipped queue fetches the second page only if decoding needs it. The
first draft assumed eager fill across that page and failed for a one-byte
target; correct the independent expected access count, not production.

Core's existing external-cycle and pending-retirement route already owns
extra waits. Extend its actual RDY pause/resume/reset and physical-publication
guard test to internal selected-SX qualification as well as retained 386.
Existing executor HOLD/LOCK/DMA tests already include 486SX. No production
Core wait or HOLD owner is replaced. Expected increment for these owner tests
is +80--100/-3--6, no production change. F2 write-buffer/overlap and invalidate
contention still require a complete, source-honest disposition; they are not
silently declared absent merely because callbacks complete synchronously.

## REP/Fetch Development Checkpoint

The existing Cache test adds 3,600 repeat input configurations (18,900
primitive/zero-count rounds plus real warmup/NOP execution): five memory
string families, three widths, F2/F3, both directions, aligned/misaligned/
split addresses, cold/full/source-only/destination-only/disabled cache and
counts 0/1/2/17. Fixed expected line-entry facts are independent of the
production observation counters. Current/previous misses, logical reads,
counts, EIP, costs, source/model flag and next-NOP clearing are checked.

The existing REP test adds numerical permission/cache costs to its mode/width
matrix, four failed later-port transfers, and 128 split-TSS/cache-mask setups
with three actual transfers each. An unaligned TSS pointer has two physical
fragments but one completed logical read. Its independent expected terms
include the existing alignment surcharge. The cached-port provider is now
one inline fixture capability reused by nominal and REP tests, not duplicated
in both files. Existing debug/entry/changed-byte/Stop Clock boundaries gain
actual memory-cost checks, and failed/early-exit queries preserve publication.

Prefix tests now cover all 0--14-byte lengths for segment prefixes, ignored
mixed REP, mixed string REP and ordinary F3 string REP, including real
continuation without recharging. The private optional model result is
preserved on failure. All new source state remains private CPU observation:
one prior-iteration miss fact at the existing repeat owner, its current
instruction copy, and one completed TSS-field miss count beside its physical
fragments. The pure cost queries never advance those observations.

Sixty actual JMP/CALL/RET/Jcc scenarios prove source versus destination
fetch ownership for one/fifteen decoded bytes, cached/cold targets, line/page
crossing and taken/untaken flow. The existing Core RDY wait/pending retirement,
reset and unallocated physical-publication guard now also execute selected-SX
internally; production construction remains closed. Existing DMA/HOLD/LOCK
tests include SX and are executed in the development selection.

Both widths build on the original graphs and pass the five related Cache,
nominal, REP, Core locality and executor tests: x64 5/5 in 5.04 s and x86
5/5 in 3.74 s. Local JUnit records are `build/t563-s4-roles-x64.xml` and
`build/t563-s4-roles-x86.xml`. A first attempted target name for the executor
test was nonexistent; the actual registered target is
`x86-core-machine-executor-run-smoke`, which is built and executed in those
passing results. This is a transient development selection, not a new
qualification inventory. Manifests, inward boundary, docs and diff checks pass.

Actual current C/H delta against ed59d3182 is production +78/-19 (net +59),
tests +368/-38 (net +330), excluding manifests/docs/artifacts. No public API,
Lib/Emulator/MyNES or asset change, implementation commit, final full unit
run or S4 completion is claimed. Source/model/failure review remains active.

## Remaining F2/F3 Source Review

Original 1997 Intel developer manual 273021-001, SHA-256
`BE6242FD0A0B0663C27AA578C6BB7A3994DB21F1EC31887C2EA440D011B82BD3`,
PDF 53--54 / printed 3-7--3-8, is visually re-read. It specifies four 32-bit
write buffers, FIFO writes, constrained read bypass, disabled buffering with
cache disabled, prior-write completion before LOCK and ordered I/O. Source
E.2 only supplies a ready-buffer assumption, not proof that every program
always meets it. The current synchronous callback path cannot by itself
prove elapsed draining or buffer-full contention; it must not be relabelled
as that proof. Likewise a cache-snoop callback alone does not identify a
contending invalidate cycle or its clock. F2 and the contention portion of
F3 therefore remain open. No guessed write-buffer state, second bus queue,
additional clock API or host-time inference has been introduced to mask them.

Next action is to reconcile the existing CPU/Core service boundary and the
whole finite write/read/LOCK/I/O/HOLD/invalidation cohort before numerical
implementation, then finish the remaining lifetime/source dispositions and
perform frozen full qualification/publication. S4's exit and T563's original
complete C16/C17 objective are unchanged; these open rows are not transferred
away or treated as non-applicable.

## Failed Target Fetch And SMM Lifetime Checkpoint

The existing branch/fetch test now has sixteen additional real execution
cases: JMP/CALL/RET/taken Jcc, line/page boundary, and first/second target-line
failure. The branch must finish before the target read fails. Only completed
target fragments contribute fetch ticks; a failed decode publishes no
retirement cost and a failed memory query preserves its output arguments.
This extends the existing fixture and failure injector by 44 lines, without
changing production. Both widths pass the Cache test; local JUnit records are
`build/t563-s4-branch-failure-{x64,x86}.xml`.

The existing SMM I/O-restart test gains six net lines of assertions: entry
and RSM clear the live REP identity, and restarted repeated INS/OUTS are first
iterations with no inherited previous-miss observation. Both widths pass;
records are `build/t563-s4-smm-lifetime-{x64,x86}.xml`. These results do not
allocate SMM elapsed cost, which remains S5.

## F2/F3 Boundary Census

| Cohort | Current owner and actual observation | Remaining timing obligation |
| --- | --- | --- |
| Completed memory reads and fills | CPU `_kma_read_bus` and actual cache; successful physical fragments only | Retain the existing serialized L2 fetch baseline and S3 data differential; neither proves write-buffer overlap. |
| Writes and cache policy | CPU `_kma_write_physical` uses one synchronous provider, updates resident lines and obeys NW/LOCK | Four-slot occupancy, acceptance and elapsed FIFO draining are not represented. A returned callback is not elapsed bus service. |
| Full versus available write buffer | No current owner for occupancy | The fifth write must wait for a released slot; the manual's ready-buffer assumption cannot classify every instruction as ready. |
| Read bypass | Current reads execute synchronously after earlier callbacks | Timing needs the cache-hit eligibility of all pending writes and the one-bypass/drain rule. Completed functional writes alone do not encode it. |
| LOCK | CPU begin/complete callbacks and Core transaction owner already serialize actual locked operations | Pending writes must drain before the locked read. Reuse the real lock owner, not a second lock or transaction. |
| I/O | CPU port execution and Core transfer/complete own real accepted accesses | Ordered IN/OUT and the repeated-OUT exception need write-buffer timing at those same boundaries. Permission/data miss costs must not include it twice. |
| CD/NW and reset | CPU owns cache controls, reset and real invalidation | CD disables buffering; NW changes external writes. Reset must discard timing occupancy with the chip, not create another reset owner. |
| Extra waits and failed access | Core `cpu_bus.c` commits page/window waits for accepted cycles and cancels failed cycles | CPU's current void cycle observer receives no wait duration. Buffered-write service must consume that same duration, not add it independently a second time. |
| HOLD/RDY/refresh | Core arbitration/scheduler and pending retirement already own bus grant and the guest timeline | CPU currently receives no general bus-service interval. WAIT advancement is architectural WAIT/ESC polling; optional prefetch reservation is not unconditional write-buffer service. Neither may be repurposed silently. |
| External invalidation | Core snoop calls CPU `signal_cache_write`; CPU owns matching valid bits | The address/size notification has no contending-clock identity. Do not charge one clock for every invalidated line or equate it with bus contention; pending writes also lose read-bypass eligibility. |

The complete source/route census confirms a boundary gap, not another defect
in the newly tested target-fetch path. No write-buffer implementation or new
public interface has been committed or added to production.

A complete repair needs a reviewed CPU/Core service contract: the chip owns
the four bounded hardware timing slots and bypass/order rules; Core retains
the only guest clock, bus arbitration and external wait calculation. Their
existing cycle handoff must convey the already computed service duration,
and actual Core advancement must convey service availability. A timing-only
model must explicitly state its atomic functional-access limitation and L2
grade; it cannot claim to implement delayed MMIO/DMA visibility. A full
deferred-write implementation is a different functional/publication change,
not an incidental timing helper. Neither design may introduce another memory
provider, executor, time axis or error recovery queue.

The active packet requires review before a public interface expansion. The
next design decision must settle that service/commit boundary and its source
grade before a signature or numerical implementation is introduced. Until
then F2/F3 remain open, and the selected-SX production gate stays closed.

## Current Component Regression

After the target-failure and SMM assertions, both original reusable graphs
build the complete `x86-core-tests` target and execute all 165 tests labelled
`x86-core`: x64 165/165 in 55.16 s and x86 165/165 in 14.92 s. Both builds
run with the persistent project ccache directory and Ninja -j8; CTest uses
-j4. Logs/JUnit are `build/t563-s4-core-{x64,x86}-build.log`, matching
`-tests.log` and `build/t563-s4-core-{x64,x86}.xml`. These are component
regressions, not fresh whole-repository, App/external or release qualification.

Actual cumulative C/H delta against ed59d3182 is production +78/-19
(net +59) and tests +420/-40 (net +380), measured by `git diff --numstat`
over tracked C/H under `src/x86-core` and `test/x86-core`. This excludes
manifests, documentation and artifacts. Source/test manifests, inward
dependency gate, documentation governance and diff checks pass. No S4
implementation P, new public API or refreshed PC artifact is claimed. S4
remains active; F2/F3's service boundary needs its design review before
numerical implementation and final complete-S qualification.

## Approved Service Contract And Transport Design

On 2026-10-16 the owner approved the CPU/Core timing contract expansion:
completed-cycle extra-wait feedback, Core-driven CPU bus advancement and a
CPU-owned four-slot L2 timing model. Lib/Emulator and the single functional
memory/port provider remain unchanged. This is not C17 admission or approval
to describe atomic functional writes as delayed MMIO/DMA visibility.

First implement and verify the existing cycle callback's return contract.
It returns the extra wait clocks of this successful matching COMMIT, zero
for BEGIN/CANCEL/OVERLAP_DECLARE or an unmatched COMMIT. Core remains the
sole calculator and retains its current round subtotal until S6 switches the
selected aggregate, so compatibility timing does not lose or double-add time.
CPU records this returned value at the same accepted-access observation
owner; it does not publish another guest-clock increment. Addition overflow
invalidates the private timing observation, never wraps to a plausible cost.

Memory and port instructions must use that one CPU publisher. Currently
Core itself emits port phases while CPU emits memory phases; leaving that
split would deliver memory wait feedback but lose I/O feedback. Move port
phase emission to the existing CPU IN/OUT accepted-transfer boundary and
delete Core's port phase helper/calls. Keep the Core write latch assignment
before transaction/device invocation, the successful CPU-local input commit
before port completion and cancellation on transfer failure. Trace order
relative to the transaction and device callback remains unchanged. Actual
port side effects and their sole provider are not replaced.

The bounded transport increment is estimated at production +45--80/-25--45,
tests +80--140/-8--20 across the existing CPU Cache/port fixtures and Core
locality test. Test actual CPU memory/port accesses, each failed phase,
non-commit/unmatched callback results, two combined wait sources and overflow;
do not use a direct observer-only test as proof of CPU feedback wiring.

## Completed-Wait Transport Review

The existing callback now returns the computed matching-cycle extra wait.
CPU uses one memory/port publisher and records successful COMMIT feedback in
the current instruction, with a sticky addition-overflow observation. BEGIN
and CANCEL values are ignored. Core's existing round subtotal and pending
retirement accounting are unchanged; selected-SX timing still uses the
unallocated compatibility recipe, not an accidentally enabled new aggregate.
The former Core port-phase helper and all of its producers are removed.

The new Cache vectors execute twenty actual memory cases across retained
386/selected-SX, load/store, first/second-fragment failure and overflow. They
check accepted writes, completed versus cancelled cycles, nonwrapping feedback
and reset. Twelve actual IN/OUT cases cover all widths, successful completion
and transfer failure; completion feedback is after CPU-local input publication
and endpoint completion. Core's existing observer test also checks returned
zero for non-commit/cancelled/unmatched cycles, two combined wait sources and
round-subtotal overflow. The four related tests pass per width in
`build/t563-s4-feedback-{x64,x86}.xml`.

The first complete component regression identifies a stale test dependency:
transaction-lifecycle invoked Core's raw port provider directly but expected
CPU cycle events. The provider no longer produces them by design. Replace
that test path with actual NOP-prefetched IN/OUT through the composed CPU,
preserving all widths and endpoint/admission failure cases and adding internal
SX qualification (36 cases). Admission failure occurs at the port, not at an
earlier blocked fetch. Assert accepted endpoint side effects, unchanged EAX
on failure, transaction owner, exact phase order and the resulting Core fault.
No synthetic phase replay or disabled assertion is introduced. One first test
draft checked the last fault event after its own DMA-cleanup transaction had
appended a cancellation; move the assertion before fixture cleanup. The
corrected x64 focused case passes; complete final component results follow.

The transport increment is production +59/-38 (net +21), tests +199/-28
(net +171), against the preceding S4 checkpoint, excluding manifests/docs.
Tests exceed the initial +80--140 estimate because the stale provider-only
transaction test is replaced with real CPU execution and retained failure
coverage, rather than reducing its phase assertions. The callback return is
the only new public contract so far. No second wait calculator, clock, memory
provider or runtime timing selector is added.

The final transport build and complete component regression pass on both
widths: x64 165/165 in 56.02 s and x86 165/165 in 14.83 s. Actual logs and
JUnit records are `build/t563-s4-feedback-final-{x64,x86}-build.log`,
matching `-tests.log` and `build/t563-s4-feedback-final-{x64,x86}.xml`.
Source/test manifests, inward dependency, documentation and diff checks pass.
Cumulative C/H delta is production +137/-57 (net +80), tests +619/-68
(net +551). This checkpoint adds no release artifact or implementation P,
and is not complete S4 or whole-repository qualification.

## Next Four-Slot Model Boundary

The remaining implementation belongs to CPU's existing private timing
mechanism, not Core's board policy. Four bounded timing slots retain only
remaining service and read-bypass eligibility; they do not copy write payloads
or issue another memory callback. They consume real successful writes and
the returned cycle wait, not predicted future guest accesses. Full capacity,
read/LOCK/I/O barriers, CD/NW, accepted-prefix failure, invalidation and reset
must all use this one state owner.

Core supplies service opportunity through the approved bus-advance operation
on its existing guest timeline. No CPU-local absolute clock is created. Any
intra-instruction predicted service needed by the atomic execution model must
be reconciled with that actual advancement, not aged twice when Core later
publishes the same instruction clocks. Full/blocked bus service and the
read-bypass rule require explicit numerical tests, including real Core
HOLD/RDY/refresh. Nominal/access/prefix calculators remain pure; S6 alone
selects the final aggregate and removes obsolete staging. Timing-only early
functional publication and coarse arbitration remain explicit L2 assumptions,
not claims of delayed device visibility or measured pin-level pipeline timing.

## Four-Slot Implementation Decision

Use one private CPU bus-timing unit and fixed four-slot state. Each slot holds
remaining 32-bit-write service and read-bypass eligibility, never data or a
second provider request. A partially serviced head is not preempted by a read;
after it completes, one read can pass the remaining all-hit writes, marking
them ineligible for another bypass. Full slots stall until the head completes.
LOCK and ordered I/O drain earlier writes; single OUT is synchronous, repeated
OUTS may buffer. CD disables buffering. Reset clears the chip's timing state;
external invalidation removes pending writes' bypass eligibility.

The L2 schedule uses the retained fastest 2-1-2 baseline and observed Core
extra waits once per completed provider transfer. Write acceptance consumes
one issue clock; a burst's word count includes its real address offset. The
model records intra-instruction service already accounted by the atomic CPU
owner. Core advancement first reconciles that credit, then services remaining
slots only while the bus/CLK is available. This is remaining work, not a
second absolute clock. A pure aggregate query takes the separately calculated
instruction baseline, adds immediate waits/barrier stalls and applies the
serialized-service lower bound. S6, not this staging unit, will select and
publish that result and reconcile Core's existing pending wait transport.

One execution-scope guard includes real instruction/event processing but
excludes host debugger reads and preview. It is not a Machine lifecycle flag.
New overflow invalidates the private timing model until chip reset; it never
changes an already accepted functional write or starts recovery.

Expected increment: production +260--380/-20--40 in a cohesive private
`cpu/cpu_bus.c`, existing CPU observations/lifecycle/private headers, the
approved public bus-advance operation and Core's scheduler handoff; tests
+350--650 in an owned real-execution bus test and existing Core handoff
regressions. No new manager, thread, configurable queue, payload copy or
write callback. Invalidation's exact contending-clock allocation still needs
its own proof and is not inferred from the number of invalidated cache lines.

## Four-Slot Development Checkpoint

The approved CPU/Core handoff and private four-slot model are implemented.
The new cohesive CPU bus unit owns bounded remaining service, FIFO order,
cache-hit bypass eligibility and reconciliation credit. It stores no payload,
calls no functional provider during advancement and publishes no guest time.
Actual CPU memory/port execution supplies successful accesses and Core's
returned extra waits. Execution scope excludes host inspection and preview.
Reset/profile replacement clears the chip-owned model. The existing Core
arbitration owner supplies elapsed service and blocks it during actual
DMA/refresh use, HOLD ownership and the enabled RDY gate. Its availability
decision is conservative for the current advancement interval, explicitly L2.

The new owned CPU test executes real instructions with wait durations 0, 9
and 30, warm/cold write destinations and ten consecutive writes. Independent
closed-form capacity bounds verify first acceptance, fifth-write stalls and
FIFO draining, including heterogeneous per-write waits. Read bypass, its
one-use invalidation, ordered IN/single OUT, repeated OUTS, ignored REP on
ordinary OUT, implicit LOCK, CD/NW, unaligned writes and accepted-prefix
failures are covered. Blocked service, reconciliation without double aging,
clock stop, reset, host inspection, preview, pure queries and overflow retain
their respective failure/lifetime assertions. Core's existing locality test
observes the actual scheduler handoff for retained 386 and internally selected
SX, calls the real CPU advancement operation, and verifies HOLD/RDY/refresh
availability. The CPU test owns private FIFO assertions; the Core test does
not borrow private CPU layout. These are complementary boundary proofs, not
final runtime/retirement selection, which remains S6.

Both original graphs build and run all 166 X86 Core tests: x64 166/166 in
8.45 s and x86 166/166 in 7.73 s. Final JUnit records are
`build/t563-s4-fifo-final-x64.xml` and
`build/t563-s4-fifo-final-x86.xml`, with matching build/test logs. An earlier
x64 component run failed only its manifest gate after the executor changed
test assertions during the run. That run is not accepted. The source was
then frozen, manifests refreshed, the changed test rebuilt and both complete
component suites rerun to obtain these final results. Prevention: no source,
test or manifest edits while a qualification process is live; any necessary
edit invalidates the affected frozen run rather than converting it to a pass.

Actual cumulative C/H delta against ed59d3182 is production +437/-62
(net +375) and tests +1050/-74 (net +976). This includes the new 195-line CPU
bus implementation and 400-line CPU test, counted in addition to tracked
`git diff --numstat` C/H entries. Compared with the transport checkpoint,
the four-slot increment is production +300/-5 (net +295) and tests +431/-6
(net +425). The positive size implements a previously absent bounded timing
mechanism and its real-execution/failure proof, not a second execution path.
No public type/enum or Lib/Emulator change is introduced. The approved
completed-cycle callback return and bus-advance operation are the public
contract changes. Source/test manifests, inward dependency, documentation
governance and whitespace checks pass at this checkpoint.

F2's FIFO/service implementation is not a disposition of F3's numeric
contending-invalidation term. Core's address/size snoop may report multiple
alias ranges for one actual external transaction. Counting these callbacks
or invalidated lines as elapsed cycles would be wrong. The remaining design
must identify actual contending cycles at their real transaction/clock owner
and distinguish CPU cache/bus resource demand from idle advancement. Final
cohort/source/lifetime reconciliation and frozen whole-repository, selected
App integration and eight-EXE publication remain required. There is no S4
implementation commit, release refresh, S closure or complete-CPU claim.

## Contending Invalidation Allocation Decision

Re-read original PRM E-2 (PDF 814), assumption 9: one delay clock per
invalidation cycle only when it contends for the internal cache/external bus
which the CPU needs. A line invalidation, alias notification or idle interval
is not itself proof of that condition. Use the already approved Core-clock
bus handoff to pass the interval's copied actual external-invalidation count;
this completes that operation's detailed design without adding another API,
public type, event producer or clock.

Core counts once at the first real positive-length write snoop in each DMA
memory-write transaction. Transaction BEGIN resets this deduplication fact;
aliases do not count twice. A cancellation after an accepted write prefix
does not undo its actual snoop. CPU/self, host inspection, zero-length and
non-write transactions do not produce the external-cycle observation. The
existing arbitration interval owns its local total and clamps CPU attribution
to intervals with real pending CPU instruction/event work. Idle DMA still
invalidates real cache state but incurs no fictional instruction penalty.

CPU records actual internal-cache use at the existing cache-access owner,
beside its completed bus service and pending buffered writes. In the atomic
L2 model, a pending CPU interval with one of these resource needs may contend;
charge one clock per observed invalidation, bounded by that interval's clock
count. This is a conservative interval-overlap model, not a pin-level trace.
The current instruction keeps the additional observed subtotal for its pure
aggregate query. S6 must reconcile observations that arise during Core
advancement before final timing publication; the compatibility selector and
production gate remain closed in S4. No extra functional write, speculative
future read, independent time publication or compensating loop is introduced.

Expected increment: production +70--130/-5--15 across the CPU bus contract,
private cache/access observations and existing Core snoop/transaction/scheduler
owners; tests +120--220 in the existing CPU bus and Core locality tests.
Tests distinguish accepted prefixes from failed-before-write, alias dedup,
idle versus pending CPU, resource-free versus real cache/bus need, interval
bounds, unavailable service, overflow, reset, preview and stopped clock.

## Frozen Cohort Reconciliation

The implementation is frozen for complete S4 qualification. This records
the before/after disposition of every admitted row, not a claim that the
later selector/system/production gates have already passed.

| Row | Implemented owner and direct proof | Source grade and remaining receiver |
| --- | --- | --- |
| F1 | Existing actual cache/refill path retained; Cache `cache_fetch_timing` and `cache_branch_fetch_timing` execute fetch, page/line, branch-source/target and accepted-prefix failure cases. Target fetch is never synthesized at the branch. | Fastest serialized 2-1 burst observations remain L2; original target assumptions retained. S6 selects and reconciles the final aggregate rather than double-adding a fetch component. |
| F2 | Sole CPU bus-timing unit now owns four FIFO timing slots and remaining service; actual execution tests cover capacity, heterogeneous waits, partial service, bypass, full stalls, read/LOCK/IN/single-OUT barriers, repeated OUTS, CD/NW, split transfers, failure prefixes and pure query behavior. Core advances that same model. | Four slots/order/barriers are original-manual facts; atomic functional writes and interval-level overlap are explicitly L2. No delayed functional payload or second write path. S6 connects the complete selected total. |
| F3 | Core retains wait calculation and sole guest time. One CPU memory/port phase publisher consumes matching completed waits. Core scheduler proves HOLD/RDY/refresh service availability. Real DMA writes now supply transaction-deduplicated invalidation cycles, including alias notifications and cancelled accepted prefixes; CPU resource-use tests prove the bounded contention subtotal. | Original one-clock contention coefficient; conservative interval overlap is L2. Failed providers may have unknown accepted extent under the existing memory contract: cache invalidation is conservative, never proof of a successful whole write. S6 reconciles the additional observation produced during Core advancement before publishing a final selected result. |
| D1 | Existing sole decode/EA/AGI owner retained; nominal tests cover prefix lengths 0--14, redundant/ignored REP, address/code sizes, ModRM/SIB/displacement/immediate and actual GPR destination identities including same-value writes and adjacency/reset/entry/debug boundaries. Effective REP setup belongs to nominal timing, not each primitive. | Existing source-backed E-2/E-11 terms retained; undocumented redundant/ignored REP uses explicit L2 one-clock decode estimate. No second prefix parser; the shared private prefix counter serves OUTS classification too. |
| R1 | One validated REP identity now carries the previous iteration's actual nonpermission misses. Cache's 3,600 configurations cover all five memory-string families and mode/width/direction/residency/layout/count variants; REP's real port/TSS matrices cover INS/OUTS and split logical permission fields. | Original first/second-operation weights are retained. Nonreference sharing, permission/split and cross-primitive allocation are qualified L2, not hardware-pipeline measurements. S6 combines the distinct decode, memory and bus observations once. |
| R2 | Real zero/first/continuing/final/early/faulting execution verifies no duplicate setup/prefix charge, zero-count no data access, no unexecuted next-iteration charge and no successful result after a failed transfer. Actual port and memory prefixes remain visible under existing failure contracts. | No failed access is relabelled successful. Event/fault elapsed totals remain the original S5 receiver, distinct from S4 continuation and accepted-access ownership. |
| R3 | Existing `source_repeat_active` lifetime owner remains authoritative. Reset, debug mutation, entry/task, byte-identity change and SMM/RSM I/O restart invalidate continuation through their existing paths; CPU/Machine reset clears the new bus facts. Query/preview/host-read/clock-stop tests preserve timing state; a stalled instruction cannot query a stale successful bus result. | No second lifecycle flag or cache. Payload fields behind a false validity fact need not be erased to become invalid. S5 qualifies full event/power elapsed totals; S6 alone publishes selected runtime/preview and budgets; C17 stays closed. |

The same-issue sweep includes `source_repeat_active`,
`source_repeat_data_misses`, `timing_previous_gpr_writes`,
`signal_cache_write`, `dma_memory_cycle`, `execution_advance_bus` and every
external-cycle callback/producer under tracked `src/` and `test/`. Production
DMA access enters the sole Board DMA-bus adapter, then the Core transaction
and memory-snoop path. CPU/FPU/Machine tests remain under their actual owner;
there is no App-private test dependency or change to Lib/Emulator/MyNES.
Host writes can invalidate cache content but do not manufacture external DMA
cycles; actual external transfer failure retains the memory provider's
existing conservative accepted-prefix contract rather than inventing rollback.

The implementation review also checks every modified production hunk and
affected test changes, not only aggregate statistics. The bus FIFO contains
latency/eligibility only; reconciliation credit is remaining accounted work,
not an absolute clock. Real cache use is observed at the existing cache
access owner; it is not another cache or a product-state flag. Core's two new
private facts are the current interval's invalidation count and current
transaction's alias-dedup bit. Both reset with Core, and the interval count
starts fresh at each arbitration advancement. All new state has one owner
and a named numeric/lifetime regression.

These are code/source and direct development-test dispositions. Whole-suite,
specialized-gate, selected-integration, artifact and actual-commit acceptance
still require their separate results below. The selected compatibility recipe
and closed production construction are unchanged.

## Final Frozen Qualification And Actual Diff

The original two Ninja/ccache graphs complete all 632 repository-only units
per width: x64 632/632 in 317.32 s, x86 632/632 in 104.36 s. This includes
all 166 X86 Core tests, Lib/Emulator, other X86 packages and all five Apps.
The three automated desktop-labelled tests also pass on each width (3.18 s
and 3.15 s aggregate). Manual RDP, physical MIDI/audio and complete guest
Windows GUI qualification were not performed or inferred from these tests.

Each width builds and deploys four 0563 PC artifacts, then passes the selected
16 integrations: default and XT three each, AT and DeskPro five each. Both
DeskPro boot matrices pass without changed budgets (x64 profile group
133.50 s, x86 165.36 s). Each profile's deployed executable passes the final
INI/CLI process check. Both graphs return to default configuration; the
default build/deployment hashes match. Eight final PE widths, sizes and
SHA-256 values are recorded in [artifact evidence](t563-s4-artifacts.json),
after all builds and gates. All four asset directories retain exactly their
current two EXEs. INIs, snapshots, media, external firmware, MyNES and sibling
repositories are unchanged.

Both complete specialized-gate targets pass. The direct compilation matrix
now retains 498 strict rows, zero deferred; existing documented source-warning
exceptions are not silently reclassified as absent. Source/test manifests,
inward dependency/Types boundaries, documentation governance and whitespace
checks pass. JUnit and logs use `build/t563-s4-{x64,x86}-units.*` and
`build/t563-s4-{x64,x86}-{nxvm,my5160,my5170,mydeskpro386}-integration.*`.
Final aggregate logs are `build/t563-s4-x64-specialized.log` and
`build/t563-s4-x86-specialized-final.log`; each final published CLI check has
the matching `-published-cli.log`.

The first x86 specialized aggregate exposed an actual stale verifier owner:
it searched for timing selection only after the public refresh definition,
but S4's execution-scope wrapper necessarily calls the private execution
body. The original selector-before-interrupt order was unchanged. Update
the NXVM-owned checker, not the runtime: require exactly one public entry,
one private body and one call bracketed by the execution-scope guard, then
check selector/interrupt ordering within that actual body. Positive source
passes. Two isolated copied-source negative probes reject an early interrupt
and omitted scope cleanup with their specific errors; logs are
`build/t563-s4-seam-negative-order.log` and `-scope.log`. No fallback,
disabled check or alternate selector was introduced. The corrected complete
x86 aggregate and all four final CLI checks pass. x64's final aggregate uses
the corrected verifier and passes too. Prevention: an ownership refactor's
same-issue sweep must include source-scanning gates, not only C callers.

| Counted C/H path | Added | Deleted | Net |
| --- | ---: | ---: | ---: |
| src/x86-core/cpu/cpu.c | 5 | 0 | 5 |
| src/x86-core/cpu/cpu_bus.c | 209 | 0 | 209 |
| src/x86-core/cpu/cpu_instructions.c | 98 | 17 | 81 |
| src/x86-core/cpu/cpu_instructions.h | 29 | 0 | 29 |
| src/x86-core/cpu/cpu_interface.h | 12 | 1 | 11 |
| src/x86-core/cpu/cpu_timing.h | 20 | 2 | 18 |
| src/x86-core/cpu/cpu_timing_model.c | 66 | 17 | 49 |
| src/x86-core/machine/cpu_bus.c | 21 | 24 | -3 |
| src/x86-core/machine/machine.c | 4 | 0 | 4 |
| src/x86-core/machine/machine.h | 3 | 1 | 2 |
| src/x86-core/machine/machine_scheduler.c | 12 | 0 | 12 |
| **Production** | **479** | **62** | **417** |
| test/x86-core/cpu/cpu_bus_timing_smoke.c | 468 | 0 | 468 |
| test/x86-core/cpu/cpu_486_nominal_timing_smoke.c | 73 | 32 | 41 |
| test/x86-core/cpu/cpu_486_repeat_timing_smoke.c | 132 | 4 | 128 |
| test/x86-core/cpu/cpu_cache_smoke.c | 294 | 2 | 292 |
| test/x86-core/cpu/cpu_smm_smoke.c | 8 | 2 | 6 |
| test/x86-core/cpu/support/cpu_port_instruction_fixture.h | 40 | 0 | 40 |
| test/x86-core/machine/machine_prefetch_locality_smoke.c | 167 | 21 | 146 |
| test/x86-core/machine/machine_transaction_lifecycle_smoke.c | 41 | 13 | 28 |
| **Tests** | **1223** | **74** | **1149** |

Counts are actual C/H line changes against ed59d3182, including the two new
files and excluding manifests/docs/artifacts/CMake. Invalidation's incremental
change over the four-slot checkpoint is production +42/-0, tests +173/-0;
it is smaller than the estimate because existing cache, transaction, scheduler
and reset owners suffice. The NXVM timing-seam verifier is separately +17/-2.
The new positive implementation is justified by previously absent four-slot
timing and actual-source observations, not duplicated memory or time paths.
The existing CPU bus helper is the only completed-access phase publisher; obsolete
Core port-phase emission is deleted. Test-only raw-provider timing replay is
replaced by actual CPU execution. Public changes remain the approved callback
return and Core-clock bus advancement; no public type/enum, Lib or Emulator
change is made. S5--S8 and the complete T563 objective remain open.

Shared P1 `f48b7984c` is committed and pushed after all required qualification.
Its 24 source/test/build/manifest paths are the frozen build input. The NXVM
P2 containing this record supplies the project-owned timing-seam checker,
complete active packet, evidence and eight final artifacts. No executable
input changes occur between these target-separated deliveries. Coordinator
acceptance still requires actual committed-diff and artifact verification;
the executor report alone does not close S4.
