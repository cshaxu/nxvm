# T544 S3 Early CPU Family Audit

## Status And Scope

Audit inventory delivered against d47399525. This is not S acceptance, Shared repair approval
or whole-family qualification. The batch remains F01-F10/F14 for both 8086
and 8088, including legal forms, operand/encoding/prefix contexts, state,
delivery and timing. Existing catalog recipes are coverage inputs, not an
independent manual oracle. Current owns the active packet.

## Primary Source

Intel iAPX 86,88 User's Manual (1981), the owner archive copy named
intel-8086-8088-users-manual-1981.pdf, SHA-256
3EEA6CA77AD4046AE7ADE731410793206EEBE8EC9A3F8AE75895685D38F4FFE5.
The original identity/provenance is retained in the
[T512 source record](t512-s1-five-cpu-source-cross-validation.md).
The actual owner archive is manuals-nxvm/cpu; no manual is copied into this
repository. PDF 56 / printed 2-37 and PDF 74 / printed 2-55 were rendered and
visually inspected in this batch, followed by PDF 58 / printed 2-39 for
shift/rotate rules. F01 also uses visually inspected PDF 54-57 / printed
2-35 through 2-38, PDF 70-71 / printed 2-51 through 2-52, PDF 73 / printed
2-54 and PDF 261 / printed 4-24. MUL uses PDF 80 / printed 2-61.
The archive hash was freshly recomputed and matches.
Extracted text served only as navigation.

## Complete Family Partition, Still Open

| Family | Early-family surface | Current disposition / remaining receiver |
| --- | --- | --- |
| F01 | Decimal/ASCII adjustments and CBW/CWD | Eight handlers and basic clocks reconciled below; valid-BCD transcription check passes; actual regression matrix, nondecimal encodings and fetch-failure receiver remain open. |
| F02 | ALU, compare and test forms | Shared arithmetic/form paths and base-clock rows reconciled below; encoding-table conflicts, 8088 regression coverage and failed-decode receiver remain open. |
| F03 | Data movement, exchange, address and segment loads | State/form paths partially reconciled below; LEA extends failed-decode inventory; source legality, bus ordering and S2 segment inhibition remain open. |
| F04 | Stack and FLAGS image/load | Basic stack/FLAGS publication inspected below; full wrap/provider-failure/form proof and S2 image/delivery repair remain open. |
| F05 | Branch, call, return, software/external interrupt and IRET | Normal predicates/frame paths inspected below; zero-displacement timing selection defect and source/bus/failure receivers remain; S2 delivery discrepancies are not repaired. |
| F06 | MUL/IMUL/DIV/IDIV, byte/word, register/memory | Partial source/code reconciliation below; early-IDIV numeric source/code discrepancy remains unresolved; three host word-concatenation defects have a proposed repair receiver, not Shared approval. |
| F07 | One/CL shifts and rotates | Full early CL count, normal flag rules and selected exact formulas reconciled; undefined metadata has no current reader; both-early boundary/bus regression receiver remains open. |
| F08 | MOVS/CMPS/STOS/LODS/SCAS and repeats | Normal iteration/termination paths inspected below; early multi-prefix interrupt restart, failed-element propagation and transfer/timing proof remain open. |
| F09 | I/O, WAIT, ESC, HLT, NOP and XLAT | CPU/external ownership inspected below; XLAT sum lacks word-offset truncation; WAIT sampling, HLT wake and complete I/O bus proof remain open. |
| F10 | Direct flag controls and LAHF/SAHF | Direct-state paths reconciled below; reserved-bit assertions, both-early regression and S2 arbitration/inhibition remain open. |
| F14 | CPU escape issuance | Original early memory ESC read differs from no-FPU dispatch; width/order/failure and both-early traces remain receivers; no complete FPU claim. |

Later-only forms inside the historical family headings do not become early
forms by heading association. Source-proven absence and negative decode proof
must be established before their non-applicable disposition. Historical List 1
rules about undefined instructions, POP CS and LOCK are reconciliation inputs,
not authority to impose later-generation behavior on early silicon.

## F01 Adjustments And Conversions

The inspected handlers are DAA, DAS, AAA, AAS, AAM, AAD, CBW and CWD in
cpu_instructions.c. The first six operate on AL/AH; CBW extends AL into AH,
and CWD extends the sign of AX into DX. The conversion handlers do not write
FLAGS. DAA/DAS explicitly update AF/CF and pass only SF/ZF/PF to the common
flag helper; AAA/AAS update AF/CF and mark the other arithmetic flags undefined.
AAM/AAD update SF/ZF/PF and mark AF/CF/OF undefined. This matches the prose
rules on printed 2-35 through 2-38 for the documented operand domain; it is
not proof of all arbitrary input or fetch-failure contexts.

### Source Conflicts And Encoding Boundary

Printed 2-51 lists AAM as one byte, but the same manual's original encoding
table on printed 4-24 explicitly shows D4 followed by 0Ah; AAD is D5 followed
by 0Ah. The existing two-byte handlers therefore match the encoding authority.
Do not shorten AAM in response to the contradictory size cell. The encoding
table also contains parenthesized displacement columns on these fixed forms;
they do not establish a memory operand absent from the instruction definition.

Printed 2-54 marks DAA's OF cell as updated, whereas printed 2-36 explicitly
states OF is undefined. The implementation's undefined annotation follows the
prose. A precise arbitrary OF assertion is not justified by this conflicting
cell. Keep this source discrepancy visible; no timing downgrade follows from
a flags-table conflict. Cross-edition corroboration remains an S3 source
receiver before an exhaustive flags qualification claim.

The documented AAM/AAD encodings contain decimal base 0Ah. The implementation
accepts an arbitrary second byte as a base, and AAM base zero requests type 0.
Existing nondecimal-base recipes from later profiles are not proof that all
such encodings are documented early forms. S3 must reconcile those extension
contexts separately, rather than imposing a modern #UD or claiming Manual-L3
for every base merely because this generic handler executes it.

### Valid Decimal Domain, Not Arbitrary-State Proof

Printed 2-36 describes DAA/DAS after addition/subtraction of valid packed
decimal operands. A read-only PowerShell transcription of the inspected
AL/AF/CF branches was compared with independent decimal arithmetic for every
ordered pair 0..99: 10,000 DAA pairs and 10,000 DAS pairs, zero mismatches in
the final packed result and decimal carry/borrow. Inputs were packed from the
decimal pair; incoming AL, CF and AF came from its binary add/subtract. Expected
results were decimal sum/difference modulo 100, with decimal carry/borrow.
No repository script or Shared test was created for this scratch calculation.

This is a mathematical check of a transcription, not execution of the actual
CPU or a durable regression. It does not qualify PF/SF/ZF, unrelated state,
arbitrary initial flags or undocumented decimal inputs. In particular, a
difference between the current DAS branch and a later pseudocode algorithm
on a synthetic AL/AF combination must not be presented as an established
early-family defect outside the source's stated operand domain.

### Timing And Existing Tests

The primary selector in cpu_timing_model.c includes both 8086 and 8088 in one
switch: simple adjustments 4, AAM 83, AAD 60, CBW 2 and CWD 5 clocks. Those
literals agree with printed 2-51, 2-52 and 2-54. These exact base values remain
Manual-L3 within their source conditions. None of these forms has a data-word
transfer requiring the 8088/odd-word footnote. This check does not close
instruction-fetch/prefetch, invalid-prefix or failed-instruction timing.

legacy_alu_test_adjust_and_xlat_forms in cpu_legacy_alu_s2_smoke.c runs one
recipe per adjust form for 8086/186/286/386, not 8088. It checks final AX, IP
and preserved IF/DF; DAA/DAS additionally check their five defined flags.
It does not exhaust both AF/CF branches or defined AAM/AAD flags. The same
file's conversion/flag matrix checks CBW/CWD results and preserved state.
cpu_sign_extend_smoke.c independently checks positive/negative CBW/CWD,
unchanged nonparticipants and FLAGS for 8086/186/286/386, again not 8088.
These are real existing tests, not a claim that the early domain is untested.
S3's remaining receiver is both-early-CPU, documented-input, defined-flags,
wrap/failure and prefix coverage in the existing test owners.

### Fetch-Failure Mechanism Lead

For profiles below 386, AAM/AAD call _d_imm without the usual
CPU_TRACE_CHECK_RETURN. _d_imm clears cimm before reading; _kdf_code returns
early if _s_read_cs fails. AAM can then interpret the cleared byte as base
zero and add a divide-error bit to the original fetch failure. The shared
exception setter ORs exception bits; it does not replace the prior failure.
ExecFinal tests several deliverable exceptions by exact equality, records
other faults and restores oldcpu. Rollback of registers therefore does not
prove preservation of the original failure identity. AAD can also perform
temporary state work after failed fetch, although final rollback may hide it.

This is an inspected failure-control-flow lead, not a newly executed fault
probe or a claim about normal hardware memory reads. The finite receiver is
early immediate-fetch failure propagation, swept across all unchecked fetch
callers and the later 186/286 variants. Any repair must retain the common
decoder/fault owner and original handler style, stop before dependent work
after a failed read, and prove diagnostic identity plus state rollback. No
Shared modification is admitted by this record.

## F02 ALU, Compare And Test Reconciliation

### Inspected State And Forms

The common _a_add/_a_adc/_a_sub/_a_sbb helpers mask the selected operand width;
CF uses carry/borrow, OF uses operand/result signs, AF uses their bit-four XOR,
and PF/SF/ZF use the masked result. ADC/SBB consume incoming CF before flag
publication. Logic clears CF/OF, computes PF/SF/ZF and marks AF undefined.
CMP/TEST compute flags without writing the destination. These are inspected
implementation facts, not acceptance of every input and failure context.

The seven mutating operations have the same six primary form paths: byte/word
r/m destination, register destination and accumulator immediate. A read-only
body comparison normalized only mnemonic/helper names and empty trace labels;
the corresponding paths have no substantive difference. Group 1 extensions
0-7 select ADD/OR/ADC/SBB/AND/SUB/XOR/CMP in 80h/81h/83h, with 83h's early
word immediate sign extension. CMP skips destination publication. TEST uses
84h/85h, A8h/A9h and F6h/F7h extension zero. Ordinary ALU/CMP decode and
operand operations check failure before dependent work. One early word TEST
decode does not; the complete direct-call inventory is below.

A scratch mathematical transcription compared ADC and SBB byte CF/OF/AF
expressions with independent widened arithmetic for all 256 x 256 operand
pairs and both carry inputs: 131,072 cases per operation, zero mismatches.
This did not execute the CPU and is not a durable regression. It does not
qualify word/dword arithmetic, addressing, PF/SF/ZF or fault behavior.

### Original Tables And Conflicts

Additional visually inspected Intel 1981 pages are PDF 72, 81, 84, 86-87,
260 and 262 (printed 2-53, 2-62, 2-65, 2-67 through 2-68, 4-23 and 4-25).
Prose on printed 2-38 defines the logic flags; the arithmetic definitions
on 2-35/2-36 and these encoding/timing rows are the primary source.

The original logic encoding table prints TEST register/memory as 001000dw,
duplicating AND rather than the implemented 84h/85h. The Intel 1985 owner's
archive copy, intel-8086-8088-80186-80188-users-manual-1985.pdf, SHA-256
2516D66CC75076D9AC9EE048E8420C09C35655FB25ED34DDA6351A3EA4E0AFFF,
was freshly hashed. Its visually inspected PDF 65 / printed 1-49 repeats
that conflict, whereas PDF 181 / printed 2-15 lists TEST as 1000010w in an
unshaded compatible form. The latter table explicitly distinguishes later
additions by shading. Do not replace TEST with AND to follow the erroneous
generic row. Encoding-source reconciliation remains explicit.

ADD/ADC/SUB/SBB/CMP immediate encodings explicitly contain the s bit;
AND/OR/XOR's generic table does not. Existing execution of 82h and sign-extended
logical 83h forms is not by itself documented early legal-form proof. Those
alias contexts need a source/model disposition, not a speculative #UD repair.

### Timing And Regression Limits

Both early CPUs share the primary timing selector. Its mutating ALU rows are
register/register 3, register/memory 9+EA, memory/register 16+EA,
register/accumulator immediate 4 and memory immediate 17+EA. CMP uses 3,
9+EA, 4 and 10+EA respectively. TEST uses register/register 3,
register/memory 9+EA, accumulator immediate 4, register immediate 5 and
memory immediate 11+EA. These base literals match the original rows.
EA and segment terms retain the source-defined addressing matrix; memory
word transfers add four clocks per transfer on 8088, or on an odd 8086 word.
Mutating r/m operations have read/write transfer count two; compare/test
have count one. The original TEST memory-immediate row has a dash in the
transfer column despite its required memory read. Preserve that discrepancy
as a source receiver, not proof of zero bus transfers or a timing downgrade.

cpu_legacy_alu_s2_smoke.c covers binary forms, accumulator immediate,
Group 1 and TEST recipes, but omits 8088. Its binary forms include
8086/186/286/386; Group 1 and TEST use narrower profile selections.
The later inc/dec-second-group suite includes ADC/SBB carry and overflow
boundaries, predominantly on 386; those recipes are not early-profile proof.
The legacy binary/Group 1 oracle compares full FLAGS even for undefined
logical AF. Current inputs do not reveal a failure, but expanded tests must
compare defined flags rather than require an arbitrary AF value. Its lib_u32
sum oracle also needs widened arithmetic if future dword unsigned-overflow
cases are added; no current test failure is inferred from that limitation.

The existing x86.cpu_legacy_alu_s2 and x86.cpu_inc_dec_second_group tests
passed once each on both existing host-width caches: x64 2/2 in 0.08 seconds,
x86 2/2 in 0.09 seconds. This transient focused selection is diagnostic
evidence only, not S3's complete-unit gate or full F02 qualification.

## Cross-Family Unchecked Decode Receiver

A search restricted to _d_imm/_d_modrm found twelve unguarded calls. The
subsequent all-_d_* direct-call sweep adds LEA's _d_modrm_ea, giving thirteen.
This corrects the earlier search scope; it is a finite inventory, not thirteen
confirmed runtime defects:

| Handler | Direct decode | Family receiver |
| --- | --- | --- |
| TEST_RM32_R32 | ModR/M word | F02 |
| XCHG_RM32_R32, MOV_RM32_R32, MOV_R32_RM32 | ModR/M word | F03 |
| LEA_R32_M32 | Effective-address ModR/M word | F03 |
| CALL_PTR16_32 | Immediate four bytes | F05 |
| RET_I16, RETF_I16 | Immediate word | F05 |
| INT_I8, JCXZ_REL8 | Immediate byte | F05 |
| JMP_REL32 | Immediate word | F05 |
| AAM, AAD | Immediate byte | F01 |

The early TEST word branch calls _m_read_rm after unchecked decode, then
_a_test. CPU_TRACE_CHECK_RETURN executes its argument before testing the
exception state, so a subsequent checked call cannot be assumed to prevent
its own dependent read. F03 and F05 must inspect every listed branch and its
possible externally observable work. The proposed repair receiver must prove
original failure identity, no dependent provider operation and rollback,
including applicable 186/286 contexts. One decoder/failure owner and the
original table-style handlers remain the design constraint. No Shared edit
or newly executed provider-fault probe is claimed here.

Follow-through inspection confirms the four ModR/M sites are the below-386
branches. MOV r/m destination proceeds to _m_write_rm; the other three proceed
to _m_read_rm. Neither helper has an entry guard for a pre-existing exception;
they choose the retained decoded reference/memory fields and invoke the next
operation before the macro tests failure. _kdf_modrm initializes those fields
only after its first code-byte read succeeds, so a first-byte and a later
displacement failure require distinct regression inputs. Do not assume either
a null reference or a valid completed address in all cases.

The six F05 sites are likewise below-386 branches. CALL passes the fetched
pointer to _e_call_far; RET/RETF invoke their stack/return helpers; INT invokes
its interrupt helper; JCXZ/JMP invoke _e_jcc. The inspected far-call path enters
real/protected validation, RET starts the pop helper, RETF starts stack access
validation, and a taken _e_jcc enters target validation. None is protected by
CPU_TRACE_CALL_BEGIN, which is empty. This establishes dependent helper entry
after failed decode, not that every such path commits a bus write or wrong
architectural result. The remaining proof is provider-operation/failure-origin
observation through the full helper and ExecFinal paths, including taken versus
untaken JCXZ. AAM/AAD retain the distinct arithmetic/exception lead above.

## F03 Data, Address And Unary Paths

Visually inspected Intel 1981 PDF 50-51 / printed 2-31 through 2-32 defines
MOV's byte/word transfer, XCHG's exchange, LEA's offset-only memory source,
and LDS/LES's memory pointer: offset first, then the segment word. PDF 78,
80 and 81 / printed 2-59, 2-61 and 2-62 supply pointer/LEA/MOV/NEG/NOT
clock rows. These pages are original-source evidence, not OCR-only claims.

MOV immediate and moffs forms use the byte/word destination and preserve
FLAGS. The inspected ordinary register/memory paths use the decoded operand
references; three early word paths have the failure lead above. XCHG reads
r/m, writes the prior r/m value into the register, then writes the prior
register value into r/m. Accumulator exchanges use a temporary and word
assignment. No normal-path flag update is present. This is not bus-lock,
partial-write or provider-failure qualification.

LEA decodes the address and writes only its offset, without _m_read_rm. Its
early branch does not check _d_modrm_ea failure before _m_write_ref. The helper
also rejects a register source; that is a source-excluded form, but the manual's
memory-only definition does not itself establish a later-style #UD on early
silicon. Existing negative tests must not be treated as independent authority.
LDS/LES read the offset and next word using the old decoded source segment,
then _e_load_far loads DS/ES and writes the general register. Both reads are
checked before that publication. Segment-load validation and full rollback
remain shared-owner proof requirements, not new F03-specific implementations.

MOV to/from segment registers uses the common segment decoder/load owner;
loading CS is rejected, and only SS sets the current inhibition flag. The S2
early any-segment inhibition discrepancy remains open. Do not count these
normal selector-transfer paths as a repair of that discrepancy.

INC/DEC use add/subtract-one with flag masks excluding CF. NOT has an empty
flag mask. NEG computes zero minus the width-masked operand, then sets CF
from nonzero original operand. Exact result/flag boundaries and their
register/memory failure publication remain regression receivers; inspecting
the shared helpers alone does not accept all forms.

Primary MOV base costs 2 (register/register), 8+EA (register/memory),
9+EA (memory/register), 4 (register immediate), 10+EA (memory immediate),
and LEA 2+EA match the rendered rows. Moffs has a separate 10-clock row.
NEG/NOT cost 3 or 16+EA. The explicit 8088 segment/pointer timing branch
uses 2 for segment/register, 8+EA+4 for segment load from memory,
9+EA+4 for segment store and 16+EA+8 for LDS/LES; these match one/two word
transfers with the manual footnote. Other generation branches, transfer
addresses and arbitrary prefix combinations still require reconciliation.

Existing cpu_gpr_mov, cpu_sreg_mov and cpu_lea real-form matrices select
8086/186/286/386, omitting 8088. Segment-selector LxS success/memory-only
matrices inspected here select 386; they do not prove both early profiles.
The five selected existing tests (gpr_mov, sreg_mov, segment_selector, lea,
inc_dec_first_group) pass once per host width: x64 5/5 in 0.19 seconds,
x86 5/5 in 0.16 seconds. This is a transient diagnostic run, not the full
S3 unit gate. No test recipe, Shared source or manifest was modified.

## F06 Arithmetic And Delivery Findings

### Source And Actual Path

Printed 2-37 explicitly defines the early signed quotient intervals as
[-127, 127] and [-32767, 32767], remainder sign matching the dividend and
truncation toward zero. A quotient outside those intervals generates type 0.
MUL/IMUL define CF/OF through significant upper-half/sign-extension results;
their other arithmetic flags are undefined. DIV/IDIV arithmetic flags are
undefined. This is functional authority, not a new timing-tier selection.

In src/x86/chips/cpu/cpu_instructions.c, INS_F6 and the early INS_F7 branch
decode and read the register/memory operand once before dispatching extensions
4-7 to _a_mul, _a_imul, _a_div and _a_idiv. Both early CPU profiles use that
same path. _a_idiv instead tests a byte quotient against -128 and a word
quotient against -32768, with no early-generation lower-bound selection.
It therefore accepts source-excluded exact minimum results for both early
profiles. This is a confirmed code/source discrepancy, not merely missing
test coverage or a reason to relabel all Group 3 timings.

For example, AX=FF80h, divisor byte=01h produces quotient -128; the common
helper currently writes AL=80h/AH=00h rather than requesting type 0 on 8086
or 8088. DX:AX=FFFF:8000h, divisor word=0001h similarly produces -32768.
These examples are deductions from the inspected implementation, not newly
executed probes. The S2 later-manual compatibility pages are reconciliation
inputs, not independent confirmation that the early silicon excludes the
signed minimum. The cross-edition disposition below leaves that numeric
boundary unresolved. S4 must establish the 80186 rule separately; ordinal
proximity is not evidence.

The shared helper computes signed byte/word quotients in lib_i64 and checks
before writing quotient/remainder. The word path explicitly guards the
full-width signed dividend minimum divided by -1; the 32-bit path likewise
guards its host signed minimum. Those host-safety guards are distinct from
the guest quotient interval and must remain intact. Unsigned DIV and the
multiply upper-half logic were inspected, but this inspection alone does not
accept every source or fault context.

### Regression Gaps

test/x86/chips/cpu/cpu_inc_dec_first_group_smoke.c:

- inc_dec_test_div_idiv_forms covers signed/unsigned byte/word/dword and
  register/memory, but selects 80386 for those recipes.
- inc_dec_test_div_idiv_attribute_and_profile covers a 386 attribute case,
  a 286 unsupported-prefix case and a normal unsigned 80186 division.
- Neither of those functions proves the early signed minimum boundary.

The PC composition machine_8086_timing_manifest_runner.c also has an actual
timing_manifest_probe_group3_function with eight normal MUL/IMUL/DIV/IDIV
recipes, plus range-model and memory/segment/odd-word timing recipes. These
are useful existing early-family tests; they do not exercise the excluded
signed-minimum quotient or prove the source-defined type-0 return frame.
The source/code discrepancy is not attributed to an absence of all early
arithmetic tests; neither the existing recipes nor these omissions resolve
the disputed silicon boundary.
cpu_legacy_alu_s2_smoke.c additionally runs Group 3 extensions 2-7 for 8086
and 80186 in byte/word register forms, checking normal signed/unsigned results,
CF/OF on multiply and preserved IF/DF. It does not include 8088 or the signed
minimum boundary. Preserve these useful recipes when extending the matrix.

### Timing Reconciliation

Rendered Table 2-21, printed 2-55, gives IDIV register-byte 101-112 and
register-word 165-184 clocks; memory-byte 107-118 plus EA and memory-word
171-190 plus EA. IMUL gives register-byte 80-98, register-word 128-154,
memory-byte 86-104 plus EA and memory-word 134-160 plus EA.
These are ranges: operand-sensitive choices inside them remain L2.

The inspected core_machine_8086_group3_model_cost bounds for those IDIV/IMUL
rows match the rendered ranges. Its subsequent memory path adds EA and the
segment-override term. For a word transfer it adds four clocks on 8088, or
the odd-word term on 8086, matching this page's explicit transfer footnote.
This reconciles these literals/terms only; it does not independently qualify
the model's iteration decisions, every memory decode, all transfers or failed
instruction timing. Original-page review also reconciles DIV on printed 2-54:
register-byte 80-90, register-word 144-162, memory-byte 86-96 plus EA and
memory-word 150-168 plus EA. MUL on printed 2-61 gives 70-77, 118-133,
76-83 plus EA and 124-139 plus EA respectively. The model's corresponding
minimum/maximum literals match all four rows for each instruction. The
operand-sensitive work formula remains a model (L2), not a manual formula
inferred from those bounds.

### Coherent Repair Proposal, Not Yet Implemented

Keep one _a_idiv helper and original table-style handlers. Only after resolving
the numeric source boundary, select any demonstrated generation-specific
negative quotient bound inside that helper;
do not add register-versus-memory or App/board special cases. Preserve later
full signed ranges and host-overflow guards. Add boundary recipes to the
existing CPU test owner for both early CPUs, both widths and both source kinds:
lowest legal quotient, excluded signed minimum, positive maximum/overflow,
zero divisor and signed remainder/truncation. Verify ordinary later-profile
signed minimum remains accepted where the manual permits it.

Actual type-0 delivery must also join the S2 coherent return-IP/exception
mechanism: early instructions save the next IP rather than the later fault
restart IP. Do not "fix" the arithmetic and silently accept the wrong frame.
The full-state rollback and diagnostic origin must remain distinct from the
source-defined return address. Do not assert exact undefined quotient,
remainder or FLAGS contents on early type-0 failure without source authority.
Concrete Shared approval and an implementation packet are required before
changing any source/test. No additional API or decoder is proposed.

## F07 Count Rule And Metadata Lead

Rendered printed 2-39 permits up to 255 shifts using CL (or a constant one).
The actual _a_shift_rotate_count helper retains the full byte for exactly
8086/8088, as selected by core_machine_cpu_profile_has_8086_semantics in the
public CPU header. The byte/word shift/rotate paths consume that count;
early RCL/RCR also avoid the later modulo-nine/seventeen reduction. This
specific source/code distinction is already implemented: do not patch it
merely because a raw 1Fh mask appears in later-only 32-bit paths.

The existing rotate_test_cl_count_profile_matrix selects 8086, 80186, 286
and 386 and compares the byte result at CL=21h. It does not include 8088,
word/full-count extremes or assert the computed carry in that matrix. Other
rotate/shift tests need reconciliation before claiming those contexts absent.
The page defines CF as the last bit shifted out, while _a_shl/_a_shr add CF
to the undefined metadata when count >= operand width even on early CPUs.
The loop still computes CF. This is an outgoing metadata qualification lead,
not evidence that the actual early carry value is wrong. S3 must inspect all
consumers of that metadata and the complete count/flags tests before deciding
the repair surface. A complete src/test C/header search outside the instruction
implementation finds only the udf field declaration; the instruction file
itself resets or adds bits but has no reader of that value. No current
production/test consumer uses it to alter outgoing FLAGS. The discrepancy is
therefore internal metadata, not an established guest carry defect or a reason
to change the shift loop. Count-zero and multibit undefined OF must not be conflated
with the defined nonzero-count CF rule.

## F04 Stack And FLAGS Paths

Visually inspected Intel 1981 PDF 50 / printed 2-31 defines word PUSH as
SP decrement by two followed by store, and POP as read followed by increment.
PDF 52 / printed 2-33 defines PUSHF/POPF and depicts the defined flag positions;
its unspecified image bits must be reconciled with the later compatibility
authority retained by S2, rather than silently treating all editions alike.

The common _kec_push computes a width-wrapped decremented stack address,
checks the store, then publishes SP. _kec_pop reads the current stack address
before incrementing SP; direct POP SP skips that increment after replacing
the stack pointer. PUSH SP explicitly selects the decremented value for
profiles below 286 and the original value for 286/later. This is actual
implemented generation selection, not a missing path. Independent early
source proof for that special case and wrap/bus/failure observations remain
receivers; the explanatory code comment is not its own hardware oracle.

The inspected segment POP helper reads the selector, validates/loads its
segment, then increments SP. Only SS sets inhibition; this remains coupled
to S2's early any-segment inhibition discrepancy. PUSHF constructs an image
without changing flags; POPF reads a word before passing it to the common
generation load helper. That helper's early mask 0FD5h plus bit one preserves
the defined state but emits zero for high image bits. S2's confirmed outgoing
image mismatch is not resolved by this normal-path reconciliation.

cpu_pushf_popf_smoke.c's default matrix really includes both 8086 and 8088,
unlike many surrounding tests. It masks early comparisons to 0FD5h, so a
passing recipe does not qualify bits 12-15 or disprove the S2 image finding.
cpu_gpr_push_pop_smoke.c tests all eight word PUSH registers and the
decremented PUSH SP distinction, but its profile matrix omits 8088. Its
POP-to-ESP-address example is a 386 addressing case, not early proof.
All stack forms, memory ordering, wrap and delivery contexts remain in F04;
neither normal-path success nor register rollback proves provider-side
failure atomicity. No stack or FLAGS repair is implemented by this audit.

## F10 Direct Flags And Byte Transfer

Visually inspected Intel 1981 PDF 51-52 / printed 2-32 through 2-33 defines
LAHF's five transferred flags and SAHF's five replaceable flags, preserving
OF/DF/IF/TF. LAHF's bits 5, 3 and 1 are explicitly undefined in this edition.
PDF 66-67 / printed 2-47 through 2-48 defines the seven direct controls and
STI's following-instruction delay; PDF 85 / printed 2-66 supplies STC/STD/STI
two-clock rows. Previously inspected printed 2-53 gives CLC/CLD/CLI/CMC two
clocks and rendered printed 2-59/2-64 gives LAHF/SAHF four. Exact literals remain L3
within the documented execution context, not proof of interrupt-delivery cost.

CLC/STC/CMC change only CF; CLD/STD only DF; early CLI/STI only IF. STI sets
the existing flagMaskInt marker. ExecInt excludes both its NMI and INTR
paths while that marker is set; this is the concrete arbitration owner,
not evidence that every generation needs the same inhibition rule. S2's
generation/priority findings remain open. The below-386 CLI/STI branches
also include 286 without protected privilege checks; S5 must reconcile that
specific shared branch against 286 authority, not borrow the 386 branch
or declare it correct from real-mode tests.

SAHF masks exactly SF/ZF/AF/PF/CF into existing flags. LAHF assigns the low
FLAGS byte with bit one set. Defined flags match the early prose; an exact
assertion on its other bits requires separate authority. The legacy
flags/sign matrix contains actual LAHF/SAHF recipes for 8086/186, not 8088,
and requires one exact AH image. That deterministic test is not independent
proof of unspecified early bit values. The direct_flags matrix checks
CF/DF operations and nonparticipants for 8086/186/286/386, not 8088; it
does not itself test CLI/STI or following-instruction delivery.

Four selected existing tests (direct_flags, pushf_popf, gpr_push_pop,
legacy_sreg_stack) pass once per width: x64 4/4 in 0.12 seconds, x86 4/4
in 0.13 seconds. This transient selection does not replace complete units,
source reconciliation, instruction-form or interruption coverage.

## F08 Strings And Repeat Boundaries

Rendered Intel 1981 PDF 60-62 / printed 2-41 through 2-43 defines byte/word
index movement under DF, zero-count suppression, count termination and
compare termination after each performed element. Printed 2-42 explicitly
does not require initial ZF to be prepared. CMPS subtracts destination from
source; SCAS subtracts destination from accumulator. The same page's JG
example reverses that relationship, a source conflict rather than authority
to reverse the implementation's subtraction. MOVS/STOS/LODS do not change
arithmetic FLAGS. Source segments can be overridden; the destination remains ES.

The five helper paths in cpu_instructions.c use those operand directions.
_kas_move_index wraps word SI/DI and selects DF-dependent byte/word deltas.
The ten byte/word handlers suppress the helper when repeated CX is zero,
perform one element per refresh, decrement count and decide whether another
element remains. CMPS/SCAS examine the newly computed ZF, not the initial ZF.
ExecFinal rewinds CS/IP only for continuing repeats; ExecInit saves a new
oldcpu for every element. Completed earlier iterations therefore remain
when a later element is rolled back. This is not whole-repeat rollback.

The below-386 branches call element helpers without CHECK_RETURN before
count/loop updates, unlike the 386 branches. This is a separate ten-handler
failure-propagation sweep, not another entry in the thirteen decoder-call
inventory. ExecFinal's register rollback can mask those transient updates;
it does not prove provider-side effects or original failure identity intact.
The receiver must inject read/write failures at each element boundary and
inspect delivery/committed effects before prescribing one coherent repair.

Printed 2-42 explicitly describes the early multi-prefix interruption
limitation: only the prefix immediately preceding the string instruction
remains in effect on interrupt return. Current repeat rewinding uses the
entire old instruction start, with no early-family selection at that point;
refresh then calls ExecInt. The source/code mismatch needs an actual saved-IP
and resumed-prefix trace for both early CPUs, coupled to the S2 delivery
receiver. Do not impose this early limitation on 186/286/386 or introduce
a second decoder. Ordinary uninterrupted repeat success cannot qualify it.

cpu_timing_model.c already owns first/continuation/zero-count phases and
charges setup once across matching uninterrupted iterations. No second
counter or timing owner is needed. It accepts F2/F3 for CMPS/SCAS, but only
F3 for the other string timing forms, whereas instruction handlers accept
either repeat prefix. Noncanonical F2 forms require source/model disposition;
their execution alone does not establish Manual-L3. Literal table rows,
word-transfer additions and interruption setup accounting remain proof
receivers; this inspection does not claim their complete qualification.

Existing MOVS/CMPS/STOS/LODS/SCAS tests include zero/nonzero repeats,
termination and modern width/segment cases. Their family matrices omit 8088.
Protected 386 fault recipes in the string tests already check preservation
of completed iterations; they are real coverage, not early provider-failure
proof. All five selected tests pass once per width: x64 5/5 in 0.17 seconds,
x86 5/5 in 0.18 seconds. This transient selection does not qualify the missing
early interruption, failure or bus contexts or replace complete S units.

## F09 And F14 CPU / External Boundaries

Fresh rendered Intel 1981 PDF 37 / printed 2-18 describes WAIT's TEST input
being retested at five-clock intervals. PDF 67 / printed 2-48 distinguishes
HLT (leave on reset, NMI or enabled INTR), WAIT (TEST synchronization), ESC
(external opcode and possible memory operand), and NOP. PDF 86 / printed
2-67 gives WAIT 3+5n and XLAT 11 clocks. PDF 73 / printed 2-54 gives ESC
register 2 and memory 8+EA with one transfer. These source literals are L3
within their stated contexts; absent or unproved inputs do not erase the
formula, and CPU execution is distinct from external operation completion.

The immediate IN/OUT handlers check immediate decoding, choose AL/AX for
early CPUs and zero-extend the immediate port. DX forms use the word port.
_p_input validates access, requests one bounded provider transfer, then
publishes the width-specific accumulator; _p_output snapshots that width
before transfer. Both call complete_port only after success. Full odd-port,
word-transfer, wrap, provider rejection and no-duplicate completion proof
remains an I/O receiver; one width-valued provider call is not proof of
the physical bus-cycle sequence. The existing port_io normal-form matrix
contains 8086/186/286/386 but omits 8088.

WAIT currently checks TS/MP and the FPU pending-exception state, then invokes
x86_fpu_complete_wait. The timing owner adds last_wait_ticks to the selected
CPU base. The FPU completion helper consumes remaining duration and clears
busy; S2's delayed-retirement inspection already identifies the enclosing
completion owner. Thus it would be wrong to claim there is no wait accounting
or add a second completion loop. What remains unqualified is the early
TEST sampling contract: 3+5n is not automatically proved by adding an arbitrary
remaining external duration. TEST availability, sampling/rounding, interrupt
recognition during wait and completion visibility need one early-family
contract/trace receiver, without mixing later MP/TS/#MF rules into it.

FPU_ESCAPE checks opcode/ModR/M fetches and decodes the effective address.
Its CPU metadata accepts D8-DF independently of particular x87 arithmetic
support. But when FPU is NONE, escape_dispatch returns CONSUME_NONE; no
operand read, extension_command or begin_command is then performed. The
original early ESC prose explicitly requires a memory operand read and
discard, unlike register ESC. This is an observable CPU-bus omission, not
permission to declare the instruction absent because a coprocessor is absent.
The current 8087 subset reads only for its selected operation semantics;
the generic handoff path likewise does not establish that CPU memory cycle.
The repair receiver must distinguish the CPU's source-defined read from
coprocessor operand consumption, confirm width/bus order and failed access,
and retain one command path. It must not double-read every full FPU operand
or apply early bus behavior to later CPUs without their manuals.

HLT's handler marks halt; refresh skips instruction execution while halted
but still calls ExecInt. That is an actual wake path, not a nonexistent one.
S2 arbitration/reset receivers still govern its eligibility and delivery.
The shared privilege branch records GP then continues to mark halt, subject
to ExecFinal rollback; outgoing state and delivery must be tested before
calling the temporary update a guest-visible defect. NOP's dedicated handler
and source cost remain separate from host waiting. XLAT reads the selected
source segment at BX+unsigned AL into AL, preserving the other registers;
its word-offset overflow and source-override boundaries remain an explicit
receiver. The inspected inc_dec_final_group XLAT cases select 186/386 and
nonwrapping offsets, not proof of both early boundary cases.

Existing FPU interface tests deliberately run no-FPU WAIT and register
escapes across the profile range, including 8088; these are not absent tests.
They do not establish no-FPU memory ESC's required read. Four selected existing
tests (port_io, fpu_interface_state, control_state, inc_dec_final_group) pass
once per width: x64 4/4 in 0.07 seconds and x86 4/4 in 0.08 seconds. Their
passing status does not qualify the above missing source/bus contexts.
No timing grade is changed, and no new unupgradable L1 is established.

## F05 Control Transfers And Branch Outcome

Fresh rendered Intel 1981 PDF 63-66 / printed 2-44 through 2-47 defines
near/far CALL/RET, conditional predicates, LOOP/JCXZ, INT/INTO and IRET.
CALL saves the following instruction address; far calls save CS and IP.
RET removes IP and optionally CS, then applies its unsigned parameter-byte
adjustment. Conditional transfers depend on FLAGS, not whether the resulting
address differs from fallthrough. Printed 2-45 explicitly permits transfer
to the first byte of the following instruction at zero displacement.
Printed 2-46 defines all sixteen distinct short conditions and INT's frame;
printed 2-47 defines IRET's IP/CS/FLAGS restoration. Software INT and NMI
do not issue interrupt-acknowledge bus cycles.

The sixteen short Jcc handlers match the inspected flag predicates, including
CF-or-ZF unsigned comparisons and SF-versus-OF signed comparisons. _e_jcc
sign-extends the selected displacement; _kec_jmp_near masks word destinations.
_e_loopcc decrements the address-width count and publishes it after checking
the taken destination; LOOP does not alter FLAGS. JCXZ tests count without
decrement. Direct near CALL decodes its displacement before saving the current
next IP; the common near/far helpers validate their destination, push the
return values and then publish the transfer. Far RET peeks IP/CS and adds
frame bytes plus parameter bytes once. Near RET reads the return IP, updates
SP, validates the destination and then adds parameter bytes. These normal
paths are not evidence of provider failure atomicity.

_ser_int_real saves FLAGS, clears IF/TF, saves CS/IP, reads the vector and
publishes the target. Its use of the common FLAGS image carries the confirmed
S2 early high-bit discrepancy into interrupt frames. _e_iret reads IP/CS/FLAGS,
validates the staged CS destination and uses the common flag loader. S2's
early IRET inhibition and later NMI/priority receivers remain open. Multiple
successful stack writes followed by a failed vector access cannot be declared
undone merely because ExecFinal restores registers. The thirteen-call failed
decoder inventory already includes INT, RET immediate, far CALL, JCXZ and
JMP immediate; this family needs the same coherent propagation receiver,
not isolated handlers or a second stack implementation.

### Zero-Displacement Timing Defect And Similar-Issue Sweep

Rendered PDF 75 / printed 2-56 supplies Jcc 16-or-4 clocks, INT 52/51,
INTO 53-or-4 and IRET 24, with documented transfer additions. The actual
early Jcc timing selector compares final EIP to fallthrough. For 74 00 with
ZF set and starting IP zero, _e_jcc takes the branch to IP two, which equals
fallthrough. The timing selector therefore chooses 4, not the taken 16.
This is a direct source/code counterexample, not a newly executed probe or
a functional wrong-target claim. Zero displacement makes both outcomes share
an address; resulting-PC equality cannot identify the branch decision.

The same construction appears in core_machine_control_stack_short_branch_taken
(8088 Jcc and early LOOP/JCXZ), the direct 286 short-Jcc selector, 386 short
and near Jcc selectors, 386 JCXZ selection and the compatibility taken-branch
surcharge. The source requirements of those later families must be checked
in their batches, but the common inference is already insufficient. Repair
should retain one authoritative executed branch outcome for existing timing
consumers, or reuse one established predicate mechanism; do not create a
parallel decoder or duplicate sixteen condition tables. The implementation
choice and cross-family zero/nonzero/wrap/prefix regression matrix require
concrete Shared review. Existing exact literals are not downgraded: the
defect is selecting the wrong source row, not absence of a clock formula.

The inspected branch smoke uses displacement two for all sixteen taken and
not-taken examples and omits 8088; the inspected early timing-ledger taken
example uses displacement one. They do not challenge this counterexample.
Far-transfer normal matrices also omit 8088. In contrast, software_int_state
and iret_s51_state explicitly include both early CPUs, and the latter includes
stack wrapping. Preserve that existing coverage rather than declaring it
missing; its flag masks do not independently qualify S2's outgoing image.
Five selected existing tests (control_transfer_branch/near/far,
software_int_state, iret_s51_state) pass once per width: x64 5/5 in 0.15
seconds, x86 5/5 in 0.17 seconds. Complete source-form/transfer/fault/timing
qualification remains open; this audit does not repair any Shared path.

## F07 Additional Count, Flag And Formula Reconciliation

The actual byte/word ROL/ROR helpers loop the full early count and publish
the last rotated-out bit as CF. RCL/RCR include incoming CF and avoid the
later width-plus-one reduction on both early profiles. Their count-zero
branches preserve guest flags and value. SHL/SHR/SAR also iterate the full
early byte count; SHL zero-fills left, SHR zero-fills right and SAR retains
the sign through the signed-width operation. Nonzero shifts update SF/ZF/PF,
not other control flags. Single-count OF is SHL's resulting sign XOR CF,
SHR's original sign, SAR zero; multi-count OF remains undefined. The unused
undefined metadata noted above does not rewrite outgoing guest FLAGS.
These inspected operations do not justify an exact multi-count OF/AF oracle.

Rendered Intel 1981 PDF 83-84 / printed 2-64 through 2-65 gives ROL/ROR
and SHL/SAR register-one 2, register-CL 8+4 per bit, memory-one 15+EA and
memory-CL 20+EA+4 per bit. cpu_timing_model.c selects those values and uses
the complete old CL for early CPUs. Word memory read-modify-write additions
are charged for two transfers. This literal/formula reconciliation is L3,
not proof of complete bus observations, all seven original table rows or
failure retirement. Do not replace full CL by a masked count merely to reuse
later CPU timing. The early CL=FFh formula yields 1028 register clocks;
that arithmetic consequence is not a newly executed CPU measurement.

cpu_rotate_smoke.c has substantial 386 byte/word/dword register/memory
and immediate/one/CL matrices, plus 386 zero-count and undefined-OF handling.
Its early full-CL matrix uses byte CL=21h and checks values, not final CF;
it omits 8088. Those exact inspected contexts should be expanded coherently
in the repair receiver, not replaced with another parallel fixture. The
existing rotate test passes once per width in 0.05 seconds each. It does not
qualify both-early word, zero/extreme-count, transfer or failure contexts.

### Remaining Rotate Rows, OF And 8088 Transfer-Width Discrepancy

Fresh rendered PDF 82 / printed 2-63 confirms RCL and RCR's same four
clock rows (2, 8+4*count, 15+EA, 20+EA+4*count). PDF 85 / printed 2-66
confirms SHR. Together with the previously inspected ROL/ROR/SHL/SAR rows,
all seven named early operations now have visually checked base formulas.
PDF 58 / printed 2-39 defines single-rotate OF by original-versus-result
sign change and multi-rotate OF as undefined. The code's ROL/RCL result
MSB XOR outgoing CF, ROR result top-two-bit XOR and RCR original MSB XOR
incoming CF implement that single-bit rule. In RCR the OF calculation
intentionally precedes rotating; moving it after the carry change without
preserving incoming CF would be a regression. None of these count-one
checks justify an exact multi-count OF value.

The 8088 primary selector reveals a separate additive-term defect: every
memory D0/D1/D2/D3 operation adds eight clocks, regardless of opcode width.
The source footnote adds four per word transfer, not per byte transfer.
Thus byte SHL [direct16],1 (D0 /4, EA=6) selects 29 rather than 21 clocks;
word D1 /4 correctly needs 29. The base operation formula itself is not
missing or L2: its byte/word transfer modifier is wrong. Existing retirement
observation coverage uses D1 and expects 29, so it does not expose the byte
counterpart. The earlier rotate test also omits 8088. This is a direct
source/selector counterexample, not a newly executed runtime trace.

The similar-width sweep finds another fixed word addition: 8088 XLAT
selects 15, while rendered PDF 86 / printed 2-67 gives 11 and defines the
extra four only for word transfers. XLAT's actual handler reads one byte.
The existing 8088 retirement observation test explicitly expects 15; its
green result therefore confirms a stale oracle, not the original source.
The other inspected fixed terms in this primary 8088 branch are segment
register word loads/stores, two-word LDS/LES and the initial ESC memory
transfer, whose CPU bus width/failure evidence remains with F14. Do not
delete all 8088 additions indiscriminately or apply a global byte penalty.

The coherent repair receiver derives these additions from actual operand
width and transfer count at the existing timing owner: Group-2 byte zero,
word two; XLAT byte zero. Preserve the original base rows and complete
paired byte/word, one/CL, register/memory, count-zero/extreme and segment
override regressions for both early CPUs. XLAT's address-wrap repair is
independent of this clock correction but belongs to the same complete
instruction acceptance. No Shared implementation or test is changed here;
no L3-to-L2/L1 downgrade is proposed.

## F09 XLAT Word-Offset Counterexample

Fresh rendered Intel 1981 PDF 31 / printed 2-12 explicitly defines segment
offsets as unsigned 16-bit quantities with modulo-64K addressing. PDF 51 /
printed 2-32 defines XLAT's BX table base and unsigned AL byte index. In the
below-386 handler, BX+AL is passed to _m_read_logical as an unmasked promoted
sum. The 386 address-size-two branch does the same. The memory helper does
not truncate that initial offset; its legacy wrap helper handles transfers
crossing FFFFh only when their starting offset is already at most FFFFh.
Real-mode data translation also permits a full-width offset without selecting
the CPU/address width. Thus BX=FFFFh and AL=1 submits offset 10000h, not 0000h.
This is a source/code address counterexample; a runtime distinction probe
with different bytes at both offsets remains required before repair delivery.

The similar-issue inspection confirms ordinary 16-bit ModR/M base/index and
displacement sums already use X86_CPU_MASK_U16, and string index updates use
word fields. Do not truncate all logical accesses: 386 address-size-four
must remain full width, and segmentation/transfer wrap is a separate fact.
The coherent repair receiver masks XLAT's computed effective offset according
to selected address width at its owning calculation, and sweeps other implicit
address sums for the same omission. It covers all below-386 profiles plus
386 word/dword selection and source overrides; later manual reconciliation
remains in the corresponding family batch. Existing inspected XLAT tests use
nonwrapping sums and do not refute this counterexample. No Shared code or
timing grade is changed here.

## F08 Exact Repeat Formula And Transfer Reconciliation

Fresh visual inspection of Intel 1981 PDF 72, 79, 80, 84 and 85 / printed
2-53, 2-60, 2-61, 2-65 and 2-66 confirms the five primitive/repeat rows:
MOVS 18 / 9+17*n, CMPS 22 / 9+22*n, LODS 12 / 9+13*n,
SCAS 15 / 9+15*n and STOS 11 / 9+10*n. The existing early repeat ledger
matches each literal. Here n means elements actually executed, not initial
CX when comparison terminates early. These exact formulas remain Manual-L3;
LODS having a slower repeated per-element cost than its primitive is not
grounds to replace the original 13 with 12.

The inspected selector charges setup plus one iteration on the first element,
iteration only on continuation, and setup only for initial zero count. It
recognizes continuation by saved CS/IP, opcode, repeat kind and selected
operand/address sizes. For uninterrupted unprefixed execution these selected
costs sum to the original formula. This is code/formula reconciliation, not
an executed total-clock or interrupt-restart measurement.

The original footnotes add four clocks per odd-address word transfer on
8086, or per word transfer on 8088. The transfer plan counts two for MOVS/
CMPS and one for LODS/SCAS/STOS. The 8086 modifier checks saved SI and DI
independently according to source/destination use; the 8088 modifier uses
the word-transfer count regardless of alignment. Both omit transfer additions
for zero-count repeat. For example two uninterrupted, aligned REP MOVSW
elements select 43 clocks on 8086 and 59 on 8088. These are derived selector
results, not a fresh runtime probe or proof of actual bus transactions.

Two accounting contexts remain explicitly unqualified: interrupt/return
restart of the continuation marker, and segment-override charging. The
current string modifier adds two segment-override clocks on every source
element, including continuation and zero-count paths. The original table
lists a two-clock prefix separately; determining its repeated/restarted
scope requires the prefix/repeat authority and total-clock trace rather than
assuming a per-element charge or silently deleting it. Keep this receiver
with F08's prefix restart discrepancy and the single existing timing owner.
No Shared implementation, test, timing grade or completeness claim changes.

## F08/F14 Retirement Context Versus Asynchronous Delivery

The continuation investigation exposes a broader context-lifetime discrepancy.
core_machine_run in x86/core/machine.c calls execution_refresh, then selects
instruction timing and captures retirement eligibility. Refresh executes
ExecIns followed by ExecInt unless debugger pause returns first. NMI and
INTR each invoke ExecInit again; _debug_deliver_trap does the same. ExecInit
replaces oldcpu, CS/IP observation, prefetched opcode bytes, operand and
address prefix state, REP state, memory flag and arithmetic operands. None
of those successful delivery paths restores the completed instruction data
before the runner's timing selection. INTR's flagIgnore is not a timing
guard: the inspected timing and Core runner do not consume that flag.

A concrete code-path counterexample is ordinary MOVSB followed by NOP,
with a pending NMI accepted after MOVSB and valid real-mode vector/stack.
MOVSB completes its data/index updates, but the NMI ExecInit observes the
following NOP and replaces the original timing inputs. The later selector
can choose NOP's three clocks rather than MOVSB's eighteen. The NMI frame
delivery does not turn this context into an NMI timing row. This is a direct
control/data-flow counterexample, not a newly executed runtime measurement;
a combined retirement/bus/time trace remains required before repair delivery.
Rendered Intel 1981 PDF 79 / printed 2-60 separately gives NMI fifty clocks
and five transfers, with the word-transfer additions in its footnote. Do
not conflate these delivery clocks with the completed instruction's cost.

For an interrupted REP element the same ExecInit clears decoded REP state
while retaining bytes prefetched at the rewound instruction address. Thus
the immediate problem is not merely whether to charge another setup after
IRET: the just-completed element's timing identity can already be lost.
An ordinary executed IRET later passes the non-string cost selector and
clears the repeat-continuation marker; that reconciles its existing marker
expiry, but does not repair the earlier accounting handoff. NMI, INTR and
trap paths across all five families, HLT wake with no new instruction,
failed delivery, and delayed external-cycle completion belong to one
retirement/delivery receiver with S2's boundary arbitration and publication.

The repair proposal is to finalize the completed instruction's timing inputs
at the CPU instruction boundary before asynchronous delivery can reuse the
decode context, and retain that one result until the existing Core retirement
publication consumes it. Give asynchronous delivery its own source-qualified
time contribution and distinguish it from a newly retired instruction.
Do not decode again, clone a second CPU owner, count interrupt entry as an
executed opcode, or move timing ownership into a diagnostic callback. The
precise internal handoff and event timing policy require concrete Shared
review; no new public API or implementation is admitted by this audit.

Inspected t359_s3 timing tests use software INT/INT3/INTO and IRET, not
post-instruction pending NMI/INTR. The 8086 timing ledger covers ordinary
REP and segment-override totals, including its existing three-element
segment REP expectation of 66; that expectation does not independently
settle prefix charging scope. Retirement observation tests cover early REP
FIRST/CONTINUATION and observer rejection, not this delivery-context loss.
These three existing tests pass once per width: x64 3/3 in 1.06 seconds,
x86 3/3 in 1.24 seconds. Preserve their genuine coverage while adding the
missing boundary matrix in the coherent repair, rather than weakening them.
No timing-grade downgrade or whole-CPU qualification follows from this result.

## Cross-Edition Source Conflict Disposition

The secondary source is Intel's 1985 8086/8088/80186/80188 User's Manual,
owner archive identity intel-8086-8088-80186-80188-users-manual-1985.pdf,
SHA-256 2516D66CC75076D9AC9EE048E8420C09C35655FB25ED34DDA6351A3EA4E0AFFF.
Fresh visual inspection of its PDF 42 / printed 1-26 shows DAA's OF cell
still marked X. It therefore reproduces the earlier table/prose disagreement,
not an independent resolution. Keep the implementation's undefined-OF
disposition tied to the explicit 1981 prose; do not add a precise DAA OF
test merely because two editions repeat the same table cell. No timing
grade changes from that flags-source conflict.

The inspected 1985 PDF 137 / printed 1-121 divide-error description says
quotient overflow produces type 0, without a numeric signed lower bound.
Its IDIV reference table on PDF 44 / printed 1-28 is a range-timing table,
not a negative-quotient specification. This cross-edition check does not
independently prove either -128/-32768 acceptance or -127/-32767 exclusion.
The previously recorded original 1981 numeric/code discrepancy remains
visible, but must not be described as a demonstrated silicon result or an
already reconciled cross-edition rule. Resolve that source boundary before
accepting the proposed generation-specific arithmetic change; preserve the
separate, documented divide-error return-IP receiver from S2.

Rendered 1985 PDF 137 Table 1-42 separately supplies INTR 61 clocks,
NMI 50, single-step 50, software INT with vector 51, INT3 52 and INTO 53.
This corroborates a distinct asynchronous-delivery timing contribution,
not the use of the next prefetched instruction as its clock identity. Its
adjacent prose describes TF single-step after the next instruction and
NMI edge triggering; pulse sampling, priority, transfer additions and
family-specific inhibition remain their already named source/bus receivers.
Do not infer later-generation arbitration rules from this early-family page.

## F02/F03 Transfer-Width Similar-Issue Sweep

The unified primary transfer-plan helper first rejects non-memory or non-word
shapes. Its existing ALU memory destinations, XCHG, INC/DEC, NOT and NEG
then select two word transfers; MOV selects one, while CMP, TEST and ALU
memory sources select one. The primary timing owner applies four per selected
word transfer on 8088 and the odd-word term on 8086. Therefore the inspected
byte forms in this unified path do not have Group-2's unconditional eight-clock
addition. This is a bounded path inspection, not a statement that all possible
encoding or bus-error contexts are qualified. TEST memory-immediate's original
dash in the transfer column remains the already recorded source-table conflict.

The opcode classifier preserves destination/source distinctions, fixed-word
short register encodings and memory width bits for FE/FF, F6/F7, 80/81/83
and MOV/XCHG. Later-only 69/6B, SETcc or unsupported alias forms cannot become
early source forms merely because this classifier knows their shape. Existing
8088 retirement examples check a word memory-source ADD at 19 clocks and
a word memory-destination ADD at 30 clocks; their passing assertions do not
replace paired byte coverage or provider transaction traces.

MOFFS deserves an explicit non-defect disposition: although the generic
word-transfer helper does not name MOV_MOFFS_READ/WRITE, the 8088 primary
selector handles A0-A3 before it reaches that helper. It supplies base ten,
adds four only for A1/A3 word forms and two for an admitted segment override.
The 8086 legacy owner similarly uses base ten and word-only odd-address
addition. Do not introduce a second MOFFS calculation or report an unallocated
fallback from the isolated helper omission. Existing 8086 odd read/write
tests expect fourteen clocks and cover those real paths.

## F03 INC Cross-Edition And Encoding Boundary

Rendered 1981 PDF 74 / printed 2-55 gives INC reg16 two clocks and one byte,
reg8 three clocks and two bytes. Fresh rendered 1985 PDF 44 / printed 1-28
instead gives reg16 3(3), still one byte; these cells are genuinely different,
not OCR's substitution of B for 8. The code selects two for early word
register INC/DEC and three for byte registers. Its one-byte INC word choice
agrees with the selected 1981 table, so changing it to three solely because
the newer compilation says three is not an audited repair.

The two-byte FF /0 or /1 register-word form must also be distinguished from
the one-byte register form: the short-form row's byte count does not itself
prove equal clocks for that alternate encoding. Keep that encoding-specific
source receiver alongside the cross-edition table difference; do not treat
operand width alone as proof that every encoding shares one source row.
No source-grade or Shared implementation changes result from this sweep.

## F06 Operand Construction And Host Arithmetic Sweep

The C11 build and lib_u16 DX declaration expose three unsafe word-concatenation
expressions in cpu_instructions.c: DIV's input at 6586, DIV's result record at
6611 and IDIV's input record at 6699. On the supported 32-bit-int targets,
lib_u16 promotes to signed int before `dx << 16`. For DX at or above 8000h,
the mathematical result cannot be represented by that signed type. A mask or
cast outside the shift does not change the type of the shift itself.
Clang's [UBSan reference](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html)
identifies signed left-shift overflow in C as a shift check. This is a C-level
implementation defect deduced from the expression and declared types, not a
newly observed production crash or a sanitizer execution result.

The input example DX:AX=8000:0000 with unsigned divisor one must request
guest quotient overflow, but the host expression is already undefined before
that check. A valid unsigned division can expose the result-record expression
too: DX:AX=0000:FFFE divided by FFFF gives AX=0000, DX=FFFE. IDIV's actual
word dividend construction already widens DX before shifting; its earlier
operand record does not. For negative word dividends such as FFFF:8000,
that record still evaluates the unsafe expression before the correct division.
No successful-result or exception trace is claimed from these deductions.

The CPU-tree literal 16/24/32-bit shift sweep finds the remaining DX:AX,
EDX:EAX and descriptor concatenations explicitly widened before shifting.
This bounded sweep is not proof about every variable-count shift or every
integer conversion in the CPU. MUL's operand record is lib_u64, so its
unsigned product does not have the same promoted-word multiplication issue.
Signed byte/word IMUL products fit the promoted signed-int range; the dword
IMUL path explicitly widens both signed operands to lib_i64 first.

The proposed repair is three expression-local unsigned widenings before the
shift, preserving the existing handlers, result records and fault checks.
No new arithmetic abstraction, generation branch, API or board workaround is
needed. This mechanism is separate from the unresolved early signed-quotient
source boundary and does not justify changing that boundary implicitly.
The repair receiver covers all five users of the shared word helpers, valid
high-bit remainders, negative dividends, zero/overflow and unchanged register
publication on rejected division. Existing ordinary Group 3 green recipes do
not establish that the C expressions are defined under optimization.
Shared edits and new regression recipes still require concrete owner review.

## F07 Host Shift And Failure-Publication Reconciliation

The early Group 2 helpers use the declared lib_u64 operand/result records.
ROL/RCL/SHL shift that unsigned record one bit at a time and mask back to
the guest width after each iteration; ROR/RCR/SHR similarly shift by one.
Thus an early CL count of 255 never becomes a host shift by 255, and the
word result does not undergo the signed-int promotion found in DX:AX
construction. The common count selector preserves full CL for both early
profiles and masks later families to five bits. This is an inspected
non-defect disposition for the host shift mechanism, not complete timing or
all-family count qualification.

SAR casts the narrowed result to the signed guest width before shifting by
one. Under C11 a negative signed right shift is implementation-defined,
not the same undefined signed-left-shift overflow as the three F06 sites.
The existing rotate tests do not independently establish the supported
compiler contract for every signed width. Preserve that portability proof
receiver; do not label every negative SAR as an observed arithmetic defect
or silently replace its source semantics while auditing.

INS_D2 checks effective-address decode and operand read before the arithmetic
helper, then checks the final write. A failed final memory write occurs after
the helper has changed FLAGS; instruction-boundary rollback must therefore
be included in the failed-write regression receiver. A checked helper call
alone is not proof of failure atomicity. The early byte/word Group 2 matrix
must distinguish read failure, write failure, zero count and successful
register/memory execution, alongside the existing transfer-width receiver.
No new failure trace or rollback defect is asserted from call ordering alone.

The analogous literal-bit sweep also finds later-only signed-one shifts:
bit-operation mask construction at 16484, both BSF/BSR scan directions through
the dword loop at 16558, and SHLD/SHRD dword loops. Their bit index can be
31, and applying X86_CPU_MASK_U32/U64 after `1 << index` does not widen the
left operand beforehand. Existing bit-scan recipes explicitly expect index
31; double-shift recipes include dword counts 1-31. Passing those recipes on
an ordinary build does not prove the C expressions defined under optimization.
The S6 80386 receiver owns the complete bit-mask sweep, unsigned-before-shift
proposal and register/memory/count boundary regressions. These opcodes are
not early-family coverage, and no Shared implementation is changed here.

## Failure-State Restoration Owner And Split-Write Boundary

The Group 2 failed-write lead now has a concrete normal-path owner disposition.
ExecInit saves the whole t_cpu in oldcpu. CPU_TRACE_CHECK_RETURN exits a helper
when the instruction exception mask becomes nonzero; it is not a status-value
return protocol. A bus write failure in _kma_write_physical publishes CANCEL,
sets the internal CE exception and does not publish COMMIT. ExecIns subsequently
calls ExecFinal. For an undeliverable internal error, that path publishes the
saved fault snapshot, restores `cpu_state = fault_cpu` and requests stop.
Deliverable exceptions similarly restore the saved CPU before exception entry.
Thus an arithmetic helper changing FLAGS before the checked write is not, by
itself, an absent register/FLAGS rollback defect. Keep the regression receiver,
but do not repair it with a second handler-local CPU snapshot.

This disposition does not mean every side effect rolls back. _kma_write_linear
prepares both page translations before split data writes, distinguishing a
second-page validation fault from a later provider failure. It then performs
the two physical writes in order. _kma_write_logical's early segment wrap also
issues two ordered sub-writes. A provider may accept the first and reject the
second; CPU restoration has no reverse-write operation for the first. The
hardware-defined bus sequence, provider failure contract, MMIO effects and
partial RAM publication must be reconciled separately before an atomicity
requirement or repair is claimed. Do not add an unconditional memory undo log:
it would not be a valid inverse for arbitrary device writes.

The existing cpu_bus_fixture fail_transfer rejects every transfer before its
copy. cpu_execution_bus_smoke sets that flag for its failure case; it does not
selectively accept a first data write and fail a second, nor distinguish a
Group 2 read from its write. The remaining receiver therefore names selective
failed-write and wrap/split contexts, not an absence of all bus failure tests.
REP's completed prior elements retain their previously recorded boundary;
this register-restoration finding does not erase that instruction's separate
failed-element and delivery receivers. This is source inspection, with no new
probe execution, Shared code or test changes.

## Early Form Admission Versus Undefined-Encoding Policy

The inspected primary metadata gate requires 80186 for 60-62, 68-6F,
C0/C1 and C8/C9; it requires 80286 for 63 and 80386 for 64-67. ExecIns
checks that metadata before dispatching the primary handler. This confirms
the current implementation does not successfully execute these later forms
on either early profile. It does not prove what early silicon does with the
same byte sequence when that encoding was not defined in its manual.

UndefinedOpcode restores oldcpu and sets the UD exception. ExecFinal can
then deliver vector six in real mode. rotate_test_8086_immediate_rejection
explicitly expects that UD mask, unchanged registers/FLAGS and zero EIP for
all C0/C1 extensions on 8086, but does not run the corresponding 8088 matrix.
The historical List 1's generic undefined-encoding row calls for each profile's
source-defined UD path. Its existence is not independent early-hardware
authority for that exact exception behavior. Reconcile undocumented encoding
policy and source-defined behavior separately from supported-form admission;
do not qualify early UD merely because that assertion is green, or silently
start executing undocumented aliases to make a test pass.

There is also a concrete source-record/code ambiguity for 0F. List 1 calls
POP CS 8086-only. INS_0F selects POP_CS for both profiles recognized by
has_8086_semantics, including 8088; it uses escaped opcode space only from
80286 and rejects the 80186 case. The inspected early timing path and
machine_8086_timing_manifest_runner include a POP-CS recipe, but a recipe
and implementation comment do not resolve the source record's wording.
Both-early POP-CS legality, stack/CS publication, inhibition and timing must
be reconciled before accepting or removing either CPU's path. This is an
explicit source-record receiver, not a demonstrated new silicon defect.

The executable metadata and the lexeme scanner also have different jobs:
the scanner rejects LOCK in its supported lexeme route, while architectural
execution processes prefixes and later handler checks. Lexeme rejection is
therefore not evidence that every such guest byte sequence faults. Preserve
the existing LOCK source/generation receiver and avoid a second decoder or
expanding the diagnostic scanner into an architectural authority.

## Original POP And LOCK Page Resolution

Fresh rendered 1981 PDF 81 / printed 2-62 Table 2-21 labels the segment POP
operand `CS illegal` under the combined 8086/8088 chapter. Its eight-clock
segment row cannot therefore serve as direct Manual-L3 evidence for POP CS
on either early CPU. The implementation's compatibility form and historical
List 1's 8086-only statement require a separate documented/reference model;
the page does not prove that the undocumented byte executes identically on
both chips or that it raises a particular exception. Keep that distinction
before accepting, removing or grading the existing POP-CS path. No broader
POP timing row or supported segment form is downgraded by this source check.

Fresh rendered PDF 67 / printed 2-48 describes LOCK's assertion during the
following instruction in maximum mode. PDF 36 / printed 2-17 additionally
states that interrupts are unaffected, consecutive locked instructions have
an unlocked interval, and a locked repeated string holds the signal through
the block. These are direct early bus/lifetime requirements, not the 386
memory-RMW whitelist. Minimum/maximum mode and board arbitration remain
distinct composition facts; a prefix timing addition alone is not bus-lock
implementation.

The current pre-386 PREFIX_LOCK branch avoids the later opcode whitelist and
advances the prefix, with the separate 286 IOPL check. It does not set
flagLock; ExecInit clears that field, and the instruction observation copies
it as lock_prefix. Thus the inspected early prefix is not preserved by that
observation field. The general ModR/M helper's register-plus-flagLock UD check
does not demonstrate an early LOCK-register failure when the field stays
clear. Do not misclassify that dormant check as such a demonstrated defect.
The remaining LOCK receiver must establish actual bus/arbitration publication,
REP lifetime and instruction boundaries before proposing one owned contract;
no new API or production lock mechanism is approved in S3. This source/code
finding concerns missing proof/representation, not a claim that a host mutex
would simulate the signal or that every LOCK timing row must become L1.

## LOCK Consumer And Arbitration Trace Inspection

The follow-up follows production consumers, not just the missing observation
bit. CPU timing independently scans the captured prefix bytes and applies the
existing two-clock early LOCK term when a base source row is allocated.
Timing-input capture also knows LOCK. Core retirement observations copy the
decoded lock_prefix, and machine timing-row keys compare it. These consumers
are clock/observation facts, not a bus-arbitration control path.

The public CPU bus provider carries memory/port transfers, interrupt query/
acknowledgement and extension issuance. Its external-cycle provider carries
BEGIN/COMMIT/CANCEL/OVERLAP_DECLARE, address, width, direction and provenance;
neither inspected contract carries a lock interval. Searches of x86/core and
ibmpc find no bus-lock consumer linking prefix lifetime to DMA eligibility.
Core scheduler's DMA grant paths use the existing transaction hold request/
acknowledgement for 286/386; the early branch directly calls attachment DMA
advance. board_advance delegates that to dma_bus's ordered advance loop.
No inspected branch checks an early instruction or repeated-block lock state.

The transaction owner prevents simultaneous ownership of an individual
transaction and hold grant. That is useful existing machinery, but its single
transaction exclusion cannot prove the manual's locked instruction/block
lifetime. In particular, guest time may advance between REP elements, so a
single-threaded host execution or synchronous operand callback is not evidence
that DMA cannot run between those elements. A complete Core/DMA/bus trace is
still required before claiming an observed incorrect interleaving.

The coherent receiver is CPU-owned prefix/operation lifetime feeding the sole
Core arbitration owner, with board minimum/maximum-mode input and DMA wait/grant
behavior at that boundary. Reuse current transaction ownership where its
contract is sufficient; do not create a host mutex, per-App LOCK flag, parallel
DMA scheduler or diagnostic-driven lock. Its verification must include
ordinary locked instruction, locked REP block, interrupted/restarted block,
adjacent locked instructions, pending DMA and failure/stop release. This is a
proposed architecture/proof batch requiring concrete Shared review, not an
approved API addition or a claim that instruction's exact prefix clocks became
L1 merely because arbitration is unqualified.

## Consolidated Early-Family Receiver Matrix

These are finite mechanism/proof batches inside T544, not new S identifiers,
Shared approval or accepted qualification rows. Both 8086 and 8088 remain
required even where one existing regression omits 8088. The section narratives
above retain exact source, handler and test context; this matrix prevents a
later checkpoint from hiding any of them.

| Receiver | Complete affected batch | Required disposition before qualification |
| --- | --- | --- |
| Source/form reconciliation | F01 AAM/AAD decimal versus nondecimal encodings; DAA undefined OF; F02 TEST encoding and logical immediate aliases; F03 alternate INC/DEC encodings/cross-edition clocks; F04 both-early POP CS; F06 signed quotient minimum; undefined encoding and LOCK policy | Resolve each legal form and source conflict independently; do not implement an inferred early-IDIV generation rule or qualify early UD from the existing oracle alone. |
| Checked decode propagation | Thirteen direct decoder sites in F01/F02/F03/F05 | Prove failure origins and publication for each site, then repair one propagation convention while preserving original handlers; include LEA and immediate control transfers. |
| Failed element propagation | Ten below-386 string element handlers, source/destination reads/writes and all repeat termination conditions | Retain completed earlier elements; prove failed current-element state and status propagation without a second string loop. |
| Effective offset width | XLAT byte access, selected segment and both address sizes where implemented | Truncate only the computed effective offset according to selected address width; prove word wrap with distinct bytes and preserve dword addressing. |
| Host arithmetic width | Three DIV/IDIV DX:AX concatenations and signed SAR compiler contract | Widen before left shift; prove valid high-bit remainders and rejected quotients; distinguish implementation-defined right shift from left-shift UB. Later dword bit masks are S6, not early forms. |
| Branch outcome identity | Jcc, LOOP/LOOPE/LOOPNE and JCXZ, zero/nonzero displacement, taken/not taken | Select timing from actual execution outcome rather than final-PC inequality; sweep later callers and compatibility additions in the same mechanism. |
| Timing transfer width | Group 2 byte/word read-modify-write, XLAT byte read and existing unified ALU/MOV transfer plan | Remove only demonstrated wrong byte surcharges; retain genuine 8088 word and 8086 odd-word additions; MOFFS is explicitly not an omitted allocation. |
| Retirement/delivery handoff | Every family asynchronous NMI/INTR/trap boundary, REP continuation, HLT wake and delayed external cycle | Preserve completed instruction timing before decode-context reuse; account separately for source-qualified delivery, never bill the next prefetched opcode as the completed instruction. |
| Repeat source accounting | All five string primitives, CX zero/one/multiple, compare termination, segment override and interrupted multi-prefix restart | Reconcile prefix charging and early saved-prefix behavior; preserve proven setup/iteration formulas and completed-element boundaries. |
| External CPU bus issuance | Byte/word immediate/DX I/O, port wrap/provider completion, WAIT TEST sampling, memory/register ESC with/without coprocessor, LOCK instruction/REP/arbitration lifetime | Prove signal/sample/width/failure sequence; issue CPU-mandated ESC read without duplicating coprocessor transfer; connect one CPU lock lifetime to the existing Core arbitration owner, never a diagnostic/host lock. |
| Architectural state/delivery | F03/F04/F05/F10 segment inhibition, FLAGS images, reset, return IP, priority and HLT wake | Consume the full S2 receivers, not separate instruction-specific workarounds; preserve family-defined state and diagnostic origin. |
| Ordered memory side effects | Data movement, stack/frame, pointer loads, Group 2 and wrapped/split accesses; selective provider rejection | Distinguish CPU restoration from RAM/MMIO effects and page validation from provider failure; prove the contract before proposing rollback, with no generic undo log. |
| Defined normal state regression | All eleven early-family partitions, documented input/defined FLAGS and byte/word register/memory forms | Retain existing useful recipes, identify every absent early-profile/boundary context and add owner-local matrix proof only after Shared admission; green catalogs alone cannot accept the family. |

No receiver has been transferred out of T544 or marked repaired by this
matrix. The final executor review reconciles all eleven early-family
partitions against List 1 and the inspected handler/regression owners: normal
source/code facts are recorded, while every unresolved source, form, bus,
state, timing and regression class retains a named row here. This is complete
audit inventory disposition, not complete semantic/timing qualification.
The fresh unit verification below is delivery evidence, not a substitute
for the pending source/context proof.

## Fresh Complete Unit Verification

The fresh S3 x64 complete repository-only unit aggregate passes 506/506,
61.92 seconds, four jobs and a 300-second deadline, using the retained
default receiving cache and its configured CTest. An initial relative-path
invocation failed before any test started because the bounded runner resolved
the test directory relative to its own working directory. The actual aggregate
uses the resolved absolute cache path; this is not a repeated successful run.
The x86 aggregate also passes once, 506/506 in 62.56 seconds through its own
retained receiving cache. Both owned test process handles exited successfully.
This completes the fresh dual-width unit gate, not S delivery, coordinator
acceptance or qualification of the unresolved contexts.

The reviewed tracked/untracked changes remain only the three NXVM audit records.
The protected implementation/test/artifact diff check returns zero. Full unit
green is current baseline evidence, not proof for the named missing contexts
or original-source disputes. No EXE rebuild is required by this docs-only audit.

## Executor Delivery And Qualification Boundary

No Shared source/test, catalog, manifest, App, INI or binary was changed.
No new L1 or timing-grade downgrade is established by this arithmetic finding.
The S3 inventory consumes F01-F10/F14 without retiring unresolved members from
T544. Its fresh complete units pass independently of S2 results. Changed-document
links, documentation governance and diff checks pass. Actual-change self-review
checks the three owned records, source-versus-inference wording, all family
dispositions and the absence of Shared/App/test/artifact edits. Coordinator
acceptance remains a separate step after immutable delivery.

Audit closure will mean the approved read-only batch has its findings and
receivers recorded; it will not mean the CPU is repaired, any missing test is
present, or the T qualification predicate is satisfied. Source conflicts remain
source reconciliation work; the concrete Shared mechanisms require owner review
before implementation. Later family audits must carry their cross-family
receivers forward rather than claim these were fixed by S3.
