# T563 S3 Memory And Translation

## Baseline And Scope

Admitted from clean 68dd1b9e0 after S2 acceptance. This is an active inventory,
not completed qualification. The proposal's complete S3 batch and the original
[C01--C17 ledger](t560-s1-cpu-convergence.md) remain authoritative. The
[S9 record](t560-s9-selected-timing.md) documents the staged access formulas;
none of its unresolved classes become accepted by this admission.

## Confirmed Current Owners And Gaps

| Class | Current observation and allocation | Required S3 proof |
| --- | --- | --- |
| Ordinary integer/bit | cpu_timing_model.c operand helper handles one completed cache miss or one locked read; rejects multi-part cases. | Preserve source coefficients and cover actual split/uncached/locked parts without repeating the nominal bus cost. |
| UMOV | S2 gives the selected alias a nominal L2 model; memory helper has no 0F10--13 cases. | Actual alias reads/writes and source-grade-consistent access allocation. |
| Stack/string/descriptor/control | Ordinary helper deliberately rejects these distinct forms. | Reconcile each table/form and actual access route; do not assign a generic ordinary-read coefficient. |
| Cold successful translation | Histogram counts committed walks by zero/one/two A/D writes; memory helper accepts only two table misses per walk and attempts equal writes. | Preserve the documented 13/21/28 cold assumptions and prove no double-counted physical transfer. |
| Cached/mixed translation | Physical owner already observes table misses; current cache tests explicitly expect rejection for cached entries. | Original-source/model reconciliation before numerical allocation; no blanket cold-walk constant. |
| Raced A/D update | Locked reread is the authority; an attempt can complete without a write. Existing test expects no allocation. | Account accepted physical work and distinguish attempts from writes. |
| Failed/nested/split work | Completed reads are counted at physical owner; walk histogram publishes only at successful commit. Failed instruction rejects successful memory subtotal. | Distinguish attempted and committed work, preserve failures and hand event-only aggregation to S5 without losing actual access facts. |
| Alignment | Completed word/dword linear read and write each add an alignment observation. | Preserve per-access semantics, including read/modify/write and partial failure. |
| Lifetime/preview | Instruction boundary clears counters; preview suppresses cache/bus facts. | Actual execution, reset/next instruction and nonmutating query proofs. |

Physical read ownership is `_kma_read_physical`; page-table update ownership is
`_kma_update_page_entry`; successful translation publication is
`_kma_commit_physical_linear`. None is replaced by an extra timing bus or a
second cache/TLB. The final runtime selector remains the S6 receiver.

Initial read-only searches covered the actual producers and consumers in
src/x86-core/cpu and the existing cpu_cache_smoke translation matrix. The
cached and raced rejection is intentional staging, not a test to delete merely
to obtain a pass. Cache-hit versus external transfer and successful versus
failed access are separate dimensions.

## Source And Model Review To Complete

Intel 1992 PRM Appendix E.2 explicitly limits 13/21/28 to uncached paging
entries and a successful translation. The existing local text extraction
also identifies per-access alignment, cache-fill completion and multi-item
boundary assumptions. Re-read the original pages and relevant instruction
tables before numerical changes; text extraction alone does not establish a
new cached/mixed formula. Preserve the previously recorded original hashes.

No external implementation is copied or transliterated. A new bounded model
must identify its inputs, limits and source grade rather than be reported as
hardware-exact. Source contradictions remain visible.

### Successful Translation Model Decision

Cold walks retain E.2's source values. For a successful walk with `a` actual
locked A/D attempts and `w` completed writes, use the cold-attempt subtotal
13/21/28 indexed by `a`. A reread which finds the bits already set avoids one
write, so deduct the fastest-bus `w=2` clocks per avoided write. Each cached
initial paging-entry read deducts the E.2/E-3 two-clock read-miss surcharge.
These differential savings are an explicit L2 model: the manuals do not give
cached/mixed/raced page-walk pipeline timings. They are not a measurement or
a claim that full cache fills overlap in exactly this way. External waits and
fetch are not included a second time.

The aggregate is `13*walks[0] + 21*walks[1] + 28*walks[2]`
minus `2*(2*walk_count-table_misses)` and `2*(attempts-completed_writes)`.
The existing three-bin histogram will describe completed walks by attempted
updates, not successful writes; the existing scalar will describe actual
completed writes. No additional cache/TLB state is needed. Any saved transfer
marks the private result modeled; unchanged cold cases preserve their source
grade. UMOV likewise carries its already admitted L2 grade. The private cost
query gains an optional modeled-output pointer, published only on success;
no public API or enum changes. S6 must consume that grade when publishing the
sole final timing result.

Failed walks are not successful translations and cannot use this formula.
Their completed A/D writes must remain observable even if a later read fails;
their event-only elapsed aggregation remains the S5 receiver. The memory query
continues to reject failed retirements and preserve both output destinations.

## Admission Verification

Worktree was clean before admission. No production code, test source, artifact,
owner input or sibling repository has been changed at this inventory point.
No S3 dynamic test result or release qualification is claimed yet.

## UMOV Access Cohort In Progress

The existing operand-cache owner now assigns the selected 0F10/11 store
aliases no read-miss surcharge and 0F12/13 read aliases the MOV two-clock
coefficient. This retains S2's explicit L2 alias model, not an Intel timing
table claim for undocumented opcodes. No execution handler, public API or
additional access/state owner changes.

The existing MOV access matrix adds this fourth form: 36 actual cold/warm
read/write executions over byte/word/dword and aligned/unaligned/cross-line
addresses, plus 18 following-NOP clears and the existing whole-state preview
comparisons. Cross-line two-miss reads still explicitly await the remaining
S3 allocation; the test does not silently weaken that boundary.

An initial development run caught reversed opcode directions in the draft
model/test encoding: 0F10/11 are stores, not loads. The real decoder and
existing failed-read vector confirmed 0F12/13 loads. Corrected source and
encoding pass the Cache and nominal tests 2/2 per width (3.67/4.08 seconds).
The initial failed x64 JUnit and corrected records remain in ignored build
output as t563-s3-alias-x64.xml, t563-s3-alias-fixed-x64.xml and
t563-s3-alias-x86.xml. This is focused development evidence, not the final
frozen S3 regression or release qualification.

At that first cohort, code delta was production +4/-0 and test +6/-2, net +8, excluding
manifests and task records. No implementation P is complete or committed;
all remaining S3 classes above are still open. Original E.2 was rendered and
visually checked: its cold-walk/no-fault and multi-item alignment assumptions
match the recorded source boundary. Failed page walks can complete real
physical work without publishing the successful-walk histogram; any extension
must preserve that distinction and S5's event-only ownership.

## Stack And Translation Development Verification

The existing operand helper now covers single stack reads, near indirect
control transfers, LEAVE, XLAT and non-repeated MOVS/LODS/SCAS/STOS, using
their distinct Appendix E coefficients. The 28-form matrix executes both
code and operand widths, cold/warm data and applicable failed reads. It
checks query purity and output preservation rather than only opcode lookup.
REP distribution remains S4; multi-item and descriptor forms are not yet
qualified by these single-access rows.

The successful translation model above is implemented in the same private
query. Thirty actual read/write cases cover uncached, both-warm, PDE-only
warm, PTE-only warm and cache-eligible cold paging entries, with zero/one/two
A/D updates. Follow-up accesses verify a TLB hit adds no translation cost.
The initial residency fixture failed because its physical prewarm exceeded
the default segment limit; fixing the fixture limit, not CPU behavior,
allowed the intended paging cases to execute.

Four injected failures distinguish a failed initial PTE read after an
accepted directory write, failed locked reread, failed page-entry write,
and failed data read after a completed walk. Actual successful A/D writes
are now recorded at their physical write owner, so a later failure cannot
erase that accepted prefix. The completed-walk histogram remains separate.
All four reject the successful memory query without modifying either output;
the bus lock is released. No public API or extra persistent state is added.

Fresh x64 and x86 builds and Cache/TLB/486 nominal tests pass 3/3 per width
(4.11 and 3.85 seconds respectively). Development JUnit records are
`build/t563-s3-memory-progress-x64.xml` and
`build/t563-s3-memory-progress-x86.xml`. These are focused results, not full
S3 qualification, desktop validation or updated product artifacts.

At this checkpoint, tracked C/H changes relative to the accepted baseline
are production +72/-22 (net +50) and tests +264/-26 (net +238). Remaining
split/locked multi-part, multi-item stack/string and descriptor/control
allocation still prevents S3 closure. No complete implementation P has been
committed or pushed; the final selector remains S6 and C17 remains closed.

## Single-Operand Split Model Decision

Appendix E.2 gives three clocks per misaligned access independently of cache
residency, and an additional cache-line fill of up to r+3b for a multiple-line
read. At the selected fastest 2-1-2 bus this upper bound is five clocks. For
the already allocated single-read forms, a second actual missing line adds
that five-clock serialized upper bound to the existing miss coefficient;
it does not multiply the whole instruction coefficient by two. A second
locked read fragment adds one two-clock non-burst transfer. Both extensions
are explicit L2 models, not exact measured overlap. Alignment remains counted
once per completed logical access, and external waits remain S4's owner.

The single-read owner accepts at most two fragments; it does not thereby
allocate multi-operand instructions. A successful instruction with no data
miss or locked read has zero data surcharge regardless of opcode, avoiding
a second inventory of cache-hit/register-only instruction semantics. Paging
and alignment still pass through their existing owners. Planned incremental
change: production about +15/-8, tests about +50/-10, with actual split,
partial-resident, failed-prefix and next-instruction proof.

## Primitive Compare Access Decision

CMPS's two logical reads cannot be distinguished from one split read using
only the physical miss count. Record the number of completed logical DATA
reads that observed a miss at `_kma_read_linear`, beside its existing alignment
observation. It is one private instruction-lifetime fact, reset by the same
boundary, not an additional cache or hardware-state mirror. Failed partial
reads keep their physical prefix but do not publish a completed logical read.
Direct SMM physical reads remain S5 system facts, not ordinary operands.

E-9's primitive CMPS six-clock cold coefficient is preserved for two missing
operands without extra line fragments. A single missing operand uses an
explicit symmetric three-clock L2 estimate; each extra missing line retains
the split upper-bound model above. REP remains S4 and is not allocated here.
This also makes the single-operand branch reject unexpected multiple logical
reads rather than misclassifying them as a split. Incremental estimate is
production +20/-4 and existing Cache fixture/test +60/-0. Actual paired reads,
shared/independent lines, warm/cold/mixed residency, alignment, failure prefix
and boundary clearing are required before qualification.

### Implemented Split And Compare Cohorts

The existing ordinary MOV/ALU alignment matrices now assert numerical split
results and L2 qualification instead of intentional rejection. Forty-eight
additional actual line/page-split executions cover word/dword, MOV/XCHG,
cold/first-warm/second-warm/both-warm/disabled cache and second-fragment
failure. A partial failed logical read publishes no completed-read count,
retains the accepted physical fragment, releases LOCK and leaves timing
query destinations unchanged. The five-clock extra-fragment value remains
a conservative L2 upper bound even when cache fill is disabled; it is not
claimed as an exact non-burst pipeline latency.

CMPS adds 144 real executions covering byte/word/dword, aligned/unaligned/
split, shared/independent addresses, every two-operand warm mask and disabled
cache. A further second-operand failure checks the accepted logical/physical
prefix. Following NOPs prove the new observation clears. Existing preview
whole-state comparisons now include this field. The nominal execution corpus
also checks zero data surcharge for its no-miss/no-lock executions, rather
than introducing another execution fixture or opcode table.

Fresh focused builds and Cache/TLB/nominal CTest pass 3/3 per architecture:
x64 3.28 seconds, x86 3.43 seconds; JUnit records
`build/t563-s3-compare-x64.xml` and `build/t563-s3-compare-x86.xml`.
Current tracked C/H delta from accepted baseline is production +103/-37
(net +66), tests +425/-33 (net +392). Counts exclude manifests/docs/artifacts.
The production increase records real completed-read facts and source/model
allocations; duplicate zero-surcharge opcode branches were deleted. No new
public interface, cache, thread, bus owner or execution path was introduced.

S3 still lacks complete multi-register stack, descriptor and far-control
allocation. Those classes are not made eligible merely because their warm
zero-surcharge form now works. Full repository qualification, release builds,
commit/push and S3 acceptance remain outstanding.

## Consecutive Stack And Pair Allocation Decision

E-3's aligned cold POPA row is 7 clocks for 16-bit operands and 15 for
32-bit operands, spanning one/two 16-byte lines. Preserve those totals when
the aligned original stack and observed miss count meet those assumptions.
Partial residency uses the upward-rounded proportional share of that row;
extra observed lines use the E.2 five-clock upper bound. Nonbaseline cases
are explicitly L2, including uncached scalar transfers and unaligned groups.
E-8 note 8 assigns ENTER six clocks per 16 copied bytes. Allocate six per
actual missing source line, qualify the aligned fully cold copy as the source
case, and label partial/unaligned/uncached variants L2. No-read ENTER levels
remain zero data surcharge through the existing fast path.

BOUND and real-mode far-pointer loads read a consecutive scalar pair: use
their seven-clock cold row for the first line and the existing extra-line
upper-bound model for additional fragments. Protected descriptors require
their own coefficients in the following cohort. Keep source qualification
with the allocation helper instead of reconstructing its opcode decisions
in the outer memory query. Planned production +45/-12 and tests +120/-0;
existing real instruction fixtures remain the only execution route.

## Descriptor Allocation Decision

Original E-12 was rendered and reviewed. Its LAR 3/5, LSL 3/6, LLDT 3/6,
VERR/VERW 3/7 register/memory rows expose the three-clock descriptor term
plus distinct explicit-operand terms (2, 3, 3, 4). E-8 MOV/POP segment and
far-data rows use the same three-clock protected descriptor difference.
Record completed descriptor physical misses and missed logical descriptor
reads at the sole `_ksa_read_xdt` owner; subtract them from explicit operand
counts and add their three-clock term once. Extra descriptor line fragments
use the existing L2 five-clock bound. Cache-hit descriptors cost no miss
term; actual successful unaccessed descriptor loads add E-8 note 9's eleven
clocks through the existing counter. LTR's blank miss column remains zero,
as does ARPL's blank column. No public API changes.

The two private descriptor observations are instruction-lifetime facts and
share the existing clear/preview boundary. They separate actual access roles,
not architectural descriptor cache state. Expected incremental production
about +45/-15 and tests +150/-0, using the existing protected CPU fixture and
real KVM-independent bus provider. Cold/warm/mixed operand/descriptor cases,
null selectors, unaccessed loads, split descriptors, failure prefix and
query purity must be checked. Far/task complete totals still need their
own grouped reconciliation with S4 target-fetch and S5 event receivers.

### Consecutive And Descriptor Implementation Checkpoint

POPA/ENTER execute 192 scenarios across operand width, aligned/unaligned
source, cold/full/partial residency, disabled cache and a failed later read.
ENTER covers levels 0, 1, 2, 3, 8, 9, 16, 31 and lexical 255 masked to 31.
The failed-read vectors retain the first completed logical read and reject
successful timing publication. Following NOPs clear the observations.

Twenty-four descriptor forms execute 792 scenarios: both operand widths,
aligned/split descriptors, all explicit operand/descriptor warm masks,
accessed/unaccessed loads, and first/second descriptor fragment failures.
The failed split descriptor retains its accepted physical prefix without
publishing a completed descriptor read. Whole CPU/instruction/context and
bus counters remain unchanged by repeated timing queries. Actual descriptor
state/selector validation is still performed by the original instruction.

The existing stack/string table also adds real-mode segment, far-pointer and
table-register forms in both code/operand widths. A separate 768-execution
pair matrix covers BOUND, LDS/LES/LSS/LFS/LGS and LGDT/LIDT across line/page
boundaries, mixed residency, disabled cache and provider failure. LGDT/LIDT
at 0x2000 have an unaligned four-byte base field at 0x2002; this adds the
existing three-clock alignment term. The initial draft expectation omitted
it and failed. Added 0x2002-start cases directly prove the aligned five-clock
cache-miss row, rather than changing production alignment to fit the test.

The current code delta from the accepted baseline is production +199/-41
(net +158), tests +742/-33 (net +709); manifests, evidence and artifact files
are excluded. The original S3 planning ranges still cover this checkpoint.
The allocation helper now owns its model flag, removing the outer query's
duplicate opcode-based origin reconstruction. Actual successful unaccessed
descriptor writes use the existing eleven-clock observation once.

The Cache tests pass on x64 after both descriptor-prefix and pair additions.
Complete X86 Core rebuild/regression is running on the existing dual-width
graphs. These results are not a finished S3: far returns, call gates/task
switches and permission-table accesses still require full role allocation,
and complete repository units, release qualification and eight rebuilt
artifacts remain outstanding. Original E-13/E-14 were also rendered: task
switch miss rows are 55/31/37 for incoming 32-bit/16-bit/VM86, in addition
to the corresponding transfer row. Those totals cannot be replaced by a
single ordinary-read coefficient or silently folded into descriptor cost.

### Dual-Width Core Regression Result

Both existing Ninja X86 Core test targets built successfully. Complete CTest
inventories passed 165/165 per width, x64 63.67 seconds and x86 14.91 seconds;
records are `build/t563-s3-stack-descriptor-core-x64.xml` and
`build/t563-s3-stack-descriptor-core-x86.xml`. The x64 run overlapped the x86
build; elapsed times are not a performance comparison. No background job
remains from those runs. These inventories include retained earlier CPU
families, FPU/Machine, manifests, Types, dependency and negative gates.

After that production freeze, test-only additions assert the existing
protection-query corpus's valid/null selectors at CPL0--3 in both code widths
and register/memory forms. Null selectors correctly incur no descriptor
read, while their explicit memory operand keeps its own coefficient. The
descriptor matrix now explicitly checks next-instruction and reset clearing
after successful/failed accesses. Cache and nominal focused tests pass 2/2
per width (3.34/3.49 seconds), recorded in
`build/t563-s3-descriptor-lifetime-x64.xml` and
`build/t563-s3-descriptor-lifetime-x86.xml`; no production changed after the
165-test runs. Manifests are refreshed for those test additions. Documentation
governance also passes. This is a Core batch regression, not the required
final frozen whole-repository S3 qualification.

Latest tracked C/H counts are production +199/-41 (net +158), tests +761/-33
(net +728). The remaining far/task/permission access classes keep S3 active;
no P, new EXE or completed 486SX delivery is claimed.

## I/O Permission Access Allocation Decision

E-15 supplies the mode-specific cache-hit I/O totals and a two-clock data
miss term for primitive OUTS. It supplies no distinct miss coefficient for
the TSS I/O-map pointer and permission bitmap. Preserve OUTS's source term
when no permission checks occur. Where checks actually execute (protected
CPL above IOPL or VM86), model each completed missed logical read with the
same two-clock read-transfer differential, plus the existing five-clock
extra-fragment bound. This is explicitly L2, even when it has the same
numeric value as an OUTS read. Warm permission-table reads add zero. REP
setup/iteration distribution remains S4, not a primitive coefficient applied
on every continuation. Failed checks and port transfers remain failed
instructions and do not publish a successful subtotal.

No additional production observation or interface is needed: the existing
logical/physical DATA facts already cover both permission reads and OUTS.
The old mode/flags record identifies whether permission checks were required.
Estimated increment is production +20/-0 and test +80/-0, reusing the current
port fixture for actual IN/OUT/INS/OUTS and its transfer failure contract.

## Far Pointer And Return Allocation Decision

E-8/E-9 indirect far CALL/JMP rows supply 8/9 clocks in real mode and 10
in protected same-level transfers. The protected descriptor contribution is
three clocks, leaving seven for the explicit pointer. Same-level gates have
two descriptor contributions (six), leaving the same seven-pointer term in
their thirteen-clock indirect row. Keep those actual role allocations;
extra pointer line fragments retain the existing L2 fill bound. Direct
far transfers have no explicit pointer read. Their target fetch remains the
S4 owner and cannot be charged as a fabricated DATA read.

Far RET's stack subtotal is eight in real mode and six in protected mode
(plus the actual three-clock descriptor loads: total 9/12 same/outer).
IRET uses eight real/same-level stack clocks and thirteen on an outer return
(plus descriptors: total 8/11/19). Consecutive stack groups use the same
full-cold, partial-share and extra-line allocation as POPA, based on frame
width and actual misses. Nonzero RET immediate gaps, partial/unaligned
groups and protected-to-VM86 returns carry explicit L2 qualification. Task
switch return is a separate TS cohort and cannot inherit a stack coefficient.
For protected-to-VM86 IRETD, use the published outer-return full miss row
of nineteen across the actual nine-field frame, without adding nonexistent
protected segment-descriptor loads. This is a selected L2 frame model;
the source does not separately print that return's pipeline. It is not
the ordinary protected stack-only thirteen-clock source subtotal.
Source tables and original note 9's unaccessed loads remain unchanged.
Estimated increment production +45/-0, existing Cache tests +110/-0.

## Inner Call-Gate Role Decision

The E-8 inner CALL gate row decomposes into three missing descriptors
(3 each), the missing TSS stack-position line (8), and four clocks per
missing sixteen-byte parameter line (note 11). An indirect pointer adds
seven. To distinguish mixed residency, record completed scalar-pair misses
at `_m_read_pair` and actual TSS field misses at `_s_read_tss`; these are
two private access-role observations with the existing instruction lifetime.
TSS field failures retain physical prefixes; a failed pair keeps its prefix
in the existing global count but never publishes a completed pair subtotal.
No decoder, descriptor cache, executor or public API is added.

Use the source coefficients for one missing pointer/TSS line and actual
parameter lines. Extra TSS/pointer fragments use the established five-clock
L2 upper bound; unaligned/uncached parameter groups are explicitly L2. This
does not allocate full task-switch TS state reads, which have separate source
rows. Expected increment production +40/-3, existing Cache tests +140/-0.

## Full Task-Switch Allocation Decision

The three E-13 cold TS rows (55/31/37) constrain the model. The 32-bit
protected/VM86 difference is eighteen, matching six three-clock segment
descriptor reads. Including the outgoing TR descriptor, this leaves a
34-clock TSS state term for 32-bit/VM86; the 16-bit row leaves sixteen after
the outgoing TR and four incoming segment descriptor terms. The incoming
state image has six/three missing cache lines in the selected aligned
32-bit/16-bit reference layout. Allocate those 34/16 state clocks across
actual missing state lines by an upward-rounded share; extra lines retain
the five-clock bound. All nonzero state shares are explicitly L2 because
the decomposition/partial-hit overlap is inferred, not separately specified
by Intel. Keep descriptor reads at their existing actual three-clock owner.
An indirect pointer adds seven; nested IRET's backlink adds one (the E-13
four-clock transfer row less its three-clock TSS descriptor).

With separate cold descriptor lines and ordinary aligned TSS layouts, this
must reproduce 3+55, 3+31 and 3+37 for direct transfers; indirect forms add
seven and nested returns add one. Warm state reads cost zero. Actual LDT,
descriptor sharing, extra physical fragments and unaccessed loads retain
their observed roles and model qualification. Successful task transitions
consume the incoming checkpoint's format, never the outgoing TSS format.
Failed transitions still reject successful instruction subtotals; their
physical prefix and event-only elapsed time remain the S5 receiver.

The candidate-LDT selector path currently performs a real descriptor read
outside `_ksa_read_xdt` and therefore lacks that role observation. Move the
SX descriptor read/observation into one `_ksa_read_descriptor` primitive used
after each existing GDT/current-LDT/candidate-LDT validation. Preserve the
distinct pre-SX routes and failure codes. No additional observation field,
public API, thread or timing publisher is required for this task model.
Expected increment production +55/-15 and tests +160/-0. The role census
revises S3's planning ranges in Current before implementation; it does not
change its goal, source grades or acceptance obligations.

## Task-Gate Source Reconciliation

Final source review finds E-8/E-9 assign the same three-clock transfer
term to a TSS selector and a Task Gate, not three clocks for each of the
Gate and target TSS. A private per-instruction two-bit miss observation
records those actual descriptor roles. On successful task transfer only,
coalesce their two cold terms once; preserve partial residency and physical
fragment costs. Every nonzero task allocation is explicitly L2, since its
role decomposition is inferred even when a complete cold row matches Intel.
No public interface, descriptor state mirror or runtime path is introduced.
Extend the existing real-execution matrix with direct/indirect CALL/JMP
Task Gates, independently warm Gate/TSS roles, 16/32-bit and VM86/LDT tasks,
and each existing failure prefix. Expected increment production +19/-1,
tests +25/-12. Qualification started before this final correction is a
development checkpoint only; final qualification follows the frozen diff.

## Frozen Cohort And Same-Issue Review

The sections above are chronological development checkpoints, not unresolved
claims after this freeze. The admitted S3 universe now has these dispositions:

| Cohort | Retained owner and direct proof | Disposition |
| --- | --- | --- |
| Ordinary integer/bit and UMOV | Existing operand helper; Cache integer/access matrices, nominal zero-access corpus | Source coefficients retained; alias and split extensions qualified L2. |
| Split, uncached, locked, failed reads | Existing physical/linear owners; Cache split and alignment matrices | Actual accepted fragments retained; failed query preserves outputs and releases LOCK. |
| Stack, pairs, primitive strings | Existing decoded operand helper; stack/string, compare, group and pair matrices | Table coefficients and explicit partial-hit/extra-fragment models; no generic ordinary-read fallback. |
| Descriptor and system operands | One validated GDT/current-LDT/candidate-LDT read owner; descriptor matrix plus nominal CPL/null-selector cases | Actual descriptor and unaccessed-load roles, LTR blank-column semantics and split failure prefixes covered. |
| Far/gate/task control | Existing pair/TSS/descriptor access owners; far, call-gate, task and cross-width matrices | Real/protected/outer/VM86, cold/partial/warm, task formats, independent Gate residency and failure prefixes allocated; inferred task decomposition L2. |
| Primitive I/O and permission reads | Existing permission and bus execution; nominal ports and port-memory matrices | Real/protected/VM86, byte/word/dword, independent TSS/bitmap/data residency and port-failure prefix covered; permission differential explicitly L2. |
| Paging | Existing physical update and successful-walk commit owners; translation/residency/failure matrices | Cold 13/21/28 retained, cached/mixed/raced differential L2; failed-walk completed writes survive. |
| Lifetime/query purity | Existing instruction boundary/reset/preview rules; Cache NOP/reset and repeated query snapshots | No query changes CPU, decoded record, cache/TLB, reads or writes; failed query publishes neither output. |

The new Task Gate matrix executes 832 successful residency/form cases and
180 injected failures inside the existing Cache test. It includes direct and
indirect CALL/JMP, nested IRET, incoming 16/32-bit and VM86/LDT state. Existing
cross-width execution tests additionally prove outgoing/incoming TSS formats.
These are iterations of existing owned tests, not new framework/CTest cases.

Same-issue sweep reviewed every producer, clear and consumer of data misses,
locked reads, descriptor roles, unaligned accesses and TLB facts. Single
logical reads, pair reads and descriptor reads have distinct semantic roles;
the helper consolidates only validated descriptor access, preserving the
pre-SX route and existing exception order. The removed translation-local
write subtotal is replaced by the physical write owner's actual completion
fact, not a second publisher. No successful subtotal is returned after a
failed retirement. IDT/event accesses, SMM physical accesses and failed-chain
elapsed totals retain S5's explicitly separate event owner. REP setup and
per-iteration distribution, instruction fetch/refill, external bus waits and
HOLD contention retain S4; they are not fabricated DATA operands. Final
aggregation and preview eligibility remain S6, with C17 still closed.

This consumes S1 ledger R3/R4 at the private access-allocation boundary.
R5/R6 remain S4, R7 remains S5/S6, R1/R8 remain S6 and R9/R10 remain S7.
R2's event/system forms retain their S5 receiver after accepted S2 nominal
allocation. Inherited C05/C06/C07 access behavior is not reopened by these
timing observations; C16 as a whole and C17 remain incomplete until their
original runtime and qualification predicates are proven.

Sweep commands were `rg -n 'timing_(data_misses|missed_reads|descriptor_misses|missed_descriptors|task_gate_miss_mask|pair_misses|tss_field_misses|locked_reads|table_misses|unaligned_accesses|tlb_)' src/x86-core/cpu test/x86-core/cpu`
and `rg -n '80486_memory_penalty|80486_operand_cache_penalty|_ksa_read_descriptor|_s_task_read_selector|_s_read_tss|_m_read_pair' src test`.
The CPU execution source and owned tests are the complete changed mechanism;
Lib/Emulator/App internals are excluded because they own no such access facts
and no public interface changes. The existing corpus gates check dependency
and Types shapes; numerical roles require actual-execution assertions rather
than a source-text allowlist.

Self-review inspected every production hunk and all three changed existing
test files against the packet and the manual rows. The sole new production
helper is `_ksa_read_descriptor`, owning validated descriptor reads and their
observation. Six new private observations distinguish completed logical reads,
descriptor fragments/logical reads, Gate/TSS miss roles, pairs and TSS fields;
none mirrors architectural cache/TLB/task state. The one-byte mask is a
bitset, not a Boolean stored as an integer. There are no public functions,
types, enums, threads, managers, App cases or Lib/Emulator changes.

`git diff --numstat` over the four changed CPU C/H files is +331/-47,
net +284. The three existing CPU test files are +1273/-34, net +1239.
Manifests, task records and generated EXEs are excluded. The positive size
implements previously unallocated roles and actual-execution matrices;
ordinary/default dispatch and existing cache/bus paths remain single-owner.

## Final Qualification And Publication

Frozen builds and execution completed in the original persistent Ninja/ccache
graphs. No test assertion, timeout budget, host setting or owner input was
weakened to obtain these results.

| Gate | x64 | x86 |
| --- | --- | --- |
| Complete repository units | 631/631, 298.01 s | 631/631, 96.58 s |
| Selected four-PC integrations | 16/16 | 16/16 |
| Published CLI/INI identities | 4/4 | 4/4 |
| Specialized aggregate | 113/113 target steps | 113/113 target steps |
| Direct compilation matrix in aggregate | 497 retained strict, zero deferred | 497 retained strict, zero deferred |
| Final artifacts | Four 0563 EXEs, PE 8664 | Four 0563 EXEs, PE 014C |

Unit JUnit records are `build/t563-s3-{x64,x86}-units.xml`; integration
records are `build/t563-s3-{x64,x86}-{nxvm,my5160,my5170,mydeskpro386}-integration.xml`.
Each profile executes boot, INI identity and session status; AT and DeskPro
also execute their Console lifecycle/INI and CMOS receivers. This is the
packet's selected integration qualification, not the whole external-media or
MyNES integration suite. Both default configuration graphs are restored.
The complete unit inventory includes Lib, Emulator, all X86 packages and all
five Apps. Its three desktop-labelled automated tests pass per width (4.30 /
2.03 s aggregate), distinct from manual RDP, physical audio or complete guest
Windows GUI validation, which were not performed. Existing warning-policy
exceptions outside this change are not removed by the strict matrix result.

Production/test manifests, inward dependency and documentation checks pass,
as does `git diff --check`. A first attempted test-manifest command used a
nonexistent test-local verifier; the correct canonical source verifier with
`CORPUS_ROOT=test/x86-core` passes, as does the registered test manifest gate.
Earlier development qualifications do not substitute for these final runs.

Eight final hashes and PE widths are in [artifact evidence](t563-s3-artifacts.json),
captured after both specialized aggregates and published CLI checks, including
their final default rebuild. Every PC directory retains exactly its current
x64/x86 pair. All tracked INIs, MyNES binaries/source/tests, Lib, Emulator,
external firmware and media remain unchanged. No background verification
process remains. Shared implementation and target-separated publication
commits follow the complete verified delivery, not a partial batch.

Shared source/test implementation is pushed as `6f4a03a89`. The NXVM
publication commit containing this record carries the reviewed packet,
evidence and eight matching artifacts. Both were built and qualified before
the first implementation commit; no executable-input changes occur between
those target-separated deliveries.

| Counted source/test path | Added | Deleted | Net |
| --- | ---: | ---: | ---: |
| src/x86-core/cpu/cpu_instructions.c | 68 | 14 | 54 |
| src/x86-core/cpu/cpu_instructions.h | 9 | 2 | 7 |
| src/x86-core/cpu/cpu_timing.h | 3 | 2 | 1 |
| src/x86-core/cpu/cpu_timing_model.c | 251 | 29 | 222 |
| test/x86-core/cpu/cpu_486_nominal_timing_smoke.c | 132 | 1 | 131 |
| test/x86-core/cpu/cpu_cache_smoke.c | 1135 | 33 | 1102 |
| test/x86-core/cpu/cpu_task_switch_cross_width_state_smoke.c | 6 | 0 | 6 |

These are actual Git C/H line counts against accepted 68dd1b9e0, excluding
manifests, task evidence and eight binary changes. Self-review finds no new
public boundary, state mirror, second execution path, source-grade escalation
or changed App CPU selection. C16 runtime/preview and C17 remain incomplete;
passing older-family PC boots cannot discharge those gates.
