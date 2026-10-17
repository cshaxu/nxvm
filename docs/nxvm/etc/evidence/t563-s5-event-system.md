# T563 S5 Event And System Elapsed Time

## Baseline And Frozen Universe

Admitted from clean accepted 54292e0bd. The original
[C01--C17 ledger](t560-s1-cpu-convergence.md),
[T563 S1 R7](t563-s1-rebase.md) and
[T560 E01--E20 system matrix](t560-s7-smm-system.md) remain mandatory inputs.
S4's [fetch/bus/decode/REP delivery](t563-s4-fetch-bus-repeat.md) supplies
actual completed-access and bus-service observations, not complete event or
selected runtime timing. The finite batch below covers successful and failed
paths, mode/CPL/width/cache/translation, reset/lifetime and preview wherever
the mechanism applies. Existing successful tests are not acceptance by
themselves.

| ID | Complete cohort | Current owner and confirmed remaining work |
| --- | --- | --- |
| E1 | Software INT/INTO/INT1, NMI/INTR, debug/fault entry; real/protected/VM86, same/inner privilege and Task Gate | Existing CPU entry owners and nominal table. Only successful external/exception entry currently accumulates base/setup; real entry cache/translation, accepted-prefix and failed/nested work still need numerical allocation. Software INT must not acquire a second event base on top of its nominal row. |
| E2 | Failed initial entry, contributory/page nesting, double fault, shutdown and wake; provider/acknowledgement failure | Existing ExecFinal/entry owner and Core pending-completion route. Complete actual accepted work and nesting setup must survive without charging nonexistent completed entry or fabricating retirement. Guard overflow and next-round clearing. |
| E3 | SMI entry, pending/nonrecursive priority, state save and valid/invalid RSM; HALT/I/O restart, relocated SMBASE and writable/reserved image state | Existing SMM owner and its admitted source/L2 image. Entry currently emits no event elapsed subtotal; RSM lacks selected timing. S4's pending write service must drain before SMIACT switches memory ownership. Source latencies and actual emitted access roles need an explicit model, not copied later-family numbers. |
| E4 | HLT/Stop Grant/shutdown special cycles, FLUSH and pending writes; HOLD/RDY/output acknowledgement/failure | Existing system-boundary, cache and bus owners. Reconcile completed writes before the corresponding physical effect, no successful no-op missing provider, and event-only versus real instruction time. Preserve all older-family behavior. |
| E5 | RESET/SRESET, qualified SMI/NMI/FLUSH/STPCLK/CLK, stopped/restarted clock and synchronizer/PLL assumptions | Existing CPU qualified signal and Core reset/clock contracts. Logical API inputs already represent qualified levels; CLK=true explicitly means PLL stable. Prove no host-generated time, no phantom instruction and correct pending state. Raw board pulse qualification remains the existing board receiver, not silently implemented inside CPU. |
| E6 | Event-only and retired-plus-entry Core transport, partial waits, budgets, overflow, reset, preview and source grade | Existing event_ticks value and one Core timeline/pending owner. Complete event allocations must reach that same path; final selected aggregate/preview eligibility remains S6, C17 production stays closed. |

The initial census is read-only. Architecture-governance first fixes the
single CPU event/access and Core time ownership; coding-governance preserves
the existing instruction style, Types vocabulary and real failure boundaries.
Do not introduce a separate event engine, state-save interpreter, clock,
memory provider, recovery loop or public mutable CPU layout.

## Primary Sources Re-read At Admission

Original Intel 1992 PRM, SHA-256
`D86207AAEBD9B10464F2058EFC61559E6141D3B072DB100E89BFE17C8187E143`,
Appendix E-13/E-14, remains the base/setup authority already used by staged
interrupt costs. Reconcile its assumptions with actual event accesses and
failed chains rather than simply assigning the complete successful row to
each attempted entry.

Original Intel 1997 developer manual 273021-001, SHA-256
`BE6242FD0A0B0663C27AA578C6BB7A3994DB21F1EC31887C2EA440D011B82BD3`,
sections 8.3.1/8.3.2 and Table 8-1 (PDF 202--204 / printed 8-4--8-6),
provides a selected SX column, not only DX2/DX4. A=2, B=40, F=4 and G=20
are minimum CLK intervals; C state save is approximately 139 and E restore
approximately 236 under the table's explicit zero-wait write/burst/nonburst
and noncacheable restore assumptions. The DX4 example totals are not SX
constants. These minimum/approximate values are not permission to claim
pin-exact constant latency. The chosen complete model and actual-access
reconciliation must be recorded before numerical implementation.

SMIACT must follow completion of pending writes and precede the first SMRAM
access; it remains active through the last restore access. The existing
synchronous functional provider is not proof that S4's modeled write service
has completed. Inspect real state-image accesses, their cache policy and the
qualified clock/signal boundary before assigning a subtotal.

The selected SX datasheet 272769-004, SHA-256
`A35A0A428168ED45428DD886876238424A228B8DB0583EE74A1815A88BF421CD`,
and the same developer manual's standard-bus clock/control sections retain
the original qualified-pin/PLL applicability restrictions. Raw synchronizer
and board clock behavior must not be inferred from a CPU setter returning OK.
No external source code, manual or firmware is imported.

## Execution Boundary And Initial Estimate

Existing CPU execution/entry/timing/private bus files and Core pending-time,
reset/scheduler owners are the production surface. Tests remain with CPU or
neutral Machine, never App/private chip layout across that boundary. Existing
event, SMM, power, cache, nominal and executor tests are extended as cohesive
matrices; a new test/helper needs an actual responsibility, not an extra route.
No public ABI change is planned. Initial C/H estimate: production
+350--750/-60--160, tests +600--1300/-20--80. Refine numerical cohorts before
editing them and count actual tracked changes at delivery.

After source freeze use both original Ninja/ccache graphs, complete repository
units, specialized gates, selected four-PC integrations and eight rebuilt
0563 PC EXEs with published CLI/PE/hash proof. A partial test or development
checkpoint is not a P or S closure. S5 remains active until every row above
has its complete source/model and actual-path disposition. The original
complete 486SX goal, final runtime/preview S6 and C17 are not narrowed.

## First Barrier Decision

Before numerical event totals, repair the explicit 8.3.2 ownership barrier.
After `ExecBoundary` establishes SMI's access observation and before asserting
SMIACT, drain pending write timing through the existing CPU bus owner.
Do not drain before the boundary, which would charge the prior instruction's
observation; do not drain after switching memory ownership. Functional writes
are never replayed. This adds one existing-helper call, no state or API.

Extend the existing SMM fixture to observe pending count and stall subtotal
at the actual SMIACT callback. Execute a real store with 0/9/30 extra waits,
reconcile its completed service through the public Core-clock handoff, then
request SMI. The activation callback must see zero buffered writes and exactly
the remaining 1+wait service clocks. This is a source-backed ordering and L2
service observation proof, not yet the complete SMM elapsed formula. Estimated
delta: production +1/-0, existing test +50--70/-0. First reproduce failure
against the frozen S4 source, then add the single drain call and rerun the
same owned SMM test on both widths.

Table 8-1 was also rendered and visually checked at admission; its SX column
and minimum/approximate qualifiers agree with the source record above. The
renderer reports missing display fonts, but the relevant numeric cells,
labels and SX/DX2/DX4 headers remain legible. No PDF or rendering is committed.

## First Barrier Checkpoint

The pre-fix SMI regression failed on a real buffered store: at SMIACT the
fixture observed one pending write and zero drain stall, instead of zero
pending writes and the remaining 1+wait clocks. The detailed reproduction is
`build/t563-s5-smiact-before-detail-x64.xml`. One drain through the existing
CPU owner fixes the order, with no additional functional memory transfer.
Existing whole SMM test passes on x64 (0.792 s) and x86 (0.103 s), recorded
in `build/t563-s5-smiact-after-{x64,x86}.xml`.

The same Intel developer manual, section 9.3.4, states that recognizing
STPCLK stops prefetch, empties pipelines and write buffers, then generates
Stop Grant. It also states that HLDA prevents this because pending writes
cannot drain. This confirms a second barrier at the existing boundary after
higher-priority interrupt handling, before assigning and polling the Stop
Grant special cycle. It does not establish a fixed total latency or justify
applying a write-back-DX4-specific power model to SX.

Extend the same fixture with an actual store and 0/9/30 extra waits, crossed
with immediate acknowledgement, pending acknowledgement and provider I/O
failure. The actual special-cycle callback records the FIFO count and drain
subtotal. Pending completion is polled again and must not recharge the old
drain; failure follows the existing cancellation/CPU fault route, not a
successful Stop Grant. All cases keep the original write count at one.
The pre-fix regression fails as `stop-write-barrier` in
`build/t563-s5-stop-before-x64.xml`; after the single boundary drain call the
existing whole SMM test passes on x64 (0.731 s) and x86 (0.084 s), recorded
in `build/t563-s5-stop-after-{x64,x86}.xml`.

Current tracked C/H delta against accepted 54292e0bd: production +2/-0,
existing test +101/-0. Both source/test manifests are refreshed. The fixture
adds observations only; production adds no field, helper, public interface,
clock or memory path. Required full S5 qualification and eight artifact
refreshes have not run. No partial P or S acceptance is claimed.

Grouped affected-path regression then passes 9/9 per width: existing CPU
SMM, bus timing and fault/event tests; neutral Machine system-power test;
source/test manifests, corpus, test Types and test ownership gates. Actual
JUnit records are `build/t563-s5-barrier-regression-{x64,x86}.xml` (x64
3.08 s, x86 1.53 s total). The four executables were incrementally rebuilt
or relinked in the original reusable graphs; Ninja -j8, CTest -j4 and the
same persistent ccache directory were used. This is affected-path background
qualification, not full repository, external integration or desktop proof.

## Open Numerical Reconciliation

E1/E2 still need actual-access and failed/nested event subtotal allocation,
including Core fault-before-publication handling. Existing successful entry
tables do not prove these paths. E3's full 128-dword SMM image now uses the
explicit numerical decision below, not the exact silicon-footprint claim
rejected at the first checkpoint. No sparse image rewrite is applied.
E4's other special-cycle/cache boundaries, E5 qualified signal/reset/clock
proof and E6 event-only transport remain open with their original scope.
The two verified drains alone do not close any complete ledger cohort.

## SMM Numerical Model Decision

Retain the admitted complete 512-byte image and its single physical save/read
path. For selected SX, use an explicit L2 serialized envelope rather than
claiming the approximate Table 8-1 intervals describe that different footprint:

- Successful entry: prior write drain + 2 + 40 + max(139, actual save service).
- Successful RSM: max(236, actual restore service) + 4 + 20.
- Failed save: prior write drain + 2 + 40 + accepted save-prefix service;
  no successful-save floor or phantom retirement.

Actual service here excludes current-round external extra waits, whose sole
elapsed publication remains the existing Core external-cycle/pending owner.
RSM also excludes already observed instruction fetch service, which belongs
to the instruction fetch allocation. Previously queued writes must finish
before SMIACT; save writes must finish before beginning the SMM handler.
The same existing FIFO owner drains them without replaying memory operations.
The reference floor, serialized overlap and full reserved-image footprint are
L2 assumptions, not exact silicon/pipeline latency. RSM's pure nominal query
is staged for S6's sole selector; it is not a second selected runtime path.

Use existing CPU timing/instruction files and SMM/nominal tests. Initial
additional estimate: production +70--100/-5--15, tests +100--150/-5--15,
excluding manifests/evidence. No public ABI, persistent state, clock or new
functional state-save path is planned. Arithmetic failure uses the existing
CPU internal/provider fault mechanism, not an architectural guest exception.

## SMM Numerical Checkpoint And Core Budget Reconciliation

Actual SMI with the admitted full image costs 298 baseline clocks; current
extra waits remain separate. Successful RSM's pure L2 allocation is 280 with
128 noncacheable dword reads, versus the explicit 260 reference floor for
less service. It remains a pure S6 selector input, not an alternative runtime
selection. Tests execute buffered/unbuffered SMI followed by actual RSM with
0/9/30 waits; check all 128 accesses, FIFO completion, no duplicate memory
penalty, separate wait observations and query nonmutation. Failed save/read
positions 1/64/128 retain only the accepted prefix. Invalid CR0/CR4/SMBASE
images retain complete restore work plus the four-clock deassertion interval,
without normal restart, successful retirement or the successful-restore floor.
Pure arithmetic overflow/inconsistent-service rejection preserves output.

The sole event accumulator now checks addition and reports through existing
internal CPU fault handling. Two private owner-local helpers express checked
event addition and failed-restore phase accounting; neither forwards an
unchanged operation, stores state or creates a new execution path.

First numerical focused qualification passes SMM/nominal/system-power 3/3 per
width in `build/t563-s5-smm-envelope-{x64,x86}.xml`; failed-restore additions
then pass SMM/fault-event/system-power 3/3 per width in
`build/t563-s5-smm-failure-{x64,x86}.xml`. A broader X86 Core run initially
reports 165/166 on both widths, with only `machine/executor-run-smoke` failing
(`build/t563-s5-smm-core-{x64,x86}.xml`). Added named case diagnostics identify
the old zero-time SMI and reset-deadline assumptions. Update those assertions
to prove actual SMI time and deadline delivery, then re-arm a fresh deadline
to retain hard-reset scheduling preservation coverage. No budget is enlarged.

The same complete event path exposes a pending-event starvation mechanism:
an event longer than each tick budget is never partially consumed. Reuse the
existing nonretiring completion subtotal, publish the bounded portion through
the sole Core timeline, retain the remainder and update the reported elapsed
time. Instruction-retirement atomicity is unchanged. A real SMI completes in
100+100+98 slices, with zero instructions in each, one entry, no re-executed
save and a scheduled tick-150 event delivered during the second slice.

This internal fixture binds the SX CPU before production admission, as it
already did, and explicitly sets the fixture's compatibility-instruction
bound to one. The initial 386 construction's conservative bound otherwise
rejects the first 100-tick request before any SMI execution. That rejected
attempt is not evidence of pending-event starvation. After the explicit
fixture bound, the actual old mechanism reproduces zero progress with
pending=true/remaining=298 at PC 38000 in
`build/t563-s5-event-budget-reproduced-x86.xml`. The corrected executor and
system-power tests pass 2/2 per width in
`build/t563-s5-event-budget-verified-{x64,x86}.xml`. S6 still owns qualified
selected-SX bounds and the production-path proof; this fixture is not C17.

Current C/H checkpoint: production +116/-11 (net +105), tests +256/-6
(net +250), counted by tracked path with `git diff --numstat` against
54292e0bd; docs/manifests/artifacts excluded. No public ABI, production state,
Lib/Emulator, App, owner input or artifact changes. E1/E2 general entry and
failure allocation, Core fatal-prefix publication, other E4/E5 contexts and
the complete E1--E6 reconciliation remain open. No full S5 or whole-CPU
qualification, implementation P or artifact release is claimed.

The reconciled whole-component attempt then passes 165/166 per width; only
the prefetch-locality test still asserts the earlier atomic event-subtotal
rule (`build/t563-s5-smm-core-reconciled-{x64,x86}.xml`). The one failed
assertion expects zero progress for a 25-clock budget against a pending
26-clock entry. Update it to require 25 clocks now plus the final one next,
checking zero retirement, the exact pending remainder and final elapsed 27
including the preceding external wait. Ordinary instruction-retirement
atomicity, physical-publication rejection, overflow and older-family tests
are retained. No production change follows that whole-component attempt.

Current corrected executor/locality/system-power plus source/test manifests,
corpus, test Types and ownership checks pass 8/8 per width in
`build/t563-s5-event-budget-regression-{x64,x86}.xml` (3.68 s / 1.50 s).
This targeted rerun resolves that failure; it is not described as a new clean
full-suite run. Current C/H counts are production +116/-11 (net +105), tests
+263/-12 (net +251). Required final S5 whole-repository qualification and
eight artifacts remain unperformed, pending completion of the full cohort.

## Fatal CPU Prefix Delivery Decision

The CPU diagnostic callback currently captures the first fatal snapshot and
immediately marks Machine FAULTED. The run owner then returns before observing
the CPU's accepted event subtotal. This loses real saved/restored work.
Keep the existing historical first-fault snapshot; Core's existing
`fault_detail` owns the active cause. Let Core commit the lifecycle failure after the existing pending event/wait owner
has delivered its accepted elapsed prefix. A nonzero pending prefix may span
tick budgets, never retire or execute another CPU instruction. CPU's fatal
stop request must not be mistaken for an App stop/cold-reset while that prefix
is pending. Explicit outer stop/reset retains its existing preemption policy.
No new pending-fault field, enum, state mirror or public ABI is required.

Use the existing Machine diagnostic/run owners and executor SMM fault cases.
First change their unbounded elapsed assertions to the actual CPU source
allocation and reproduce the old zero-time return. Then cover finite 1-tick
budgets and a deadline during the prefix. Keep rollback snapshots, accepted
RAM writes, transaction cancel and teardown assertions. Estimated additional
production +35--55/-8--15, existing test +40--80/-5--15. This is Core time/fault
publication, not a new fault engine or recovery model.

## Fatal Prefix Implementation And Lifetime Checkpoint

The real save-failure regression first reproduces expected 44 baseline clocks
versus actual zero at transfer 2 in `build/t563-s5-fatal-prefix-before-x86.xml`.
The run owner now feeds the captured CPU event subtotal and Core external
waits into its existing nonretiring pending completion before committing the
fatal lifecycle. The fault commit helper owns status, lifecycle, trace and
result publication; its existing status-returning operation is checked.
No accepted instruction is fabricated and no CPU execution follows that
pending failure. Unbounded save/restore failures pass the executor/locality
pair in `build/t563-s5-fatal-prefix-after-{x64,x86}.xml`.

The same existing executor fixture covers failures at 2/127 of both saving
and restoring, crossed with unbounded delivery, one-tick budgets and reset
after the first pending tick. Each bounded slice returns no retirement, the
exact elapsed increment, and PAUSED/BUDGET until the last accepted tick;
only then does FAULTED/failure publish. A deadline in the prefix fires at
its exact guest tick. Repeating a completed fatal run adds no time, transfer
or cancellation. Original CPU rollback, accepted RAM writes, cancellation
count and exact SMM teardown remain asserted.

Self-review catches a lifetime distinction before delivery: the first-fault
snapshot is retained over processor reset and cannot be an active-fault
predicate. The existing `fault_detail` instead records the active CPU cause
even when history already exists. Processor reset clears that active cause
alongside the existing pending completion, without deleting history. Hard
and soft reset after one accepted prefix tick retain time/RAM/timeline,
cancel the rest, release SMM, preserve the old diagnostic, and then execute
NOP normally. No new pending field or hardware-state mirror is introduced.

The first complete X86 Core regression after moving lifecycle commit reports
165/166 on each width (`build/t563-s5-fatal-core-{x64,x86}.xml`). The remaining
transaction-lifecycle test directly invokes CPU refresh and therefore omits
the real Core failure-commit owner. Its port cases now enter via Core run,
including their prefetch-warming NOP. Success additionally checks the real
retirement/run-boundary trace pair; all original port transaction, external
cycle, side-effect failure and cancellation order assertions remain, without
a filter or compatibility path. Executor/transaction tests pass 2/2 per
width in `build/t563-s5-fatal-reset-{x64,x86}.xml`; later final assertions and
status checking are included in the next frozen regression.

Current tracked C/H counts: production +142/-15 (net +127), tests +327/-21
(net +306), against accepted 54292e0bd with docs/manifests/artifacts excluded.
General interrupt/exception access allocation and failed/nested CPU-prefix
costs remain unimplemented; this Core transport repair alone does not
qualify all fatal CPU work. E4/E5/E6 full disposition, S6 selection/preview,
C17 and final whole qualification remain open, with no partial P or release.

The final frozen CPU/Core fault-lifetime cohort passes the entire X86 Core
group 166/166 per width, including the retained older CPU families, FPU,
transaction, reset, timing, memory and static/manifests/Types/ownership tests.
`build/t563-s5-fatal-core-reconciled-x64.xml` records 46.17 s;
the matching x86 record reports 9.48 s. Both original graphs incrementally
build successfully with ccache/-j8 before CTest -j4. No source/test/manifest
edit occurs during those live runs. These are background component tests,
not full repository/external/desktop qualification or S5 closure.

The next E1 source review re-reads original Intel PRM chapter 9/Table 9-2.
It explicitly distinguishes completed-instruction debug traps from
next-instruction fetch faults and NMI/INTR priority. Do not blindly remove
prefetch from all external entry paths: debug-trap precedence is not evidence
that NMI has the same priority. Reconcile that actual boundary before generic
event access accounting; software INT must not acquire a duplicate event
base and failed/nested work still needs its original complete allocation.

## Completed-Debug Priority And Descriptor Read Ownership

The actual TF regression places a NOP at the final valid code byte, crossed
with 16/32-bit code and plain/66/67 forms. Before the change, the following
fetch produces GP instead of the already-pending DB
(`build/t563-s5-debug-priority-before-x86.xml`). For SX only, completed debug
trap delivery now begins at the existing execution boundary without fetching
the next instruction. This follows PRM Table 9-2; NMI/INTR prefetch behavior
and retained CPU-family behavior are not generalized from this debug rule.
Fault-event/debug/signal-prefetch tests pass 3/3 on each width in
`build/t563-s5-debug-priority-after-{x64,x86}.xml`.

Actual protected INTR regressions count IDT/CS/SS reads for same-level and
inner 16/32-bit gates plus VM86 entry. The old same-level 16-bit path reads
both gate and CS twice (`build/t563-s5-entry-reads-before-x86.xml`). The gate
dispatcher now passes its fetched descriptor to the selected handler; the
32-bit inner handler receives the CS descriptor already fetched by its
router. Existing code/stack preparation helpers consume their explicit
descriptor value instead of re-reading it. All nine code callers already
own the read; four stack callers gain the explicit read at its former helper
position, while the two call-gate callers retain their existing read.
Accessed-bit checks/writes, frame validation and architectural publication
remain in their original owners/order. No public interface, readiness flag,
parallel preparation path or new state is introduced.

The six related fault-event, far-transfer, outer-return, IRET and nominal
tests pass on both widths in
`build/t563-s5-entry-reads-after-{x64,x86}.xml`. A further call-site review
confirms the outer IRET CS read precedes preparation, as do the other eight
code callers. The wider helper change therefore also receives the full
retained-family X86 Core regression, not only the new INTR assertions.
Current production C/H +174/-53 (net +121), tests +425/-21 (net +404),
against accepted 54292e0bd; documents/manifests/artifacts excluded.
These are functional entry/priority corrections and observations, not the
still-open general event memory-cost or nested-failure allocation.

The subsequent frozen complete X86 Core regression passes 166/166 on both
widths: `build/t563-s5-entry-core-x64.xml` (63.37 s) and the matching x86
record (15.60 s). Both original graphs build the complete component test
target incrementally before running CTest -j4. Source/test manifests,
dependency, Types and ownership checks are included. No source/test/manifest
edits occur during either live qualification. No full-repository, App,
external desktop/RDP or refreshed-artifact qualification is claimed here;
S5 remains active without an implementation P.

## Software Interrupt Access Allocation

Re-read the original PRM PDF pages 825--826 (printed E-13/E-14) directly:
INT3 uses INT, INT n adds 4 in real/virtual or 0 in protected mode, taken
INTO adds 2; the INT miss columns are 2 real, 6 protected same-level, and
17 protected/VM86 inner entry. Task Gate retains its separate TS receiver.

Before the change, actual cold INT3 completes successfully but the existing
memory-penalty query rejects its one IVT miss and leaves the output sentinel
unchanged (`build/t563-s5-software-entry-memory-before-x86.xml`). Add the
missing CC/CD/CE case to the existing operand-cache owner, not a second
event parser. IVT contributes 2; protected IDT contributes 3, with CS/SS
remaining in the existing descriptor owner (3 each), and an inner TSS-line
miss contributes 8. Thus the complete reference cold envelopes match 2,
6 and 17 without duplicating descriptor or translation terms. The per-role
partial-residency distribution and additional misses remain explicit L2;
the complete cold match is not evidence of exact partial-hit silicon timing.

The existing fault-event test executes 180 contexts: INT3/INT n/taken INTO,
real, same-level 16/32-bit gates, inner 16/32-bit gates and VM86 inner entry;
each crosses cold/repeated execution with normal fills, uncached gate, CS,
TSS and all data. A small cache-query callback uses the fixture's explicit
uncached range; it does not inject cache contents. The first real-mode test
setup is corrected to enable caching rather than incorrectly expecting warm
hits with reset CR0.CD set. Its relocated IVT also keeps instruction and IVT
in distinct cache lines, as required by the reference assumptions.

Uncached TSS stack fields cause two accepted misses in the same physical
line, not one cached line fill: the explicit L2 model allocates the extra
five-clock miss and the tests expect 22 cold/13 TSS-only warm, rather than
claiming the 17-clock reference applies to uncached traffic. Full warm
reference cases return zero. Every case also checks nominal setup, actual
handler entry, no exception/fatal report, and zero additional event subtotal
for the successfully retired software INT. Task Gate, failed INT and
asynchronous entry are not qualified by these direct-query tests.

After a table-driven fixture cleanup, both original graphs build and the
eight affected CPU/state/manifest/corpus/ownership tests pass in
`build/t563-s5-software-entry-memory-final-{x64,x86}.xml` (4.22/3.96 s).
No source/test/manifest edit occurs during those jobs. This is a staged pure
memory-cost allocation: final selected runtime/preview integration remains
S6. The preceding 166/166 records predate this addition and are not claimed
as fresh full-component qualification. Current production C/H +195/-53
(net +142), tests +521/-21 (net +500), relative to accepted 54292e0bd;
this numerical subcohort adds production 21 and test 96 lines. No public
function/type/state, source file, framework or non-X86-Core code is added.

## Retired Instruction Followed By Fatal Entry

Reproduce another E6 transport distinction using actual NOP followed by NMI.
The attachment rejects the IVT read after the CPU has selected the completed
NOP's timing. The old Core fatal branch reports zero retirement and zero
ticks despite the accepted instruction (`build/t563-s5-retired-entry-fault-before-x86.xml`).
The CPU's nonzero selected instruction subtotal is the existing proof of a
completed instruction; no new completed/fault-phase flag is needed.

Only zero-instruction-subtotal delivery enters Core's nonretiring event path.
A completed instruction followed by fatal entry instead uses the single
existing immediate/delayed retirement path, and that owner commits the active
fault after publishing the accepted elapsed completion. Do not reserve fresh
prefetch after a provider fault. The pending owner, reset rules, source and
external-wait separation are retained; no second queue or lifecycle state
is introduced. This also avoids making an after-retirement fault into a
synthetic new retirement, or dropping the real prior retirement.

The existing executor test crosses zero/three-clock external fetch waits
with unbounded/one-tick budgets. It sums exact returned time and retirement
counts over slices, checks PAUSED/BUDGET before completion and FAULTED/error
at completion, preserves diagnostic EIP 0x101 (not the NOP's entry 0x100),
checks one rejected IVT access, and repeats the failed run to prove no
additional transfer, time or retirement. Attachment finalization is exactly
once. CR0.CD/NW are explicitly set in this fixture to make the injected IVT
transfer independent of cache-line fills. This is Core transport proof,
not qualification of the still-unallocated failed-entry CPU prefix itself.

Both original graphs build the normal and observable Core targets; the
eight executor/locality/retirement-observer/transaction/static tests pass
in `build/t563-s5-retired-entry-fault-cohort-{x64,x86}.xml` (5.10/0.74 s).
No source/test/manifest edit occurs during those jobs. This cohort adds
production +8/-2, tests +104/-0. Current S5 production C/H +203/-55 (net
+148), tests +625/-21 (net +604), excluding documents/manifests/artifacts.
No fresh full-repository, App or desktop qualification is claimed. S5 E1/E2
numerical access/failure allocation, the remaining system rows, S6 final
selection and C17 remain open; no implementation P or release is created.

## Accepted Failed-Entry Prefix And Nested Allocation

The existing bus completion owner now records the fastest-bus base of each
accepted physical read/write, excluding returned Core waits. It reuses the
existing read duration and write-fragment calculation; rejected/CANCEL
transactions do not add work. `timing_transfer_ticks` is a per-boundary
observation, not elapsed time, a new clock, FIFO service or a replay. The
second private observation, `timing_entry_transfer_ticks`, identifies the
accepted work already allocated by entry/fault handling. Both reset with
the existing execution boundary. There is no public layout/API change.

Failed instruction work is allocated before architectural exception entry.
Each interrupt/exception attempt takes local accepted/allocated snapshots.
Its completion claims only its own additional transfers, subtracting work
already claimed by a recursively entered handler. Successful entry retains
the original INT/TS base plus its separate setup; failed entry uses the
accepted-transfer prefix plus setup, never a fictitious successful INT/TS
latency. The failed-prefix rule is explicitly L2, not a claim of exact
pipeline failure latency. SMM's existing save/restore envelope marks its
accepted work allocated, so the common fault finalizer does not charge it
again. Core extra waits remain outside the CPU event subtotal.

Forty-eight new actual provider-failure contexts cross 80386/SX, initial GP
versus GP-to-DF, zero/nine extra waits and IDT/CS/four stack-write rejection
positions. An independent external-cycle callback sums committed physical
work and waits. SX's accepted prefix matches that sum exactly, with zero
retirement; 80386 timing remains unchanged. Rejected address/exception,
first fault and original cancellation/rollback tests remain asserted.

The first changed-path run identifies old zero-prefix assertions rather than
silently accepting them: `build/t563-s5-entry-prefix-first-x86.xml` reports
UD event 31 instead of 26 and completed-NOP/fatal-NMI total 10 instead of 1.
The former now retains the accepted 16-byte fetch baseline of 5. The latter
retains NMI setup 3 and three accepted two-clock frame writes before the
rejected IVT read, in addition to the one retired NOP. VM86 setup rows retain
the separate source base; their actual prefix is fetch 5, plus TSS permission
read 2 for I/O and a prior source read 2 for OUTS. The instruction owners are
inspected to confirm this order; the table is not merely adjusted to a new
aggregate. Successful software INT has no extra event base.

### One Pending Owner, Retirement Then Event

Once fatal entry has nonzero duration, a one-tick budget exposes a second
transport issue: the old pending retirement requires the entire instruction
plus event to fit atomically. The existing pending owner now publishes the
one selected instruction retirement first, then uses its existing nonretiring
branch to deliver the remaining event duration across budgets. It obtains
the event portion from the stable selected CPU result; no new production
field, phase enum or queue is needed. An outstanding completion may finish
after the instruction budget is met, but cannot execute another instruction.
Fatal lifecycle commits after the last accepted tick. Ordinary waits and
the existing full-round source observation retain their contract; final
physical qualification and observation reconciliation remain S6.

The first full component run is 164/166 per width
(`build/t563-s5-entry-prefix-core-{x64,x86}.xml`). The two remaining assertions
are reconciled at their owners. Prefetch locality now preserves fetch 5 plus
entry 26 after its separate one-clock board wait, delivering 30+1 event ticks
without retirement. The task32 test adds an external-cycle observation at
the real task-switch instruction: it captures the work before subsequent
GP entry and asserts exactly that prefix plus successful GP base 44. This
proves the failed recursive task #DB prefix is not charged twice and retains
all original incoming-task diagnostic/frame checks; it does not substitute
a hardcoded aggregate. Their targeted pair passes in
`build/t563-s5-entry-prefix-reconcile-x86.xml`.

After manifest refresh and source freeze, both original Ninja/ccache graphs
build the complete component test target. Reconciled X86 Core tests pass
166/166 per width in
`build/t563-s5-entry-prefix-core-reconciled-{x64,x86}.xml`: x64 55.13 s,
x86 13.88 s. No source/test/manifest edit occurs during live qualification.
This includes retained CPU families, FPU, CPU/Core lifetimes, transactions,
timing, memory, manifests, dependency/Types/ownership and negative gates.
It is background component qualification, not whole repository, external
desktop/RDP or refreshed EXE qualification.

This cohort changes production C/H +79/-14 and tests +121/-19. Cumulative
S5 against accepted 54292e0bd is production +282/-69 (net +213), tests
+746/-40 (net +706), excluding documents/manifests/artifacts. New production
state is limited to the two named private observation scalars; the existing
FIFO, Core clock and pending completion remain sole owners. No Lib/Emulator,
App, source import, public ABI, new source file or framework is introduced.
General successful asynchronous/Task Gate cache and translation allocation,
the full paging/failure matrix, remaining E4/E5 dispositions and S6/C17/final
qualification remain open. This checkpoint is not an implementation P or
S5 closure.

## Successful Ordinary Entry Access Scope

Do not copy the instruction access formulas into an asynchronous-event engine.
The existing memory calculator now shares one internal translation/alignment/
descriptor calculation and one ordinary-interrupt cache-role calculation.
The instruction wrapper retains its LTR exception and task-role coalescing;
nominal INT software instruction queries still use the cache-hit base only.
The existing private entry-cost seam now adds the scoped successful entry's
access terms and returns zero for inconsistent observations. The executor
rejects that inconsistency rather than silently dropping a necessary cost.
Task Gate still explicitly retains its nominal subtotal until its separate
memory-role receiver is finished; this does not widen SX production admission.

Each actual interrupt/exception attempt temporarily isolates only its access
counters. A small automatic `cpu_entry_access` value saves and clears those
counters on entry and restores the interrupted form's counters afterward.
Recursive handlers use the same bounded local scope. No cache, TLB, CPU
register, FIFO, bus-service or clock state is copied, reset or restored by
this scope. Accepted-transfer/failure allocation remains round-wide and is
not cleared. Therefore successful child-entry misses cannot also appear in
the parent instruction or Task memory calculation. No persistent production
field, public type/function or second timing path is added by this cohort.

The existing memory matrix is extended rather than duplicated: 180 actual
software INT contexts plus 60 actual INTR contexts cross real/same/inner/
VM86 entry, 16/32-bit gates, cold/repeated execution, normal fills and
explicit uncached IVT/IDT, CS, TSS or all-memory ranges. Software cases query
the staged instruction penalty; INTR cases check the already-published
instruction/event subtotals and actual acknowledgement. Both now consume the
same role coefficients. Non-filling inner TSS reads retain the explicit
22-clock L2 miss envelope rather than being relabelled as the 17-clock
reference. Existing common translation and nominal-instruction matrices
continue to pass; a dedicated whole paging-entry failure matrix remains open.

The first affected run (`build/t563-s5-entry-access-first-x86.xml`) catches
the omitted ordinary-entry miss terms in old assertions: real DB becomes
26+2, real UD becomes accepted fetch 5 + entry 26 + IVT miss 2, and VM86
entry gains its actual non-filling 22-clock access subtotal. The task
recursive-failure witness still retains its independently counted prefix;
its final GP now costs 44+6, not only 44. Core locality expectations retain
their separate board wait and the newly complete ordinary entry subtotal.
No functional frame, handler, retirement or rollback assertion is removed.
The affected four-test cohort passes in
`build/t563-s5-entry-access-second-x86.xml`.

The first complete component run reports 165/166 per width in
`build/t563-s5-entry-access-core-{x64,x86}.xml`. Its sole old REP/NMI assertion
expects 29, omitting the uncached IVT miss. The test now explicitly documents
and checks 26 entry + 3 NMI setup + 2 IVT miss = 31; real IRET and REP restart
checks remain. After manifest refresh, only that changed test needs a fresh
build; the complete frozen component inventory is then rerun on both original
graphs. All 166/166 tests pass per width in
`build/t563-s5-entry-access-core-reconciled-{x64,x86}.xml`: x64 7.34 s,
x86 7.73 s. Source/test/manifest files remain frozen during live jobs.

Actual cohort C/H delta is production +163/-30, tests +19/-2. This is less
test duplication than the initial estimate because one existing matrix
owns both software and hardware entry. Cumulative S5 against 54292e0bd is
production +445/-99 (net +346), tests +765/-42 (net +723), excluding
documents/manifests/artifacts. Private helpers have named roles: counter
scope lifetime, shared access terms, and the existing private full-entry
calculation. No new file, framework, public ABI, Lib/Emulator/App or artifact
change occurs. Complete Task Gate memory, dedicated paging/failure coverage,
remaining system rows, S6/C17 and final whole-repository/release qualification
remain required. No implementation P or S5 closure is claimed.

## Task Gate Access Reconciliation

The existing task memory model now owns both ordinary task transfer and
IDT Task Gate entry. E-14 gives Task Gate 37+TS with a three-clock miss
term; its TS16/32/VM86 terms retain the existing 143/162/140 nominal and
31/55/37 reference miss totals. IDT misses are distinct from TSS image
misses. The existing Gate/target-TSS cold-role coalescing is applied once,
whether the Gate originated in GDT/LDT or IDT. Partial residency remains
explicitly L2, not measured silicon timing.

One private per-round IDT miss counter records that physical role and joins
the existing automatic entry-access scope. No public ABI, cache copy, bus
owner or clock is introduced. The retained task matrix adds software INT,
actual acknowledged INTR and NMI Task Gates across 16/32-bit, VM86 and LDT
targets, cold/partial/warm roles and provider failures. It now contains
1,216 successful contexts and 252 failure contexts. Async cases check
published event ticks, not a decoder reused by entry; existing instruction
query-purity checks remain unchanged for their applicable forms.

Initial new-test failures identify fixture mistakes: querying an async
decoder as an original instruction, and placing NMI's Gate at vector 30h
instead of vector 2. Neither is repaired by weakening original functional
or timing assertions. Corrected targeted execution passes. After manifest
refresh, complete frozen X86 Core qualification passes 166/166 per width in
`build/t563-s5-task-entry-core-{x64,x86}.xml`: x64 59.76 s, x86 14.22 s.
This is background component proof, not a full-repository or desktop claim.

Cumulative tracked C/H delta against 54292e0bd is production +496/-123
(net +373), tests +860/-64 (net +796). The incremental cohort is production
+51/-24 and tests +95/-22. No App, Lib/Emulator or artifact changes occur.
Dedicated paging/failure vectors and remaining system dispositions stay open.

## Special-Cycle Service Decision

Re-reading 273021-001 sections 9.6/9.6.1 confirms that Stop Grant waits for
write-buffer completion and RDY/BRDY acknowledgement; HLDA cannot complete
that transition. Its latency depends on pending writes and memory service,
not a universal fixed constant. Qualified signal APIs do not model raw pin
synchronizers, five-clock STPCLK reassertion or PLL stabilization themselves.
No guessed fixed delay is added.

Current Stop Grant drains the sole FIFO before its output but drops that
service duration from event time. Move the drain to the existing special-cycle
poll owner so an emitted special bus cycle cannot overtake buffered bus work.
Charge only newly consumed stall service through the existing event subtotal;
polling an already empty FIFO cannot repeat the charge. HALT/shutdown bus
serialization is the existing explicit L2 resource model, not a measured
pipeline latency. SMM keeps its distinct state-image envelope.

Core must accept nonzero event-only work even when the outcome is system
waiting, using its sole pending-completion owner. Preserve WAITING at completed
system waits, zero retirement, bounded delivery, fatal cancellation and reset.
Estimated production +15--25/-3--8; tests +70--120/-0--5. No public interface,
new state or new source file is required. This is an E4/E6 cohort, not S5 closure.

### Special-Cycle Checkpoint

The sole special-cycle poll owner now drains the existing FIFO and adds only
the newly consumed service to event_ticks. Existing stop-specific draining is
removed. Invalid bus accounting takes the existing fatal path before emitting
an output. SMM retains its independently allocated state-image envelope.
Core sends event-only system-wait work through its existing pending owner and
retains WAITING when completed. No flag, second queue or public ABI is added.

CPU's existing barrier matrix now covers HALT/shutdown/Stop Grant, each with
0/9/30 waits and immediate/pending/failed output: 27 contexts. HALT preserves
its one real retirement; the two pure system entries have zero. The next poll
must charge zero. Core adds 18 real-store-to-Stop-Grant contexts crossing the
same waits, output outcomes and unlimited/one-tick budgets. Total service,
acknowledgement/cancellation, timeline publication, no retirement and no
repeated service are directly checked.

The first new Core expectation incorrectly omitted the already-serviced write
issue clock; source review of cpu_bus_write_complete reconciles it explicitly.
An initial HALT fixture invalidated prefetch after editing code and therefore
charged service to instruction fetch instead of the special cycle. The fixture
now supplies HALT before the real initial fetch. Neither correction changes
production coefficients or weakens a retained assertion.

Both original frozen graphs pass all 166/166 X86 Core tests in
`build/t563-s5-special-service-core-{x64,x86}.xml`: x64 60.98 s, x86 15.91 s.
Manifest, dependency/Types, negative and retained-family tests are included.
This cohort's production delta is +14/-1, tests +113/-0. Cumulative C/H
against 54292e0bd is production +510/-124 (net +386), tests +973/-64
(net +909), excluding documents/manifests/artifacts. No implementation P,
release refresh, desktop or whole-repository qualification is claimed.

Dedicated paging-entry/failure coverage and complete E1--E6 source/lifetime
reconciliation remain open, followed by the already assigned S6 selector and
preview/budget qualification, S7 C17 and S8 final whole qualification.

## Paged Entry And Failure Checkpoint

The existing CPU event fixture now maps IDT, GDT, TSS and entry stack to
separate pages. A real NOP at a distinct code address warms only the code
translation; no hand-built TLB state is installed. The matrix crosses
same/inner 16/32-bit and VM86, software INT, acknowledged INTR and NMI,
already-accessed pages, PTE A/D updates, and an initial PDE.A update.
Cold expected translation is independently 13 per walk, 21 per walk, or
28 for the first plus 21 for subsequent walks. Repetition proves hot TLB
and data reuse; actual IRET releases NMI service rather than changing its
private in-service flag. Valid outer descriptors make that return real.

There are 105 successful contexts: 90 cold/hot INT/INTR/NMI entries and
15 real fetch-PF deliveries. There are 270 provider-failure contexts covering
PDE/PTE reads, A/D writes, Gate/descriptor reads and the first frame write;
135 guest-fault contexts cover missing IDT/GDT and write-protected stacks,
with PF/DF escalation to shutdown. The independent COMMIT observer sums
only accepted physical transfers. Failed entries match that prefix plus
the appropriate setup, never a successful entry latency or rejected work.
Another shutdown round has zero new event/instruction time.

The successful PF cases also prove that a directory A-bit update can commit
before an absent PTE causes a fault. With initial PDE.A clear, its accepted
prefix is eight clocks, not four; subsequent entry walks see the already
accessed PDE and use the 21-clock row, not another 28-clock row. This retains
the S3 owner and its actual ordering rather than changing production to fit
an incorrect aggregate. New-test setup corrections also separate warmed code
from INT bytes, match a rejected byte inside a cache-line transfer, and provide
valid outer segments for IRET. No production paging change is required.

## Qualified FLUSH And Stop Grant Wake Model

Re-read primary 273021-001 PDF 268 / printed 9-40, section 9.6.4.5: deferred
FLUSH is presented internally for one clock on normal-state re-entry. The
qualified internal FLUSH action now adds one event clock; held FLUSH repeats
that one-clock action without retiring. Applying the same internal-operation
coefficient to normal-state FLUSH is an explicit L2 allocation, not a claim
to have measured raw pin recognition or complete external latency.
The existing timing selector already preserves a pre-instruction event
subtotal, so no alternate selector or copied event state is added.
Core preserves system-wait disposition through its existing consume-wait
operation; a pulse plus real NOP has one retirement and two total clocks,
including delivery across one-clock budgets.

PDF 265 / printed 9-37, section 9.6.4.2 gives approximately 10--20 clocks
from Stop Grant release to normal execution. Figure 9-12's ten-clock example
does not supersede that interval. Choose twenty as a declared L2 upper-end
model and charge it once on actual release, including return toward HALT.
It composes with actual NMI/SMI/INTR entry or the next instruction; it is not
a guessed PLL delay. CPU and Core public contracts already specify qualified
levels and that CLK=true means restarted and PLL-stable. Raw pulse widths,
two-clock input synchronizers, five-clock STPCLK reassertion and the board's
minimum 1-ms PLL stabilization remain caller preconditions, not host sleep,
a second clock, or fabricated CPU retirement.

SMM's retained power tests now check 20 plus actual NMI/SMI/INTR entry
subtotals. Core tests check write-service delivery followed by one real
instruction plus the 20-clock wake; two additional Core cases check held
and pulse FLUSH with unlimited/one-clock budgets. The existing provider,
special-cycle and reset/finalization owners remain unchanged.

After manifest refresh and source freeze, both original Ninja/ccache graphs
pass all 166/166 X86 Core tests in
`build/t563-s5-paging-power-core-{x64,x86}.xml`: x64 58.58 s, x86 15.56 s.
No edits occur during live qualification. These are background component
tests, including manifests/boundaries/negative/retained-family cases, not
whole repository, fresh external integration, RDP or artifact qualification.

This cohort is production +14/-1, tests +329/-2. Cumulative tracked C/H
against 54292e0bd is production +524/-125 (net +399), tests +1302/-66
(net +1236). The extra test size beyond the first local estimate includes
the successful PF and actual Core FLUSH transport proofs; no new framework,
public API, persistent production field, source file or queue is introduced.
The only new declarations are private forward declarations for existing
finalization/checked-event helpers; the old finalizer declaration is moved.
Lib/Emulator, Apps, INIs/media/ROMs, siblings and artifacts remain untouched.

Ordinary paged entry and failure proof is now present. S5 remains open for
Task Gate translation-role reconciliation, the complete special-cycle
latency disposition, E1--E6 source/lifetime/purity closure audit, and required
whole-repository/release qualification. No P or S5 closure is claimed;
S6's sole selected runtime/preview, C17 and final CPU qualification stay closed.

## Task Paging And Special-Cycle Completion Decision

Extend the existing task matrix, not a second fixture: real identity paging
and an incoming 32-bit CR3 equal to the outgoing value still require the
architectural translation lifetime. Code is warmed separately. A 16-bit task
uses two cold data-page walks; ordinary/LDT 32-bit tasks translate descriptors
again after loading CR3 and use three; VM86 task entry needs no subsequent
descriptor walk and uses two. With A/D already set, these add 13 clocks each.
All twelve software/INTR/NMI-by-target contexts pass. The existing physical
COMMIT seam also checks every injected Task Gate service-failure prefix with
and without paging. The retained matrix now contains 1,228 successful cases
and 324 provider failures; 144 IDT Task Gate failures directly compare event
time with independently accepted physical work and setup. No CPU production
change is needed for these translation/prefix witnesses.

Original 273021-001 section 10.3.1.1 (PDF 295 / printed 10-17) gives a
two-CLK fastest nonburst bus transaction; 10.3.11 (PDF 320 / printed 10-42)
requires RDY/BRDY acknowledgement of special cycles. The previous immediate
Stop Grant/shutdown notification had no base service at all. Add two clocks
at their acknowledged completion as an explicit serialized L2 model. Pending
polls and cancellation add no fictitious completed service. Real additional
waiting stays with the existing Core device/board timeline and is not charged
again by CPU polling.

HALT is deliberately different: the source HLT nominal row already owns its
zero-wait indication, and the declared 20-clock return-to-HALT envelope includes
reindication. Adding another base would duplicate those terms. This distinction
needs no new field, manager, bus path or special-case App policy. All physical
cycles still use the same poll/drain/acknowledgement owner.

The existing real write/Stop Grant Core matrix now verifies service plus the
two-clock acknowledgement, still sliced through the one pending owner. Invalid
RSM's source-qualified failed result gains the acknowledged shutdown service.
For an asynchronous failed entry, special-cycle polling already occurred in
that CPU round: its two shutdown clocks belong to the next round. Software
fault entry happens earlier and may commit the indication in the same round.
Tests assert both phases and then zero repeat work; they do not force different
event origins into one fabricated retirement. Targeted x86 tests pass in
`build/t563-s5-special-base-final-x86.xml`; full frozen qualification remains
required before this batch or S5 can be delivered.

## Frozen E1--E6 Source And Actual-Diff Review

Review is against accepted 54292e0bd, not against the preceding development
checkpoint. The counted tracked C/H surface is production +528/-125 (net
+403) in seven existing files, and tests +1361/-69 (net +1292) in nine existing
files. Manifests, documentation and executables are excluded. The larger test
increase contains real paging, accepted-prefix and bounded Core transport
matrices; no new fixture framework or production source file is introduced.

| Cohort | Implemented disposition and direct proof | Retained boundary |
| --- | --- | --- |
| E1 | One nominal/access decomposition serves software INT and asynchronous entry. Descriptor preparation consumes the already read value. Ordinary entry tests cover real, same/inner protected and VM86 paths, cache roles, 16/32-bit gates, paging and completed-debug priority. Task tests retain 1,228 successes and 324 failures, including paged software/INTR/NMI Task Gates. | Actual instruction selector and final observation/source-key reconciliation remain S6; no complete selected-SX claim. |
| E2 | One checked event accumulator and scoped accepted-transfer allocation preserve failed and nested prefixes. Independent COMMIT witnesses exclude rejected transfers; ordinary guest-fault chains and task-after-debug failure retain their actual setup and physical work. Core publishes real retired work before a later fatal entry, then faults only after accepted event time. | No successful-entry floor is charged for a failed entry; historical first-fault storage is not active lifecycle state. |
| E3 | Existing SMM save/restore path drains the sole write FIFO before SMIACT and before handler execution. Full-image L2 envelopes, waits, all original image failure positions, invalid RSM, pending SMI and restart contexts remain with their original owners. | Approximate/minimum source rows and the 512-byte admitted image are an explicit L2 envelope. RSM instruction selection is staged for S6. |
| E4 | All special outputs share the existing drain/poll/acknowledgement path. CPU tests cross HALT/shutdown/Stop Grant, waits and ready/pending/failure. Actual Core stores cross the same outcomes and bounded/unbounded delivery. Acknowledged non-HALT service adds the declared two-clock L2 base exactly once. | HALT nominal/reindication has a distinct source envelope and is not charged twice. No second bus or functional write replay. |
| E5 | Held/pulsed FLUSH, stop/restart, pending signals and RESET/SRESET retain the original E01--E20 functional qualification. New direct Core FLUSH and stop-release tests prove event clocks, real retirement and timeline advancement. Reset cancels active pending work while preserving documented memory/time/history lifetime. | Inputs are qualified levels; CLK=true means caller-completed PLL stabilization. Raw pin timing and the board's PLL delay are not CPU setters or host sleeps. |
| E6 | Existing pending completion slices nonretiring event time across budgets, preserves accepted waits, and prevents an additional opcode when an instruction budget is exhausted. Fatal/reset/overflow/deadline and older-family tests exercise the same Core timeline. | Safe selected-SX maximum bounds, preview/eligibility and final C16 aggregation remain S6; C17 construction stays closed. |

The same-issue sweep checks every event subtotal writer (the single checked
CPU accumulator), software/external/debug/exception/task entry, descriptor
preparation callers, SMM and special-cycle outputs, reset and Core pending
completion. The local entry scope temporarily isolates access counters only;
it never snapshots or restores architectural registers, cache/TLB contents,
FIFO state or a second clock. The three new private per-round fields record
accepted transfer work, its allocation and IDT miss provenance; existing
boundary/reset lifetime owns their clearing. Private helpers own checked
addition, entry scoping/allocation, SMM envelope/failure phases or shared access
formulas, rather than forwarding a second operation. No public ABI changed.

One concrete existing S6 receiver is retained: Machine's
`retirement_observation_capture_eligibility_key()` copies CPU observation
after asynchronous entry may reuse decoder context. S6 must reconcile the
already captured retirement and source key at the sole observation boundary;
S5 does not add a second callback, copied decoder or eligibility path.

## Frozen Qualification Progress (Not Acceptance)

The first frozen x64 complete repository run executes 632 cases and reports
631 passes, with `lib.audio/midi/native_midi_output` failing its native-create
assertion; JUnit is `build/t563-s5-frozen-x64-units.xml` (265.86 s). The same
unchanged executable then passes a host contrast and a restricted contrast,
both 1/1, in `build/t563-s5-x64-midi-{host,restricted-contrast}.xml`. This does
not establish sandbox causation, stable first-open behavior or an audio fix.
Lib, its tests, assertions, system services and host settings are unchanged.
The initial failure remains evidence rather than being overwritten by a pass.

The frozen x86 complete suite passes 632/632 in 79.78 s, recorded in
`build/t563-s5-frozen-x86-units.xml`. Both runs include all 166 X86 Core cases.
Both specialized-gate aggregates pass. Eight-artifact release, final selected
integrations and the required uninterrupted final x64 unit gate are still
being completed; no implementation P or S5 acceptance is claimed here.

The final uninterrupted x64 gate again reports 631/632, in 52.78 s, with
the same native MIDI create assertion as its only failure
(`build/t563-s5-final-x64-units.xml`). This is a repeated suite failure, not
an interrupted process or an accepted gate. Do not rerun the whole suite
until an evidence-backed diagnosis changes the experiment. Read-only caller
inspection finds only the native smoke opening the real MIDI output; other
MIDI tests use injected or unsupported backends. A shared endpoint collision
has therefore not been demonstrated. The assertion does not print the
returned Lib/native status, so IO_ERROR versus NO_MEMORY and its failing
native operation remain unproven. A Lib/test-Lib diagnostic or fix requires
separate review under this S's explicit stop condition; no assertion, native
capability classification, timeout, scheduler or host setting is changed.

The x64 selected boot matrix passes for all four profiles: default 2.77 s,
XT 31.14 s, AT 42.37 s and DeskPro 141.11 s. Actual-change review catches
three integration probes that had not been relinked by the initial selected
build command. Their earlier passes are not final proof. Rebuild the lifecycle,
CMOS and Model40 Console probes against frozen libraries, then rerun the two
relevant short cases per AT/DeskPro configuration; both groups pass 2/2 in
`build/t563-s5-x64-{my5170,mydeskpro386}-relinked-integration.xml`.
The sixteen unique selected scenarios now have fresh x64 proof; the redundant
pre-relink short results are retained only as development history. x86 release
commands explicitly include every probe before their first run.

## Release Proof And Interim Stop

The x86 four-profile selected groups pass 3/3, 3/3, 5/5 and 5/5. Boot matrix
times are default 3.94 s, XT 36.43 s, AT 57.79 s and DeskPro 171.56 s, with
unchanged assertions and budgets. JUnit files are
`build/t563-s5-x86-{nxvm,my5160,my5170,mydeskpro386}-integration.xml`.
Together with the fresh x64 proofs above, every one of the sixteen selected
scenarios passes per width. These are selected S5 regressions, not all external
integrations or complete guest Windows qualification.

All eight 0563 PC executables are rebuilt and deployed. Each final deployed
executable passes the existing isolated INI/CLI process harness, including
INFO/SPEED/help and wrong/missing adjacent configuration rejection. The
original graphs return to default; each default build EXE matches its deployed
hash. PE headers have the expected x64/x86 machine codes and `objdump -h`
shows no compiler debug sections. Final hashes/sizes are in
[artifact evidence](t563-s5-artifacts.json). Each asset directory retains its
existing pair only. INIs/media/snapshots/ROM originals, all MyNES files and
siblings remain unchanged. No App, Lib or Emulator source/test is modified.

Source/test manifest hash checks, both specialized aggregates, documentation
governance and whitespace checks pass. Both complete unit runs include three
passing automated desktop-labelled tests; manual desktop/RDP, physical MIDI
playback, full guest Windows GUI and PC110 verification were not performed.
The native MIDI create check is not a physical playback assertion.

S5 is **not accepted or closed**: the repeated x64 complete-unit failure is
still a required gate. There is no implementation P, push or clean-worktree
claim. The owned uncommitted implementation, documentation and candidate
artifacts are retained intact. A review of additional Lib/test-Lib diagnostic
scope is required before changing that explicitly excluded owner. No partial
commit, test waiver, downgraded assertion or repeated full-suite retry is used
to bypass the stop. S6 and C17 remain unadmitted/closed respectively.

## Read-Only Native Failure Localization

On resume, verify the current worktree and inspect the existing optimized
x64 native test rather than changing Lib/test-Lib. Its SHA-256 is
`912E98C308F82BE8C7621984AC669A3A259A957CC49C10B3DB69CD437A073941`.
Disassembly locates the return from `midiOutOpen` at
`lib_audio_midi_output_create+0x88`. A debugger breakpoint at that existing
instruction reads native EAX=1, completed-event handle=0xbc and cancellation
handle=0xc0. Allocation and both event creations succeeded; the failing
primitive is native `midiOutOpen`, which Lib correctly maps to IO_ERROR.
The unchanged test then reaches the same assertion and its inferior exits 3.
The debugger driver's exit 0 is not a test pass.

This isolated debugger run also fails, so the evidence no longer supports
even a suite-only or concurrency-only characterization. Earlier isolated
passes remain valid historical observations; their difference is unexplained.
A read-only WinMM query reports one output device, and
`midiOutGetErrorTextW(1)` succeeds with `Undefined external error.` Both
Audiosrv and AudioEndpointBuilder report Running. These facts identify the
native-open boundary, not its underlying driver/host cause; they do not prove
CPU causation, resource contention, a sandbox fault or a defective error map.

No test/source, binary, manifest, scheduler, system setting or service is
modified by this diagnostic. No MIDI message or playback is sent, and the
debugger process is terminal. There is no reason to add source-level logging
merely to rediscover this now-observed return. The remaining review decision
is an additional bounded Lib/native-open investigation or an explicitly
approved qualification disposition; the current packet authorizes neither a
Lib repair nor silently accepting this required test failure. All pending
S5 changes remain intact and uncommitted; full 486SX completion is unproven.

## Windows Session Restored And Final Qualification

The owner reports that the Windows session had been closed, withdraws the
request for additional Lib work, and requests the unchanged test be rerun.
No Lib/test-Lib, host setting, service, assertion or budget is modified.
First confirm no retained CTest/debugger/native-test process is running.
The existing x64 native MIDI test then passes 1/1 in 0.67 s, recorded in
`build/t563-s5-session-restored-x64-midi.xml`. The complete uninterrupted
x64 repository suite subsequently passes 632/632 in 57.49 s, including the
native MIDI case (0.51 s), all 166 X86 Core cases and three automated
desktop-labelled cases. JUnit is
`build/t563-s5-session-restored-x64-units.xml`.

The earlier failures and raw native error remain in the historical sections
above; they are not relabeled passes. Owner-reported session closure and the
unchanged successful rerun resolve this qualification stop without proving
that every generic WinMM error has that cause. No unrelated Lib repair or
permanent native-failure TODO is justified by this evidence. The earlier
provisional TODO is removed; no test is skipped or reclassified.

Final qualification is therefore complete for S5's admitted batch: 632/632
units per width, both specialized aggregates, sixteen unique selected
integrations per width, eight final published CLI checks and unchanged eight
artifact hashes. Both original build graphs remain selected to default, using
the persistent project ccache. Documentation governance, manifests and
whitespace checks pass. Actual C/H counts and the E1--E6 owner/source/model
review above remain unchanged. Manual desktop/RDP, physical playback, full
guest Windows GUI and PC110 qualification remain unperformed.

The interim stop and uncommitted-state statements above describe the earlier
checkpoint, not a competing current authority. This complete delivery is ready
for target-separated Shared/NXVM P commits and coordinator actual-commit
review. S5 acceptance does not admit S6 or C17, complete the selected runtime
recipe or claim the entire 486SX goal is achieved.

## Complete Target-Separated Delivery

Shared implementation P1 is `0b7c189e1`, immediately pushed to origin/master.
Its exact eighteen paths are seven production C/H files, nine existing test
C files and their two manifests. The complete changed source matches the
frozen qualification and the embedded code in the eight rebuilt candidates;
the artifacts were built before the commit from that same source content.
Committing does not change an executable input and requires no timestamp-only
rebuild. NXVM P2 delivers the complete packet, this source/model/test record,
artifact hashes, family history/index and eight PC EXEs, without another
target's files. Coordinator review must inspect both actual commits before
acceptance; the successful gate record alone is not that review.
