# T544 S5 80286 Family Audit

## Status And Scope

Read-only audit against ebdb40098. The complete finite inventory below has
source/code/regression dispositions; verification and coordinator acceptance
are recorded at delivery. No Shared repair or CPU qualification is claimed.
Current owns admission. This batch consumes F12
and inherited F01-F11/F14; it is not just a list of newly introduced opcodes.
S2-S4 cross-family receivers stay pending inside T544.

Primary source selection is Intel 80286/80287 Programmer's Reference Manual
(1987), order 210498-005, with its Appendix B clocks and mode/exception rules.
Original hardware source supplies the bus boundary, not a replacement
instruction table. Fresh identity and visually inspected pages are recorded
as each source batch is consumed; prior citation alone is not a reading claim.

Fresh SHA-256 of the selected 1987 programmer original is
AD487BA99B48CD9F61B14C0FE912A04C7CDB4C7C14A18419AA9FAF62D8962460,
matching the [T512 source record](t512-s1-five-cpu-source-cross-validation.md).
PDF 229 / printed B-21 was rendered and visually inspected for the first
ARPL reconciliation below. Extracted text is navigation only. The original
and rendered scratch remain external/ignored, not repository assets.

## Finite Batch

| Partition | Required contexts | Disposition / owner |
| --- | --- | --- |
| Admission and state | Real/protected, unsupported later encodings, prefix/length, reset, FLAGS image/load and PE transitions | Pending CPU admission/state audit; retain S2/S3/S4 receivers. |
| Descriptor queries | ARPL, LAR, LSL, VERR, VERW; selector null/bounds/type/present, CPL/RPL/DPL, register/memory, ZF and failed fetch/read/write | Pending sole CPU descriptor/query owner and existing query fixtures. |
| System registers/tables | SGDT/SIDT/LGDT/LIDT, SLDT/STR/LLDT/LTR, SMSW/LMSW/CLTS; mode/privilege, six-byte images and task busy publication | Pending system-state validation/publication owner. |
| Segment/address access | MOV/POP/LDS/LES, null and conforming selectors, limits, access checks, expand-down, logical/wrap and failed descriptor/data access | Pending segment/memory owner plus inherited effective-offset receiver. |
| Control/gates | Near/far CALL/JMP/RET/IRET, direct/call/interrupt/trap/task gate and privilege stack transitions | Pending sole control-transfer/delivery owner; frame ordering and partial effects retained. |
| Tasks | 16-bit TSS load/save, backlink/busy/NT, task gate/call/jump/IRET, descriptor failures and publication | Pending task owner; cross-width paths cannot qualify 286 by association. |
| Exceptions/asynchronous delivery | Fault/trap/abort, error codes, double fault/shutdown, SS/NMI/debug arbitration and return location | Pending S2 generation-qualified delivery receiver. |
| Inherited ordinary forms | F01-F07/F10/F11 arithmetic, movement, stack, count, multiply/divide and branches | Pending 286 function/clock/source reconciliation; do not inherit early/186 numbers or boundaries without authority. |
| Strings/external cycles | F08/F09/F14 REP, ports, WAIT/ESC/HLT/LOCK, external effects and restart/time publication | Pending repeat/external-cycle owner and S2-S4 retirement/failure receivers. |
| Timing qualification | Appendix B forms and assumptions, real/protected formulas, modifiers, timing attribution and independent regression predicates | Pending sole timing owner; catalog has 771 reported L3 keys, not 771 independent source proofs. |

Later-only F13, 32-bit operands/addressing, paging and VM86 require explicit
source absence/admission proof, not advertised 286 capability. Every unresolved
member receives a named context/owner before audit delivery; green tests cannot
substitute for that mapping.

## First ARPL Source / Handler / Regression Reconciliation

Original B-21 defines a word destination and word-register source. If the
destination RPL is lower, its bottom two bits become the source RPL and ZF
is set; otherwise the operand stays unchanged and ZF is cleared. No other
FLAGS are modified/undefined. Real mode raises type six. Base clocks are
ten for register and eleven for memory, exact source values rather than a
range midpoint. The page also specifies protected memory address/permission
exceptions; it is not a full failed-transfer publication contract by itself.

ARPL_RM16_R16 admits 286 and later only in protected mode. It checks ModR/M
and word operand read, compares the two RPL fields and writes the adjusted
word through the checked common operand owner only when needed. Otherwise
it clears ZF without a write. This normal operation agrees with the inspected
source. The real-mode path invokes UndefinedOpcode; whole type-six delivery
and rejected write restoration still belong to the S2 delivery/publication
receiver, not proof from that call alone. The source-qualified behavior of
a read-only destination when no adjustment is needed remains an explicit
permission/transfer context; no speculative unconditional write is proposed.

The actual primary 286 timing branch selects memory eleven/register ten for
unprefixed protected ARPL. The existing manifest's I286-ARPL register recipe
expects ten. That is a reconciled base value, not full prefixed/memory/failed
retirement proof. The similarly named system-cost helper first rejects every
primary opcode except 0F, so its later switch arm for 63 cannot be reached.
Do not cite that dead arm as the production evidence for prefixed ARPL.
The independent 386 system helper's 20/21 values are not a 286 timing defect;
all dispatch, modifier and attribution contexts still require reconciliation.

cpu_arpl_smoke has an actual three-form 286 check: adjusting register,
unchanged register and ES-prefixed memory adjustment. Its all-register-pair,
other-FLAGS, extensive memory/attribute and protected-limit tests select 386.
Real-mode rejection does include 286, but intentionally forces terminal
delivery through the fixture and does not prove a normally installed type-six
frame. Therefore retain the 286 full RPL-pair, FLAGS/nonparticipant, segment,
permission, alias and selectively rejected access matrix under the shared
query/operand/delivery receiver. No normal 286 ARPL path is missing merely
because those broader regression contexts are unqualified.

The existing x86.cpu_arpl test passes once each host width. This is selected
baseline behavior evidence only; the complete S5 unit gate, other partitions
and CPU qualification remain outstanding. Source/test/artifact changes are zero.

## Descriptor Query Source / Handler / Regression Reconciliation

Original PDF 268 / B-60 (LAR), 279 / B-71 (LSL), 319-320 /
B-111-B-112 (VERR/VERW), and 189-190 / 11-3-11-4 were rendered and
visually inspected. Chapter 11 supplies the privilege interpretation:
conforming code is not rejected merely because CPL or RPL exceeds DPL;
other visible segments require DPL at least both CPL and RPL. VERW requires
writable data and never accepts code. LSL returns limits of segments, TSS
and LDT, not gates. These queries are not legal real-mode instructions.
The Appendix B base clocks are fourteen for register and sixteen for memory;
modifier, failed delivery and attribution contexts remain in the timing batch.

The inspected source lists query bounds/type/privilege conditions without a
Present-bit condition. LAR returns the access byte for subsequent software
inspection. In contrast, LAR_R32_RM32 and LSL_R32_RM32 reject !Present before
their type/privilege checks, and INS_0F_00 applies the same rejection to both
VERR and VERW. This is one four-query source/implementation discrepancy,
not four unrelated BIOS defects. The VERR Appendix wording comparing validation
with DS/ES access must be reconciled with its explicit condition list and
Chapter 11, rather than silently adding every segment-load check. Retain
Present/non-Present paired descriptor cases and the source resolution under
the sole descriptor-query owner before proposing Shared edits.

The shared selector helper rejects null selectors and entries whose full
eight bytes exceed GDTR/LDTR limit. Descriptor reads use the checked table
access path. Normal query rejection clears ZF; successful LAR/LSL writes only
the destination, while VERR/VERW changes only ZF. Normal helper use is not proof
of every rejected operand/descriptor access or normally installed fault frame.
Those contexts stay with the checked operand and S2 delivery receivers.

There is also a generation-boundary lead: LAR's system whitelist includes
32-bit TSS and call gates, LSL includes 32-bit TSS, and both apply these lists
without a 286 guard. LSL computes a shared extended/granular segment limit;
cpu.h decodes upper limit bits and granularity without knowing the profile.
The 286 descriptor layout and reserved type/word rules must be visually
reconciled before specifying behavior for reserved inputs. Sweep segment/TSS/
LDT loads and other consumers of these macros, not only LSL's result.

Existing LAR/LSL register cases run both 286 and 386, but explicitly expect
LAR to reject the non-Present data descriptor at selector 18h. The VERR/VERW
matrix makes the same expectation for that selector on both profiles. These
tests encode the implementation assumption, not an independent manual oracle.
The LSL fixture additionally supplies C0h in descriptor byte six on both
profiles, with FFFFh as the word expectation; that coincidentally truncated
result cannot distinguish a 286 limit from a 386 granular limit. Broad memory,
prefix, LDT and fault contexts in these tests predominantly select 386.
Retain a source-qualified 286 type/Present/CPL/RPL/DPL matrix, all non-ZF FLAGS,
unchanged destination on rejection and selectively failed access cases.

Existing query regressions pass once each receiving width in this S5 run:
x64 two cases in 0.45s, x86 two cases in 0.56s. This proves their current
predicates execute successfully, not that the disputed predicates are correct.
The full S5 inventory and complete unit gate remain outstanding; no Shared
source, test, ABI or deployed artifact is changed.

## Descriptor Layout And Generation-Boundary Sweep

Original PDF 115-116 / printed 6-5-6-6, Figures 6-3 and 6-4, were rendered
and visually inspected. They define a 24-bit base and 16-bit limit for 286
segments/LDT/TSS, with bytes six and seven reserved and required to be zero.
System type zero/eight is invalid; types nine through F are reserved. This
does not license inventing a precise result or exception for every nonzero
reserved input. Valid zero-reserved layouts and reserved-input uncertainty
must have separate dispositions.

The table read chain _s_read_xdt -> _ksa_read_xdt -> _ksa_read_gdt/ldt reads
the complete eight-byte descriptor through the checked memory path, without
a generation-specific descriptor normalization there. The following consumers
then interpret the common extended base/limit/size macros:

- _ksa_prepare_code_sreg and _ksa_prepare_stack_sreg prepare control-transfer
  caches, including default operand size or stack size;
- _ksa_load_sreg applies the same decoding to CS, data segments, SS, TR and
  LDTR, and publishes accessed/busy descriptor writes before its live cache;
- _s_task_cache_descriptor populates task-switch CS/data/stack/TR/LDTR
  caches from those macros and has no profile argument;
- LAR/LSL use the shared system whitelist, with LSL also using the extended
  limit/granularity formula already identified above.

With the upper reserved word zero, the shared base/limit arithmetic reduces
to the documented 286 layout. Therefore this sweep does not assert that every
valid 286 descriptor is misdecoded. The existing query fixture's C0h upper
attribute byte, however, is not a valid documented 286 input and cannot qualify
its limit behavior. A receiver must distinguish lawful 286 inputs, later-only
system types and explicitly underdetermined reserved-word behavior.

LTR has a concrete related acceptance path: INS_0F_00 calls _s_load_tr,
which checks CPL and TI, then _s_load_sreg/_ksa_load_sreg. Its available-TSS
predicate _IsDescTSSAvl masks type with seven, accepting both type one and
type nine without a CPU-profile check. It then sets busy and publishes the
descriptor/cache. Type nine is reserved in the inspected 286 table, whereas
it names a 32-bit TSS in the common definitions. Retain this with LAR/LSL
type admission and all task/gate consumers under one generation-qualified
descriptor validation/publication repair batch. Exact reserved-type failure
semantics still require the relevant LTR/transfer source paragraphs; no
Shared patch or hardware-result claim follows from the mask alone.

The owner-local repair design to review after source reconciliation is a
single generation-aware descriptor interpretation/type policy consumed by
queries, loads and transfers. It must preserve raw descriptor bytes for the
existing accessed/busy writes, separate query visibility from segment-load
Present requirements, and avoid adding a second cache or per-board exception.
Its tests need paired 286/386 lawful layouts plus source-qualified unsupported
types and ordered failed descriptor/cache publication. This remains a proposed
receiver, not an approved implementation or completed qualification.

## System Register / Table Instruction Reconciliation

Original Appendix B pages were rendered and visually inspected: PDF 239 /
B-31 CLTS; 273 / B-65 LGDT/LIDT; 274 / B-66 LLDT; 275 / B-67 LMSW;
280 / B-72 LTR; 309 / B-101 SGDT/SIDT; 310 / B-102 SLDT; 311 /
B-103 SMSW; 316 / B-108 STR. They give exact base clocks: CLTS two;
LGDT/SGDT eleven; LIDT/SIDT twelve; LLDT/LTR seventeen register/nineteen
memory; LMSW three register/six memory; SLDT/STR/SMSW two register/three
memory. These are manual L3 bases, not proof of every effective-address,
prefix or failed-retirement timing modifier.

SLDT/STR/LLDT/LTR reject real mode. SLDT/STR have no CPL-zero restriction;
LLDT/LTR require CPL zero and GDT selectors. LLDT accepts a null selector
and invalidates LDTR without altering DS/ES/SS/CS caches or the TSS LDT
field. Non-LDT is GP(selector), non-Present LDT is NP(selector). LTR marks
an available TSS busy without a task switch; non-TSS or already-busy is
GP(selector), non-Present TSS is NP(selector). The inspected LTR source and
286 system-type table together resolve the earlier type-nine acceptance
lead: it is not a documented 286 TSS and must not be advertised as valid
286 LTR. Operand-address faults and ordinary installed delivery remain
separate checked-memory/S2 contexts.

INS_0F_00 implements the protected-only admission and word selector stores.
_s_load_ldtr/_s_load_tr enforce CPL and TI before the descriptor load helper;
the handler currently decodes/reads a memory operand before reaching those
privilege checks. The precedence of simultaneously illegal privilege and
operand access needs the protection-priority authority, not inference from
the fact that each error is individually listed. _ksa_load_sreg validates
type/Present, writes the TSS busy descriptor through the checked path before
publishing TR, and handles null LLDT by clearing flagValid/selector. Its
type-nine acceptance remains the generation-policy repair above. Selective
busy-write failure, ordered partial memory effects and stale-cache visibility
need their own independent regression predicates.

Null LLDT also exposes a query/load distinction: _s_check_selector checks
null selector and table limits, but not LDTR validity. _ksa_load_sreg leaves
old LDTR base/limit storage when invalidating it. A TI query within that old
limit can reach _ksa_read_ldt and _kma_linear_logical, which raises GP(0)
for invalid LDTR. B-66 expressly excludes LAR/LSL/VERR/VERW from the ordinary
invalid-LDT fault rule. Retain the LLDT-null followed by all four TI-query
contexts under the same query-validation receiver; do not repair ordinary
segment loads by suppressing their proper invalid-LDT faults.

LGDT/LIDT use six memory bytes: a word limit, a 24-bit base, and an ignored
last byte. Register forms are UD; protected CPL other than zero is GP(0).
The code checks protected privilege before memory decode, rejects register
forms, reads word plus dword into local values and only then publishes the
table. For 286 the operand-size-two base mask yields the documented 24 bits.
SGDT/SIDT are memory-only, unprivileged and valid in both modes. The sole
store helper preflights the complete six-byte write and then writes one image.
This is the relevant owner, not a guarantee that any provider failure is
atomic. Boundary/wrap/provider failures remain checked-memory receivers.

B-101 calls the sixth SGDT/SIDT byte undefined. The helper selects FFh on
286 and its comment claims a physical-software compatibility observation.
Retain that deterministic choice pending an independently identified behavior
source/model; neither the comment nor the original's undefined byte proves
FFh as Manual-L3. The first five specified bytes and the instruction clocks
remain separate source-defined facts; no downgrade/removal is implemented.

SMSW stores the low word; _s_load_cr0_msw changes the low four bits and
preserves an already-set PE, matching LMSW's inability to leave protected
mode. CLTS changes only TS and checks protected CPL before that change.
The source requires a following jump on LMSW protected-mode entry; the
prefetch/transition and MSW reserved-bit image are retained under the existing
admission/state receivers rather than qualified by low-four-bit code alone.

Complete actual regression reads reveal three different coverage boundaries.
cpu_descriptor_system_smoke's layout, load/store and rollback contexts all
select 386; its existence is not direct 286 proof. cpu_dttr_s61_smoke does
select both profiles, but its GDT uses type-nine TSS and explicitly requires
32-bit busy type/8Bh on 286 too. This is a contradicted oracle for the same
production admission defect, not a valid 286 success test. Its null-LLDT
memory/attribute checks select only 386 and do not follow invalidation with
the four TI queries. cpu_control_state_smoke has 286 register CLTS/MSW and
CPL-zero PE-retention checks, while its broader privilege and memory cases
select 386. Its memory tests inside the two-profile MSW loop also explicitly
prepare 386 on each iteration, so that loop does not multiply 286 coverage.

The 286 system timing helper uses the inspected exact base values for these
normal admitted forms, then adds memory_ea where applicable. Whole modifier
and attribution proof remains in the timing partition; normal values do not
qualify an illegally accepted type-nine LTR. Existing system/control tests
execute successfully once each width in this run: three cases x64 0.13s,
x86 0.15s. Their source-disputed oracle and missing contexts remain open.
No source/test/ABI/artifact change is made and S5 is not delivered or closed.

## Segment-load and address-boundary reconciliation

Original Appendix B pages B-61/B-62, B-73/B-74 and B-83/B-84 were visually
inspected for LDS/LES, MOV segment registers and POP segment registers.
Their exact normal clocks are respectively 7/21 (real/protected), MOV
segment loads 2/17 register and 5/19 memory, and POP segment loads 5/20.
These source values do not by themselves qualify the implementation timing
modifiers or failure paths, which remain in the timing partition.

The source permits null DS/ES loads but requires GP(0), without a memory
reference, when that invalid cache is subsequently used. Non-null DS/ES
loads require data or readable code, the applicable CPL/RPL/DPL checks,
and Present; failed Present is NP(selector). SS additionally requires a
non-null selector, matching CPL/RPL/DPL and writable data, with absent
Present reported as SS(selector). POP SS inhibits interrupts including
NMI through the following instruction. MOV CS is not an admitted form.

The production paths converge on _s_load_sreg/_ksa_load_sreg. MOV rejects
CS, reads its operand through checked helpers, and sets flagMaskInt only
after a successful SS load. LDS/LES read the offset and selector before
_e_load_far loads the segment and writes the destination register. POP uses
_e_pop_sreg: checked stack read, checked segment load, then stack-pointer
increment and successful-SS interrupt mask. ExecInt tests that mask for
both NMI and maskable IRQ. This establishes the shared mechanism and
publication order in code, not complete interrupt-shadow or provider-fault
qualification. Accessed-bit descriptor writes, error-code delivery, operand
boundaries and unchanged register/cache state on failure remain receivers.

A concrete real-mode generation-boundary discrepancy is visible in
_kma_linear_logical: its branch described as allowing 80386 32-bit data
offsets tests real mode, SREG_DATA and nonzero byte count, but does not test
the CPU profile. It therefore replaces the data-segment upper bound with
FFFFFFFFh for 80286 as well. _kma_real_legacy_segment_wrap excludes 286;
_kma_read_logical then uses that widened bound directly. In contrast,
B-74 and B-84 expressly require interrupt 13 for a word at FFFFh, and
B-62 requires it for a four-byte LDS/LES pointer at FFFDh or FFFFh.
For LDS/LES the two checked word reads at offset and offset+2 likewise
reach the common widened data path. This is a source/code contradiction,
not a newly executed behavioral probe or an approved Shared repair.
Retain one generation-aware real-data boundary receiver covering reads,
writes, preflight, segment overrides and far-pointer second-word access;
do not patch just LDS/LES or reject legal early-family wrapping globally.

The actual 286 segment-selector test batch was read for its eighteen
normal forms and four cache-load rejection contexts. It checks LDS/LES,
MOV/POP DS, null DS and delivered NP/GP with retained caches, but also
includes memory/register LTR against its type-nine TSS fixture. That is
the already identified contradicted 286 oracle, now confirmed in a second
test batch. Its successful execution would not qualify all eighteen forms.
The protected stack-limit contexts inspected in cpu_legacy_sreg_stack_smoke
explicitly select 386, not 286. The remaining segment suites and exhaustive
rights/expand-down boundaries are still being read; no complete segment
partition or fresh regression result is claimed here.

### Segment regression coverage follow-through

Complete reads of cpu_les_lds_smoke, cpu_les_lds_s41_smoke and
cpu_sreg_mov_smoke distinguish admitted real forms from protected coverage.
Both LDS/LES suites execute ordinary real forms on 286 and reject later
operand/address prefixes there; the S41 suite also checks register-source
UD on 286. Their protected null/type/Present/privilege and pointer-source
limit/failure cases explicitly prepare 386. Neither exercises a 286 real
pointer at FFFDh/FFFFh. MOV's real register/memory forms cover 286, but
its protected cache/accessed-bit, null, permission and operand-limit
matrix selects 386. The separate segment-selector batch supplies some
actual 286 protected cases, not that whole matrix.

The complete legacy segment-stack suite likewise covers ordinary real
PUSH/POP on 286 and rejects later prefixes, but its protected POP,
null/non-present/type/privilege and expand-down/normal stack-limit cases
select 386. Its IRQ fixture tests POP SS versus other segment operations
on 386 with a maskable provider IRQ; it does not inject NMI or select 286.
Retain explicit 286 MOV/POP SS shadows for both interrupt sources, failed
SS loads without a successful-load shadow, and delivered selector error
codes rather than treating terminal DF alone as their proof.

Original Chapter 11 section 11.2.2 and Figure 11-1 (PDF 188, printed 11-2)
were visually inspected: expand-down offsets are strictly above the limit;
limit zero yields 65535 bytes and FFFFh yields an empty segment.
_kma_linear_logical uses limit+1 through FFFFh for a non-big data/stack
cache and tests the whole byte span. This matches those legal 286 bounds
in code; the previously retained generation-aware cache-layout receiver
still matters because 286 cannot acquire a 386 big-segment interpretation
from a reserved descriptor word. Full DS/ES/SS rights, endpoint and
provider-access predicates remain outstanding, not proved by this formula.

All five existing segment suites execute once each width without failures:
x64 5/5 in 0.22s, x86 5/5 in 0.15s. These runs establish the current
regressions only; they neither exercise the uncovered 286 contexts nor
correct the type-nine oracle. The primary timing path separately supplies
LDS/LES's 7/21 mode distinction and MOV's mode/form bases; a lower helper's
fixed seven is not by itself evidence of the selected execution cost.
Continue tracing selector precedence and all modifiers in the timing
partition before reporting a timing contradiction. No Shared changes made.

## Far-control and call-gate reconciliation

Original CALL pages B-23 through B-25 (PDF 231-233), section 7.5.1.2
and Table 7-3 (PDF 144), section 7.5.1.4 (PDF 146), and Figure 8-1
(PDF 152) were visually inspected. CALL distinguishes direct code,
call gate, task gate and TSS destinations. Gate targets ignore the input
offset; the gate supplies CS:IP. Conforming target code retains the caller's
privilege, while a nonconforming target at a smaller DPL changes privilege
and selects the matching TSS stack. Figure 8-1 gives SP/SS pairs at
2/4, 6/8 and 10/12 for CPL 0, 1 and 2. The five-bit gate word count
permits zero through 31 copied parameters. Exact gate-call clocks distinguish
immediate and memory forms, zero versus nonzero parameters, and include
the next-executed-instruction byte term; those are timing receivers, not
qualified merely because a gate transfer runs.

_e_call_far dispatches descriptor classes to the existing shared helpers.
The inspected _ser_call_far_call_gate 16-bit route checks gate DPL/RPL
and Present, prepares local caches, preflights a new frame, reads parameters
and writes descriptors before publishing a new stack and pushing the frame.
_GetDescCall_Count masks five bits and the reverse push loop preserves
parameter order. Complete provider failure and simultaneous-fault priority
are not established by this normal ordering.

Three concrete gaps belong to a single gate-target/stack-selection receiver:
the 16-bit CALL gate route rejects every conforming target by requiring
_IsDescCodeNonConform, although section 7.5.1.2 expressly admits conforming
intra-level targets; it rejects TI on the target code selector and the new
stack selector, although the inspected checks require valid descriptor-table
selection rather than restricting these selectors to GDT; and it reads
SP/SS only at TSS offsets 2/4 for a 16-bit busy TSS, irrespective of
target_cpl. That is ring zero's pair, not the specified ring-one/two pairs.
The 32-bit-TSS compatibility branch likewise uses fixed offsets 4/8;
its generation admission belongs to the previously retained type-policy
receiver, not a reason to admit it as 286 behavior.

The similar-issue sweep finds TI/nonconforming-only restrictions in
_ser_jmp_far_call_gate and _ser_ret_far_outer too. JMP also accepts the
32-bit gate class without an explicit 286 profile check in this helper.
Retain generation, conforming and local-table predicates for CALL/JMP/RET
under one coherent validation repair; Appendix JMP/RET, downstream cache
preparation and complete tests still need reconciliation before that whole
partition can be delivered. Direct far-call stack-before-cache validation
and descriptor accessed writes additionally require fault-priority review.

The inspected far-control fixture explicitly prepares 386 for its protected
forms; its four-profile main loop covers real forms separately. The outer
return suite selects the 286 decoder for delivered-error contexts, but its
shared fixture installs a 32-bit TSS even for that profile. These cases are
not a valid whole-286 transition fixture; their same-level error delivery
can still exercise checks that do not consume TR. Complete gate regression
coverage is therefore not established by either suite.

Source precision also requires preserving a prose inconsistency: PDF 144
says EPL is compared with gate DPL but the following condition prints
EPL > CPL. Appendix B-24 independently lists gate DPL >= CPL and RPL.
Use the explicit checks to resolve that comparison, not OCR inference or
literal adoption of the inconsistent prose condition.

### JMP/RET and regression follow-through

Original JMP B-57 and RET B-94/B-95 (PDF 265, 302 and 303) distinguish
conforming and nonconforming code. JMP through a call gate permits conforming
DPL <= CPL without changing privilege; nonconforming code requires DPL = CPL.
RET permits a conforming return target with DPL <= return RPL. The inspected
JMP gate helper requires nonconforming code, and the outer RET helper rejects
conforming code after _e_ret_far has already accepted its privilege predicate.
These are confirmed members of the gate/return validation receiver above.

RET's outer-level algorithm also requires the old stack's entire 8+immediate
byte span to fit and invalidates DS/ES when they are no longer usable at the
new privilege. _ser_ret_far_outer tests a four-byte head and separately peeks
the new SP/SS words; it does not explicitly preflight that entire span. This
remains a boundary/fault-order receiver, not an assertion that every omitted
byte necessarily produces a distinct failure. More directly, the helper
publishes SS/SP, CPL and CS/IP without revalidating DS/ES. _MakeCPL only assigns
CS.dpl, and the complete ExecFinal body performs no successful-return segment
cleanup. Retain DS/ES invalidation with the same transition/publication repair;
the exact validity predicate still needs reconciliation with Chapter 7 before
implementation, rather than blindly adopting ambiguous Appendix prose.

The complete machine_call_gate_smoke read supplies a legal 286 fixture:
LGDT/LMSW/LTR establish a type-one 16-bit TSS, IRET enters ring three, a
zero-parameter GDT call gate enters nonconforming ring-zero code, and RETF
returns to the user marker. It does not cover ring-one/two TSS stack pairs,
nonzero parameter counts, conforming targets, LDT targets or failure priority.
The timing runner reuses that fixture and adds same-level CALL/JMP and
next-instruction-byte observations; its inspected gate recipe still leaves
the gate parameter count zero. Those additional contexts are real evidence,
but do not fill the missing parameter/selector/privilege matrix.

cpu_outer_return_fixture sets a type-0Bh busy TSS, limit 67h, ESP0 at offset
4 and SS0 at offset 8 for every selected profile, including 286. Successful
outer RETF/IRET tests select 386 and do not assert DS/ES invalidation. The
286 delivered-error cases retain a narrower check/error-code claim only;
generation-correct fixtures and legal 286 successful returns remain owned
regression receivers. No test expectation has been changed to bless current
production behavior.

The existing far-control and outer-return CTest cases pass once each width:
x64 2/2 in 0.65s and x86 2/2 in 0.14s. This is current regression execution,
not proof of the uncovered source predicates. S5 remains incomplete; Shared
source/tests, ABI, manifests and deployed binaries are unchanged.

## Task-switch reconciliation

Chapter 8 section 8.3 and Tables 8-1/8-2 (PDF 155-157, printed 8-5 through
8-7) were visually inspected. Direct CALL/JMP authorization compares TSS
DPL with both CPL and RPL; task-gate authorization applies to the gate, not
an extra ring-zero restriction on its target TSS. IRET bypasses the initial
privilege authorization. The manual permits outgoing/incoming tasks at
different privilege levels. TSS descriptors must be global, but task code
and data can use the incoming LDT.

_ser_task_transition_tss's 286 route instead rejects nonzero source CPL or
target RPL and requires target TSS DPL zero. Its shared new-task validators
require CS.RPL/DPL zero and nonconforming code; SS, DS and ES all require
RPL/DPL zero and writable data. That excludes legal non-ring-zero tasks and
readable non-writable data/code for DS/ES. The cross-width planner has similar
fixed-level predicates, so the repair receiver is the common task admission
and cache-validation mechanism, not a 286-specific instruction exception.

The 16-bit state offsets from IP at 0Eh through LDTR at 2Ah match the
inspected TSS layout. Normal nesting writes the incoming backlink and sets
NT, retains outgoing busy and sets incoming busy; non-nested transitions
clear outgoing busy, and returning requires incoming busy. However Table 8-2
also requires JMP to clear incoming NT and IRET to clear outgoing NT before
its state is saved. The inspected 286 route loads incoming flags and only
sets NT when nested; it saves outgoing flags unchanged. _e_eflags_load's
286 mask retains NT. Retain both missing clear operations and unchanged
IRET incoming NT under the same cause-qualified transition receiver.

More fundamentally, the helper validates all incoming caches before saving
the outgoing state or publishing the new TR/registers. The manual distinguishes
early admission faults from late LDT/segment checks in the incoming task's
context. ExecFinal's oldcpu rollback cannot prove that late context merely
because it preserves early-fault atomicity. The repair must define the
architecture's transition boundary and exception origin, rather than make
every task switch a wholly rollbackable transaction. Provider write failures,
descriptor busy locking and simultaneous-fault priority remain to reconcile.

Source-versus-oracle conflicts are explicit: Table 8-1 says a non-present
incoming LDT produces Invalid TSS; _s_task_prepare_ldtr produces NP, and the
existing 286 CPU_TASK16_LDT_NOT_PRESENT test expects NP. Table 8-1 prints
incoming TSS limit greater than 43, while the descriptor layout needs bytes
through 43 and code admits limit 2Bh. Preserve that endpoint inconsistency
for Appendix reconciliation before changing the limit predicate. Stack and
data exception classes likewise need exact Appendix cross-checks, not blanket
replacement from one table.

The existing task16 fixture uses genuine type-one/type-three 16-bit TSS
descriptors and runs normal direct/indirect, CALL/gate, LDT, nested IRET,
IDT gate and selected fault contexts on 286. Its normal tasks are ring zero
and its initial NT values are clear; those cases cannot expose the privilege
or NT-clear gaps above. Test expectations remain unchanged. The entire task
partition is still in progress and no complete qualification is claimed.

The registered task16 test passes once per width: x64 1/1 in 0.07s,
x86 1/1 in 0.14s. Documentation governance and diff whitespace checks pass.
These results preserve the existing oracle, including its disputed LDT
expectation; no Shared source/test or artifact change was made.

### Appendix reconciliation and transition boundary

Original Appendix B-10 through B-13 (PDF 218-221) and Chapter 8 section
8.2.1 (PDF 153, printed 8-3) were visually inspected. Section 8.2.1 explicitly
requires an incoming TSS limit at least 2Bh, and SWITCH_TASKS independently
uses new limit >= 43. This resolves Table 8-1's greater-than wording in favor
of the inclusive endpoint. Do not change the current incoming 2Bh predicate
to 2Ch. The outgoing cache condition is separately >= 41, because dynamic
state ends with DS at 28h/29h; it is not the incoming full-layout condition.

Section 8.2 also explicitly makes the task LDT selector and ring-zero/one/two
stack pairs static fields never changed by 286 task switching. The 286 route
nevertheless preflights sizeof(task_switch_state_16), including LDTR, and
writes cpu_state.data.ldtr.selector at 2Ah into the outgoing TSS. Thus a
legal outgoing limit 29h can fail the larger preflight, and an intervening
LLDT can overwrite the stored task LDT selector. The reusable 16-bit state
writer used by the 386 planner also includes LDTR; retain generation-specific
save semantics rather than assuming 386's rule from the 286 source. Regression
receivers must inspect preserved static fields and outgoing dynamic bytes,
not only the resumed registers or marker.

B-10 and SWITCH_TASKS both expressly require TS for a non-present task LDT,
confirming the NP implementation/test oracle discrepancy. B-13 resolves the
other task-validation classes more precisely than the summary Table 8-1:
invalid/null SS, wrong RPL/DPL or non-writable type produce TS; an otherwise
valid non-present SS produces SS. Invalid CS or wrong privilege produce TS,
non-present CS produces NP, and conforming code is permitted with DPL <= CPL.
Non-null DS/ES admit data or readable code, with both CPL/RPL comparisons for
data/nonconforming code; invalid bounds/type/privilege produce TS and absence
produces NP. The shared validator's non-present SS currently produces NP,
another confirmed cause-specific member of the task-validation receiver.

SWITCH_TASKS describes locked incoming busy publication, outgoing save,
backlink/current-task updates, then new-register loads with invalid caches
before incoming LDT/SS/CS/DS/ES validation. B-10 explicitly says CS/DS/ES NP
occurs in the new task with selectors loaded but caches potentially incomplete.
The current fixture's cpu_task16_expect_fault requires the old TR for every
listed task failure, including its non-present incoming LDT. Consequently
repairing only NP to TS would preserve a second incorrect oracle about the
fault context. Keep error code, transition phase, busy/backlink, saved IP/FLAGS,
selector/cache visibility and exception handler context together in the repair.

Remaining source discrepancies are preserved rather than silently normalized:
B-11's broad not-present-selector sentence names NP for SS, while B-11's
dedicated SS description and B-13 explicitly name SS. Use the specific SS
rules for that case. B-12's line joining nesting, exception context and the
non-nested outgoing busy update is awkwardly printed; further exception-order
reconciliation remains required before a complete publication algorithm is
approved. No new runtime repeat was needed for this read-only source refinement.

## Interrupt and exception-delivery reconciliation

Original section 9.6.2 (PDF 172, printed 9-10), Appendix B-8 (PDF 216)
and INT B-49/B-50 (PDF 257-258) were visually inspected. Double fault is an
80286 behavior. The manual includes DE among eligible first faults,
distinguishes a fault during delivery from a later handler instruction, and
requires shutdown if protection fails again during DF delivery. NMI or RESET
may exit shutdown under documented conditions; NMI retains protected mode,
RESET does not. Shutdown is not an ordinary HLT.

ExecFinal attempts initial protected delivery for 286, but its failed-delivery
DF branch requires profile >= 80386. Failed 286 protected exception delivery
therefore restores/reports the original fault instead of following that
DF/shutdown chain. _e_is_contributory_exception includes TS/NP/SS/GP but
not DE; do not reuse its 386 predicate as an unexamined 286 oracle. The task16
DF gate test and the fully inspected cpu_execution_fault_event fixture select
only 386. Retain generation-qualified escalation and shutdown/NMI recovery
with S2/S5, not merely a widened profile comparison.

INT's source order is gate type, software-origin DPL, then Present. The 16-bit
helper checks Present before software DPL. It requires nonconforming/global
target code, although the manual admits conforming same-level destinations
and valid local-table selectors. Inner-level stack fetch uses fixed TSS
offsets 2/4 (or 4/8 for its 32-bit compatibility branch), irrespective of
destination ring. This is the CALL gate validation/stack-selection class,
not a vector-specific exception. The wrapper does explicitly reject 32-bit
interrupt/trap gates on 286; retain that correct generation guard.

The 16-bit helper saves FLAGS/CS/IP, adds old SS/SP for a privilege switch,
pushes a supplied error word last, clears TF and clears IF for interrupt
gates only. Those normal frame operations match source. It does not clear NT
after saving FLAGS, although both same/inner INT algorithms require that.
Provider failure visibility and simultaneous-fault priority remain unqualified.

For task gates, _ser_int_protected switches task without passing the
error-frame flag/code or pushing the required error word on the incoming
stack. B-50 expressly requires this post-switch push for faults with error
codes. Keep it with task late-publication/delivery, not a second frame owner.

_ser_idt_error_code always returns vector*8+2 and comments that validation
is synchronous. ExecInt nevertheless calls _e_intr_n for external NMI/IRQ;
origin reaches the gate wrapper only as software_origin=false. The inspected
chain supplies no EXT bit to IDT/target-selector errors. Retain cause
propagation to all relevant error producers and reconcile B-8 versus B-50's
different numeric-extension examples before selecting the full cause matrix.

B-50 also specifies real INT/INTO shutdown for initial SP 1, 3 or 5.
_ser_int_real uses six-byte stack preflight but does not itself request
shutdown. Downstream limit/delivery and recovery tracing remain required.
A pending shutdown event alone does not prove persistent architectural state
or NMI exit. Full IRET and asynchronous arbitration contexts remain outstanding.
No new test run, Shared edit or complete partition qualification is claimed.

## IRET source, implementation and regression reconciliation

IRET B-51/B-52/B-53 (PDF 259-261) and system flags section 10.1
(PDF 179, printed 10-1) were visually inspected in the same original.
The source gives real/same-level/outer-level/task-return base clocks
17/31/55/169, plus one clock per byte of the next executed instruction.
cpu_timing_model.c selects these bases and the existing next-term owner;
complete next-instruction decoding and failed-transition timing remain to
reconcile. Exact bases/formula are Manual-L3, not evidence that every
transition currently implements their preconditions.

The complete _ser_iret_protected_same helper admits local-table selectors
through _s_read_xdt and both conforming/nonconforming code with their
same-level DPL conditions. It preflights the six-byte 286 frame and checks
code presence/limit before publication. Both same/outer helpers preserve
IOPL when old CPL is nonzero and preserve IF when old CPL exceeds old IOPL,
consistent with section 10.1. Keep these correct mechanisms; do not replace
them merely because another return branch is defective. Descriptor-memory
publication failures and combined invalid-frame priority remain pending.

The outer helper rejects TI for both CS and SS and accepts only
nonconforming code. B-52 uses the selector's descriptor table rather than a
GDT-only restriction and explicitly includes a conforming branch. This is
the shared gate/return selector receiver, not an IRET-only compatibility fix.
The printed outer conforming condition on B-52 is DPL > CPL, unlike the
same-level condition and the earlier RET listing. Retain this original-source
discrepancy for reconciliation before prescribing a common predicate; it is
visible on the original page, not an OCR correction to silently make.

Outer IRET also tests target IP before validating the new SS descriptor;
B-52 places SS validation before IP. A simultaneous bad IP and invalid or
non-present SS therefore needs an explicit exception-priority regression.
The helper publishes CS/SS/SP/IP/FLAGS without validating/invalidation of
DS/ES for the new outer privilege level. B-52 requires zeroing the selector
and clearing its valid flag when it is no longer usable. This is the already
identified RETF/IRET successful-return cleanup mechanism. The page's final
DPL-versus-CPL/RPL alternatives are preserved for source reconciliation;
no unreviewed simplification of their predicate is introduced here.

Real 286 IRET is not just the generic contiguous-frame path:
_e_iret calls _s_test_ss_iret_pop_286, which checks three individual words
at wrapped 16-bit offsets. cpu_iret_s51_state_smoke exercises the legal
FFFEh/0000h/0002h sequence and the FFFFh rejection with CPU rollback.
These are concrete existing 286 regressions; do not claim that real IRET
has no boundary coverage or conflate it with the separate real data-range gap.

The fully inspected cpu_protected_iret_state_smoke fixture selects only
80386, including its operand-16, conforming and user-FLAGS cases. The Core
machine counterpart also selects 80386. In contrast, the fully inspected
core_machine_protected_16_outer_iret_board_smoke and its shared bootstrap
fixture do select 286 with zero reserved descriptor bytes and a type-one
16-bit TSS declaration. They cover same-level IRET and a GDT nonconforming
ring-zero-to-three return, including SP and FLAGS. They do not assert DS/ES
cleanup, local-table/conforming outer targets, simultaneous-fault priority or
nonzero old-CPL FLAGS restrictions. This qualifies the earlier broad
regression inventory: legal 286 successful returns exist, but the full
return-context matrix is still missing.

Existing real-IRET and 16-bit outer-return cases each passed once per width:
x64 two of two in 0.13s, x86 two of two in 0.10s. This is unchanged-code
runtime evidence, not a repaired implementation or complete S5 delivery.

## Asynchronous arbitration and NMI service-state reconciliation

Original section 9.2/9.4 (PDF 165, printed 9-3), section 9.7.1
(PDF 176, printed 9-14), MOV B-73 and POP B-83 (PDF 281/291), and
STI B-106 (PDF 314) were visually inspected. NMI service inhibits another
NMI until IRET; hardware remembers at most one further request. The source
separately gives single-step priority over external interrupts, with a pending
external interrupt examined before the first single-step handler instruction.
MOV SS inhibits all interrupts through the next instruction; POP SS expressly
includes NMI. These are CPU contracts, not board- or BIOS-specific responses.

The inspected CPU state, execution-context layout, signal API, ExecInt and
IRET paths contain an external mask (flagMaskNMI) and single pending flag
(flagNMI), but no NMI-in-service latch or IRET release of such a latch.
Successful NMI entry clears only the pending flag. A second unmasked request
sets that flag again and can enter at the next ExecInt without waiting for
IRET. Keep external admission mask, remembered pending edge and in-service
inhibition as distinct facts owned by the CPU; do not repurpose the board
mask or create a board compatibility workaround. This missing service-state
transition joins the S2/S5 delivery/return receiver.

ExecInt currently processes NMI before _debug_deliver_trap, then considers
maskable IRQ. _debug_complete_instruction schedules TF traps for the retained
286 as well as 386 (only the DR address comparators are 386-qualified).
Thus the NMI-before-pending-single-step order conflicts with section 9.7.1's
explicit external-interrupt ordering. The comments describing these fields
as 386 debug-trap state do not make their TF consumer generation-specific.
Retain simultaneous TF/NMI/IRQ, post-trap saved state and handler-entry proof;
do not fix only the order of two calls without checking failure/stop paths.

MOV SS and _e_pop_sreg set flagMaskInt only after a successful segment load.
ExecInt uses it to defer NMI and IRQ, and the next instruction's ExecInit
clears it before that instruction's completion arbitration. This supplies
the intended ordinary one-following-instruction external-interrupt delay.
However TF scheduling/delivery does not consult that shadow; the all-interrupt
SS contract and consecutive SS-load cases require their own contextual
regressions. STI also sets the same flag, so it defers NMI as well as IRQ.
B-106 alone does not define that combined shadow; retain the STI/SS distinction
for hardware-source and cross-generation reconciliation, not a guessed
predicate derived from the brief instruction prose.

The inspected cpu_signal_case in cpu_execution_signal_prefetch_smoke runs all
five profiles, including 286: external-mask rejection, coalesced requests,
masking after admission, unmask/HLT wake and first delivery without PIC ACK.
It never requests a second NMI inside a handler or returns with IRET. Its
existing case passes once per width (x64 0.05s, x86 0.06s), but does not test
the missing service-state or simultaneous-trap obligations. The inspected
idt_test_nmi selects a 386 fixture and likewise tests a single entry only.
No Shared implementation change or complete asynchronous qualification is
claimed; the retained regression owner is the CPU signal/return batch, with
board mask wiring checked separately rather than duplicated.

## Inherited multiply/divide reconciliation

Original Appendix B DIV B-39, IDIV B-43, IMUL B-44 and MUL B-76
(PDF 247/251/252/284) were visually inspected. DIV/IDIV use AX for byte
division and DX:AX for word division, reject zero divisors and quotients
outside the destination width, and leave arithmetic FLAGS undefined. IDIV
truncates toward zero and preserves the dividend's sign in the remainder.
The 286 page does not exclude the signed minimum quotient; do not import
an earlier-family range discrepancy as a 286 rule. MUL sets CF/OF from the
upper product; IMUL sets them from whether the low result's sign extension
reproduces the complete product. Other arithmetic FLAGS are undefined.

The inspected _a_mul/_a_imul byte/word paths implement those normal product
and CF/OF rules. The signed word product fits the host's 32-bit intermediate,
including -32768 times -32768. It is not the separately retained 386 dword
IMUL overflow mechanism. IMUL 69/6B admits 186 and later, obtains checked
source/immediate values, computes the word result and writes the destination
through the checked operand helper. Immediate-byte sign interpretation and
the aliased destination do not require a second multiplication path.

Three shared word-division capture expressions still shift the promoted DX
before widening: _a_div's operand and result captures and _a_idiv's operand
capture. DX is unsigned 16-bit but promotes to signed 32-bit int; DX at least
8000h makes the left-shift result unrepresentable. A cast or mask after the
shift cannot cure that C undefined behavior. Actual IDIV arithmetic separately
widens DX before shifting and uses a wider signed dividend/divisor, but its
earlier capture remains unsafe. This confirms the S3 DX:AX concatenation
receiver also covers 286; repair all captures at the shared arithmetic owner,
not a 286-only or board workaround. The divide helpers test zero/quotient
overflow before writing AX/DX. That local ordering is not proof of complete
#DE delivery, saved fault IP or failed-read rollback, which remain S2 receivers.

The 286 timing branch in cpu_timing_model.c selects the original base clocks:
MUL/IMUL byte register/memory 13/16, word 21/24; DIV byte 14/17,
word 22/25; IDIV byte 17/20, word 25/28. Immediate IMUL is word 21/24.
These exact values remain Manual-L3. EA and odd-word additions, prefixes,
fault-versus-retirement attribution and bus assumptions are separate timing
contexts, not proven by these matching bases. Visual B-44 inspection resolves
the extracted text's apparent 68 opcode and 2t clocks as 6B and 21; neither
is a real source/implementation discrepancy.

cpu_legacy_alu_s2_smoke's Group-3 form matrix selects only 8086/80186;
its simple multiplication/division successes are not 286 proof. In contrast,
cpu_imul_immediate_s56_smoke's defaults and word-memory matrix explicitly
select 286 alongside 186/386. They cover signed immediate forms, aliasing,
signed overflow and unchanged source/nonparticipant state. Their later
attribute/VM86/SIB contexts select 386 and do not extend that 286 coverage.
Retain the 286 byte/word implicit-product and divide matrix, signed extrema,
zero/overflow, memory limits/permissions, failed access and installed #DE
frame under the arithmetic/operand/delivery receivers. The manual's real-word
FFFFh exception also joins the already retained generic real data-range gap.
No new test or Shared edit was made; no complete inherited-arithmetic or S5
qualification is claimed.

## Stack/frame reconciliation

Original section 7.4.1 (PDF 139 / 7-13), ENTER B-40/B-41
(PDF 248/249), LEAVE B-64 (272), POPA B-85 (293) and PUSHA B-88
(296) were rendered and visually inspected. Section 7.4.1 requires push-class
limit checks before memory references and internal-register changes, and
explicitly includes PUSH, PUSHA, ENTER, CALL and INT. ENTER additionally
raises SS if SP would cross its stack limit anywhere during execution.
These are not just individually valid word accesses.

PUSHA saves the original SP, in AX/CX/DX/BX/SP/BP/SI/DI order; POPA
restores the reverse sequence and discards the stored SP. The normal 286
handlers agree with those values/order and leave FLAGS unchanged. Each
handler instead invokes eight separate checked push/pop helpers without a
whole-frame precheck. _kec_push checks one word, writes it and changes SP;
the next word can then fail after earlier external writes. General exception
CPU restoration does not turn that into the source-required early range
validation. POPA's whole-span, wrap and discarded-slot access requirements
remain a separate read context; do not infer its provider accesses solely
from the word listing. The shared stack owner must plan the applicable span
before publication rather than accumulating per-instruction guards.

ENTER checks both immediates, reduces the level modulo 32 for 286, pushes
old BP, copies the level-minus-one previous-frame words, optionally pushes
FRAME_PTR, installs BP and subtracts the local allocation. This normal
algorithm matches B-40, including level zero and one. The final allocation
subtraction has no stack-limit check; earlier pushes/copies likewise occur
before complete range validation. This joins the ordered stack/frame receiver
with nesting-source reads, destination writes, final allocation and failure
priority. The generation-specific full-byte 186 level remains distinct.
LEAVE probes the BP-addressed word before assigning SP and popping BP;
that precheck is a live source-qualified difference, not a wrapper to remove.
Its real FFFFh word boundary follows the already retained real-range receiver.

PUSHA's real-mode initial SP values 1/3/5 require shutdown; 7/9/11/13/15
require exception 13. The generic word-push helper instead reaches SS for a
nonzero SP smaller than the word width; neither PUSHA nor its dispatch has
an initial whole-frame distinction. Retain these exact 286 contexts with
the existing shutdown/recovery receiver, not a BIOS compatibility branch.

Exact base timing matches for PUSHA 17, POPA 19, ENTER level zero 11,
level one 15 and higher levels 12+4*level after modulo 32. LEAVE does not:
B-64 gives 5, while core_machine_80286_source_timing_ledger selects 8 and
machine_80286_timing_manifest_runner's I286-STACK-LEAVE recipe expects 8.
The earlier t435-s1-80286-ledger already records 5; the historical T359
control-stack record preserves the disputed 8. Keep the correct exact term
Manual-L3 and correct the producer/oracle/source chain together after review;
neither downgrade nor a passing self-confirming oracle resolves the mismatch.

The inspected cpu_pusha_popa_smoke and cpu_enter_leave_smoke default matrices
really select 286 and assert ordinary register/stack images; ENTER includes
levels 0/1/3/33, with 33 reduced to one. Their protected-limit/failure
fixtures select 386, not 286. ENTER's failure oracle explicitly expects
already written words, so it cannot prove the 286 early-limit contract.
The later attribute/32-bit-stack tests likewise do not qualify 286 by name.
Retain valid 286 descriptors, installed SS handler, complete boundary and
shutdown matrices plus every failed provider access. Both existing CPU cases
passed once per width: x64 two of two in 0.11s, x86 two of two in 0.10s.
These passes confirm existing narrow behavior only. No Shared code/test
change, complete stack qualification or S5 delivery is claimed.

## Count, rotate/shift and short-branch reconciliation

Original rotate B-90/B-91 (PDF 298/299), shift B-97/B-98
(305/306), Jcond B-54 (262) and LOOP B-70 (278) were visually inspected.
The 286 uses the low five count bits, unlike 8086. Rotates preserve other
arithmetic FLAGS; shifts define result SF/ZF/PF and CF with AF undefined.
OF is defined for a single operation and undefined for multiple operations;
zero count does not modify FLAGS. Undefined OF is not permission to advertise
an independently source-defined value. The memory exception conditions remain
applicable contexts; zero arithmetic change alone does not prove no operand
access or no write-permission check.

_a_shift_rotate_count implements the five-bit rule for 286. The inspected
byte/word ROL/ROR/SHL/SHR/SAR loops produce the normal bit/CF results;
SAR relies on the signed host right-shift behavior and keeps the sign bit.
RCL/RCR additionally reduce the masked count modulo 9/17 before storing opr2.
For example, word count 18 becomes one, so the current OF-definedness logic
uses a one-step predicate although the original masked count was multiple.
Count 17 becomes zero and skips the operation's metadata. Retain result-cycle
optimization separately from original count, FLAGS definedness and elapsed
work; do not label the source's undefined OF as a wrong required physical
value. SHL/SHR/SAR also mark OF undefined at zero although guest FLAGS remain
unchanged. This is a diagnostic/attribution context in the shared count/FLAGS
receiver, not evidence that zero-count guest OF was changed.

C0/C1 and D0-D3 check operand decoding/read, invoke the same arithmetic
helpers and write the result through the operand owner, including zero count.
Failed fetch/read/write and count-zero read-only memory need explicit source
and regression dispositions; do not delete the write based only on no result
change. Protected operand faults and real word FFFFh boundaries share the
existing permission/range/publication receivers. The unprefixed timing path
uses constant-one register/memory bases 2/7 and variable-count 5/8 plus the
masked count, with the existing memory EA term. It retains raw masked timing
count rather than the RCL/RCR modulo result; prefix, odd-word and failure
attribution still require the timing partition's complete proof.

The inspected short Jcc handlers match the individual Appendix conditions;
_e_jcc sign-extends the displacement and invokes checked near transfer only
when taken. _e_loopcc computes wrapped CX-1, preserves FLAGS and publishes
CX after successful target validation. JCXZ tests CX without changing it.
These normal rules do not prove installed protected-fault frames or all
16-bit target-wrap contexts. The shared instruction outcome must distinguish
taken from merely landing at a different IP.

The 286 primary Jcc timing branch compares final IP with fallthrough and
returns 7/3, while B-54 requires taken 7 plus one clock per next instruction
byte. That branch returns before any next-term helper in the selected timing
chain; its concrete next-byte term is missing. A taken zero-displacement Jcc
also lands at fallthrough and receives the wrong not-taken base. The shared
core_machine_control_stack_short_branch_taken helper makes the same IP-only
inference for LOOP/JCXZ. Their 286 base 8/4 matches B-70/B-54, but actual
condition/count, zero displacement, prefixes and attribution remain the
S2/S3/S5 branch-outcome receiver. B-70 prints no next-byte footnote; do not
extend B-54's Jcc/JCXZ term to LOOP by association. JCXZ's B-54 term must
also be reconciled with its current base-only control-stack return.

cpu_rotate_smoke includes 286 CL=21h byte forms and 286 immediate byte/word
count-one forms, including illegal extension six. Its broad zero/count-cycle,
FLAGS and memory matrices select 386. cpu_control_transfer_branch_smoke
includes 286 taken/not-taken short conditions, LOOP conditions and JCXZ,
and rejects later near-Jcc encoding; its protected target-fault helper selects
386. These are real narrow 286 tests, not complete 286 count/branch proof.
Both existing cases passed once per width, two of two in 0.10s each. Retain
286 full count-cycle and operand-failure coverage plus semantic branch timing
with zero displacement and varied target instruction lengths. No Shared edit
or complete inherited/count/branch qualification is claimed.

## String iteration, port and restart reconciliation

Original CMPS B-34, INS B-47, LODS B-69, MOVS B-75, OUTS B-82 and
REP B-92/B-93 (PDF 242/255/277/283/290/300/301) were rendered and visually
inspected. Source strings use DS or its override, destinations use ES without
override, and DF selects byte/word index movement after the operation.
CMPS computes source minus destination; its arithmetic FLAGS and SCAS's
comparison FLAGS determine termination only after the first operation.
REP first tests CX; zero executes no primitive. Each successful iteration
then decrements CX, tests comparison termination where applicable, and permits
pending interrupt recognition between iterations. Memory faults, real word
FFFFh and denied I/O remain instruction-specific exceptions, not REP faults.

The shared primitive owners use checked read/write and move indices after
success. CMPS/SCAS publish comparison FLAGS after checked operands. However,
the pre-386 MOVS/CMPS/STOS/LODS/SCAS byte/word handlers call those owners
without the enclosing CPU_TRACE_CHECK_RETURN used by the 386 handlers;
their repeated paths subsequently decrement CX and can set flagInsLoop after
a failed primitive. This is the inherited checked-propagation receiver from
S2/S3, not a reason to introduce a second string executor. ExecFinal restores
oldcpu for faults and uses that snapshot for protected exception entry or
terminal stop. Therefore this source inspection does not prove that the
incorrect intermediate CX decrement survives into the guest's fault frame.
It does prove inconsistent failure propagation; neither register rollback
nor a passing normal fixture establishes provider-side rollback or correct
failed-iteration timing. Preserve prior successful iterations, restore the
failed iteration's architectural state, and qualify irreversible effects
through the same operand/bus and delivery owners.

INS/OUTS use checked helpers for 186 and later, including 286. INS validates
the destination before port input, writes ES and then advances DI. OUTS reads
the source before port output and advances SI only on success. _p_input and
_p_output check mode before transfer_port; complete_port follows a successful
transfer. The 286 CPL>IOPL path reaches _kpa_test_iomap's pre-386 rejection
and produces GP(0), rather than consulting a 386 I/O bitmap. This reconciles
normal privilege denial, not the priority of simultaneous memory/I/O failures.
Retain destination-provider failure after input consumption, source failure
before output, denied privilege with invalid memory, word boundaries and
every completed versus rejected transfer as explicit receiving contexts.

flagInsLoop restores CS/IP to oldcpu at an unfinished iteration; ExecInit
takes a new snapshot per execution step. This preserves a source-visible
restart point and prior successful iteration state locally. Complete IRQ/NMI
and fault/IRET replay, prefix preservation and timing re-entry remain in the
shared delivery/repeat receiver; no checkpoint here proves that full chain.

B-92 explicitly gives MOVS/INS/OUTS 5+4*CX, STOS 4+3*CX,
CMPS 5+9*N and SCAS 5+8*N, with N actual executed iterations. Their current
setup/per-iteration entries match those printed bases. The same table has no
LODS row, and B-69 supplies only its primitive five clocks. Production's
comment claiming an Appendix-B REP LODS S+4*CX formula, its 5/5/4 entry,
and the existing L3 REP-LODS manifest thus require another exact authority
or an honest model disposition; absence from this table alone neither proves
illegal encoding nor authorizes immediate downgrade. This source-allocation
receiver is retained for review. Segment-prefix/odd-word terms, zero-count
modifiers, interrupted continuation identity and failed-retirement attribution
remain in the complete timing partition; matching bases are not full proof.

The five ordinary string fixtures really include 286 normal byte/word and
repeat contexts, including zero count and comparison termination. Their
protected failure fixtures select 386. cpu_port_strings_smoke's successful
port-string matrix selects 186/386, and its protected failure cases select
386; its 286 coverage rejects later attribute encodings, not legal I/O.
cpu_port_io_smoke does include 286 scalar normal forms. Thus narrow green
tests do not qualify the missing 286 repeated-fault, privilege or interrupt
matrix. Existing seven CPU cases passed once each width: x64 seven of seven
in 0.34s, x86 seven of seven in 0.62s. No Shared source/test/API, product or
artifact change, complete unit closure, or S5 acceptance is claimed.

## External control and NPX boundary reconciliation

Original 3-29/3-30, Table 10-1/10-2, 11-5, B-9, HLT B-42,
LOCK B-68 and WAIT B-113 (PDF 85/86/182/183/191/217/250/276/321)
were visually inspected. HLT is CPL-zero-only in protected mode, preserves
FLAGS, and enabled interrupt return points after HLT. Its base is two clocks.
WAIT's base is three clocks, excluding external BUSY duration. MP and TS
jointly control WAIT's NM in Table 10-1/10-2; EM causes ESC emulation.
B-113 and 11-5 abbreviate the WAIT predicate to TS alone. These statements
cannot justify deleting MP from the implementation without reconciliation;
the explicit recommended MSW matrix distinguishes MP-zero emulation, where
WAIT does not cause NM. Keep the broader control-bit context in the CPU/NPX
source receiver, including non-recommended combinations, rather than treating
an abbreviated entry as complete proof of a production bug.

WAIT checks TS&&MP, then pending numeric error, otherwise calls
x86_fpu_complete_wait. That call clears busy/remaining ticks and records the
whole remaining interval; the selected CPU timing later adds last_wait_ticks
to the instruction's base. Thus this is an atomic modeled completion with
elapsed-time attribution, not proof that IRQ/NMI/device events during the
wait observe the same ordering as the original asynchronous BUSY interval.
Its three-clock 286 base matches the printed row. The FPU's external service
choices are explicitly L2 and must remain distinct from this exact CPU base.
Retain interruptible BUSY wait, deadline progression, error-versus-NM priority,
and re-entry state under the one CPU/NPX/Core time receiver; do not add a
host sleep or second timer. No unsupported full-287 arithmetic claim follows
from a valid command handoff.

B-9 defines MF testing on WAIT with EM clear and on most ESC instructions,
with named no-wait exceptions FNCLEX/FNINIT/FSETPM/FNSTCW/FNSTSW/FNSAVE/
FNSTENV. Current WAIT does not gate its pending-error test on EM, and
FPU_ESCAPE has no pending-error test at all. Its EM/TS checks precede the
command dispatch after ModR/M decoding. The checked generation/control-bit
and pending-error matrix therefore needs the same source-qualified receiver,
including no-wait operations, not a blanket check for every escape opcode.
B-9 also distinguishes initial address/permission GP from later multiword
NPX protection fault nine. For 287 handoff, FPU_ESCAPE decodes the effective
operand but does not perform the memory access checks or transfers used by
its partial 8087 execution subset. The Core extension callback records only
opcode/ModR/M in a transaction. This is command-observation evidence, not
a complete 286/287 operand/ERROR/PEREQ/transfer boundary. Retain that boundary
inside the CPU external-cycle audit without expanding into full 287 execution.

HLT currently raises GP for protected nonzero CPL, then still advances IP
and sets flagHalt; ExecFinal restores oldcpu before fault delivery/stop.
As with unchecked strings, do not claim the transient halt survives the
fault frame. Required receiving proof is denied HLT with an installed 286
handler, unchanged FLAGS/state, correct fault versus successful next-IP,
and enabled IRQ/NMI/reset wake. ExecInt clears halt only after successful
delivery. The existing signal fixture evidence remains narrow and is not
re-run here. Successful HLT clocks select the exact two-clock owner.

LOCK B-68 prints zero clocks, names MOVS/INS/OUTS bus locking and says XCHG
always locks. Section 3-29 independently defines CPL<=IOPL. The current
pre-386 prefix correctly enforces the 286 privilege predicate without using
the 386 opcode whitelist; existing real CBW/ADD/REP-MOVS tests establish
transparent execution, not every claimed physical lock. Unlike the 386
branch, this prefix does not set flagLock, and the bus provider exposes no
locked memory-sequence boundary. Implicit XCHG, descriptor accessed/busy
updates and interrupt bus cycles need a generation-qualified atomicity/lock
disposition under the existing CPU/bus owner. No multiprocessor support is
inferred from a single execution owner. Additional hardware original evidence
is required for the exact signal interval and legal-form scope; neither
unconditional UD nor an added 386 LOCK clock term is justified for 286.

cpu_fpu_interface_state covers all-family no-FPU success and later-encoding
rejection, not 286 busy/error combinations. machine_fpu_interface_s65 has
real 286/287 command handoff and next-deadline fixtures; its WAIT handoff
oracle explicitly expects modeled completion after one run. These fixtures
cannot prove interrupt ordering during WAIT. The legacy LOCK fixture has
actual 286 real-mode transparency/rejection contexts but no protected IOPL
or physical lock observer. Three existing cases passed once each width:
x64 three of three in 0.39s, x86 three of three in 0.38s. No Shared change,
complete external-cycle qualification or S5 acceptance is claimed.

## Ordinary ALU / Decimal Source / Handler / Regression Reconciliation

Original PDF 223-228 / B-15-B-20, 241 / B-33, 244-246 / B-36-B-38,
254 / B-46, 285 / B-77, 287-288 / B-79-B-80, 307 / B-99,
317-318 / B-109-B-110 and 324 / B-116 were visually inspected. They
define the byte/word ALU, sign-extended byte immediates, nonparticipant
FLAGS and decimal adjustment rules; their exception rows distinguish
read-only CMP/TEST from writeback forms and require real word FFFF checks.
Those checks remain in the already identified segment/operand receiver.

The common _kac_arith2 owner masks operands/results to the selected width,
sign-extends its bit=12 immediate to a word, and computes CF/OF/AF/SZP.
ADC/SBB calculate the result before publishing CF, including the all-ones
second-operand carry/borrow case. CMP computes the same width-reduced
subtraction without writing the operand. INC/DEC omit CF from their mask;
NEG uses zero-minus-operand and separately sets CF for nonzero input;
NOT has an empty FLAGS mask. AND/OR/XOR/TEST clear CF/OF, calculate SZP
and mark AF undefined. These normal 286 byte/word formulas agree with
the inspected source; this is not exhaustive operand or failed-write proof.
Checked operand handlers cannot by themselves prove provider rollback or
fault-time FLAGS restoration; the S2 publication receiver remains required.

AAA/AAS separately adjust AL by six and AH by one, clear the high AL
nibble, set/clear AF/CF and mark OF/SZP undefined. AAM/AAD with the
documented 0A second byte split/combine decimal AL/AH and calculate SZP,
marking OF/AF/CF undefined. DAA/DAS use the adjusted AL for their second
condition and compute SZP/AF/CF, marking OF undefined. These agree for the
source-defined decimal-use contexts. Broader nondecimal inputs and other
AAM/AAD immediate values are not promoted to manual-qualified forms from
these pages. Both pre-386 AAM/AAD paths call _d_imm without checking its
result; they join the inherited fetch/failure-propagation receiver rather
than acquiring a separate decoder or a fabricated successful fault path.

The live primary timing selector, not the terminal unallocated 286 fallback,
owns the reconciled normal rows: binary ALU register two/memory seven,
immediates three/memory seven; CMP memory destination seven versus memory
source six, immediate three/memory six; TEST two/memory six and immediate
three/memory six; INC/DEC/NEG/NOT two/memory seven. Decimal simple forms
are three, AAM sixteen and AAD fourteen. These exact bases match the original.
EA, odd transfers, prefixes and failed attribution remain the common modifier
batch; matching one base is not proof for all 771 catalog keys.

cpu_legacy_alu_s2 actually includes 286 for binary register/memory directions,
accumulator immediate byte/word and documented decimal/XLAT samples. Its
Group-1 sign-extension, TEST, INC/DEC and Group-3 matrices still select
8086/186, not 286. Decimal samples do not enumerate every defined FLAGS
result or undefined-bit assertion. cpu_direct_flags includes 286 ordinary
direct FLAGS and later-attribute rejection; cpu_eflags_local's principal
SAHF/LAHF cases use 386. Retain the missing 286 arithmetic extrema, alias,
nonparticipant/undefined FLAGS, protected rejected access and fetch/publication
contexts under the sole arithmetic/operand receiver. Three existing tests
passed once each width: x64 0.15s, x86 0.19s. The initial mistyped CTest
filter selected zero tests and is not evidence; the corrected named runs above
are the actual baseline. No Shared change or complete S5 unit gate is claimed.

## Movement / Bounds / FLAGS Reconciliation

Original PDF 230 / B-22, 235 / B-27, 243 / B-35, 267 / B-59,
271 / B-63, 294 / B-86, 297 / B-89, 304 / B-96 and 322-323 /
B-114-B-115 were visually inspected in addition to the earlier MOV and
LDS/LES pages. BOUND performs signed inclusive word comparisons, requires
memory bounds, leaves FLAGS unchanged and faults for a four-byte real operand
beginning FFFD or higher. LEA calculates an offset without operand access;
register-source encoding is UD. XLAT uses unsigned AL with BX and a DS/override
byte read. CBW/CWD preserve FLAGS and sign-extend into AH/DX.

Normal MOV byte/word direction/immediate paths and register aliases retain
the original width-specific assignments. The pre-386 word ModR/M MOV and
XCHG paths omit checking _d_modrm; LEA omits checking _d_modrm_ea and its
write. They join the existing fetch/admission/publication receiver. LEA's
helper does report UD for register form, but continuing after that report is
not full fault-atomic proof. LDS/LES read the offset and selector separately
then use _e_load_far; their complete-span and failed selector contexts remain
the segment receiver, not proof from two successful reads. XCHG publishes
the register before its final memory write; ExecFinal restores registers on
fault, while provider effects and implicit bus locking remain separate proof
requirements. The original explicitly requires BUS LOCK independently of
IOPL and an explicit LOCK prefix.

BOUND checks its two word reads and comparison. Its upper-bound read changes
the decoded offset by two without a whole four-byte preflight; real-mode
FFFF/10000 handling therefore joins the existing 286 limit receiver. XLAT's
BX+AL sum reaches _m_read_logical without explicit 16-bit reduction, joining
the inherited effective-offset receiver. CBW/CWD normal 286 assignments agree
with the inspected source. Their broader regression contexts cannot be
borrowed from a 386-only attribute test.

SAHF changes only SZAPC; LAHF copies the low FLAGS byte and sets bit one.
B-59 calls its other low bits indeterminate, so deterministic reserved-bit
choices are not exact silicon proof. Their two-clock bases, CBW/CWD two,
LEA three, XLAT five, XCHG three/memory five, successful BOUND thirteen,
PUSHF three and POPF five match the live primary/control-stack selector.
Existing EA/odd/prefix/next-term and failure attribution receivers still apply.
The inspected XLAT scan contains clipped operation prose and an incongruous
real word exception for a byte instruction; neither is used as a speculative
extra XLAT word access.

POPF has a concrete 286 privilege gap. B-86 requires real-mode NT/IOPL to
remain unchanged; in protected mode IOPL changes only at CPL0 and IF changes
only when CPL<=the current IOPL, otherwise preserving those bits without an
exception. The pre-386 POPF path instead passes the whole popped value to
_e_eflags_load. That helper is only a defined-bit mask, not a mode/privilege
merge. It cannot enforce either rule. cpu_direct_flags's real-identity recipe
specifically expects 286 POPF of F000 to produce 7002, thereby blessing this
gap. Preserve the 386 expectation separately; do not fix every CPU by the
same high-bit mask. Required receiver is the sole generation/mode FLAGS
load/image owner, with real NT/IOPL preservation, protected CPL/current-IOPL
matrix, other defined FLAGS, stack fault restoration and independent oracle.
Pre-386 PUSHF also ignores its _e_push result and lacks the source's real
SP=1 shutdown distinction; it joins the same stack/delivery receiver.

cpu_gpr_mov includes actual 286 byte/word normal directions, immediate memory
and reserved-extension rejection; protected segment tests select 386.
cpu_lea includes 286 real addressing/register-source rejection, while null-DS
and protected cases select 386. cpu_bound includes 286 legal word/attribute
rejection, not the full signed failure/segment matrix. cpu_xchg includes 286
default word and accumulator forms; atomic bus observation remains absent.
Four existing named cases pass once each width in 0.16s each. Their green
result does not cancel the incorrect POPF oracle or qualify all contexts.

## Simple FLAGS / Hardware Boundary Reconciliation

Original programmer PDF 236-238, 240, 286 and 312-314 (B-28-B-30,
B-32, B-78, B-104-B-106) were rendered and visually inspected. CLC/STC/CMC
change only CF; CLD/STD only DF; NOP only advances IP. Their live handlers
and two-clock flag / three-clock NOP bases agree. CLI has three clocks,
STI two. Both must raise #GP(0) in protected mode when CPL exceeds IOPL.
The actual CLI/STI handlers put this check exclusively in the >=80386
branch; their pre-386 branch unconditionally changes IF. Thus protected
286 CLI/STI join POPF in the generation/mode FLAGS privilege receiver.
The fix must preserve ordinary 8086/8088/186 behavior, legal 286 CPL<=IOPL,
denied 286 old IF/IP/stack and normally installed #GP delivery, and distinct
386 VM86 behavior. A shared I/O permission helper is not evidence that these
two handlers use it. cpu_direct_flags tests normal 286 flags, not that matrix.

The original hardware manual is Intel 80286 Hardware Reference (1987),
order 210760-002, fresh SHA-256
D3ECE037A200B17EF32D78A055D0D96C3E47474D755D94569B9576A9A68C6915.
PDF 45, 62, 115, 117 and 238 / printed 2-17, 3-12, 3-65, 3-67,
7-18 were rendered and visually inspected. NMI recognition is instruction
boundary based; another NMI cannot be acknowledged before IRET. Its input
low/high qualification is a signal-level requirement, not a substitute for
the CPU service latch. The current flagMaskNMI is externally controlled;
ExecInt clears pending NMI after successful delivery but does not establish
service inhibition. Existing nested-NMI, IRET-unblock and TF/NMI priority
receivers therefore remain substantiated by an independent hardware original.
No instruction-following-STI wording was found in this hardware original;
do not claim it resolves the programmer B-106 shorthand or the shared
STI/MOV-SS NMI shadow distinction.

Hardware LOCK is not only a decoded prefix. It prevents bus surrender and
is automatically asserted for XCHG, interrupt acknowledgement and some
descriptor operations. The hardware list includes MOVS/INS/OUTS and memory
rotate; Table 3-5 also lists MOV. These are explicit original-context leads,
not permission to apply the 386 legal-LOCK whitelist backwards. Table 3-5
gives HOLD-latency formulas in doubled system clocks, with separate memory
and INTA wait-state inputs; divide by two for processor clocks. They are
not replacement Appendix-B retirement costs. The CPU bus provider has
memory/port completion and interrupt acknowledgement but no locked interval
or automatic-lock signal. Core's external-cycle callback records individual
BEGIN/COMMIT/CANCEL and DMA arbitration but does not derive these CPU lock
intervals. A lock_prefix observation cannot close that receiver. Retain one
CPU-to-neutral-bus lock boundary with explicit/automatic contexts, failed
transfer release and DMA exclusion; no board-specific handler or second CPU.

Hardware Table 3-6 independently confirms reset FLAGS=0002, MSW=FFF0,
CS=F000/base FF0000, IP=FFF0, all four segment limits FFFF and IDT limit
FFFF. The last value conflicts with programmer section 10.4, which gives
IDT.limit=03FF. This is a source conflict, not a demonstrated current IDT
reset defect: the existing 03FF agrees with the selected programmer source.
The other confirmed fields reinforce the S2 reset-image receiver rather
than accepting a 386-compatible reset image. Physical reset pulse and
power-up intervals are not invented retirements or a host sleep policy.

## Timing Assumptions / Modifier Owner Reconciliation

Original programmer PDF 214 / B-6 was rendered and visually inspected.
Appendix-B clocks are processor clocks, not the doubled CLK input. The
additional exact terms are one for base+index+displacement, two for each
odd physical word operand reference, and one for each memory-read wait
state. Writes/fetch waits do not necessarily add directly. The stated
five-to-ten-percent average fetch excess is a range, hence any selected
macro ratio is L2, not an exact per-instruction L3 adjustment. Exact base
rows remain Manual-L3 under their documented assumptions; this does not
establish full physical execution time from the bases alone.

cpu_timing selects one 286 producer chain (string/primary/control-stack,
then an explicitly unallocated 286 fallback), not 386 compatibility costs.
The EA helper tests ModR/M mode!=0/3 and rm<=3 for the one-clock term;
the odd helper uses segment base plus effective offset. Primary ALU/move
costs multiply the odd term by their transfer count, while individual
system/stack/string paths have their own references. The shift path lacks
an odd-word term and the moffs path relies on retained ModR/M fields;
these require real operand-reference proof, not only a green base recipe.
Protected descriptor/stack implicit references and zero-count operand
access also belong to this one modifier-context receiver.

Core external-cycle timing separately adds configured waits on successful
transfer COMMIT, with source provenance and cancellation. It can add both
page timing and address-window waits and includes writes/fetches depending
on the configured contract. This is not automatically the Appendix-B
read-wait formula or proof of overlap. Required closure maps units, eligible
references, read/write/fetch overlap and failure charging to the single
guest-clock owner, preventing double charging or falsely exact labels.
No nominal board MHz or hardware HOLD-latency table closes this proof.
Existing 286 manifest recipes test base/EA/odd/repeat/next-form observations,
but their LEAVE eight-clock oracle already contradicts the original five;
query/task fixtures also encode the mismatches recorded above. Preserve
these independent-source corrections when reconciling all 771 catalog keys.

## Admission / Original-Edition Conflict Follow-through

Programmer PDF 184, 263, 328-329 / 10-6, B-55, C-2/C-3 were freshly
rendered and inspected. Appendix C independently confirms the ten-byte
286 instruction limit and type-six violation, no such limit on 8086/8088,
five-bit shift counts, largest-negative signed divide acceptance, TF priority
over external interrupts, and NMI blocking until IRET. It also specifies
memory XCHG's automatic LOCK, no LOCK during instruction prefetch and the
286 signal's bus-to-next-bus interpretation. These are additional contexts
in the retained runtime-decode, arithmetic, arbiter and bus receivers,
not separate board work or an excuse to use the 386 limit/rules on 286.

The runtime minimum-family metadata and handler dispatch reject 64-67,
later 0F forms, FS/GS loads and MOV CS, while allowing the documented
286 system group, ARPL and inherited 186 forms. That establishes ordinary
later-form admission, not a complete invalid-encoding exception contract.
INS_0F_00/01 still need their extension and memory-only predicates; group
metadata validity alone does not certify execution. The copied lexical
scanner has a fifteen-byte cap and rejects LOCK wholesale; ExecIns does
not invoke it as an execution gate. It cannot prove the 286 ten/eleven-byte
runtime boundary, lawful LOCK observation or any failed body-fetch rollback.
Paging/VM86/32-bit instruction capability remains non-applicable to 286;
the reserved descriptor/type leakage recorded above is not such a capability.
Reset and PE transitions still belong to the chip state owner, not profiles.

The owner's archive also contains Intel iAPX 286 Programmer's Reference
Manual Including the iAPX 286 Numeric Supplement (1985), fresh SHA-256
9C6067E777AE694D5F71D8ADE23014558BE4E9D156041C778AED84C1246D8538.
PDF 3-4, 129, 174, 180, 249, 254 and 302 were rendered and inspected
for identity and the specific conflicts; no complete second-edition reading
is claimed. This is cross-check evidence, not an imported implementation.

- 1985 B-102 explicitly delays enabled external interrupts until after the
  next instruction, with STI/RET and STI/CLI examples. This supports the
  IRQ shadow missing from 1987 B-106's abbreviated prose. It does not
  independently establish an IF-based NMI delay; retain the SS/STI/source
  distinction and already-set IF context under the arbiter receiver.
- Both programmer editions' 10-6 state IDT.limit=03FF, versus hardware
  Table 3-6's FFFF. Keep current 03FF aligned with the selected programmer
  contract; do not propose FFFF solely from the hardware table. Programmer
  reset processing says 3-4 clocks whereas hardware gives at least 38 CLK
  cycles before the first memory cycle; their stated intervals/units are
  not interchangeable. A physical-reset timing claim needs its own resolution.
- 1985 7-13 repeats the expand-down/full-stack FFFF prose that conflicts
  with its own 11-2's explicit strict-greater formula and empty FFFF case.
  The detailed formula matches both 1987 Chapter 11 and current non-big
  bounds. Retain that formula as the selected contract; record the prose
  error rather than changing a correct empty-segment implementation.
- 1985 B-54 correctly presents JMP-to-call-gate conforming and
  nonconforming predicates as alternatives, not nested requirements.
  It reinforces the existing conforming-gate receiver. No later-family
  TSS/gate type is inferred from this cross-check.
- 1985 B-49 repeats outer IRET conforming DPL>CPL and the contradictory
  nonconforming <=RPL prose. Repetition is not resolution. Keep the
  outer-return source conflict distinct from demonstrable TI restrictions,
  missing DS/ES cleanup, stack span and published CPL/FLAGS issues.

Existing lifecycle/prefix/decoder fixtures remain receivers for reset caches,
ten/eleven-byte runtime decoding and invalid-group priority. Decoder inventory
enumerates lexical candidates, not execution success. The complete-family
audit will not accept its candidate count as proof of these state contracts.

## Complete Batch Disposition And Repair Ownership

All ten finite partitions now have an audit disposition. This closes the
inspection inventory, not the listed production gaps or whole-family
qualification. The opening table's pending entries describe the starting
batch; the following table is its delivery disposition. Every residual stays
inside T544 with the already selected shared owner, rather than becoming a
board workaround or a different queued task. Sections above provide the
original pages, actual handler/helpers, fixture limitations and tested
contexts for each entry.

| Partition | Reconciled contracts | Residual receiver / required proof |
| --- | --- | --- |
| Admission and state | 286 additions/inherited forms, unsupported later forms, ten-byte rule, reset/PE, mode-specific FLAGS | S2 CPU admission/state owner: runtime ten/eleven-byte and invalid-group exception priority; reset MSW/CS cache; POPF/CLI/STI privileges, image versus writable state. IDT reset edition conflict remains separate, not a proven 03FF defect. |
| Descriptor queries | ARPL RPL/ZF and 10/11 bases; LAR/LSL/VERR/VERW visibility, types and 14/16 bases | Sole descriptor/query owner: Present-bit assumptions, valid LDTR/TI query handling, 286 reserved types/layout, all RPL/DPL/conforming combinations, rejected operand/descriptor/write and destination/FLAGS publication. |
| System registers/tables | Table six-byte images, selectors, protected privilege, sticky PE, CLTS and source bases | Sole system/cache owner: generation-qualified descriptor decoding, null LLDT cache clearing, LTR type/busy publication, privilege-before-operand priority, table span/access failures. Undefined sixth-byte policy is not falsely numeric L3. |
| Segment/address access | Normal loads, GDT/LDT bounds, access classes, detailed expand-down formula, word boundary and SS shadow | Sole segment/operand owner plus S2/S3 failure receiver: real FFFF bounds, LDS/LES whole spans, reserved upper descriptor bits, null/conforming/local-table selectors, early validation versus cache publication and failed references. |
| Control/gates | Direct/call-gate CALL/JMP, same/outer RET/IRET, word frames, ring-specific TSS stacks and exact formula bases | Sole control/delivery owner: GDT-only/nonconforming restrictions, ring stack selection/parameter spans, DS/ES cleanup, SS/IP validation priority, protected FLAGS and external effects. Conflicting outer IRET predicates require source resolution before a scalar repair. |
| Tasks | Incoming 2B limit, dynamic outgoing save span, backlink/busy/NT, task CALL/JMP/IRET and incoming-context faults | Sole task owner: legal CPL/RPL/TSS cases, incoming versus outgoing publication point, static LDTR preservation, late fault context, NP/TS/SS distinctions and task-gate error word; current task16 oracle is not independent proof. |
| Exceptions/asynchronous delivery | 286 contributory exceptions, DF/shutdown, frames, EXT, TF/SS/STI/NMI and IRET service interval | S2 generation-qualified delivery/arbiter: 286 DF class and terminal behavior, NT/EXT provenance, early real stack shutdown, NMI-in-service and TF priority, SS inhibition, source-qualified STI IRQ versus NMI behavior, full installed handler/fault matrix. |
| Inherited ordinary forms | ALU/decimal/movement/stack/count/MUL/DIV/branches with 286 bases and normal state assertions | S3/S4 arithmetic/offset/stack/count owners: unsafe DX:AX capture, unchecked fetch/operand results, early stack spans/ENTER allocation, RCL/RCR FLAGS count versus cycle count, zero-count accesses and actual branch decision. LEAVE exact five versus producer/oracle eight is a confirmed correction receiver. |
| Strings/external cycles | REP phases, INS/OUTS memory/port order, WAIT MP/TS, ESC/HLT/LOCK and original hardware intervals | Sole repeat/external/delivery owner: checked pre-386 string propagation and restart/publication, failed port/memory ordering, WAIT BUSY interruptibility/error conditions, ESC operand/protection/error exemptions and meaningful LOCK bus interval. REP LODS claimed Appendix authority remains unresolved, not implicitly illegal. |
| Timing qualification | All inspected base/formulas, EA/odd/read-wait assumptions, next-form terms and hardware versus retirement units | Sole timing/Core guest-clock owner: per-reference modifiers, implicit accesses, shift odd cost, moffs effective-reference proof, taken zero-displacement branch attribution, read/write/fetch overlap/no double charge, source-conflict/model labels and independent predicates across the 771 keys. |

The original 1987 JMP continuation at PDF 266 / B-58 was also rendered and
inspected at final reconciliation. Its task-JMP text says nesting, whereas
the task transition chapter distinguishes non-nesting JMP; this is retained
as a source-pseudocode conflict, not copied into the implementation. The
1985 B-54 alternative call-gate predicates support the conforming admission
receiver. Repetition across editions is not sufficient to resolve the outer
IRET contradiction or IDT reset discrepancy recorded above.

Manual numeric/formula bases and proven normal semantics stay L3 under their
stated assumptions. Unknown source/context and false current oracle claims
are retained explicitly; no automatic demotion to L1 or guessed scalar fixes
are made. A range/model can be L2 only with an actual model and provenance.
Source-undefined behavior is not an invented exact result. Original source
conflicts, missing regression contexts and confirmed implementation defects
are distinct categories, all still T544 obligations. This S does not
advertise 80188/486, change the board clock, or qualify a CPU by product boot.

## Delivery Verification Boundary

All selected existing test runs above are baseline observations only, not
new regressions or independent confirmation of their own oracles. Shared
source/tests/ABI/manifests, App INIs/media and deployed PC/MyNES EXEs have
zero changes. This docs-only delivery needs no new executable; ignored
research/render scratch and receiving caches remain useful for S6 and the
convergence repair audit. Fresh complete repository-only unit and governance
results are recorded here before executor P1 delivery.

Fresh complete x64 repository-only units pass once: 506/506, exit zero,
174.97 seconds; x86 passes once, 506/506, exit zero, 145.33 seconds. Both
use eight jobs and a 300-second bounded deadline. The long negative-boundary
fixture accounts for 171.69/142.02 seconds respectively; these are retained
as observed, not rerun to obtain a preferred duration. Both owned process
handles are closed. Documentation structure, changed-document relative links,
sixteen-field packet shape and git diff --check pass.

Executor self-review maps the entire ten-partition batch to original-page
inspection, current handlers/state/timing providers, actual fixture coverage
and named residual receivers. It rejects any inference of family qualification
from the complete green baseline or 771 catalog keys. All changed paths are
NXVM evidence/ledger/Current; Shared source/test/manifests, other product
surfaces and binaries are unchanged. Delivery requires actual-change
coordinator acceptance and governance P before S5 closure.
