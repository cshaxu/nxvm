# T546 S6 Stack And Frame Publication Design

## Admission And Whole Batch

Continue from accepted S5 governance ad791b4c9. The owner approves consecutive
numeric admission and existing CPU/Core/IBMPC repairs, including all timing
upgrades; downgrades require approval. Shared source/test and NXVM evidence/
receiving artifacts are admitted; Lib/Common/MyNES and new public APIs are not.
Preserve unrelated MyNES documentation. No S6 production repair is claimed.

Consume the complete T544 stack/frame receiver: S3 F04, S4 PUSHA/POPA,
immediate PUSH and ENTER/LEAVE, S5 stack/frame reconciliation and S6 stack/
alias continuation. Original pages and every caller precede concrete edits.

| Member | Current source fact / required proof |
| --- | --- |
| Scalar PUSH/POP | _kec_push checks/writes before SP publication; _kec_pop reads then increments except aliased SP/ESP. Reconcile both early profiles, PUSH SP generation rules, widths/wrap and provider failure without replacing a correct alias guard. |
| PUSHA/PUSHAD | Original SP/ESP and order are already preserved; eight scalar pushes lack instruction-level range admission. Prove applicable spans and real/VM special starting-SP exception/shutdown selection before instruction writes. |
| POPA/POPAD | Reverse order and discarded SP assignment already agree. The 386 original explicitly reads the discarded slot through Pop(); do not remove that read without source proof. Reconcile 186/286 source, full frame and each rejected slot. |
| ENTER | Immediates are checked; 186 keeps full lexical byte and later CPUs mask to 31. Chain address follows SS.B inside operand branches, contrary to independent OperandSize/StackAddrSize. Final local subtraction lacks boundary admission. Reconcile every source/destination, frame snapshot and allocation together. |
| LEAVE | Frame-to-stack uses SS.B, pop width follows OperandSize. Preserve its legitimate probe but reconcile permissions and hard-coded nonprotected limit against source/cached-limit rules; prove failures and cross-width combinations. |
| POP aliases/destination | INS_8F pops a temporary before EA decode; POP SP/ESP alias handling exists. Preserve post-increment ESP addressing and prove failed destination publication/old-SP restoration. |
| Shared validation/publication | _s_test_ss_push also serves CALL/INT frames. Map every caller before a shared change. Early range admission differs from page/provider transfers; restoring registers does not undo accepted RAM/MMIO effects. |

## Design Boundary And Verification

CPU remains sole stack/register/exception owner. Preserve original table
handlers and checked scalar transfers. Reuse/replace a genuinely shared private
validation owner only after its complete width/span/caller contract is proven;
no caller-local protection pile, second executor or generic undo log.
An aggregate frame can exceed the byte-width transfer argument (186 ENTER's
255 levels); never truncate its span into lib_u8.

Initial source review confirms ENTER's wrong chain-width selection and missing
final allocation admission; PUSHA/POPA have only scalar checks. Existing shutdown
request state/API is present. S6 selects the appropriate existing request;
complete delivery/shutdown remains S9, not a product-stop workaround.
Preserve legitimate wraps and distinguish early segment checks from later
page/provider failure and already accepted irreversible transfers.
Full gate/task/paging/delivery remains S9/S11-S13, FLAGS S7 and scalar clocks S16;
changed shared callers nevertheless need direct proof or an approved revision.

Read original early PUSH/POP, 186 added-form/full-level, 286 section 7.4.1 and
stack dictionaries, and 386 operations/independent attributes/special stack
cases/failure clauses. T544 retains exact archived PDF identities/pages. OCR
only navigates; original pages decide. Record source conflicts before choices;
exact values/formulas stay L3, never invent precise undefined output.

Retain gpr_push_pop, pusha_popa, enter_leave and segment-stack regressions.
Add code-owned owner-local family/operand/SS.B/mode/span/odd/wrap/alias matrices,
nesting 0/1/2/31/32/33/255, allocation endpoints, discarded-slot semantics and
every selectively rejected access. Distinguish instruction effects from delivery.
No external files enter units. Detectable retired/wrong owner shapes receive
a narrow static guard and selftest, not a framework.

Before P delivery: final complete x64/x86 units at unchanged 300 s after builds
cease; applicable supplemental/manifests/Types/corpus/gates; eight affected
stripped 0546 products and original 58 integration contexts once per final
group. No INI/media/firmware/MyNES mutation. Count code-size and retained paths,
preserve failures and complete batch dispositions; immutable-P coordinator
review precedes closure. No production repair, P or S6 acceptance yet.

## Fresh Original Pages And Concrete Pre-fix Reproducer

Intel 386 DX PRM 1990 SHA-256
9A8188F9D2282B113FC421E225CC2A643FCDC349E5C3C43659BD2CF6620F1EA1:
PDF 380/17-62 and 381/17-63 were rendered and visually read. The operation
selects BP/EBP chain decrement/read by OperandSize; local allocation uses
StackAddrSize. Protected ENTER requests SS if SP/ESP exceeds the limit at
any point. The description's simplified operand/stack wording cannot erase
the explicit independent attributes. Its frame-ptr assignment names ESP;
reconcile that with the support-function/chapter definitions before changing
the existing stack-size-selected frame snapshot or its oracle.

PDF 457/17-139 and 458/17-140 confirm PUSHA/PUSHAD order and original SP/ESP,
protected starting/ending stack limits, and real/VM initial SP/ESP 1/3/5
shutdown versus 7/9/11/13/15 exception 13 before execution. No inference from
eight scalar checks can substitute for that starting-state decision.

Intel 286/287 PRM 1987 SHA-256
AD487BA99B48CD9F61B14C0FE912A04C7CDB4C7C14A18419AA9FAF62D8962460:
fresh PDF 139/7-13 says limit/access checks precede memory references and
push-class internal-register changes, explicitly including PUSH/PUSHA/ENTER/
CALL/INT. Keep this segment-admission fact separate from later provider and
page effects. Remaining original dictionaries/family clauses still need full
reconciliation before production changes. Originals/renders remain external/
ignored; no protected acquisition or import occurs.

The existing enter_leave test now adds all four 386 operand/SS.B combinations.
Both candidate chain addresses are valid within one cached wide expand-up SS;
correct and wrong locations contain different values. Thus it proves address
selection rather than using an artificial bounds fault as its oracle. It
checks the copied chain cell, EIP and preserved FLAGS without guessing the
still-to-be-reconciled frame-ptr upper bits.

Pre-fix strict builds pass, but both x64/x86 tests correctly return 1 with:

- operand16/stack32: observed 1234, required BEEF;
- operand32/stack16: observed 12345678, required CAFEBABE.

Matching-width contexts do not fail. This directly confirms the two crossed
chain defects; it is not a full S6 regression pass. Test manifest is refreshed
and production/source/products remain at accepted S5. No P is delivered.

The caller sweep also finds existing distinct _s_test_stack_frame_16/_32
helpers used by four inner CALL/INT gates, alongside _s_test_ss_push's scalar,
CALL and INT callers. Their current _kma_test_access includes translation;
ordinary logical segment admission must not casually replace or duplicate
those gate/page contracts. Complete caller and source decisions remain next.

## Sourced Initial Production Scheme Within The Whole Batch

Fresh 386 PDF 452-453 confirms the explicit discarded Pop() and complete
starting/ending checks; 455-456 separates StackAddrSize from OperandSize and
PUSH's real/VM SP=1 shutdown; 414 confirms LEAVE's SS.B assignment then
operand-width Pop, read-only logical reference and real operand-boundary error.
Fresh 286 PDF 248-249/272/293/296 confirms ENTER's intermediate/allocation
limit requirement, LEAVE read and exact five clocks, POPA starting/ending
checks, PUSHA initial odd-SP distinctions. Complete timing corrections remain
S16; do not change a clock just to make state tests pass.

Intel 386 ENTER's frame-ptr=ESP is independently corroborated by local read-only
86Box x86_ops_stack.h: both word/dword ENTER capture full ESP after PUSH, while
chain reads choose BP/EBP and allocation alone chooses SP/ESP. This is reference
comparison only; no external code is copied. Preserve the manual full register
snapshot, including high ESP when stack addressing is 16-bit. Correct the
existing zero-extended dword/word-stack test oracle with that source, not with
the current implementation.
Reference identity: 86Box local commit
4fef696a4eead1d55a28d6ac0e5bd2864e5454da; x86_ops_stack.h SHA-256
3F1B9CE14340EABF356729FAD0124E192AE4C45ED69E94874F79DD954430BA23.

Apply the now-sourced width/snapshot/probe subset in-place: remove ENTER's
redundant nested SS.B chain branches, retain operand branches and transfers;
capture ESP directly; use LEAVE's existing logical read admission and cached
limit instead of a second hard-coded 64K guard. This is not S6 completion:
aggregate stack admission, scalar/alias/wrap, special starting-SP selection,
ordered failures and all shared callers remain the admitted production/proof
batch. No partial P or narrowed exit follows from the first corrected tests.

The sourced initial patch is applied only in cpu_instructions.c (+8/-59,
net -51). Source SHA-256 is
0086AF50B0046EE237A8FA3DF28F437AD81CCA634E082DF8569B6D23F6F80613.
ENTER chain branches now use operand-selected BP/EBP, full ESP supplies the
frame snapshot and LEAVE delegates logical read/limit admission to its existing
owner. The old test's dword/word-stack frame snapshot oracle is corrected by
the original operation and independent reference, while physical stack-image
offsets still follow SS.B. No clock/public API/board/Lib/Common/MyNES change.

The formerly failing crossed combinations now pass. Existing CPU stack probes
also pass 3/3 per width (x64 0.54 s, x86 1.49 s) after rebuilding the sole CPU
archive and test targets. These are early diagnostics, not full unit/S6 closure.
Source/test X86 manifests are refreshed to S6 and verify; diff checks pass.
No receiving EXE is rebuilt or claimed current for S6 until production freezes.
Aggregate admission, special starting-SP selection, scalar/wrap/alias and all
ordered provider failures remain the same admitted batch and must still be
implemented/proven before any P delivery or S acceptance.

## Whole Ordinary Frame Admission Decision

The new provider-rejection probe confirms all ten eligible 286/386 width/SS.B/
push-pop combinations reference the first valid slot before discovering the
invalid later slot. Its invalid IDT prevents delivery from legitimately using
the watched frame; source-required early segment admission must make zero
provider calls. This is a concrete failure-priority proof, not RAM rollback.

Add one private ordinary-frame logical admission owner, consumed by PUSHA,
POPA and ENTER. Project actual width-masked stack addresses without changing
registers; check each source/destination span through _m_test_logical only.
Do not translate or commit page effects here. Protected downward underflow and
ENTER's final allocation pointer are checked before instruction transfers.
ENTER also projects operand-width BP/EBP chain reads before its first push.
Use a wide element count (186 lexical 255 is not a transfer byte count).
Existing translated inner-gate frame helpers remain distinct, with complete
gate/page priorities retained for S9/S11-S13; no new writer/executor is created.
Scalar early wrapping, real/VM initial-SP selection and selective accepted
provider effects remain this S6's pending receiver, not waived by this patch.

The ordinary logical-frame helper is applied and shared by PUSHA/POPA/ENTER.
ENTER's projected chain references and protected final allocation are checked
before its first transfer. No page walk or external access is introduced by
this helper; existing scalar writers/readers and gate probes remain their
respective owners. Current production source SHA-256 is
AA5CA6DA8C646C01098419C9C2B855E57893B6A40F27A8EA4F78C82C0F01FE63.
Cumulative production diff is +53/-59, net -6 against accepted S5.

The formerly failing ten invalid frame contexts now make zero provider calls.
Ten matching valid-frame controls make one rejected provider call, preventing
a vacuous always-reject/no-access implementation from satisfying the probe.
Both widths pass that final twenty-context admission matrix. Existing PUSHA/
ENTER cases also pass (x64 2/2 in 0.21 s; x86 2/2 in 0.54 s before adding
the ten valid controls, whose rebuilt PUSHA targets then pass both widths).

An existing ENTER protected-fault assertion expected old partial writes even
though the failure was a segment-range violation; that is corrected to an
independent full-frame sentinel comparison under the original early-check
contract, not to arbitrary current results. Existing PUSHA limit sentinels had
four values identical to their prospective writes, masking the defect; all
five values are now independent. Source/provider failures after valid segment
admission must still preserve earlier accepted effects and remain to be tested.
No oracle claims complete delivery or fictitious memory rollback.

Source/test manifests are refreshed. This is still an incomplete S6 batch:
real/VM special starting-SP selection, early wrap/scalar/alias matrices, each
selective accepted transfer and remaining shared caller admission/publication
proof are not closed. No full suite/product qualification or P is delivered.

## PUSHA Starting-SP Selection

Original 286 B-88 and 386 17-140 explicitly require pre-instruction 1/3/5
shutdown and 7/9/11/13/15 GP in real/VM mode. Sixteen real-mode word-context
regressions (two families, eight odd pointers, valid GP vector) reproduce the
missing shutdown request and 386's wrong SS selection. The 286 GP contexts
already work; preserve them. Place this classification at PUSHA admission,
before the ordinary frame validator, using the existing SHUTDOWN exception
marker and GP raiser. Do not alter the general logical memory owner or make
all invalid stack accesses shutdown; complete shutdown lifetime remains S9.

PUSHA's existing request selector now passes all sixteen real word cases on
x64 without changing delivery APIs. Early PUSH AX wrap tests across 8086,
8088 and 186 and SP 0/1/2/3/FFF0/FFFF reproduce one common defect per family:
SP=1 is rejected by _s_test_ss_push before the existing legacy wrap owner can
split the word at FFFF/0000. Keep modern checks, but restrict this scalar
underflow guard to protected-capable families; legacy logical/physical span
admission remains at its existing owner. Sweep CALL/INT users of the same
helper during complete qualification; do not add a second wrap path.

The starting-SP selector and legacy guard correction are applied. Sixteen
real word PUSHA contexts and eighteen early PUSH AX boundary contexts now
pass; existing stack targets pass 3/3 per width (0.38 s/1.25 s). All three
existing GPR PUSH/POP profile matrices also include the previously omitted
8088, preserving their original eight-register and memory/alias assertions.
Rebuilt GPR targets pass both widths. Production SHA-256 is
93C3FA7D12D84787363686FA706434065932689E5F03831EE46BBB23582F366A.

These remain early diagnostics. Extended operand/stack-width and VM special
states, per-transfer provider rejection with accepted-effect observations,
ordinary scalar PUSH's modern SP condition and complete shared-caller proof
still belong to S6. No new public type/function, generic shutdown conversion,
delivery lifetime rewrite, product artifact qualification or P is claimed.

## Explicit Scalar PUSH Shutdown Admission

Ten 386 SP=1 cases across register/immediate/memory/segment/FS PUSH and
real/VM mode reproduce missing explicit admission: real mode stops without
requesting the right shutdown; VM mode reaches shutdown only through failed
exception delivery, with no source shutdown snapshot. Intel 17-138 gives the
same lack-of-stack shutdown in both modes. Add it to the regular _e_push owner
for the documented PUSH opcode families using its existing prefix classifier
and shutdown marker. Exclude ENTER, PUSHA and PUSHF from this dictionary rule;
implicit CALL/INT pushes already use _kec_push and are not reclassified.
No public interface or delivery-lifetime change is introduced.

The scoped regular-PUSH admission is applied and its ten real/VM cases pass
both widths; compound ENTER/PUSHA and implicit CALL/INT are not classified by
that opcode rule. Current production SHA-256 is
139D5250694C1AA2EE7207363C394A17BB8D6AB9E5FA8D6F509DC7086149FB02.

The existing owner-local frame probe now also observes accepted transfers and
rejects a selected one. Ninety-six valid-segment contexts cover 186/286/386,
applicable operand/SS.B combinations, PUSHA/POPA and every slot 1..8. They
require the exact attempted-transfer count, CE provider rejection and original
CPU register restoration. For PUSHA, each earlier accepted slot retains its
source value; the rejected and later slots retain independent sentinels.
For POPA, all input cells remain unchanged, including the discarded-SP read.
Both widths pass this matrix, using the same fixture as the twenty invalid/
valid early-admission controls. No extra test executable or production undo
mechanism is introduced. Direct test readback bypasses the rejecting spy;
it observes actual fixture memory, not a mirror of expected writes.

Source/test manifests are refreshed to this stage. No full-unit/product/S6
qualification or P yet: extended special-state/attribute matrices, remaining
ENTER source/read/write rejection positions and complete shared-caller proof
remain in the same batch. Source grades and existing delivery-lifetime owner
are unchanged; these early results do not waive S9/S11-S17.

## ENTER Ordered Effects And Allocation Endpoints

The truly shared test observer is extracted to CPU-local
support/cpu_stack_probe_fixture.h and its old PUSHA-only definition deleted.
Both owners consume the same actual read/write callbacks and one fixture CPU;
it is not a second machine state or production/public API. Manifest includes
the new header; Types and manifest checks pass.

ENTER now has 1,421 finite success/selective-rejection contexts across 186,
286 and eligible 386 operand/SS.B combinations and levels 0/1/2/31/32/33/255.
Every provider position is rejected once, with zero meaning the valid control.
The independent event order distinguishes initial BP push, each chain read and
push, and final full-ESP frame snapshot. Tests check attempted count, provider
CE and CPU restoration; accepted earlier pushes retain their bytes, while
rejected/later destinations retain independent sentinels. All copied chain
slots are independently populated, not only the first two. Successful 186
full-byte levels and later masked levels preserve the source formulas.

A further 420 protected allocation-admission contexts cover both families,
eligible operand/SS.B, normal/expand-down, those seven lexical inputs and
allocation 0/8/80h/FFh/100h/FFFFh. The mathematical span/allocation oracle
expects no provider access on invalid ranges and a first rejected provider
call on valid ranges. This proves admission without a fake data transfer for
uninitialized local space or a vacuous no-access implementation.

Both matrices pass x64 and x86. Updated PUSHA/ENTER owners pass 2/2 on x86
in 2.50 s and x64 in 0.31 s before the final allocation addition, whose
standalone rebuilt ENTER target passes. Production remains at 139D5250...;
all changes here are code-owned test proof/fixture reuse. No full unit/product
qualification or P follows. Extended special/alias/wrap and shared gate/frame
callers remain S6; complete general page/delivery behavior remains its named
later owner, never inferred from these nonpaged provider checks.

## Alias, Shared Callers And Remaining Error Selection

Seventy eligible POP contexts cover real/cached-wide, protected and ordinary
VM, operand/SS.B, direct SP/ESP and 8F register aliases, post-increment ESP
memory addressing and source/destination provider rejection. Ordinary VM uses
16-bit SS, 64K caches and consistent selector/base pairs; synthetic wide VM
conditions are not advertised as hardware coverage. Both widths pass.
Twenty-seven early near/far CALL and INT wrap controls across 8086/8088/186
and SP 0/1/FFFE also pass, directly covering the changed underflow helper's
shared callers. Twenty-four ENTER BP=SP overlap contexts pass; copied slots
observe preceding actual writes, proving admission does not pre-cache values.

Final range-owner reconciliation adds POP/LEAVE real FFFF-word regressions.
286 already requests GP; 386 incorrectly requests SS. Original 386 POP
17-135 and LEAVE 17-96 specify exception 13, as do their VM clauses. Expand
the existing nonprotected modern stack-range GP selection from 286 to 386;
do not scatter caller-specific conversions or turn all bounds failures into
shutdown. Explicit PUSH/PUSHA initial-SP requests remain their own admission.

The corrected modern nonprotected range selector is applied and POP/LEAVE
FFFF-word GP regressions pass, alongside all three stack targets on both widths
(x64 3/3 in 0.35 s, x86 3/3 in 3.36 s). Production SHA-256 is
64E357CFE02A65CCE79A4FB47E783E053E03ADDDA04BB47BA874557BF3D4D4C1.
The VM PUSHA admission matrix additionally covers both operand widths and
all eight odd initial pointers: shutdown markers reach no IDT fetch, while
GP cases first fetch vector 13's descriptor at 3068h. Both widths pass; this
proves request selection before unqualified general VM delivery, not the
complete TSS/error-frame/lifecycle contract retained by S9/S12.

Source/test manifests are refreshed. No source clock value, public API,
Lib/Common/MyNES or firmware/INI/media change. Complete final units, guards,
all shared-caller regression/gates, eight receiving products and original
integration are still mandatory before P delivery; this is not S6 closure.

Final cleanup decision: _e_pop still only forwards to _kec_pop, with empty
trace notation and duplicate error checks. Retarget its complete 52-caller
set to the existing scalar pop owner and delete that private wrapper. The
_e_push boundary stays because it now owns explicit instruction admission;
segment/gate pop boundaries likewise retain their distinct responsibility.
Add a narrow source guard and negative selftest for early frame admission,
logical-only ordinary probes and independent ENTER chain width. No public API
or generic validation framework is introduced.

## Final Source Freeze And Qualification Start

The forwarding POP layer is removed and all 52 actual callers now use _kec_pop.
Its SP/ESP alias predicate is unchanged. The private frame predicate uses
lib_bool and original trace notation; no public ABI change. Source is frozen
at EAEBFB069779AE10F46E0170583FCD234AF1445B5810B571608C9822F6C6A391.
The source guard and five negative probes plus one valid control pass: no
retired forwarding owner, ordinary admission cannot translate/publish, each
aggregate admits before transfer and ENTER chain has no SS.B switch.

The registered complete unit route adds those two guards to the prior 539;
final expected universe is 541. No old runtime test entry is removed.
Counted eight source/test/build paths give +961/-135, net +826 (production
alone +139/-120, net +19). New test code provides independent finite source/
effect/failure matrices and one shared observer, not another production path.
Manifests/docs/generated/artifact paths are excluded from that count.

Both complete default Release builds are launched; six other receiving-cache
product/harness builds run serially with MyNES OFF. Do not restart a live
handle because generation is quiet. Wait for builds to cease before full
unit execution, preserving S5's diagnosed containment/load evidence. Then
run final dual units, supplemental/gates/manifests/Types/corpus and original
58 external contexts once per final group; verify all eight deployed PE/hash
identities, unchanged INIs/media masters/MyNES and actual final changes.
This is qualification start, not an S6 acceptance or P delivery.

## Qualification Resume And Static Evidence

The original full-build handles 58215/52811 and serial receiving handle 51803
are re-polled and remain live; no observation timeout caused a restart.
Their terminal success must precede the complete sequential dual-width unit
route, with the unchanged 300-second deadline and four jobs. The waiting
orchestrator starts that route only after all three build handles succeed.

Fresh verification confirms all eight source/test manifests, the ordinary
stack-owner guard, its valid control and five negative probes, test/x86 Types
and git diff --check pass. Production hash still equals the frozen EAEBFB06…
identity. Review of the actual production and three owner-test diffs confirms
that admission does not cache chain bytes, provider rejection tests read actual
fixture memory, discarded POPA slots remain reads and the private forwarding
POP wrapper is fully removed. No clock or public interface changes are present.
This executor review and static evidence do not substitute for final runtime
qualification or the later coordinator review of pushed immutable changes.

## Final Shared-Caller Inventory

The fresh sweep uses rg for _s_test_ss_push, _s_test_ss_frame,
_s_test_stack_frame_16/_32, _kec_pop and _e_push in cpu_instructions.c,
followed by inspection of the enclosing declarations and transfer bodies.
Declaration macros such as _______todo must not be misattributed to the prior
static function by a simplistic owner-name scanner.

| Actual caller set | S6 disposition and retained receiver |
| --- | --- |
| Nine _s_test_ss_push call sites: _kec_push; two _kec_call_far; two _ser_int_real; one each _ser_call_far_call_gate, _ser_call_far_call_gate_32, _ser_int_protected_16 and _ser_int_protected_32_same | The early-family underflow guard change is exercised by ordinary PUSH and near/far CALL/INT wrapping. Protected-capable guards are unchanged; their complete gate/delivery priorities stay S9/S11-S13. Full receiving regression remains pending. |
| Three _s_test_ss_frame callers: PUSHA, POPA and ENTER | One logical-only ordinary-frame admission owner, followed by original ordered scalar transfers. Independent invalid/valid, attribute, allocation, overlap and selective-provider matrices supply direct owner proof. |
| Four inner-gate probes: 16/32-bit CALL and 16/32-bit outer INT | Existing translated destination-stack admission remains; its different privilege/page contract is not replaced with the ordinary logical helper. Complete gate/task/page qualification remains S11-S13. |
| Fifty-two removed _e_pop forwarding calls: 24 GPR, 16 POPA, seven POPF, two 8F, two LEAVE and one POP CS | Direct calls retain exactly the existing _kec_pop reader, increment and SP/ESP alias guard. Existing direct RET/IRET scalar calls are unchanged; FLAGS and return semantics remain S7/S11. |
| Remaining _e_push callers: ordinary register/immediate/memory/segment PUSH, PUSHA, ENTER and PUSHF | The explicit SP=1 selector admits only the manual PUSH opcode families. Compound PUSHA/ENTER and PUSHF are excluded; implicit CALL/INT use _kec_push, not a duplicated instruction classifier. PUSHF privilege/FLAGS coverage remains S7. |

This inventory closes the source-hit accounting, not the still-pending full
runtime qualification. No unchanged gate/return path is advertised as fully
qualified merely because the forwarding POP owner is gone.

Both original complete default build handles 58215/52811 now terminate with
exit 0. Serial receiving handle 51803 remains live. Waiting orchestrator 1058
is retired without stopping any native builder; it has not launched units.
Default explicit 0546 product and full specialized/artifact/INI gate builds
run in the now-idle caches under handle 88217. Replacement orchestrator 1070
waits for both 51803 and 88217 to succeed before launching unchanged sequential
complete units, preventing unit/build resource overlap. No production input,
test budget or coverage universe is changed by this scheduling correction.

## Whole-Batch Implementation-To-Proof Map

The initial seven-member batch is preserved in full. The following maps its
current implementation to direct owner proof; final full-suite, receiving
and artifact qualification remains pending, so these are not accepted exits.

| Member | Final implementation and direct regression owner |
| --- | --- |
| Scalar PUSH/POP | Existing scalar transfer/publication and generation-specific PUSH SP rules remain. gpr_push_pop retains all-register/memory cases, adds 8088 to the original three family matrices, eighteen early-wrap controls and ten ordinary modern shutdown contexts. |
| PUSHA/PUSHAD | Original saved SP/ESP and register order remain; logical whole-frame admission and sourced initial odd-SP selection precede effects. pusha_popa covers invalid/valid provider priority, all eight rejected slots and real/VM starting-SP selection. |
| POPA/POPAD | Same frame owner admits the complete read span, and the discarded-SP slot still performs its scalar read. The per-slot rejection matrix includes that slot and requires original CPU restoration without changing input memory. |
| ENTER | Operand-selected chain addresses, full ESP frame snapshot, projected segment spans and protected allocation admission use one frame owner. enter_leave supplies four crossed attributes, 1,421 ordered success/rejection contexts, 420 allocation contexts and 24 overlapping-chain contexts. |
| LEAVE | SS.B selects frame-to-stack assignment; OperandSize selects pop width. Logical read admission owns cached limits and permissions. Existing enter_leave width/failure cases and gpr_push_pop real FFFF-word controls cover the corrected probe/error boundary. |
| POP alias/destination | Existing scalar SP/ESP alias predicate and post-pop memory EA ordering are unchanged. Seventy gpr_push_pop eligible mode/width/form/failure contexts prove them without a second register or destination owner. |
| Shared validation/publication | The nine scalar-admission sites, three ordinary frame callers, four retained gate probes and 52 removed forwarding calls are accounted above. Twenty-seven early near/far CALL/INT controls cover the changed wrap guard; full gates/units still must pass. Full FLAGS, gate/task/page/delivery semantics keep their original named receivers. |

Source pages, pre-fix failures, corrected independent oracles and effect
observations above are the evidence for these dispositions. Passing boot or
the narrow source guard alone cannot substitute for this member-level proof.

Default explicit product/gate handle 88217 now terminates with exit 0: both
0546 products and both complete verify-current-specialized-gates,
verify-product-artifact-roots and verify-t533-integration-ini-boundary targets
succeed. Per-width final gate logs are retained under the S6 qualification
directory. Fresh CTest -N inventories independently show 541 unit entries for
each width; these inventory results are not runtime passes. Serial receiving
51803 remains live, so full unit execution has not yet started.

After default gates succeed, verbose waiting cell 1070 is retired without
touching native receiving handle 51803. Quiet waiting cell 1084 retains the
same prerequisite and launches the identical complete unit command only on
receiving exit 0. This changes polling output only, not build execution,
test scheduling constraints or acceptance evidence.

Fresh transitive inspection confirms _m_test_logical delegates to
_kma_test_logical, which performs legacy logical-span splitting and
_kma_linear_logical admission only. In contrast, _m_test_access reaches
_kma_test_access and its additional _kma_test_linear translation step.
Thus ordinary frame admission is genuinely logical-only, not merely named
that way or accepted by the narrow helper-shape guard. Legacy wrap has one
existing span owner; actual scalar transfers still own provider/page effects.

## Receiving Build And Artifact Identity

All original receiving builds now terminate successfully; waiting cell 1084
returns receiving exit 0 and launches sequential complete unit handle 54309.
No owned build remains active during these units. All eight explicit 0546
products are present at their existing asset roots; final runtime acceptance
still requires the pending units, supplemental and original integration suites.

Direct PE checks pass 8664/014C and the expected optional-header magic for all
eight files. Each contains 0.5.0546 and objdump section inspection finds no
.debug/.zdebug/.stab section. Same-width CPU objects are identical in all four
caches: x64 A0E82107289DA5A322C1CECE90274016F64D5D2BA383275E57C188340AADED66;
x86 2DA9821852909A6D3F0DF4325CC05624F56D59FCBEC0AC5B9BECDF18B2D05982.
This proves the receiving builds share one frozen CPU implementation rather
than a per-App compile-time repair. Inspection output is artifacts.log in the
ignored final qualification directory.

| Product | SHA-256 |
| --- | --- |
| nxvm_default_0_5_0546_x64.exe | 1D31893B09A5F679DB5A65935AC931984F421DB7C8454A80D176E97881EF664A |
| nxvm_default_0_5_0546_x86.exe | F9EB4B628E5E4E469AE38E1B7DA908DA94472C9BD09CA7818212D0C5584F639D |
| nxvm_at_0_5_0546_x64.exe | 1AD2D8742BA41C93894ABE5A20C415345D5E8B9D25C2BA85C841385B4DBE61A1 |
| nxvm_at_0_5_0546_x86.exe | 64EEC9275299E4A289E912C9633D5ADBE87A122CA2C16840C15755CDF5ED7BF5 |
| nxvm_xt_0_5_0546_x64.exe | 53E8EB3B189C75E76AF9004F67032491383C9D538EF9EA6C4C0DADA598AB98AE |
| nxvm_xt_0_5_0546_x86.exe | 6BED2A3CE452668ACD8806F637E6339E6497AE54E766A451F0F96F45E4E74C20 |
| nxvm_model40_0_5_0546_x64.exe | BA57CF396222AA4E40D6CA6F3D5406E198FCF36AF7E91235F506DE2F7BA6961A |
| nxvm_model40_0_5_0546_x86.exe | A9E7C81390279CA940F65465015D5D07E9CA273376F0117CEE69BD979B12AB8F |

All four owner INI hashes and the exact MyNES 0043 pair still match S5.
Repeat unchanged-master/artifact checks after integration before delivery.

## First Full-Unit Containment And Diagnosis

The first quiescent x64 aggregate terminates at its unchanged 300-second
deadline after 193/541 passing entries. No individual Failed/Timeout entry is
reported, and the sequential x86 route never starts. This is incomplete
verification, not a pass or evidence that the unrun entries are correct.
x64-unit.log and x64-unit-error.log retain stdout and the real deadline error.
CTest wrote a fresh LastTest.log.tmpa6b86 rather than LastTest.log.tmp; that
fresh partial detail is preserved separately as x64-unit-incomplete-detail.log.
The old completed LastTest.log and old LastTestsFailed.log are not adopted.

Read-only direct launch comparisons pass an unchanged Lib frame-copy test
in 0.0468 s, the chip MOV test in 0.1569 s and the Core MOV test in 1.7359 s.
The initial diagnostic looked for corpus executables at the cache root;
that path lookup failed before execution and was corrected to their actual
test/lib and test/x86 locations. It is not counted as a runtime result.
The first aggregate's generally multi-second case timings are not explained
by an observed instruction failure; the throughput cause remains unproven.
No antivirus, mount, unrelated process, source, artifact or budget is changed.

Original full-unit handle 54309 is terminal with exit 1. The unchanged complete
300-second/four-job route is launched again as handle 4394, with distinct
*-unit-quiescent logs. Fresh detail capture now accepts the actual timestamped
LastTest.log* file, never an older summary. A retry cannot combine partial
passes into acceptance: each width still requires a complete 541/541 result.

## Complete Unit Failure Batch And Original-Source Reconciliation

Unchanged x64 rerun completes all 541 in 193.45 s: 539 pass and exactly two
fail, unit.core-machine-enter-leave-smoke and x86.cpu_address_span. Handle
4394 terminates with exit 1; sequential x86 again does not start. The full
stdout/error/fresh-detail files remain separate from the first containment.

Fresh original 386 PDF 284/14-6 explicitly assigns real-address range crossing
to GP for data segments and SS when SS addresses the segment. This is the
more specific segment rule underlying S3, not a guessed emulator value. S6's
global extension of the 286 GP rule to 386 was overbroad; restore that owner
predicate. Dictionary POP/LEAVE's terse generic exception-13 clause does not
erase the explicit SS distinction. Correct the new POP/LEAVE oracle to GP
for 286 and SS for 386; retain PUSHA's separately explicit initial-SP requests
and ordinary PUSH's explicit shutdown admission. No numeric timing row or
source grade changes, and the earlier claimed GP reconciliation is superseded.

Core ENTER still expects the earlier forbidden partial frame writes on a
logical segment rejection. Apply the same independently initialized full-frame
assertion already sourced in the chip test. Sweep its Core PUSHA receiver too:
its old sentinels mirror prospective register writes and can mask the defect;
replace them with independent values without weakening any CPU/fault assertion.
This complete in-scope source/receiver correction precedes a new source freeze,
full dual rebuild/units/gates, all eight artifacts and original integration.
The EAEBFB06… object/product identities above are superseded diagnostic builds,
not final S6 qualification. No partial P or source rollback-by-test is allowed.

The corrected failure batch passes both widths, 4/4 in 1.28 s/4.59 s.
The strengthened Core frame read initially used memory_read in the faulted
lifecycle and correctly failed its state contract; use the existing
memory_inspect route, as the receiver's other readback already does. This
observes actual bytes without altering lifecycle or adding an API. The new
real-range regression installs the corresponding GP/SS vector as well as
checking the family-specific mask. No production change beyond restoring
the sourced 386 SS predicate is required.

The earlier direct Core MOV timing probe used a retired cache-root binary,
not its registered test/ibmpc command. It is not current-source performance
proof; the actual generated CTest command remains authoritative. Direct Lib
and chip probes used their actual current corpus paths. No old root executable
or old log may substitute for final tests.

Production freezes anew at
A6EC7E73027619E3D52B435757E03E25DB101A25D00DC3B0AB49CE0E546B0768.
Default CPU archives and corrected receivers compile successfully on both
widths. Refresh the remaining consumers using the existing generated fast
recipes: read CPU-archive dependencies from build.make, intersect with the
current top-level /fast targets, build the sole CPU archive first, then all
current consumers. Default inventory contains 407 consumers including the
explicit 0546 product and integration harness. This avoids repeated dependency
traversal, not compilation/linking or test coverage. No build description or
registered case changes; all final 541 units, gates, eight products and original
58 integration contexts remain required after the refreshed builds cease.

Final full CPU-consumer refresh runs under handle 76023; six receiving
product/harness leaf refreshes run serially under 17665. Their MyNES exclusion
uses the actual REPOSITORY_BUILD_MYNES cache key; an initial NXVM_BUILD_MYNES
key lookup rejected before any build and is not an execution result.
Static handle 5301 terminates successfully: all eight manifests, stack owner,
test/x86 plus test/ibmpc Types and documentation checks pass. Quiet cell 1130
waits for both refreshed builds to succeed, then launches complete dual gates
and the unchanged sequential full unit routes, preserving every prior failure
and the 300-second/four-job boundary in separate final logs.

Final counted source/test/build surface is ten paths: cpu_instructions.c,
three chip test bodies, two Core receiver bodies, test/x86/CMakeLists.txt,
the shared CPU-local fixture and two stack guards. git diff --numstat plus
the three untracked-file line counts gives +967/-141, net +826; production
alone +138/-119, net +19. Manifests, documentation, ignored outputs and EXEs
are excluded. The Core assertion correction removes more than it adds; the
positive total supplies the missing independent matrices rather than a new
production layer. Final actual-source review confirms no public API, timing
constant, Lib/Common/MyNES source or second executor is changed.

The six other-profile final receiving builds finish with exit 0. Default
consumer refresh remains live. Fresh generated-Makefile inspection finds
.NOTPARALLEL at the top level: multiple /fast goals are serial despite -j4.
This explains a concrete scheduling limitation, not a CPU execution defect.
Keep the current native refresh running; a later refresh can dispatch distinct
generated leaves concurrently only after the CPU archive is complete and
their output ownership is checked. No Makefile, timeout or test is altered.

## Final Complete Units, Gates And Refreshed Products

Full consumer refresh 76023 and six receiving refreshes 17665 terminate with
exit 0. Final verification handle 3833 also terminates with exit 0: both complete
specialized/artifact/INI gate aggregates pass; final complete units pass 541/541
on x64 in 248.83 s and x86 in 233.31 s under the unchanged 300-second/four-job
route. Final stdout and fresh details use *-unit-final logs; no earlier partial,
failed, direct or stale-cache result is merged into these complete passes.

Read-only inspection now verifies all eight final A6EC7E73… products: PE
8664/014C, 0.5.0546 and no .debug/.zdebug/.stab sections. Same-width CPU objects
match across all four caches: x64
528A50BB1FBADBFE0A15F8ECAA310F88042D44CADAABD1054C2234E14B2B2C65;
x86 58B4B98820F45085DF6CBA3303BDAB55701B209EB21A43381D53D34E1E5D140D.
The eight hashes below supersede the earlier diagnostic artifact table.
Inspection is artifacts-final.log; an initial shell-expression syntax error
stopped before inspection and was corrected, not counted as a result.

| Final product | SHA-256 |
| --- | --- |
| nxvm_default_0_5_0546_x64.exe | 72038B4FB4CA3F560C1FE929C64CC7BB53690B823956E8E0869567844FBDF1FF |
| nxvm_at_0_5_0546_x64.exe | C230D2AE7C545F17401EB0777B1FBC435C0EC9B8947E7FB5E2895863B85C03C6 |
| nxvm_xt_0_5_0546_x64.exe | A51046718CD54AC2EE28086250EFC302B1CEBD7090AA4FFE4315663691260974 |
| nxvm_model40_0_5_0546_x64.exe | ADAA4DFBD3794897E297369972FE51DA7E52BB6E43B30CA66479699EE102D1E4 |
| nxvm_default_0_5_0546_x86.exe | 3704F1CA72CD71815CF860974A16E6C6860288F391965A32C88FC98E4E26C649 |
| nxvm_at_0_5_0546_x86.exe | AAA20EEE073665104F3ADA24B699C528326F461F9664C9A1B9781D126BE9A2EC |
| nxvm_xt_0_5_0546_x86.exe | 0B4B2E788339AC1B9B19FEEF3D4341C438D2C2B33DBC02A1CFB917D26F35E1BE |
| nxvm_model40_0_5_0546_x86.exe | 2DBC296C62D23181F19A5A09214D4C6BD09BA95DBFFB9082A1C233A41488D4B9 |

Original receiving integration starts once per final group under 34321,
followed by both supplemental suites. No checkpoint, input, budget, source,
public API or numeric timing is changed. No P or S6 closure is claimed while
that qualification remains incomplete.

## Complete Receiving Qualification And Executor Self-review

Final receiving/supplemental handle 34321 terminates with exit 0. Every original
integration context passes once with its unchanged checkpoint and 300-second
containment; both supplemental routes pass 33/33. No new failure requires a
second integration group. Detailed logs remain in the ignored S6 directory.

| Final group | x64 | x86 |
| --- | --- | --- |
| Default | 22/22, 52.67 s | 22/22, 61.63 s |
| AT | 3/3, 33.76 s | 3/3, 40.52 s |
| XT | 1/1, 17.21 s | 1/1, 23.51 s |
| Model40 | 3/3, 106.17 s | 3/3, 147.53 s |
| Supplemental | 33/33, 207.60 s | 33/33, 200.09 s |

Fresh post-integration hashes match all five external masters and the exact
MyNES pair recorded by S5. All four owner INIs and all eight final product
hashes remain unchanged. No CTest/CMake/Make process remains live. Fresh eight
manifests, final diff check and documentation governance pass. Lib/Common and
MyNES source/test/artifact diffs remain empty; unrelated MyNES documentation
is preserved outside this delivery.

Executor actual-diff self-review exhausts the seven admitted members and
shared-caller inventory against original sources and the complete runtime
proof above. Early whole-frame logical admission remains distinct from actual
scalar page/provider effects; original register tables/aliases and accepted
earlier writes remain. Full ESP snapshot, independent chain/address attributes,
early wrap and sourced explicit shutdown use the existing owners. The corrected
386 SS rule follows original 14-6, not the misleading candidate interpretation.
Independent Core/chip frame oracles retain all state/fault checks. There is no
new public API, timing value, mirrored owner, general undo path or source-grade
downgrade. Existing gate/task/page/delivery/FLAGS/NPX/retirement qualifications
remain their original S7-S17 receivers and are not inferred from S6 passes.

All S6 executor requirements are proven for target-separated implementation
delivery. This is not coordinator acceptance: pushed immutable Shared/NXVM
changes must still receive actual-change review and a pure governance P.
The ignored receiving caches/research/final failure and success logs remain
needed for that review and the immediate following CPU batch; no unrelated
workspace or protected input is removed.

Shared implementation P1 is 1db282a102f869a026d60329b24a56a7ede2a476,
immediately pushed to origin/master. Its thirteen paths contain only the CPU,
ten counted code/test/build paths and three matching manifests. The final
CPU/object/artifact proof above corresponds to that exact source revision.
NXVM P2 separately delivers this complete evidence, scope/status/history and
the eight verified PC products; no excluded source or configuration is added.
