# T2 S38 Ordinary Software And Mode Control Nominal Timing

Baseline `72f17bd18`, after accepted S37. This consumes the complete
eight-encoding ordinary software/mode-control nominal batch at original
V08/V10/V13/V15. It is not full asynchronous entry, elapsed time, Machine,
board or gameplay qualification.

## Primary Source And Model Decisions

Pinned NEC October 1986 V20/V30 User's Manual SHA-256
`FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C`.
[Original copy](https://www.dosdays.co.uk/media/nec/NEC-V20-V30-Users-Manual.pdf).
Printed 6-1,12-167..170,176..177 (PDF26,212..215,221..222), all seven pages,
were rendered and visually inspected. Only original V30 70116 rows are used;
the source's V20 columns, instruction Bytes typos and physical queue/wait
assumptions are not silently substituted.

| Completed encoding / outcome | Even native stack | Odd native stack | Disposition |
| --- | --- | --- | --- |
| BRK3 / BRK immediate | 38 | 50 | Original source aggregate |
| BRKV taken | 40 | 52 | Source 6-1 and 12-168 reconciled |
| BRKV untaken | 3 | 3 | Explicit reference-backed L2; no stack transfer |
| RETI | 27 | 39 | Original source aggregate |
| BRKEM | 38 | 50 | Original source aggregate |
| CALLN | 38 | 58 | Its own printed odd row, not a BRKEM-derived slope |
| RETEM | 27 | 39 | Original source aggregate |

CHKIND's untaken source rows are 18 for even operand and 26 for odd operand.
Taken rows are ranges 53..56 all-even and 73..76 all-odd. The conservative
L2 control model uses 56 for all-even and 76 if operand or stack is odd,
including mixed alignment for which no exact source row is supplied.
All CHKIND costs use L2_CONTROL_MODEL: the existing signed-comparator
semantic model and the range/mixed-alignment choice are not upgraded to
silicon facts. The actual successful software-entry fact owns taken versus
untaken; timing never rereads the bounds or reconstructs a second comparator.

The BRKV page labels the even 40-clock line as V=0, directly below the
V=1 odd-address rows; section6 explicitly lists 40 for the V30 software
entry. Thus taken even40/odd52 is supported by the combined originals.
The missing untaken row is a distinct L2 model, not a transcription of that
misplaced label. Read-only MAME NEC research at already pinned tree
`c1425a1af00b88aa07477b1b23a30ed5e5f856ff` observes the independent untaken
constant three in
[necinstr.hxx](https://github.com/mamedev/mame/blob/c1425a1af00b88aa07477b1b23a30ed5e5f856ff/src/devices/cpu/nec/necinstr.hxx).
The fetched UTF-8 source SHA-256 is
`317B56C9E7E1B1F092D54112DAE2BC721B6DECC55CA981E7643D7C458CC96BEE`;
it declares BSD-3-Clause, Bryan McPhail. The file was read/hashed in memory;
no implementation, table, source text or payload is saved/imported/compiled.
Only that numeric modeling fact is consumed. The taken MAME constants are
not substituted for primary even/odd NEC rows. Source versus L2 origins
remain explicit in every execution result.

These are combined successful instruction subtotals in ticks, charged once,
not a separate normal-instruction charge plus another frame/event charge.
event_ticks remains zero in this nominal batch. Asynchronous NMI/INT/step,
combined DE, real service/wait/prefetch and complete prefix costs retain
V11/V14/V15. Total time stays unallocated, maximum zero, Machine gated.

## Unique Ownership And Actual Proof

Only the existing V30 timing selector changes. It consumes original captured
opcode/PSW/SP/EA and the already existing successful-entry fact. The same
publication/key/stack-penalty owner handles native and explicit mode calls;
original PSW distinguishes native from 8080, not a new persistent mode flag.
Ordinary 8080 remains unknown except the two actual mode escape encodings.
No executor, instruction handler, entry/return/priority semantics, clock,
state, production helper, public contract/enum/layout, Lib/Emulator or Intel
evaluator changes. Mode keys distinguish ED ED/ED FD from native 0F and
string-prefix identities. Extra native prefixes remain zero/unattributed.

Three existing actual execution owners gain independent source/outcome
oracles. Their common CPU-local memory fixture has one nominal-result
comparison checking clocks/key/inputs/origin/repeat-none/no-event/no-observe,
unallocated total and zero maximum, not another decoder or execution model.
The local oracles retain their independent expected CPU/frame outcomes.
All original 655,265 software-entry, 328,192 mode-transition and 3,457
software-arbitration inputs remain. Every selector/PSW and original
code/stack/operand/vector alias, parity/wrap, preview/refusal, real return,
mode latch and asynchronous-pending admission assertion remains.
Failed executed transfers additionally cannot publish a successful timing
origin or clocks. The existing fixture assertion checks no extra observation.

Git numstat for one production C plus three test C and one existing fixture,
excluding manifests/docs/assets: production +43/-4/net +39;
tests +114/-2/net +112; total code +157/-6/net +151.
Manifests separately +5/-5/net0. The test increase supplies independent
source/outcome expectations in three real seams and one shared comparison,
not duplicate execution, new test targets or a framework. Existing
non-control arithmetic/string oracles retain their own family/phase
semantics; no production path is duplicated.

## Actual Initial Failure And Correction

The first strict build succeeded. The first x64 seven-unit focus passed five
but failed the CALLN assertions in mode and software-arbitration:
clocks=0/58 and 0/38, key=ffffffff/ed00ed. The runner is terminal exit one;
it never started x86 focus, full units or receiving publication.
Original logs/XML are retained as
`build/myraidenii/t2-s38-initial-focused-x64.{log,xml}` and the matching
initial-focused-build log. A direct failing probe additionally printed
`bytes=6 opcode=ed/ed/00 oldflags=2`.

Cause: captured oplen is an available-byte extent and can include prefetch
lookahead beyond the completed three-byte CALLN. The first candidate
incorrectly required equality with the encoded length, despite correct
mode execution and captured bytes. Final selection requires only the
appropriate minimum two/three bytes. It performs no additional fetch,
observation, cache invalidation, result mirror or instruction metadata edit.
All existing warm/cold, wrap and alias cases now exercise that correct
capture contract; both final focused groups pass.
The packet records this existing capture meaning before final qualification.
No assertion or time budget is weakened.

Prevention/sweep: distinguish available capture extent from the completed
encoding before selecting costs; use the sole executed identity and actual
outcome rather than assuming a capture ends at the next PC.

## Similar-Issue Sweep

Queries include:

- `rg -n '_ser_int_real|_e_int3|_e_int_n|_e_into|NEC_EMULATION' src/x86-core/cpu/cpu_instructions.c`
- `rg -n 'instruction_software_entry|nec_md_write_enabled' src/x86-core/cpu/cpu_timing.c`
- `rg -n '(oplen\\s*(==|!=|<=|>=|<|>)|byte_count\\s*==)' src/x86-core/cpu/cpu_timing.c src/x86-core/cpu/cpu_timing_model.c`
- `rg -n 'event_ticks|capture_timing' test/x86-core/cpu --glob 'cpu_v30*' --glob '*v30*.h'`

All eight forms use existing frame/return execution; only one V30 selector
allocates their subtotals. CALLN keeps its own source row; the two conditional
outcomes use actual completion facts. The sole capture-equality defect in
these timing consumers is removed; all remaining extent guards are minimum
bounds. Existing asynchronous/DE event receivers are not silently assigned
ordinary software costs. The source/timing public interface, Intel evaluators,
reset/repeat ownership and neutral Machine validation remain unchanged.
The original seventeen ledger rows and all broader model/form/bus/board/game
requirements are retained.

## Final Qualification

Both final original cached Ninja all-target builds and complete repository
units pass 678/678 per width, including 191 X86 Core tests, zero failures or
skips. Full units take 298.73/121.40 seconds; entry seams take 7.88014/10.3182
and mode seams 0.951777/1.14924 seconds. Both seven-unit focused groups,
498 retained strict rows and 1389-input Types coverage pass. Historical
warning exceptions remain, not exception-free strict compilation. Original
arithmetic/string/Intel tests and all earlier V30 evidence counts remain.

Eight current PC0564 EXEs are rebuilt/deployed. Fourteen selected receiving
integrations plus one restored-default identity pass per width; DeskPro
groups take 127.93/163.20 seconds. Both original graphs end at default;
switching/restoration recompiles zero CPU objects. All ten PE/hash records
pass and contain no compiler debug sections. Both actual caches retain
Release and the persistent ccache launcher. MyNES0044, INIs and protected
firmware/media/handler/interface/model inputs are unchanged. The sole final
runner is terminal exit zero. No final build/dynamic test failed, and no
budget or assertion was weakened. The initial failing focus remains above.

[Results](t2-s38-results.json) preserve exact case names/counts/logs;
[artifacts](t2-s38-artifacts.json) preserve all ten deployed identities.
The frozen collector checks actual inputs and proof, not just exit codes.
Its temporary report-variable name was corrected so the retained ordinary
control test duration is read from its own XML case, not a subsequent loop.
This changed neither qualified production/test inputs nor test results.
Shared P1 `ddff07dbd` and NXVM receiving P2 `509c10d81` are pushed;
coordinator actual committed-diff acceptance follows the records delivery.

No fresh complete external suite, manual desktop/RDP/audio, silicon elapsed
time, Machine/board or gameplay qualification is claimed. No runnable
MyRaidenII EXE yet. Original V01--V17, T2/M1 and the full goal remain open.

## Complete Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, MyRaidenII M1 T2 S38, next unused S after accepted S37. |
| Admission And Approval | Owner's standing automatic single-person coordinator/executor MyRaidenII implementation and timing-upgrade approval; coordinator admits original V08/V10/V13/V15 ordinary software/mode control nominal batch on 2026-10-18 at clean 72f17bd18. Targets Shared CPU/tests, NXVM eight receiving artifacts/status and MyRaidenII records. No Lib/Emulator/MyNES or public contract change. |
| Objective | Allocate all eight BRK3/BRK/BRKV/CHKIND/RETI/BRKEM/CALLN/RETEM encodings and conditional outcomes through the existing V30 selector, with native-stack parity, copied original PSW/mode and actual successful software-entry facts. Preserve source/model grades, complete result/frame/vector/return/priority/refusal proof and original unallocated total-time receiver. |
| Non-goals | No guest arithmetic/flags/entry/return/priority changes, new native/emulation executor, operand observation, persistent state, public API/enum/layout or Machine/board/game admission. No asynchronous NMI/INT/step event-time, DE aggregate, physical bus/prefetch/wait or full prefix accounting; these remain original V11/V14/V15. No copying external implementation. |
| Reference Baseline | 72f17bd18; pinned NEC October 1986 manual SHA FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C, printed 6-1,12-167..170,176..177 (PDF26,212..215,221..222), seven pages rendered/visually inspected. Existing S22/S23/S28 real entry/mode/priority proofs. MAME necinstr.hxx at c1425a1af00b88aa07477b1b23a30ed5e5f856ff, SHA317B56C9E7E1B1F092D54112DAE2BC721B6DECC55CA981E7643D7C458CC96BEE, BSD-3-Clause/Bryan McPhail, read-only single untaken BRKV constant; no code/table import. |
| Candidate Proposal | proposals/m1-v30.md and original V08/V10/V11/V13/V15 ledger. BRK3/BRK/BRKEM=38 even,50 odd native SP; RETI/RETEM=27/39; CALLN=38/58, retaining its distinct printed odd row. Taken BRKV=40/52 reconciles section6 with12-168; untaken uses explicit reference-backed L2_CONTROL_MODEL=3 because the primary row is mislabelled. CHKIND uses actual entry fact, not reread operands: untaken18/26 by original operand parity; taken conservative L2 upper56 for all-even,76 if operand or stack odd, explicitly including mixed parity and retained signed-comparator model. No invented per-transfer slope. Captured opcode extent may include prefetch lookahead: require the mode encoding's minimum bytes, never equality with a presumed instruction length. |
| Files And ABI Surface | src/x86-core/cpu/cpu_timing.c and source manifest; existing software-entry/mode-transition/software-arbitration smoke C files, existing CPU-local memory fixture for one shared nominal-result assertion and test manifest. No production headers/registration/public contracts or instruction handlers change. Estimate production +40..70/-3..10; tests +90..140/-0..10; one test-only assertion owner reused by three real execution consumers. |
| Applicable Rules | MyRaidenII guide/goal/proposal/ledger/queue/roadmap/architecture/coding/source policy; shared Execution/Architecture/Coding/Documentation, architecture/coding governance and read-only PDF skills; receiving NXVM guide/current/architecture/coding/source policy and X86 Core README. One state/entry/time/publication owner, Lib Types, honest source/L2 attribution, frozen proof, target-separated Ps and actual committed-diff acceptance. |
| Verification | Extend existing actual software/mode/priority execution seams with independent source-row/key/input/origin/no-observation checks. Preserve 655265 software-entry,328192 mode-transition and3457 arbitration inputs, all selectors/PSWs, code/stack/operand/vector aliases/wrap, both parities, real transitions/returns/pending events, preview and each applicable provider refusal. Transient focus: software-entry, mode-transition, software-arbitration, nominal-transfer, interrupt, completion and form-admission. Both original Ninja/ccache all-target builds -j8, full678 repository units -j4, specialized498 strict rows/Types,14 selected PC receiving integrations plus restored identity per width, ten PE/hash records and protected-input/docs checks. |
| Expected Markers | Original V30:SOFTWARE-ENTRY:OK cases=655265, V30:MODE-TRANSITION:OK cases=328192 and V30:SOFTWARE-ARBITRATION:OK cases=3457 retained; add V30:NOMINAL-SOFTWARE-CONTROL:OK at these three owners. All final suites zero failures/skips; source/L2 distinctions, zero event_ticks for this nominal batch, total-time unallocated/max0/closed V30 Machine admission preserved. |
| Asset Needs | Synthetic ROM-free inputs, pinned manuals and original reusable cached graphs only. Rebuild/deploy eight PC0564 releases, verify all ten identities and unchanged MyNES0044, INIs, firmware/media/snapshots. No new ROM, MyRaidenII EXE, external implementation import or required native desktop input. |
| Reporting Requirements | Before implementation report source/model/owner decisions, exact files and estimated diff; retain actual failures and correction evidence. Report production/test add/delete/net, unique paths/test helper purpose, complete batch dispositions, final qualification/artifacts/commits and original whole-game boundary. |
| Stop Conditions | Public/source/license authority expansion, guest/Intel semantics changed, source assumption represented as silicon fact, extra timing observation or state/execution path, protected-input mutation or failed qualification prevents acceptance. No weakened assertions/budgets or eligibility promotion. |
| Exit Criteria | Every admitted encoding/outcome has a source or explicit L2 nominal disposition at the existing selector, proved at actual entry/return/conditional/priority/failure seams. Full dual-width builds/units/gates/selected receiving integrations/artifacts/docs pass; complete target-separated Ps pushed, coordinator actual committed-diff review accepts and closes only S38. |
| Original Owner Request | Single-person two-role governed implementation of complete playable MyRaidenII, NEC V30 and needed facilities. Nominal control timing is not full physical CPU timing, Machine/board/video/sound or complete gameplay. |
| Similar-Issue Sweep | All eight opcode/mode identities and shared real entry/return callers, parity/source aliases, original versus mutated PSW/SP/operand facts, taken/untaken outcomes, publication after failure, pending asynchronous priority, extra prefix/8080 rejection, source/model origins/key collisions and max/Machine/Intel guards. Separate asynchronous event and DE totals retain original V11/V14/V15; no duplicate entry or timing state. |
