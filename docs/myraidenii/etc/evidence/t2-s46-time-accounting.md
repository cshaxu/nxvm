# T2 S46 Time Reconciliation And Escape/Mode Stage

Baseline3b96006c0. The owner requests stage closure on 2026-10-18 after
packaging unfinished work into the same T2's subsequent S batches. This
record distinguishes implemented escape/mode prerequisites from remaining
complete CPU/time/Machine/game qualification. It does not declare T2 done.

## Complete Remaining Accounting Matrix

The existing17-row task universe remains unchanged. This matrix refines
V03/V14/V15/V16 prerequisites without replacing their completion predicates.

| Class | Current real owner and fact | Required disposition/proof |
| --- | --- | --- |
| A ordinary native/NEC nominal rows | cpu_timing.c sole V30 selector has source/L2 rows for most completed forms | Reconcile every missing retired encoding and mode/form boundary, not blanket fallback |
| B packed scalar/HALT |27/2F/F4 complete but currently report zero; original NEC gives3/7/2 | Connect source rows and existing semantic tests, preserving unknown total time |
| C FPO escape identity/CPU bus |D8--DF retire without the documented dummy memory read;66/67 are incorrectly rejected as later Intel prefixes | One existing escape handler and scanner own both original NEC families, source2 register/11 aligned memory/15 odd memory; actual full CPU/data/failure proof |
| D aliases/nonstandard forms |82, register C6/C7, non-ten radix and repeated/ignored prefixes can execute without a source row | Source-backed defined/undefined/model disposition per class; no silent Intel timing substitution |
| E fetch/prefetch baseline |Existing six-byte V30 fetch cache and width splitting, no elapsed fetch accounting | Primary queue/odd-first/bus-cycle facts, one explicit overlap/serialization model and actual callback/purity/restart proof |
| F actual extra waits |External-cycle callback returns accepted wait counts; SX accumulates them in CPU, older families already accumulate them in the sole Core external_cycle_round_ticks envelope | Retain the existing Core owner for V30 reception, not a duplicate CPU wait accumulator; prove accepted COMMIT-only, once, memory/port/fetch/split/repeat/entry scopes, overflow/cancel and no preview/debug charge |
| G operand baseline |NEC nominal rows already include EA, operand fetch and execution | Never charge the same data-cycle baseline again; preserve per-form alignment terms |
| H software/exception/asynchronous entry |Known successful software/mode nominal aggregates; asynchronous/DE totals unallocated | Separate instruction/event/failed accepted prefix and exactly-once accounting before publication; no stale decoder state |
| I ordinary8080 |244 documented slots plus constrained ED escape execute; ordinary timing unknown | Establish a justified explicit model before assigning clocks; discrete8080 compatibility is not V30 timing evidence |
| J POLL/HLT and idle |Existing logical pin/deadline/wake owner, HALT nominal missing | Source initial versus later samples, no duplicate initial charge, idle time from sole timeline, real entry/return |
| K runtime/preview/maximum |Current V30 partial timing unallocated/max0; preview conservative for LOCK | Same owner and bounds for actual outcomes, failure/overflow and pure preview; never invent a maximum over unbounded waits |
| L neutral Machine |Validation currently rejects V30 | Admit only after complete required time/form predicate and real public-route execution/interrupt/deadline/failure proof; no flag-only bypass |

## Primary Source Reconciliation

Pinned original NEC October1986 V20/V30 manual SHA256
FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C.
Full printed3-4/PDF17,4-3/PDF22 and12-1/PDF46 are rendered and inspected.
V30 has six queue bytes, normally fetches words and begins an odd branch
target with one byte. Queue clears on control transfers/external entry.
A memory cycle is four clocks. Section12 nominal rows assume prefetched
bytes and already include decoding/EA/operand fetch/execution. Consequently
adding every data baseline again would be double accounting. Exact internal
overlap is not supplied merely by those facts; it must retain its grade.

Full printed12-173..175/PDF218..220 are also rendered and inspected.
FPO1 isD8--DF; FPO2 is66/67, not Intel operand/address-size overrides.
Both have a ModRM, no immediate, register no-operation and a memory bus read
whose value the CPU ignores. Their source CPU costs are2,11 aligned and15
odd clocks, independent of external floating arithmetic. No FPU execution
or new coprocessor provider is implied. Existing V30 admission with no FPU
does not justify dropping the CPU's address/read side effect.
The newer original-V30-inclusive NEC U11301EJ5V0UMJ1 manual, SHA256
19A69547651AA65E32DF33F1BF84DEA095A00237F680A67B166213FCC6D96F6A,
printed83--86, independently lists both formats and CPU memory-read effect.
Original even/odd difference proves a CPU word-read lane contract for this
model; it is not the coprocessor operand's arbitrary floating width.

Original packed-adjust and HALT pages retain3/7/2 clocks. Earlier semantic
contradictions resolved by S44 do not invalidate these independent clock rows.
No imported/reference implementation or table is used.

## Actual Discovery Probe And Limits

An ignored CPU-owned synthetic probe executes one actual CPU refresh for each
of256 primary slots times256 following-byte choices, then256 NEC suffix
slots times256 ModRM choices, then256 emulation slots times256 following
bytes:196608 bounded attempts. It uses the accepted x64 CPU/FPU libraries,
existing same-owner RAM fixture and explicit successful port callbacks.
No game ROM or Machine admission bypass is used. Captured unknown totals:
native4547 of61751 retired; NEC0 of872 retired at these seeds; ordinary
emulation62464 of62466 retired. These are diagnostic counts, not source
coverage or independent correctness assertions. Zero-count/zero-immediate
seeds cannot prove every branch, bit-field, byte count, arithmetic or fault.
The source/model matrix above, not the first failure, selects implementation.

Two initial harness issues are corrected before those terminal-zero counts:
the compiler cache declared its compiler UNINITIALIZED, not FILEPATH; and a
successful synthetic port callback originally lacked its matching required
completion callback, causing a probe access violation. No production crash
or product regression is inferred from an incomplete test provider. An
initial attempt used a nonexistent stdout alias; flushing existing Lib's
fflush with LIB_NULL is sufficient and no Types change is warranted.

## Before-Code Design

Classes B/C use the existing CPU selector and FPU_ESCAPE dispatch only.
One private original-NEC escape predicate serves scanner/form admission;
66/67 receive the existing escape handler only for V30. That handler decodes
ModRM, issues the existing two-byte memory read for memory forms, discards
the value and returns without calling a nonexistent coprocessor. Other CPU
profiles retain their original FPU/size-prefix paths. No public metadata
signature/layout or second dispatcher is added. Raw profile-blind metadata
continues to describe its existing Intel identity; the profile-aware actual
scan/admission is the correct NEC owner.

The existing V30 selector adds the three known scalar rows and both complete
escape families, using the decoded effective address/alignment and existing
source key/prefix finish. Successful nominal rows stay unallocated total
time until classesD--L are reconciled. No Machine gate opens in advance.
Expected first code increment: production+45--80/-10--20, test+250--400/-3--8.
This remains within the admitted larger S46 estimate. Further accounting
design and actual full qualification remain pending before S46 acceptance.

## Initial Implementation And Real Verification Findings

The complete escape matrix adds61440 actual executions: ten primary bytes,
all256 ModRM values, both address parities, both code-wrap layouts and six
no/segment/BUSLOCK prefix choices. Independent CPU/PC/flag snapshots, exact
read-lane plans and nominal keys/inputs are checked; no data write or FPU
callback is required. Truncated raw scan, pure preview and each applicable
fetch/read/observation refusal have direct proof at representative complete
addressing boundaries. The original66/90/00 and67/90/00 negative witnesses
are retained as truncated4-byte NEC memory forms in the new contrast, not
discarded to allow an Intel prefix interpretation. Six old Intel profiles
retain their own reject/size-prefix behavior. Native HALT and its segment/
BUSLOCK variants check the source initial cost separately from idle time.
The existing817152 decimal and40360 packed inputs retain all result/defined
flag/CPU/refusal assertions and gain their independently sourced3/7 rows.

The first escape execution finds a real shared fact-owner defect: the timing
prefix counter unconditionally treats66/67 as Intel prefixes, so66/00 is
misattributed as opcode00 and returns18 instead of11 clocks. The correction
does not add a second special V30 counter. The existing decoder's profile-
aware prefix recognizer becomes CPU-private shared code; its old timing-only
duplicate is removed. Prefix counting now takes the existing execution
context, respects original emulation mode (which has no native prefixes),
and uses that one recognizer. All existing private callers carry their actual
context; one existing string-transfer-plan helper receives it as well.
No public signature, layout, enum or persistent state is added.

One mechanical caller lacked a context variable and caused a compile error;
that private transfer-plan signature/caller is corrected. An initial test
selection was launched before its new target finished linking and reports
Not Run, not a product failure or valid pass. The corrected sequencing waits
for terminal build success; the next three-test x64 run passes escape/native/
decimal. Full dual-width X86 Core regression is now in progress; full S46
accounting, repository qualification and receiving publication are not done.

The first complete193-unit x64 Core run is terminal nonzero:191 pass and
two existing boundary units retain obsolete zero-clock expectations for
packed adjustment. Preserve all original19 ALU boundaries and four transfer
boundaries, adding the exact3/7 and segment-prefixed5/9 expected clocks/keys/
source grades. No guest/defined-flag/failure assertion is removed or relaxed.
The focused two-unit rerun passes. A new complete dual-width Core run is
required on this frozen corrected test set; the failed run is not green proof.

The replacement x64/x86 all-target builds and complete193/193 Core units
pass per width, zero failures/skips,23.67/41.41 seconds. Runner63615 is
terminal zero. This intermediate result does not substitute for final full repository or
receiving qualification. The original legacy Intel no-FPU consume-none path
is unchanged and has its own explicit source-audit TODO; NEC's documented
word-read side effect is not silently applied to different Intel bus designs.

## Remaining Boundary Review And Single Time Ownership

Actual Core source changes the next action for classF: older families do not
lose their waits. machine/cpu_bus.c already accumulates accepted configured
page/access waits in external_cycle_round_ticks, and machine.c publishes
that envelope once beside CPU timing. SX alone owns its deferred-write wait
inside CPU. Extending the SX accumulator to V30 without removing that Core
owner would double-charge; the design retains the existing old-family Core
envelope for V30 instead. A callback's return being unused by the CPU is not
evidence that the complete Machine path discards it.

The Machine retirement observer also has its own Intel-only prefix/ModRM/I/O
interpretation and currently receives no copied original MD fact. It would
misread native66/67 FPO2 and ordinary8080 bytes after admission. No current
V30 Machine execution is claimed, and no private CPU layout is imported to
work around this missing observation. Owner review is requested for one
appended lib_bool nec_emulation_mode in the existing public CPU instruction
observation: stamp from original instruction-entry PSW, false on Intel,
reuse completed observation before asynchronous entry mutates current mode.
No new function/callback, Machine mode latch or clock is proposed. The opaque
timing form already distinguishes mode, so another eligibility-key field is
not justified. Expected contract/receiver change+15--30/-5--15 source and
owned producer/receiver regressions; exact surface is reviewed before coding.
The owner explicitly approved this single appended readonly field in the
2026-10-18 review reply. CPU and existing Machine receivers implement it
together; no other public layout expansion is authorized by this approval.
The full time/admission predicate remains a separate, unfinished obligation.

No remaining source fact is replaced with a guessed one-tick instruction or
discrete8080 clock mislabeled as V30. Ordinary8080 timing still needs a
justified explicit model, not merely a compatibility claim. Full execution
time/max/Machine predicates remain closed while classesD--L are unresolved.

## Approved Instruction-Entry Mode Observation

The approved field is appended to cpu_interface.h without changing previous
members, enum values or functions. The existing copy_observation producer
reads instruction_state.data.oldcpu.data.eflags only for V30. Its existing
completed-observation publication preserves that fact across asynchronous
entry; Intel returns false even when its flags happen to contain the same bit.
There is no new retained mode state or Machine mode getter.

The existing Machine observer consumes this one copied fact: emulation has
no native prefixes, ModRM or 0F escape; its D3/DB port operations are byte
OUT/IN with an immediate port. Native V30 keeps 64/65 repeat prefixes but
treats 66/67 as FPO2, recognizes FPO1 ModRM and the existing NEC extension
formats. Intel receiver behavior is unchanged. This is interpretation of a
copied observation, not a second instruction executor or CPU-private import.

Producer regressions extend the existing native escape matrix and ordinary
8080 execution matrix. The real BRKEM/CALLN/RETI/RETEM program checks mode
before each actual instruction against its published copy. An asynchronous
INT test checks the just-retired emulation instruction remains marked as such
after native handler entry. Machine's existing test adds eighteen synthetic
mode/encoding receiver cases and an Intel-false assertion. Its explicitly
labeled receiver fixture borrows a real Intel executor only for the unchanged
timing accessor; it does not run V30, change an executor's profile or claim
that V30 Machine admission is complete.

The initial focused x64 builds and three selected tests pass. A first regex
selected only the Machine test because CPU registrations use underscores;
the two actual CPU names were then selected explicitly and both passed.
The frozen extended set passes complete dual-width Core regression in the
original cached graphs:193/193 per width, zero failures/skips,70.21/36.06
seconds. Runner42271 is terminal zero. Both all-target Release builds pass;
public header consumers necessarily rebuild. No alternate graph or cache
directory is introduced. Full repository, eight receiving EXE publication
and complete S46 accounting remain uncompleted. This result does not replace
the final task qualification or imply that V30 Machine/game execution works.

This approved mode slice adds44/deletes12/net32 production lines and adds82
test lines, excluding manifests/documents and the earlier escape batch. The
initial15--30 added-source estimate omitted the complete NEC0F and 8080 I/O
receiver interpretations; preserving an Intel-only receiver would leave the
approved fact unused at those real callers. No helper, public function,
eligibility-key member or persistent hardware state is added. Full S46 code
currently adds131/deletes77/net54 production lines and adds277/deletes11/net266
test/registration lines, including the new187-line escape test. These are
intermediate reviewed counts, not final task delivery or qualification.

## Owner-Approved Stage Boundary And Transfer

The owner explicitly requests that all unfinished S46 work and the status
review's roadmap be retained in later S tasks within this NEC CPU T2, then
asks to close this S. The coordinator narrows S46 before any further code:
the existing FPO1/FPO2 format/read/nominal batch, scalar-adjust/HALT nominal
rows and approved copied-mode receiver are the implementation delivery.
Full units, manifests/gates, eight affected PC releases, preserved inputs,
actual review and pushed target commits remain S46 closure requirements.

The [proposal](../../proposals/m1-v30.md#owner-approved-remaining-s-plan)
owns the intended next five batches, not already active packets: S47 residual
semantics/forms/prefix/arrival dispositions; S48 complete native/NEC/8080
nominal timing; S49 full fetch/bus/wait/entry/idle time and runtime/preview/
bounds; S50 actual neutral Machine; S51 full V01--V17 CPU qualification.
Accounting A/D/I and nominal H/J go to S48, semantic D and residual V01--V14
to S47, E/F/G and elapsed H/J/K to S49, L to S50, and final reconciliation/
V17 to S51. Existing B/C results are retained, not declared full accounting.

No unfinished CPU capability leaves T2. Original real-ROM board checkpoints
remain mandatory at the [board/COP owner](../../proposals/m1-board-cop.md),
which implements their prerequisites. T2 retains a real public Machine route
with ROM-free synthetic non-PC bus/attachment/vector proof. This resolves a
circular prerequisite; it neither removes the game checkpoint nor pretends
it already passed. No MyRaidenII EXE, gameplay or silicon-exact claim follows.

## Similar-Issue Sweep And Actual Design Review

The coordinator reviews the staged production/test diff, not just pass counts.
Queries use rg over src/x86-core/cpu, src/x86-core/machine and test/x86-core:
`FPU_ESCAPE|CONSUME_NONE|_kdf_has_modrm|0x66|0x67`,
`instruction_is_prefix|instruction_prefix_count|capture_io|modrm_index`,
`copy_observation|completed_observation|NEC_PSW_MD`, and
`external_cycle_round_ticks|timing_transfer_ticks|advance_wait`.
The scope is CPU opcode/form dispatch, escape effects, prefix/timing facts,
copied retirement facts and their real Machine receivers, not unrelated
devices or App policy. Corpus/Types/manifest gates separately check the
declared boundaries; ordinary literal data bytes are not opcode-path hits.

Actual format scan, minimum/form admission, primary dispatch and FPU_ESCAPE
all use the one original-NEC escape classification. Existing CPU-private
prefix recognition replaces the timing-only duplicate; all private count
callers carry the actual context, including string-transfer planning.
Machine keeps its public-contract receiver instead of importing CPU-private
decoding. It now distinguishes native/8080 prefix/ModRM/NEC0F/I/O facts through
the approved copied mode. Initial BRKEM, CALLN/RETI/RETEM and asynchronous
entry demonstrate original-mode preservation rather than a current-MD guess.

Retained paths have distinct owners: Intel size overrides and coprocessor
actions keep their old generation contract; old Intel no-FPU dummy-bus
qualification is a named separate TODO, not an implicit NEC behavior change.
Core's existing old-family wait envelope remains authoritative; no extra CPU
accumulator is added. Unknown V30 alias/prefix/radix/entry/emulation/total-time
and admission classes retain the exact S47--S51 receivers above; synthetic
receiver tests do not stand in for S50 runtime. Public shape adds one approved
copied bool only. One small private escape classifier is justified by scanner,
dispatch and handler users; there is no new manager, timing registry, runner,
clock or hardware-mode mirror. Positive net source adds the real CPU bus
effect and receiver interpretations rather than forwarding layers.

All original decimal, ALU and transfer boundary inputs remain. Removed old
66/67 rejection witnesses reappear as exact truncated NEC forms in the new
matrix; only their incorrect unsupported-opcode interpretation is removed.
The nineteen ALU boundaries and four transfer boundaries keep their full
CPU/failure checks while source-backed packed-adjust rows replace zero-time
expectations. Native LOCK preview remains explicitly unavailable, not silently
promoted. Six reviewed S46-only PNGs are removed after review; pinned original
manuals, bounded diagnostic sources, logs and reusable graphs/cache remain.

## Receiving Static-Gate Correction During Closure

The first final driver49580 completes all680 x64 repository units with zero
failures/skips in511.00 seconds, then terminates nonzero at the specialized
successful-sentinel matrix. That NXVM-owned static check still requires the
obsolete private-call text prefix_count(data). The real common CPU helper now
requires context to distinguish V30 escapes and emulation from Intel prefixes.
The original failed gate log is retained separately, not counted as a pass.

The coordinator records this necessary receiving-check correction in the
active and archived packet before changing it: one token now requires
prefix_count(context, data), with no removed assertion, widened alternative or
production change. The complete src/test/cmake/tools old-call query finds only
that gate; it is empty after correction. Direct execution passes. The final
driver resumes after the unchanged, already terminal-successful x64 unit run,
reruns all specialized gates, then continues x86 units/gates and both receiving
release sequences. It does not repeat680 successful tests merely because a
later independent static check had stale text. NXVM check diff+1/-1/net0 is
reported separately from the frozen Shared source/test counts.

## Final Bounded-Stage Verification

The corrected final driver18325 is terminal zero. Both original Ninja/ccache
Release all-target graphs pass and finish at the default profile. Complete
repository units pass680/680 per width (193 X86 Core), zero failures/skips,
511.00/168.82 seconds. The already successful x64 unit run is retained from
driver49580; only its later stale static assertion needed correction. Both
complete specialized gate runs pass498 strict rows and1391 tracked Types
inputs. No production file changes after the frozen qualification baseline.

Four profiles per architecture rebuild and deploy the eight current0564 PC
EXEs. Each width passes14 selected receiving integrations and one additional
restored-default identity. Every profile switch and restore recompiles zero
CPU objects; the collector verifies both Ninja path separators. All ten actual
release PE widths/SHA256/sizes/stripped sections are collected. MyNES0044 and
all owner INIs remain unchanged. Six selected media files are independently
read before/after execution, retaining size/SHA256/access-mode identity.

[Results](t2-s46-results.json), [artifacts](t2-s46-artifacts.json) and
[media inputs](t2-s46-media-inputs.json) are actual post-run collections.
The collector checks the full frozen source/test input digest set and the
real XML/logs rather than constructing green results from expected counts.
Both owned manifests, inward/Types checks, changed Markdown file links and
MyRaidenII/NXVM documentation governance pass. Actual staged/worktree diffs
are reviewed against the owner-revised packet and original17-row ledger.

Shared production6 files+131/-77/net54; tests/registration8 files+277/-11/net266,
including the new187-line escape test; two manifests separately+14/-13/net1.
Counts use git diff numstat against3b96006c0, including the newly staged file,
excluding docs/assets/ignored orchestration. NXVM's receiving static assertion
is separately+1/-1/net0. The sole new public member is the approved copied
bool; no new public function/type/enum, hardware state, worker or clock exists.
One private escape classifier and the consolidated prefix fact have real
users; legacy Intel behavior and FPU arithmetic remain unchanged.

The six reviewed PNGs and the stale S45-linked inventory executable are
removed. Pinned PDFs, bounded probe source, logs and original build/cache
graphs remain for evidence and immediate successor work. No full external/
native integration suite, manual desktop/RDP/audio, whole archive audit,
silicon-exact time, actual V30 Machine route or gameplay is claimed. Existing
legacy warning exemptions are unchanged;498 strict rows are not a claim that
the entire source tree builds without exemptions. Only the revised S46 batch
is eligible for acceptance; all remaining CPU work stays at S47--S51 in T2.

## Executor Delivery

Shared P1 `128204d91` contains the reviewed frozen CPU/Machine changes,
owned tests and two manifests. NXVM P2 `687a59755` publishes all eight0564
receiving EXEs, the one unchanged-purpose static assertion update and the
receiving receipt. Both commits are pushed. The following MyRaidenII delivery
records actual qualification and the owner-approved residual S47--S51 plan;
coordinator acceptance remains separate. No unfinished CPU requirement is
declared complete by this stage delivery.

## Archived Owner-Revised S46 Packet

This archives the bounded active packet before closure. Current alone owns
admission/status; this table is evidence of the revised stage assignment.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: MyRaidenII M1 T2 S46, next unused S after accepted S45; one active packet. |
| Admission And Approval | Owner's persistent full MyRaidenII implementation request and standing automatic S/V30 mechanism approval; S46 originally admitted full remaining accounting after S45. On 2026-10-18 the owner explicitly requests packaging unfinished work into later S batches within this same NEC CPU T2 and then closing S46. This authorizes bounded stage closure, not CPU/T2 completion. The separately approved appended lib_bool nec_emulation_mode remains included. Targets Shared, NXVM receiving releases/receipt and MyRaidenII evidence; Lib/Emulator/MyNES remain excluded. |
| Objective | Deliver the implemented complete NEC FPO1/FPO2 CPU-bus/format/nominal batch, source-backed packed-adjust/HALT nominal rows and approved instruction-entry mode observation with existing Machine receivers. Reconcile every unfinished accounting/semantic/admission obligation into proposed S47--S51 within T2, with finite exits and preserved V01--V17 receivers. Resolve the circular real-board acceptance dependency explicitly, without claiming CPU or gameplay completion. |
| Non-goals | No board/COP/video/sound/game completion, third-party source import, guessed silicon timings, copied reference tables, Intel-profile changes, second clock/runner, permissive fallback or protected-media modification. This S does not redefine T2's complete form/entry obligations as timing-only success. |
| Reference Baseline | Clean3b96006c0; pinned NEC1986 and NEC2000 manuals, existing primary8080AF/Intel8080 documents as semantic evidence only unless an explicit timing model is justified; S45 source/results/ten release/six selected media guards. Actual source and callers outrank old progress prose. |
| Candidate Proposal | ../proposals/m1-v30.md and ../etc/t2-v30-ledger.md V03--V16 with emphasis V15/V16. Freeze one finite accounting matrix with source/model/unknown dispositions and exact receivers, not another sequence of unrelated opcode fixes. |
| Files And ABI Surface | Frozen X86 Core CPU format/instructions/timing and existing Machine observer; owned tests/manifests; NXVM eight0564 EXEs/receipt and verify_successful_sentinel_matrix.cmake's stale private-call assertion only; MyRaidenII Current/proposal/roadmap/ledger/history/evidence and board receiving proposal. Actual Shared source+131/-77/net54; tests/registration+277/-11/net266; NXVM static assertion+1/-1/net0, excluding manifests/docs/artifacts. One approved appended readonly nec_emulation_mode field; no public function/callback, other layout change, new timing enum or Machine admission. No further production implementation is added during this stage closure. |
| Applicable Rules | Execution16 fields, complete mechanism/ledger, actual-diff review, full units and receiving releases; architecture one CPU/bus/timing owner, no parallel mode/clock; Coding Lib vocabulary and original table style; primary source facts with explicit L2 when modeled; no imported implementation; Document current/history/packet retention. Architecture/coding governance and read-only PDF skills apply. |
| Verification | Independently reconcile all affected mode/slot/form/outcome classes and required source pages. Actual CPU callbacks prove zero/nonzero waits, parity/split/fetch/prefetch, entry/failure/overflow, preview purity, reset and repeats; native/8080 result/time distinctions and old Intel contrasts. Both original Ninja/ccache Release all-target builds/full repository units, strict/Types/dependency/manifests; eight current PC0564 releases with14 selected receiving integrations and restored-default identity per width; actual ten PE/hash/stripped checks and fresh six selected media before/after guards. Any newly enabled Machine route needs direct owned runtime proof, not just validation tests. |
| Expected Markers | Existing complete V30 semantic markers remain; new timing/accounting markers identified by the frozen matrix. Zero unit failures/skips both widths. Unknown source facts may not be reported as physical timing or Machine qualification. |
| Asset Needs | Existing ignored primary PDFs/text and bounded owned page renders; ROM-free synthetic CPU/Machine inputs. Existing receiving ROMs/media read-only or existing overlay; protect MyNES0044 and all INIs. No game-ROM acquisition or patching. |
| Reporting Requirements | Report reconciled missing classes and chosen accounting before implementation, then actual source/test added/deleted/net, retained owners and new helper/state/API justification, complete results and receiving hashes, model grades and all unverified desktop/external/gameplay boundaries. |
| Stop Conditions | Stop before unapproved public function/layout/Lib/Emulator changes, source/license conflict, fabricated source values or a model requiring a new owner decision; report that precise boundary and continue independent in-scope reconciliation. Material scope/model/acceptance revisions precede further implementation. |
| Exit Criteria | Implemented FPO/nominal/mode batch passes direct runtime/failure/preview and Intel regression proof; complete repository units/gates and required eight receiving releases pass both widths. All unfinished A--L and V01--V17 obligations have exact S47--S51 receivers in the same open T2; original real-ROM board checkpoints are explicitly retained in the board proposal as approved, not declared passed. No complete-time/max/Machine gate opens. Actual diff/ledger review and complete target commits must be pushed before coordinator stage acceptance. |
| Original Owner Request | Single-session dual-role implementation of playable MyRaidenII including NEC V30 and needed facilities, without shrinking full scope to local test success. |
| Similar-Issue Sweep | All V30 selector early returns, profile/form eligibility, native and8080 table/entry paths, CPU external-cycle phases and wait counters, prefetch invalidation/continuation, result publication/overflow/fault prefixes, preview/max and Machine time qualification; separate legacy Intel/SX semantics remain untouched. |
