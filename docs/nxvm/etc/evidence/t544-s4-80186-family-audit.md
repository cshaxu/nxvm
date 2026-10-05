# T544 S4 80186 Family Audit

## Status And Source

Read-only audit delivery against 946f7a737; no Shared repair or CPU
qualification is claimed. Coordinator acceptance remains separate. This batch covers
F11 additions and inherited F01-F10/F14, not just opcodes first seen in 186.
The S2/S3 mechanism receivers remain pending inside T544.

Primary source: Intel iAPX 86/88/186/188 User's Manual (1985), order
210912-001, owner archive intel-8086-8088-80186-80188-users-manual-1985.pdf.
Fresh SHA-256 is
2516D66CC75076D9AC9EE048E8420C09C35655FB25ED34DDA6351A3EA4E0AFFF.
Original provenance is in the
[T512 source record](t512-s1-five-cpu-source-cross-validation.md).
PDF 171-172 (printed 2-5 through 2-6) and 178-179 (printed 2-12 through
2-13) were rendered and visually inspected. Extracted text is navigation
only. Originals stay external; renders remain in ignored research scratch.

## Finite Batch And Dispositions

| Batch | Required context | Current disposition / receiver |
| --- | --- | --- |
| PUSHA/POPA | Original SP, register order, skipped SP, FLAGS, stack wrap and selective failed transfer | Normal word handler/source ordering inspected below; complete boundary and regression proof pending. |
| PUSH immediate | Byte sign extension, word immediate, fetch/write failure and stack wrap | Checked decode/push, ordinary sign-extension regressions and base clocks reconciled; endpoint/failure/wrap proof remains in the decode/stack receiver. |
| IMUL immediate | Byte/word immediate, signed r/m source, overflow CF/OF, undefined flags, failure | Word arithmetic, midpoint rows and normal 186 register/memory regressions reconciled below; extreme/failure/segment proof pending. Later dword signed-product receiver is retained for S6. |
| Immediate/CL shifts | All seven operations, count zero/31/32/255, byte/word register/memory | Mask, checked decode/publication, selected clocks and regression limits inspected below; extreme counts, FLAGS and provider-effects receiver retained. |
| BOUND | Signed inclusive limits, both adjacent reads, type-5 frame, source segment and wrap/failure | Signed compare, checked reads and existing 186 upper-bound vector/frame regression reconciled below; source-qualified return location and complete lower/prefix/wrap/failure matrix pending. |
| ENTER/LEAVE | Levels 0/1/2/31/32/255, local size, frame copy, stack wrap and failed access | Full-byte level, frame algorithm and base formula reconciled; existing level-255 proof is selected image/state, not every copied slot or provider failure. |
| INS/OUTS | Byte/word, DX stable, ES destination/overridable DS source, DF, REP zero/termination and provider effects | Checked helper order and normal tests reconciled; bus effects and failed-element contexts remain in the port/string receiver. |
| Inherited F01-F03 | Adjustments, ALU and movement forms, encodings, flags, address width and failure | Normal clocks/forms reconciled; carry S3 source/decode/arithmetic/XLAT receivers and the paired-width/directional timing findings below. |
| Inherited F04/F05/F10 | Stack/FLAGS, branches, return IP, inhibition and interrupt arbitration | 186 source differences and ordinary tests inspected; carry S2 state/delivery and S3 branch/ordered-write receivers. |
| Inherited F06/F07 | Multiply/divide ranges, signed boundaries, count/host arithmetic | Range choices and exact DIV rows reconciled; signed quotient authority, host arithmetic and boundary proof remain pending. |
| Inherited F08 | Five memory strings, prefixes, interrupted repeats and completed-element state | Carry S3 propagation/restart/retirement receivers; do not impose early prefix-loss semantics without 186 source. |
| Inherited F09/F14 | Ports, WAIT/HLT/NOP/ESC, unused opcode and configurable escape interrupt | Ordinary clocks and actual handlers inspected; external-cycle, unused delivery and missing integrated-control input receivers remain. |
| Timing qualification | Every selected row, width/range/formula and original prefetch/no-wait/even-word conditions | Tables/selectors/oracles reconciled with numeric, range and conflicting-source distinctions; residual contexts and regression owners are mapped in the final disposition table. |

None of these rows is accepted as whole-instruction qualification. Later-only
F12/F13 and dword forms cannot become 186 capabilities merely because a shared
handler supports them; source-defined absence and admission proof are still
required. The public implementation remains 80186, not an invented 80188 alias.

## First Source / Handler Reconciliation

Printed 2-5 specifies PUSHA order AX, CX, DX, BX, original SP, BP, SI, DI;
POPA ignores the popped SP value. PUSHA captures SP before its first checked
push and uses that snapshot for slot five. POPA's reverse word sequence reads
the SP slot into a temporary rather than assigning it. No handler changes
FLAGS. The inspected normal sequence agrees with this source. It does not
prove selective provider failure, partial stack writes or every wrap boundary.
Both helpers continue to use the one CPU push/pop owner, not a second stack.

Printed 2-5 defines PUSH immediate byte sign extension to a word. PUSH_I8
checks its byte decode, converts through lib_i8 and then invokes the checked
width-selected push; PUSH_I32 checks its selected-width immediate. The 186
decoder admits word execution, not later operand-size prefixes. Both immediate
IMUL handlers likewise check ModR/M, r/m read, immediate and final register
publication. The arithmetic helper and complete CF/OF matrix remain to inspect.

The same page explicitly limits every 186 multiple shift to count modulo 32,
including immediate forms. This differs from the early full-CL rule. S3's
inspected common count helper already selects full CL only for 8086/8088 and
five-bit counts for later families. No count-mask repair is justified from
the early source. All immediate/CL and through-carry contexts still need proof.

Printed 2-6 defines signed inclusive BOUND limits in adjacent memory words.
BOUND_R16_M16_16 reads both words with checked calls and compares signed
lib_i16 values, raising BR only below/above, not at an endpoint. ExecFinal
contains an actual BR-to-vector-5 delivery path. This disproves a claim that
BOUND only sets a bit with no delivery support, but does not establish its
186 saved-IP frame or provider-failure behavior. The current test's default
word success includes 186; inspected endpoint and segment examples use 386,
so those cannot qualify the 186 boundary by association.

Printed 2-6's ENTER pseudocode saves BP, copies prior frame entries, sets BP
to the saved post-push SP and allocates locals. Its unsigned level may reach
255. ENTER retains the byte on 186 and reduces it modulo 32 only from 286.
The existing enter_leave_test_defaults includes 186 levels 0, 1, 3, 33 and
255, with later 33 becoming one. This is useful existing generation coverage,
not a missing implementation to replace. Full source size limits, stack-wrap,
provider rejection, asynchronous boundaries and timing totals remain pending.

## Original Timing Conditions And Initial Rows

Printed 2-12 says Table 2-9 gives minimum execution clocks with needed bytes
prefetched, no waits/HOLD and even-addressed word data. Jumps/calls include
destination opcode fetching; memory references can require one or sometimes
two more clocks due to BIU/EU handshake. Preserve these conditions rather than
promising unconditional cycle-exact board time from a table scalar.

Rendered printed 2-13 gives PUSHA 36, POPA 51, PUSH immediate 10 and ordinary
word-register PUSH/POP 10. The existing 186 timing ledger matches these exact
rows. These values are Manual-L3 under their source conditions, not new L2
merely because bus conditions remain unproved. The range-based Group 3 and
immediate IMUL owners still require the corresponding source rows: their
midpoints are model choices, not exact manual formulas. No tier change is
made in this first source check.

## Additional Interrupt Source Lead

Printed 2-6 explicitly lists 0F, 63-67, F1 and FFFF as unused encodings that
generate type 6. It also describes programmable type-7 handling for D8-DF
through a relocation-register bit, disabled at reset. These are 186 source
facts, not proof that generic 386 TS/EM behavior or early undefined encoding
policy is sufficient. The implemented metadata blocks selected later forms
on 186, but the complete negative encoding/delivery and configurable ESC
input paths remain to inspect. Integrated peripheral implementation is not
silently added to this CPU-only audit; any missing control boundary needs an
explicit owned receiver before a complete 186 claim.

## Segment MOV Timing Direction And Source Conflict Sweep

Further visual review of PDF 49 / printed 1-33 corroborates the segment MOV
directions in Table 2-9: memory-to-segment is nine 186 clocks and segment-to-
memory is eleven, before applicable modifiers. The actual 80186 selector's
8C branch marks a segment store but selects nine, while its 8E branch marks
a segment load but selects eleven. Both inspected source tables distinguish
the opposite assignments. This is a source/selector direction discrepancy,
not a newly executed timing probe or a downgrade of the exact source rows.
The coherent receiver must pair 8C/8E register/memory, segment choice, odd/even
word address, override and failure contexts at the sole timing owner. Check
the existing catalog oracle before treating its green status as independent
proof. Do not repair guest register semantics or add an App special case.

Other MOV rows show why this cannot be generalized from one table alone:
Table 1-16's accumulator memory forms on printed 1-33 assign nine to a store
and eight to a load, while Table 2-9 on printed 2-13 assigns nine to memory-
to-accumulator and eight to accumulator-to-memory. The actual dedicated 186
ledger uses the latter directions. Ordinary register/memory MOV rows likewise
need explicit direction/encoding reconciliation rather than a blanket swap.
Those are distinct source-conflict receivers; the independently corroborated
segment rows above must not be hidden by them. No numeric correction or tier
change is implemented in this working audit.

## INS/OUTS Failure Boundary Follow-Through

The actual _p_ins helper validates ES destination access before input, then
performs one checked port input, checked ES write and checked index update.
_p_outs performs a checked overridable-source read, port output and index
update. All four 186 element handlers check these helpers before decrementing
repeat CX; they are not instances of S3's unchecked memory-string element
calls. Preserve that non-defect distinction and one shared port completion
owner. A provider accepting input before a later memory write fails still
requires explicit side-effect/failure proof; register restoration cannot
prove the external read undone.

Existing port_strings_test_forms includes 186 byte/word input/output and
three-element repetitions. Its inspected zero-count, DF and segment-override
boundary function selects 386. Therefore those existing tests are useful
normal 186 coverage, not 186 failure/boundary qualification. The receiver
must distinguish destination validation, input acceptance, memory rejection,
output rejection and completed prior elements, without introducing a second
string executor or assuming all external effects can roll back.

## Range, Formula And Oracle Reconciliation

Additional original Table 2-9 pages PDF 180-184 / printed 2-14 through
2-18 were visually inspected. The following comparison separates selected
cost, routing origin and documented grade; these are not interchangeable.

| Form | Original row | Current selected cost / evidence | Disposition |
| --- | --- | --- | --- |
| MUL register byte/word, memory byte/word | 26-28 / 35-37 / 32-34 / 41-43 | 27 / 36 / 33 / 42 in dynamic arithmetic selector and existing recipes | Midpoint L2; matching range choice, not exact L3. |
| Unary IMUL same four forms | 25-28 / 34-37 / 31-34 / 40-43 | 27 / 36 / 33 / 42 | Rounded-up midpoint L2; sharing MUL's chosen numbers does not make their source ranges identical. |
| Immediate IMUL 6B / 69 | 22-25 / 29-32 | Dedicated helper selects 24 / 31; recipes require dynamic-arithmetic origin | Rounded-up midpoint L2, not operand-dependent exact timing. |
| DIV register byte/word, memory byte/word | 29 / 38 / 35 / 44 | Same four numbers; legacy result records label L3 | Exact Manual-L3 under table conditions; dynamic-arithmetic route name alone is not a downgrade. |
| IDIV register byte/word, memory byte/word | 44-52 / 53-61 / 50-58 / 59-67 | 48 / 57 / 54 / 63; legacy result records label L2:midpoint | True range-based L2, unlike the width-annotated immediate rows inspected below. |
| BOUND | 33-35 | Ledger and recipe select 34; legacy manifest/results explicitly label L2:midpoint | Grade and midpoint agree; control-stack route alone is not an L3 claim. Fault delivery still needs separate proof. |
| ENTER levels zero / one / greater than one | 15 / 25 / 22+16(n-1) | Selector uses these values and keeps the full byte on 186; recipe level two expects 38 | Exact/formula Manual-L3 for the base term; complete access/failure and modifier accounting remains pending. |
| LEAVE | 8 | Ledger selects 8 | Exact base term matches, not whole state/failure qualification. |

The legacy timing manifest and results already distinguish exact DIV from
range BOUND despite their selector-origin names. Do not report those labels
as wrong merely by searching for L2_DYNAMIC_ARITHMETIC or CONTROL_STACK.
The source-form identity for the inspected DIV records is unattributed
(4294967295), however: that existing attribution limitation remains a
receiver and cannot be repaired by relabelling the source as a range.

The segment MOV discrepancy is also present in the existing runner's oracle:
I186-MOV-SREG-TO-M expects nine and I186-MOV-SREG-FROM-M expects eleven.
The legacy results repeat those expectations and mark both L3. Their segment
override and odd-word records inherit the reversed bases. Thus these records
are implementation agreement, not independent source proof. The repair batch
must correct both directional bases and their derived expectations at the
one timing owner, preserving the exact source grade rather than downgrading
the erroneous scalar to L2. No historical result is silently rewritten here.

Printed 2-18 explicitly says EA calculation is four clocks for all modes and
is already included where appropriate. It also gives WAIT six only when TEST
is zero, LOCK two, and ESC an undifferentiated six. These conditions constrain
the inherited F09/F14 receiver: do not add early-family EA calculation again
or treat WAIT's six as its unbounded external wait. Existing ESC recipes use
two for register and six for memory, requiring reconciliation against the
separate generic table rather than an immediate blanket correction.

Printed 2-17's JCXZ/LOOP-family clock-column alignment needs reconciliation
with Table 1-16 and the actual branch predicates; it is not accepted as a
new timing defect from extracted text. The S3 zero-displacement outcome
receiver remains independent of that source-table issue.

## Group 2 State, Count And Timing Follow-Through

The actual seven operation helpers use the shared profile count selector:
186 masks to five bits; early 8086/8088 retain the full byte. C0/C1 check
ModR/M, operand read and immediate before dispatch, then check the result
write. D2 and both width branches of D3 likewise check decode/read/write.
Extension six calls the sole UndefinedOpcode path; the original Table 2-9
encoding inventory has only extensions zero through five and seven. This
does not independently prove the complete type-six delivery frame.

ROL/ROR iterate the masked count and update CF at the last rotated bit.
RCL/RCR additionally reduce byte/word counts modulo nine/seventeen, using CF
as the extra bit. SHL/SHR iterate the mask, set SF/ZF/PF for nonzero count,
and mark AF undefined; SAR shifts a narrowed signed value. The last is the
existing S3 implementation-defined signed-right-shift compiler-contract
receiver, not a newly demonstrated signed-left-shift UB.

Zero-count helpers leave the guest value and FLAGS bits unchanged, but the
shift helpers mark OF undefined even for zero; rotates return before doing
so. RCL/RCR test their reduced count for one/zero rather than retaining a
separate masked architectural count. These internal validity decisions need
source and boundary reconciliation for counts 0/9/17/18/31/32/255; they are
not evidence that guest OF changed on a zero-count instruction. Repository
search found no reader of the udf field, only its assignments/declaration.
Do not invent an external undefined-FLAGS consumer to turn that metadata
lead into an observed runtime regression.

The timing selector uses the original CL/immediate count masked to five
bits, not the reduced carry-ring count: single-bit register/memory costs
2/15 and variable counts 5+n/17+n. This agrees with the inspected Table 2-9
base formulas. It retains checked word read-modify-write odd-address terms
for both transfers and a segment override term, without adding another EA
calculation. Masked zero still selects the setup term; it is not free time.
The memory handlers still perform read and write even for a zero result
count. Bus/MMIO effect and rejected-access proof belongs to the existing
ordered-transfer receiver; unchanged register values cannot prove no bus
activity, nor justify suppressing a write without its source contract.

The existing cpu_rotate_smoke's extensive forms, zero-count and FLAGS
matrices select 386. Its CL cross-profile matrix includes 186 (CL=33, masked
to one), but compares result/nonparticipants rather than CF/OF; it omits
8088. The dedicated 186 immediate checks cover byte ROL and the three shifts
at count one, not the full word/memory/count/FLAGS product. The 186 timing
runner has all seven operations at count one/two and selected memory/prefix
contexts, not the complete extreme-count semantic matrix. Thus existing
green tests support these selected contexts only. One shared Group-2 receiver
must cover all seven operations, byte/word, immediate/CL, register/memory,
the named boundary counts, defined FLAGS and failure effects; do not create
a separate 186 arithmetic path or assert values for undefined flags.

## Immediate IMUL Arithmetic And Regression Follow-Through

_a_imul3's word-by-byte and word-by-word cases sign-interpret both operands,
produce the low word, and set CF/OF together when the full product differs
from the sign extension of that low word. Their promoted products fit the
supported 32-bit host int: even -32768 multiplied by -32768 is 1073741824.
This is not the later dword product overflow. SF/ZF/AF/PF are marked
undefined; the inspected regressions correctly exclude them from preserved
FLAGS assertions.

cpu_imul_immediate_s56_smoke runs both 69/6B word forms on 186/286/386,
including a negative source/immediate, aliased source/destination, an
overflow case, and ordinary memory source with nonparticipant preservation.
Its segment and detailed attribute paths select 386, so they do not prove
186 overrides, failed fetch/read, signed extremes or wrap boundaries.
Those remain the immediate-IMUL receiver, not an absent normal 186 path.

The same helper's later 32-by-8 case multiplies lib_i32 by lib_i8 before the
outer 64-bit mask/assignment. For source 0x40000000 and immediate four, the
mathematical product cannot fit the signed 32-bit intermediate. The outer
mask does not widen the multiplication first. This is a C-level overflow
deduction, not a newly executed crash or a 186 defect. S6 must consume this
dword host-arithmetic receiver alongside its other arithmetic-width cases;
the 32-by-32 case already widens operands before multiplying. No Shared edit
or new runtime probe is claimed here.

## BOUND Delivery And Multiword Address Boundary

ExecFinal preserves oldcpu for the fault, maps BR to vector five and calls
the same real-exception helper used by other delivered exceptions. That
helper restores the fault CPU before _e_except_n; successful real delivery
uses _ser_int_real, not a BOUND-specific board or App interrupt bypass.
The existing test/ibmpc/board-common/machine_bound_board_smoke.c exercises
80186 AX=3 with bounds -2/2, IVT vector five, a handler at 0100h and a
three-word frame. It checks unchanged AX, handler IP, saved IP=0, saved CS=0
and saved FLAGS, then executes the handler's HLT. This is actual existing
186 delivery coverage, not only a BR bit. The lower-bound fault in that test
selects 386; protected-limit and pending-IRQ branches do not qualify all 186
contexts. The CPU-only bound smoke has ordinary 186 word success but its
register-form rejection, endpoints and segment matrices predominantly use
386. Both test owners must be considered before reporting a missing path.

The original printed 2-6 prose establishes signed inclusive comparisons,
adjacent lower/upper words and type five. That paragraph alone does not
specify saved-IP semantics. The existing zero-IP frame expectation is
implementation evidence, not independent source authority; S2's return-IP
receiver must reconcile the 186 source before accepting nonzero and prefixed
BOUND frames. Keep delivery failure and ordered stack writes in that same
receiver rather than inventing a second BOUND frame builder.

The handler initially decodes a 16-bit effective offset, reads the lower word,
increments its lib_u32 offset by two, then separately reads the upper word.
_kma_real_legacy_segment_wrap handles a transfer which begins at/below FFFFh
and itself crosses the end; it does not normalize an already incremented
10000h offset. _kma_linear_logical also gives real-mode DATA references a
32-bit upper bound without restricting that branch to 386, despite its
comment describing 386. Thus intra-transfer wrap and subsequent-operand
offset arithmetic are distinct, and the former helper does not prove the
second-word boundary correct. The effective-offset receiver must compare
BOUND, pointer loads and XLAT across address width and DATA/STACK selection,
with distinct low/high boundary bytes and source proof of each operand's
address rule. This is a concrete unqualified address path, not a newly run
boundary counterexample or permission to mask every logical address.

## ENTER/LEAVE Stack Publication Boundary

ENTER decodes both immediates before pushing old BP, snapshots post-push SP,
then iterates BP decrement, checked SS read and checked push for prior frame
entries. It pushes the frame snapshot when level is nonzero, assigns BP and
subtracts the unsigned local size from the selected stack register. The 186
word register assignments naturally retain 16-bit SP/BP; no new 186 stack
owner is necessary. LEAVE validates the BP-selected stack range, assigns SP
from BP and uses the checked pop owner. Its preliminary range probe uses the
write flag even though the subsequent operation is a read; it is only a
logical-range/access check, not a media/provider write. Retain its source
and protected-mode applicability question for S5/S6; do not claim a 186
write side effect from that parameter alone.

The existing defaults cover 186 levels 0/1/3/33/255 and ordinary LEAVE, with
SP/BP, nonparticipants, FLAGS and selected stack-image assertions. The
level-255 test verifies final state and selected first/second/current-frame
slots, not every intermediate copied slot. It seeds two explicit prior-frame
values; remaining fixture bytes are not a complete independently varied
frame chain. Protected failure/32-bit-stack tests select 386. Consequently
levels 2/31/32, allocation extremes, wrap, source/destination alias and each
selectively rejected read/push remain the stack receiver. A later rejected
transfer cannot be treated as undoing earlier RAM/MMIO writes merely because
ExecFinal restores oldcpu. The coherent receiver must distinguish completed
transfers from current-element failure, not wrap ENTER in a fake all-or-none
machine transaction.

## REP Restart And Optional ESC Control Boundary

ExecFinal handles flagInsLoop by restoring only CS/EIP from oldcpu; the
completed element's CX/SI/DI and data transfers remain visible. Refresh then
calls ExecInt, whose accepted interrupt path calls ExecInit and the existing
interrupt owner. This supplies an element boundary, not proof of source-correct
restart or retirement accounting. In particular, S3's ExecInit-before-timing
receiver also applies to 186 and must preserve the completed element's
observation separately from interrupt delivery.

The source-repeat timing identity compares CS/EIP, opcode, REP kind and
operand/address sizes. It does not include the segment override. Its continuation
state is inferred from equal current and previous EIP. Treat these as inspected
implementation constraints, not a demonstrated interrupt counterexample:
an intervening handler, modified instruction bytes or another segment selection
must be covered by the same restart/retirement receiver before qualification.
Do not introduce a second REP executor or apply an early-family prefix-loss
rule to 186 without generation-specific source evidence.

The existing 186 manifest runner explicitly exercises FIRST/CONTINUATION/ZERO
with segment, odd-word and combined modifiers. Its phase-context recipe executes
two ordinary elements and a separate zero-count machine; it does not inject an
interrupt between those elements. Thus normal repeated formulas have existing
coverage, while IRQ/NMI plus IRET, handler mutation and selectively rejected
element transfers remain named contexts in the restart/publication receiver.
The checked INS/OUTS helpers do not eliminate S3's inherited unchecked-memory-
string receiver; the two sets of handlers must not be conflated.

FPU_ESCAPE decodes the escape operand and uses generic CR0 EM/TS to request NM,
otherwise dispatching through the configured FPU and extension-command owner.
The profile compatibility helper permits an 8087 with 186. Neither the inspected
CPU context/provider declarations nor the CPU source tree expose the relocation-
register escape-trap selection described in the already rendered printed 2-6.
Generic EM/TS is not evidence for that 186 input. The existing board FPU escape
test's prepare_machine fixes its CPU to 386, including its NM delivery cases;
it cannot qualify 186 trap enable, reset-disable or saved-IP behavior.

Keep this as an explicit missing 186 control capability, not a downgrade of
the manual-defined behavior or an invented L1 estimate. Its owned receiver is
CPU escape admission/delivery plus the neutral input from the integrated-control
owner: establish reset and enable semantics, D8-DF register/memory forms and
delivery before proposing the smallest concrete interface. The whole integrated
peripheral block and x87 arithmetic are not silently added by this audit.
WAIT/TEST, no-FPU memory bus activity and ESC timing/source ambiguity remain
the separately named S3/S4 external-cycle receivers.

## Inherited Branch And Unused-Encoding Follow-through

Visual review of PDF 46 / printed 1-30 gives the 186 JCXZ parenthesized
taken/not-taken clocks as 16/5. PDF 49 / printed 1-33 gives LOOP 15/5,
LOOPE 16/6 and LOOPNE 16/5. The dedicated Table 2-9 on printed 2-17 has
clock entries vertically displaced relative to several branch labels. It
cannot be read as a clean independent corroboration merely by pairing adjacent
OCR lines. The current selector uses JCXZ 16/5, LOOP 16/5, LOOPE 16/6 and
LOOPNE 16/6. Consequently JCXZ and LOOPE match the unambiguous generic rows;
LOOP's taken and LOOPNE's not-taken values differ from those rows and require
source reconciliation. Keep this clock-source conflict separate from the
confirmed segment-MOV direction error; do not silently pick a new scalar,
average these exact values or relabel all branch timing L2.

The selector's short_branch_taken helper infers taken from final EIP differing
from sequential EIP. A taken zero-displacement branch ends at that same EIP,
so this inference cannot represent the branch decision in every legal form.
This is the same S3 condition/retirement receiver, now explicitly applicable
to 186 JCXZ/LOOP/Jcc. LOOP handlers check immediate decoding and use the shared
loop helper; the below-386 JCXZ handler leaves both immediate decode and jump
calls unchecked. Carry that exact site in S3's decode-propagation batch rather
than treating a passing ordinary branch recipe as failure-path coverage.
Existing 186 manifest recipes include ordinary taken/not-taken JCXZ/LOOP
and every short Jcc condition; they do not establish zero-displacement
decision publication or selectively rejected immediate fetch.

For FFFF, the below-386 INS_FF extension-seven arm explicitly invokes
UndefinedOpcode. Primary metadata also excludes 63-67 by minimum-family
admission and rejects selected invalid segment/group forms; the early 0F
POP-CS path is not selected for 186. These are existing execution/admission
paths, not missing implementations inferred from source-defined absence.
The decoder inventory enumerates opcode/next-byte lexical candidates and
checks aggregate counts; its own comment correctly says it is not retirement
or timing conformance. Its passing count cannot prove type-six vector/frame
delivery for the entire unused set, prefixed forms or failed fetches. Retain
those contexts under the single admission/delivery receiver with S2's frame
and failure rules, not a separate undefined-instruction executor.

## Inherited ALU, Movement And Port Timing Follow-through

The inspected 186 primary selector distinguishes AAA eight, AAS seven,
DAA/DAS four, AAM nineteen, AAD fifteen, CBW two and CWD four. The existing
manifest recipes contain those same ordinary base terms. This is reconciliation
of clocks, not acceptance of the adjustments' input/undefined-FLAGS semantics:
S3's arithmetic/source and decode receivers still apply to these shared handlers.
XCHG uses three for accumulator-register, four for other register pairs and
seventeen for memory, while LEA uses six. Those bases agree with the rendered
dedicated rows. Port lookup uses IN immediate/DX ten/eight and OUT immediate/DX
nine/seven, also matching those rows; its 186 branch returns the base directly.
Full external-cycle, odd port/word, failure and publication qualification remains
the port receiver, not a claim derived from four matching scalars.

Printed 2-14 explicitly labels accumulator-immediate ADD/ADC/SUB/SBB/CMP
3/4 with the comment 8/16-bit. These are width-selected numbers in that
table, not an unqualified latency interval. The actual 186 selector uses four
for accumulator-immediate ALU regardless of byte/word and also four for CMP
3C/3D. Its existing byte recipes, including 04 and 14, expect four. The legacy
results label those recipes L2:midpoint and call the source a range. This is
an attribution/selection discrepancy requiring a paired byte/word source
reconciliation, not evidence that a manual numeric width rule lacks L3 data.
Preserve the generic-table wording separately if it conflicts; do not average
away the dedicated width annotation. Extend the same source-width receiver
to logical accumulator immediates and TEST after their individual rows are
confirmed, rather than assuming every grouped form shares one source rule.

Printed 2-13 likewise puts 8/16-bit next to immediate MOV's 3-4 and 12-13
entries. Both the legacy and primary selectors currently use four/thirteen
without a width split, and legacy recipes label these as midpoint L2. The
receiver must reconcile B0-BF and C6/C7 independently, including register
versus memory destination, rather than mechanically upgrading the labels
while leaving the selected byte value wrong. These width-annotated entries
are distinct from the genuinely range-based MUL/IMUL/BOUND rows above.

Ordinary register/memory MOV has a further source conflict: dedicated printed
2-13 pairs encoding 88/89 (register to r/m) with 2/12 and 8A/8B (r/m to
register) with 2/9; generic printed 1-33 assigns memory-store nine and
memory-load twelve. The current ledger follows the generic direction.
Keep this conflict with MOFFS in the MOV-source receiver, not with the
independently corroborated reversed segment-MOV defect. Its repair must use
resolved encoding/direction evidence, not swap all MOV rows together.

XLAT's unprefixed eleven-clock row matches. The legacy helper adds its segment
override term only for 8086, although printed 2-13 separately gives 186 segment
prefixes two clocks. The 186 prefix/address receiver must reconcile the actual
XLAT-selected path and observation modifiers before accepting prefixed timing;
the unprefixed recipe cannot prove it. The existing BX+AL effective-offset
and checked-read concerns remain in their S3 receiver. No duplicate lookup,
App-specific timing or Shared implementation change is introduced here.

## Logical Width Sweep And Signed Division Boundary

Visual review of PDF 50 / printed 1-34 also confirms NEG and NOT's generic
186 register/memory base of three (parenthesized in the early table), and
NOP three. The actual unary selector chooses three for both operand locations;
NOT's dedicated printed 2-15 row also gives three. The apparently unusual
memory value is therefore not itself a confirmed defect. Transfer/prefix
modifiers and failure effects still require their existing proof receivers.
The same original page gives non-repeated MOVS nine, whereas dedicated
printed 2-15 gives fourteen and the current 186 repeat ledger uses fourteen.
Both tables give the repeat formula 8+8/rep. Retain the unprefixed non-repeat
source conflict in the timing/restart receiver; do not conflate it with a
wrong REP formula or silently average two exact numbers.

Visual inspection of printed 2-15 confirms that accumulator-immediate
AND, OR, XOR and TEST each have 3/4 with an 8/16-bit comment. They therefore
join ADD/ADC/SUB/SBB/CMP in the same width-selected source receiver, now with
their individual rows inspected. The selector groups logical ALU accumulator
forms with arithmetic forms at four, and separately gives TEST accumulator
four. Existing byte recipes and legacy midpoint labels use those values.
The receiver must cover all paired opcodes and preserve each operation's
register/FLAGS semantics; do not introduce separate logical versus arithmetic
timing paths just to fix the shared width selection.

IDIV on that page instead gives four genuine latency intervals. The dynamic
arithmetic owner selects 48/57/54/63, the corresponding midpoint, and the
legacy results correctly label L2:midpoint. The helper is explicitly restricted
to 8086/8088/186 before reaching that branch; its later-looking shared name
does not mean it assigns those clocks to 286/386. No grade correction is
needed for these IDIV range rows.

The shared _a_idiv implementation widens its arithmetic into lib_i64 and
checks zero divisors before division. Its byte quotient condition permits
-128..127, and its word condition permits -32768..32767. The word computation
widens DX before shifting for the actual dividend, but an earlier operand
bookkeeping expression still shifts promoted DX before conversion. Carry
that expression in S3's signed-host-arithmetic receiver; a safe actual divide
does not make preceding C evaluation safe. The explicit signed-overflow guard
does not establish the source-qualified 186 quotient interval: S3's source
conflict remains pending rather than inferred resolved by shared code.

The CPU DIV/IDIV form test in cpu_inc_dec_first_group_smoke.c selects 386 for
its byte/word/dword register/memory result matrix; its additional legacy
admission case selects 286. The 186 timing manifest and normalization test
provide ordinary register/memory clocks, not an independently varied signed
boundary/fault matrix. The 186 receiver therefore still needs both quotient
endpoints, just-outside values, zero, mixed signs/remainder sign, memory-read
failure, unchanged faulting architectural state and qualified vector-zero
return location. Keep arithmetic publication, source interval and exception
delivery in their existing owners rather than adding a 186-only divide path.

## Remaining Stack And Inherited State Scope

The existing PUSHA/POPA default matrix explicitly includes 186 word execution.
PUSHA checks the complete eight-slot stack image against the original register
values, including original SP; POPA uses eight distinct input slots, checks
the seven restored registers and final SP, preserves FLAGS/segment state and
checks that the input image was not rewritten. This is stronger ordinary-state
coverage than a final-SP-only assertion. Do not describe the normal image/order
as missing. The attribute and protected stack-limit tests select 386, however;
186 wrap, odd SP and selectively rejected individual reads/writes remain the
ordered-stack receiver. POPA currently reads the discarded SP slot through
the same pop helper, so skipped architectural assignment alone does not prove
its source-qualified bus/failure behavior.

PUSH immediate's defaults include 186 68 with 1234h and 6A with 80h producing
FF80h. They check FLAGS and unchanged nonparticipants in the shared success
helper. These are real immediate-word and negative-byte tests, not proof of
all sign-extension endpoints, fetch rejection or SP wrap. Keep 00/7F/FF and
the failure/wrap matrix with that same checked-decode/stack receiver. A later
protected failure case cannot be relabelled as 186 evidence.

Current reset still assigns IP=FFF0h, CS selector F000h, base F0000h and
FLAGS=0002h for 186. S2's visually sourced Table 2-30 discrepancy (visible
FFFF:0000 and status F002h) is therefore unresolved; retaining physical
FFFF0h does not correct the visible reset registers. The current below-286
FLAGS mask is 0FD5h for both load and image, and the direct-FLAGS regression
expects 186 high bits cleared after its POPF/PUSHF sequence. That expectation
is implementation evidence, not source proof overriding the reset/readout
receiver. The coherent correction must distinguish writable canonical state,
architectural image and reset rather than placing F002h into every FLAGS load.

MOV/POP segment handlers still set flagMaskInt for STACK selection, and the
common interrupt arbitration/fault-delivery owner remains as inspected in S2.
Keep 186 inhibition and return-IP qualification generation-specific: the
early source's broader segment-inhibition rule and later protected LSS rule
are not automatic 186 requirements. No new interrupt queue, board correction
or generic FLAGS owner is proposed. Integrated relocation/UMCS registers are
still an explicit missing integrated-control boundary, not silently emulated
by the CPU instruction profile.

## Complete Batch Disposition And Repair Ownership

This is the disposition of the thirteen audit partitions, not an acceptance
of every instruction as qualified. Source/code/test reconciliation above
identifies ordinary matched contexts and the complete residual context classes.
Every residual stays inside T544. S2/S3 receiver definitions remain in force;
the following mapping adds the 186-specific members instead of creating a
parallel implementation plan or transferring them to another T.

| Audit partition | Reconciled evidence | Residual receiver and required contexts |
| --- | --- | --- |
| PUSHA/POPA | Word order/original-SP, ignored SP assignment and full normal image/state assertions | Ordered stack publication: all eight access positions, wrap/odd SP, discarded-slot bus behavior and rejected transfer. |
| PUSH immediate | Checked decode/push, word 1234 and byte 80 sign extension, ordinary ten clocks | Checked decode plus stack publication: byte endpoints, fetch/write failure, SP wrap and observable prior effects. |
| Immediate IMUL | Word product/truncation/CF/OF, ordinary register/memory tests and true range L2 | Arithmetic/publication: signed extremes, alias/segments, rejected fetch/read; later dword-by-byte C overflow is explicitly S6, not 186. |
| Seven Group-2 operations | 186 five-bit mask, checked decode/read/write and single/variable base formulas | Count/FLAGS/publication: zero/9/17/18/31/32/255, carry-ring versus original count, memory effects and rejected transfers. |
| BOUND | Signed inclusive checked comparison, adjacent reads, existing upper-bound type-five delivery | Effective offsets plus exception delivery: lower/endpoints, nonzero/prefixed return-IP, second read/wrap and failed frame access. |
| ENTER/LEAVE | Full-byte level, algorithm/formula, normal and selected level-255 state/image tests | Ordered stack: levels 2/31/32, all copied slots, size/wrap/alias and each failed access; LEAVE probe applicability also follows S5/S6. |
| INS/OUTS | Checked helpers, normal byte/word/REP and exact base/repeat formulas | Port/string publication: zero/DF/segments, accepted input before failed memory write, output rejection and prior completed elements. |
| Inherited F01-F03 | Shared forms inspected with 186 adjustment/ALU/movement clocks and existing normal ALU tests | S3 decode/arithmetic/address/FLAGS receivers, plus paired-width exact source selection and corroborated segment-MOV correction; unresolved ordinary MOV/MOFFS sources remain distinct. |
| Inherited F04/F05/F10 | Ordinary stack/control clocks, shared reset/image/delivery owner and branch tests | S2 reset/FLAGS/inhibition/return-IP plus S3 ordered writes/branch-decision receiver; LOOP clock conflicts require source resolution before scalar changes. |
| Inherited F06/F07 | MUL/IMUL/IDIV true ranges, DIV exact rows, word signed arithmetic path | Quotient/source/host-arithmetic receiver: boundaries, zero, sign/remainder, operand bookkeeping and fault-state/frame contexts; no unsupported L3 claim from range midpoint. |
| Inherited F08 | Normal repeat phases and segment/odd combinations, element rewind implementation | S3 checked-element and retirement/restart receivers: IRQ/NMI/IRET, handler mutation, full prefix identity, failed element and completed earlier effects. |
| Inherited F09/F14 | Ordinary ports/HLT/flags base rows, actual unused handlers and FPU admission path | External cycle and admission/delivery: WAIT/TEST, LOCK lifetime, ESC memory/clock conflict, complete type-six contexts and missing 186 relocation-controlled type-seven input. |
| Timing qualification | Original numeric/formula/range conditions, actual selectors, legacy labels/oracles and source conflicts | Sole timing/retirement owner: fix paired directions/widths and attribution with independent predicates, preserve source assumptions, and do not use green old oracles as manual proof. |

The integrated-control receiver is a retained missing capability, not a new
implemented peripheral or a fabricated L1 fallback. The confirmed source
discrepancies require concrete Shared review before code/test edits. No
permission to lower an exact manual rule to L2/L1 is inferred from this audit.
No whole-family qualification predicate is satisfied while these receivers
remain pending. Completing this read-only inventory does not exhaust T544's
repair/source/regression obligations or qualify 80188/486 aliases.

## Verification Boundary

Seven selected existing tests ran once each host width in the retained caches:
unit.machine-80186-timing-manifest-runner,
unit.machine-80186-instruction-timing-ledger-smoke and x86.cpu_push_immediate,
x86.cpu_pusha_popa, x86.cpu_enter_leave, x86.cpu_port_strings, x86.cpu_bound.
The exact anchored CTest selection passed 7/7 x64 in 2.29s and 7/7 x86 in
2.71s. No new regression/probe was added. These are baseline behavior checks,
including the identified reversed MOV oracle, not independent manual proof.
S3's green full-unit results are not relabelled as S4 evidence.
Two further selected existing tests, x86.cpu_rotate and
x86.cpu_imul_immediate_s56, pass once each width (2/2, 0.07s each). Their
profile/context limitations are stated above; no full-unit gate is inferred.
The existing unit.machine-bound-board-smoke passes once each host width
(1/1, 0.05s each). Its explicit 186 upper-bound delivery context is recorded
above without extrapolating to the omitted boundary/failure matrix.
Shared source/tests, manifests, App configuration, media and artifacts remain
unchanged. No binary rebuild is required for this documentation-only audit.
The existing x86.cpu_inc_dec_first_group, x86.cpu_legacy_alu_s2 and
unit.machine-legacy-timing-normalization-s2-smoke pass once each host width:
3/3 x64 in 0.23s and 3/3 x86 in 0.25s. These corroborate existing behavior
only, with the profile and source-boundary limitations recorded above; they
are not new regressions or the complete S4 unit gate.

Fresh complete repository-only units now pass once each host width:
x64 506/506 in 142.71s and x86 506/506 in 63.31s. Both use the retained
default receiving cache, RunTestAggregate.ps1, eight jobs and a 300-second
deadline. Both owned process handles returned exit zero. These are complete
baseline non-regression gates, not independent source qualification or proof
for the omitted contexts listed in the disposition table. No rerun is needed
to replace the slower x64 result with a preferred duration.

Executor self-review maps all thirteen partitions to the source/code/test
inspection and named residual receivers above. Confirmed discrepancies,
source conflicts, inspected non-defects and incomplete regression coverage
remain distinct. Source/test/artifact changes are zero; all existing deployed
EXEs remain current. Documentation governance, changed-document links,
the active sixteen-field packet and actual diff are checked before delivery.
The ignored research scratch and receiving caches remain needed for the
next family audit. S4 acceptance is inventory acceptance only; T544's full
CPU repair/source/regression qualification is not achieved by this delivery.
