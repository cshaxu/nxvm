# T560 S5 Cache And Physical Bus Investigation

## Baseline And Scope

Admission baseline is 5560979f9 after accepted S4 c32103a24. CURRENT owns
the active packet. This record retains the source/route investigation and
implementation chronology; the Final Delivery below identifies the qualified
S5 snapshot. Earlier pending statements are historical progress, not current
qualification or a second task authority.

Target is NXVM: CPU cache state and existing Core physical routing. Shared-six,
MyNES, owner INIs, ROM originals and media are excluded. Eight affected 0560
PC EXEs are rebuilt and verified in this delivery. Production 486 admission
stays closed until the remaining T560 batches qualify the complete CPU.

## Original Sources Inspected

Pinned original identities are in
[T558 source availability](t558-s2-source-availability.md#acquired-intel-originals).
No third-party implementation is imported or transliterated.

- Intel 486 PRM, chapter 10.7, printed 10-12--10-16; INVD at 26-171 and
  WBINVD at 26-281. Figure 10-6 was rendered and visually inspected at PDF
  page 264; OCR alone is not the authority for register bit positions.
- Intel developer manual 273021-001, sections 4.4, 6.4.4--6.4.6,
  7.2--7.6, 9.2.17 and appendix B.2.1--B.2.5. These distinguish the
  write-through SX from the write-back enhanced DX4.
- Ignored research inputs/renderings stay under `build/`; archived originals
  stay external. No hardware-exact pin timing is inferred from guest boot.

## Source-Qualified Semantics

1. The selected SX has an 8-KiB, four-way cache: 128 sets of four 16-byte
   lines. Physical bits 10:4 select the set and bits 31:11 supply the tag.
   Ordinary instruction/data accesses and test commands must use these same
   tags, valid bits, data and replacement state.
2. Fills occur on cacheable read misses, not write misses. CD prevents fills,
   not reads from an already-valid line. PCD and the board's cache-enable
   response determine whether a missed read allocates. PWT controls external
   cache policy, not the SX's internal write-through policy.
3. CR0 CD/NW states are 00 normal, 10 no fills with write-through/snoops,
   and 11 no fills, no write-through on hits and no snoop invalidation.
   State 01 is invalid. In state 11 a write hit changes cached data without
   changing main memory; a write miss still reaches the external bus.
   Clearing/enabling cache control does not implicitly erase old cache lines.
4. Replacement checks invalid ways first in order 0, 1, 2, 3, then uses the
   documented three-bit tree. B0 selects the less recently used pair;
   B1/B2 select within that pair. This differs from the TLB's permitted
   replacement of valid entries while invalid entries exist.
5. TR3 writes select one of four doublewords in the fill buffer; TR3 reads
   select one of four doublewords in the read buffer. TR4 carries tag 31:11,
   selected valid bit 10, LRU bits 9:7 and all-way valid bits 6:3.
   Only tag and selected valid participate in a test write.
6. TR5 uses set 10:4, entry 3:2 and control 1:0. Controls 00, 01, 10 and
   11 select buffers, cache write, cache read and internal flush respectively.
   TR5 cannot be read. Test writes update normal replacement circuitry;
   test reads load TR4 and the read buffer from the selected real cache entry.
7. TR5 flush clears valid/LRU only, retains tag/data, and issues no external
   flush cycle. INVD invalidates internally and requests external flush.
   WBINVD invalidates internally, requests external writeback, then external
   flush. No internal dirty/write-back owner is needed for this SX personality.
8. Test buffers are not isolated diagnostic storage: developer B.2 warns
   ordinary memory operations can change TR4/read-buffer contents. The normal
   cache access implementation must reconcile that shared circuitry; merely
   connecting test-written lines to normal reads is incomplete proof.
9. A20M masks physical bit 20 before both cache lookup and external access;
   snoop invalidation ignores A20M. Therefore adding a cache above the current
   Core-only A20 wrapping without an explicit CPU/Core address contract is
   incorrect. Pin-recognition latency is a separate C16 timing obligation.
10. PDE reads/updates use CR3 PCD/PWT; PTE reads/updates use PDE PCD/PWT;
    ordinary translated accesses use PTE attributes cached in the TLB.
    Split-page accesses require independent attributes, not one ambient
    last-translation flag.

PRM asks software to set CD before test operations; developer B.2 recommends
CD and NW together. Neither observation authorizes an invented #GP merely
because a test command runs with cache enabled. Defined test sequences use
disabled fills; unspecified interactions remain explicitly distinguished.

## Actual Production Owners And Gaps

| Owner/path | Current behavior | Required disposition |
| --- | --- | --- |
| CPU `_kma_read_physical` / `_kma_write_physical` | One physical transfer through borrowed bus; trace every transfer | Add CPU-owned hit/miss/write behavior without a second executor or fake trace on hits. |
| CPU translation prepare/commit | TLB carries PCD/PWT but physical transfers receive no attributes | Carry attributes on each prepared access, including both split pages and page-table cycles. |
| CPU MOV TR decoder/handlers | TR3--5 rejected; TR6/7 use accepted TLB state | Extend the existing decoder/handlers, retain 386 forms and forbid TR5 read. |
| CPU opcode table | INVD routes to UD; WBINVD is a UD TODO | Replace those entries with source-qualified internal plus external effects. |
| Core `cpu_bus.c` | Serialized memory transactions and diagnostic external-cycle observations | Supply bounded cacheability/address data and real cache-operation delivery; observations are not commands. |
| Core `memory.c` route resolver | A20, selected aliases, replacement/ordinary/fallback providers | This remains the only physical decode owner; CPU must not classify addresses as RAM/MMIO itself. |
| Core `memory.c` range query | Resolves a whole span; narrower provider may still exist inside it | A cache-line eligibility query must prove every constituent byte has a safe consistent route, not trust a whole-span fallback alone. |
| Core `memory.c` RAM writes | Notify registered observers after publication; provider writes return earlier | Do not assume existing observer coverage is a complete snoop contract, or consume one of the four board observer slots without auditing callers. |
| Core DMA/debug/firmware writes | Share physical publication, but no CPU cache notification | Establish one actual invalidation boundary and distinguish CPU write-through from external writes. |
| Core `core_machine_signal_a20` | Changes memory routing only | CPU lookup address must reflect the live mask; indiscriminate flush would lose source-defined cache-as-RAM state. |
| Core physical aliases/decode changes | Can change backing/decode at runtime | Audit cache tags, route safety and coherence for aliasing; no hard-coded MMIO exclusions. |

## Bounded Interface Design Under Review

The CPU bus query described below is implemented. Remaining contract review
narrows the existing CPU bus, not a cache manager or device framework:

- CPU owns cache lines, valid/LRU state and the two four-doubleword buffers.
  Reset initializes that state; INVD/TR5 invalidation must not erase tags/data.
- A copied physical-access result from Core must identify the cache lookup
  address after A20 and cache-line eligibility from the actual route. The
  original request address remains available for the existing reset overlay.
  Ordinary device/ROM routes must not be operationally read while probing.
- Existing memory transfers need per-access PCD/PWT where required by paging.
  Do not introduce an ambient attribute that the next page walk overwrites.
- A typed external cache operation must have exactly writeback/flush semantics,
  serialized delivery and the existing `lib_status` failure result. Successful
  operation with no installed external cache means the special cycle was
  delivered and no external device responded; it is not skipped CPU work.
- External memory publication needs a bounded CPU snoop capability at the
  existing owner. It must respect NW, raw snoop addresses and actual DMA writes;
  CPU-originated write-through must not destroy its own hit line.

The first bounded Core contract is frozen as
`core_machine_cpu_cache_memory { address, fill_enabled }` and optional
`core_machine_cpu_bus_provider.query_cache_memory(context, address,
query, out_memory)`. Its typed query is LOOKUP, RESET_LOOKUP or FILL_PROBE.
It is observational: lookup resolves live A20, reset lookup preserves the raw
address, and only a fill probe checks the complete aligned 16-byte line. The sole
production caller is CPU physical access; Core's adapter consumes the existing
memory route resolver. Missing optional callback means identity address and
no automatic fills for isolated CPU providers, not success for an unimplemented
cache operation. Core supplies the real callback. This does not alter memory
transfer or Shared interfaces.

Core's initial external cache-enable policy accepts ordinary direct RAM only;
registered providers, reset overlays and physical/backing aliases disable
fills. This is an explicit board cache-enable policy, not a chip claim that ROM
cannot be cached. The resolver still examines every byte to reject narrower
device windows, and a failed eligibility query never performs a data read.
Selected 486 board admission must review that policy against its real wiring.
Snoop and external PCD/PWT delivery remain to be frozen before their
implementation; no real-operation result is inferred from this query alone.

The next frozen Core-only operation is
`core_machine_cpu_cache_operation { WRITEBACK, FLUSH }`, delivered by the
existing CPU bus provider's `cache_operation(context, operation)` status
callback. Core owns one `CPU_CACHE_OPERATION` transaction and optionally
delivers it to the construction-frozen attachment's `cache_operation` callback.
No attached external cache means a delivered special cycle has no board cache
work; a missing CPU bus operation callback is an implementation failure, not
success. CPU invalidates internally before INVD's flush or WBINVD's ordered
writeback then flush. An external-stage failure preserves the already-applied
internal invalidation/accepted external prefix, cancels only the failed Core
transaction and returns failure through the existing CPU internal-fault path.
No rollback, silent replay, new state, retry or diagnostic-as-command is added.
Estimated operation-only production +70--110/-5--15, tests +100--180/-0--10.

Snoop is frozen as the serialized CPU signal
`core_machine_cpu_signal_cache_write(context, physical, bytes)`. It ignores
CPU A20 masking, respects selected SX/NW and invalidates matching lines only.
Core's physical-write success path has one private borrowed `cpu_snoop`
callback/context pair, separate from the bounded board-observer slots.
Core inspects its existing active CPU-memory-write transaction to suppress
self-snoop; no mirrored origin flag or new request queue is created.
After a non-CPU write, the callback covers the raw address and, when different,
the actually changed RAM backing/provider address. The latter closes existing
Core A20/alias decoding coherence; that dual-address policy is Core L2, not
an assertion about Intel's EADS pin cycle count. Provider errors may follow
accepted effects, so attempted ranges invalidate on failure as well as success;
only an UNSUPPORTED decline promises no effects.
The callback is detached before CPU teardown and must survive stopped RAM
replacement for the same machine. Expected production +60--100/-0--10;
direct CPU/Core tests +100--180/-0--10. No Shared or App interface changes.

External page control is frozen as `lib_u8 cache_control` appended to existing
CPU bus memory read/write parameters, carrying the declared PWT (08h) and
PCD (10h) bits only. CPU drives PCD when CD is set and otherwise uses the
access's CR3/PDE/PTE attributes; earlier profiles pass zero. Core preserves
these bits in the existing memory transaction detail and delivers one optional
construction-frozen `memory_cache_control(owner, address, bytes, write,
cache_control)` status callback before data access. Failure cancels that same
transaction; observations deliver no pins/effects. There is no ambient page
state, separate transfer route or compatibility callback. Existing Core test
adapters migrate signatures and forward the new value; callbacks without an
external cache explicitly ignore it. All callers are Core-owned. Revised
whole-S estimates are production +500--850/-40--120 and tests +800--1100/-40--100
because the sole ABI must migrate roughly thirty owned adapter files.

The query operates on the masked bus address before checking alias/provider
eligibility. It publishes copied results only and performs no read/transaction.
A whole-line route failure disables speculative fill; required data access
still uses the normal bus result and does not inherit a speculative fault.
The real CPU connection must use that same address for lookup and ordinary
external transfers, except the explicitly unmasked reset fetch route.

## Finite Verification Batch

| Group | Direct proof required |
| --- | --- |
| Geometry/test state | All 128 sets, four ways, four buffer indices, five operations, V/tag/LRU readback, retained tag/data after flush. |
| Normal/test sharing | Test-written line read/write by real instructions; ordinary fills visible to TR reads; normal buffer/status side effects. |
| Replacement | Invalid-way order, all eight tree states, hit/write/fill updates, conflicts and unrelated sets. |
| Controls/attributes | Three legal CD/NW states, invalid state rejection, PTE/PDE/CR3 PCD/PWT, independent split-page attributes, read/write hits and misses. |
| Routes/coherence | RAM, immutable ROM, device and narrow straddled provider, live A20, selected aliases, DMA/debug writes, protected cache-as-RAM state. |
| Bus commands | INVD flush; WBINVD writeback then flush; empty and populated cache; stage failures; no TR5 external cycle; no success-only observer substitute. |
| Encoding/faults | TR directions/indices, TR5 read rejection, modes/CPL/VM86, prefixes/LOCK, fetch failures, flags and restart state; old-family isolation. |
| Observation/reset | Preview leaves cache/buffers/LRU/Core effects unchanged, failed publication boundaries, reset and existing debug patch semantics. |
| Real Core integration | Production CPU bus, memory decoder and non-CPU write route, not a cache-enabled test fixture alone. |

Carried C04 second-read/locked page-table update and C16 bus timing remain
explicit T560 obligations. Developer 6.4.5 requires using the second read if
an entry changes during its locked update. The initial first-read-plus-write
was insufficient despite passing cache tests. S5 must reconcile the exact
receiver and the implemented attribute path before its complete delivery.

The renewed source review is developer 6.4.5, printed 6-33: when A/D bits
need setting, PDE/PTE entries are read again under a locked read/write, and
the second value is authoritative if it differs. At that review preparation
wrote PDE A from its first value; commit wrote PTE A/D from the saved
first value. Neither proved this requirement, and the CPU bus then had
no explicit page-update lock boundary. Cache success did not cure it.
The next implementation review must reconcile second-value permission/base
selection with the existing split-access preflight and single transaction
owner; blindly adding a second read at commit could still use a stale target.
Unchanged older-family behavior and preview remain separate contracts.

## First CPU Implementation And Focused Proof

The existing execution context now owns 128 by four cache lines, three-bit
replacement state and the fill/read buffers. Existing MOV handlers and lexical
validation implement TR3/4/5 without a second dispatch path. TR5 flush retains
tags/data and clears valid/LRU; test writes update the tree and test reads
return tag, selected/all valid bits and replacement state. Context construction
and reset initialize the real cache and buffers. No public API was added in
this first step.

Actual tracked production diff so far: +103/-11, net +92, in the CPU's
`cpu.c`, `cpu_instructions.c` and `cpu_instructions.h`. The new CPU-owned test
is 208 lines, with one registration line. Documentation/manifests/artifacts are
excluded from those counts. These are interim counts, not delivery totals.

The test performs real instruction dispatch across all 128 sets/four ways,
four buffer indices, invalidation with preserved data/tag, legal code-size and
prefix forms, lexical/real TR5 read rejection, preview non-mutation and each
fetch-byte failure for the five legal TR3--5 direction/index combinations.
An unchanged cache and architectural CPU are asserted after failed fetch.
Older-family/privilege/LOCK and TLB regression tests are retained.

Using the original persistent Ninja/ccache trees, focused x64 and x86 tests
`core.chips/cpu/cpu_cache`, `cpu_tlb` and `cpu_test_register` pass 3/3 per
width. After adding the preview/fetch checks, x64 cache was rechecked and x86
ran the complete focused three-test selection. The unchanged x64 TLB/TR tests
reuse their preceding passing result; no full-suite result is claimed.

At the first CPU step ordinary reads/writes did not yet use the cache.
Test-register proof alone cannot close C07. No S5 rebuilt release artifact or
manual desktop/RDP verification exists. S4 EXEs remain the published baseline.

The production Core CPU bus now implements the copied cache-memory query.
Its existing memory-inspection regression exercises this actual callback,
including a two-byte provider inside a 16-byte line, ordinary RAM, A20 masked
and enabled addresses, alias exclusion, reset-overlay exclusion and query
failure. Query failure disables fill, whereas a subsequent required provider
data read returns its original I/O error. No query invokes device reads or
parity effects. Final updated test passes on x64 and x86; CPU cache regression
also passes on both widths. This first query proof alone does not prove
normal CPU cache integration.
The query step adds 69/removes one production line and adds 41/removes two
existing test lines. Combined interim source is +172/-12 (net +160); tests
are +250/-2 (net +248), including the 208-line new test. Manifests,
documentation and artifacts remain excluded. The new copied type and callback
carry actual A20/line-eligibility data across the CPU/Core boundary; the one
private memory helper owns geometry/route inspection, not a second resolver.

## Ordinary CPU Access Implementation

The existing physical-read/write owners now use the actual CPU lines. Reads
look up the Core-supplied A20 address, serve hits without an external cycle,
and allocate a fully read 16-byte line only on an eligible miss with CD/PCD
clear. The read is staged before replacing a victim. A preview may observe
resident data, but cannot fill, update LRU/buffers or publish external effects.
Writes allocate nothing: hits update the cache after a successful write-through,
or locally with NW set; misses always reach the bus. Existing old-family
transfers and selected-SX miss transfers share one read/write bus-publication
owner. The status/read buffer is also shared with TR test reads.

Read attributes are explicit private arguments: directory reads use CR3,
table reads use PDE, and data reads use the prepared PTE/TLB attributes.
Split-page accesses retain each prepared page's attributes independently.
No ambient last-page attribute, extra cache object or public transfer API was
introduced for this step. PWT has no internal SX write-policy effect; its
external pin/control delivery is still part of the unfinished bus contract.

The cache fill is currently one completed 16-byte logical bus transfer. This
models data/state and failure publication; it does not claim the critical-word
first burst order or exact pin timing. C16 retains that physical-cycle proof.
Normal hit status/read-buffer updates cover data and page-table reads, not
cached instruction fetch; instruction fills use the actual fill/read circuitry.
Do not infer unspecified test-buffer contents after an unrelated external bus
cycle from this implementation choice.

The direct test now proves ordinary fills, stale-until-invalidation hits,
write-through, CD-disabled-fill hits, NW cache-as-RAM writes, no write-miss
allocation, and normal lines read back through TR commands. All four PCD/PWT
combinations have two actual paged reads; a split-page word proves cached first
byte and uncached second byte remain independent. Failed fill/write-through
does not publish a new/modified line or change external data. Test-written
geometry tags are outside the executing code range, so those tests do not
accidentally execute their own data as instructions.

The first new paging test failed because its protected-mode DS selector was
null; the observed shutdown was not an accepted cache outcome. The fixture now
has a valid non-null DS, preserving the actual permission path. A failed compile
also exposed signed promotion in byte-split arithmetic; it was fixed without
warning suppression. A test invocation before that failed build completed used
old binaries and is explicitly not proof of this implementation.
After successful source builds, the focused cache/TLB/TR/Core-inspection
selection passes 4/4 on x64 and x86. These are focused results, not full S5
qualification. External cache operations, snoops, complete replacement/form
matrix, actual Core CPU cache execution, and remaining page-walk/bus work are
still mandatory before delivery.
After the final test-only literal/output cleanup, the cache test was rebuilt
and rechecked on both widths; the other three unchanged focused proofs are
reused. Current interim tracked source is +331/-37 (net +294), tests
+440/-2 (net +438), counting the 398-line untracked new test explicitly.
Counts exclude manifests, documentation and artifacts. This remains work in
progress: refine line-eligibility query cost on hits during the S5 quality
review, rather than adding a cache-policy mirror or ignoring live A20.

## External Cache Operations

INVD and WBINVD now use the existing 0F metadata/dispatch owner and one
internal invalidation/operation helper. Only selected SX metadata admits them;
386 retains real #UD and LOCK remains invalid. CPL is checked before effects.
Internal valid/LRU and CPU prefetch are invalidated before external delivery;
tag/data storage is retained. WBINVD delivers writeback then flush. No passive
successful handler or diagnostic-only event substitutes for those operations.

Core implements the operation through one existing serialized transaction,
including real optional attachment invocation, commit or cancellation and
failure status propagation. A missing CPU bus implementation stops internally;
an absent attached external cache is different: Core completes the actual
special transaction with no external cache work. No failure replay or
compensating rollback occurs. CPU tests retain already-applied invalidation
after either failed WBINVD stage; Core tests prove cancellation releases the
transaction and a subsequent operation can succeed.

Direct instruction tests cover both opcodes, four mode/code-size combinations,
four operand/address-prefix combinations and empty/populated cache; flags and
operation order are checked. Privileged failures use actual protected #GP
handlers, including VM86 ring transition and stack frame. All four fetch-byte
failures in prefixed opcodes preserve cached lines and send no external
operation. Missing implementation, retained 386 and invalid LOCK are explicit
negative proofs. The original TR privilege test now shares the CPU-owned
protected-exception fixture setup while preserving every former assertion;
there is no second synthetic fault handler or test framework.

The initial operation form failed before execution because the existing
metadata gate still rejected opcodes 08/09. Correcting that same gate, rather
than bypassing it, connects preview/dispatch and old-family isolation.
Focused current production passes 4/4 per width; the final fetch-only test
extension was then rebuilt and rechecked per width, with the other unchanged
results reused. These are not whole-repository or S5 closure claims.

Interim whole-S counts are production +402/-43 (net +359), tests +682/-36
(net +646), including the 554-line untracked new cache test. The new operation
enum/callback and attachment field carry the actual effect; the transaction
kind records that effect within the existing owner. No persistent retry,
completion mirror or new worker was introduced. Documentation, manifests and
artifacts are excluded from these counts. Snoop, external PCD/PWT, full cache
replacement/integration and closure qualification remain unfinished.

## Snoop And Real Core Execution

The CPU range signal now invalidates matching valid bits only. It respects NW,
does not apply the CPU A20 mask, preserves tags/data/LRU and handles both short
spans and large ranges without iterating every byte. Older profiles return
without cache work. Core binds one private borrowed RAM callback/context at
construction and clears it before CPU destruction. Stopped backing replacement
keeps that binding; no board observer slot or separate origin state is consumed.

Existing RAM/provider write publication invokes this hook. The initial version
did not notify on provider failure; the accepted-prefix correction below
supersedes that incomplete rule. Recursive mixed-route byte writes notify their
accepted prefix through the same owner. CPU-originated memory writes are identified
by the existing active CPU memory-write transaction and do not self-invalidate.
DMA, debug and firmware writes instead propagate the published physical and
decoded/backing ranges. Alias dual-address handling is explicitly Core L2,
not an assertion about physical EADS cycle count. Complete provider partial-
failure and decode-change sweeps remain ahead of closure.

The CPU test proves a two-byte boundary-crossing snoop, a broad range and NW
inhibition through real resident lines. The Core test uses the actual CPU bus,
physical routes and DMA operation, not a mock transfer: a selected chip is
bound in a Core-owned unit fixture while production 486 machine admission
remains closed. Real instructions verify fill, alias/backing write visibility,
DMA visibility, successful write-through with its line still resident and
NW preservation after an external write. The intended load is published before
enabling NW and AX is changed beforehand, so stale cached store instructions
cannot make the NW assertion pass accidentally.

A separately seeded real CPU line isolates provider publication in the Core
integration fixture. The later correction invalidates it on both accepted and
possibly partially published failed writes. This fixture state is not a second cache or a test-only public
API; ordinary CPU algorithms are already tested at their chip owner.
The same real Core fixture replaces RAM and verifies callback/context lifetime.
Both widths pass the focused four-test selection; after the final provider-only
test extension the changed Core test was rebuilt/rechecked on x64 and the
four-test selection passed again on x86. Unchanged x64 chip results are reused.

Interim whole-S source is +466/-43 (net +423); tests +800/-39 (net +761),
including the 579-line untracked new test. Manifests, documentation and artifacts
are excluded. Positive growth represents actual cache state/signals and direct
Core proof, not a generic observer registry or recovery framework. External
page-cache control, full replacement/route/form sweep, query hit cost and final
whole-unit/integration/artifact qualification remain unfinished.

## Per-Access External Page Control And Failure Sweep

The existing CPU bus read/write signatures now carry PCD/PWT, and private
write accesses receive explicit attributes just like reads. CR0 CD forces PCD;
otherwise each access carries its own CR3/PDE/PTE bits. Core delivers these
bits in the same existing transaction to its optional frozen board callback
before data access, validates the allowed mask and preserves the bits in trace
detail. Observation accesses invoke no control callback and publish no cycle.
Callback failure cancels the existing transaction before required data access;
there is no ambient current-page/control state or new memory path.

The mechanical signature/forwarding migration includes adapters and direct
fixture setup calls throughout the CPU/board CPU tests. The scan found no App
or Shared provider. Direct setup operations use zero control; delegated bus
operations forward the actual control. There is no compatibility callback.
Three pre-existing timing-model observation calls also receive zero, preserving
their observational behavior; cache-aware/full selected timing remains C16.
The actual changed adapter bodies and representative setup calls were reviewed;
the x64 `build-unit-tests` aggregate compiled all registered consumers successfully.

A 64-case matrix varies CR3, PDE and PTE PCD/PWT independently and checks
read and A/D-write controls at each level. It preserves the internal PCD/PWT
and split-page proofs. Actual Core tests check pin delivery, invalid-mask
rejection, cancellation-before-data and observational non-delivery. The updated
focused four-test batch passes on x64, as does the original transaction
lifecycle regression. Both aggregate unit builds subsequently completed.
The six-test cache/TLB/TR/Core inspection/transaction/timing-preview selection
passed on both widths, including the x86 terminal LastTest.log evidence.
Build success is not full-suite test success.

The sweep found an important existing contract in the transaction lifecycle
test: provider side effects can precede an error, and cancellation does not
undo them. Treating every failed write as unpublished was wrong. Core now
snoops attempted provider ranges on error as well as success; only an
UNSUPPORTED decline promises no effects. CPU write-through failure invalidates
its hit line, preserves tag/data and stops through the existing internal fault,
rather than leaving stale valid data or inventing storage rollback. Read-only
accepted bus writes also drive the existing snoop path. No new quarantine
state, replay or recovery framework was added. Tests explicitly check the
changed valid-bit contract, not a fabricated unchanged-storage guarantee.

Current interim source is +828/-140 (net +688), tests +2068/-427 (net +1641),
counting the untracked cache test and excluding manifests/docs/artifacts.
Most extra replaced test lines are the single necessary ABI migration; the
growth does not represent additional test frameworks or product paths.

## Qualification Status

### Final Delivery

The final snapshot passes 601/601 full repository units per width and all
eight sequential release/integration groups. Each group ran once:

| Profile | x64 integration | x86 integration |
| --- | --- | --- |
| default-pc-at-80386-1440k-hdd | 35/35, 56.21 s | 35/35, 66.49 s |
| ibm-5160-model-268-360k | 15/15, 33.30 s | 15/15, 41.10 s |
| ibm-5170-model-339-1200k | 17/17, 49.43 s | 17/17, 57.67 s |
| compaq-deskpro-386-model-40-1200k | 17/17, 144.30 s | 17/17, 171.19 s |

Logs use `build/t560-s5-<width>-<profile>-integration.log`. The 84 executions
per width include the same 12 MyNES integration registrations in each profile
graph; they are not 84 distinct proof scenarios. Background aggregate results
and native desktop-labelled registrations are automated qualification, not
manual RDP, game interaction or a Windows 3.1 desktop claim.

Both original Ninja/ccache graphs are restored to the default profile.
Shared/Core targets were reused between profiles; only selected App/firmware
and directly configuration-dependent consumers rebuilt on the switches.
All eight artifacts passed the existing PE verifier (8664/014C), Release
deployment and a section inspection rejecting compiler debug/stab sections.
Version remains 0.5.0560. The implementation commit containing this record
identifies the source snapshot; the hashes identify the exact deployed bytes.

| Artifact under assets/ | SHA-256 |
| --- | --- |
| my5160/my5160_0_5_0560_x64.exe | 99F6E6BD49EC69DFA8A9C561588A1E7C6F89CC70E0F4B04BDCDD81607A60ECEF |
| my5160/my5160_0_5_0560_x86.exe | AE2498AF54CC1311931B67BFA4AB473D35B1DCA8F8EA7C90790867277DF03048 |
| my5170/my5170_0_5_0560_x64.exe | 321D139083F8C403D384776B0B59AAA7142BE42BD10728DBECB3CF151AF745E6 |
| my5170/my5170_0_5_0560_x86.exe | 8CF412A51011D911E20D1AA4D0652633A2E980AE44A8ACEC7E0C9A071CC7CBD0 |
| mydeskpro386/mydeskpro386_0_5_0560_x64.exe | F477A26A339B9C1B0564514C71B70D7A7D9B674BF8BF61E08AA3436E52D4A62A |
| mydeskpro386/mydeskpro386_0_5_0560_x86.exe | 91C8F1B4D8706894E414E082680BC832D9A1CEF8439F0B80231FFA9978ED49EE |
| nxvm/nxvm_0_5_0560_x64.exe | 520364D412982431531E3BEFE0E92AA8CFD81E0D0EBFF51B7CC636FBF3922BC5 |
| nxvm/nxvm_0_5_0560_x86.exe | A70876D987BB6177816906D8491FD48E85102D3923C2DE858B218A4191977FC3 |

C07's state/command/cache-control/coherence batch and carried C04 second-read
and locked-update semantics have source-qualified CPU and actual Core proof.
Logical line fills, conservative route cacheability and alias coherence remain
explicit L2 models; C16 owns pin/burst/wait/clock timing. S6--S10 retain feature,
VME/debug/SX-FPU, SMM/power, inherited semantic, timing and final admission
obligations. No applicable obligation leaves T560. Coordinator review of the
complete pushed P remains required before S5 closure.

### Final LOCK-Fixed Whole-Unit Snapshot

The same final production/test snapshot passed 601/601 units on x64
(235.06 seconds) and 601/601 on x86 (64.73 seconds), sequentially in the
original release-ccache graphs. Logs are
`build/t560-s5-x64-unit-lock-fixed.log` and
`build/t560-s5-x86-unit-lock-fixed.log`. Both contain the Shared, Core and
all App unit groups, including the existing desktop-labelled units.
This does not qualify external profile integration, eight deployed PC EXEs,
manual RDP interaction or the remaining T560 CPU batches. The eight affected
profile/width release and integration groups are the next qualification step.

Actual-diff review confirms every accepted modifying-memory LOCK handler
retains the common RMW entry: ALU/group forms, INC/DEC, NOT/NEG,
bit-set/reset/complement, CMPXCHG and XADD. Memory XCHG alone requests the
implicit signal; register forms never acquire it. Existing prefix legality
and fault tests are retained. Arithmetic bodies and lexical-prefix ownership
are not replaced. The later C16 batch still owns bus/pin/clock accuracy.

### Final Code And Boundary Review

Against the accepted S4 baseline, production C/H changes are +828/-140,
net +688. Core test C/H/CMake changes, including the new cache test, are
+2068/-427, net +1641; manifests, documentation and binaries are excluded.
Most old test-file replacements append the same per-cycle cache-control
argument or forward it to their existing fixture. They do not replace the
instruction algorithms or remove their old assertions. The protected TR
setup moves to the existing CPU-owned fixture and retains actual GP delivery.
An additional whole-file comparison checks all 51 purely mechanical test
migrations against HEAD after removing only the declared new parameter,
its unused cast and the appended/forwarded argument. Ignoring whitespace,
every remaining file matches exactly; no unexplained semantic difference is
left in that set. The other changed tests and registration are reviewed as
real proof changes, not classified as mechanical.

Necessary state is one CPU cache/tag/valid/LRU array, fill/read buffers and
one actual LOCK signal; there is no board copy of those states. Core owns
the existing transaction and a borrowed private snoop callback, detached
before CPU destruction and retained during RAM replacement. CPU owns line
lookup/fill/publication and test-register commands; Core owns routing, A20,
provider eligibility and actual external effects. The query avoids probing
all 16 routes on hits/writes. The unpublished page-only lock contract is
removed rather than retained as an alias. No manager, thread, retry or second
execution path is added. Source vocabulary uses existing Lib Types symbols.

Core source/test manifests, the Core dependency gate, documentation-governance
gate and whitespace check pass. The initially attempted test-local manifest
script path did not exist; the successful check uses the existing
`src/core/verify_manifest.cmake` with `CORE_ROOT=test/core`, as registered by
the test package. No verifier or assertion was relaxed. Existing legacy CPU
warning exemptions remain; this is not a claim of exception-free strict
warning qualification of all source.

No Shared-six, MyNES source/test/artifact, INI, raw firmware, media or snapshot
change belongs to this batch. Selected 486 tests execute the real CPU/Core
paths inside Core-owned fixtures; the admitted eight releases still use their
existing hardware populations. Their regression does not qualify a 486 board
or prove Windows 3.1 desktop execution. Manual RDP/owner interaction is not
executed by the automated desktop-labelled test cases.

### Whole-Unit Baseline Observation

The original x64 aggregate compiled the refreshed consumers and executed all
601 units. It finished with 599 passes and two failures, not a timeout. Both
registrations use the same protected-UD source: its independent 0F table still
classified INVD/WBINVD as reserved after their implementation. The expectation
now records their exact 486SX minimum, retains the entire 256-opcode matrix
and adds actual 386 protected-UD delivery for both instructions.
The two failed registrations then rebuild and pass on both widths. This is a
targeted correction, not a new 601/601 claim. The sequential aggregate stopped
on x64 failure, so x86 whole-unit execution did not start.

The baseline logs stay at `build/t560-s5-x64-unit.log`. The confirmed locked
RMW gap remains even though existing tests do not cover it; final full-unit,
integration and artifact qualification waits for that repair and stable inputs.
No failure was attributed to Windows updates, system settings or MyNES.

### Page-Update Transaction Boundary

Developer 3.1.5 requires a locked read/write sequence without intervening bus
masters and a read from the bus rather than the internal cache. The existing
CPU bus now declares a paired begin_page_update/complete_page_update boundary
for one aligned four-byte entry and its PCD/PWT control. Core uses its existing
transaction state with the appended CPU_PAGE_TABLE_UPDATE kind; there is no
additional lock flag, object, worker or transaction manager.

Only matching page-table read/write callbacks participate in that held
transaction. Individual transfers still deliver control and actual memory
effects, but do not commit/cancel the pair. The CPU caller must complete once
after successful begin, including every guest/provider fault. Unrelated data
traffic or another owner cannot acquire the occupied transaction. A complete
pair may be absent only on an isolated sole-owner CPU bus, not a multi-master
Core bus; Core installs both callbacks. CPU now uses this boundary for second
reads, but the finite matrix below remains incomplete; C04 stays open.

Direct actual-Core tests cover held ownership across read/write, exclusion of
DMA and ordinary data reads, exactly one commit, control failure at either
transfer with explicit cancellation, reuse and invalid alignment/control.
The memory-inspection and original transaction-lifecycle selection passes 2/2
on x64 and x86 after rebuilding. Earlier aggregate consumer builds predate
this new paired contract and are not final qualification for it. Pin timing
and complete bus arbitration remain C16 rather than inferred from this test.

### CPU Second-Read Connection

One private page-update helper consumes the existing translation record and
paired boundary. Its read uses the existing external read owner, bypassing
internal cache. Begin/read/write failures use the existing CPU fault path and
every acquired pair is completed exactly once. The second value is checked
for presence and, for PTE, effective permission before A/D publication.
PDE A retains its established early walk ordering; directory permissions
remain checked by preparation. Still-valid hit lines receive successful
updates without new fills; failed writes invalidate hits because providers may
accept a prefix. Preview and older profiles do not use the helper.

The copied CPL in the stack-local translation record is needed for second-value
permission checks, not persistent state. Preparation uses the second PDE's
table base; commit recomputes address and attributes from the second PTE before
TLB/data publication. No stale-address fallback or replay loop is introduced.

Real-instruction tests change PDE/PTE only during the locked second read and
verify new read/write targets, preservation of the old target and new control
attributes. Injection covers begin/read/write failure, exact completion counts
and no target transfer. Valid GDT/IDT/stack PF cases prove second-value absence
and supervisor-WP faults, CR2, restart point and pushed error/EIP/CS frame,
without target access or A/D writes. Initial fixture counts incorrectly treated
exception descriptor reads and the stack-page D update as target activity;
separate target counters and the explicit successful stack update correct
that fixture error while retaining architectural assertions.

Cache/TLB/alignment/RMW plus Core inspection/transaction tests pass six cases
per width (x64 as 4+2, x86 as one six-case batch), then rebuilt cache checks
pass per width for final PF assertions. User/VM86/PDE permission changes,
cached-entry and split-redirection matrices and actual Core paged execution
remain unqualified. This does not close C04/S5 or open production 486 admission.

### PTE Second-Value Permission Matrix

The original two supervisor PF cases are expanded in the same fixture, not
duplicated into another runner. All CPL0--3 and VM86 modes, WP off/on, read
and write directions and second-PTE absence/user restriction/read-only cases
produce 60 source-qualified outcomes. Expected outcomes are selected by the
named scenario rather than calling production permission helpers. Successful
cases verify the new target contents and exact A/D update; rejected cases
verify real PF code, CR2, restart point and EIP/CS/error frame with no target
transfer or rejected-PTE update. Fault delivery's separate stack-page update
is explicitly counted, including the VM86 ring-zero stack frame.

The rebuilt cache test passes x64 and x86. This covers second-PTE privilege
changes, but not changed PDE permissions or cross-page redirected commits.
Directory faults may affect instruction fetch and the exception-delivery
tables themselves; those fixtures must provide a separately valid handler
mapping rather than hide a real double/triple fault or weaken PF assertions.
No production code or interface changes are made by this matrix expansion.

### Directory And Split Completion Matrix

The same permission fixture now covers PDE as well as PTE: 120 outcomes
across CPL0--3/VM86, WP, read/write and absent/user-restricted/read-only second
values. Directory cases retain a distinct valid exception PDE for GDT, IDT,
handler and stack, rather than hiding the resulting PF behind an invalid
delivery mapping. Assertions distinguish fetch faults at linear zero from
operand write faults and verify directory A's established early publication,
exact commit/cancel counts, saved frame and absence of rejected target access.

Cross-page word cases redirect both PTEs at their locked second reads. Normal
reads/writes use both new targets and preserve both old targets. If the second
page becomes absent/read-only, no target transfer occurs: the complete access
is still preflighted and committed before data. The first already-completed
A/D update remains published; no storage/cache rollback or replay is invented.
Real PF assertions retain the second linear address and instruction restart.

The replacement matrix now includes ordinary read and write hits for every
way and every tree state, checking data, write-through count and no fills.
All additions pass in the rebuilt existing cache test on both widths. These
finite proofs support the upcoming whole-S qualification, not whole-suite,
integration or artifact success; C16 physical timing remains in this T.

### Locked RMW Cache Sweep — Confirmed Open Gap

After stabilizing the above matrix, the same-class sweep found that ordinary
RMW still calls the ordinary cache read path. PREFIX_LOCK validates/stores
the prefix but does not alter cache selection, and memory XCHG does not
establish its implicit lock. Developer 3.1.5 requires a locked read to use
the external bus, while PRM 13.1.2 states memory XCHG is implicitly locked.
This is cache/data semantics, not merely missing C16 timing or prefix legality.

An ignored, one-shot diagnostic `build/t560-s5-lock-probe.c` links the same
existing x64 CPU/FPU libraries without changing the running unit cohort.
The selected SX fixture sets RAM=1111h, its real test cache=2222h and AX=3333h
in reset CD/NW mode, then executes memory XCHG. Actual result is status=0,
AX=2222h, RAM=1111h, cache=3333h, fault=0: the normal cached path was used.
The expected locked bus exchange uses RAM for the read and performs the
external write. This diagnostic is evidence of the gap, not production,
desktop, full-suite or cross-width qualification.

S5 must reconcile the locked read/write path, implicit XCHG and accepted LOCK
forms at the existing RMW/cache/Core transaction owners before closure. Do
not close C07 while relabelling this as timing debt. The current whole-unit
run is a baseline regression observation and cannot prove the untested lock
semantics. No Shared change or production admission is authorized by this gap.

### Unified Memory-Lock Repair

The unpublished page-update-specific pair is replaced, not aliased, by
begin_memory_lock(context)/complete_memory_lock(context, commit). Its real
consumers are page-table updates and the existing RMW path. The Core
transaction kind is now CPU_MEMORY_LOCK. Reads/writes retain their original
callbacks and per-cycle controls; one held owner covers split operands and
any required instruction/page-table memory cycles. Core's existing transaction
address/value identify only the actual write while self-snoop is considered,
then return to the held signal scope. Provider/alias writes outside that range
are not suppressed. No extra arbiter, manager, queue or compatibility path exists.

One private lib_bool memory_lock_active records the actual CPU LOCK signal,
not the lexical prefix. It is necessary to bypass cached reads and force the
external write in CD/NW cache-as-RAM mode. The existing RMW preflight runs
before assertion; memory XCHG passes its implicit requirement explicitly,
leaving lock-prefix diagnostics truthful. All other RMW callers keep their
existing handlers with an explicit non-implicit argument. ExecFinal releases
the signal before guest exception/interrupt delivery. Page updates borrow an
already-held signal or acquire/release their own. Older profiles are unchanged.
Locked reads still resolve live A20 before the bus; no fill, lookup-address
mirror or alternate data path is introduced.

The existing cache fixture now checks XCHG/ADD with/without explicit prefix,
byte/word/dword widths and all three legal CD/NW modes: 36 comparisons plus
begin/read/write failure checks. Unlocked ADD is retained as the normal-cache
control. Real Core XCHG through a RAM alias proves fresh bus data, external
write, retained raw-hit update and the distinct NW snoop policy. The actual
board control callback attempts DMA during read and write; both requests are
excluded while CPU ownership remains intact. This uses an effect callback,
not diagnostic trace as a command. Prior PDE/PTE, cache and fault proofs remain.

The eleven-test cache/exchange/XADD/CMPXCHG/RMW/alignment/TLB/TR/legacy-LOCK
and actual-Core inspection/transaction cohort rebuilds and passes 11/11 per
width. This repairs the diagnosed data-source defect. Complete accepted LOCK
form/fault audit, refreshed whole-unit/integration/artifact qualification and
C16 pin/clock fidelity remain distinct outstanding requirements; no full
CPU qualification or release is claimed.

### Actual Core Paged Execution

The existing actual-Core cache fixture now exercises paged instructions on
the production CPU bus and registered provider. As before it creates an
admitted 386 construction then binds the selected chip only inside the
Core-owned unit fixture; the production 486 construction gate remains closed.
The provider changes the operand PTE at its locked second read and observes
the real CPU_PAGE_TABLE_UPDATE transaction. The instruction reads the new
RAM target and retains the old target. Later TLB-hit reads do not query the
provider again, while the first write to that clean translation performs a
new second read and publishes Dirty to the actual provider before data write.
Every pair releases ownership and no cancellation is unexpectedly introduced.

A second case seeds one stale line in the actual CPU cache while RAM contains
a different PTE. This is fixture selection of a real hardware-visible cache
state, not a shadow cache or test public API. In both normal and CD/NW
cache-as-RAM modes the locked read selects RAM, the data access reaches its
new target, and successful A publication updates both RAM and the still-valid
cache hit. Source qualification remains logical locked-cycle behavior, not
pin timing or a claim about a newly admitted board personality.

The existing Core inspection test was rebuilt after each cohesive addition
and passes on x64/x86. These proofs replace the earlier absence of actual
Core paging evidence for these paths, but do not cover the entire privilege,
changed-entry failure, split-redirection or arbitration matrix. No production,
Shared-six, App, asset or owner-input changes were needed in this increment.

### Already-Satisfied Page Bits

Developer 6.4.5 describes the extra update only when A/D bits need setting.
The selected 486 walk now omits PDE writes when A is already set and PTE
writes when all bits required for that read/write are already set. Older
profiles retain their original writes. Existing prepare/commit ownership,
split-page preflight and TLB publication order are unchanged; this is not
the still-required second-read/locked-update implementation.

The direct matrix covers both 486 and retained 386, eight PDE-A/PTE-A/PTE-D
combinations and read/write directions: 32 cases. It counts actual page-table
writes and rejects any write in the 486 already-satisfied cases, proving these
walks succeed even when page-table writes are denied. Missing-bit write failure
is injected for both read and write: existing CPU internal fault, restart EIP,
unchanged fail-before-effect PTE, no target read and no target TLB publication.
The 64-case PCD/PWT test now starts with A clear where a write is asserted,
so it still exercises control delivery rather than expecting an unnecessary
write. Its data/control/hit/split assertions are retained.

Rebuilt cache/TLB/alignment/RMW tests pass 4/4 on x64 and x86. This directly
checks the retained preflight boundary; it is not complete paging or whole-S
qualification. No interface, state, alternate path or Shared change is added.

### Lookup And Fill Responsibility

Review found that the initial query checked all 16 routes on every cache hit
and write, although these operations only need the live lookup address.
The same callback now takes a typed LOOKUP/RESET_LOOKUP/FILL_PROBE purpose.
There is no additional callback, A20 mirror or persistent eligibility cache.
CPU asks for fill eligibility only on a non-preview read miss with CD and PCD
clear. Writes and cache hits use lookup only; reset lookup never probes fills.
The private Core helper returns before provider/mapping iteration for lookup.
Real fills still inspect every constituent byte, preserving narrow provider,
alias, A20, reset-overlay and speculative-failure rules.

Direct Core assertions check that ordinary/reset lookup invoke no provider
queries and invalid query values fail before routing. CPU counters check one
fill probe for a read miss and none for subsequent hits, writes or cache-as-RAM
operations; the complete replacement matrix checks both miss and hit counts.
This optimization does not claim measured end-to-end speedup or change any
Shared interface. Production 486 admission remains closed.
The updated cache, actual Core memory-inspection and timing-preview tests pass
3/3 on each width after rebuilding in the original Ninja/ccache graphs.
Both aggregate unit-consumer builds then completed successfully: x64 954
actions and x86 951 actions. The newly extended cache test was rebuilt once
more on both widths before rerunning the three-test selection, which passed
3/3 per width. These compile and focused results are not full-suite execution.

The new reset test poisons all cache lines, LRU entries and both buffers, then
invokes the real CPU reset and checks its deterministic cleared private state.
This verifies the implementation's initialization, not a claim that Intel
architecturally specifies every invalid tag/data byte after reset. Snoop tests
cover zero bytes, a range saturating at the 32-bit address end, an oversized
whole-address-space range and preservation of invalidated tag/data/LRU.
They verify that a high-address write cannot wrap and invalidate low tags;
both host widths use their actual lib_size width.

The replacement proof now covers all eight tree-LRU states against all 16
valid-way masks, plus every hit way for every LRU state. Ordinary MOV reads
trigger replacement and hit updates; a literal victim/next-state table checks
the expected tree rather than calling the production helper. The fixture
seeds private cache state only to select each starting condition. The first
test table incorrectly retained the root bit after upper-half hits; corrected
expectations clear that bit and preserve the unvisited lower-half bit.
The expanded existing cache test passes x64 and x86 after rebuilding each.

The actual Core alias sweep also checks CPU writes through a different decoded
address: only the range of the current CPU memory-write transaction is exempt
from self-invalidation, not unrelated alias/backing notifications. Older-profile
lexeme previews skip copying unused cache/TLB storage while still copying all
live scalar fields; selected 486 previews retain the complete private snapshot.
These changes add no Shared API, mirror state or alternate execution path.

Preliminary packet estimates remain production +400--800/-40--120 and tests
+450--800/-20--60 pending the exact bus contract. Positive code is justified
only by real cache state/operations and direct proof; no generic abstraction is
planned. CURRENT/proposal retain S5 active and T560 open.
