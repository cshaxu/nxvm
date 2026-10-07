# T546 S10 Exception Finalizer Design

## Admission And Complete Batch

Continue from accepted S9 15e2b7430, Shared 746e3e214 and NXVM 04bd419e1.
Current holds the owner-approved sixteen-field S10 packet. Existing CPU/Core/
IBMPC repair is automatically admitted; new public interfaces require review.
Lib/Common/MyNES, INIs/media/firmware and clock redesign are excluded.
No production change or P occurs at admission.

Consume the full T544 B01/B06 and linked family findings, not merely the first
failing DF fixture. Unit of proof is family, producer/form/mode, original and
secondary exception, frame return versus copied origin, commit boundary,
error/CR2, successful/failed service and shutdown/recovery/board response.

| Member | Required disposition and proof |
| --- | --- |
| Return point | Early DIV/IDIV/AAM type-0 next IP versus later restart IP, prefix lengths and register/memory/zero/overflow forms; traps/software/external return points and rejected entry retain their own origins. |
| Ordered pairs | Every documented benign/contributory/PF/DF first/second class by 286/386, including DE and vector 9 generation difference. Early families lack protected DF/PF; missing NPX-overrun production retains S16, not a falsely implemented opcode. |
| Serial service | Non-DF secondary events enter their valid handlers, retaining correct restart/error/CR2 and irreversible prior effects; do not restore the original fault then stop. Distinguish handler-entry faults from execution inside an already entered handler. |
| Error and failure | Exact DF zero, selector/IDT/EXT and PF error identity, repeated delivery failure and host-provider/internal failure; no fabricated guest exception or arbitrary compatibility success. |
| Shutdown | CPU owns resident shutdown separately from a consumed notification, ordinary HALT, product stop and reset; failed DF and source-qualified real-stack causes produce it. |
| Recovery and reception | No execution/retirement during shutdown; IRQ alone cannot wake it. Qualify NMI success/failure, reset and explicit DeskPro processor-reset response; generic Core cannot cold-reset every shutdown. |
| Late context | Preserve the incoming context after an actual published task transition and a subsequent entry fault; full task validation/publication order remains S13, not silently proven by this receiver. |

## Initial Source And Caller Inventory

Read T544 B01/B06 and the 386 exception-combination continuation, the existing
ExecFinal/real delivery/exception entry, all shutdown request/consume/reset
methods, Core run/diagnostic consumers and explicit DeskPro board binding.
Current source confirms incomplete 386-only contributory classification,
no serial secondary service and shutdown notification coupled to stop. Core
preserves the explicit DeskPro reset consumer, but its generic stop branch
cold-resets. Diagnostic record_fault also turns an architectural delivery
failure into Core product fault. These are whole-owner issues, not reasons to
patch individual BIOS or DIV call sites.

Fresh original 386 PDF220/9-16 Table 9-3 includes DE and vector 9 in contributory
exceptions; contributory/contributory, PF/contributory and PF/PF cause DF,
while the other named ordered classes can be serviced in succession. DF has
zero error and abort semantics; failed DF waits in shutdown for NMI or reset.
Fresh 286 PDF172/9-10 defines its different first-exception set (0/10/11/12/13),
requires a DF task gate, allows qualified NMI recovery without leaving PE,
and says failed shutdown-NMI service requires reset. Original pages must be
visually reviewed and remaining return/error/context sources reconciled before
selecting production changes. No exact source value is downgraded.

The candidate keeps those two tables distinct rather than flattening them into
one broad predicate. `cpu_execution_fault_event_smoke` enumerates every listed
first/second pair for all retained CPU profiles: 80286 asserts its documented
first-exception row (with no page-fault row), while 80386 asserts the ordered
three-class Table 9-3 matrix. The fresh x64 owner test passes; this audit found
no additional classification gap or timing downgrade.

The existing one-shot shutdown notification is insufficient by itself for
Core to distinguish resident shutdown from ordinary HALT across later runs.
Inventory existing public copied observation/query contracts before proposing
the smallest indispensable public change; do not silently add a field/function
or mirror the CPU latch in Core. This does not block independent source and
ordered-pair/return proof. No source/framework or public extension is admitted
by this evidence record.

## Source Review And Direct Before-repair Probes

Read-only PDF review visually confirms original 386 PDF220/9-16 and 286
PDF172/9-10; ignored renders are retained in build/t546-s10-research. Original
386 PDF283/284 (14-5/14-6) distinguishes early divide's following-IP from later
restart IP. The 186 hardware volume does not settle its precise return point
in the inspected interrupt section; read-only PCjs x86help.js helpDIVOverflow
selects trap through 8088 and fault from 186. Preserve that existing 186 row
with explicit L2 model basis, not invented Manual-L3 or guessed cycle cost.

An existing CPU-owned true-storage fixture executes 176 DIV/IDIV contexts:
all five profiles, byte/word register/memory, zero/positive overflow, segment
prefix, and 386 dword forms. It inspects actual delivered vector/frame and
copied fault origin without asserting undefined arithmetic results. On both
host widths, 80 contexts fail. Sixty-four early rows save the original IP
instead of the documented next IP; sixteen 386 dword rows also expose a
frame-size dependency on the faulting instruction's operand override.
Reconcile exception-entry width independently of explicit software INT width;
do not change the expected frame to copy this current behavior. Original 386
14-3 and read-only Bochs real_mode_int independently describe word IP frames.
Full protected gate/return geometry still retains S12.

An actual protected 386 GP-to-PF probe maps real page tables, leaves the GP
target's descriptor page absent, and supplies a separately reachable PF gate/
descriptor/stack. Current code stops with original GP, loses secondary CR2
and never enters that valid PF handler. This is not a missing-IDT fixture or
a synthetic successful-memory provider. The primary/secondary copy and serial
delivery boundary require one owning repair.

A 286 probe installs a real available 16-bit TSS/task gate for DF, current
busy TSS and valid incoming code/stack selectors. Divide error plus rejected
first gate stops as original DE rather than entering DF; backlink remains
unset. The eventual repair must also prove the task-handler zero error push
and proper actual publication context, not substitute a 386 interrupt gate
for the 286 manual's task-gate requirement.

All three probes execute together on both widths after strict fresh target
builds and reproduce the results above. The first attempted new compile failed
on a signedness warning; the following old-EXE pass was stale and is not
evidence. Fix that test expression, remove its unused form field, enforce
build-success before execution, and use only the fresh failing results. No
production, API, artifact, INI, master, Lib/Common or MyNES change occurs.
Test manifest follows these exact S10 probe bytes; no passing unit/full-suite
qualification or partial P is claimed. All handles are terminal.

## Historical Minimal Shutdown-query Review

The owner review request is exactly one public read-only function:
core_machine_cpu_is_shutdown(const core_machine_cpu_execution_context *)
returning lib_bool. It exposes no layout, handle or mutable pointer and adds
no public type, snapshot field or Core stop enum. CPU remains the resident
state owner; the existing consume function clears notification only. Core
can use its existing waiting result plus SHUTDOWN detail and preserve the
explicit DeskPro processor-reset consumer. This was the admission-era request
only: the owner later approved this exact query on 2026-10-07, and the
candidate implements it without a state mirror, layout exposure, or mutable
path. Independent source, return and ordered-pair work remains separately
qualified.

## Fixture-validity Controls

Add two positive controls to the same code-owned fixtures: software INT 8
successfully enters the exact 286 TSS/task gate with a valid backlink and
without touching a sentinel below its new stack; mapping the GP descriptor
page successfully enters the exact protected GP gate and preserves CR2 zero.
Both controls pass on both host widths while the corresponding architectural
DF/serial cases still fail. This independently excludes invalid task tables,
descriptors, page mappings or delivery stacks as explanations for the failing
probes. No production change is selected from a missing-handler artifact.

Current frozen probe body hash is
E397732474615B7D9EC9658143EBA475ED1F1A4240024E676C8A5995BB3866E4.
All 180 new contexts execute in one existing target per width: 176 divide
rows, two task-gate and two mapped/absent-page contexts. Eighty divide plus
two delivery cases fail identically. These are before-repair results, not a
passed unit package. The complete seven-member source/caller grid, including
AAM, error origins, shutdown recovery and actual late-task checkpoint, remains
open before cohesive production repair and any P.

## Selected Existing-owner Scheme

Reconcile the full member grid into one private architectural vector selector
and ordered-pair rule at the existing finalizer. Protected 286 uses its first
contributory set; 386 adds vector 9 and its asymmetric PF pairs. Internal/
provider failures never enter that guest classifier. One delivery loop retains
secondary mask/code and CR2 rather than restoring the first fault; valid entry
returns to the next Core round with no successful retirement. Actual entry
failures are contributory/PF, so the source dispositions converge to valid
service, DF or shutdown without an arbitrary retry budget.

Real architectural exceptions use word return frames independent of the
faulting instruction's operand override. Early DE changes the frame return IP,
not the copied source/rollback image. Explicit software-INT width and full
protected gate geometry retain their separate S12 source review. Task-gate
error frames use the incoming TSS width, not the initiating instruction width.

An actual task publication records one private restart checkpoint, validated
by the existing instruction_task_switched flag. This is a distinct incoming
failure boundary, not mirrored live CPU state or a new transition path.
Preserve original instruction oldcpu for its original observation/timing
contract; use the new checkpoint only when a published task's later entry
fails. Copied diagnostics must identify that actual fault context, not attach
outgoing instruction bytes to the incoming task's PC. Full producer staging
and paging/selector semantics remain S13/S14, explicitly not qualified here.

At this design checkpoint the shutdown query remained unapproved and
unimplemented. Notification/stop behavior was therefore retained until
resident state and Core recovery could be integrated together; this historical
residual prevented acceptance at that time. The later approved repair is
recorded by the current-result section; no partial P or acceptance follows
from this early design record alone.

## Developing Candidate And Direct Results

Replace the repeated protected/special-real branches with one vector selector,
one ordered delivery loop and the private source-qualified pair rule. Remove
the superseded contributory predicate and separate real final-delivery owner.
Secondary mask/code/CR2 remain active instead of reverting to the first fault.
The original opcode tables are unchanged; early DE's return image uses the
actual decoded next IP while copied origin and rollback stay at the original
instruction. Word exception frames no longer inherit prefix66 from DIV/IDIV.

Actual task commit captures a private incoming restart checkpoint using the
existing validity flag. Error pushes use the incoming TSS width, and a failed
post-publication trap is handled in that actual new task. Copied diagnostics
identify that context and omit outgoing bytes when its PC/TR/CR3 differ.
They do not overwrite the original instruction observation/timing checkpoint.
This qualifies the actual published-context receiver only; full producer
staging/late-selector/page contexts still retain S13/S14.

Fresh visual 386 PDF216/9-12 Figure 9-6 confirms task-handler error pushes
and selector/IDT/EXT fields; PDF225/9-21 confirms incoming late-task context.
Private wrappers mark EXT only on selector-family errors during hardware or
exception entry, not software INT or PF error codes. A real-frame software
INT/NMI/IRQ matrix proves distinct 102h/13h/183h codes and admitted ACK counts.
DF still carries zero. Correct the misleading synchronous-only helper comment
and vector-9 comment; remove the unused later-CPU alignment-check error row
without adding that unsupported generation or changing a public value/API.

All 180 original new probes now pass. Add 1,096 independent table-driven
architectural pair decisions: 286's first-only contributor rule, 386's full
asymmetric class table and early non-applicability controls. The private rule
is tested at its owning CPU boundary; host/internal masks are filtered before
calling it. This classifier coverage is not fake NPX vector-9 production.

Twenty AAM controls preserve documented D4/0A decimal behavior and the existing
D4/00 divide-error model. Fresh visual PDF338/17-20 specifies only base ten
and no exception for that form; read-only PCjs opAAM/helpDIVOverflow explicitly
models the other immediate and its generation return point. D4/00 remains
reference-model L2, not newly claimed Manual-L3. No exact instruction clock
row or current accuracy tag is downgraded.

A new actual post-publication task-trap failure enters a valid GP handler in
the incoming task, with new PC/register/stack, retained BT and completed old
TSS save. It proves no restoration of the outgoing task or false attachment
of its instruction bytes. Existing debugger/task/IDT/VM86 regression bodies
execute on the candidate. Three initial receiving failures are false original
exception oracles: absent UD/NP/SS handlers require serial/DF processing, not
the previous immediate stop. Keep their register/cache/TR/stack assertions;
correct only source-proven final exception/code, including UD-entry GP 33h.
The first late-context test build used a CPU-C-file-only BT macro; use its
documented bit value like the surrounding owner tests, then rebuild before
counting its successful execution.

Strict changed-owner builds and six bounded development executions pass per
host width. They are not complete unit/Core/board/product acceptance. Current
production is net -64 lines in three CPU paths; one temporal restart image,
not mirrored runtime state, is the only new checkpoint. Source/test manifests
follow the exact candidate; no public function/type/enum value changes. At
this historical checkpoint the shutdown query was not implemented, the old
notification/stop behavior was still residual, and no S10 P was eligible.
Later candidate work resolved that owner-approved query and receiver boundary;
the remaining source qualification is recorded in the current result below.
Eight accepted S9 EXEs, INIs, masters, Lib/Common/MyNES remain unchanged; all
current tool handles are terminal.

## Actual PF-first And Provider-failure Reception

Extend the one existing real-page-table fixture to actual PF-to-GP and PF-to-PF
entry failures with a separately reachable DF gate/stack. Both enter DF with
zero error. CR2 remains the original data address for PF/GP and the secondary
descriptor address for PF/PF. The first extension accidentally retained a null
DS, so its store raised GP instead of the intended PF; do not qualify that
result. Declare the actual valid data selector before executing the store,
then retain only the corrected passing results. Existing mapped-GP and GP/PF
serial controls still pass.

An owner-local provider seam rejects either the first exception's IDT read or
the subsequent DF IDT read. Both report internal CE and the existing physical-
address diagnostic, with no fabricated guest DF or shutdown. Do not confuse
this memory diagnostic with the different ACK status detail; the initial
expected I/O status was corrected after inspecting the existing physical
transfer contract. No production error shape or public field is changed.

The same parameterized seam rejects the second frame write for a normal or
DF handler after an earlier write was accepted. Both report CE without guest
shutdown and retain the already-written FLAGS bytes. The total write count
also includes descriptor accessed-bit publication, so prove the actual saved
bytes and nonzero accepted writes rather than asserting a fake total of one.
There is no universal memory undo, duplicate file/memory backend or test-only
production API.

The current extended owner body passes after strict fresh builds on x64 and
x86. Its manifest records hash
82173105FDB264225A2A070AD803C4CBD1FF24D18C1AD0E9ECDC32EF6D31492D.
Only code-owned tests/documentation advance in this round; production candidate
and accepted S9 artifacts/INIs/masters/Lib/Common/MyNES are unchanged. Resident
shutdown, its public-query review and complete Core/board/final qualification
remain pending; no S10 P, closure or complete-CPU claim. All handles terminal.

## Approved Resident Shutdown Candidate

On 2026-10-07 the owner explicitly approves the one read-only shutdown query.
CPU owns one private NONE/WAITING/RESET_ONLY state, distinct from its consumable
board notification. Failed DF publishes shutdown rather than a product fault
or stop. Refresh in shutdown executes no instruction, ignores IRQ/debug wake,
and admits NMI; successful NMI entry clears shutdown, while failed 286 entry
requires reset. Provider failures still take the internal-error path. Core
uses its existing waiting result with SHUTDOWN detail, preserves guest state
and time, and retains only the explicit board processor-reset binding.

Five changed-owner development cases pass on x64 and x86 after fresh builds.
The recovery fixture proves notification consumption does not clear state,
IRQ does not acknowledge or retire, successful real NMI entry recovers, and
processor reset clears both resident state and notification. The actual Core
receiver additionally proves two unchanged shutdown waits and the existing
processor-reset request on x64. Protected/failed-NMI recovery, all remaining
receivers and final qualification still require proof; no S10 P or closure.

The real Core GP/DF receiver now causes an actual segment-overrun GP, not an
out-of-range software INT. A separately reachable DF handler proves zero code,
word return frame, preserved source IP/flags and zero retirement/time. Missing
real IDT entries follow the original 386 14-3 double-fault rule. Existing RF,
cache and task-register assertions remain; unserviceable DF now asserts chip
shutdown rather than the retired product-fault behavior.

A transient test expression initially matched no tests and is not evidence;
the successful executions use the actual registered names. One own ccache
launch stalled in the restricted shell. Read-only full process-tree inspection
identified only this S10 build before stopping it and rebuilding the same
targets with the permitted execution route. No unrelated process or cache
was removed. MyNES, Lib/Common, deployed S9 EXEs, owner INIs and media remain
untouched. Manifests follow the current candidate, not a committed baseline.

The subsequent Core repeat-wait/reset receiver also passes x86. Additional
real-mode tests cover unsuccessful shutdown-NMI service, 286 RESET-only
recovery, and repeated 386 NMI entry. Review caught a Core retirement hazard:
successful shutdown-NMI entry outside an instruction must set the existing
non-retirement delivery outcome directly, rather than call its instruction-
only helper. The CPU-owner case passes after that correction on x64 (0.14 s)
and x86 (0.48 s), each after a fresh strict build. No complete suite
or protected-mode/NMI claim follows from these bounded cases.

## Protected Recovery And Strong Core Receivers

Fresh visual original 286 9-10 and 386 9-16 confirm the recovery distinction.
Four code-owned true-storage cases use actual GDT/IDT descriptors and proper
16/32-bit NMI gates: successful protected NMI keeps PE and commits its frame
without instruction retirement; unsuccessful 286 entry permits only RESET;
reset clears shutdown and PE. The existing CPU-owner case passes on both widths.
The absent NPX-overrun producer remains S16; its abort must not gain the fault-
image RF bit merely because it shares exception entry with faults.

The three 286 negative Core cases now install independently valid handlers.
They still prove exact GP/NP/SS error codes, not merely final shutdown. The
handler needs a separate present code descriptor at selector 18h, so the
invalid-selector input moves from 18h to 28h, still beyond the source-proven
GDT limit. Its exact error code follows that input; copied original CS/IP and
the actual handler CS/base/IP are checked independently. The real-mode 286
rejects-386 case uses a real UD vector, preserving its original semantic test.
Existing positive stack/task/LIDT/IDT assertions are not removed.

Core's copied-diagnostic case now runs the identical NOP/UD sequence with a
valid UD table and an unavailable UD/DF table. Both preserve source PC, opcode
bytes, zero development history and the successful-NOP retirement count;
the first returns delivered UD, the second resident shutdown, never a fake
host fault. These two Core receivers pass x64 and x86. All are development
proof; complete suites, board consumers, gates and artifacts remain pending.

## Complete Receiving Failure Baseline And Early-chip Convergence

The first complete x64 execution finishes, not a timeout of observation:
379/505 pass, 126 fail, elapsed 345.58 s. Its full log remains ignored at
build/t546-s10-unit-x64.log. This is a failed candidate, not qualification.
The receiving batch includes missing-handler shutdown versus product-fault
oracles, injected host IVT failures mislabeled as guest UD, and CPU-private
negative setups/loops which assume every exception requests stop. No failing
row is removed, no timeout is increased, and final acceptance remains blocked.

Inspection also identifies a real classifier convergence defect: an early
chip's private impossible PE image can repeatedly raise UD while attempting
nonexistent protected delivery; a fake early IDTR limit can similarly recur
through GP. Original 386 14-3/14-7 distinguishes its programmable IDTR and
invalid-opcode exception from 8086. Keep early strict unsupported-encoding
diagnostics distinct from real hardware vectors. Pre-286 fixed IVT accesses
retain the original logical/linear/bus recording path, but ignore nonexistent
IDTR base/limit; no alternate memory backend or retry bound is introduced.
The selector excludes impossible early protected delivery and unavailable
vectors instead of fabricating guest faults in those configurations.

Three identical CPU real-UD rejection fixtures now share one code-owned
valid-handler proof: inspect delivered UD, original CS/IP, actual return frame
and unchanged CPU fields except documented entry effects. Do not shorten IDT
to manufacture a terminal first-UD outcome. All original encoding forms stay.
DTTR, LGDT/LIDT and SGDT/SIDT change from timeout/false oracle to passing x64
executions alongside the existing finalizer case (4/4, 0.41 s). A subsequent
explicit early fixed-IVT regression and x86 replication are tracked separately
before counting them. Remaining complete receiving rows still require repair.

The new three-family fixed-IVT case passes after a fresh x64 build (0.10 s),
and the four-owner replication passes x86 (4/4, 0.98 s). IDTR system-table
translation retains the original reference/bus path and ignores nonexistent
base/limit only before 286. No exact instruction clock changes. This fixes
the CPU's repeated impossible-delivery cycle, not just a test timeout.

The T359 S5 neutral Core case explicitly installs a host read failure at IVT
vector 6. Its first fault is therefore CE at physical address 18h, not guest
UD. The receiving oracle now checks that exact classification/address while
retaining zero retirement, source/device time and budget assertions; its fresh
x64 execution passes. Other callers of the same host failure fixture remain
in the complete receiving sweep, with early strict nonarchitectural diagnostics
kept distinct rather than mechanically replacing every UD assertion.

## Receiving Reconciliation Progress

Three additional host-IVT readers (CPU profile gate, board control state and
FPU-interface rejection) now prove exact CE/address 18h from the injected
read failure. Early strict unsupported encodings retain their separate UD
diagnostic. Register/FPU/function assertions remain; no injected host failure
is reclassified as guest DF or silently successful delivery.

The shared CPU rejection proof accepts a seeded fixture so FLAGS cases retain
their original register values. It checks the delivered source CS/IP/EFLAGS,
actual frame CS/IP/FLAGS and full CPU-state equality after accounting only for
documented entry effects; it neither restores live CPU state nor fabricates a
first-fault snapshot. The name is now expect_ud_rejection, explicitly including
early nonarchitectural strict diagnostics. Direct FLAGS and LAHF/SAHF use this
same proof; the table-form cases keep every original form. BOUND additionally
retains its unchanged bounds memory; FPU keeps genuine unsupported-capability
failures distinct from UD and preserves its object lifetime.

Five protected string-board receivers had no serviceable handler and already
expected final DF, not a directly observed first GP. They now assert resident
shutdown through Core's waiting result and copied terminal observation, rather
than a product fault. Every prior element/index/count, memory effect and IRQ
check remains. No test is deleted, timeout increased, or compatibility path added.

The fourteen repaired receiving executions pass x86 after fresh strict builds
(14/14, 9.80 s); their x64 groups pass as recorded in the tool results. BOUND
and FPU CPU cases also pass x64 (2/2, 0.21 s); the final renamed-helper/x86
replication is tracked before counting its result. These are development
proofs only. The 126-failure full baseline remains preserved, the remaining
receiving batch is open, and no S10 implementation P or product refresh occurs.

The final renamed-helper replication passes x86 after strict builds (7/7,
1.72 s), including BOUND/FPU and the five table/FLAGS receivers. No existing
memory or full seeded CPU comparison is deleted. All handles for this receiving
increment are terminal; no full-unit pass is claimed from these results.

## Real New-CS Entry And Stack Reception

A fresh original 386 14-3 visual review and the accepted real-cache reload
owner expose a production entry defect: real INT tested the word handler IP
against the interrupted CS limit, although that target belongs to the reloaded
CS. The direct before-repair case fails on 286 software INT with old limit 7Fh
and a valid handler at 100h, spuriously entering GP at zero instead. Remove
only that old-CS check; the existing CS-load owner and word target remain.
Four 286/386 software-INT/GP-handler cases then pass along with the complete
CPU exception-event body. No opcode table, clock row, memory backend or public
contract changes. The ignored original render is 386-281.png.

The owner-local real fault proof is now parameterized for UD/GP rather than
duplicating entry logic. It checks original copied CS/IP/FLAGS, zero real error
code, actual word frame and the complete CPU image after explicitly accounting
for documented CS reload, IP, SP and live FLAGS changes. Near CALL/JMP and
conditional/LOOP target failures use real GP handlers; early strict diagnostics
remain distinct. Five corresponding x64 development cases pass (5/5, 0.52 s).

Stack negative cases need an additional irreversible-effect proof: the valid
handler itself writes over part of the original stack sentinel. The test
provider therefore counts accepted writes, and the real fault proof requires
exactly its three word-frame writes, or zero for early strict rejection. An
illegal PUSH cannot hide behind a later overwriting frame. Sentinel assertions
retain the unaffected input and explicitly check the documented frame where
it legitimately replaces that stack memory. This is test observation only,
not production trace, rollback or a synthetic first-fault snapshot.

PUSH/POP and PUSH-immediate keep all positive forms, early wrap, alias/failure,
protected stack limits and unchanged-register/data checks. Actual shutdown is
asserted through the approved resident query with no stop or internal fault;
UD goes through its genuine handler. Both complete x64 owner bodies pass after
strict builds. The twelve-case x86 replication is tracked before counting its
result; remaining receiving reconciliations and final qualification stay open.

The current twelve CPU receiving executions pass x86 after strict fresh builds
(12/12, 5.06 s). All source/frame/counter assertions execute; no test timeout or
registered case is removed. The production mechanism stays at the sole real
INT entry and cache-load owners; successful handler admission does not preview
or require its subsequent executable-byte fetch. A fresh x64 replication of
the same frozen helper/source is tracked separately before counting its result.

Fresh x64 replication passes the same twelve owner bodies (12/12, 0.59 s).
Both widths therefore execute the frozen new-CS/frame/write-count proof, not
an old executable with changed test source. Complete qualification remains
unproven until the full remaining receiving batch and required suites finish.

## String And Port CPU Reception

Five CPU string-owner bodies retain every normal form, direction/address-size,
IRQ/repeat, completed-element index/count/FLAGS and memory assertion. Their
UD setup now uses the same real handler and full-state/frame/write-count proof;
their absent protected handlers produce resident shutdown, not first-fault DF
or stop. The obsolete CMPS all-GPR helper has no remaining call and is removed
after strict compilation identifies it; warnings and timeout remain unchanged.
The five full bodies pass x64 (5/5, 0.52 s) and x86 (5/5, 0.96 s).

Scalar/string port bodies preserve transfer and completion counts, unchanged
data, host-provider errors and accepted prior REP I/O. UD performs no port I/O;
VM86/TSS permission failure with unavailable handlers is shutdown without stop.
The first scalar attempt still fails because its second VM86 DF oracle was
not migrated; reconcile that same class before counting the passing rebuild.
Both complete x64 port bodies now pass. The seven-case x86 replication is
tracked separately before its result is qualified. No production, API, clock,
Lib/Common/MyNES, INI, master or product EXE changes occur in this increment.

The current seven complete string/port owner executions pass x86 after fresh
strict builds (7/7, 1.51 s). All handles are terminal. The complete failed
qualification baseline is retained; this increment does not close S10 or T546.

## Arithmetic Reception And Current CPU-owner Sweep

Bit scan/test, double shift and both IMUL owners retain their full normal,
alias, undefined-result, overflow and FLAGS matrices. UD observes a valid
handler and complete CPU/frame proof; operand probes still require zero operand
reads/writes. The IMUL synthetic invalid-stack case explicitly observes resident
shutdown without stop. Five complete x64 bodies pass (5/5, 0.50 s), and x86
replication passes (5/5, 2.34 s). A mis-targeted loop-variable removal causes a
strict compile failure first; restore the count-zero variable and remove only
the retired profile-negative variable before counting any execution. No old
EXE pass or relaxed compiler warning is accepted.

SIGN-EXT/SETcc/FS-GS keep every result/flag/segment case and full seeded state.
The same real rejection proof replaces false terminal UD; absent stack handlers
observe shutdown, not product stop. The dead SIGN-EXT comparison helper has no
remaining calls and is removed; positive nonparticipant comparisons stay.
All three complete x64 bodies pass (3/3, 0.28 s); x86 replication is tracked
before counting it. This increment changes tests only, not instruction clocks,
API, Lib/Common/MyNES or existing products/inputs.

The active packet now names a transient executable-owner scan derived from
actual CTest commands, not a new fixed suite. Its first fresh x64 build executed
all 78 CPU owner cases: 41 pass, 37 fail, elapsed 4.59 s, no timeout. The full
log is ignored at build/t546-s10-cpu-owner-x64-reconciliation.log. This provides
the original remaining failure batch after the earlier nonconvergence fix; it
does not replace the original failed 505-case baseline or prove complete units.
Remaining stacks/transfers/queries/decoder/VM86/paging/FLAGS receivers require
their own source-qualified disposition before S10 implementation delivery.

After the owner-local LES/LDS, LSS/LFS/LGS, SREG MOV, MOFFS, XCHG, GPR MOV,
legacy LOCK, PUSHF/POPF, PUSHA/POPA and ENTER/LEAVE receiver repairs, the same
fresh x64 selection executes 51/78 cases: 51 pass, 27 fail, elapsed 1.01 s, no
timeout. The ignored second log is
build/t546-s10-cpu-owner-x64-rescan-2.log. This is progress evidence only: the
remaining receivers still require individual source classification and the
complete final unit/integration/product evidence remains mandatory.

After the legacy SREG-stack, ARPL and descriptor-query fixture reconciliation,
the same fresh x64 selection executes 56/78 cases: 56 pass, 22 fail, elapsed
1.03 s, no timeout. The ignored third log is
build/t546-s10-cpu-owner-x64-rescan-4.log. The descriptor-query fixture now
uses a real real-mode #UD vector where that is the fixture's mode, while its
deliberately IDT-less protected/VM86 cases assert resident shutdown rather than
misreporting a delivered fault. This is not final qualification.

The final eight-case x86 arithmetic/extension/FS-GS replication passes after
fresh builds (8/8, 0.65 s). Source/setup failures are retained above, not counted
as successful tests. All current handles are terminal and the receiving batch
is still incomplete; no P, artifact refresh or complete-CPU claim follows.

## Receiverless Reconciliation And 8087 Preservation

The remaining receiving fixtures were reconciled at the owning finalizer
boundary.  A valid architectural exception receiver still enters normally;
an intentionally absent or unusable guest receiver now reaches the CPU-owned
resident shutdown state, which Core reports as an ordinary waiting result with
`VCPUINS_EXCEPT_SHUTDOWN`.  Deliberately rejected host/provider reads remain
host diagnostics and must not be reclassified as guest delivery or shutdown.
The tests retain 8086 strict unsupported encodings as their direct diagnostic
path, while 80186-and-later fixtures distinguish actual real delivery from a
receiverless protected-mode terminal state.

The consolidation initially over-applied the early-CPU internal-vector filter
to an existing 8087 contract.  This was corrected narrowly: real-mode `#MF`
from an attached 8087 remains eligible for vector 16 delivery on 8086/8088;
the generic early-family exclusions for other vectors remain unchanged.  This
preserves the prior, source-recorded `FWAIT` vector-16 frame route rather than
adding a new NPX mechanism.

Fresh focused receiver suites pass on x64 and x86, including the 8087 `FWAIT`
case.  This historical full-unit snapshot originally retained two failures:
`library.console_broker_display` (Lib viewport scope) and
`unit.vm-fault-outcome-runner-smoke` (a runner timeout).  The latter was later
found to be an S10 integration defect and repaired; the current result is
recorded below.  The three affected owner manifests are complete and rehashed.
This is S10 progress evidence, not S10 or T546 closure: the packet's remaining
source-qualified receiver and later mechanism obligations still apply.

After the owner-local rotate receiver repair, the same fresh x64 selection
executes 57/78 cases: 57 pass, 21 fail, elapsed 1.22 s, no timeout. The
ignored fourth log is build/t546-s10-cpu-owner-x64-rescan-5.log. The repaired
rejected Group-2 extension forms now use the same real #UD receiver contract;
no rotate or shift algorithm changed. This is progress evidence only.

After the INC/DEC first-group receiver repair, the same fresh x64 selection
executes 58/78 cases: 58 pass, 20 fail, elapsed 1.04 s, no timeout. The
ignored fifth log is build/t546-s10-cpu-owner-x64-rescan-6.log. 8086 retains
its strict unsupported-prefix diagnostic; 80186 and later receive #UD through
the valid real-mode vector where the core supports that architecture. This is
progress evidence only, not final S10 qualification.

After the three INC/DEC groups were reconciled, the same fresh x64 selection
executes 60/78 cases: 60 pass, 18 fail, elapsed 1.01 s, no timeout. The
ignored sixth log is build/t546-s10-cpu-owner-x64-rescan-7.log. The changes
replace stale terminal #UD assertions for 80186/80286 receiver cases, while
retaining 8086's strict diagnostic behavior; no arithmetic algorithm changed.
This remains progress evidence only.

The protected transfer/data fixtures also deliberately omit a usable IDT
receiver. Their prior expectation of a host-visible replacement fault was
therefore stale: failed protected exception delivery reaches the CPU-owned
resident shutdown state without retirement, a host stop, or a host fault.
The shared fixture now asserts that exact terminal state while retaining each
owner's original atomicity checks. Focused protected-far/data cases pass on
both x64 and x86. The next fresh x64 owner scan is 64/78 passed and 14 failed
in 1.03 s (build/t546-s10-cpu-owner-x64-rescan-9.log); this is progress
evidence only, not S10 closure.

Decode admission and software INT rejection probes had the same stale premise.
Decode now distinguishes normal admission from the final resident-shutdown
result of an intentionally absent protected receiver; it does not relabel a
source exception as a host I/O failure. Software INT keeps 8086/8088's strict
diagnostic result, exercises actual 80186 real-mode #UD delivery, and asserts
resident shutdown only for the deliberately receiverless 80286/80386 cases.
Each focused owner passes on both widths. The fresh x64 owner sweep is 66/78
passed and 12 failed in 1.05 s (build/t546-s10-cpu-owner-x64-rescan-10.log).
This remains incremental evidence only.

LEA, protected operand-address, and prefix/LOCK owners likewise distinguish
early strict diagnostics, real 80186 delivery, and receiverless 80286/80386
shutdown rather than calling every rejection a host fault. All three focused
owners pass on both widths. The current fresh x64 sweep is 69/78 passed and 9
failed in 1.00 s (build/t546-s10-cpu-owner-x64-rescan-12.log). Deep protected
state owners remain open; this does not close S10.

## Current Result (After The Historical Increments)

The historical counts above record the discovery sequence only.  After the
nine named protected-state receiver owners were reconciled, a fresh registered
`x86.cpu_*` scan passes 78/78 on x64 and 78/78 on x86.  No test is removed;
valid-receiver frame assertions remain beside receiverless terminal-shutdown
assertions.  The CPU-owned `VCPUINS_EXCEPT_SHUTDOWN` result originally exposed
one runner defect: the IBM-PC runner treated it like ordinary HLT and yielded
forever.  It now ends the host run while preserving the Core-resident CPU state
for its architectural NMI/reset recovery.  Fresh full units are 504/505 in
each width; the sole remaining failure is the excluded Lib viewport test.
S10 remains active pending the packet's cross-domain closure evidence and
coordinator acceptance.

The prior T331 static verifier was also reconciled: it had required the
superseded four-branch real-delivery helper, contradicting this S's one
classifier/one-finalizer design.  It now requires the sole
`_e_exception_vector` classifier, one primary and one secondary `ExecFinal`
classification, and rejects revival of the retired helper.  The #UD gate also
now derives its inventory from each registered unit target's actual sources,
then requires a declared disposition for every owner; this deletes its former
hand-maintained second target list.  All 82 x64 and 418 x86 specialized gates
pass after these verification corrections.

## Complete CPU-owner Reconciliation

The same registered `x86.cpu_*` owner selection was then freshly rebuilt and
executed after the remaining nine protected-state receiver fixtures were
reconciled.  It passes 78/78 on x64 and 78/78 on x86.  The formerly failing
owners are segment-selector, descriptor-system, execution-paging,
outer-return, far-transfer, IDT-privilege-entry, VM86-delivery, protected
IRET, and IRET state.  Their test bodies still retain their positive valid
receiver/frame assertions; only deliberately receiverless protected setups
now assert the CPU's terminal resident-shutdown state.

This scan closes the prior owner-test failure inventory.  It does not turn
unimplemented S12-S18 mechanisms into S10 work, nor does it close S10: the
packet still requires its named gates, external contexts, products and
coordinator acceptance.  The isolated dual full-unit run is 504/505 in each
width, blocked only by the pre-existing Lib viewport failure outside this S's
admitted ownership.

All original 58 external contexts now pass once with current candidate binaries:
per width, the default product contributes 22 contexts, XT one, 5170 three,
and DeskPro three.  The default group includes DOS keyboard and memory-fault
probes, FDC/ATA media paths, CGA/EGA, debug pause, and the Windows 3.1
checkpoint; the fixed-profile groups run their own owner INI and boot matrix.
This is external execution evidence for the candidate, not a substitute for
the packet's remaining source and coordinator qualification.

## Test-source namespace cleanup

The retired `test/vm/` source directory remains absent.  Within the current
NXVM application test tree, the remaining historical `vm_` source filenames
and the presentation-capture test helper were renamed to `nxvm_`; CMake source
references and the keyboard-transport static gate now name the same files.
There is no compatibility filename or duplicate test path.  This is a source
ownership cleanup only: `vm_machine` remains the real machine-assembly API and
`VM86` remains an architectural CPU term, so neither was mechanically renamed.
The product's historical `vm-*` CMake target identities are deliberately out
of scope because changing them is a separate cross-product naming surface.

Both x64 and x86 rebuilt every renamed test source successfully.  Fresh full
unit runs remain 504/505 in each width; the sole failure is the pre-existing,
excluded Lib Console viewport assertion.  No production source or product
binary changed as part of this naming-only cleanup.

After that source-only cleanup, the packet's actual registered CPU-owner
selection was executed again rather than inferred from compilation: all
`x86.cpu_*` CTest owners pass 78/78 on x64 (1.26 seconds) and 78/78 on x86
(4.14 seconds).  This re-executes the finalizer, real/protected/VM86 delivery,
shutdown, recovery, task, paging and receiving cases under their current
registered targets; it does not claim completion of later S11--S18 mechanisms
or replace the required coordinator review.

The S10-owned corpora were then independently SHA-256 checked against their
tracked manifests: `src/x86`, `src/ibmpc`, `test/x86`, and `test/ibmpc` all
match.  This validates the exact candidate bytes used by the owner tests and
receiving boundaries; it is not a substitute for the separate cross-domain
acceptance requirement.

The eight receiving 0546 executables are present at the four fixed App asset
roots.  PE inspection reports `8664` for each x64 binary and `014C` for each
x86 binary; all eight PE Debug directory RVA/size pairs are zero.  Their
current SHA-256 identities were recorded during this candidate verification.

Finally, the T331 static construction verifier was executed directly in both
configured widths.  It passes in x64 and x86, confirming the source has the
sole `_e_exception_vector` classifier and the ordered primary/secondary
`ExecFinal` delivery structure required by this S; it rejects revival of the
retired real-delivery helper.
