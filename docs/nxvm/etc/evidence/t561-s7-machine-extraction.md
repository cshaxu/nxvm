# T561 S7 Neutral Machine Extraction

## Frozen Preflight And Owner Decisions

Baseline is accepted S6 `5ad9ce3dc`; this batch consumes I3 extraction, not
I4's NXVM consolidation or I5's final qualification. The original owner
requires matching production and test hierarchies. Architecture governance
therefore assigns the owner before coding governance updates paths/symbols.

[Current path preflight](t561-s7-machine-paths.tsv) hashes every current N1,
T1 and T2 input from the frozen S2 ledger. It accounts for 35 source files,
47 neutral test files, six support files and two mixed Board runners. S2's
source/test baseline sizes are 7,550/20,469 lines for N1/T1; they are not
expected net growth or a count of new implementation. The current preflight
uses S6 bytes, not the obsolete hashes from S2.

Neutral implementation and owner tests move to `x86-core/machine`, retaining
cohesive clock, timeline, memory, port, transaction, execution and debug files.
CPU/FPU stay where accepted S3 placed them. Only package metadata/gates remain
at package roots; owner-local fixtures follow `machine/support`. The retired
arithmetic catch-all header is not a new shared API. Real uses become direct
Types expressions at their existing owner.

The 80286/80386 manifest runners actually construct PC Boards. They first move
to `test/core/board-base/qualification`, matching the still-current Board
source owner; S8 subsequently moves that whole owner to X86 NXVM. Their CPU
source keys and timing grade predicates remain unchanged. No duplicated
implementation or old neutral path/target alias is retained.

Actual registration review also finds that the 8086/8088 and 80186 manifest
runners consume the same generated five-profile metadata catalog and NXVM
manual-ledger export/result tools. Their pure execution substrate is neutral,
but these programs are outer qualification consumers, not standalone shared
unit inputs. Accordingly, the two additional runner source files follow the
already-classified T2 runners into the NXVM qualification owner. All five
executions and their result/decoder-ledger checks remain registered with their
original assertions and source keys. The direct instruction/retirement timing
tests remain Machine-owned. This explicit N1 refinement prevents shared tests
from acquiring a dependency on product documents/tools; it does not remove
behavior coverage or change I0--I5. The frozen path preflight retains the
original decision so the deviation is reviewable rather than overwritten.

Public neutral declarations, guards, targets and receivers use the
`x86_core_machine_*`/`X86_CORE_MACHINE_*` family. Build the exact symbol map
from the neutral definitions, not a global `core_machine_` prefix replacement:
Board config, PC policy and Board timing contracts must not be reclassified.
Mechanical changes are reviewed separately from assertion/setup adaptations.

The current Ctags-based preview identifies 494 neutral owned symbols and 492
direct source/test/build/tool receivers. It includes the opaque `core_machine`
name itself, not merely names with a trailing underscore. Board config/A20/
qualification names are not selected. Of those receivers, 46 are family CMake
check/build files, 89 remaining Core source files, 261 Core test files, 94 App
source/test files and two NXVM tools. This is a preview of required substitutions,
not 492 new implementations or an already-applied migration. The reviewed
ignored generator is `build/t561-s7-plan.py`; it writes only preview JSON and
does not edit production or tests. Actual migration must also partition build
registration, fixtures and mixed runners rather than blindly apply the preview.

## Private Receiver Review

The admitted receiver universe includes the three CPU-private execution tests
and 25 Board/adapter files recorded by S3/S5. The sweep uses actual includes:

```text
rg -l '#include "core/x86/(machine|memory|port|transaction|timeline)\.h"' test/core/board-base test/core/board-at test/core/board-xt test/core/machine
```

Outer users of the six T1 support files also require review. Moving a private
fixture into X86 Core does not make it public. Preserve pure execution
white-box assertions with Machine's own tests; preserve Board wiring through
public operations and owned Board setup. Do not copy CPU layouts or publish
test-only state setters. CPU-cache integration should create real cache state
through public CPU execution/bus operations rather than inject private lines.
Closed production 486SX admission is preserved; internal qualification may
continue using the existing public CPU profile binding on an owned context.

## Required Direct Proof And Receiving Qualification

The new non-PC consumer is project-authored, ROM-free and public-interface
only: small RAM, immutable reset ROM, aliases, MMIO/unmapped behavior, real
instruction retirement/IRQ delivery, HALT with device advance, bounded run/
pause, side-effect-free observation and reset/teardown. It links no PC Board,
chip, Product or private test fixture. This is not V30 or Raiden II support.

The active packet requires complete dual-width repository units/specialized
gates, four-profile integrations, eight rebuilt/deployed 0561 PC EXEs and
actual published CLI/INI/PE/version/hash checks, using the original persistent
Ninja/ccache graphs. The bounded progress below does not satisfy that complete
receiving gate. Lib/Emulator, MyNES,
owner INI/ROM/media/snapshots and sibling repositories remain outside changes.
Shared publication and NXVM cutover use separate complete target-scoped P
commits before coordinator acceptance. T561 remains open.

## Implementation Progress, Not Acceptance

The 34 cohesive production files now reside in `src/x86-core/machine`; the
arithmetic catch-all is retired and its four actually used expressions become
equivalent direct Types operations in 76 receivers. The reviewed 494-symbol
and 492-receiver substitutions are applied. CPU/FPU production files remain
unchanged. One ordinary and one observable Machine target replace the retired
Core target; Board's two variants explicitly link their corresponding inward
Machine target. No compatibility alias or second runtime is added.

Test registration is partitioned by owner. The five original manual-manifest
executions and three result/decoder-ledger checks remain outer qualification;
direct Machine behavior cases belong to `test/x86-core/machine`, with support
inside that owner. The new public-only non-PC case executes an authored reset
jump, ROM protection, RAM/alias/MMIO, observation, HALT deadline, IRQ entry/
return and reset/teardown on a real 80386 CPU. Interrupt entry is explicitly
checked as a non-retiring run boundary, followed by handler/guest retirement;
it is not silently retried until a desired result appears.

The three CPU-private test borrowers are repaired without production API or
CPU implementation changes. Power verifies absence of Stop Grant through
the existing rejected clock transition. Soft reset establishes SMBASE through
guest SMM save-area writes and observes real subsequent SMI entry/return.
Cache tests prime real CPU lines, then use Machine-owned unsnooped RAM test
writes to establish stale data. A stale alias line is filled before alias
publication; selected=false is correctly not treated as disabling a RAM alias.
Cached PTE hit update, accepted/failed provider-write invalidation, alias/DMA
snooping, locked reads/write-hit updates, NW-dependent backing invalidation
and DMA exclusion retain observable checks. Copied CPU state/operations replace
CPU-private layouts. No test-only getter, cache setter or public fault API is
introduced.

X64 builds both Machine variants and all receiving Board/adapter/Product
production libraries. The 43 Machine runtime cases pass in 33.48 seconds;
the complete 156-case X86 Core owner suite then passes in 10.72 seconds.
Logs use `build/t561-s7-x64-{machine-runtime,owner-tests}.log` and production
build logs. The source gate and its negative tests reject CPU/FPU -> Machine,
Machine -> private chip headers, outward includes and outward links. Relative
manual check inputs are normalized before scanning, avoiding an empty glob
being mistaken for a checked source tree. Source/test manifests pass at this
checkpoint; they must be refreshed again after subsequent changes.

X86's complete 156-case owner suite also passes in 10.04 seconds, from the
same original persistent graph after compiling its new Machine targets. Its
log is `build/t561-s7-x86-owner-tests.log`. The
25 higher Board/adapter private-Machine files and outer support-fixture users
still require actual receiver repair; several family/App tests therefore are
not yet ready for full qualification. All-S7 actual diff/behavior reconciliation,
final names, full repository units/gates, four-profile integrations, eight
updated artifacts and P acceptance remain open. No complete S7/T561 or desktop/
RDP qualification is claimed. The working changes remain uncommitted while
the complete admitted delivery is unfinished.

## Higher Receiver Partition Progress

The D4 transaction case now constructs a real neutral Machine and fills its
public registration boundary until it returns `NO_MEMORY`. It checks the App's
five existing configuration failure/retry modes without reading Machine RAM or
registry layout. App-local parity input invokes the same App implementation's
electrical callback, without a new public fault injector. The Machine-owned
memory-device registration case separately retains provider/observer counts,
parity allocation identity, partial-batch rollback, replacement ownership and
complete removal assertions. It covers provider exhaustion at zero/one free
slots, incompatible parity size, observer exhaustion and an invalid second
route after the first was accepted. The now-unused cross-owner callback
fixture `machine/support/memory_registration_fixture.{c,h}` is deleted rather
than exported. The invalid-second-route case proves the generic registry's
rollback; D4's missing-alias rejection remains in its own App test.

CPU handle identity across repeated reset is checked inside Machine's existing
lifecycle test. The Board reset test retains its two real CPU/PIC execution
checks and no longer reads the neutral Machine layout. These four cases
(registration, lifecycle, Board reset and D4 transaction) pass on both widths;
logs are `build/t561-s7-{x64,x86}-receiver-batch-{build,tests}.log`.

Actual behavior inspection refines four additional former Board test owners:

| Old Board composition file | Actual new Machine owner | Retained behavior |
| --- | --- | --- |
| `machine_retirement_observation_smoke.c` | `machine/machine_retirement_observation_smoke.c` | Original CPU/FPU retirement contexts, formula attribution, physical admission, copied snapshots and callback boundary rejection. |
| `core_machine_fpu_8087_smoke.c` | `machine/machine_fpu_8087_smoke.c` | Guest arithmetic/FNINIT, stack exceptions/reset, FWAIT and CPU/FPU profile gates. |
| `machine_transaction_lifecycle_smoke.c` | `machine/machine_transaction_lifecycle_smoke.c` | CPU port/memory begin/commit/cancel, accepted side effects, observation, busy-owner rejection, FPU commands and reset trace order. |
| `machine_instruction_timing_ledger_smoke.c` | `machine/machine_instruction_timing_ledger_smoke.c` | Existing neutral source-time, qualification, fault/rollback and overflow predicates. |

All four previously used Board construction or fixture convenience but assert
no PC device behavior. They now link only Machine (observable where originally
required), use explicit CPU/RAM configuration and Machine-owned support. Their
old registrations/targets are removed; receiving build/checker paths follow the
new owner. This is a disposition refinement within the admitted 25 private
receivers, not removal of qualification or a change to CPU timing grades.
The small timing-ledger lifecycle setup remains local to its five callers;
it does not export a cross-owner fixture API.

The complete owner suite reached 157/157 on both widths after the retirement
move (`owner-receivers` logs). FPU and transaction-lifecycle cases additionally
passed focused x64 execution. Later full-owner reruns are recorded separately;
these intermediate passes do not qualify the remaining outer receivers,
product artifacts, integrations or S7 closure.

The subsequent complete owner rerun passes 160/160 on x64 (9.77 seconds)
and x86 (9.31 seconds), including all four reclassified tests and source/test
manifests, ownership, dependency and negative gates. Logs are
`build/t561-s7-{x64,x86}-owner-partition-{build,tests}.log`. The timing-ledger
source explicitly includes the public debug contract it previously obtained
indirectly through the Board fixture; no production declaration changes.
The original private-header query now reports 20 rather than 25 higher files:
four pure tests moved to Machine and the Board reset identity test became
public-only. The three CPU-private cases remain repaired. This closes only
those receiver dispositions; the remaining 20 files, outer boot fixture,
complete receiving qualification and eight PC artifacts remain open.

## Further Public Receiver Repair

The firmware-capability Board case now queries existing public immutable-ROM
coverage after rejected configuration. Machine's immutable-ROM case owns the
exact preserved provider/ROM counts, absent failed mappings, prior-image
contents and ordinary-RAM fallthrough assertions, including original status
propagation. Both cases pass on both widths (`firmware-owner` logs).

The two-instance Board case no longer writes `lifecycle = RUNNING` or accesses
the private port latch. A real guest writes KBC D1h/03h, reads port 92h, invokes
the existing test device and writes port 92h to disable A20. During that actual
running callback the stopped-only mask setter must reject the call; copied
mask observation and guest-stored port value prove the enabled state. The
final mask proves disable; all RAM/port-instance/reset assertions remain.

Machine's RAM/port context case now owns public route-batch failure proof at
every one of six read/write allocation positions, successful retry, removal,
preservation of an unrelated sentinel, and the private single-entry sticky
failure contract. DMA's case retains actual conflicting-port rollback through
public bus operations. Its same-owner implementation assay injects an existing
`NO_MEMORY` result at the public batch call, requiring a null unpublished bus,
then retries against the real Machine and uses its installed ports. This
separates allocator/registry behavior from consumer cleanup/error propagation;
no Machine test fixture, layout, production API or failure hook is exported.
The process-local failure Boolean exists only in that isolated test assay,
following the existing DMA/PIC same-owner source-inclusion pattern.

KBC follows the same declared-boundary assay. Per-entry allocation failures
are Machine-owned; KBC verifies `NO_MEMORY` propagation, unpublished children,
successful retry, real self-test reply, explicit route removal and idempotent
finalization. Actual conflicting 64h registration is rejected repeatedly,
with no leaked 60h/64h readers or 60h writer and the unrelated 64h writer
preserved. The old execution-private `kbc_controller_fixture.c` and its test
declarations/registration are retired. Its CPU/PIC IRQ1 receiver now performs
the entry patch through the existing public debug API after reset rather than
using a raw CPU handle inside the reset callback. Guest reply/IRQ1 assertions
remain in place. DMA and KBC focused cases pass on both widths (`port-owner`
and `kbc-owner` logs).

The explicit-time Board test remains an unresolved private receiver. An
attempt to replace direct advancement with five next-deadline calls was
rejected by actual execution: the legacy RTC setup has no qualified deadline
declarations, so those calls do not mean five ticks. That substitution is
removed, not accepted by relaxing the time assertions or changing production.
The original Board time assertions still pass; their eventual partition is
still required. Generic initialization rejection, zero-step rejection, exact
five-tick advancement, reset and overflow rejection also have direct proof in
Machine's attachment-phase case. Board setup now uses its actual public bus
and copied alias contracts instead of the retired broad fixture include.

The original private-header query now reports 16 higher files, down from 25.
The outer boot observer, transitive fixture consumers and complete receiving
qualification remain open regardless of that direct-include count. No new
artifact, complete-P commit or S7 acceptance is claimed.

After these repairs, the complete owner suite again passes 160/160 per width
(x64 6.51 seconds, x86 6.75 seconds). The five affected Board cases pass
5/5 per width. Logs are `build/t561-s7-{x64,x86}-receiver-cleanup-*`.
Documentation governance and `git diff --check` also pass at that checkpoint.

Four further composite-fixture operations now use existing public copied
observations/bus operations: elapsed ticks, CPU halted state, and post-run port
read/write. The old comment claiming a bounded run remains RUNNING was wrong;
the actual return boundary is PAUSED. Calls execute outside assertion macros,
and this fixture explicitly retains assertions for root-registered Release
tests as well as component-registered tests. CMOS seed and PC/AT composition
cases pass x64 using these public operations; the x86 receiving run is recorded
in `build/t561-s7-x86-public-observation-*`. Other private operations in this
composite fixture remain unresolved and are not hidden by these four repairs.

## Continued Owner Convergence

XT PPI construction now has its own `board-xt/controller_registration` case.
It is removed from the broad Board protocol case rather than executed twice.
The XT-owned source assay observes the public route-install failure, preserves
the unpublished PPI predicate, retries, uses real public port I/O and removes
only the PPI's routes before repeated finalization. The unrelated 80h sentinel
remains live. Machine's existing registration case now covers all eight
read/write allocations of a four-route batch plus success, preserving every
old PPI allocation position at its real owner. The execution-private
`xt_ppi_controller_fixture.c` and its declaration/build input are deleted.
The new owner case and the Machine case pass on both widths (`xt-registration`
logs). The broad XT guest/protocol case still awaits full receiving proof.

The composite port-lifetime helper now constructs/destroys a genuine opaque
Machine through its public contract, not a manually allocated partial Machine
with only its private port table initialized. The three outer-IRET,
IDT-privilege and 16-bit task-switch PIC consumers pass on both widths
(`public-port-lifetime` logs). Their CPU-private fixture debt remains the
explicit S8 receiver; this repair does not claim to resolve that separate debt.

Timeline assertions are partitioned without changing their predicates. The
Machine timeline case schedules the immediate event and checks immediate
progress, actual 80286 execution, one callback, three ticks, no pending event
and sequence 1. The Board case retains actual PC CPU/DMA/PIT/PIC trace order
and elapsed time without injecting an event into private Machine storage;
its own untouched sequence is consequently 0. Both owner cases pass on both
widths (`timeline-owner` logs).

Three more files follow actual execution ownership:

| Former Board file | New Machine file | Preserved scope |
| --- | --- | --- |
| `composition/machine_scheduler_smoke.c` | `machine/machine_scheduler_smoke.c` | Empty budget, real instruction timing, before/after callbacks, deadline/IMMEDIATE/L1 selection and representable clock horizon. |
| `core_machine_cpu_timing_preview_smoke.c` | `machine/machine_cpu_timing_preview_smoke.c` | Original instruction/profile lexeme cases, limited fetch, taken-target inspection, CR-MOV quirk and unchanged register/transaction/trace publication. |
| `composition/core_machine_rom_route_transaction_smoke.c` | `machine/machine_rom_route_transaction_smoke.c` | Every original capacity/rejected-source case, complete ROM/provider rollback, prior bytes, retry and raw reset-window read. |

Scheduler now binds a genuine synthetic attachment through the public API,
instead of replacing a stored production Board callback bundle. None of its
original assertions observed PC chip state; the no-op forwarding callbacks
and unused DMA counters are retired, while all budget/callback/deadline/time
predicates remain. Real Board-order assertions remain in their Board owner.
ROM transaction tests supply their own explicit 16-byte reset-window completion
callback; Board-selected reset-alias policy stays in Board tests. Both tests
also prove attachment finalization exactly once.

The preview test's only PC-policy case, `preview_test_default_reset_alias`,
moves intact to the existing Board reset-ROM case as `reset_alias_without_rom`.
The Machine preview owner supplies explicit RAM/profile and reset-RAM aliases;
it no longer gets those defaults from Board construction. Private publication
and CPU-preview inspection helpers are local to that Machine test, and their
old composite-fixture declarations/bodies are removed. No lexeme input,
profile matrix, successful/failure predicate or success marker is dropped.

Parity tests now divide storage mechanics from electrical wiring. Machine's
existing corrupt-parity inspection case verifies that an operational read
calls the registered fault observer without losing the data byte; its registry
test owns allocation/owner identity. AT's test observes its real public memory
registration, verifies the callback and owner, and invokes that exact callback
to check latch/mask behavior. Board tests use the public parity-fault API and
prove exclusive storage ownership by rejecting a second lease, revoking the
expected owner and successfully installing/removing a fresh lease. No lower
parity pointer is read or flipped by Board. Both parity owner tests and the
Machine corruption case pass on both widths (`parity-owner` logs).

PIC's private DMA-busy acknowledgement assay moves to Machine's existing
transaction-lifecycle test. Rejection preserves the vector, pending source and
DMA owner without calling the source or publishing a trace; success consumes
the source once, publishes the original two transaction events and releases
ownership. The real cascaded PIC test instead executes the guest interrupt,
checks vector-2Eh handler arrival, master/slave ISR bits and acknowledge-before-
stack-write order. Register entry setup uses the existing public debug API
after reset; generic reset transaction counters remain asserted in Machine's
transaction-lifecycle case. No raw CPU/transaction state remains in PIC tests.

Board binding identity is observed at the actual public bind call in the
same-owner Board assay. It checks all four original callback identities and
the Board context/back-pointer. Machine's attachment case checks their copied
storage and lifetime. The Board identity case no longer inspects a private
Machine attachment. The borrowed observation pointer is test-only, scoped to
the synchronous construction call and cleared immediately afterward.

The complete current Machine/CPU/FPU owner suite passes 163/163 on both widths
(x64 8.31 seconds; x86 8.88 seconds). The five affected PIC/binding/XT/parity
Board cases pass 5/5 per width. Logs are
`build/t561-s7-{x64,x86}-final-receiver-batch-*`. All four changed package
manifests were refreshed before that run. Direct private-header borrowers
are now nine rather than the original 25; transitive composite/boot users,
full receiving qualification and artifact delivery are still unresolved.
This progress is not S7 acceptance, a complete implementation P or T closure.

### RAM Construction Receiver Partition

The Board RAM-construction case now uses only public neutral Machine contracts.
It retains default 16 MiB and explicit 2 MiB construction, installed-size,
freeze/reset, three actual Board bindings, seven rejected clock-ratio
preflights and null-output validation. A same-owner Board assay intercepts
the actual `x86_core_machine_neutral_create` call in `machine_board.c`, not a
copied construction sequence: injected `NO_MEMORY` must clear both outputs
after exactly one attempt, while invalid Board clocks must fail before any
constructor attempt. The scoped test observation is cleared after the
synchronous call. Normal construction still calls the real neutral constructor.
The test no longer links the copied `board_construction_fixture.c` sequence.

Allocator representation remains at Machine's owner. Its existing configuration
case now proves actual RAM-allocation success/failure at both 2 MiB and 16 MiB,
exactly one allocation attempt and null publication on failure. Explicit zero
RAM is rejected before allocation; it does not acquire the Board default.
The existing RAM/port case retains the original 16-to-2-MiB resize, installed
capacity and written-byte round trip, alongside its independent-memory checks.
No production file, public API, persistent state or allocation algorithm changes
in this receiver partition. Raw attachment storage remains covered by Machine's
own attachment test; Board binding checks observe the actual public bind call.

Both original Ninja/ccache graphs rebuild successfully. The entire X86 Core
owner suite passes 163/163 per width (x64 8.23 seconds; x86 6.74 seconds),
including source/test manifests and package boundary gates. The two affected
Board construction/binding cases pass 2/2 per width. Logs are
`build/t561-s7-{x64,x86}-ram-owner-{regression-build,regression-tests,board-tests}.log`.
An initial focused regex selected only the two Machine cases; the actual
`unit.core-machine-*` Board registrations were subsequently selected and run,
not counted from that first command. Direct private-header borrowers are now
eight (17 of the original 25 repaired). The composite constructor/port fixture
still has other live callers; it is not falsely declared retired. Transitive
fixture users, full receiving qualification, new EXEs and S7 acceptance remain
open. Protected-scope diff and `git diff --check` remain empty.

### Reset-ROM, Transaction, HOLD And Explicit-Time Receivers

Four further direct private-header receivers are partitioned by behavior:

| Receiver | Board proof retained | Machine proof retained at owner |
| --- | --- | --- |
| `machine_reset_rom_alias_smoke.c` | Both actual 286/386 reset fetches execute the far jump and HLT in F000:0001; exact 14/15/16-byte presence policy, explicit high ROM, intersected two-byte segment, no-ROM reset RAM and absent-memory rollback/ROM priority. | Both 24-bit/32-bit reset windows retain exact mapping counts 1/2/2/4, raw reset EA/CC reads and holes/coverage. Actual provider-allocation rejection proves one attempt, no publication/capacity and successful retry. |
| `machine_transaction_smoke.c` | Actual 8237A channel-2 port programming and five service phases publish one DMA write/commit, no cancellation, the expected A5 byte and a released bus that accepts the next cycle. | Original CPU prefetch/provenance/port/reset trace predicates, private transaction reset counters and DMA query/before/memory/after/failure ordering move to `test/x86-core/machine/machine_transaction_smoke.c`. |
| `machine_80386_dma_hold_smoke.c` | Real CPU retirement precedes DMA commit, then DMA/PIT/PIC progression; HOLD request/ack/release are ordered, FDC/HDC remain absent and reset clears DMA delivery. | Artificial HOLD acquisition rejects a CPU transaction; release/reset clear ownership and preserve request-before-ack-before-release trace order in the existing transaction-lifecycle test. |
| `machine_explicit_time_smoke.c` | Real guest HLT and repeated run publish no extra progress; five RTC deadlines add exactly five ticks/seconds; reset clears Machine time but preserves RTC, then one deadline adds one tick. | Existing attachment-phase test retains unstarted/zero-step/overflow rejection, exact five-tick publication and reset behavior without a Board reading private time. |

Reset-ROM failure injection occurs at Board's actual public memory-registration
call, in the same-owner source assay. Board's state is cleared on rejection;
inspection still sees ordinary zero RAM and retry restores fallback behavior.
Machine's separate test proves the real allocator failure and registry counts.
No test-only production mutation API or opaque-layout copy is introduced.

The transaction body is moved with `git mv`; its remaining stack-owned raw
transaction/RAM checks stay with Machine. Board's genuine DMA case constructs
a real Board/Machine, rather than a manually initialized partial executor.
The unchanged channel-2 guest port program is shared by the two Board DMA cases
in their existing same-owner fixture, not copied across packages.

The HOLD integration no longer invokes private `advance_time(8)`. That interval
was setup to finish the pending controller transfer, not an asserted final
elapsed duration. It now explicitly admits the existing bounded L1 policy and
uses the public L1 operation until the normal controller path completes. The
original three-tick CPU retirement, byte value, event order, absent-device and
reset predicates remain, with added ordering checks on the real HOLD events.
This changes test setup, not production timing grades or admission.

The explicit-time fixture uses an actual public Board plan to obtain the
existing L2 deadline declarations. It programs RTC update interrupts via guest
ports and re-enables them after reset, because RESET correctly clears UIE while
retaining calendar phase. A failed initial attempt placed register B in CMOS
seed defaults; Board correctly rejected this, because those defaults exclude
volatile RTC control registers. That invalid test setup was removed, not fixed
by weakening production validation or accepting an unqualified deadline.
Every one-second advance still checks actual elapsed time, and the original
five-second RTC/timeline/reset predicates are unchanged.

Fixture-shape inventory retains all 101 historical identities with the neutral
transaction body at its new owner. The four metadata runners now point to their
actual Board qualification paths. The additional public port-owner constructor
introduced earlier is explicitly classified: the direct-constructor universe
is 136 rather than 135. This is a named new real constructor replacing manual
partial-layout allocation, not an unexplained count allowance. Unclassified
paths are reported before count mismatch; the gate passes with no exemption.

The full current X86 Core suite passes 164/164 per width (x64 7.14 seconds;
x86 9.34 seconds); all six affected Board cases pass 6/6 per width. Logs are
`build/t561-s7-{x64,x86}-bus-time-owner-{regression-build,regression-tests,board-tests}.log`.
Direct private-header borrowers are four, down from eight before this batch:
the mixed composite fixture, construction fixture, EGA registration case and
time fixture. Their transitive callers are still part of S7, not hidden by that
direct count. Full receiving qualification, eight rebuilt PC EXEs, complete P
delivery and coordinator acceptance remain open.

### Plan, Port Registration And Instance Receivers

`plan_core_fixture.c` no longer imports `construction_fixture.h` or reads
Machine attachment/port/time storage. Its redundant attachment helper is
retired: the Board test already observes its own `board->core` association,
and the binding/neutral attachment tests independently prove the actual public
binding and stored copy. PIT/DMA deadline tests retain exact ticks 1 and 3,
using checked public bus writes and copied elapsed-time observations.

The XT route table preserves every original address/read/write/presence
predicate before reset. A scoped public registration must conflict on an
installed endpoint, or succeed on an absent endpoint and immediately revoke
only its probe owner. After reset, actual public bus reads must reach the
master PIC and reject absent slave-PIC/RTC endpoints. No private route table
is queried or copied into the fixture.

Plan failure tests now reject the actual DMA, PIC and PIT dependency calls in
the same-owner Board construction assay. Each must reach the selected failure,
return `NO_MEMORY`, clear both sentinel outputs and destroy the candidate
Machine exactly once. This replaces copied constructor execution and private
port-allocation injection at Board. The first three raw allocation positions
remain exercised by Machine's existing eight-position route-atomicity case.
The test-only observation/counter is active only during that synchronous call;
normal construction continues through the real dependencies. Plan no longer
links `board_construction_fixture.c`; the latter remains live for the separate
mixed port receiver and is not yet declared retired.

The three pure range-registration, batch-registration and timestamp/wired-OR
cases move from `port_assembly_fixture.c` into the standalone
`test/x86-core/machine/machine_port_registration_smoke.c`. Original bodies and
assertions were compared against accepted HEAD after only the recorded neutral
namespace, explicit 80386/2-MiB constructor and local naming substitutions;
all three match. They retain allocation-position rejection, unrelated routes,
empty failed prefix, retry, duplicate rejection, real public port round trip,
four-byte reads, shared 64-bit tick and zero-time convenience semantics.
The old declarations/Board dispatches are removed, not kept as a second path.
The two tiny value-provider callbacks are separately owned test fixtures at
their independent package boundaries, not a borrowed private fixture.

Port B's arbitrary-tick table (including 0x100000040) moves to AT parity's
same-owner test of the real endpoint. It checks all original bit-4 transitions
while public Machine elapsed time remains zero. Board still executes the real
NOP/IN sequence and checks that the endpoint sees the live guest tick. It no
longer calls the private Machine port operation merely to supply a synthetic
tick to a Board callback.

The NXVM two-session case no longer links the mixed private Machine fixture.
It retains real independent register, RAM and watchpoint behavior, Board device
isolation and distinct DMA binding tokens. Exact CPU/RAM/port identity checks
move to Machine's lifecycle test, which additionally checks distinct allocated
RAM backing. Unused `test_core_write_physical` and
`test_core_invoke_parity_fault` declarations/bodies are removed after the whole
source/test/build/tool scan finds no caller. Frozen S2 symbol records remain
historical evidence, not live declarations to preserve.

The additional neutral port-registration constructor is explicitly classified
in the fixture gate: 137 direct sources, still retaining the 101 historical
identities. The gate is not bypassed or reduced to an allow-all path scan.
Both original graphs pass the complete current 165-case X86 Core owner suite
(x64 7.80 seconds; x86 8.21 seconds), and five affected Board cases plus the
real NXVM two-session case pass 6/6 per width. Final logs are
`build/t561-s7-{x64,x86}-plan-port-final-{build,owner-tests,receiver-tests}.log`;
earlier focused evidence uses `plan-owner`, `port-partition`, `port-b-owner`
and `instance-owner` log prefixes. Four package manifests were refreshed.

The direct private-header count is still four: indirect Plan/App consumers
and three pure functions are repaired without falsely counting that as a
retired mixed fixture. Remaining construction/port, EGA, time and composite
fixture callers require their own complete behavior partition. No production
code, public interface, Lib/Emulator/CPU/FPU implementation, MyNES, EXE or
owner input changes in this receiver batch. Full receiving qualification,
eight new PC artifacts, S7 P delivery and T561 acceptance remain open.

## Input, Runner Failure And Video Registration Owner Partition

The input/display guard case no longer links the mixed execution-private
fixture. It observes actual INIT, STOPPED, PAUSED and FAULTED states. A real
guest OUT callback executes the RUNNING matrix; a valid installed firmware
reset callback executes the paused firmware-operation guard matrix. It does
not assign lifecycle or firmware-operation flags from the Board test. The
neutral configuration/open-state assertions remain at Machine's owner.
The unused private flag mutator is removed rather than relocated.

The App runner-error case no longer overwrites Machine lifecycle or replaces
a frozen firmware provider. A same-owner Core Machine source assay intercepts
its actual run/reset calls and returns the existing INVALID_STATE/UNSUPPORTED
failure. One-shot atomic requests identify the exact opaque Machine, and the
App verifies the rejection was consumed before asserting its actual worker's
error notification. It clears the requests after unbinding. This fixture adds
no production state, thread or API. The actual firmware-reset failure contract
is independently retained at X86 Core Machine: a valid public firmware
provider rejects reset, the original error escapes, lifecycle is INITIALIZED,
RAM reset effects have occurred, the provider remains installed and retry
reaches STOPPED. The two obsolete lifecycle/provider replacement helpers are
removed from the outer mixed fixture.

Video registration now constructs an opaque neutral Machine. The Board test
does not allocate a partial Machine or read its port/memory representation.
At the real Board source owner, a synchronous source assay rejects the public
memory/port installation boundary. It verifies CGA's eight-route call and
the actual 15/27/22-route EGA/VGA/Compaq batches. A port-stage rejection must
follow one successful memory publication and remove that exact owner token;
the active CGA chip remains unchanged. Each variant retries successfully.
The actual final Compaq endpoint collision remains a real public conflict,
not a mocked success or failure.

Public scoped registration conflicts prove route presence/absence before
freeze; successful probes are immediately revoked. After video teardown,
actual stopped bus/memory reads and a real RAM write prove unrelated port,
memory-provider and write-observer callbacks remain installed. These are
behavior checks, not copied private registration counts.

The old video test's representation-level assertions now belong to the
existing Machine cases: registry growth allocation failure and exact
count/capacity/retry; standard-versus-overlay priority and frozen rejection;
two-standard-route partial failure, retained provider/observer counts,
duplicate rejection, retry and removal. Existing provider-limit and full
observer atomicity cases remain. The port-registration case additionally
checks every allocation position for batch sizes 8, 15, 27 and 22, with
empty failed prefixes, exact allocation counts, successful complete retry,
complete owner removal and an unrelated live sentinel. Board-specific
addresses/variants stay at Board; raw allocation mechanics stay at Machine.

The first new Board test setup was correctly rejected by the existing
non-null memory callback validation. The fixture now supplies a write
callback that explicitly declines access; production validation was not
changed or bypassed. Final dual-width owner suites pass 165/165 and the three
input/display, runner-error and video receivers pass 3/3 per width. Logs:
`build/t561-s7-{x64,x86}-video-owner-final-{build,owner-tests,receiver-tests}.log`.
The direct-constructor gate passes 138 explicitly classified sources with
all 101 historical identities retained. Four manifests are refreshed.

The direct private-header count is now three: composition_fixture.c,
construction_fixture.h and time_fixture.c. Transitive consumers still need
their complete partition; this number is not a whole-tree acceptance claim.
Lib/Emulator, CPU/FPU production, MyNES, assets and owner inputs remain
unchanged in this receiver batch. No new artifacts, implementation P or
S7/T561 closure is claimed.

The mixed port receiver's DMA page-lane assertion now runs authored 80386
IN/OUT instructions instead of assigning the private port latch and invoking
its dispatcher directly. Public bus reads verify all four 0x81--0x84 byte
lanes; a real wide IN must return EAX=0x44332211. A second wide OUT at 0x8e
must write its supported lane without overwriting the separately programmed
0x8f value. The same original byte values and bank-boundary assertions remain.
The complete mixed port-rollback test passes on both widths; logs are
`build/t561-s7-{x64,x86}-dma-lanes-public-{build,tests}.log`. Other private
construction/registration branches in that fixture remain explicitly open;
this repair does not count the entire fixture as migrated.

## Complete Construction And Port-Rollback Receiver Partition

Before retiring the private constructor fixture, the unchanged three-variant
test was run with count-only diagnostic output. Its ordinary, auxiliary-PIT
and XT construction paths made 113, 120 and 61 port allocation attempts.
The original case passed; the count witness is
`build/t561-s7-construction-preflight-tests.log`. The temporary print is removed.

Machine's existing port-registration test now retains every failed allocation
prefix for those three sizes and for the device batches of 2, 3, 7, 8, 15,
18, 19, 22 and 27. It verifies exact failure attempts, empty unpublished
prefixes, unrelated sentinel retention, complete retry, exact success
allocation counts and complete removal. The original capacity/rollback
assertions are not replaced by an App or Board test that cannot own them.

Board construction now runs the actual machine_board.c path, not the copied
test constructor. The existing same-owner source assay rejects the real
DMA, PIC, PIT, video, KBC, XT PPI, auxiliary PIT and A20 port-publication
dependencies as applicable to each of the three original configurations.
Each must be reached, return NO_MEMORY, clear both sentinel outputs and
destroy its candidate exactly once. Actual binding identity remains proved
by the separate Board binding assay and Machine-owned stored-copy test.
The obsolete construction_fixture.h and board_construction_fixture.c/h
are deleted, with both Core and D4 target registrations updated.

The real PIT/PIC adapters now receive an opaque public Machine. Their public
route-publication failures retain the PIT chip, clear failed PIC outputs,
publish no endpoints and permit successful complete retry. FDC distinguishes
actual chip-construction failure (zero submitted routes) from its real
seven-route publication failure; configured/topology clearing and DMA
request release remain asserted. RTC tests the actual two-route publication
failure, zero configured state, complete retry and the existing live endpoint
collision. Scoped public conflicts prove endpoint presence and absence.

ATA, Compaq WD and Xebec retain distinct chip/publication failures and all
prior endpoint/cleared-topology/DMA-token assertions. Xebec's occupied DMA
lease and ATA's occupied task-file port remain real conflicts. Their raw
18/19/7 allocation positions remain at Machine. The real Board submits
9/10/4 copied route descriptors, respectively; failed-publication tests
check these actual counts. Compaq's FDC/HDC wired-OR is now verified through
the stopped public bus after real reset, compared with its two real
adapter callbacks rather than a private port latch.

AT and D4 Port B failures are injected independently at their own source
owners. The D4 assay stays in its App; the AT fixture stays under
test/core/board-at/support. The common Board test checks unbound profile/
parity state, absent endpoints and successful retry. A real exclusive
public parity claim/removal proves AT failure released the allocation;
no outer test reads its private pointer. The original refresh count,
reprogramming and repeated-reset checks remain. The pure port-latch reset
property moves to Machine lifecycle and is exercised twice after setting
a nonzero latch. The D4 failure assertion therefore does not borrow AT or
Machine implementation state.

All private Machine use is removed from port_assembly_fixture.c, not hidden
behind a renamed fixture. Dependency injection is test-local, serialized
and cleared after each call; production source, APIs and execution remain
unchanged. Focused dual-width results use construction-owner, pit-pic-owner,
fdc-rtc-owner, hdc-owner and port-b-owner log prefixes. The final port-B run
passes the real Core/D4 receiver pair and Machine lifecycle 3/3 per width.
Complete owner/static requalification follows before any delivery claim.
Only composition_fixture.c and time_fixture.c remain direct execution-private
borrowers; their outer users and the full S7 release requirements stay open.

Final complete owner requalification for this partition passes 165/165 on
both widths (x64 8.15 seconds; x86 7.40 seconds). The five affected Core/D4
receivers plus five Core manifest/corpus/types/ownership gates pass 10/10
per width. Logs are
`build/t561-s7-{x64,x86}-port-owner-final-{build,owner-tests,receiver-tests}.log`.
The fixture-shape gate retains 138 classified sources; documentation and
whitespace checks pass. Updated manifest counts are source Core 151, test
Core 311, source X86 Core 50 and test X86 Core 175. The changed boundary
has no production-source or protected-input delta in this receiver partition.

## Time And Composition Receiver Reduction

Auxiliary PIT's existing counter-loading, 1:4 clock conversion, latch, GATE
trigger and reset assertions are now owner-local tests of the actual Board
advance callback. They explicitly leave Machine elapsed time at zero; they
do not claim a synthetic tick publisher is the real execution path. A second
case constructs a real qualified plan and verifies public deadline advancement
at ticks 1 and 4 with both counters' exact values. The old private time fixture
is no longer linked by this target.

Default NXVM's four floppy-format refresh checks and 80186 polling setup now
advance through actual public deadlines. They retain the original 200 guest
tick bound, not merely 200 possibly much longer events. The actual 80186 guest
polling program and halt outcome remain unchanged. The target no longer links
the private time fixture.

DeskPro BYOB now reaches its exact first PIT input tick through real public
deadlines, rejecting absent/non-progressing/overshooting events. Its reset
vector and exact refresh count remain unchanged. Instead of reading the
immutable-ROM registry, it checks a nonzero byte through the compatibility
alias, attempts a write and verifies the ROM byte is retained. The project-
authored Option ROM fixture adjusts its final checksum byte to preserve the
existing valid checksum; original validation correctly rejected the initial
uncompensated test edit. No external firmware is modified. The obsolete
private ROM-mapping predicate and both private fixture links are removed.

5170 and DeskPro timing-configuration assertions now observe the actual
public neutral-constructor input at the existing Board source assay. A
synchronous bounded capture counts exactly one invocation and clears its
borrowed outputs before local storage leaves scope. This is not a new
production callback, getter or mirrored Machine state. All original selected
page/window/gating/retirement values remain asserted at the product boundary;
Machine's own configuration case independently verifies every transaction
field, both retirement-contract variants, copied wait windows, initial CPU/
DMA ready state and independence from subsequent input mutation. These are
synthetic construction checks, not new physical timing qualification.
Four obsolete private contract/gating helpers are deleted. DeskPro DMA's
existing real public port writes continue proving its installed endpoints.

My5160's complete positive/negative FDC/CGA/Xebec endpoint matrix now uses
configuration-scoped public conflicts before freeze. It no longer links the
mixed private Machine fixture. The same scoped probe replaces duplicate
probe implementations in Plan, video and port-rollback tests. It explicitly
checks configuration remains open before interpreting INVALID_STATE as a
conflict, and revokes every successfully installed probe before its local
owner leaves scope. No device I/O callback is executed by the probe.

Focused dual-width logs use auxiliary-pit-owner, default-apply-deadline,
byob-deadline, constructor-contract and public-port-probes prefixes. The
constructor cases pass 4/4 and public-port consumers pass 5/5 per width.
Remaining composition/time fixture callers stay explicit pending work;
these receiver reductions do not close either entire fixture or S7.

Complete owner/focused requalification passes 165/165 X86 Core cases and
18/18 affected Core/App receivers and Core boundary/manifest gates on each
width. Logs are
`build/t561-s7-{x64,x86}-time-composition-final-{build,owner-tests,receiver-tests}.log`.
This remains receiving-batch evidence, not the whole-repository result.

## Complete Receiving Compilation Inventory

The first complete unit-aggregate compilation exposed stale references that
focused owner builds did not exercise. A keep-going pass collected ten failed
compile jobs: the two 80286/80386 manual-ledger runners included moved unit
programs, seven control/interrupt variants depended on the moved exception
fixture, and one EGA case referenced the moved reset helper. Shared package
unit tests are not made dependent on Board fixtures to repair these receivers.

Board now owns its public-only synthetic RAM reset-window setup and its
explicit IVT-denial input callbacks. The reset helper installs a complete
copied alias batch; the IVT route still returns the original I/O failure.
EGA uses the same Board helper as other Board receivers. These fixtures do
not query private execution or CPU storage.

The two timing runners include qualification-local protected-task/call-gate
guest setup, rather than renaming and including complete shared unit programs.
The finite descriptor/program bytes and public reset/debug/write setup are
retained from the project-owned original recipes. Only setup is repeated
across independent packages; no interpreter, timing evaluator, scheduler,
expected-result table or unit assertions are copied into a second runtime.
Shared tests retain their complete independent unit programs and assertions.
The obsolete outward-includer comment is removed from the shared task test.
Qualification's two public-construction recipe headers are explicitly added
to the fixture gate; its 140 classified sources retain all 101 historical
identities, without an include-path exemption or unclassified constructor.

After these repairs, the complete x64 build-unit-tests aggregate compiles.
The original graph/cache is retained; no new build directory or artifact
deployment is used for the inventory. The latest full-unit execution has its
own complete-unit-build/complete-unit-tests log prefix and is not qualified
merely by this compile result. Both remaining private fixtures and S7's full
release requirements remain open.

### Complete-unit inventory and public ESC observation

The frozen inventory passes 632/632 repository unit cases on each original
x64/x86 graph, including 165 independent X86 Core cases and all registered
Core/App unit receivers. See the complete-unit-build/complete-unit-tests logs
for each width. These automated results do not claim manual desktop/RDP
acceptance or release/integration completion.

The 80386 manifest runner no longer links the mixed private composition
fixture for ESC handoff. It uses the existing observable Board/Machine build
flavor, copied committed CPU/FPU transaction events and public time observation.
The finite handoff case constructs a neutral Machine with the same CPU/FPU
pair and authored reset-window bytes: no unrelated Board deadline competes
with the FPU completion deadline. It checks exactly one committed command,
the original opcode/ModR/M and the original positive 1--19 tick completion
interval. Exact private FPU/transaction assertions remain in the Machine
owner's existing FPU test; neither algorithm nor admission changes.
Both widths pass the complete 80386 manifest runner after this replacement
(esc-public-build/esc-public-tests logs). The obsolete private-field helper
and its declaration are deleted, not moved into a different outer fixture.

### Public operational port and D4 parity receivers

The four remaining App port-table borrowers now perform real stopped-bus
reads/writes. The fixture names explicitly say accepts_read/accepts_write,
not a side-effect-free topology inspection. Only OK and UNSUPPORTED are
accepted statuses; a wrong lifecycle or real callback failure cannot silently
pass an absent-port assertion. NXVM resets after its full profile matrix
before checking initial KBC/wiring state. The original present/absent and
direction assertions remain. All four receiving cases pass on both widths
(public-port-build/public-port-tests logs).

D4 parity now observes its actual App-owned d4_memory.c registration at the
public memory-device boundary. It verifies one successful one-MiB parity
producer with its real owner and callback, injects that registered fault
callback, and retains actual diagnostic-byte, IOCHK, NMI-mask, write-clear
and reset assertions. It does not mutate or copy Machine parity storage.
The source assay stays within the App's implementation and tests; normal
route installation still executes, not a replacement registry. Both widths
pass the D4 case (d4-parity-public-build/d4-parity-public-tests logs).

The original corrupted-parity-byte detection and exact physical-address
delivery are owned by the Machine memory-device case. That case is extended
with the original 0x12345/0x5a input and neighboring 0x12346 write-observer
delivery, alongside its existing allocation/atomicity cases. This separates
memory fault production from App fault consumption without losing either
half. The two outer storage-corruption helpers are deleted.

The resulting complete-unit aggregate rebuilds and passes 632/632 per width
(public-receivers-unit-build/public-receivers-unit-tests logs), including the
extended lower parity producer case. This is an intermediate qualification,
not S7 delivery or artifact acceptance.

### Retire the duplicate artificial HOLD receiver

Board competition no longer invokes private transaction HOLD request/ack or
injects a second artificial HOLD for reset. The exact CPU-admission rejection,
release/reset ownership and ordered request/ack/release assertions already
reside in Machine's lifecycle_hold_reset case. Board retains the real 8237
transfer, CPU retirement, memory result, actual HOLD event presence and Board
device order/reset assertions. D4 queries its existing App-owned refresh
producer directly instead of finding that callback through Machine's private
attachment. The three obsolete forwarding/mutation helpers are deleted.
Both widths pass Board competition, D4 refresh/HOLD and the Machine
transaction-lifecycle case (hold-owner-build/hold-owner-tests logs).

The actual controller emits multiple HOLD intervals, including selection
before the eventual memory transfer. An exploratory stronger assertion that
the first RELEASE must follow the first memory COMMIT was rejected by the
trace (selection request/ack/release indices 5/6/7, transfer indices 45/46).
That new incorrect assertion and its temporary print are absent; original
Board predicates are retained, and the exact artificial sequence is verified
only at its Machine owner. No production behavior is changed to suit a test.

### D4 source deadline versus Machine timeline ownership

The App deadline case no longer injects a private timeline counter. A real
qualified Plan composes the Board clocks; the original D4 refresh completes
at tick 1 while the actual system PIT loads count 3, and the PIT's later
terminal-output deadline remains at tick 4. Both deadline captures, elapsed
ticks, refresh completion and actual PIT outputs are checked. The App's own
borrowed PIT input is queried through the chip's public interface, without
opening Machine state. The time fixture is removed from this target.

Machine scheduler now owns the exact original generic sequence: attachment
deadline at 1 versus queued timeline counter at 4; count remains zero at 1
and becomes one at 4. Its finite test attachment expires the early source
through the normal after-advance callback and is finalized exactly once.
This is test input to the existing scheduler, not a new production clock.
The obsolete upper schedule-counter function/declaration are deleted. Both
widths pass the two cases (deadline-owner-build/deadline-owner-tests logs).

### DMA wait and shutdown owner completion

The remaining upper DMA wait-counter getter is deleted. The existing neutral
executor DMA test now checks BUSRDY-low retaining zero, the first ready quantum
setting one without granting, the next quantum granting and clearing, and reset
clearing a pending quantum, for its existing 286/386/internal-486 matrix.
Board competition retains actual 8237 RAM transfer timing; D4 retains refresh
HOLD ordering, pending address and transferred RAM, without reading Machine's
private wait field. Both widths pass all three cases (dma-owner logs).

The upper shutdown mutator is also deleted. Machine's system-power test owns
the original halted-CPU shutdown injection and verifies reset returns without
retirement or time advancement. DeskPro instead executes guest LIDT with a
zero IDT limit followed by INT 3: failed exception delivery reaches its actual
shutdown/reset policy. A public processor-reset signal first leaves the prior
NMI HLT without clearing the board latch. The final CPU reset address, unchanged
failure-time clock and retained failsafe latch are checked. Both widths pass
the two cases (shutdown-owner logs). No production API or state is added.

### Board/App clock inputs and public execution receivers

DeskPro platform tests drive their actual borrowed shared/auxiliary PIT inputs
for port-B, speaker and failsafe assertions. They no longer link the private
Machine time fixture. The existing real Plan deadline test separately proves
the scheduler delivers the PIT load and terminal edges at ticks 1 and 4.
Both widths pass the pair (d4-pit-owner logs).

XT keyboard BAT, repeat, FIFO and refused-completion cases drive the existing
keyboard's public clock input, retaining every original boundary assertion.
A finite public-deadline case separately advances the real serial source
until tick 260, checking no earlier byte, actual PPI delivery, IRQ1 and 1Eh.
This target no longer links the private time fixture. No synthetic Machine
clock or production API is added.

D4 refresh/HOLD receives both real PIT outputs, then uses the existing bounded
L1 idle policy to deliver DMA. Its actual memory-write commit must still occur
within the original 11-tick bound, with refresh HOLD before DMA and no CPU
transaction in between. The non-D4 negative executes seven real NOPs covering
at least the original 20 ticks and still observes no refresh transaction.
An attempted run-budget replacement was not equivalent: after one HOLD tick,
the CPU boundary returned without consuming the requested idle duration.
That attempt and temporary prints are removed, not worked around in production.
Both XT and D4 pass x64 individually and x86 together (xt-clock-owner,
d4-hold-public and input-clock-owner logs). Fixture classification still passes
all 140 constructors; the five package/dependency/negative gates pass.

At that freeze, two direct private fixtures still served Board DMA/RTC and
competition time users, and the two execution-time diagnostic memory readers.
The complete final build, integrations, eight EXEs, review and delivery remain
open; this focused batch is not a full-S qualification claim.

### Final private time-fixture removal

DMA/RTC authority now uses public run/deadline boundaries for actual elapsed
time and IRQ delivery. Separate Board input probes retain the one-tick-early
RTC assertions at 1, 8 and 16 MHz. Competition uses the existing bounded-idle
policy and actual transaction commit timestamps for BUSRDY, refresh and DMA
ordering, retaining the original timing bounds. The unused Machine-time
fixture link is removed too. Both private time-fixture files are deleted.
The three focused cases pass on both widths in final-time-owner build/test
logs; fixture classification (140 constructors), five boundary gates and
diff whitespace checks pass. No production API or clock owner is added.

### Approved diagnostic sampling adjustment

The owner approves paused-boundary diagnostic sampling rather than a new
public instantaneous observation contract. Retirement capture now keeps the
existing stopped-boundary target-byte read and removes its duplicate private
callback-time read. Its output explicitly names `sample=paused-boundary`.
The DOS boot probe remains in progress: copied write/event information must
not be confused with data read after pausing. This is not yet a claim that
both diagnostic executables build or that S7 qualification has completed.

The retirement-capture executable builds in both original Ninja graphs and
passes its existing ROM-free `--self-test` predicates on x64 and x86
(`t561-s7-<width>-paused-capture-build.log`). These are predicate checks,
not a fresh DeskPro firmware boot or desktop acceptance. The remaining DOS
probe still requires its paused sampling and public metadata migration.

### Paused diagnostic and final video receiver delivery

The DOS probe now runs one-instruction diagnostic quanta when collecting
retirement or Model40 write data. The retirement callback copies the existing
public observation only; the paused consumer performs all memory sampling.
The existing public debugger instruction record supplies CPU write addresses
and instruction-entry attribution. No new write observer, production API,
private layout, queue or Machine state is introduced. Paging or a full bounded
access record reports an incomplete sample instead of inventing a physical
address or silently claiming complete coverage. DMA/provider-side writes are
not included: output explicitly says `cpu-debug-record-not-bus-log`.

All existing guest-data sample locations remain. The old private ROM registry
dump and instantaneous transaction/hold fields are not re-created; actual
first-retirement bytes replace the reset-only private read, and the public
Board refresh observation remains. This is diagnostic evidence, not an
assertion that paused samples equal old instantaneous snapshots. The existing
wall budget is unchanged; finer sampling has greater diagnostic overhead and
is not a replacement performance/integration baseline.

The new ROM-free `--self-test` executes authored CPU code through the real
public Machine: two vector writes, no sample mutation during callbacks,
paused values 1234h/5678h, correct producer PCs and the first sample preceding
the second write. Both widths build both diagnostics and pass this test plus
retirement capture's existing self-tests (paused-probe/paused-capture logs).
This does not claim a new external-ROM or desktop run.

The final all-files include sweep found an additional legacy video-topology
fixture omitted by the earlier tracked receiver count. It now uses the
existing public bus probes and memory-route query, not Machine port/RAM
storage. Default/5170 reset after operational port probes before checking
power-on display state; DeskPro's gate/aperture checks retain their original
assertions. All four cases pass on both widths (video-public logs).

`test_core_read_physical` and its private Machine include are deleted. No
higher source/test file now includes a private Machine header. The separate
Board CPU/PIC white-box fixture family is still the proposal's named S8
receiver; it must not be mistaken for completed whole-T boundary closure.

The migrated but now unreferenced Machine `support/boot_fixture.c/.h` pair
is deleted rather than retaining a dead private-access shim. X86 Core's test
manifest now contains 173 files; its updated manifest check passes on both
widths after that deletion. No executable linked the retired pair.

### Final receiver unit freeze and production comparison

After the diagnostic/video repairs, complete repository units pass 632/632
on x64 (66.78 seconds) and x86 (59.78 seconds), including 165 X86 Core,
228 Core and all five Apps' registered unit cases. Logs use the
`final-receivers-unit-build`/`final-receivers-unit-tests` prefix. This is
full unit evidence, not yet four-profile integration/artifact acceptance.

The frozen-S6 production-body comparison normalizes only the reviewed
494-symbol map and declared paths. Thirty-three migrated files then match
exactly. The sole remaining body delta is `memory.c`: the removed arithmetic
facade include and two equivalent 16-bit casts in real-mode address formation.
The 41-line arithmetic header is retired, not exported. CPU/FPU source has
no S7 diff. The reproduction is `build/t561-s7-production-review.py`, with
its residual diff in `build/t561-s7-production-review.txt`.

Four-profile integration, original-graph restoration and eight PC artifact
qualification are now running via `build/t561-s7-qualify.ps1`. Full units are
not redundantly run inside that script; the final receiver freeze above owns
that proof. Delivery and complete-P review remain open until the remaining
results and assets have actually been checked.

The same comparison covers 68 outer production C/H receivers: 67 are exact
after the declared name/path substitutions. The remaining Board CMOS checksum
writer drops the retired arithmetic header and replaces its two 8-bit mask
macros with equivalent `lib_u8` casts. Thus neither the moved Machine bodies
nor their production receivers introduce a new algorithm, CPU/FPU behavior,
timing grade or fallback. The residual is retained in
`build/t561-s7-production-receivers-review.txt`; this equivalence check does
not replace the independent test/build/ownership review.

The final specialized-gate run exposed three checker receivers that the
ordinary unit aggregate does not execute: interpolated neutral READY/validator
names, the historical side of the physical-eligibility symbol mapping, and
the moved outer 80386 manifest runner. Their checks now point to the actual
owner while retaining their predicates and unchanged historical evidence.
The complete x64 specialized aggregate then passes (`gates-sweep.log`). The
same-class tool sweep also updates the Model339 callback ledger to the already
accepted before/after-internal Board callbacks; no production repair is needed.
Previously passed x64 profile integrations are retained, not rerun merely
because these static receiver checks changed.

## Actual Diff Review

The [file/hash review](t561-s7-file-review.tsv) records the source, test,
build, checker, tool and manifest bytes against S6. Its additions/deletions
deliberately disable rename detection, so moved bodies count on both sides:
production C/H totals +7,742/-7,785 (net -43); test C/H totals
+43,692/-41,478 (net +2,214). This is not 43,692 lines of new test behavior.
The declared path/symbol map and production equivalence review separate moves
from substantive changes. The behavior-level fixture/receiver ledger above
accounts for the substantive test setup changes and the approved diagnostic
sampling adjustment. CPU/FPU algorithms and product admission are unchanged.

The coordinator review also checks registration/link ownership, negative
dependency checks, removed helper callers, historical-token mappings and
current design paths. New files are included in the index-based vocabulary
sweep; whitespace review includes staged additions, not only previously
tracked files. The two dormant private boot helpers and the arithmetic facade
are removed rather than recreated as public APIs. S8 retains the specifically
named Board CPU/PIC fixture debt; this S does not claim that final boundary
already closed.

## Final Qualification And Delivery

Both original graphs now pass 632/632 repository units, all specialized gates,
48 receiving integration executions and the restored-default INI identity
test per width. The 48 comprise default's 35 (including 12 MyNES), XT's three,
AT's five and DeskPro's five; they are executions across the configured
profiles, not 48 different test definitions. DeskPro boot passes in 146.56
seconds on x64 and within its unchanged budget on x86. No failed check is
counted as a pass: the static receiver failures and their repairs are recorded
above, and the final aggregate actually exits zero on both widths.

All eight 0561 PC EXEs are rebuilt/deployed through the existing graphs and
pass actual published CLI/INI, version, release-count and PE-width checks.
The [artifact record](t561-s7-artifacts.json) freezes their sizes and SHA-256;
the NXVM implementation commit containing this record identifies their source
inputs, together with Shared `0da6818d5`. Both graphs return to default.
The 616 file-review entries match their actual current hashes/deletions.
Manifests, documentation governance, both extra executor/Model339 tool checks
and staged/unstaged whitespace checks pass. MyNES source/assets, Lib/Emulator,
external inputs and all INI/media/snapshot contents are unchanged.

The owner-approved diagnostic adjustment is implemented and both diagnostics
build and self-test on both widths. Automated registered desktop cases run as
part of the recorded suites; manual RDP, physical desktop interaction and a
fresh external-ROM run of the two diagnostic executables are not claimed.
Shared P1 `0da6818d5` and NXVM receiver/artifact P2 `b77d1f482` are pushed.
Coordinator actual-change review verifies their committed scope, the declared
production-body substitutions, behavior ledger, all 616 review entries and
eight artifact hashes against the original packet. I3/S7 is accepted, not
final I4/I5 or CPU C16/C17 qualification. The remaining Board CPU/PIC fixture
family stays explicitly assigned to S8; no final clean-package claim is made.
