# T2 S49 Elapsed-Time Ownership And Bounds

## Admission And Baseline

Coordinator admits S49 after clean pushed a1f9829f9, following the accepted
same-T S47--S51 plan. Current owns the sixteen-field packet. This record is
the implementation inventory, not acceptance or a replacement completion
predicate. S48's complete nominal proof remains historical; its unit passes
are not new S49 execution evidence.

The owner automatically approves justified timing upgrades, not accuracy
downgrades. The scope is Shared X86 Core CPU/Machine, owned tests and the
eight affected current0564 PC receiving artifacts. Lib/Emulator, MyNES,
App configuration, firmware and media remain unchanged. The initial scope
excluded Machine admission; the explicit approved ordering correction below
now includes minimum genuine configured execution, not complete Machine or
gameplay qualification.

## Complete Accounting Batch

This consumes S46 classes E/F/G and elapsed H/J/K, original V14/V15.
Each row requires both-width actual proof before acceptance; no row below
is already accepted merely because its implementation was located.

| Row | Current actual owner and finding | Required implementation/proof |
| --- | --- | --- |
| E1 queue fill | ExecInit uses the existing six-byte V30 prefetch state, aligned word cycles and odd-first byte. _kma_read_bus records fetch baseline only for SX. | Attribute actual accepted fetch service once; resident queue hits add no transfer; reset, redirect, segment/page/code wrap and short remainder preserve the one queue. State model/overlap limits explicitly. |
| E2 decode beyond queue | Actual instruction fetch goes through the same read-bus publication, while diagnostic capture remains fifteen bytes. | Include accepted extension/immediate/prefix fetch cycles without re-reading code, another decoder or a diagnostic-buffer length assumption. Repeated/ignored prefix nominal remains S48's owner. |
| E3 prefetch reservation | cpu.c reservation advances real fetch only for8088; other profiles retain their existing reservation metadata without a fetch. | Do not invent V30 background fetch or charge a non-existent transfer. Preserve8088 behavior; document the qualified V30 queue model rather than silently claiming physical overlap. |
| F1 configured waits | machine/cpu_bus.c accumulates accepted old-family COMMIT waits in external_cycle_round_ticks; SX deliberately uses CPU-local bus service instead. | Keep the Core V30 envelope. Test memory/port/fetch/split/repeat/entry, zero and nonzero waits, COMMIT versus CANCEL, exact sums, reset and overflow. No duplicate CPU wait accumulator. |
| F2 publication | Machine combines CPU ticks, event_ticks and the Core wait envelope; event-only and pending-retirement paths already exist. | Preserve causal pending time and one publication, including fault after an accepted prefix. Failed operations do not charge a successful instruction. Accepted waits must not disappear on cancellation of a later transfer. |
| G nominal operand cost | NEC nominal already contains decoding, EA, operand fetch and execution; S48 owns odd word and stack adjustments. | Add no blanket per-memory-callback baseline to a successful nominal row. Native/NEC/8080 aggregate distinctions remain explicit; test same nominal and different accepted extra waits. |
| H1 completed software entry | S48 assigns combined native divide-error and software/mode-entry nominal aggregates before async dispatch. | Do not add another successful entry cost to these aggregates. Keep failed-entry accepted work distinct; test odd stack/operand independence and committed-mode observation. |
| H2 asynchronous entry | _e_intr_n/_e_except_n timing and failed-prefix allocation currently apply only to SX. V30 async callers use ExecInit, which can fetch the interrupted next instruction. | Account successful V30 NMI/INTR/step entry separately from retired timing; account failed accepted entry work without a successful floor. Audit whether next-instruction fetch is required, not inherited merely from the Intel old-family path. Preserve one entry path and copied observation. |
| H3 failure allocation | _e_timing_failed_prefix is SX-only; ExecFinal is its existing failure owner. | Accepted V30 physical prefix is not zero merely because retirement fails. One allocation marker prevents nested entry/failure double charge. Refused transfer itself adds no accepted cost. Reset and public debug/preview must not inherit execution costs. |
| J1 POLL | Initial nominal is7 for one sample. begin_wait still starts a7-clock countdown; Core advances that countdown while publishing the instruction's elapsed time. | Reconcile actual sampling instants and initial retirement versus later five-clock cadence, including fetch and additional waits. Do not change only the countdown literal without accounting for when Core advances it. Test initial clear/set, zero/partial advances, later clear, interrupt and reset. |
| J2 HALT/idle | HALT nominal is2; halted refresh returns without retirement and Core owns idle/deadline advancement and wake. | Never manufacture another HALT retirement or CPU idle clock. Verify no work/no time on bare halted refresh; elapsed idle belongs to the existing timeline and successful wake entry is separate.8080 HLT keeps its approved nominal model. |
| K1 preview | Existing public preview is lexical, not a full timing-preview API. It uses copied CPU/instruction state, observe-only reads and suppressed external publication. | Preserve purity: no waits, queues, trace/diagnostics, live decoder, sample or time mutation. Unavailable long lexeme is not execution failure or permission to scan through another decoder. Do not claim an existing public exact elapsed preview. |
| K2 bounds | V30 maximum_ticks is0; Core's pending retirement can stage actual completion separately. Long native prefixes and arbitrary external waits do not justify a fabricated finite physical maximum. | State the exact meaning of0 and actual-budget behavior. Test finite known costs, overflow, long prefixes, event-only/failure and bounded pending completion; reserve real public V30 Machine execution for S50. No guessed one-tick or constant physical bound. |

## Initial Actual-Source Reconciliation

Read-only queries cover src/x86-core/cpu and src/x86-core/machine, matching
test/x86-core owners, the original ledger and S46/S48 evidence. Queries include
external_cycle_round_ticks, timing_external_wait, timing_fetch_bus_ticks,
timing_transfer_ticks, timing_entry_transfer_ticks, _e_timing_entry,
_e_timing_failed_prefix, ExecInit/ExecBoundary/ExecInt, advance_wait,
wait_poll_remaining_ticks, source_repeat and timing_maximum_ticks.

The code confirms that extending SX's CPU wait accumulator to V30 would
double-charge the real Core path. A CPU-only provider returning waits is not
the Core timeline contract and must not be mistaken for missing Machine time.

POLL needs a timeline correction, not just a new interval constant. At the
current source level, the instruction samples once and publishes nominal7;
begin_wait sets remaining7, and Core decrements that countdown during elapsed
publication. This permits another sample while publishing the first nominal
interval. Tests currently advance a bare CPU manually and expect7; they do
not prove the full initial-retirement/idle seam. Fetch and external waiting
make a literal-only correction insufficient. No production edit is made yet.

The async-entry issue is a complete NMI/INTR/step/acknowledgement-failure
class: all relevant old-family callers must be reconciled together. Merely
adding an event cost while retaining an unnecessary interrupted fetch could
introduce both a phantom bus operation and a spurious entry failure.

Existing timing tests have81 textual timing/result/wait references across
V30 CPU units. This is an impact locator, not81 proven tests or exhaustive
coverage. Nominal tests must retain their independently derived nominal
oracle when full elapsed cost is introduced; expected values must not be
obtained by calling the production aggregate itself.

## Primary Source Inspection In This S

The pinned NEC1986 manual identity is retained from S46/S48; no new source
payload or implementation is imported. Text is read for complete printed
pages3-4,4-3,6-1,12-1 and12-171. At the first source checkpoint only printed6-1
was rendered and visually inspected; the remaining pages were inspected before
the Before-Code Decision below. One initial extraction selected the next
page for3-4/4-3/12-1; corrected one-based PDF pages17/22/46 give the intended
pages. This correction is not a source change.

The inspected interrupt table distinguishes V30 NMI38, INTR49 and single-step38
from software aggregates. Section12 describes nominal prefetched instruction
cost, not a complete serialized bus/CPU timeline. Prefetch overlap therefore
requires an explicit model grade; no internal cycle-exact schedule follows
solely from a four-clock bus cycle.

## Intended Design Constraints And Estimate

Preserve actual accepted-cycle publication as the fact owner, the CPU selector
as retirement-cost owner, the existing entry/failure allocation as event owner,
and Core as the additional-wait/guest-timeline owner. Reuse counters only with
their meaning documented and checked; do not route V30 through the SX FIFO.
No CPU provider gets called a second time to discover a cost.

Successful aggregate must distinguish nominal from fetch/model overhead in
the proof, without exposing another mutable clock or broad public layout.
Whether existing result fields express this without semantic ambiguity is
part of the before-code design, not a reason to silently overload event_ticks.
If a new public surface is truly required, follow the packet's review stop.
No model or accuracy downgrade is authorized by this record.

Initial estimates excluding manifests/documents/artifacts are production
+180--350/-60--150 and owned tests+350--650/-30--100. The81 impact locations
may make the test estimate too small; revise the packet with concrete file
scope and a cohesive design before material growth. No production/test edits
or new S49 compilation/test results exist at this inventory checkpoint.

Full repository unit qualification and eight receiving releases remain
mandatory for S49 acceptance. Do not substitute historical682/682 passes or
the primary-page renders for executed new qualification. Temporary output is
owned build/myraidenii-t2-s49 under Current's budgets and cleanup contract.

## Before-Code Decision

All five relevant primary pages are now rendered and visually inspected.
Implement an explicit serialized-fetch L2 aggregate at the existing timing
selector: successful nominal plus actual accepted instruction-fetch service,
with configured extra waits still owned separately by Core. This is an upgrade
from unallocated total time, not a replacement of a qualified physical model.
It preserves the one existing functional queue; it does not invent a background
prefetcher or claim NEC's unreported internal overlap schedule. Append the
approved necessary V30 elapsed-L2 timing-origin value; no public layout/function
is added. A zero finite maximum remains honest for unbounded prefix/wait inputs.

The existing COMMIT publisher records four clocks per accepted V30 bus cycle
in existing private transfer/fetch observations. A successful nominal already
contains operand cycles, so only fetch baseline enters its aggregate. A failed
instruction/entry uses the existing accepted-prefix allocation, not a complete
nominal floor. Successful async NMI/INTR/step uses its source entry aggregate
and actual stack alignment, separate from the already selected instruction.
Entry uses the existing boundary initializer without fetching an interrupted
instruction; all NMI/INTR/step/ack-failure callers change together for V30.

POLL's subsequent five-clock countdown belongs to idle time after its initial
retirement. Core therefore does not decrement that V30 countdown during pending
or immediate CPU-retirement publication; it uses existing retirement ownership,
not a new flag. The deadline query follows the same distinction. Intel WAIT
and ESC cadence is unchanged. V30 uses the existing pending-retirement path
for bounded completion, including zero configured waits, rather than adding
another completion owner.

The full aggregate occupies ticks; event_ticks retains its entry/failed-work
meaning and is not overloaded with successful fetch service. Existing nominal
test oracles remain independent source/L2 values, with the actual fetch term
checked separately. Impact estimate is refined before production edits:
CPU source+150--280/-25--80, Machine source+20--45/-5--15; owned tests
+650--1100/-120--250. All other source packages remain unchanged. The larger
test estimate accounts for strengthening the existing nominal units rather
than deleting their original assertions.

## First Full Qualification Checkpoint

Both final X86 Core builds and both complete repository unit-target builds
completed. The first x64 unit-target build had a transient output-file link
failure; the same-tree targeted relink and resumed build succeeded. An optional
owned CTest catalog process was stopped before relinking; no evidence proves
that it owned the failing file handle. No system setting or cache was changed.

The first full x86 unit run executed684 tests and failed two: Emulator's
verifier-negative timed out at60.11 seconds, and Machine's chip-boundary rejected
the new receiver fixture. An isolated unchanged Emulator rerun passed in56.36
seconds. This is not proof that the original parallel run passed or that its
timeout cause is established. Full x64 units remain in progress at this record.
Specialized x64 gates passed498 retained-strict rows,44 CPU compile commands,
536 unique declarations,150 deferred ownership rows and1401 Types inputs.

The Machine rejection is a real test-design problem, not a reason to weaken
the guard: machine_v30_time_smoke.c directly assigns the private profile and
binds a CPU outside configured Machine construction. Its comments explicitly
call it a partial time/bus receiver, but that does not meet the existing guard.
Do not disguise those operations with aggregate syntax, move the fixture out
of the scan, add a filename exception, or count it as a real Machine run.

The admitted ordering excludes V30 Machine admission from S49, while S50 opens
the genuine configured public route only after S49 qualification. Completing
the time/publication proof on the genuine route therefore requires an explicit
plan correction: bring the minimum genuine construction/receiver work forward,
with full runtime qualification still accounted for, rather than bypassing the
factory in tests. No such scope change or production admission is implemented
at this checkpoint. S49 remains open and uncommitted; no release is qualified.

## Approved Ordering Correction

On2026-10-19 the owner explicitly approves the proposed order correction.
Minimal genuine V30 Machine construction and public time/run receiver now
belong to S49; S50 retains complete Machine qualification. The existing boundary
guard remains unchanged. Replace the partial fixture rather than changing its
syntax or location to bypass rejection. V30 accepts only deterministic modeled
time and no integrated FPU; its L2 total cannot qualify a verified physical axis.
Before-code additional estimate: construction+5--10 lines, replace the roughly
150-line partial receiver and update the original rejection assertion. No new
public function, type, mutable state, execution loop or clock is planned.

The first full x64 run finished684 tests: Machine chip-boundary failed as above,
and Lib boundary/types selftests timed out. Those failures are not relabeled.
Both final specialized gate builds passed the same498/44/536/150/1401 counts.
After the approved correction, both widths pass the three genuine receiver,
profile-admission and unchanged chip-boundary tests. The replacement exercises
actual neutral_create, attachment binding, freeze/reset, debug register setup,
memory loading, run and public time/state/retirement observations. No CPU
profile rebinding or fabricated Machine exists in the replacement.

The genuine one-tick/wait2 route exposed an additional existing budget boundary:
the last accepted external-wait tick could exhaust the current budget, then
the pending-retirement branch attempted to publish zero CPU-execution ticks.
The existing publication interface correctly rejects zero. Guard the partial
publication with remaining budget; preserve the pending instruction for the
next run instead of adding a state, retry or permissive zero-time publication.
The failed attempt is retained separately. An initial reset assertion also
found the new cycle-failure fixture had omitted freeze; the test now follows
the real freeze/reset lifecycle rather than relaxing reset preconditions.

Final receiver checks include zero/two configured wait clocks per fetch cycle,
whole/one-tick/seven-then-rest budgets, source19 versus Core19+3*wait, first POLL
sample exactly once, later4+1 idle sampling, one NOP retirement, one HLT retirement
and subsequent no-retirement idle, exactly-once attachment destruction, cycle
COMMIT/CANCEL accepted waits, overflow and reset. Full frozen-unit and receiving
qualification remains outstanding at this checkpoint.

The second full x64 attempt found an additional old admission assertion in
neutral_link.c:80486SX+1 is now the admitted V30, not an unknown profile. The
negative vector now uses V30+1; all invalid-input checks and cleared-output
assertions remain. A complete same-issue query finds no other SX+1 invalid
profile assumption in X86 Core source/tests.

That attempt also found a boundary-selftest rerun problem independent of CPU
source. The first timeout left a44-byte audio/midi/probe.c negative fixture;
the next legal-owner check saw it and reported audio-midi -> audio-fm. Exact
path/content/size and absence of an owning process were verified, then only
that generated file was removed. This is not a Lib source change or proof
of a production outward dependency. The shared test cleanup issue has an
explicit MyRaidenII TODO receiver outside this CPU S; the final qualification
must still execute the unchanged tests, not omit or weaken them.

The second x64 attempt also observed a transient test-manifest mismatch while
the executor was applying the neutral_link correction and refreshing its hash.
That attempt is explicitly not frozen qualification. The subsequent source/test
freeze is FECA5A9FC08D6F9B454C31C6F8B3A5BE272BDAB5F33AB952AFB16F2DAD1C7AC9
and1DD7EC91F3E5C89FC3AA71CD384A7D44A857960518A3581E43BCBC93E591A7CF
for the two manifests. Do not edit manifest-covered files during the final
qualification. This avoids generating a second avoidable mixed-input result.
Final full suites use four concurrent tests, without changing cases, assertions
or timeout budgets; compilation remains Ninja-j8. This limits subprocess
contention from the large negative-verifier scripts rather than weakening them.

Both frozen full suites passed684/684, without failures or skips (x64 real
133.86 seconds, x86 real161.44 seconds). After those runs, actual-change review
corrected ten continuation-line indents in six adapted tests; no token, line
count, assertion, expected value or production input changed. The final test
manifest is86F98127F31A9447BCE2CA5676312A129B3624B9A9E51B742AEB0396DE05D8E0;
the production manifest remains FECA5A9FC08D6F9B454C31C6F8B3A5BE272BDAB5F33AB952AFB16F2DAD1C7AC9.
Twelve corresponding pre-cleanup object hashes are retained for comparison
after the final dual-width rebuild and actual V30 test replacement runs.
Do not describe those unexecuted replacements or receiving releases as passed.

## Final Implementation And Qualification

The production selector now publishes nominal plus accepted serialized fetch
in ticks, with the explicit appended V30_ELAPSED_L2 origin when fetch contributes.
Operand nominal work is not charged twice; Core alone adds accepted configured
waits. Entry/failed-work remains event_ticks. No background prefetch schedule,
physical time axis, finite maximum over unrestricted prefixes/waits, second
decoder, executor, clock, provider or compatibility path is invented.

| Accounting class | Final evidence and retained boundary |
| --- | --- |
| E1/E2 | cpu_v30_elapsed independently counts accepted provider cycles over cold/resident/odd/page/offset-wrap streams and15/30 prefixes. Existing nominal units retain their original source/L2 rows. |
| E3 | Static inspection of cpu.c retains real reservation fetch only for8088. V30's existing functional queue/serialized L2 does not claim background electrical overlap or invent work for metadata-only reservation. |
| F1/F2 | Actual CPU COMMIT/CANCEL and refusal proof retains accepted waits; genuine Machine run proves Core19+3*wait versus copied CPU19 under whole, one-tick and seven-then-rest budgets. Own Machine cycle seam proves cancellation, overflow and reset. The exhausted-budget zero-publication defect is fixed at the one pending owner. |
| G | Existing native/NEC/8080 nominal oracles remain independent, including operand alignment and the full emulation table. Only separately checked actual fetch enters the aggregate; CPU does not add Core-returned waits again. |
| H1/H2/H3 | Software combined rows retain their ownership. NMI/INTR/step uses existing entry allocation; accepted failed entry/fetch/data prefixes are charged once without a successful floor. Every entry failure position and both stack parities retain proof; async entry does not fetch the interrupted next instruction. |
| J1/J2 | Genuine factory/run/attachment proof covers first POLL retirement, later4+1 idle samples, source versus Core cost, one HLT retirement and no repeat retirement during idle. Bare CPU refresh/reset proof remains separate from elapsed Core time. |
| K1/K2 | Preview failure/success preserves copied CPU/instruction/context and performs no real transfers, waits or TEST samples. Debug reads do not change accounting. Maximum0 remains honest; actual pending completion follows bounded Core budgets. No new elapsed-preview API or guessed one-tick fallback is introduced. |
| Minimum V16 receiver | Owner-approved real deterministic V30 construction replaces the rejected partial fixture. FPU and verified-physical V30 configurations fail before publication. Complete native/emulation/entry/deadline/non-PC board qualification remains S50. |

Both final30-case V30/test-boundary replacement runs pass after the indentation
cleanup:16.22 seconds x64 and33.06 seconds x86. All twelve rebuilt object hashes
are byte-identical to the objects used in the full684-name runs. No production
input changed between full qualification, receiving publication and this final
test-only cleanup. The [results](t2-s49-results.json) preserve every attempted
JUnit hash/count, including failures; they distinguish full suites from those
final replacements rather than relabeling a failed run or adding subset counts.

Both widths pass the final498 retained-strict rows,44 target-local CPU compile
commands,536 unique declarations,150 deferred-ownership rows and1401 Types inputs.
Both manifests, dependency and unchanged chip-boundary guards pass. Legacy
instruction-file warning exceptions remain; no all-source waiver-free claim
is made. Documentation structural checks are separate from semantic review.

Eight stripped0564 PC EXEs are rebuilt/deployed, with14 selected receiving
integration cases per width. DeskPro boot completes in130.70 seconds x64 and
171.44 seconds x86 within the unchanged300-second budget. PE widths, built versus
deployed SHA-256 and absence of compiler debug sections are checked. Both graphs
finish at default; each of eight profile switches recompiles zero CPU objects.
[Ten release identities](t2-s49-artifacts.json) distinguish eight rebuilt PC
EXEs from the two unchanged MyNES0044 EXEs. [Protected inputs](t2-s49-media-inputs.json)
retain all33 tracked asset and six selected external media identities unchanged;
media modes remain overlay. This is not an entire external archive audit.

Actual code delta uses git diff --numstat plus exact newline counts for the three
new C/H files, excluding docs/generated/artifact data: production six existing
files+129/-39/net90; owned C/H/CMake tests27 files+623/-117/net506; two manifests
+33/-30/net3. CPU accounts for+105/-30, Machine+24/-9. The positive test delta
provides the independent cycle/failure/preview/budget proof above. Production
adds no helper, public function, layout, persistent state, thread or clock;
only the approved timing-origin value is appended. The cohesive test helper
transports independent nominal oracles rather than calling the production
timing evaluator. Original guest/register/memory/transfer assertions remain.

The obsolete Init-side BUSLOCK release is moved to the sole boundary owner,
not retained as another path. The partial receiver/profile rebinding is deleted
and replaced by real construction/run. Distinct retained Intel/SX paths own
their source clocks, write-buffer and physical-wait semantics, not V30 fallbacks.
The similar-issue sweep retains those differences with reasons and records
the interrupted shared selftest cleanup issue in TODO rather than changing Lib.

No full external suite, manual desktop/RDP, physical audio, silicon-exact timing,
complete S50 Machine qualification or playable game is claimed. S49 qualification
does not close T2 or M1. Commit identities and coordinator acceptance follow
only after actual-change review and the complete scoped delivery.

Shared P1 `a07c371073ab70061d11e6f2dc0d8b147e3edbe4` and NXVM receiving
P2 `98656815cf7f369e2dc04cbf5b5d4e675c997e6e` are pushed. Necessary result/hash
summaries are retained in the linked evidence. All owned process handles are
terminal and process inspection finds no owned task process. The actual cleanup
tool first previews, then removes only build/myraidenii-t2-s49:105 generated
files/3398492 bytes. The separately verified44-byte interrupted-test probe had
already been removed. Final build contains only .ccache/.ninja; reusable cache,
global cache, configuration, external research, firmware and media are retained.

## Coordinator Acceptance

Sequential coordinator review accepts pushed P1 `a07c37107`, P2 `98656815c` and
P3 `cd8680dff` after reading their actual source/test/build/doc/artifact changes.
The final manifest, model/ABI/boundary and retained/removed-path facts above
match the committed implementation. Ten artifact identities,33 tracked guards
and six media hashes/modes are rechecked from current files. Cleanup and
process completion are verified, with only persistent caches remaining.
Original request, class ledger, scope correction and remaining S50/S51 exits
are reconciled; structural doc checks are not substituted for that review.
This accepts S49 only, not complete Machine, T2 or playable M1.

## Archived S49 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: MyRaidenII M1 T2 S49, the next unused numeric S after accepted S48. Single-session sequential coordinator/executor roles; no delegated agents. |
| Admission And Approval | Owner-approved same-T S47--S51 plan and standing automatic S admission. Justified timing upgrades are automatically approved, not accuracy downgrades. On2026-10-19 the owner explicitly approves bringing minimal genuine V30 Machine construction and time-receiver execution forward from S50 to S49; S50 retains complete Machine qualification. Targets Shared (X86 Core CPU/Machine and owned tests), NXVM (eight affected current0564 PC artifacts and receiving receipt), MyRaidenII (task records). Lib, Emulator, MyNES and unrelated product changes are excluded. |
| Objective | Consume complete S46 accounting E/F/G and elapsed H/J/K plus original V14/V15: exactly-once fetch/prefetch baseline, accepted additional waits, operand alignment, failed accepted prefixes, asynchronous/software entry separation, POLL/HLT/idle, pure preview and honest execution bounds. Freeze the complete class inventory before production edits. |
| Non-goals | No game/board/COP/video/audio implementation, complete Machine qualification (S50), final T2 qualification (S51), second executor/clock/wait accumulator, nominal operand baseline duplication, fabricated finite bound over external waits, third-party source import or protected asset modification. Minimal configured V30 deterministic construction is now included solely with real time/run proof, never a private profile bypass. |
| Reference Baseline | Clean pushed a1f9829f9, S48 Shared1ea7733d3 and receiving3aa8dbb81; actual current CPU/Core callers; S46 accounting matrix and S48 nominal evidence; pinned NEC1986/2000 primary manuals and accepted explicit8080 compatibility model. Historical results are not new S49 qualification. |
| Candidate Proposal | ../proposals/m1-v30.md#proposed-s49-complete-elapsed-time-ownership-and-bounds and ../etc/t2-v30-ledger.md. Retain every E/F/G/H/J/K class and original receiving predicate; S50/S51 are not substitutes for this S's complete time proof. |
| Files And ABI Surface | Existing src/x86-core/cpu timing/instructions/bus/context and src/x86-core/machine construction/time/bus/scheduler owners; matching test/x86-core and manifests. Before-code refined estimate CPU source+150--280/-25--80, Machine source+20--45/-5--15, owned tests+650--1100/-120--250, excluding docs/artifacts/manifests. Approved ordering correction adds approximately5--10 construction lines, replaces the approximately150-line partial receiver with genuine public construction/run proof and updates the old V30 rejection assertion. Preserve independent nominal oracles. Append only the already approved necessary V30 elapsed-L2 timing-origin value; no public function, layout or clock. Other public expansion stops for review. |
| Applicable Rules | Shared Execution sixteen-field packet, full mechanism ledger, complete units, one target per pushed P, actual-change review and receiving release; Architecture sole CPU decode/bus and Core timeline/wait owners; Coding Lib vocabulary, original table style, cohesive helpers and no duplicate paths; primary-source grades and no imported implementation; Document authority/current/history separation. Architecture/coding governance skills apply, read-only PDF skill before visual source inspection. |
| Verification | Direct ROM-free actual CPU callback tests for native/NEC/8080, empty/resident/redirected/wrapped fetch, odd/even/split memory and ports, COMMIT/CANCEL/overflow/reset, successful/failed/software/async entry, repeat first/continuation, POLL initial/later samples and HALT idle. Preview must be side-effect-free and bounds explicit. Both stable Ninja/ccache graphs build and run complete repository-only units, strict/Types/dependency/manifests and affected receiving regressions. Rebuild/deploy eight0564 PC EXEs; verify PE/hash/identity, unchanged MyNES/INI/media. |
| Expected Markers | Existing CPU/Core markers remain; class-specific exact clocks, callbacks and failure publication assertions must distinguish accepted costs from nominal-only values. Full unit run has no failures/skips both widths. No silicon-exact, real Machine, desktop/RDP or gameplay claim from modeled timing. |
| Asset Needs | ROM-free synthetic inputs; pinned research outside repository. Receiving firmware/media use existing admitted read-only/overlay routes only. Owned disposable build/myraidenii-t2-s49 for probes/logs/renders, executor ownership, two-hour phase budget, five-minute no-progress budget,128MiB output cap; preserve results/hashes then safely clean via existing Clear-Build tool after owned processes exit. Only .ccache/.ninja remain reusable. |
| Reporting Requirements | Report actual source-owner findings, complete accounting design and per-component estimates before production edits; progress at inventory/design, full-unit and receiving-publication boundaries. Delivery records source/test additions/deletions/net, retained and removed paths, helpers/state/interfaces, actual qualification, model grades, source/artifact hashes and unverified boundaries. |
| Stop Conditions | Stop affected work before Lib/Emulator changes, unapproved public layout/function changes, accuracy downgrade, unsupported source/physical-bound claim, source/license conflict or a new independent time owner. Revise material scope/acceptance before implementation; continue safe independent investigation. |
| Exit Criteria | Every E/F/G/H/J/K input/outcome has exactly one accountable owner and unit with actual dual-width proof, total-time disposition is complete with honest source/model grading, and preview/bounds/cancel/overflow/reset agree. Full units/gates and eight affected receiving releases pass; actual diff/ledger review, scoped commits/push and safe cleanup precede coordinator acceptance. Complete V30 Machine qualification remains S50 and T2 remains open. |
| Original Owner Request | Implement MyRaidenII fully in single-session dual-role governance, retaining all unfinished NEC CPU work in this T2, with automatic justified precision upgrades and no unapproved downgrade. |
| Similar-Issue Sweep | Query all src/test x86-core CPU and Machine fetch, external-cycle phases, transfer/wait/event counters, prefix/repeat ownership, entry/failed-prefix publication, POLL/HLT/idle, timing selection/preview/max and scheduler waits. Each live path is fixed, justified distinct or explicitly retained with its same-T receiver; Intel/SX differences require semantic reasons, not blanket V30 substitutions. |
