# T544 S6 80386DX Family Audit

## Status And Scope

Read-only audit against a8527a828. No Shared repair or whole-CPU
qualification is claimed. Current owns admission. The full
F01-F14 batch includes inherited and 16-bit forms; protected dword success
does not qualify real, VM86, size-override or failed delivery contexts.
Every S2-S5 receiver remains inside T544.

Selected primary original is Intel 386 DX Microprocessor Programmer's
Reference Manual (1990), order 230985-003. Fresh SHA-256 is
9A8188F9D2282B113FC421E225CC2A643FCDC349E5C3C43659BD2CF6620F1EA1,
matching the retained source index. The owner-managed archive original has
557 pages and Adobe Paper Capture metadata. Cover PDF 1 was rendered and
visually inspected; order number is visible. OCR is navigation only; no
full source/form reading is inferred from identity or previous T512 citation.
Originals and rendered scratch stay external/ignored; no new source import.

## Finite Batch

| Partition | Required source/implementation/regression contexts |
| --- | --- |
| Admission/reset/FLAGS | Prefix 64-67, 15-byte limit, later-only rejection, reset caches/control/FLAGS, PE/PG transitions and image/load privileges. |
| Effective operand/address/stack size | D/B defaults, 66/67 combinations, 16-bit offset wrap, 32-bit ModR/M/SIB, segment selection, instruction/operand fetch and whole span. |
| Ordinary ALU/decimal/movement | F01-F03 byte/word/dword, immediate extension, aliases, MOVSX/MOVZX, LEA, LDS/LES/LFS/LGS/LSS, flags/nonparticipants and failed access. |
| Multiply/divide/count/bit | F06/F07 implicit/explicit arithmetic, signed extrema, count-zero/multiple, SHLD/SHRD, BT-family signed register indices/immediates, BSF/BSR and source clock formulas. |
| Stack and branches | F04/F05 all operand/stack widths, ENTER/LEAVE/PUSHAD/POPAD, near/far/taken-zero targets, short/near Jcc, LOOP/JCXZ/JECXZ and SETcc. |
| Queries/system tables/control/test | F12/F13 selector/descriptor rights, LDT/TR/table images, CR/DR/TR legality/privilege, sticky PE, CR3 and translation invalidation. |
| Segmentation/protection | Null/data/code/stack, normal/expand-down, DPL/RPL/CPL, Present, GDT/LDT limits, accessed writes and validated cache publication. |
| Gate/control returns | Direct/call/interrupt/trap/task gates, 16/32-bit TSS and gate mixtures, parameter/ring stacks, RET/IRET and segment/FLAGS cleanup. |
| Tasks | 16/32-bit save/load, backlink/busy/NT, early versus incoming-context faults, CR3/debug/bitmap state and publication order. |
| Paging | PDE/PTE P/RW/US and CR0, supervisor/user accesses, A/D, spanning/fetch/implicit references, CR2/error code and failure publication. |
| VM86 | Entry/exit/task/IRET/interrupt frames, default sizes/limits/CPL, FLAGS/IOPL, sensitive instructions, I/O bitmap and segment restoration. |
| Exceptions/asynchronous/debug | Fault/trap/abort, RF/resume/return IP, contributory/page/DF/shutdown, EXT, NMI/SS/STI arbitration, DR comparators/GD/safe delivery. |
| Strings/ports | Byte/word/dword, address-selected counters/indices, REP phases/restart, zero/count/DF, segment/page/permission and irreversible port effects. |
| External/NPX/LOCK | HLT, WAIT/ESC control/ERROR/BUSY ordering, valid/invalid explicit LOCK, automatic XCHG, bus intervals and attribution. |
| Timing/retirement qualification | Chapter-17 base/variable formulas and assumptions, taken/repeat/memory modifiers, sole selector precedence, Core waits/overlap/failure, independent predicates for all 1,413 catalog keys. |

These fifteen partitions require complete audit dispositions before S6
delivery. Missing regression contexts, source conflicts and implementation
defects will be mapped to their existing owner, not qualified by a catalog
count or transferred outside T544. Original source absence distinguishes
non-applicable forms from missing implementations.

## Size, Addressing And Fetch: Original Versus Runtime

Fresh visual inspection covers primary PDF 319-325 (17-1 through 17-7),
224 (9-20), and 302 (15-12). Table 17-1 makes operand/address sizes depend
independently on CS.D and the presence of 66/67; implicit stack selection
depends on SS.B, not 67. Current `_GetOperandSize`, `_GetAddressSize` and
`_GetStackSize` implement these distinct inputs. Both prefix handlers set a
presence flag rather than toggling it, so a repeated 66/67 is not a second
width reversal. This establishes the selector mechanism, not every handler's
correct use of it.

The complete inspected ModR/M/SIB switches in `_kdf_modrm_with_mod_quirk`
match Tables 17-2/17-3/17-4 for 16-bit modulo address arithmetic, 32-bit
register bases, no index for SIB index 4, scale 1/2/4/8, Mod=00 base 5 as
disp32 without EBP, and EBP/ESP bases selecting SS. Signed disp8 is decoded
before addition. Segment-override inputs supply DS/SS replacements. A source
typo must not be copied: Table 17-4's note says ESP for base 5 with nonzero
Mod, whereas its explicit addressing examples say EBP, matching the register
encoding and current switch. These inspected decode branches are not proof
of far-pointer span, operand-failure or handler publication correctness.

Two fetch receivers are directly supported by originals:

- PDF 224 and 302 require #GP(0) for an instruction exceeding 15 bytes.
  `ExecIns` loops over prefixes; `_kdf_skip` advances EIP, `_kdf_code` reads
  bytes, and `_s_read_cs` can read beyond the cached window. None of these
  runtime paths enforces cumulative instruction length. `oplen=15` in
  `ExecInit` describes the observation array, not the consumed length. The
  separately bounded debug lexeme scanner cannot enforce the executor's
  architectural limit. Retain S2 admission/fetch receiver; regression must
  distinguish a legal 15-byte instruction from a 16-byte prefix sequence in
  real/protected/VM86 execution, without importing an 8086 length limit.
- PDF 302 explicitly distinguishes 386 sequential execution past offset
  FFFF from 8086 wrapping. `_kdf_skip` currently masks every CS.D=0 advance
  to 16 bits; `_s_test_eip` then validates the already wrapped value. This
  loses the out-of-segment condition rather than generating #GP. Keep one
  instruction-cursor/fetch owner, with tests for final instruction byte,
  prefix, displacement and immediate crossing, not a second decoder.

`ExecInit` also eagerly translates up to 15 prefetched bytes before knowing
the instruction's required bytes. The first-page-last-byte success versus
unneeded next-page fault receiver remains open: this inspection identifies
the whole-range prefetch path but does not claim a fresh dynamic result.
Current prefix grid covers repeated widths and segment selection;
prefetch smoke covers cache reservation. Neither those tests nor debugger
overlong-input tests qualify the runtime length/fault boundary above.

## Paging Source Conflict And Publication Boundary

Primary PDF 141 (5-21), 172-173 (6-24/6-25) and 224 (9-20) were freshly
rendered and visually read. They specify present checks, supervisor bypass
of page write protection on this CPU, A at both levels, D at the leaf,
segment checking before page checking, and supervisor implicit references
for descriptors/TSS/IDT and inner stacks. `_kma_prepare_physical_linear`
tests P at both levels and applies rights only to vpl=3; fault paths set
CR2 and encode present/write/user. `_kma_commit_physical_linear` sets A
and leaf D before payload access and bypasses publication in preview mode.
Whole-span preparation, implicit-reference callers and rollback still require
their own dispositions; there is no blanket paging qualification here.

Table 6-4 contains a material edition-specific conflict with the current
implementation: its mixed supervisor/user rows allow a combined user page,
whereas the code rejects when either PDE.US or PTE.US is clear. The prose
also incorrectly calls a protection violation #GP on 6-24, while the
explicit page-fault definition on 9-20 identifies insufficient paging
privilege as #PF. Do not replace the code or test oracle from this one
table: reconcile the selected 386 revision and independent Intel originals
before choosing the U/S combining rule. This is a precise source receiver,
not permission to apply a later CPU's rule or silently relabel L3 as L2.

Independent archived Intel 80386 Programmer's Reference Manual (1986),
fresh SHA-256 E97605911BC589E5D33FFBC7B7E742DD6AB0DDA69E12AD972CDC8D32A1CDC775,
was extracted for navigation and PDF 150 (6-22) was rendered and visually
inspected. Its Table 6-5 explicitly combines mixed U/S rows as supervisor:
both levels must be user for a user mapping. This agrees with the current
PDE/PTE rejection mechanism and contradicts the 1990 mixed rows. Retain the
edition conflict for convergence and selected-revision confirmation; there
is no demonstrated need to change this implementation based on 1990 alone.

## Initial Timing Cross-Check

Primary PDF 328-329 (17-10/17-11), previously visually inspected in this S,
separate ready/prefetched/aligned, zero-wait/no-HOLD/no-exception instruction
clocks from additional machine conditions. The variable m counts components
of the next executed instruction, not bytes of displacement/immediate.
`core_machine_control_stack_next_term` correctly selects the preview
lexeme's component count for 386 and byte count for 286. Correct preview
failure semantics and each instruction's applicable m term remain separate
receivers.

Table 17-5 gives interrupt/exception task-switch clocks by old/new task
format and old VM state (309/282, 314/231, 307/282). Current control-stack
INT/INTO task branches select 309 unconditionally for 386. The existing
`timing_80386_manifest_run_s6_task_interrupt_recipe` asserts only 309 and
does not establish all old/new format and VM combinations. Retain the
task-outcome timing receiver, with source and oracle review before repair;
the ordinary CALL/JMP and IRET formulas must not be replaced with this
interrupt/exception table. No new L1 or approved downgrade is asserted.

## Task State, Privilege And Fault Context

Fresh visual originals: PDF 179-180 (7-3/7-4), 183 (7-7), 185-186
(7-9/7-10), 188 (7-12), and 190 (7-14). The TSS32 field offsets in
Figure 7-1 match the named offsets and layout assertions; 67h is the
inclusive minimum incoming limit. TSS descriptors reside in the GDT, but
task gates may reside in the LDT or IDT. Task-gate access uses the gate's
DPL, not the target TSS DPL, and its embedded selector RPL is ignored.
Interrupt/exception/IRET task transfers are not constrained by a target
DPL equal to zero. Table 7-1 and its following paragraph explicitly allow
an incoming CPL independent of the outgoing CPL.

Current `_ser_task_transition_tss_plan` is shared by 16/32-bit task images,
but rejects outgoing CPL!=0, incoming selector RPL!=0 and target TSS DPL!=0.
Its cache validators further require nonconforming CPL0 code and writable
CPL0 data, including ordinary DS/ES/FS/GS. These restrictions exclude
documented tasks and readable/read-only or conforming data references.
This joins the S5 task privilege/type receiver; merely sharing image layout
has not closed the architectural transition.

PDF 185 distinguishes early faults in the old task from faults while loading
the new task. Current plan validates incoming LDTR/code/stack/data and ESP
before saving outgoing state or publishing TR/CR3. Thus late #TS/#NP/#SS
is reported as an old-task failure; incoming descriptor reads also still
use the outgoing page translation before new CR3 is published. Current
`task32_expect` rejection branch explicitly requires old TR=0028h and an
unchanged outgoing image even for LDT/segment rejection. That oracle cannot
qualify Table 7-1's late-fault context. Reconcile phase boundaries in the
sole task transition and exception finalizer, not a parallel success-only
task path or a test changed to bless current behavior.

The same helper reports a not-present incoming LDT as #NP; Table 7-1
specifies #TS. `_s_task_validate_data_selector` reports not-present SS as
#NP, whereas Table 7-1 specifies stack fault. The incoming state selection,
presence and fault-code cases require coordinated source/test repair.

The TSS LDTR field is static (chapter 7.1's classification, freshly visually
checked on PDF 178). Both outgoing writers currently write it.
Do not call the whole 1Ch-63h image 'saved-state' qualification merely from
the static layout assertion. CR3 save/load, NT/backlink, local DR7 and task
trap need remaining explicit source checks; current code clears local DR7
only when the incoming task is 32-bit. The task helper does not branch on
incoming VM before interpreting its selectors as protected descriptors,
so VM86 task entry is another precise pending receiver rather than covered
by successful CPL0 protected task recipes.

PDF 187 (7-11), freshly visually checked, distinguishes NT by transfer:
JMP clears incoming NT; CALL/INT set incoming NT; IRET preserves incoming
NT but clears NT in the outgoing saved image. Current plan sets incoming
NT only for nesting and otherwise preserves it, and saves outgoing FLAGS
without the IRET clear. Both image formats share these defects. The precise
Table 7-2 also resolves the adjacent prose's erroneous inclusion of JMP in
the nested list; do not implement a nested JMP from that sentence.

Fresh IRET dictionary PDF 396 (17-78) gives six task timing cells, not one:
old 386 protected -> new 386 protected/VM86/286 = 275/223/271;
old 286 -> new 386 protected/VM86/286 = 265/213/232. The current constant
275 recipe qualifies only its matching format/context. These cells join
the task timing selector receiver; interrupt, CALL and JMP retain their own
tables rather than borrowing IRET constants.

## VM86, FLAGS Images And Reset

Fresh visual originals PDF 297-300 (15-7 through 15-10) allow VM86 interrupt
delivery through a task gate or a 32-bit interrupt/trap gate to nonconforming
CPL0 code. `_ser_int_protected` instead explicitly admits only a 32-bit
interrupt gate, and demands a current busy 32-bit TSS before dispatch. This
excludes trap/task cases, not an unsupported later-CPU extension. Its outer
helper reads ESP0/SS0 but permits any numerically inner CPL; VM86 requires
zero. Ordinary protected transfers to rings 1/2 need their own TSS stack
offsets. Existing VM86 delivery fixtures exercise 8Eh gates, not the missing
positive trap/task cases. Source parenthetical type names on PDF 297 are
swapped; descriptor encodings must not be inferred from that typo.

The nine-dword VM86 interrupt frame order and clearing of DS/ES/FS/GS in
the inspected helper match Figure 15-3. The figure's error-code column
repeats 'old SS' labels; do not reproduce those mislabeled saved segments.
`_kpa_test_mode` applies the I/O bitmap in VM86 regardless of IOPL, matching
PDF 300. This is a correct permission selection, not proof of every port
width/span or irreversible-effect ordering.

Fresh POPF/POPF(D) PDF 454 (17-136) describes both operand widths, preserves
VM/RF, restricts IOPL changes to CPL0 and IF changes to CPL<=IOPL; VM86
raises #GP only for IOPL<3. Current VM86 POPF additionally faults on any 66h
prefix, making its four-byte branch unreachable even at IOPL=3. Retain a
width/permission repair and explicit 16/32-bit VM86 regressions. The page
also interchanges the names of EFLAGS bits 16/17 in one sentence; bit-layout
authority, not that prose typo, identifies RF/VM.

Fresh PUSHF PDF 459 (17-141) confirms both widths and VM86 IOPL restriction.
Current PUSHFD clears VM/RF from the pushed image. The real stack shutdown
and whole-span receiver from S5 still applies; checking a successful image
does not establish exception or shutdown behavior.

Fresh IRET dictionary PDF 396-400 (17-78 through 17-82) requires validating
the VM86 return EIP limit and complete 36-byte frame, and invalidating
inaccessible data segments on outer protected returns. Current VM86 helper
reads all nine fields before publishing them but lacks an explicit pre-
publication EIP<=FFFF check; post-execution checking and rollback must be
traced before assigning a fault-context verdict. Its IOPL=3 VM86 return
branch agrees with the exception summary on PDF 400 and chapter 15; the
opening pseudocode on PDF 396 instead unconditionally faults when VM=1.
Record this intra-manual conflict, not a reason to remove the documented
IOPL=3 path. Outer-return conforming-DPL and absent-SS clauses also need
cross-section reconciliation rather than copying contradictory pseudocode.

Fresh reset Table 10-1, PDF 233 (10-2), specifies hidden CS limit FFFFh;
current reset initializes it to FFFFFFFFh. The reset cache receiver is
confirmed on the selected 386 original. Undefined reset fields/high FLAGS
bits must not be converted into fictitious exact-value assertions.

Fresh PDF 224 (9-20) explicitly lists MOV CR0 with PG=1/PE=0 as #GP.
`_s_write_cr0_80386` currently raises #UD. `cpu_execution_paging_smoke`
includes that combination in its generic invalid-control list and expects
#UD, so its green result certifies the wrong exception for this case.
Reserved control-register bits and CR3 alignment require separate original
rules; this finding does not justify extrapolating the same exception to
every rejected value. The named paging smoke is primarily control/admission
coverage, not a PDE/PTE permissions or A/D test.

## Added Movement, Bit And Double-Shift Forms

Fresh dictionary originals PDF 347/349 (17-29/17-31), 350-351
(17-32/17-33), 417-419 (17-99 through 17-101), 438-439
(17-120/17-121) and 482-485 (17-164 through 17-167) were visually read.
MOVSX/MOVZX handlers select source byte/word independently of destination
operand width, preserve nonparticipating high halves for word destinations
through `_m_write_ref`, and do not modify flags. Their source clocks 3/6
match the selector's register/memory split. This disposes the extension
operation itself; shared operand/fetch fault receivers remain applicable.

BSF/BSR start at opposite ends and set ZF for zero; current handlers skip
the destination write when ZF is set. The original declares the zero-source
destination undefined, so preservation is permitted but not an exact L3
result to require. Timing implements 11+3n / 9+3n with directional zero
scanning. The original does not separately specify the zero-source timing;
the selector uses n=operand width. That edge needs source qualification,
not an automatically exact 59/107 or 57/105 assertion from current code.
Both scan and double-shift primitives build masks as signed `1 << index`
before casting to a wide unsigned type; index 31 is not made safe by the
later cast. This joins the existing arithmetic host-representation receiver.

BT-family register memory indices are signed; the decoder implements floor
division for negative indices. Its dword formula `(bitoff32 - 31) / 32`
can overflow for INT32_MIN before dividing. Mask generation has the same
signed-bit-31 problem. More importantly, the immediate encoding is different:
PDF 350 says it selects a bit within the addressed word/dword, with a large
assembler offset encoded through displacement. Current immediate memory
decoder instead adds 2*(imm8/16) or 4*(imm8/32) to the address. Existing
`bit_test_immediate_and_memory` explicitly expects imm16 to touch base+2
and imm33 to touch base+4; it blesses the incorrect encoded-immediate
extension. Repair the one decoder and those expectations together, retaining
signed register indexing and the source's whole-word/dword access allowance.
Source notes mix a general modulo-32 phrase with word/dword forms; width-
specific encoding and original cross-edition checks remain part of repair.

SHLD/SHRD capture both inputs before modifying the destination, mask count
to five bits, preserve flags for zero, and suppress the handler's zero-count
write. For defined nonzero counts their loops, last-bit CF, SF/ZF/PF and
single-count sign-change OF follow the explicit operation and flag clauses;
3/7 clocks match the timing selector. For word count=16 the original
pseudocode marks >=width undefined while code computes a deterministic
result; that result is allowed but is not source-defined L3. Counts 17-31
leave `result` unassigned in the helper and the handler still writes it;
undefined architectural output does not excuse relying on stale instruction
scratch or host undefined shifts. Regressions must assert only defined
outputs and flags, and verify the intended memory permission/zero-count
boundary rather than manufacture an exact undefined result.

IMUL's immediate-byte/dword path (`_a_imul3`, bit=20) multiplies signed
32-bit values before widening to 64 bits. For example 40000000h*4 is a
valid truncated result with CF/OF set, but the host expression can overflow
before the overflow comparison. The full dword-immediate and two-register
paths already widen each operand before multiplication; consolidate the
same primitive arithmetic rule, not a new multiply path. DIV/IDIV dword
paths assemble EDX:EAX in unsigned 64 bits, and signed division guards
INT64_MIN/-1 before host division; their word observation assembly still
contains the S5 DX<<16-before-widening defect.

All five far-pointer load handlers read offset then selector and publish
through `_e_load_far`; word/dword sizes and FS/GS/SS destinations match
the dictionary. There is no aggregate 4/6-byte pointer preflight in these
handlers, so the common whole-span/fault ordering receiver also covers new
LSS/LFS/LGS rather than only inherited LDS/LES. Timing independently
distinguishes real 7, protected LDS/LES/LSS 26/28 and LFS/LGS 29/31, matching
the original rows. Null/unusable segment, access-bit writes, SS shadow and
publication failure still require the shared segment-load audit below.

## Architectural Debug State And Delivery

Fresh visual chapter-12 originals PDF 260-267 (12-2 through 12-9) distinguish
architectural DR/TF/RF behavior from the product debugger. DR0-3 hold linear
addresses despite Figure 12-1's 'physical address' labels; paragraph 12.2.1
explicitly defines linear addressing. Current instruction match uses the
first-prefix linear address and LEN=00; data matching excludes instruction
fetch, distinguishes write versus read/write and masks alignment by length,
consistent with 12.2.4. Reserved RW=10 and LEN=10 are undefined, not new
I/O/eight-byte breakpoint support. Product watchpoints remain distinct from
these architectural comparators.

Confirmed debug receivers:

- PDF 262 requires local L0-3 and LE clearing on every task switch, global
  enables retained. Current 0155h mask is right, but its application is
  conditional on incoming 32-bit TSS; a 286-format task on 386 also requires
  the clear. This now confirms the previously pending mixed-format receiver.
- DR6 B0-3 describe all matching comparators when a debug exception occurs,
  including disabled matches; only enabled matches cause the exception.
  Current helpers return only enabled bits, so cause generation and status
  publication incorrectly share the same mask. Separate those two values at
  the existing comparator boundary without making a second breakpoint owner.
- PDF 265 sets RF in the pushed FLAGS image on fault-handler entry. Current
  `ExecFinal` sets RF only for #DB; #PF/#GP/#UD and other fault deliveries
  lack this image behavior. Fault/trap/abort classification must select the
  saved image, not an unconditional bit added to every interrupt.
- The same page exempts IRET, POPF and task-switch CALL/JMP/INT from ordinary
  RF retirement clearing. `_debug_complete_instruction` exempts only opcode
  CFh, so POPF or task transfers can lose RF. The POPF dictionary says RF
  unaffected whereas this chapter describes loading it from the saved copy;
  retain that source conflict, but do not call ordinary post-POPF clearing a
  source-closed behavior.
- TF trapping uses its pre-instruction state; the source separately explains
  INT/INTO clearing and new-task tracing beginning after that task's first
  instruction. Current completion requires both old and current TF. This
  suppresses a traced POPF that clears TF and can conflate outgoing TF with
  an incoming task's TF. Scope the INT exception explicitly and reconcile
  task completion, rather than treating every TF-clearing instruction as INT.
- `ExecInt` attempts NMI before pending debug-trap delivery, and SS shadow
  only gates NMI/INTR, not debug delivery. Source priority/SS inhibition need
  reconciliation with chapter 9. No NMI-in-service lifetime is added merely
  by clearing its pending edge; the S5 NMI/IRET receiver remains open.

The TSS T-bit path sets BT and delivers before incoming instruction execution,
matching the successful ordering in PDF 267. Failed #DB delivery still runs
through the broader task/fault-context receiver; a successful trap does not
qualify early/late failure publication. General-detect in this manual relates
to in-circuit emulation; DR7 bit 13 is reserved in its figure. Do not import
later-CPU GD semantics or add a new ICE interface without a selected 386
hardware source and an approved external-input boundary.

MOV CR/DR/TR dictionary PDF 434 (17-116), freshly visually checked, fixes
the data width to 32 regardless of 66h. Current handlers use width four and
privilege-before-transfer, with CR0/2/3, DR0-3/6/7, TR6/7 decoding. Source
clock rows match current CR 6/11/4/5, DR address 22, DR6/7 read14/write16
and TR12 selectors. TR6/TR7 currently only store/read fields; chapter-10
TLB test commands require a separate functional check, not qualification
from the transfer opcode alone. The optional non-11 Mod compatibility setting
is not the documented Mod=11 encoding and must retain its distinct source
classification instead of silently claiming the same Manual-L3 admission.

Fresh arithmetic dictionaries PDF 378/383/385 (17-60/17-65/17-67) confirm
division overflow/zero exceptions, truncation toward zero, remainder sign,
and IMUL's early-out formula. Exact formula, not the table's min/max midpoint,
can qualify IMUL timing when the selected multiplier matches the decoded
form; the receiver still must inspect that operand choice independently.

## Exception Combination And Selector Query Continuation

Fresh visual PDF 207-209 and 220/225 (9-3 through 9-5, 9-16 and 9-21)
confirms rather than merely infers the following shared delivery receivers:

- Table 9-2 orders pending debug exceptions before NMI and INTR. Current
  `ExecInt` delivers NMI first. Loading SS inhibits interrupt and debug delivery
  through the following instruction boundary, not just the current INTR/NMI
  checks. NMI is inhibited until IRET; clearing a pending edge is insufficient.
- Table 9-3 includes #DE and coprocessor segment overrun among contributory
  exceptions. `_e_is_contributory_exception` includes only #TS/#NP/#SS/#GP.
  The double-fault predicate also omits #PF followed by #PF or a contributory
  exception. A contributory exception followed by #PF is not the same case:
  it remains separately handleable. Replace the incomplete combination test
  with the source matrix at the sole delivery owner, not per-opcode patches.
- #DF carries zero error code and abort semantics; failed #DF delivery enters
  shutdown until reset or NMI. The current request-shutdown branch and CPU
  rollback need qualification against that lifetime, not a successful #DF
  fixture alone. Task-switch #PF before and after publication uses the old
  and new task contexts respectively; PDF 225 confirms the earlier task gap.

Fresh visual PDF 238-240 explicitly defines TR6/TR7 commands and observable
TLB readback, not just writable register storage. Whole CPU source search
finds only the TR6/TR7 fields and their MOV decoder references; no test-command
or TLB entry mechanism exists. Thus successful MOV TR timing cannot qualify
the complete function. The original itself has conflicts: page 238 defines
C=0 write/C=1 lookup, while page 239's write recipe says set C; Table 10-2
also describes both 01 and 10 as matching a clear bit despite different
stored values. Cross-edition/hardware confirmation remains an explicit source
receiver before choosing command/bit-pair behavior. Do not implement these
ambiguities by copying OCR or silently count stored fields as functional L3.

LAR/LSL/VERR/VERW handlers and fresh dictionary PDF 410/429/497 were inspected
together. Their destination-width handling, unchanged destination on rejected
selectors, conforming-code visibility exception and CPL/RPL checks are visible
in the existing table-style paths. LSL computes `(raw_limit<<12)|FFFh` for G=1
and records the granularity input used by the 21/22 versus 25/26 clock row.
All four handlers reject P=0 explicitly; the source query descriptions do
not make presence a query prerequisite, and this carries the S5 source/code
receiver rather than being accepted from current oracle expectations.
Invalid LDT selectors must yield query failure without a selector protection
exception; the common selector/read boundary still needs that distinction.

LAR's 1990 descriptor table accepts 16/32-bit interrupt and trap gates,
whereas the current handler admits only TSS/LDT/call/task gates. This is a
source-edition reconciliation receiver, not an immediate addition of IDT-only
types to GDT queries. Its returned limit-high nibble is explicitly undefined;
the current deterministic bits are not an exact architectural guarantee.
LSL's prose calling its 32-bit destination a 16-bit register contradicts its
opcode table; retain the actual width table. Query timing successes do not
cover invalid-selector, inaccessible-descriptor-page or rejected-type paths.

Independent 1986 PDF 215/216/375 were freshly rendered and visually read.
They resolve the TR6 write recipe to C=0, lookup C=1, matching 1990's command
definition and showing the latter write recipe is inconsistent. Their bit-pair
01/10 rows explicitly match X=0/X=1. Undefined 00/11 in 1986 versus miss-all/
match-all in 1990 remains revision-specific. Both LAR dictionaries label
interrupt/trap gate types valid; their agreement is recorded, not falsely
described as an edition disagreement. Further selected-hardware confirmation
must explain their difference from narrower architectural models before
changing the accepted type set. The current query fixture has positive widths,
G/LDT/visibility and source-limit cases, but not the full P=0/type/invalid-LDT
matrix. Its source-limit test expects #DF from failed delivery, so it is not
an independent first-fault #GP proof.

## CPU External Interface And Branch Timing Continuation

Fresh PDF 382/423/499 and chapter-11 PDF 252-255 separate opcode admission,
instruction time and external signal behavior. HLT has five clocks, no FLAGS
change, CPL0-only in protected mode, and resumes after HLT on enabled IRQ,
NMI or reset. Current HLT advances then marks halted, even after raising
#GP; final rollback must therefore be included in its rejected-privilege
predicate, not only successful halt/wake. WAIT's dictionary #NM clause omits
MP, while chapter 11 explicitly requires TS&&MP for WAIT; current check uses
that pair correctly. ERROR# is sampled on WAIT and selected ESC commands.
Current WAIT samples pending error but ends BUSY via `x86_fpu_complete_wait`
atomically, and its remaining ticks are then added by the sole timing selector.
This is not an interruptible CPU wait state. The handoff ESC branch likewise
does not implement all memory operand widths/endpoint checks or selected
ERROR sampling. Coprocessor arithmetic is outside this CPU audit, but CPU
interface conditions, exception production and failure publication are not.

LOCK's legal memory-destination set and invalid-combination #UD are explicit
in 11.2.1 and the dictionary; a zero-clock prefix does not imply no external
locking interval. Chapter 11 additionally requires automatic locking for
memory XCHG, interrupt acknowledgement, TSS busy update, segment-descriptor
load and paging A/D update. Current form admission/flag observations are not
a bus-ownership interval: CPU bus source searches expose no lock begin/end
signal. These cases join the single external arbitration receiver; a CPU-local
successful exchange cannot prove DMA/other-master exclusion or misaligned
multi-cycle indivisibility. Do not add separate LOCK paths at each opcode.

Both short and near Jcc timing selectors infer condition outcome from final
EIP versus fallthrough. `_e_jcc` instead receives the actual condition and
performs a taken branch even for displacement zero. Therefore taken-zero
branches incorrectly select three clocks rather than 7+m. The short path
also computes fallthrough without prefixes. Preserve the decoded outcome
at the existing retirement boundary; do not infer it from changed PC or reread
mutable FLAGS. LOOP/JCXZ and all control timing must undergo the same outcome
sweep. The secondary SETcc handlers use condition values directly, but their
memory/failed-write and timing coverage still belongs to that finite partition.

## Stack, Loop And String Continuation

Fresh visual originals cover PDF 380-381, 401-402, 413, 427, 452,
457-458, 463-464, 389-390, 447 and 480-481. ENTER's operation distinguishes
OperandSize from StackAddrSize: the frame-chain walk uses BP with word
operands and EBP with dword operands, while stack allocation uses SP/ESP
according to SS.B. Current ENTER instead selects BP/EBP for chain reads
using SS.B inside each operand-width branch. Thus word operands with a
dword stack and dword operands with a word stack read the wrong chain
address. The final local allocation also subtracts size without the
manual's stack-boundary check. Retain one stack/frame receiver with both
cross-width combinations, nesting 0/1/31/32, allocation endpoints, paging
and publication order. The description's simplified SP-versus-ESP wording
must not override the operation's independent attributes.

POPA/POPAD's selected 1990 operation explicitly assigns the discarded
SP/ESP slot using Pop(). Current `_e_pop` into a temporary agrees with
that source; a provisional suggestion that this read itself was a bug is
withdrawn. The complete 16/32-byte frame and fault/publication boundary
still require coverage. PUSHA/PUSHAD's source ordering and original SP/ESP
temporary match current handlers. Their real/VM86 special starting-stack
values and protected starting/ending stack checks are not equivalent to
eight individually validated pushes: an early write cannot qualify the
required pre-instruction boundary. These join the existing aggregate-stack
receiver rather than a new per-opcode protection patch.

LOOP uses address size for CX/ECX and operand size for its target; current
`_e_loopcc` makes this distinction and changes no FLAGS. Its successful
386 time is 11+m. JCXZ/JECXZ is 9+m when taken but five clocks when not
taken. The current timing branch infers JCXZ outcome from EIP and calls
the m adder in both cases, so the not-taken case is overcharged and a
taken-zero displacement is misclassified. Preserve actual condition outcome
once at retirement for the whole Jcc/JCXZ family, with prefixes and width
masking included, rather than comparing PC or rereading mutable FLAGS.

REP's source operation on PDF 464 contains reversed ZF-exit conditions;
the immediately following seven-step description explicitly exits REPE
on ZF=0 and REPNE on ZF=1. Current CMPS/SCAS continuation matches that
description, not the erroneous pseudocode. The instruction helpers select
SI/DI or ESI/EDI by address size, step by operand width and DF, use the
override source segment and fixed ES destination, and check helper status
before decrementing the repeat count. `_m_movs`, `_m_stos`, `_m_lods`,
`_a_cmps` and `_a_scas` were read across byte/word/dword switches. This
establishes local successful-iteration ordering, not fault/restart/IRQ
publication or complete REP timing qualification. The sole repeat timing
selector tracks first/continuation/zero-count phases by CS/EIP/opcode/prefix
and widths; cancellation/restart and source formulas remain in its receiver.

INS prechecks the full destination span before irreversible port input;
OUTS reads its source before port output, and both move their index only
after successful transfer. I/O permission, spanning page preparation and
failed bus publication still require the whole-port receiver. PDF 463
distinguishes protected CPL<=IOPL from bitmap/VM86 REP INS and OUTS setup
costs; those are not interchangeable with primitive instruction costs.

SGDT/SIDT on PDF 480 reverses the stated operand-width cases; PDF 481's
compatibility note explicitly identifies the 16-bit form's upper zero
byte on 386. Current handlers mask base to 24 bits for operand16 and
retain all 32 bits for operand32, then precheck and write one six-byte
image. This matches the compatibility note and is not evidence for
reversing the code. The 286 FF compatibility value remains distinct from
the original undefined-byte claim. LGDT/LIDT split reads and privilege/fault
order remain separate table-image receivers.

Fresh independent 1986 PDF 398/425/440 confirms that some of these are
edition differences, not an OCR error or an unexplained code constant:
MOVS is seven primitive clocks and REP MOVS is 5+4*count there, versus
eight and 8+4*count in the selected 1990 PDF 436/463. Current repeat
ledger uses the 1986 MOVS values. REP INS setup is 13/7/27 in 1986 versus
14/8/28 in 1990; the current dynamic entry uses the former. Current OUTS
primitive is 12/6/26 while 1990 PDF 447 says 14/8/28, and the 1986 REP
OUTS real-mode row itself differs from the protected rows. Preserve this
entire string-clock edition matrix for selected-hardware reconciliation;
do not certify either edition wholesale or adjust constants piecemeal.
The SGDT width-description reversal exists in both editions and both
compatibility notes specify the 16-bit upper-zero behavior.

## Ordinary Operations And Decimal Source Continuation

Fresh originals PDF 336-341, 345, 375-376, 412-414 and 432 accompany reads
of the byte/word/dword ALU helpers, decimal handlers, BOUND, LEA, segment
MOV, AAM/AAD and XLAT. ALU shared helpers mask result width, include input
carry/borrow in ADC/SBB, derive result flags, preserve CF for INC/DEC,
set NEG carry for nonzero input, and clear CF/OF for logical operations.
Memory-destination versus memory-source timing is distinct (for example
ADD/ADC use 2/7 versus 2/6), not a single generic memory surcharge. These
local helper facts do not qualify failed memory writes or all opcode-form
clock allocation. Undefined flags must remain excluded from exact oracles.

AAA/AAS preserve the documented defined AF/CF result and clear AL's upper
nibble in either path; AAM/AAD publish SF/ZF/PF from AL and identify the
other arithmetic flags as undefined. The originals describe decimal base
10. Current immediate-base generalization and base-zero divide fault must
retain their existing extension/source classification rather than being
newly certified Manual-L3 from these decimal-only pages. DAA/DAS also
propagate low-adjust overflow/borrow into CF before the high adjustment,
which these simplified 1990 operations do not show. Valid BCD results and
arbitrary AL/AF/CF bit-pattern behavior need separate source/oracle
dispositions; do not infer a defect or exact qualification by copying the
simplified decimal pseudocode outside its stated input domain.

BOUND uses signed inclusive lower/upper comparisons, rejects register-only
source, and reads word/dword pairs. The source operation places the second
bound at base+operand-width; its description incorrectly adds that width
to the upper bound's value. Current comparison follows the operation.
Two separately checked reads still join the aggregate 4/8-byte operand
span and fault-order receiver. LEA writes only the computed offset, with
independent source address/destination widths, truncation or zero-extension,
no memory reference and no FLAGS change; `_d_modrm_ea` rejects register
forms. Current LEA fixture includes four attribute combinations and null
DS without access. LEAVE independently uses SS.B for frame-to-stack and
operand size for BP/EBP pop, matching PDF 414; its stack endpoint and
fault-publication coverage remains separate.

MOV segment selectors uses a word operand in both directions; MOV to CS
is rejected and successful MOV SS sets the shadow. Shadow coverage still
has the debug/NMI arbitration receiver identified above. LGDT/LIDT read
before their late CPL check, and LLDT/LTR also apply privilege inside the
load helper after operand decoding/reading. Their source-defined privilege
versus invalid-address priority must be tested at instruction admission,
not treated as correct merely because a later #GP is produced.

## Gate Width, Ring Stack And Transfer Timing Continuation

Fresh original PDF 162-167 (6-14 through 6-19), visually checked against
the rendered pages, separates gate type, stack address size and destination
privilege. Gate size determines saved operands/parameter units; SS.B
determines SP versus ESP update. The current TSS supplies the destination
ring's stack, including rings 1 and 2, not always SS0/ESP0. Table 6.5.3
expressly includes 16-bit gates with 32-bit destination stacks and preserves
the documented unreliable upper return ESP cases rather than manufacturing
an exact expectation. Its notes describe a particular level-3/level-0
example, not a universal restriction on all legal gates.

Current `_e_call_far` passes instruction operand width to
`_ser_call_far_call_gate`, which chooses its 16/32-bit branch from that
width rather than the descriptor type. Its branches exclude LDT target
code/stack selectors and conforming code, and use the ring-0 TSS offsets.
The word branch additionally rejects an incoming 32-bit ESP above FFFFh
and updates SP without using the destination stack's B attribute. These
are one gate/stack selection receiver: descriptor gate width, old/new
SS.B, current 16/32-bit TSS and destination CPL must remain independent.
The new-stack full-frame check and reverse parameter copy already present
are useful local mechanisms, not proof that the selection is correct.

`_ser_jmp_far_call_gate` selects gate width from the descriptor, unlike
CALL, but also rejects LDT targets/conforming code and requires target
DPL=CPL. PDF 162 explicitly allows gate JMP to conforming code with
DPL<=CPL without changing CPL. Its 16-bit offset publication also needs
the whole-EIP clearing rule checked, not merely assignment through IP.
Direct conforming/nonconforming CALL helpers apply distinct DPL checks;
do not replace these with a common rule that loses conforming semantics.

The source paragraph saying CALL can transfer to a "less privileged"
level contradicts its adjacent DPL<=CPL formula and stack-switch
description. The exact formula, gate operation and destination-ring
selection agree; this prose error is recorded, not used as permission to
invert privilege. Table 6-2's error-code column also repeats Return CS
for several stack-selector failures: retain instruction-dictionary/error
code reconciliation rather than copying the column into all failures.

PDF 166 requires outer RET to invalidate inaccessible DS/ES/FS/GS, and
explicitly does not check the restored ESP plus immediate against the
outer stack limit until a subsequent access. This separates required
frame validation from an incorrect post-return extra limit guard.

The current direct CALL/JMP timing selector infers task change from TR
selector change and allocates a single old32/new32 non-VM cost; task
branches also add the next-instruction m term. Their original task
matrices have their own format/VM cells and are not ordinary branch
costs. Indirect FF task branches likewise use a fixed 397/gate adjustment
plus m. Together with INT and IRET above, retain a single task-outcome
timing receiver covering all transfer causes, old/new TSS formats and VM
states. Gate classification must come from the actual admitted descriptor
and transfer outcome, not comparison of the original selector to the
post-transfer CS or a reread of a descriptor that execution can modify.

## Stack Alias And Segment-Span Cross-Check

The full `_kec_pop` helper contains an address-alias guard against the
ESP storage; SP and ESP share the same union address. Thus POP SP/ESP
does not increment the newly popped value. The preliminary suspicion
from its caller alone is withdrawn. `cpu_gpr_push_pop_smoke` checks
register index 4 separately and includes POP [ESP+4] using the incremented
stack pointer. `INS_8F` pops into a temporary before decoding the memory
destination, which agrees with that ESP addressing case. Full failed
destination rollback and all operand/SS.B combinations remain separate
qualification contexts, not a reason to remove the existing alias guard.

Primary PDF 449-451 gives segment POP 7 real/21 protected clocks for
DS/ES/SS/FS/GS. Actual timing precedence in `cpu_timing.c` reaches the
control-stack selector for the primary DS/ES/SS opcodes, which returns
20 protected clocks for both 286 and 386. FS/GS instead reach the 386
privileged selector and return 21. This is a cross-entry source mismatch,
not merely an unused fallback constant; reconcile the 286/386 rows and
their separate form owners without a second timing producer.

PDF 455-456 distinguishes PUSH's old ESP value and the particular
real/VM86 SP=1 shutdown case. `_kma_linear_logical` currently turns any
out-of-range nonprotected 386 stack access into shutdown, irrespective
of instruction. POP's PDF 451 specifies the real-address invalid-span
interrupt instead, and PUSHA has its own enumerated shutdown/fault cases.
Keep instruction admission/stack-span exception selection together rather
than treating every stack limit failure as PUSH's shutdown.

The same linear helper widens every nonprotected DATA segment limit to
FFFFFFFFh when a byte span exists. Address-size 32 does not itself erase
the real/VM86 segment limit: MOV and XLAT dictionaries explicitly retain
the effective-address-space exception. This joins the S5 real-data-span
receiver across ordinary loads, strings, far pointers and XLAT, while
preserving separately evidenced cached-limit/unreal-mode behavior.
Expand-down `lower=limit+1` also wraps when limit=FFFFFFFFh; the documented
empty segment must not become a usable full-range segment. Span arithmetic
already checks end against upper by subtraction; correct the empty-range
representation without adding a per-instruction collection of guards.

XLAT selects BX/EBX by address size, override DS and unchanged flags, and
its selected clock is five, matching visually inspected PDF 501. The
16-bit base-plus-AL expression is passed to memory without an explicit
16-bit address mask, unlike ModR/M formation. Retain effective-address
width and segment-limit rules as distinct checks in the common address
receiver, with boundary BX=FFFFh plus AL and independent overrides.

## Simple Forms And System Compatibility Disposition

Fresh original PDF 365/374 confirms CBW/CWDE and CWD/CDQ independent
operand widths, unchanged FLAGS and three/two clocks. Current handlers
sign-extend AL/AX or select DX/EDX by the sign bit; no extra address-size
dependency appears. SAHF masks exactly SF/ZF/AF/PF/CF; LAHF copies the low
FLAGS byte and forces bit 1. Reserved-bit normalization remains the common
FLAGS image receiver, not a new per-command policy. CLC/STC/CMC and
CLD/STD change only their designated bit; successful CLI/STI apply real,
protected CPL/IOPL and VM86 IOPL=3 checks in the 386 branch. The earlier
286 unconditional branch is still the S5 receiver, not hidden by this
386-local match. STI's shadow still joins asynchronous arbitration above.

All sixteen SETcc handlers were read, including signed SF/OF combinations,
ZF conjunction/disjunction and aliases. `_m_setcc_rm` writes one byte 0/1
without changing FLAGS. Visually checked PDF 478 itself prints SETG with
an OR while its same-opcode SETNLE row has the correct AND; SETNA omits
ZF although its SETBE alias includes it. Current signed-greater AND and
not-above OR agree with the paired conditions, not these table typos.
The four/five-clock source row and failed write publication belong to
their actual timing selector and shared memory receiver respectively.

CLTS privilege is checked before clearing TS; PDF 369's VM86 "None"
contradicts the same page's privileged-CPL0 text and chapter-15 VM86
rules. LMSW's literal operation also overstates a 16-bit assignment:
PDF 422 notes sticky PE and unaffected PG/ET; current helper changes only
the low four bits, preserves PE once set and checks CPL. Its memory
read-before-privilege ordering remains the admission receiver. SMSW is a
word image, including memory despite operand-size prefixes, and its
two/three real versus two protected clocks are separately allocated.
No later-CPU SMSW wide-register behavior is inferred from this source.

A successful VM86 FS/GS POP is excluded by the 386 privileged timing
selector even though these are ordinary real-style segment loads in
VM86; the fallback lacks a secondary POP allocation. LFS/LGS/LSS has a
similar VM filter after testing nonprotected state, whose actual reachability
depends on `_IsProtected` versus the timing helper's VM exclusion. Retain
the full effective-mode/form allocation receiver and verify final origin
and tier for each such successful form before declaring a new L1. A
present key in the static catalog does not establish this runtime path.

Memory XCHG's explicit/implicit bus interval remains unimplemented as
identified above; PDF 500 asserts LOCK for the entire exchange regardless
of prefix or IOPL. Register exchange and NOP alias preserve FLAGS;
failed memory destination must not publish the register exchange. Neither
the contradictory duplicate syntax clock rows nor a passing register
fixture qualify the locked memory transfer mechanism.

Actual `core_machine_primary_source_instruction_cost` assigns three clocks
to the whole conversion shape, so CWD/CDQ reaches three rather than PDF
374's two; CBW/CWDE's three remains correct. The same switch uses only
`prefix_oprsize` to choose word versus dword DIV/IDIV costs. Effective
operand size is CS.D XOR prefix presence, already used by execution and
other timing selectors. Consequently default32 without 66 selects the
word cost, and default32 with 66 selects dword. Keep both defaults and
both overrides in one width-qualified selector regression, not a patch
to one product's default mode. Other multiply/count/bit selectors must be
swept for this same prefix-versus-effective-size conflation.

The final instruction wrapper performs `_s_test_eip` and `_s_test_esp`
after every handler. The former tests a one-byte translated code reference
at the resulting EIP, and the latter checks a zero-byte stack endpoint
against the segment limit. This is not harmless observation: it may fault
ordinary instructions that merely set ESP, a successful POP that reaches
the endpoint, or the outer RET adjustment that PDF 166 explicitly defers
until the next access. It also must not turn a next-instruction fetch/page
fault into failure of the instruction just retired. Join this wrapper,
branch target logical checking, preview translation and retirement
publication into one admission-versus-next-fetch receiver; removing a
single RET-specific check while leaving this wrapper would not repair it.

`ExecFinal` restores the entry CPU image for faults and retains CR2 for
PF or DR6/RF for DB. This establishes general-register rollback, not
memory/I/O rollback: committed pushes, accessed-bit writes and device
effects are not undone by copying a CPU structure. It also explains why
late incoming-task exceptions cannot be fixed just by adding validators:
the selected old/new fault context has to survive this finalizer. Repeat
restart restores CS/EIP after successful iterations; partial iteration
side effects and exception delivery require the same commit boundary.

## Finite-Batch Disposition And Verification

Every partition is now assigned below. "Retained receiver" means an
explicitly identified source, runtime or regression gap inside T544, not
successful qualification and not permission to change Shared code.
Inherited handler repetition uses the S3-S5 helper/form audit together
with the fresh 386 width/mode/clock inspection in this record; it does
not import a 286 result as a 386 timing oracle.

| Partition | Disposition and actual receiving surface |
| --- | --- |
| Admission/reset/FLAGS | Retained prefix-length/fetch, reset-limit, CR0 exception and generation/mode FLAGS receivers; `cpu_control_state_smoke`, `cpu_eflags_local_smoke`, `cpu_direct_flags_smoke` are current predicates, not full privilege/return-image proof. |
| Sizes/address/stack | Fresh D/B/66/67 and ModR/M/SIB inspection; retain XLAT mask, real/VM span, empty expand-down, post-instruction endpoint and aggregate-access receivers in the existing operand/address and protected-data fixtures. |
| Ordinary ALU/decimal/movement | Defined helper effects, exact nonparticipant rules and width paths inspected; retain decimal-source/extension, far-span/publication and selected-edition clock receivers. Existing GPR/ALU/LEA/segment fixtures do not cover all faults. |
| Multiply/divide/count/bit | Retain pre-widening host overflow, immediate-memory bit range, signed index, undefined count/destination oracle and default32 DIV/IDIV timing receivers; existing bit-scan/bit-test/double-shift and multiply/divide matrices own regressions. |
| Stack/branches | POP alias is locally confirmed, not a defect; retain ENTER mix, complete frames, endpoint checks and actual branch-outcome/m-term clocks. Existing GPR push/pop and near/far/branch fixtures own these cases. |
| Queries/tables/control/test | Retain P-independent query, invalid LDT, system-type visibility, table-image privilege ordering, CR legality and TR6/TR7 functional/source-revision receivers. Existing descriptor-query, DTTR and control-state fixtures are receiving surfaces. |
| Segmentation | Null/Present/accessed cache mechanisms inspected; retain full real/VM and expand-down spans, admission priority and fault-publication receivers at `_kma_linear_logical`/segment load owners. |
| Gates/returns | Retain descriptor-selected gate width, LDT/conforming target, ring-stack selection, mixed TSS/SS.B and outer segment cleanup receivers; existing protected call/gate and outer-return board/CPU fixtures cover only subsets. |
| Tasks | Retain CPL/selector/NT/busy/static versus dynamic save, old versus incoming fault context, paging and VM/debug-state receivers at the sole transition/finalizer; cross-width/16/32 task fixtures include incorrect rejection oracles needing review. |
| Paging | Retain edition mixed-US conflict, full-span implicit/fetch preparation, CR2/error/A/D and committed side-effect receivers; existing execution-paging/task-paging fixtures do not qualify arbitrary rollback. |
| VM86 | Retain trap/task delivery, bitmap/FLAGS/IRET source conflicts and FS/GS POP runtime time allocation; withdraw unsupported claim that all far-pointer timing is filtered in VM86. No board-specific bypass is proposed. |
| Exceptions/asynchronous/debug | Retain RF/TF, comparator causes, SS/STI/NMI arbitration/lifetime, exception combination/DF and shutdown/fault-context receivers. Existing debug-state/fault-event/delivery fixtures cannot prove the omitted matrix. |
| Strings/ports | Local helper/index/count ordering and one repeat timing owner inspected; retain source-edition setup/per-iteration clocks, restart/IRQ and irreversible bus effects in existing port/string and timing runners. |
| External/NPX/LOCK | Retain interruptible WAIT, ESC CPU reference/control and actual explicit/implicit bus-lock interval receivers. NPX arithmetic belongs to the absent companion implementation and is not a claimed CPU instruction result. |
| Timing/retirement | Runtime producer precedence inspected, including conversion, segment POP, DIV width, branch and task matrices; retain actual-result attribution, preview/fault and selected-edition/form closure. The 1,413 catalog keys remain an inventory, not 1,413 independently proven results. |

Fresh complete repository-only units pass once per width on the unchanged
production baseline: x64 506/506 in 68.14s and x86 506/506 in 83.88s,
using `RunTestAggregate.ps1`, eight workers and the existing default caches.
These runs include current timing/decoder manifests and Shared predicates;
none was rewritten to bless the discrepancies above. Documentation
governance and `git diff --check` pass. Shared code/tests, manifests,
App/INI/media/MyNES and all release binaries remain untouched; this is
audit-document delivery, not a reason for EXE rebuilding or redundant
integration startup. Full T convergence and qualification remain open.

The source and runtime protected-mode predicates both exclude VM86.
Thus LFS/LGS/LSS actually takes the nonprotected seven-clock branch;
the preliminary VM-filter concern for those pointer forms is withdrawn.
FS/GS POP, however, explicitly rejects VM before its ordinary seven-clock
allocation, and cannot use the primary single-byte segment POP path.
This precise remaining runtime allocation gap needs a final tier/origin
predicate and receiver; do not describe all VM pointer forms as missing.
