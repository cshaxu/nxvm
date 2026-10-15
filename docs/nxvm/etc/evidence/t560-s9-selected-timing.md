# T560 S9 Selected 486SX Timing

## Frozen Stage And Owner Scope Decision (2026-10-15)

The owner approved stopping expansion and accepting a bounded stage after
actual-diff review, complete required regression and eight current PC EXEs.
This explicitly supersedes the original whole-C16 S9 exit; it does not claim
whole C16, C17 or complete 486SX qualification. The following chronological
stage reports retain their original verification limits. Final closure proof
is recorded separately at the end of this evidence, not inferred from those
intermediate runs. All remaining work has a complete [CPU successor](../../proposals/m5-486sx-timing-production-completion.md)
at queue position two after the Core refactor. C17 remains closed.

## Admission And Baseline

S9 consumes the whole C16 batch, not a subset selected by firmware progress.
The clean implementation baseline is `6621f09f011449e08cbe24b04951e001e37f5af6`.
S8's C15 semantics remain accepted at their recorded original/L2 grades.
C17 production admission remains closed. CURRENT owns the active packet.

## Initial Verified Findings

- `cpu_timing.c` has no selected-SX branch. The shared candidate chain finally
  reaches `core_machine_compatibility_instruction_cost`, which explicitly marks
  its result unallocated. A deterministic compatibility tick is not 486 proof.
- `cpu_timing_model.c` contains earlier-family tables and formula owners, but
  no selected-SX evaluator. Do not widen a 386 predicate or relabel its values.
- `cpu_instructions.c` owns the real cache/TLB state and physical access paths.
  They currently do not feed access-specific costs into a selected timing
  result. Cache/TLB test-register commands must continue using those same
  owners, not a timing-only mirror.
- `core_machine_cpu_execution_refresh` selects successful retirement timing
  before asynchronous interrupt entry reuses the decoder. Fault/interrupt-only
  execution currently reports delivery without a retired-instruction tick.
  C16 must explicitly qualify event-entry time rather than count a fault as a
  successful retired instruction or lose its elapsed clocks.
- Timing preview, actual execution and Core guest-time advancement require
  inspection together. A preview must not fill caches/TLB, modify page-table
  bits or consume hardware events merely to estimate cost.

## Primary Source

Intel *Intel486 Family Programmer's Reference Manual*, 1992, SHA-256
`D86207AAEBD9B10464F2058EFC61559E6141D3B072DB100E89BFE17C8187E143`.
The external original remains outside the repository. Relevant pages located
in that original are PDF 509 (printed 26-11), 510 (26-12), and 814--827
(E-2--E-15). Original-page visual review and table/footnote reconciliation are
required before implementing numeric values; extracted text is a locator.

26.2.2.3 describes approximate nominal instruction clocks under cache/TLB
hits, alignment, uncomplicated EA, no dependency/exception/write-buffer delay.
It does not authorize a hardware-exact claim for an emulator pipeline model.
Appendix E specifies a separate external bus model `r-b-w`; its table miss
penalties assume the fastest `2-1-2` bus. Non-default wait costs cannot be
folded into nominal clocks and then charged again by Core.

## Complete C16 Work Census

| Class | Required source and proof | Current disposition |
| --- | --- | --- |
| Integer/data/ALU/stack | E-3 onward, every legal primary/escaped/group form, widths and aliases from C15 | Nominal calculator and direct mode/form proof staged; final whole-universe allocation and penalties remain open. |
| Multiply/divide/bit/shift | Whole timing ranges, formula footnotes and operand-dependent cases | Original unsigned multiply, divides and scans plus explicit L2 IMUL/variable carry-rotate models staged; final source-grade publication and full dynamic costs remain open. No 386 formula relabeling. |
| Control/descriptor/task | Mode, gate, privilege/stack change, TSS/VM86 and destination-fetch costs | Actual gate/count/A-bit facts and nominal TSS/mode paths proved; dynamic and destination-fetch terms remain open. |
| Repeat/string/I/O | Primitive, zero count, setup/continuation, early stop, mode/IOPL and cache penalties | Mode-specific nominal and actual lifetime/partial-failure proof staged; complete dynamic penalties and selected cost publication remain open. |
| Selected late-SX/system | CPUID, VME, cache/test/control, SMM/RSM and power events, absent-FPU faults | Open: use admitted selected-source supplements where 1992 tables cannot define later features. |
| Fetch/cache | Actual hit/miss/uncacheable/line crossing and destination fetch; cache fill overlap assumptions | Actual successful fetch-bus baseline and cache/queue/preview/failure proof staged; complete publication, control association and overlap qualification remain open. |
| Translation | Actual TLB hit/miss, table cache state, Accessed/Dirty updates and page fault | Open: distinguish the E-2 no-table-cache-hit assumption from other cases. |
| Alignment/EA/dependency | Misaligned accesses, index, base dependency, displacement plus immediate and prefixes | Actual EA/immediate decode facts staged and lifetime proved; alignment, preceding-write identity and final additive terms remain open. |
| External bus/contention | Read/burst/write readiness, invalidate contention and write-buffer assumptions | Open: inspect existing external-cycle provider before proposing transport. |
| Exceptions/interrupts | E-13/E-14 task/INT tables, descriptor access, synchronous fault and asynchronous-only entry | Basic entry/setup and event-only Core transport staged and proved; access penalties, failed-entry chains and selected SMM costs remain open. |
| Preview/Core transport | Nonmutating preview and executed timing, complete result and one guest timeline | Open: inspect existing caller contracts and upper-bound behavior. |

## Boundary And Planned Review

The selected profile must have one explicit selector, source allocation and
result-publication owner. Existing CPU access/event owners supply actual
dynamic facts. Core supplies physical routing and external waits, not nominal
opcode costs. Older profiles keep their current independently qualified paths.
No Shared changes or second executor/timeline are admitted.

Initial production/test estimates remain the proposal's +450--900/-100--250
and +500--1000/-0--100. They are provisional, not quotas. Complete table/form
and caller inspection must refine this surface before numeric implementation;
larger necessary source/proof coverage must be reported rather than omitted.
No production code had been changed at admission. The following staged work
does not constitute an S9 complete P or production admission.

## Original-Page Review And Source Conflicts

Visual inspection completed for the complete original Appendix E integer,
control, string, system and I/O pages E-2--E-15 (PDF 814--827), including
both task/interrupt tables and all 23 integer footnotes. The ignored local
renderings are research scratch, not committed source or release material.

The pinned 1997 Intel developer manual, order 273021-001, SHA-256
`BE6242FD0A0B0663C27AA578C6BB7A3994DB21F1EC31887C2EA440D011B82BD3`,
is also visually reviewed at PDF 384--387 (printed 12-20--12-23) and PDF 453
(D-3). Its Table D-1 specifies CPUID leaf 1 as 14 clocks and leaf 0 or greater
than 1 as 9, not a blanket 14-clock implementation. It distinguishes SX from
SL/487 entries which do not implement CPUID. This supplements the already
admitted selected late-SX semantic contract, not all 486 silicon.

Two source inconsistencies constrain qualification; the later Bounded
Arithmetic Model Decision records explicit L2 alternatives, not a silent
reconciliation or a hardware-exact claim:

- The 1992 chapter-26 IMUL prose reproduces a 9-clock/`+6` formula and a
  three-clock memory addition, inconsistent with its own 13--42 table and
  Appendix E's `10 + max(log2(abs(m)), n)` and per-form miss columns. The
  later developer manual repeats the Appendix-E-style 13--42 rows, and also
  has a separate DX4-only faster block. Do not use the DX4 rows for SX or
  reuse the existing 386 formula just because it matches the prose typo.
  The exact optimizing operand, integer-log rounding, sign and miss terms
  still need a complete disposition before selected multiplication timing.
- Variable RCL/RCR memory range is printed 9--31, but E-14 footnote 5's
  large-count formula has the same `+9` as the register formula rather than
  an obvious extra clock. The later developer text repeats this shape.
  Preserve the conflict and inspect actual reference models/source limits;
  neither taking a range endpoint nor counting semantic rotate iterations
  automatically proves the microcoded time.

Neither conflict blocks independent nominal rows or dynamic-owner inspection.
No accuracy downgrade or unreviewed fallback has been introduced.

## Staged Nominal Implementation And Proof

The existing `cpu_timing_model.c` gains a private, pure selected-SX nominal
calculator declared only in `cpu_timing.h`, not a public interface header.
It consumes completed decoder/operand facts and does not reread memory,
mutate qualification, add access penalties or advance guest time. Unknown or
not-yet-reconciled forms return false without changing the destination.
The final selector has deliberately **not** been connected to this incomplete
calculator: doing that now would mislabel a cache-hit subtotal as complete
C16 time. The old compatibility state remains explicitly unallocated until
the selected complete route is ready. This temporary staging must be removed
by full S9 integration; it is not an accepted second production path.

Implemented source rows cover ordinary moves/extends, integer ALU, unary,
stack/flags, decimal adjust, simple shifts/double shifts, near branches,
real far transfers, primitive memory strings, I/O nominal mode cases,
descriptor/control/debug/test rows, XADD/CMPXCHG, BSWAP, divide, bit-test,
bit-scan and selected CPUID. REP aggregate costs, variable carry rotation,
multiply, complex protected transfers and event costs are still open.
EA and all prefix/access penalties are explicitly outside this subtotal.

`cpu_486_nominal_timing_smoke.c` currently proves 773 successful actual
instruction executions: 136 explicit source-table vectors; 432 mode/prefix
cases (real, CPL0--3 and VM86, both default code sizes and operand/address
overrides); five CPUID leaves; and 200 word/dword register/memory bit-scan
positions including zero. The bit-scan expectation uses independent explicit
source-derived position tables, not the production formula. Older-profile,
empty-input and null rejection preserve the output, and calculator calls
preserve the existing complete timing result.

Development failures retained here for traceability:

- Initial ENTER lookup incorrectly used the last captured byte; the actual
  decoder record can include a later fetch. It now consumes the known level
  byte at `prefixes + 3`, like the existing form owner, and checks its bound.
- The first BSWAP vector accidentally requested its explicitly rejected
  16-bit form; it now supplies operand-size override for the real 32-bit form.
- The initial word DIV vector inherited the reset signature in DX and
  correctly faulted on overflow. The nominal fixture now explicitly uses a
  zero high dividend; no production divide behavior was changed.

Both original persistent Ninja/ccache graphs build and execute this new
CPU-owned smoke. Final per-width results and adjacent/static checks are
recorded below after the stable staged snapshot. They are focused development
evidence only; S8's whole-suite pass is not an S9 whole-suite claim.

## Dynamic Transport Findings

`_kma_read_physical` already determines resident hit, actual fill, locked
bypass and uncacheable access. `_kma_write_physical` owns write-through and
resident-line updates. Selected timing must consume those actual decisions,
not query the final cache later and assume an initial hit. Speculative
preflight/observation must not count the same access twice.

`_kma_prepare_physical_linear` and `_kma_commit_physical_linear` distinguish
TLB hits, clean-entry first write, successful walk, permission failure and
locked A/D updates. E-2's 13/21/28-clock rule explicitly assumes neither page
entry is cached and no page fault. It cannot be charged blindly on top of
every actual page-table bus cycle, nor used to qualify the cached-table case.

The external-cycle BEGIN/COMMIT/CANCEL provider already has one Core owner
for board-specific extra waits. It receives only physical cycles and suppresses
preview effects. Reuse it; do not add another board-delay callback or include
the same wait in both CPU and Core subtotals. Current page-window policy is
an earlier-family contract, not proof of selected-SX burst overlap.

`core/x86/machine.c` currently gives successful retirement its selected CPU
cost plus external waits; a fault-only delivery consumes only already committed
external waits. Selected event elapsed time therefore needs an explicit result
transport through this one run boundary without incrementing `executed` or
publishing a false retired instruction. An asynchronous entry after retirement
must retain the successful instruction's result while adding its distinct
elapsed event cost. S10's general production admission does not excuse leaving
this mandatory C16 timing path unimplemented.

Base-register dependency must reflect a preceding destination write even if
its numerical value did not change; comparing old/new registers is insufficient.
Prefix, EA and displacement/immediate facts should come from the existing
decode, not a second standalone decoder. REP setup/continuation timing and
partial-fault costs must share the existing instruction lifetime and leave
S8's semantic pre-comparison-flags owner independent.

## Retained First-Stage Verification

The final staged nominal smoke passes on both host widths: x64 0.13 seconds,
x86 0.10 seconds for the test, with all 773 actual execution cases inside
each. Original Ninja/ccache graphs and default composition are retained.
The adjacent selection also builds and passes transfer-boundary, CPUID,
BSWAP and the retained 8086/80186 instruction-ledger and 80386 secondary
timing smokes. Core source/test ownership, Types and corpus checks pass.
The global fixed-width gate reports 1,296 code/script files and passes for
both graphs. Documentation governance and both Core manifests pass.

The first x64 grouped selection passed 12/12. The x86 selection passed its
eleven other rows but correctly rejected a stale test manifest after an
in-flight assertion-only test edit replaced padding-sensitive structure
comparison with explicit timing-field comparison. After that process finished,
the manifest was regenerated; both widths rebuilt the changed smoke and
passed the two affected smoke/manifest rows (0.88/0.87 seconds total). No
relaunch was caused by an observation timeout, and unchanged passing tests
were not rerun merely to hide this intermediate manifest failure. Freeze
test edits with their manifest before later grouped qualifications.

That first-stage size was production +357/-0 and tests +356/-0. It is
historical development evidence, not the current complete S9 diff.

## Operand, Control And Decode Stage

The next grouped implementation uses only the real decoder and execution
owners. ModRM/SIB decode records base-register identities, index and
displacement use; immediate decode records its actual presence. The normal
instruction boundary clears these transient facts. CALL/JMP records the
selected descriptor class and actual copied call-gate parameter count;
descriptor publication counts only successfully written, previously clear
Accessed bits. INVLPG records a valid matching entry before invalidating it.
No descriptor reread, second decoder, register-value mirror or TLB/cache
mirror is introduced.

The pure nominal calculator now covers E-15 I/O mode-specific primitive and
REP setup costs; unsigned MUL using the original 26-218 formula; LEA with or
without an index; memory-string first/zero iterations; protected far transfer,
task/gate and IRET routes; and INVLPG hit/no-hit. The existing repeat identity
comparison and integer ceiling-log helper are shared without changing older
profiles' formulas. REP continuation accounting still awaits the selected
retirement integration; first-iteration checks do not prove its full lifetime.
The IMUL and variable carry-rotate source conflicts above remain open.

The nominal smoke now contains 1,606 cost checks through real execution:
145 explicit vectors, 432 mode/prefix variants, 200 bit scans, five CPUID
leaves, 408 unsigned multiplies, 192 I/O cases, 80 first/zero REP cases and
144 protected transfers. It also executes 1,660 decoder/lifetime witnesses,
including every ModRM/SIB form across code/address-size combinations and
clearing on the next instruction. These are internal cases, not 3,266 CTests.

Existing owned tests receive direct nominal checks, not duplicate fixtures:

- `cpu_outer_return_smoke.c`: the existing 32 call-gate cases check the real
  gate class, copied argument count, and 69 versus 85 nominal clocks.
- `cpu_tlb_smoke.c`: all 96 instruction forms execute a resident INVLPG and
  the same now-absent INVLPG, expecting 12 then 11 clocks while retaining the
  no-target-read/no-write assertions and actual test-register lookup proof.
- `cpu_task_switch_cross_width_state_smoke.c`: ten selected-SX task transitions
  check incoming 16/32-bit TSS, CALL/JMP/task gates and nested returns. The
  existing 80386 semantic cases still execute unchanged.
- `cpu_task_switch32_state_smoke.c`: 2,560 existing selected-SX completion
  cases check protected TSS32 timing; 384 existing VM86-entry cases check
  incoming VM86 timing across direct, CALL, task gate, IDT gate and IRET.
  Incoming TF/RF and other flags remain part of the semantic assertions.
  Fault-delivered INT1 is not mislabeled a successful instruction subtotal.

Two development failures were fixture defects, retained rather than hidden:
the first protected I/O-string fixture used null DS/ES selectors, and the
first indirect protected-transfer fixture used null DS. Both correctly
raised GP. Explicit valid selectors/DPL repair the fixtures, not CPU exception
semantics. The corrected smokes pass on both widths.

Before final grouped checks, the four-test selection (nominal, outer return,
TLB and old 80386 secondary timing) passes 4/4 per width, 1.51/0.48 seconds.
The two task-switch tests then build/pass individually on both widths,
0.18/0.14 and 0.26/0.21 seconds total respectively. These are focused CPU
development results, not complete repository qualification.

The frozen control-stage grouped selection passes 13/13 per width, 8.86/8.72
seconds, including both Core manifests, corpus, Types, ownership and route
partition positive/negative gates, all five selected-SX CPU smokes above and
the retained 80386 secondary-integer timing smoke. Ignored JUnit evidence is
`build/t560-s9-research/control-stage-{x64,x86}.xml`. Documentation governance
and `git diff --check` pass. No passing static check discharges the remaining
runtime selection/preview/event or complete-C16 coverage obligations.

After removing three unnecessary native scalar casts from the new diagnostic
prints, both task-switch targets rebuild and the two affected smokes plus
test manifest pass 3/3 per width, 1.50/1.38 seconds. Global fixed-width
vocabulary passes for both original graphs (1,296 tracked code/script files).
The final wider CPU-owned regression then rebuilds all 98 binary targets and
passes all 104 selected CTests per width, 17.78/7.91 seconds with four workers.
The remaining six rows are existing CMake-script CPU boundary checks, not
binary build targets. JUnit records are
`build/t560-s9-research/cpu-stage-{x64,x86}.xml`.
The first target-list construction mistakenly included the CMake runner as
a Ninja target and terminated before any test; excluding script runners from
the binary build list corrected the orchestration without skipping their
CTest execution. No observation timeout caused a restart.
This wider set includes inherited instruction semantics, paging/cache,
SMM/VME, context/preview and two Board-Base CPU integration witnesses. It is
not the complete repository suite or the final S9 qualification.

At the end of this control stage, source/test-only diff versus admission,
including the new source file
and excluding docs/manifests/generated/artifact files:

- Production: +533/-11, net +522, in the existing instruction C/private
  header and timing-model C/private header.
- Tests: +766/-5, net +761, including the new 695-line nominal smoke,
  four existing CPU tests and the one registration.
- No public API, enum, persistent hardware state, thread or second executor
  is added. The new decode facts are instruction-local observations, cleared
  at the existing boundary; the private calculator is only a nominal subtotal.

The next access/event batch must preserve these decisions: capture actual
resident/fill/bypass results at physical access, logical alignment before page
splitting, and translation hits/walks at their existing owners. E-2's cold
page-table walk and E-3--E-13 form-specific miss terms must not double-charge
the same bus work or Core's external wait provider. Successful retirement is
selected before asynchronous entry reuses the decoder. Event-only elapsed
time must cross that same Core boundary without inventing a retired instruction;
partial/failing entry and nonmutating preview remain required direct proofs.

## Committed Event Subtotal And Core Transport Stage

The existing `core_machine_cpu_timing_result` gains one appended `lib_u64`
`event_ticks` field. The CPU clears it with the existing per-refresh result,
preserves entry before retirement selection (for task debug traps), and adds
asynchronous entry after successful timing selection. `ticks` is still the
successful-instruction subtotal; event-only delivery keeps it zero. This is
an existing Core value-contract extension with a real Core/x86 consumer,
not a public operation, hardware-state mirror or second timing axis.

The single E-14 entry table now serves both software-interrupt nominal cost
and actual event entry. E-13 supplies NMI/INTR setup and page/bounds/VM86-GP
setup at the original opcode, before handler entry changes mode. A PIC which
returns vector 2 is still INTR, not NMI: the existing external caller passes
the actual origin. Older profiles do not acquire selected-SX event clocks.
These remain nominal entry/setup subtotals; they do not qualify missing cache,
TLB, descriptor-access, partial-entry or selected SMM terms.

Core/x86 consumes event-only clocks through its existing pending-wait owner:
external waits drain separately; the event completion observes budget and
overflow checks and advances the existing scheduler without incrementing
`executed` or publishing a retired instruction. A retired instruction followed
by entry retains one retirement and includes the separate event delta in its
elapsed round. One private publication-origin enum member prevents event-only
completion being traced as CPU retirement. The physical-source publication
gate is not widened; complete source qualification and C17 admission remain
mandatory. No persistent pending field, thread or manager is added.

Direct evidence extends existing test owners:

- CPU fault/event smoke: 70 actual real/protected/same-level/inner/VM86 routes
  over 386 and SX, including NMI, INTR, PIC vector 2, HALT wake, provider
  acknowledgement failure and next-round clearing; 22 canonical VM86-GP
  operations independently check the E-13 setup table. Existing fault/frame
  and older-family checks remain.
- Executor smoke: five actual CPU-to-Core cases prove event-only 26 clocks
  with zero retirement, NOP plus NMI with one retirement, 386 retention,
  no next-round duplicate time and elapsed-counter overflow rejection.
- Prefetch/locality smoke: retain 386's committed data-read wait; SX's
  committed fetch supplies one external wait, followed by its 26-clock GP
  entry. A 25-clock budget preserves the completion; a 26-clock budget
  consumes it once, leaving total elapsed 27 and zero retirements.
- Task-state smoke retains a real regression for task-load debug entry
  failure followed by ordinary GP entry. The newly wired subtotal initially
  returned 199 instead of 44 because the instruction-wide task-switch marker
  was mistaken for this event's route. The actual IDT owner now returns its
  task-gate choice through a caller-local output; no mirrored route state is
  added. Task nominal cost reads the existing incoming-task checkpoint, not
  a CPU mode subsequently changed by the debug handler.

The first SX external-wait fixture wrongly expected MOVS to read its source
before validating the bad destination. It actually reported ticks=0,
pending=true, completion=26: the already-qualified precise preflight was
working. Only the SX fixture's explicit wait window moves to the preceding
actual fetch; the 386 data-read witness and production ordering are retained.
The corrected five-test batch passes both widths (3.67/1.05 seconds), then
the task-route correction passes its seven-test x64 batch (3.36 seconds).

The frozen Core-labelled selection rebuilds all 359 test/core binary targets
per width, plus global Types vocabulary, on the original graphs. Both full
runs pass all 380 Core unit registrations and one of their two labelled
integration rows; the other row, `integration.session-ini-status-smoke`,
returns `OPEN-FAILED:1`. Initial aggregate results are therefore 381/382,
167.27/37.88 seconds, not an all-green run. The selection's two root-level
integration executables were omitted from that binary build list. Explicitly
rebuilding both for the current restored-default graph makes their original
commands pass 2/2 per width (1.76/0.64 seconds), without a source or INI repair.
The stale executable was not retained to independently identify its previous
embedded profile, so that more specific cause is not asserted. Future grouped
build inventories must include every selected repository-owned executable,
not just those beneath test/core.

Ignored JUnit records are `core-event-stage-{x64,x86}.xml` and
`core-ini-event-stage-{x64,x86}.xml` beneath `build/t560-s9-research/`.
This proves the 380 Core units and the requalified two Core-labelled INI
integrations at this stage, not all repository tests or all four profiles.
Global Types vocabulary, Core manifests/corpus/ownership/route gates and
documentation governance pass. Original Ninja/ccache graphs, -j8 and four
CTest workers are retained; no observation timeout causes a process restart.

Root integration prerequisites also automatically deploy the freshly linked
default NXVM pair. Those two uncommitted outputs are deliberately restored
to accepted S8 HEAD, preserving the whole released eight-EXE baseline until
complete S9 qualification. The newly built pair remains intact in the original
build graphs; SHA-256 before restoration matches each auto-deployed copy.
No source/input/user change is discarded. Shared six, MyNES, released assets,
INI, external firmware/media and snapshot inputs end this stage unchanged.

Current source/test-only diff versus S9 admission, excluding manifests/docs
and generated or released artifacts, is production +650/-54 (net +596) and
tests +1,015/-12 (net +1,003), including the new 696-line nominal smoke.
The event stage itself adds +117/-43 production and +249/-7 tests relative
to the control-stage snapshot; the additional proof covers VM86 setup,
committed waits/budgets and the actually reproduced task-route defect.
The only public addition is the existing result's event field. Two private
numeric operations share E-13/E-14 ownership with the instruction model;
the private route output and publication origin have direct consumers.

S9 stays active and uncommitted under the complete-P rule. Remaining work is
whole legal-form/source-conflict disposition; actual cache/TLB/alignment,
EA/prefix/dependency, branch/REP and contention costs; failed/nested entry and
trap-retirement reconciliation; selected SMM/RSM time; and the sole selected
runtime/preview path. Nested task-gate/trap chains still require direct timing
proof rather than an inference from the final task checkpoint. Complete
dual-width repository/profile qualification and all eight current PC EXEs
remain required before a complete implementation P and acceptance.

## Actual Access Accounting Stage

The next grouped stage observes completed selected-SX accesses at their existing
owners. Physical reads distinguish real data misses, page-table misses and
LOCK-bypassed data reads before a fill changes residency. Failed reads are not
counted as completed misses. The existing locked A/D updater records attempted
rereads and successful writes in its local translation record; committed walks
publish a three-bin histogram for zero, one or two A/D writes. Completed linear
word/dword accesses record misalignment once, not once per physical split.
The existing instruction boundary clears all facts. No cache/TLB state is
duplicated, no extra provider lookup is performed for timing, and old-family
access and page-update behavior remains unchanged.

The private `core_machine_80486_memory_penalty` consumes those facts without
rereading guest memory. Its staged source allocation is deliberately explicit:

- Ordinary MOV E-3 loads: two clocks for one completed data-read miss.
- Completed unaligned word/dword operands: E.2's three additional clocks.
- E.2 cold page-table walks: 13/21/28 clocks for zero/one/two A/D updates,
  only when both paging-entry reads missed and every attempted update wrote.
- MOV stores, immediate/register MOV, NOP and LEA have no invented data-read
  penalty. Fetch-cache, prefixes, EA/dependency and event costs are separate.
- Multiple cold data lines, cached paging entries, raced locked rereads that
  need no write, other instruction families and LOCK RMW remain unallocated
  here. The operation returns false and leaves its output unchanged rather
  than guessing a number. These remain required C16 work, not accepted omissions.

One existing Cache fixture gains an explicit page-table-cache eligibility
control. Its previous behavior is unchanged by default. Actual CPU execution
covers moffs, ModRM and immediate-store forms, byte/word/dword operands,
aligned/in-line-unaligned/cross-line addresses, cold/warm execution, implicit
LOCK, and next-instruction clearing: 90 operand checks plus 45 NOP clears.
Each operand execution also proves lexeme preview leaves CPU, instruction
facts, execution/cache/TLB state and real data read/write counts unchanged.
Nine translation scenarios check first access and TLB-hit repetition, including
six cold read/write A/D combinations, two cached-entry cases and a raced
directory update. Existing failed-publication tests now assert that failed
accesses do not publish successful miss/alignment facts. Null, empty and old
profile guards retain an unchanged output. No new test framework is introduced.

The first x64 six-test run fails in the new cached-entry test: its PTE was
actually in the next 16-byte cache line, so the real observation correctly
reported one table miss instead of the fixture's expected zero. The test now
places the data PTE in the warmed code-PTE line. This fixes the actual cache
setup, not the calculator's rejection or its expected cost. The original
`access-stage-x64.xml` failure is retained; the corrected group passes 6/6
per width, 0.48/0.66 seconds. No production workaround is added for the fixture.

The next frozen CPU-owned selections pass 104/104 per width, 14.70/7.67
seconds. x86 builds all 98 selected binary targets. The initial x64 build
filter excludes three differently named decoder binaries; they are explicitly
relinked and rechecked 3/3 (0.85 seconds), rather than claiming those stale
executables were rebuilt by that filter. Final test-only expansion to all
MOV forms passes the affected Cache smoke per width, 0.20/0.16 seconds.
Ignored JUnit records are `cpu-access-stage-{x64,x86}.xml`,
`decoder-access-stage-x64.xml`, and `cache-access-final-{x64,x86}.xml` in
`build/t560-s9-research/`. Both original graphs pass global Types vocabulary
over 1,296 tracked files. This is CPU-owned regression, not a complete repo or
desktop/profile release qualification.

After the actual-diff review makes the histogram's intermediate multiplication
explicitly 64-bit, both models and the six affected binaries rebuild. The final
six-test selection plus six Core manifest/corpus/ownership/route checks passes
12/12 per width, 3.30/3.65 seconds. The records are
`access-final-static-{x64,x86}.xml`. Both Core manifests, documentation governance
and `git diff --check` pass. A mistaken direct test-manifest script path was
corrected to the existing Core verifier with `CORE_ROOT=test/core`; no test
or verifier implementation was changed to accommodate that command error.

At this stage, source/test-only diff versus admission is production +743/-55
(net +688) and tests +1,197/-13 (net +1,184), including the new 696-line nominal
test. Relative to the prior event stage, the access implementation adds
+93/-1 production and +182/-1 tests. The sole new operation is private and
has the final selected timing calculator as its required S9 consumer; it is
not installed as a second runtime timing selector. Six instruction-local
counter members (including the three-bin array) and two local translation
counters describe real access outcomes, not persistent hardware mirrors.
No public interface, enum, thread or manager is added by this stage.

S9 remains active and uncommitted. The compatibility selector remains visibly
unallocated for SX until the complete source/form and dynamic allocation is
ready. Shared six, MyNES, released S8 EXEs and owner INI/ROM/media/snapshot
inputs remain untouched. This work neither closes C16 nor admits C17.

## Prefix And Effective-Address Subtotal Stage

The private `core_machine_80486_decode_penalty` now consumes the existing
completed decode facts without another operand decoder or guest-memory read.
E-11 assigns one clock to each ordinary segment, size and LOCK prefix. A
single REP prefix on a string is already included in that instruction's
setup; other ordinary prefixes are still counted. Redundant REP and REP on
non-string instructions remain explicitly unallocated here. The operation is
per decoded macroinstruction: final selected REP integration must charge it
on setup, not again on every continuation.

E.2 assumptions 5 and 8 describe a possible one-clock index term and a
possible one-clock displacement-plus-immediate term. This staged deterministic
allocation adds each when its actual decode condition is present. Those
assumption-based additions require explicit L2 qualification in the eventual
selected result; they are not a claim to reproduce every undocumented pipeline
overlap. Indexed LEA already has a separate two-clock nominal row, so its
index term is not added twice. The direct real-mode far-transfer rows already
include note 22 and have no decoded ModRM displacement; the test proves no
extra guessed immediate/address term is added to them.

Actual-execution tests cover 512 ordinary-prefix combinations across 16/32-bit
code, 15 duplicate-prefix lengths through the 15-byte instruction limit,
12 focused address/immediate/LEA/transfer/REP forms, the existing 1,658 complete
ModRM/SIB forms and two next-instruction lifetime checks. The existing Cache
LOCK matrix adds 36 checks, distinguishing implicit locking from an explicit
LOCK prefix and retaining real bus/lock assertions. These are 2,235 numerical
decode-subtotal cases, not 2,235 new CTest registrations. Twelve repeated
calculator calls also compare whole CPU/instruction/execution state for purity;
null, empty, old-profile and unallocated-REP cases keep an unchanged output.
No hardware state, public API, metadata enum or test framework is introduced.

The two directly affected smokes pass 2/2 per width (0.32/0.25 seconds).
The frozen broader selection builds all 98 binary targets on both original
Ninja/ccache graphs, then passes 110/110 per width (16.87/9.73 seconds):
104 CPU-owned regression rows plus six existing manifest/corpus/ownership/route
checks. Unlike the prior build filter, the inventory includes all three
differently named decoder binaries and rejects an incomplete target count.
Both global Types vocabulary gates pass. Ignored JUnit records are
`decode-stage-{x64,x86}.xml` and `cpu-decode-final-{x64,x86}.xml` in
`build/t560-s9-research/`. No claim is made for unexecuted full repository,
desktop/RDP, four-profile integration or new release qualification.

The actual-write census explains why AGI is not silently folded into this
subtotal. The current file has 311 direct named-GPR assignment statements
across 105 containing functions, including earlier-family branches; this is
not a count of live SX paths or proof that all writes were enumerated.
Explicit reference writes, successful stack reads into GPR destinations,
implicit ALU/string/stack updates, task/SMM restoration and event boundaries
also need one coherent preceding-write lifetime. Comparing old/new values
would miss same-value writes such as MOV EAX,EAX. Instrumenting only the
reference-write helper would omit real owners. No incomplete AGI history,
second semantic decoder or guessed dependency cost is added by this stage.
The complete actual-write census, adjacency and reset/fault/event treatment
remain mandatory C16 work.

This stage adds +44/-0 production and +116/-1 tests relative to the prior
access stage. Source/test-only diff versus admission is now production
+787/-55 (net +732), tests +1,313/-14 (net +1,299), including the new 808-line
nominal/subtotal test. Changes are confined to the existing private CPU timing
C/header and two existing CPU tests. Source ownership, minimal interface and
actual-diff review find no extra public path or hardware-state mirror. The
private subtotal's final selected consumer is still required within S9; passing
this staged proof does not install an incomplete second retirement selector.

S9 remains active and uncommitted. AGI, full access/fetch/branch/REP allocation,
remaining source conflicts, SMM/event details, sole runtime/preview selection
and complete release qualification remain open. Shared six, MyNES, accepted
S8 assets and owner inputs remain unchanged.

## Adjacent Register Writes And AGI

The actual-write census is implemented at the CPU's existing destinations,
not as a second opcode-effect interpreter or a comparison of changed values.
A private inline destination operation records the canonical GPR bit for
byte/word/dword lvalues, successful reference copies and successful logical
reads into GPR destinations. Compile-time layout assertions protect its eight
slot mapping. Three typed lvalue macros preserve the original assignment
width and expression, including compound assignments and postfix decrement.
All 311 named direct assignments are mechanically instrumented. Reversing the
macro replacement reproduces each original line byte-for-byte; no original
RHS, condition or execution ordering is rewritten by that transformation.

BSWAP's pointer destination now uses the existing reference-write owner.
The SMM bulk restore explicitly records all eight destinations. DAA/DAS need
an explicit AL-destination observation even when their original numerical
implementation skips a redundant decimal adjustment. These observations do
not alter the arithmetic/flags algorithm. String/port index movement and XLAT
supply their actual implicit address bases; actual SS reads/writes identify
the stack-address dependency. There is no extra memory query, value mirror,
public interface or execution path.

Four private instruction-record members hold the current/preceding write
masks, the preceding decoded opcode pair, and actual stack access. The
existing boundary advances the one-instruction history; older profiles do
not collect or scan it. The decode subtotal applies E.2's one-clock base
dependency. Its PUSH/POP exemption removes the SP adjustment dependency,
not a POP's non-SP result used by the following memory operand. The index
term remains separate from the base-register condition in the source.
These facts also make same-value writes and AL/AH/AX/EAX aliases observable.
Final selected source/model qualification and REP setup/continuation handling
remain required; this is not a claim to model every internal pipeline overlap.

Successful exception/NMI/INTR entry breaks the preceding-instruction history;
failed or stalled execution cannot publish partial GPR writes to the next
instruction. Reset uses its existing whole-record clear. Successful debugger
patch and committed prepared-entry replacement discard the old dependency;
rejected patches and aborted entry replacement retain it. The prepared-entry
case was found by the same-owner whole-CPU replacement sweep, not by adding
a generic reset manager. SMM restore and actual task-load assignments stay
at their existing architectural owners. Older-family arithmetic/flags and
timing tables are unchanged.

The expanded CPU smoke executes 61 fixed writer vectors plus the complete
register/width/write-form matrix, eight BSWAP destinations and fourteen
accumulator exchanges. CPUID checks its four destinations once; the other
writers each feed eight real address-base instructions without host register
changes between producer and consumer. This gives 1,425 writer-mask checks
and 1,424 actual adjacent address consumers. Nine focused pairs cover
same-value SP writes, PUSH/POP, POP-to-address dependency, NOP separation,
masked-zero shift, base versus index, and string addresses. Five numerical
boundary checks cover exception/NMI, debugger patch and prepared-entry
commit/abort. Additional assertions retain rejected-patch history, clear it
at Reset, and reject the partial writes of a provider-failed ENTER after a
real stack write. The existing SMM state test directly checks the bulk-restore
write mask. No fake register-mutation callback or new test framework is used.

The first model build rejects an integer-promotion signedness mismatch in
the packed opcode's conditional expression; using `lib_u32` before shifting
fixes it without suppressing warnings. The first three-test x64 run passes
Cache/SMM but fails the new DAA mask assertion (zero rather than AL). The
original arithmetic result is not defective: it performs no redundant AL
assignment in that branch. The explicit DAA/DAS destination observations fix
the new instrumentation gap. The corrected group passes 3/3 (1.28 seconds),
then the expanded lifecycle/prepared-entry group passes 4/4 (1.51 seconds).
The original `dependency-stage-x64.xml` failure is retained, together with
`dependency-stage-recheck-x64.xml` and `dependency-stage-final-x64.xml`.

Both original Ninja/ccache graphs build all 359 binary targets for the frozen
Core-unit selection, with -j8 and four CTest workers. Both full runs pass
380/380 units, zero disabled and zero failures, in 176.10/36.57 seconds.
The selection explicitly excludes the two integration-labelled root INI
tests, rather than accidentally running their old binaries or auto-deploying
a partial default release. JUnit records are
`core-dependency-stage-{x64,x86}.xml`. This proves the complete current Core
unit selection, including earlier-family ledgers, boards and new observation
facts, not a complete repository/profile release or host-speed benchmark.

Both Types gates pass over 1,296 code/script paths. Inspection confirms that
the existing gate enumerates both Git-cached and untracked paths, so the new
nominal smoke is included despite the emitted message saying "tracked".
The two new S9 files are subsequently registered as intent-to-add for complete
diff visibility only; no contents are staged, committed or delivered. The
Types rerun remains green. Both Core manifests, corpus/ownership/route gates
and the existing older-family timing assertions pass. Historical CPU `-w`
exceptions remain unchanged; this is not a claim of warning-exemption-free
qualification for those old files.

Source/test-only diff versus admission is now production +1,217/-369
(net +848) and tests +1,609/-14 (net +1,595), including the new 1,103-line
nominal/subtotal smoke. This stage's net addition is 116 production and 296
test lines; most source replacement is the reversible lvalue instrumentation.
Per-stage figures above describe changes to the cumulative admission diff,
not separately committed P patches. The complete S9 diff and qualification
will be reviewed before its complete implementation P.

The AGI destination/boundary mechanism is now exercised, but whole C16 remains
open: REP continuation integration, remaining access/fetch/control costs,
source-conflict disposition, complete entry/SMM time and the sole selected
runtime/preview path. No complete S9 or C17 admission is claimed. Shared six,
MyNES, owner inputs and released S8 EXEs remain unchanged.

## REP Phase And Lifetime Stage

Selected SX now has one explicit selector branch instead of passing through
earlier-family candidates which can clear the repeat record. Its cost still
uses the explicitly unallocated compatibility recipe: this stage does not
qualify a partial nominal sum as complete hardware time. Replacing that
recipe with the complete C16 cost remains mandatory before the complete P.

The existing repeat owner records primitive, zero-count, first and continuation
phases. Older families reuse the extracted recording operation with the same
identity, numerical tables and setup/iteration behavior. SX additionally saves
the actual prefix/opcode bytes in its private repeat identity, so changing a
segment prefix at the same CS:EIP cannot inherit the previous setup. There is
no new repeat scheduler, public API, event, queue or counter. Ordinary prefix,
EA and AGI setup terms are not charged again on a continuation. Nominal string
and port queries use the published current phase, not whether another iteration
will remain active after this one.

Actual exception/interrupt entry, provider-failed execution, successful debugger
patch and committed prepared-entry replacement terminate SX's repeat identity.
Rejected patches, aborted entries and a stopped clock do not. Existing reset
and SMM/RSM owners retain their clearing behavior. The new smoke executes 896
memory-string cases across code/operand/address widths, direction and count,
plus 256 port-string cases across real/protected/bitmap/VM86 permission modes.
It also checks comparison early termination, primitive execution, reset, byte
changes, NMI/IRET restart and an actual failing provider read. No simulated
retirement callback or new test framework is introduced.

Three new-test assumptions fail during development and are corrected without
weakening production behavior: a byte-modified MOVS restarts setup but retains
the preceding SI/DI write dependency (two decode clocks, not one); asynchronous
entry reuses the decoder, so the test must inspect the already-published
retirement and 29-clock real NMI entry instead of recalculating nominal timing
on that reused record; and a default 16-bit SI wraps instead of exceeding the
fixture's RAM. The latter fault test now uses an actual address-size override
and a valid segment limit, then observes the second read failing beyond RAM.
The original failed JUnit reports are retained under the ignored research
directory. The corrected four-test x64 group passes 4/4 in 0.47 seconds.

Both original Ninja/ccache graphs subsequently rebuild all 360 Core unit
binaries and pass 381/381 unit rows, with no failures or disabled cases, in
174.55/43.11 seconds. JUnit records are
`core-repeat-stage-{x64,x86}.xml`; Types passes over 1,297 paths in both graphs.
The two Core manifests and production corpus boundary check pass. This is
unit-only background verification, not integration, desktop/RDP, a complete
repository release or a performance comparison.

Cumulative source/test diff versus admission, excluding manifests/docs/assets,
is production +1,308/-389 (net +919), tests +1,943/-14 (net +1,929), including
the new 333-line repeat smoke. The existing untracked repeat test is included
in those counts and in the Types/manifest qualification. No partial P or new
artifact release is claimed. Whole C16, source-conflict reconciliation,
remaining dynamic/entry/SMM costs, complete selected runtime/preview and full
repository/profile/artifact qualification stay open. Shared six, MyNES and
owner inputs remain unchanged.

## Ordinary Integer And Bit Operand Cost Stage

Original-page review of E-2--E-6 and E-9 confirms that the existing MOV-only access
subtotal cannot supply the rest of the ordinary integer rows. One private
operand-cost helper now owns those cache-miss coefficients; the existing
memory subtotal remains the single owner of combining them with committed
translation and alignment facts. No bus operation, execution handler,
earlier-family selector, public API or persistent state is added or changed
by this stage.

E-4 distinguishes a two-clock read miss from an unlocked six-clock read/modify/
write miss and a two-clock locked transfer. E-6 gives SHLD/SHRD immediate
and CL forms different coefficients, XADD the unlocked/locked pair, and
CMPXCHG two clocks independent of comparison outcome. E-3 note 2 already
includes the always-locked memory XCHG transfer in its nominal five clocks;
it must not receive an invented extra miss cost. Its prior explicit
unallocated assertion is replaced with the source-backed zero **additional**
cache cost while retaining proof of the real locked read. MOVSX/MOVZX,
ordinary shifts/rotates and the independent E-5 multiply/divide operand
columns are also allocated. A blank additional-cost column does not mean
zero instruction time: the unresolved IMUL and variable RCL/RCR nominal
formulas are still unallocated in the nominal owner.

The same-owner sweep also allocates E-9's bit rows: BT immediate/register
indices have one/two-clock miss costs; modifying bits uses two/zero for an
unlocked/locked immediate-index operand and three/one for a register index.
BSF and BSR memory sources add two and one clocks respectively. These stay
at the same operand-cost owner and use the existing real effective-address
and LOCK observations, not a bit-operation-specific timing path.

The helper consumes actual successful cache/LOCK observations, never rereads
guest operands or infers a cold cache from a memory opcode alone. Multiple
cold operand lines and multiple locked parts remain explicitly unallocated,
leaving the destination unchanged. Strings, stack, descriptor and control
accesses cannot inherit a coefficient just because they happen to use memory.
Existing E-2 cold page-walk qualification and committed Accessed/Dirty-update
checks remain intact. E-2 charges three clocks per misaligned access: a
misaligned read/modify/write has both a read and a write, not one operand-wide
counter. Actual alignment observations retain that distinction.

The existing cache fixture gains 87 source-derived encoding rows and executes
1,124 byte/word/dword, register/memory, cold/warm, explicit/implicit LOCK and
CMPXCHG-result variants. Each successful query is repeated with whole CPU,
decoder and execution-context comparisons plus bus-read/write counts, proving
that this private calculation has no hardware-state side effect. Ninety-six
separate alignment/cross-line variants cover read-only and read/modify/write
forms, including warm split operands and rejected multi-miss/locked-part
combinations. Four provider failures retain the accepted read prefix while
refusing a successful cost after the instruction fails. The test uses actual
KMA/cache/bus execution, not directly assigned timing counters.

The initial three-test x64 group and extended alignment test both pass.
Both original graphs then rebuild all 99 binaries for the frozen selection
and pass 111/111 CPU/static rows with zero failures or disabled cases, in
16.19/9.77 seconds (`integer-access-{x64,x86}.xml`). The bit-row extension
subsequently repeats that complete CPU/static qualification and passes
111/111 per width in 15.20/9.54 seconds, recorded in
`integer-bit-access-{x64,x86}.xml`. Both Types
gates pass over 1,297 paths, and Core manifests, corpus, ownership and route
checks pass. This grouped private-calculator stage does not rerun every
Core board unit, repository unit, integration or desktop test. The preceding
REP stage's complete Core results remain separately recorded, not relabeled
as qualification of this newer source snapshot.

Cumulative source/test diff against admission is production +1,419/-389
(net +1,030), tests +2,218/-14 (net +2,204), excluding manifests/docs/assets
and including the untracked repeat smoke. One private helper owns the new
cache-column classification; no new public contract, persistent observation
field or framework is introduced. Complete C16 runtime/preview, remaining
fetch/translation/control/string/entry/SMM costs, source conflicts and final
repository/profile/artifact qualification remain open. Shared six, MyNES,
owner inputs and S8 releases stay unchanged; no partial P is delivered.

## Bounded Arithmetic Model Decision

The same-session coordinator freezes the following L2 choices under the
owner's standing approval for timing upgrades. These replace previously
unallocated costs, not an already-qualified original timing implementation.
They do not resolve the conflicting Intel editions into a claim about fitted
silicon. The source discrepancies above remain part of their qualification.

- IMUL uses Appendix E note 3's `10 + max(ceil(log2(abs(m))), n)`, with
  `n = 3` for nonnegative and `n = 5` for negative multipliers; zero uses
  the 13-clock minimum. Chapter 26 supplies integer rounding, not its
  contradictory old `+6` baseline or three-clock memory surcharge.
  The model's optimizing multiplier is the encoded final multiplicative
  source: r/m for one/two-source encodings, immediate for 69/6B. The 6B
  immediate is signed eight-bit; other values use their actual operand width.
  This operand choice is an explicit model assumption, not evidence that
  real silicon selects the same internal early-out operand. Per-form memory
  columns stay at the already separate access owner.
- Variable RCL/RCR uses the five-bit masked count before the semantic
  modulo-by-width-plus-carry operation. At counts no greater than operand
  width, register/memory costs are eight/nine clocks. Larger counts use
  the printed note 4/5 formula `floor(count / width) * 7 + 9` for both.
  Thus the memory model follows the equation, not the conflicting printed
  maximum of 31. Fixed implicit-one opcodes retain their original three/four
  clocks at the nominal owner. No semantic rotate loop is counted as time.

One private L2 arithmetic evaluator owns only these encodings; the
original nominal evaluator must continue declining them rather than calling
the model a directly specified cache-hit row. The eventual sole selected
publisher must expose the existing L2 dynamic-arithmetic origin when this
model contributes. No new public API/type, mutable flag, timing path or
compatibility fallback is authorized by this decision. Direct execution,
independent numerical vectors, purity, width/prefix/memory forms and the
unchanged older-family formula are required proof before delivery.

## Arithmetic Model Implementation And Proof

The CPU-private `core_machine_80486_l2_arithmetic_cost` now implements the
frozen models. It neither changes CPU instructions nor publishes retirement
time. Its encoding set is disjoint from the original nominal evaluator;
tests require that the latter continues declining these forms without
modifying its output. Both reject missing inputs, other profiles, failed
instructions and forms owned elsewhere. Failed model queries leave their
destination unchanged.

The existing width-safe unsigned magnitude operation is renamed from a 386
name to its shared source-timing name and reused. Its arithmetic and the
386 `+6`/zero/memory formula are unchanged. SX instead uses its independently
qualified L2 `+10`, sign-dependent minimum, rounding and optimizing-operand
choice. There is no helper which silently chooses between 386 and SX models
based on a broadened profile predicate.

The existing nominal smoke adds 1,944 real IMUL executions across five
encodings, widths, code defaults, register/memory operands, two other-source
values and 27 explicit numerical vectors. These include zero, negative
minimums, byte/word/dword sign boundaries and values just above powers of two.
There are 1,344 real RCL/RCR executions across directions, implicit CL versus
immediate count encodings, widths, code defaults, register/memory and CF, with
14 independent masked-count vectors. Every successful model query checks
whole CPU, decoder and execution-context purity and disjoint original nominal
ownership. Two real beyond-RAM provider failures prove that partially fetched
operands cannot receive a successful model cost. Additional negative cases
retain fixed-one rotates, unsigned MUL, NOP and the older 386 path at their
existing owners. These are model-equation/input proofs, not silicon timing
measurements.

Initial x64 model and expanded failure tests pass in 0.25/0.23 seconds. Both
original graphs then rebuild 103 binaries for the expanded frozen CPU/386/
static selection and pass 115/115 rows, zero failed or disabled, in
20.64/11.20 seconds. JUnit records are `arithmetic-model-{x64,x86}.xml`.
Core manifests/corpus, ownership/route and Types checks pass; Types still
covers 1,297 paths. Broader Core/repository units, integration, native desktop
and artifact qualification are not repeated or claimed in this stage.

Cumulative source/test diff versus admission is production +1,477/-391
(net +1,086), tests +2,391/-14 (net +2,377), excluding manifests/docs/assets
and including the untracked repeat smoke. Only a private function declaration
and cost implementation are added; no public API, type, persistent state or
second runtime publisher is introduced. The complete selected publisher must
still consume this explicit L2 grade, combine the remaining source costs and
complete preview/Core/repository/profile/artifact qualification. WAIT, selected
aliases, fetch/control/string/entry/SMM and remaining translation/bus models
also stay in the full C16 obligation. No partial P, S9 acceptance or C17
admission is claimed. Shared six, MyNES, owner inputs and S8 releases remain
unchanged.

## Fetch Bus Observation And Baseline Model

The one existing `_kma_read_bus` owner now records successful selected-SX
instruction-fetch/prefetch transfers, after the provider succeeds. Failed
reads and observe-only preview do not contribute. The existing instruction
boundary clears one private `timing_fetch_bus_ticks` subtotal; reset uses its
existing whole-record clear. There is no public API/type, extra cache or
prefetch state, timer, queue or recovery path. Data/page-table/other-family
reads cannot accidentally enter this fetch subtotal. The existing external
cycle callback and Core's external-wait owner are unchanged.

This is an explicit L2 fastest-32-bit-bus baseline under the owner's standing
timing-upgrade approval, not measured pipeline time. E-2 defines the `2-1-2`
bus and separate external waits. The pinned 273021-001 developer manual
10.3.2.3/10.3.2.4 and 10.3.3 distinguish non-cacheable non-burst/burst and
cache-fill transfers. The model uses the fastest burst baseline for each
accepted read: two clocks for its first covered dword and one per subsequent
dword. A cache-line fill is therefore five clocks. Coverage includes the
physical starting offset, not just `ceil(bytes/4)`. Serializing the actual
transfers is a bounded model choice; no pipeline overlap, BRDY# trace, narrower
external bus or fitted-hardware exactness is claimed. The complete selected
publisher must preserve this L2 qualification when it consumes the subtotal.

The new tests exercise the existing fifteen-byte software prefetch policy;
they do not claim it is the Intel hardware's complete internal prefetch queue.
Seventy-two scenarios vary instruction length (one/two/fifteen), initial
position (aligned/offset/cross-line), code width, cacheable region and CD.
Each executes cold and warm after an explicit queue invalidation; the shorter
forms also execute a real sequential NOP from the retained queue. This gives
144 explicit frontend reloads and 96 sequential queue-hit executions.
Independent fixture counters record only completed real fetch callbacks and
bytes. All 144 preceding lexeme previews compare whole execution context,
CPU, decoder and those callback counts without modifying them.

Four segment-clipped reads prove starting-word coverage. Their initial
regression fails at start address one: the first implementation records two
clocks for four bytes spanning two dwords, while this frozen burst model
requires three. `fetch-clipped-initial-x64.xml` preserves that reproduction.
Adding the physical offset to the covered-word calculation fixes it directly,
without a compensating state or second path; the recheck passes in 0.20 s.
Two actual failed fetches retain only the successfully completed prefix
transfer and publish no retired-instruction ticks. A mixed MOV check keeps
the five-clock code fill separate from the two-clock operand miss, then reset
clears the observation. A real 386 execution keeps its new subtotal zero.

The initial observer snapshot passes 117/117 CPU/386/Core/static rows per
width in 21.66/11.17 seconds (`fetch-model-{x64,x86}.xml`); it predates the
clipped-read correction and is not the final qualification. Both original
graphs rebuild all 105 selected binary targets after that correction and pass
117/117 in 21.31/10.88 seconds, zero failures or disabled rows, recorded in
`fetch-final-{x64,x86}.xml`. Both Types checks cover 1,297 paths; manifests,
corpus, ownership/route checks pass. Broader Core/repository units,
integration, desktop/RDP and release qualification are not claimed here.

Cumulative source/test diff versus admission is production +1,484/-391
(net +1,093), tests +2,539/-15 (net +2,524), excluding manifests/docs/assets
and including the untracked repeat smoke. The source change adds one private
subtotal and its observation/reset at the existing owners. Fixture-only
controls/counters do not change production cache eligibility. Existing CPU
warning exceptions remain unchanged; this is not exemption-free compilation.
Full C16 still requires complete selected publication/preview, remaining
translation, split, repeat, control/event/SMM models and final repository/
profile/artifact qualification. No partial P, S9 acceptance or C17 admission
is claimed. Shared six, MyNES, owner inputs and S8 releases remain unchanged.

## Frozen Actual-Diff Review For Staged Closure

The owner approved the bounded stage on 2026-10-15. The executor reviewed the
actual diff against `6621f09f0`, not only the intermediate summaries. Source
changes are confined to nine existing CPU/Core files. The private timing
queries consume captured operands and actual access outcomes; false queries
preserve outputs. They do not advance time, reread guest memory or constitute
a second CPU executor. The selected-SX runtime still deliberately marks its
instruction cost compatibility/unallocated; the successor must replace this
staging at the one selector before production admission. Numeric proof of a
private subtotal is not proof of complete runtime timing or timing preview.

Actual review checked these ownership and lifetime boundaries:

- Decoder facts clear at the existing instruction boundary. Fetch accounting
  occurs only after successful real fetch/prefetch, not preview or failed read.
  Cache/TLB facts are observations of existing state, not parallel hardware.
- All 311 typed GPR-assignment replacements were reversed mechanically for
  inspection; every restored complete line exists in the original file, with
  zero unmatched assignment lines. Arithmetic RHS, flags and existing ordering
  are retained. Reference, BSWAP and SMM write observations were read separately.
- Same-value and partial-register destinations, actual REP identity and
  exception/NMI/debug/reset/prepare-commit boundaries have real producer/consumer
  tests. Rejected or aborted requests do not clear successful prior state.
- The appended public Core timing field `event_ticks` has one actual consumer
  in Core's existing pending completion. Committed entry does not increment
  successful retirement; old-family event subtotal remains zero. Budget and
  overflow tests cover delayed publication without synthetic retirement.
- The four private nominal/access/decode/L2 arithmetic queries and supporting
  source helpers remain cohesive in the existing timing owner. The access
  query deliberately rejects unallocated combinations. Its cold page-table
  numerical assumption is not full cached/mixed transfer qualification.
- Test changes retain old assertions and add actual execution, independent
  source vectors, failure injection, state-purity and ownership proof. The two
  new tests are registered under the CPU owner, not an App or Shared package.
- The existing two legacy CPU warning exceptions are unchanged. Strict-matrix
  and Types gates do not imply exemption-free compilation of the whole tree.

Counted source/test diff versus admission excludes manifests, docs and binary
artifacts and includes the new repeat smoke: production +1,484/-391, net
+1,093; tests +2,539/-15, net +2,524. The positive increase implements actual
source calculations, observation and direct proof, not a framework or copied
executor. No new public function/type is added; the existing timing-result
structure gains the consumed event field. Private decoder/context records
gain the documented observations, adjacency and REP identity, not guest state
mirrors. No new manager, queue, thread or recovery loop is introduced.

### Whole-T Retrospective And Transfer

The accepted implementation commits and their actual recorded qualification
were reconciled against the unchanged C01--C17 ledger: S1 `cab60e0ac` freezes
coverage; S2 `ddb260095` base system; S3 `cb52586a9` alignment/RMW;
S4 `c32103a24` translation/TLB; S5 `33cb2efe1` cache/LOCK;
S6 `926bfe185` VME/debug/exception/SX-FPU; S7 `0059bfd51` SMM/power;
S8 `585e08dad` whole inherited functional forms. Their accepted source/model
limits and initial findings remain intact, not rewritten as new failures or
hardware-exact qualifications. Current full regression must prove these
retained mechanisms still pass with the frozen S9 changes.

The whole-T review also retains S2's separately approved Shared preset-only
commit `e7c3f4e78`: three PC build target revisions change from 0559 to 0560.
It is not a Shared-six production/test change and is not repeated in S9.
No MyNES receiving code or release change is introduced by this staged closure.

All uncompleted timing, runtime/preview, source applicability, production
construction and final CPU obligations transfer through the complete residual
ledger in the [CPU successor](../../proposals/m5-486sx-timing-production-completion.md).
It is second after Core refactoring; no numeric T is preallocated. C16 and
C17 remain unmet. Neither staged closure nor passing the four shipped older
CPU products qualifies a production 486SX or PC110. This explicit owner
decision is the sole reason the original no-transfer T may close as a stage.

## Final Frozen-Stage Qualification

The original persistent Release Ninja graphs and one project ccache directory
are reused, with eight build workers and the configured four unit-test workers.
Production/tests/manifests remain frozen throughout qualification; the closure
does not extend the CPU implementation. `run-unit-tests` and the complete
`verify-current-specialized-gates` target finish successfully on both widths:

| Host | Repository units | Terminal unit time | Skipped, disabled or failed |
| --- | --- | --- | --- |
| x64 | 607/607 | 244.52 s | 0 |
| x86 | 607/607 | 61.70 s | 0 |

Logs are `build/t560-s9-research/closure-{x64,x86}-units-gates.log`.
The whole unit set includes Lib, Emulator, shared X86, all Core owners and all
five Apps, including three registered desktop-labelled Lib tests. Therefore
604 non-desktop-labelled unit rows and three automated desktop-labelled rows
pass on each width; this is not a manual RDP/interactive desktop acceptance.
The strict matrix reports 513 rows, 510 retained strict and three deferred;
Types covers 1,297 code/script paths. Existing exceptions are not removed or
misrepresented. Core source/test manifests and NXVM documentation governance
also pass. A direct changed-document link audit finds zero missing targets
after archiving the original T560 proposal.

The existing profile qualification route then builds/deploys each admitted
PC App and executes its actual external/INI/process suite once per width.
Shared objects are reused across profiles in the original graph. The default
group includes the twelve existing MyNES integration cases once; the other
profile groups exclude those unchanged cases, not their own receiving tests.

| Host | Default | XT | AT | DeskPro | Restored-default identity |
| --- | --- | --- | --- | --- | --- |
| x64 | 35/35, 58.12 s | 3/3, 27.31 s | 5/5, 43.31 s | 5/5, 123.24 s | 1/1 |
| x86 | 35/35, 65.32 s | 3/3, 34.58 s | 5/5, 50.72 s | 5/5, 159.12 s | 1/1 |

That is 48 integration executions plus the final restored identity check per
width. Per-profile logs are under the same ignored evidence directory with
`closure-<width>-<profile>-integration.log`; the three non-default groups also
retain JUnit XML. All terminal results pass with no disabled or skipped cases.
The two automated desktop-labelled MyNES native integration cases pass on
each width; the other 46 integration rows are not desktop-labelled. No manual
RDP, phone input/gameplay, fitted PC110 or complete Windows GUI session is
qualified. Passing old-family guests does not replace C16/C17 proof.

Both graphs are restored to default Release after qualification, with their
compiler launcher explicitly using the one persistent project ccache path.
All eight current 0560 PC EXEs pass the existing PE architecture verifier and
their normal deployment/process INI identity checks. An objdump section audit
also confirms all eight contain no debug-info sections. No superseded pair is
retained in Assets. Artifact inputs are this frozen S9 source/test tree against
`6621f09f0`, delivered in the same implementation P; the acceptance record
pins that P's exact commit. These are local owner-approved embedded-firmware
Release products, not a new firmware/source import.

| Artifact under assets/ | Bytes | SHA-256 |
| --- | --- | --- |
| my5160/my5160_0_5_0560_x64.exe | 1501133 | `B8F43C9F7D9317D5AF61A4CC02574BAC292405A9CEFE7354E1A9807C329D8390` |
| my5160/my5160_0_5_0560_x86.exe | 1645178 | `0A271714A3B38E5116ED95697E0AA4908DFAC05B59992014D62885D6498C86F6` |
| my5170/my5170_0_5_0560_x64.exe | 1513497 | `BF223FE7A0BDD93CDB8D97401F3A81B98569C28637E5C469E16681E38FF78ED4` |
| my5170/my5170_0_5_0560_x86.exe | 1658945 | `D0D83D37EBDFAD08D9EA5E846F3A96C1B0AD4F16BB8053F55527F432B66A4E79` |
| mydeskpro386/mydeskpro386_0_5_0560_x64.exe | 1500737 | `1DEC18AE79570529B689C31295F2A33BBAA705D27FCDA4AFFFBB37E893173420` |
| mydeskpro386/mydeskpro386_0_5_0560_x86.exe | 1645680 | `54474B92F7B7554F79C22EBDDCA269731F20F2AF85EB7A9EA42F002B1B8F2E6D` |
| nxvm/nxvm_0_5_0560_x64.exe | 1512667 | `2AA394E9B2C0B3651BB241816065A28C93B984F097C50D95A3FC7F94D8AA7097` |
| nxvm/nxvm_0_5_0560_x86.exe | 1657731 | `737B2D9ED36CD880B9A1D08D63CAD599395934C47AD94B0D86F718AF5D3351B9` |

The final ownership sweep finds no diff in Shared-six source/tests, MyNES
source/tests/released pair, owner INIs, external ROMs, media or snapshots.
Existing untracked refactor planning is read and retained as the first queued
candidate, with no refactor implementation mixed into this P. The second
candidate contains every remaining CPU obligation. Original T560 proposal
archiving repairs links only; historical facts are not globally renamed.
Whitespace, documentation, both Core manifests and changed local links pass.
No new CPU implementation was added after the owner froze the stage.

The complete amended S9 delivery is ready for implementation commit/push and
coordinator actual-change review. This is bounded-stage acceptance only; the
original CPU-complete exit remains unsatisfied in its successor.

## Coordinator Actual-Change Acceptance

Pushed implementation `b906c7138` is accepted after same-session coordinator
review of its actual 43 changed paths and exact source/test/artifact content,
not merely its report. Source/tests/artifacts still match that P exactly.
The review confirms the final terminal results and artifact hashes above,
original/manual versus explicit L2 boundaries, unchanged retained-family
assertions, inward dependency and Types gates, no Shared-six/MyNES/input diff,
one CPU/REP/cache/TLB/event owner and complete original-ledger transfer.

The nine source and twelve test paths have the reported +1484/-391 and
+2539/-15 diff. The new event-result field is consumed; the private staged
queries and unallocated SX bridge remain explicitly temporary at the CPU
timing owner, gated from production and received by the next CPU proposal.
No extra executor, hardware-state mirror or fallback qualification is accepted.
Both original graphs are default Release and all eight committed products
reflect the accepted P. Local link and documentation gates are rechecked for
the governance-only acceptance; no duplicate full dynamic run is manufactured.

The owner-approved amended S9 and bounded T560 stage close. Whole C16, C17
and final 486SX qualification do not. Original ledger/evidence and queued
receiver preserve the original completion standard after the Core refactor.
No successor is automatically admitted; CPU implementation remains paused.
