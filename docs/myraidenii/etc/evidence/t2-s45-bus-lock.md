# T2 S45 Native BUSLOCK Arbitration Lifetime

Baseline `c4a92e0e1`, after accepted S44. This consumes the V05/V14
instruction/block bus-lock lifetime and neutral port receiver, not all
prefixes, physical time, maximum, V30 Machine admission or game readiness.
Corrected runner46811 is terminal zero. The collected results below describe
executed complete suites, not the deliberately stopped initial partial run.

## Primary Boundary And Actual Before Witness

The original NEC V20/V30 October1986 manual SHA-256
`FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C`,
printed12-173/physical PDF218, and NEC V Series Instruction manual
U11301EJ5V0UMJ1 fifth edition September2000 SHA-256
`19A69547651AA65E32DF33F1BF84DEA095A00237F680A67B166213FCC6D96F6A`,
printed47/physical PDF58, are rendered and visually inspected in full.
Both require the subsequent instruction's bus ownership and continuous
ownership across a repeated block. Large-scale mode has an external pin;
small-scale mode still inhibits HOLD acceptance. The existing callback pair
models arbitration ownership, not a new configurable electrical pin mode.
The later manual warns against BUSLOCK/POLL and disallows interrupt/step
acceptance between prefix and its instruction; neither becomes an invented
POLL semantic or electrical-clock claim.

The new actual CPU test fails before repair on BUSLOCK NOP:
`row=0 lock=1 begin=0 complete=0 active=0 bad=0 observed=0`.
Earlier nominal prefix accounting did not supply this physical ownership
effect. Merely enabling the modern RMW helper would still miss register,
load/store, stack, string and port instructions and would release each REP
element separately. No new opcode interpreter is justified.

## One Production Path

The existing native PREFIX_LOCK now acquires the existing memory-lock
callback/state for V30. Modern Intel opcode-whitelist legality remains
unchanged: its flagLock is not misused to reject valid early-family register,
load/store or stack forms. The existing prefix-presence helper is shared
CPU-privately by observation and timing; the redundant timing-only scan is
removed. This is an existing function's private linkage/type adjustment,
not a public capability, alias or state mirror. Public interfaces/layouts
remain unchanged. The Boolean helper uses Lib's Boolean vocabulary.

ExecFinal keeps the same instruction-owned memory_lock_active across a
successful REP continuation, and completes once when the block ends or fails.
An automatic previous-prefix/opcode snapshot inside one ExecIns call detects
code replacement before dispatch; it is not persistent state and does not
derive arbitration from the separate nominal-timing repeat cache. Changed
CS/PC/mode similarly cancels a prior partial block. The existing finalizer
releases any held output on reset/destruction; asynchronous entry leaves the
old block before handler fetch/frame effects, and INT releases before its
acknowledgement transaction. IRET executes the actual remaining block and
acquires one new lock. No second execution, bus, recovery or time path exists.

Machine's real transfer_port callback preserves an outer CPU_MEMORY_LOCK
transaction, just as existing memory callbacks do. Its completion does not
commit the outer owner, and a refused lane does not cancel it prematurely;
the CPU lock finalizer alone owns completion/cancellation. Accepted device
write prefixes remain accepted, not replayed or rolled back.

## Owned Proof And Initial Harness Corrections

The CPU-owned test reuses the existing real memory provider and injects
actual lock/port callbacks. It checks22 locked/unlocked scalar contexts:
NOP/register ADD, memory ADD/MOV/XCH, PUSH immediate, both scalar port widths/
directions and NEC bit operation. Unprefixed XCH receives no invented implicit
V30 lock. Preview remains conservative for LOCK, performs no real transfer
or lock admission and preserves the complete CPU image.

Four count states0--3, both directions and both parities span all fourteen
ordinary block/compare/load/store/port string primitives:224 program contexts.
Callbacks must remain held between actual element refreshes, begin once and
complete once. Zero count and early comparison termination both release.
Additional failure contexts cover begin refusal, both odd-word read/write
positions, first/pre-body fetch, port refusal and either missing callback.
CPU rollback, absent successful completion, exactly-once cancellation and
accepted RAM-prefix retention are asserted. Only a sole-owner bus may omit
both callbacks under the existing contract.

Lifecycle contexts cover idempotent finalization, reset, actual INT/NMI entry
and IRET continuation, code replacement with/without BUSLOCK and PC replacement.
The existing Machine executor unit adds40 actual route contexts, combining
outer lock/no lock, read/write,1/2/4-byte widths and refusal at every lane.
Every outer transaction field remains byte-identical across the port access;
accepted provider-write prefixes and final owner release are explicit.
CPU and Machine tests stay under their respective owners, with no App fixture
or test-only production API.

Initial test-development failures are retained rather than misreported as
product regressions. The shared memory-only fixture originally rejected port
phase space; the new test-local cycle adapter delegates memory phases and
does not reinterpret port data. One post-prefix fetch witness originally fit
entirely in the prefetch window; its FFFF/FFFE starts now force the declared
next fetch. The byte-latch test provider originally retained shifted upper
bits instead of consuming its byte lane; masking that synthetic latch fixes
its independent accepted-prefix expectation. A dangling-else compiler warning
in the new control macro use was fixed with braces. An incorrect target name
and an ignored PowerShell runner-generation quoting error produced no valid
qualification and were corrected before the current persistent runner.

## Similar-Issue Sweep And Retained Distinctions

```text
rg -n 'begin_memory_lock|complete_memory_lock|memory_lock_active|flagLock' src/x86-core test/x86-core
rg -n 'transaction_begin|transaction_commit|transaction_cancel' src/x86-core/machine/cpu_bus.c
rg -n 'PREFIX_LOCK|ExecBoundary|ExecInit|ExecFinal|ExecInt|execution_finalize|execution_reset' src/x86-core/cpu
```

Memory callbacks already preserve the outer lock; their semantics remain.
Port nested admission/completion was the actual missing receiver and is fixed.
INT acknowledgement must occur after the interrupted block's release and now
does. Older Intel profiles retain their pre-existing unqualified physical
callback lifetime, explicitly recorded in the selected product's
[debt receiver](../../states/TODO.md); no current guest corruption or new
regression is asserted and no V30 obligation is transferred there.
Cache/special operations are modern-only and not legal locked V30
forms; old Intel whitelist/A-D/cache lock behavior remains distinct. The FPU
command callback is only a transaction/trace marker, not the coprocessor
operation itself; V30/FPU admission is absent, and no false FPU capability
is introduced. CPU entry and existing state reset/finalize remain the lifetime
owners. Undefined BUSLOCK/POLL and general long/noncanonical prefixes stay
their original V05/V15 receivers, not implied qualification.

Actual-diff review before publication found a genuine new boundary defect:
the pure preview also calls ExecInit with instruction_in_progress false.
During a held REP it released the real callback even though its CPU record
was only a copy. A new mid-block preview witness fails before correction:
`op=a4 count=2 part=0 active=0 complete=1`. ExecInit's asynchronous-release
condition now explicitly excludes preview_mode. Every partial iteration in
the full repeat matrix verifies that conservative preview neither admits nor
releases a lock and preserves the complete CPU. The initial qualification
runner76932 was deliberately stopped and confirmed terminal, not treated
as a passing/failing complete suite; its partial log is retained separately.
Only the replacement run on corrected frozen source may supply acceptance.

## Verification And Receiving Publication

Both original persistent cached Release graphs perform complete all-target
builds, transient focused regressions, repository-only units and strict/Types/
dependency/manifest checks. Eight current0564 PC products are rebuilt/deployed
in those same graphs, with selected receiving integrations and final default
restoration; profile switches must not recompile CPU objects. Ten PE/hash/
stripped identities and six selected media actual before/after size/SHA/mode
guards are collected only after the runner exits successfully. MyNES0044 and
all INIs are protected. No new full external/native integration, desktop/RDP,
physical audio or gameplay qualification is inferred. All original V01--V17
rows, full T2/proposal/Queue and playable objective remain open.

### Actual Final Results

Both widths pass7/7 focused and679/679 repository units (192 X86 Core),
zero failures/skips; full unit wall times308.17/133.55 seconds. Strict498 and
Types1390 pass. [Results](t2-s45-results.json) retain the actual XML/log
collection. Each width passes14 selected receiving integrations and one
restored-default identity. Both original persistent Ninja/ccache Release
graphs end at default; profile switches and restoration compile zero CPU
objects. All eight PC0564 EXEs are rebuilt and deployed; all ten actual
PE/size/SHA/stripped identities are checked in [artifacts](t2-s45-artifacts.json).
Both MyNES0044 EXEs and all INIs remain byte-identical to the baseline.

The collector reopens all six actual selected media after execution and
compares size/SHA/access mode to the fresh pre-run
[inputs](t2-s45-media-inputs.json); every comparison matches. This is not an
audit of the entire external archive. No full external/native integration
suite, manual desktop/RDP, physical audio or game execution is claimed.

Tracked code counts use git diff --numstat against c4a92e0e1, excluding
manifests, documents and artifacts: production5 files+68/-37/net31;
tests/registration3 files+411/-1/net410; total+479/-38/net441.
Two manifests separately+8/-7/net1. The new test's337 lines independently
check scalar/repeated/failure/lifecycle behavior; the existing Machine test
adds40 real port routes. No public API, enum, layout, persistent field,
clock, thread, manager or second execution path is added. One existing
CPU-private helper is shared and one redundant prefix scan is deleted.
The positive code growth supplies the missing lifetime and independent
failure proof, not a new execution framework.

Actual source/test diff review confirms all changes remain in X86 Core,
Intel-specific legality/cache/A-D paths retain their owners and original
tests, and all seventeen convergence rows remain. Exactly two S45-owned
reviewed PDF page images are removed after review; PDFs/text/logs/cache are
retained for subsequent V30 work. Receiving publication and executor
delivery precede independent coordinator acceptance; T2 stays open.

## Archived Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: MyRaidenII M1 T2 S45, next unused S after accepted S44; one active packet. |
| Admission And Approval | Owner's persistent full MyRaidenII implementation request and standing automatic S/V30 mechanism approval; coordinator admits CPU BUSLOCK and neutral locked-port receiver on 2026-10-18. Targets Shared, NXVM receiving artifacts/receipt and MyRaidenII evidence; no Lib/Emulator/MyNES or public API extension. |
| Objective | Consume V05/V14 BUSLOCK arbitration lifetime: one lock around the native instruction and complete repeated block, correct failure/interrupt/reset/finalize release, no Intel whitelist substitution. Use existing CPU lock callbacks/state; neutral Machine port cycles must preserve the outer lock transaction. |
| Non-goals | No V30 Machine eligibility, full electrical timing or pin-mode model, new clock/worker/state machine, implicit-XCH lock claim, BUSLOCK/POLL undefined behavior, whole native/prefix/8080 qualification, board/game completion or protected-media modification. |
| Reference Baseline | Clean c4a92e0e1; original NEC1986 BUSLOCK printed12-173/PDF218 and NEC2000 printed47/PDF58 require actual page confirmation; current CPU/memory-lock/transaction owners and accepted S43 ten release/six media identities. |
| Candidate Proposal | ../proposals/m1-v30.md and ../etc/t2-v30-ledger.md V05/V14/V15/V17; repair the complete externally observable lock-lifetime batch, not only one RMW opcode. |
| Files And ABI Surface | CPU instructions/timing private units/header as needed to share existing prefix fact, Machine cpu_bus.c, owned CPU/Machine tests/registration and manifests; NXVM eight0564 EXEs/receipt; MyRaidenII evidence/ledger/history/Current/index. Expected production+60--90/-25--40, tests+300--450/-0--20. No public signature/enum/layout or persistent field addition. |
| Applicable Rules | Execution16 fields, actual-diff review, full units, one target per pushed P and receiving releases; architecture sole instruction memory_lock_active and transaction owner, private prefix fact not mirrored latch; Coding Types/table style/failure-prefix preservation; source policy primary facts only/no imported source; Document history/current/packet retention. Local architecture/coding skills and read-only PDF skill apply. |
| Verification | Before/after synthetic actual CPU bus callbacks prove all relevant scalar memory/register/port and repeated string classes, segment/order/zero/odd/split, no implicit lock, no preview effects, failure at lock/fetch/read/write/port boundaries, external entry and IRET restart, code/context replacement and reset/finalize. Machine owned test proves port transaction preservation/failure. Both original Ninja/ccache Release all-target builds/full units, strict498/Types gates/manifests, eight PC0564 builds and14 selected receiving integrations plus restored identity per width; fresh ten PE/hash/stripped checks and six selected media before/after guards. |
| Expected Markers | V30:BUSLOCK:OK and existing decimal/string/cache/entry markers; full registered repository unit count per width, zero failures/skips; no V30 timing/max/Machine gate opened. |
| Asset Needs | Existing ignored primary PDFs/text and owned renders; synthetic CPU/port vectors only. Existing owner ROMs and six receiving media stay unchanged in content/access mode. MyNES0044 and all INIs protected; no game ROM access or new asset acquisition. |
| Reporting Requirements | Confirm scope and estimates; report initial witness, mechanism/variant dispositions, actual production/test additions/deletions/net, new helper/state/API justification, complete results, receiving hashes and excluded external/desktop/RDP/audio/gameplay qualification; pushed commits and remaining original ledger. |
| Stop Conditions | Stop on required public/Lib/Emulator extension, source/license conflict, unowned state/second executor, or unrelated worktree overlap. Revise packet before material scope expansion; no model may silently become physical timing/admission proof. |
| Exit Criteria | Complete lock-lifetime batch and real neutral port receiver pass independent success/failure/cleanup proof both widths; existing Intel behavior and every original test remain; complete units/gates and eight receiving releases/select integrations/ten identities/six media guards pass; actual diff reviewed and all target commits pushed before coordinator acceptance. |
| Original Owner Request | Single-session dual-role implementation of playable MyRaidenII including NEC V30 and needed facilities, without shrinking full scope to local test success. |
| Similar-Issue Sweep | Trace PREFIX_LOCK, flagLock legality, begin/complete_memory_lock, ExecBoundary/Init/Final/Int, reset/finalize, preview/observation, CPU memory/port provider and Machine transaction nesting. Reuse one prefix-presence fact where applicable. Keep modern Intel LOCK whitelist/cache/A-D paths distinct; no claim for implicit V30 XCH or undefined BUSLOCK/POLL. |
