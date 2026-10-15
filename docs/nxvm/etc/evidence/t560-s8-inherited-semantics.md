# T560 S8 Inherited Instruction Semantics

## Baseline And Complete Batch

Baseline 8b6d48888, accepted S7. This record consumes C15, not a smaller
firmware-reachable subset. S8 remains active; the initial inventory below
does not accept any previously unproved form. C16 timing and C17 production
admission remain S9/S10. No Shared-six, MyNES or owner-input change is admitted.

The original 1992 Intel family PRM is hash-verified as
`D86207AAEBD9B10464F2058EFC61559E6141D3B072DB100E89BFE17C8187E143`.
Its complete opcode-map pages, printed A-4--A-8 (PDF 794--798), were rendered
and visually read. Individual instruction specifications and later selected
SX sources must resolve applicability; the map alone is not a sufficient
semantic oracle. It includes historical LOADALL/LOADALLD, A-step A6/A7,
SALC and group aliases, while later SX CPUID/RSM are absent. Do not implement
or remove a form solely from that appendix. The previously qualified T559
and S2--S7 source contracts remain inputs, not proof of the inherited corpus.

## Actual Dispatch And Proof Inventory

At the accepted S7 baseline, the sole initialization in
`core/chips/cpu/cpu_instructions.c` assigns exactly
256 entries in `insTable` and 256 in `insTable_0f`. Primary has two direct
UndefinedOpcode entries; escaped has 172. The other escaped 84 entries include
RDMSR/WRMSR handlers that metadata may reject for the selected SX; a handler
name is not admission proof. The primary 0F escape and eleven prefix slots
are syntax, not independent instructions. Counts do not establish correctness.

There are sixteen primary group handlers (80/81/83, 8F, C0/C1, C6/C7,
D0--D3, F6/F7, FE/FF), three escaped groups (00/01/BA), and the primary
0F dispatcher. Primary 82 aliases INS_80. All eight reg fields and the
memory/register restrictions must be reconciled, including reserved groups.

At admission, 90 CPU smoke files exist and 23 contain an explicit SX profile
selection. This is an investigation index, not a coverage percentage: a file
may exercise several profiles indirectly, or select SX without proving all
forms. Actual setup and assertions determine the disposition.

Reproduction queries:

```text
rg 'insTable(_0f)?\[0x' src/core/chips/cpu/cpu_instructions.c
rg '^static void INS_' src/core/chips/cpu/cpu_instructions.c
rg -l 'PROFILE_80486SX' test/core/chips/cpu -g '*smoke.c'
rg 'PROFILE_80386|profile_has_8086' src/core/chips/cpu
```

`_______todo` is a declaration macro expanding to static void; its spelling
does not prove a missing implementation. Read the complete function body.
Lexeme, metadata, execution and preview have distinct responsibilities; an
instruction's lexical availability is not proof of its effects or legality.

## Finite Family Matrix

Every row expands over the C15 dimensions: applicable operand/code/address
sizes, register/memory and aliases, segment overrides, repeat/LOCK prefixes,
real/protected CPL0--3/VM86, flags/count/divide edges and required fetch/data/
stack/provider failure boundaries. A retained test is reusable only where its
actual setup and expectations cover the selected contract. The following is
the complete family partition; rows remain under reconciliation until their
direct proofs and source-qualified exceptions are recorded.

| Family | Actual owner / opcode coverage | Existing regression owners and admission finding |
| --- | --- | --- |
| F01 | ADD/OR/ADC/SBB/AND/SUB/XOR/CMP, accumulator and 80--83 groups; common arithmetic/flags helpers | legacy_alu, rmw_access, alignment_access; generic legacy matrices stop at 386 and need actual SX execution, carry/overflow and aliases. |
| F02 | DAA/DAS/AAA/AAS/AAM/AAD/XLAT | legacy_alu; BCD matrix ends at 386 and has only one value per adjust form; full defined flag/value and divide-zero behavior need proof. |
| F03 | TEST/NOT/NEG/MUL/IMUL/DIV/IDIV, F6/F7 and 69/6B/0FAF | legacy_alu, imul_immediate, imul2; legacy group-3 runs only 8086/186, not selected SX. Include full quotient range, zero divisor and signed minimum/-1 without host undefined division. |
| F04 | INC/DEC primary register, FE/FF, flags | inc_dec_first_group, inc_dec_second_group, inc_dec_final_group, legacy_alu, rmw_access; preserve CF, all GPRs and shared destination preflight. |
| F05 | ROL/ROR/RCL/RCR/SHL/SHR/SAR, C0/C1/D0--D3 | rotate, legacy_alu, rmw_access; distinguish count masks, effective zero, undefined OF and reserved-extension policy from table aliases. |
| F06 | SHLD/SHRD, 0FA4/A5/AC/AD | double_shift, rmw_access; reconcile zero/full/oversized count, size and alias behavior at existing helper. |
| F07 | BT/BTS/BTR/BTC/BSF/BSR, 0FA3/AB/B3/BA/BB/BC/BD | bit_test, bit_scan, rmw_access; signed memory bit index, register modulo and zero-source destination/flag contract. |
| F08 | MOV/GPR/immediate/moffs/segment, LEA, MOVSX/MOVZX, XCHG | gpr_mov, moffs, sreg_mov, segment_selector, lea, movx, xchg, operand_address; reconcile size, selector upper half, address overrides and alias/publication. |
| F09 | PUSH/POP/PUSHA/POPA/ENTER/LEAVE, immediate/segment/FS/GS forms | gpr_push_pop, pusha_popa, enter_leave, push_immediate, legacy_sreg_stack, fs_gs_stack; SP/ESP addressing and failed stack effects require selected-family proof. |
| F10 | CBW/CWDE/CWD/CDQ, LAHF/SAHF, CMC/CLC/STC/CLD/STD/NOP | sign_extend, lahf_sahf, direct_flags, legacy_alu; preserve unaffected upper halves/flags and operand-size selection. |
| F11 | Short/near Jcc/JMP/CALL/RET and LOOP/JCXZ/JECXZ | control_transfer_branch/near, transfer_boundary, setcc, legacy_alu; all sixteen conditions, count/address size and stack/fetch limits. |
| F12 | Far JMP/CALL/RET, call/task gates and outer returns | control_transfer_far, protected_far, outer_return, task_switch16/32_state/cross_width_state; selected CPU setup and hidden descriptor/state publication matter. |
| F13 | MOVS/CMPS/STOS/LODS/SCAS and REP family | movs, cmps, stos, lods, scas; zero count, both directions/size/address overrides, per-iteration interruption and committed-prefix failure. |
| F14 | IN/OUT/INS/OUTS and IOPL/TSS bitmap checks | port_io, port_strings, vm86_delivery_state, smm; successful port prefixes and failed memory/port restart, not only copied register values. |
| F15 | BOUND/ARPL/LES/LDS/LSS/LFS/LGS/LAR/LSL/VERR/VERW | bound, arpl, les_lds_profile_matrix, lss_lfs_lgs, lar_lsl, verr_verw; real/protected/VM86 legality, selector privilege and page/descriptor precedence. |
| F16 | 0F00/01 descriptor/task registers, task switches | descriptor_system, descriptor_table_register, sgdt_sidt, lgdt_lidt, task_switch16/32_decode/32_state; common descriptor accessed/busy and task-load effects retain S2/S6 proof where unchanged. |
| F17 | PUSHF/POPF/CLI/STI/INT/INTO/INT3/IRET/HLT | pushf_popf, software_int_state, idt_privilege_entry, iret/protected_iret/vm86_iret_state, vme, smm; source-qualified S6/S7 proofs retained with actual prefix/mode linkage. |
| F18 | CR/DR/TR, CLTS, INVLPG, INVD/WBINVD, CPUID/RSM | control_state, debug_state, test_register, tlb, cache, cpuid_flags, smm; preserve qualified S2--S7 forms, reserved indices and non-mutating preview. |
| F19 | BSWAP/XADD/CMPXCHG, complete LOCK admission | bswap, xadd, cmpxchg, legacy_lock; reuse T559/S3/S5 accepted form/failure proofs only where current inputs are unchanged. |
| F20 | ESC/WAIT absent FPU, undefined/reserved/prefix/length/decode forms | fpu_interface_state, decode_admission, prefix_attributes, execution_fault_event; selected no-FPU NM/privilege priority and actual UD delivery, not false success or host error. |

## Initial Confirmed Gaps And Next Proof

Confirmed now: inherited test selection is incomplete. In legacy_alu,
binary/immediate/BCD arrays end at 386, group-1/group-3 and several flag/count
matrices select only 8086/186; dword ALU is hardcoded to 386. These are proof
gaps, not yet claims that the production helpers are wrong. The first bounded
test extension will run unchanged inherited ALU/BCD/group-3 forms on SX and
retain every old profile. It adds no test framework, alternate execution path
or production state. Invalid early-CPU 66/67 rejection tests must remain early
CPU tests: adding SX there would invert a valid architectural distinction.

The full source/mode/exception reconciliation remains open. Do not accept F01
through F20 from merely finding a named test, reaching DOS or counting opcode
entries. Before any production repair, record the complete confirmed class,
original instruction authority, exact files and estimated diff. There is no
S8 implementation commit or new qualification/artifact assertion yet.

## First Actual SX ALU Proof

The existing legacy_alu owner case now includes SX in its binary, accumulator,
group-1, decimal-adjust/XLAT and group-3 matrices; it also fills retained 286/
386 group-1/group-3 selection holes without deleting early-family proof.
The binary cohort covers 8/16-bit all five selected profiles and 32-bit 386/SX,
eight arithmetic/logical operations, two directions, memory/register, eight
value pairs and both input carries: 12,288 actual CPU executions. Values
include zero, full unsigned value, signed boundary, alternating bits, carry
and borrow. This is F01 subcase proof, not complete F01 or all-family closure.

Inspection confirmed one test-oracle defect: legacy_alu_flags used lib_u32
for both sum and right-plus-borrow. At dword width those intermediates wrap
before the expected carry/borrow comparison. They now use existing lib_u64;
the architectural expected result remains width-masked. The expanded actual
CPU cohort passes on both widths without a production change. No new helper,
state, API, test framework or reference executor is added.

The first selection-only replay passes 1/1 per width. The expanded boundary
replay also passes 1/1 per width, 0.22 s / 0.19 s, in the original cached
graphs (`build/t560-s8-{x64,x86}-alu-edges-{build,test}.log`). A subsequent
test-only readability change removes positional profile indexing; its final
recheck is recorded separately below. These are focused development results,
not full-unit, integration or S8 closure qualification. The accepted S7 eight
EXEs remain current because no executable production input changed.

Final readability rechecks pass the CPU case plus manifest, 2/2 per width in
0.66 s / 0.57 s (`build/t560-s8-{x64,x86}-alu-final-test.log`). A further F02
cohort now executes ADC/DAA and SBB/DAS for every pair of valid packed-decimal
values 00--99 and both input carries: 40,000 decimal cases, 80,000 actual
instructions. It checks upper registers and all defined flags independently
using decimal arithmetic. One immutable 60,000-byte guest program per operation
reuses the existing fixture; it is not a new interpreter or reference CPU.
Both widths pass the expanded existing case, 0.22 s / 0.20 s
(`build/t560-s8-{x64,x86}-decimal-{build,test}.log`).

The original printed 26-18/21/65/66 text was read completely. Its DAA/DAS
pseudocode and valid-BCD prose are not interchangeable: the pseudocode's
temporary-value/threshold wording cannot be blindly translated into a chip
repair. The valid-BCD cohort provides direct functional proof without guessing
arbitrary invalid-BCD/ASCII inputs or silently changing old-family semantics.
Complete source reconciliation of those input domains remains open in F02.
Current changes are tests +106/-18, production zero; the full S8 matrix is not
yet qualified. Final closure must still run the whole required suites.

## Multiply/Divide, Shift And Decode Cohort

`cpu_integer_multiply_divide` adds 36 fixed boundary vectors across 8/16/32-bit
MUL/IMUL/DIV/IDIV, both retained 386 and selected SX, real/protected CPL0--3/
VM86, valid 16/32-bit code sizes, both address sizes and register/memory forms.
That is 2,880 normal/guest-DE executions, plus 1,440 actual memory-provider
failure injections. It proves quotient/remainder sign and limits, zero divide,
the host-dangerous signed minimum/-1 cases, implicit accumulator/high-register
publication, defined multiply CF/OF and preserved nonparticipants. No undefined
division flags or overflow destination is treated as an exact oracle.

DE runs through a real IVT/IDT and existing privilege/TSS stack wiring. Guest
code is at 1000h rather than overwriting IVT entry zero. Fault snapshots must
identify the interrupted instruction, and neither implicit result register may
commit before DE. Each memory form also runs with a failing source read: the
existing host fault must identify 5000h, preserve the entire CPU state and take
precedence over any potential divide error. The local bus callback supplies
one failure injection only; it delegates ordinary memory to the existing
fixture. One heap fixture is finalized before reuse/free. No public contract
or second execution/reference engine is introduced.

The initial multiply/divide cohort passes both widths. The final cohort with
source failures passes x64 in the four-case aggregate and x86 in the same
aggregate. Logs are `build/t560-s8-{x64,x86}-integer-shift-{build,test}.log`;
the aggregate initially passes 3/4 because the rotate test exposes the distinct
scope error below. This is not a claim that the initial aggregate was green.

Existing IMUL2, sign extension, MOVSX/MOVZX and all single-bit BSF/BSR owners
now rerun their original assertions on SX as well as 386, retaining old-profile
rejection tests. Register aliases, zero-source ZF, upper-half preservation,
memory/register, operand/address prefixes and invalid LOCK proofs stay at their
original test owners. The extend/scan aggregate passes 3/3 per width in
0.34 s / 0.26 s. No production change is justified by those passing additions.

### Source-Qualified Count Boundary And Test Scope Repair

The complete original PRM printed 26-263--266 was rendered and visually read
(PDF 761--764). Both SHLD/SHRD operation blocks explicitly use ShiftAmt >=
OperandSize for undefined result/arithmetic flags. Existing 386 undefined-range
proof is therefore retained for this selected 486 contract too. All immediate/
CL, direction, width, zero-count and undefined-result capture assertions now
run on both profiles; no modern-CPU rule replaces the original source silently.
These cases pass within the four-case aggregate above.

RCL/RCR carry-ring and SAR extreme-count matrices now include SX at every
applicable width. Their initial run fails because old test code chooses a
186/286 timing formula by array index, accidentally applying it to the newly
appended SX. The timing subset is now expressed using actual profile values:
8086/186/286 remain exactly asserted, 8088/386 remain excluded as before,
and SX functional proof does not pretend to discharge C16/S9 timing. Width
selection likewise uses the profile rather than index 4. No old assertion is
removed and no timing model changes. Final rotate rechecks pass 1/1 each in
0.19 s / 0.14 s (`build/t560-s8-{x64,x86}-rotate-profile-test.log`).

The inherited decode owner now includes SX in required-suffix failures,
15-byte/prefix and component limits, endpoint publication, wrapped immediates
and privilege-before-operand cohorts. Existing family distinctions remain;
the selected SX joins the existing 386-and-later attributes, not 286's ten-byte
limit. Final owner case passes 1/1 per width in 1.10 s / 0.21 s
(`build/t560-s8-{x64,x86}-decode-profile-test.log`). The prior broad feature-
fetch matrix remains intact. These are development proof results only; full
S8 closure, other family reconciliation and whole-suite qualification remain.

The exact-386 production predicate sweep finds distinct reset identity,
FPU population and LSL timing provenance, plus explicitly SX-aware reset-
fetch/0F admission. None is a missing inherited semantic branch; no mechanical
predicate widening is made. Timing provenance remains with S9. Current code
delta, excluding manifest/docs/artifacts, is tests +378/-68 (net +310),
including the new 192-line case; production remains zero.

The current test manifest verifies, documentation governance and whitespace
checks pass, and the global Types audit passes across 1,295 code/script files,
including the new untracked test. The existing fixture lifecycle gate passes
its 44 declared owners; the new heap fixture's finalize-before-reuse/free is
also reviewed directly rather than inferred from that gate's inventory.
There is no production, public-six, MyNES, INI or artifact diff. S8 retains
the remaining family/mode/failure reconciliation, including stack, transfers,
strings and descriptor/task coverage. It is not eligible for complete-P
publication or closure yet; no partial implementation commit is made.

## Stack, Control Transfer And String Cohorts

Existing FS/GS and immediate PUSH owners now exercise their real/protected
forms and failure boundaries on both 386 and SX. GPR PUSH/POP adds SX to
the generic register/rm matrices and explicitly replays ESP destination and
operand/address attributes. Other GPR protected/failure subsets still require
reconciliation; this addition does not qualify every F09 form.

PUSHA/POPA and ENTER/LEAVE preserve every older-profile frame, ordered failure,
allocation-bound and overlapping-chain assertion while adding actual SX
selection. Width eligibility is expressed by CPU family, not array position.
The local isolated memory buses supply the existing zero-wait special-cycle
and cancellation callbacks needed by SX shutdown; no production fallback or
new bus contract is added. ENTER level masking includes 0/1/2/31/32/33/255,
and failed transfers retain the accepted write prefix and unchanged result
registers. FS/GS plus immediate/GPR stacks pass 3/3 per width in 0.31/0.24 s
(`build/t560-s8-{x64,x86}-stack-base-{build,test}.log`). ENTER/PUSHA pass 2/2
per width in 0.18/0.19 s (`stack-frame` logs with the same prefix).

Near CALL/RET/RET-immediate, far immediate/indirect transfers/returns, all
sixteen short/near conditions, LOOP/JCXZ and 16/32-bit address/count forms
now replay the existing assertions on SX as well as 386. Earlier profile
behavior, including 286 near-Jcc rejection and early far-pointer wrapping,
is retained. The old zero-displacement timing assertion stays scoped to its
original CPU families; SX timing is not silently inferred from 386. All
three transfer owner tests pass both widths, 0.35/0.26 s
(`build/t560-s8-{x64,x86}-transfer-{build,test}.log`).

MOVS/CMPS/STOS/LODS/SCAS now exercise SX through the existing helpers: byte/
word/dword, address overrides, source segment selection, fixed ES destination,
direction, zero/one/multiple REP counts, REPE/REPNE termination and preserved
register/flag/memory effects. CMPS/SCAS explicit sign/borrow/overflow/equality
flag cases, all existing LOCK rejection vectors, source/destination segment
limits and completed-iteration restart state run on both 386 and SX. Their
five-case aggregate passes 5/5 per width, 0.46/0.29 s
(`build/t560-s8-{x64,x86}-string-profile-{build,test}.log`). The preceding
MOVS/STOS development pair also passes 2/2 per width, 0.27/0.24 s; it is not
counted as another whole-suite run.

The actual ten string opcode handlers use the existing 386-and-later width
branch for SX. Each repeat decrements count only after its primitive succeeds.
The five primitives were read at their source owner: MOVS preflights both
spans, then reads source/writes ES before publishing indexes; STOS writes
before indexes; LODS reads before indexes; CMPS/SCAS read before indexes and
flags. This source review and the focused tests do not yet discharge every
page/provider fault, interrupted-repeat or privilege permutation in F13.

In particular, the inherited protected far/string limit fixtures deliberately
have no usable exception handler and assert terminal guest shutdown plus
preserved/committed effects. They do not prove the original GP/SS/NP vector
or its error code. The real-handler exception owners must supply that proof;
no assertion is weakened or renamed into a stronger claim. Far call/task
gates, port strings, descriptor/task and the remaining family/form/mode/failure
matrix remain open. No production defect is established by these passing
extensions, so no speculative production edit is made.

Current cumulative code delta is tests +849/-398 (net +451), including the
new 192-line multiply/divide test and its registration; manifest, evidence
and artifacts are excluded. Production, public-six, MyNES and owner inputs
remain unchanged. S8 is still active and no partial implementation P is
published. Final complete-S8 qualification remains required.

Final development recheck builds and runs all 22 changed CPU owner cases
together in each original graph: 22/22 pass x64 and 22/22 pass x86, 0.58 s
each (`build/t560-s8-{x64,x86}-inherited-current-{build,test}.log`). This is
the changed-case aggregate, not the full repository unit suite. The updated
test/Core manifest verifies, NXVM documentation governance and whitespace
checks pass, and the global Types gate again passes its 1,295-file inventory.
No background test handle remains live after these checks.

## Confirmed Selector Error-Code Class And Pre-Repair Design

The new real-IDT protected-far test initially fails on retained 386: JMP to
selector 001Bh reports GP error 001Bh instead of 0018h. The first attempt
also needed to account for the legitimate accessed-bit update when the
handler reloads CS. The subsequent diagnostic run isolates the actual raw
selector error, not shutdown or a host error. Initial logs are retained under
`build/t560-s8-x64-protected-far-initial-test.log` and the diagnostic log.

Original PRM 9.8/Figure 9-6, printed 9-13 (PDF 233), is rendered and visually
read. Selector-error bits 1:0 are IDT/EXT, not the source RPL; bit 2 retains
GDT/LDT identity. This is one source-qualified failure-encoding class across
segment/table access, segment/TR/LDTR loads and direct far CALL/JMP. The
existing dispatcher/form inventory remains the entire C15 universe above,
not a reduced firmware subset. Other still-open families are not accepted.

Actual-owner sweep finds 27 unmasked selector sites in cpu_instructions.c:
12 GP(selector), four NP(selector), one SS(selector), one TS(rsreg->selector),
five GP(newcs) and four NP(newcs). Every argument names a segment selector,
not a pre-encoded IDT or external-event error. Existing masked call/task/
interrupt paths, IDT index construction, zero errors, page faults and later
external-origin handling must remain unchanged. Do not mask all errors at
the common raise function, which would destroy legitimate IDT/EXT fields.

Planned production change: only cpu_instructions.c, 27 one-line replacements
using the same existing & 0xfffcu idiom. No new helper, API, state, enum or
branch; no decoder/executor rewrite. Existing protected-only branches make
the correction applicable to retained 286/386 as well as SX rather than
preserving a proven error in old-family diagnostics. The exact IDT regression
replays 386/SX immediate/indirect CALL/JMP with both operand widths; prior
unusable-IDT/shutdown checks stay intact. Source/test manifests and final
eight PC EXEs become affected, so S8 cannot retain the S7 artifact hashes
as its final delivery after this repair. Full qualification remains pending.

### Actual Repair And Direct Exception Proof

The planned 27 replacements are now implemented exactly, source +27/-27,
net zero. No broad raise-function normalization, new API, helper or state
is introduced. The original _e_external_entry_error still sets EXT at its
existing owner. An exact raw-selector re-scan finds no remaining instances
of the 27-pattern class. Existing masked selector and IDT-error paths are
unchanged; a complete semantic absence claim is not inferred from this scan.

The protected-far owner retains its unusable-IDT/shutdown checks and adds
valid 32-bit IDT gates. Twelve cases cover privilege/type/presence, null
selector, GDT/LDT bounds, nonzero RPL and retained TI, plus destination-limit
failure. Immediate/indirect JMP/CALL and both operand sizes produce 96
executions per CPU profile (192 across 386/SX). Each requires the original
GP/NP vector, exact error, original CS/EIP diagnostic, handler entry, correct
stacked error/EIP/CS/flags and unchanged non-entry state. Legitimate CS
Accessed on handler entry is expected, not treated as forbidden publication.
The existing positive/conforming/rollback forms also replay SX. Final tests
pass x64/x86, 0.18/0.13 s, in
`build/t560-s8-{x64,x86}-selector-error-{build,test}.log`.

The two initial failures remain explicit: the first test run did not yet
isolate Accessed from the real selector mismatch; the diagnostic run then
reports mask 00002000h with error 27 (001Bh) for case 1. The subsequent
27-site production repair makes the intended 0018h assertion pass. Expected
error values were not changed to fit the defective implementation.

### Port Instruction Proof And Retained Completion Prefix

Scalar IN/OUT default/attribute, provider failure, VM86, TSS bitmap and LOCK
owners now replay selected SX alongside retained CPU profiles. INS/OUTS adds
286/SX and removes array-index CPU assumptions, keeping early rejection
semantics. Byte/word/dword, both address widths, repeat zero/one/multiple,
segment selection and direction remain actual guest execution. The isolated
port bus explicitly supplies the existing zero-wait special-cycle pair, just
as the isolated instruction bus does; this is test wiring, not a production
fallback or API addition.

A 24-case 386/SX port-string cohort accepts one REP iteration, then injects
the next actual port-provider failure using the existing fail_status field.
All widths, input/output and both address sizes must retain exactly one
accepted/completed transfer, previous index/count progress and memory, with
host CE naming port 00E0h and no guest exception. Failed transfer must neither
advance the CPU nor replay the accepted prefix. No additional queue, provider
state or retry framework is added. The original invalid-segment/shutdown
checks remain, without mislabelling them as exact GP-vector proofs.

Final port pair passes 2/2 per width, 0.22/0.17 s
(`build/t560-s8-{x64,x86}-port-failure-{build,test}.log`). Previous selection
pair also passed 2/2 each, 0.26/0.22 s. After the source change, all 93 current
CPU executable targets are rebuilt in each original graph and all 99 CPU
owner tests/checks pass: x64 13.16 s, x86 7.49 s
(`build/t560-s8-{x64,x86}-cpu-owner-{build,test}.log`). This is CPU-owner
regression, not whole-Core/repository/integration or final-S8 qualification.

Current code delta excluding manifests/docs/artifacts: production +27/-27,
tests +1100/-459 (net +641), including the new 192-line multiply/divide case.
Both Core source/test manifests verify. Shared-six, MyNES and owner inputs
remain unchanged. The eight deployed EXEs still represent accepted S7, not
this uncommitted production repair; complete S8 artifact qualification and
release remain mandatory before its implementation P can be delivered.

## Descriptor Query, BOUND, ARPL And Segment-Load Cohort

The existing LAR/LSL and VERR/VERW tests now run their modern forms on 386
and SX, while retaining 286 register/type distinctions. This covers visible
and invisible selectors, GDT/LDT, null/invalid LDTR negative results, source
segments/SIB and operand/address overrides, source limits, flags and unchanged
descriptor bytes. The generational system-type matrix is not widened by
array position. Initial query pair passes 2/2 per width, 0.24/0.19 s
(`build/t560-s8-{x64,x86}-descriptor-query-{build,test}.log`). Protected
unusable-IDT helpers still prove shutdown only, not exact original vectors.

BOUND had positive in-range and invalid-encoding checks, but no actual
out-of-range BR receiver. Twelve literal signed boundary vectors now cover
16/32-bit lower/upper inclusion, below/above limits, signed minimum and
inverted bounds. Both profiles execute real16, protected CPL0--3 with each
valid code width, VM86 and both address widths: 480 executions total. Valid
cases preserve the full non-EIP state and source pair; failing cases enter
a real IVT/IDT BR handler with original CS/EIP and unchanged non-entry GPRs.
Existing segment/SIB/VM86/LOCK and width cases also replay SX without removing
older rejection assertions. Initial range case passes 1/1 per width in
0.19/0.13 s (`bound-range` logs).

ARPL's 64 register alias combinations, flag and memory/segment/attribute
forms now replay SX. The register test now seeds actual nonzero high halves
before asserting preservation, instead of inadvertently zeroing those halves
while preparing the low-word operands. Early-family and real-mode rejection
remain distinct; SX gets the same real-mode/LOCK rejection. LSS/LFS/LGS
replay real/protected pointer sizes, invalid register encodings and incomplete
source-span non-publication on both 386/SX; the real LGDT/LMSW/segment/far-jump
bootstrap is retained. Their pair passes 2/2 per width, 0.25/0.22 s
(`selector-arpl` logs).

Descriptor-system stores/loads, layout, privilege, system selector and memory
failure assertions replay SX. The paged physical-GDT compatibility case stays
386-only: SX's supervisor-linear table access has distinct accepted S2/S5
proof and must not inherit that earlier-family route. The unrelated retained
C7 example is not reclassified as descriptor semantics. Original PRM SGDT/
SIDT printed 26-261/262 explicitly qualifies the 16-bit upper byte and its
zero-store compatibility behavior; the neighboring LGDT description has
different wording. This test extension retains the existing zero-byte behavior
using the instruction-specific compatibility note, not a speculative repair
or an assertion that every source passage is identical.

The existing protected descriptor helper deliberately discards its nominal
exception-code argument when no valid IDT exists. It is retained as old
shutdown regression, not counted as precise selector-error proof. A new local
table in that same test supplies valid GP/NP gates and ten LLDT/LTR selector
failure vectors. Register/memory and both operand-prefix forms run on 386/SX:
80 executions prove exact selector errors, original fault CS/EIP, handler
entry, stacked error/CS/EIP/flags, unaffected GPRs, table registers, LDTR/TR
and every GDT byte apart from legitimate handler Code Accessed. Invalid loads
cannot mark a TSS Busy. No new fixture type, public API or production change
is required. The initial descriptor-system/BOUND pair passes 2/2 per width,
0.29/0.20 s (`descriptor-system` logs).

Final six-owner cohort passes 6/6 per width in 0.29/0.26 s after the stronger
descriptor entry flags/GPR/table assertions
(`build/t560-s8-{x64,x86}-descriptor-cohort-{build,test}.log`). These are
focused development checks, not another full CPU/repository or artifact run.
Current cumulative code delta, excluding manifests/docs/artifacts, is source
+27/-27 and tests +1536/-655 (net +881), including the new 192-line integer
case. The current six-file cohort itself is tests +436/-196. No additional
production repair is justified by these passing extensions. Remaining exact
page/provider, task-switch/return and complete C15 form dispositions still
need reconciliation; S8 is not eligible for complete-P publication yet.

After this cohort, both Core manifests, NXVM documentation governance,
whitespace and the 1,295-file Types vocabulary audit pass. The existing
44-owner fixture gate also passes; it is not presented as a new per-function
resource proof. Direct fixture review confirms the new automatic fixtures
use the existing isolated memory/special-cycle bus with no heap CPU instance
or outstanding special cycle; the separate new integer heap fixture retains
explicit finalize/free. No Shared-six, MyNES, INI or asset file is changed.

## Task Widths, Outer Return And Local Debug Ownership

Five existing owners now execute their applicable retained assertions on SX:
outer_return, task_switch16, task_switch32_decode, task_switch32_state and
task_switch_cross_width_state. The word-TSS matrix retains 286 and adds SX
beside 386; the dword/cross-width matrix retains 386. Tests pass the actual
profile into fixture preparation, not a later profile-field overwrite.
Conforming and nonconforming outer-return checks, nonpresent outer stack,
error delivery and LOCK rejection retain their prior frame assertions.
Task-state recipes preserve saved GPRs, selectors/caches, backlink, Busy,
CR0.TS and fault-stage ownership. Reserved CR3 low bits deliberately retain
386's 1003 versus accepted SX's 1000; this is not array-index-dependent fitting.
The initial five-owner cohort passes 5/5 per width, 0.18/0.39 s
(`build/t560-s8-{x64,x86}-task-return-cohort-{build,test}.log`).

Direct inspection then confirms a production defect: the common task-load
owner cleared local debug enables only when the incoming TSS was 32-bit.
Original PRM 11.2.2, printed 11-4/PDF 278, was rendered and visually read
(`build/t560-s8-research/task-local-breakpoints.png`). L0--L3 clear at every
task switch; G0--G3 remain enabled. Word-TSS compatibility does not exempt
the selected SX from that rule. Same-class review finds one task-load owner
and one existing local-enable mask; no alternate task/debug path is needed.

Before the production change, the added real word-TSS regression fails on
both 386 and SX: TR=0030, DR7=000004FF instead of 000004AA. Evidence is
`build/t560-s8-x64-task-debug-before-test.log`. The repair changes the existing
condition from destination width to CPU family >=386 (+2/-1), retaining the
existing mask and leaving 286's nonexistent debug-register behavior alone.
No API, helper, state, queue or retry is added. This is functional task
ownership, not a new timing or physical breakpoint-accuracy qualification.

The eleven word-TSS recipes per modern profile cover direct/indirect JMP,
CALL, task gate, nested IRET, IDT task gate, late LDT failure and four
pre-switch rejections. The nested case explicitly rearms DR7 after entering
the called task, so IRET must clear local enables independently of CALL.
Addresses DR0--DR3 are outside executed code and remain unchanged; globals
remain enabled. Early invalid/not-present/Busy/short targets preserve all
enables because ownership never changes. The late-LDT case proves the
completed fault-task chain's final state, not an isolated intermediate
checkpoint. Cross-width direct/CALL/gate/IRET recipes also seed and verify
DR7 and DR0--DR3 in both directions, retaining full saved-state assertions.
LE/GE-specific behavior is not newly qualified by these L/G tests.

Final five-owner checks pass 5/5 per width in 0.34/0.30 s, including the
stronger cross-width and rearmed-IRET assertions
(`build/t560-s8-{x64,x86}-task-debug-final-{build,test}.log`). The earlier
post-repair pass before those extra assertions is retained in `task-debug-after`
logs, not substituted for the final checks.

## Flag Transfers And SETcc Selected-Profile Proof

LAHF/SAHF and direct-flag legacy arrays now include SX. Existing 386 attribute,
VM86 and checksum-sequence tests receive an explicit modern profile and run
both 386/SX; early-family prefix rejection remains early-family only. SAHF
tests all 256 AH images, including ignored/reserved bit combinations, while
LAHF checks the corresponding status images and fixed bit 1. Nonparticipant
GPRs, EAX upper/lower bytes, unrelated flags and instruction lengths retain
their assertions. No new status-normalization rule is introduced.

CMC/CLC/STC/CLD/STD retain exact flag effects and modern LOCK rejection;
CLI/STI's actual protected privilege/IOPL/initial-IF matrix also selects SX.
POPF/PUSHF identity keeps the real-mode generation distinctions. SETcc's
sixteen conditions, true/false witnesses, register/memory and operand/address
prefix cases now execute both modern profiles; pre-386 actual UD remains.
This is not an exhaustive truth-table/mode/failure acceptance for F11.
These three owners pass 3/3 per width, 0.39/0.29 s
(`build/t560-s8-{x64,x86}-flags-setcc-{build,test}.log`). Their actual test
delta is +86/-54; no production change is justified by the passing cohort.

Current cumulative code delta, excluding manifests/docs/artifacts, is source
+29/-28 (net +1) and tests +1831/-848 (net +983), including the untracked
192-line integer test. Remaining complete C15 disposition and final whole/
artifact qualification are still required; no partial S8 P is published.

After the stable task/flag cohort, all 93 CPU executable targets are rebuilt
per width in the original cached graphs. All 99 CPU-owner tests/checks pass
per width in 11.22/6.27 s
(`build/t560-s8-{x64,x86}-task-flags-cpu-owner-{build,test}.log`). This is a
CPU-owner aggregate, not full Core, public-six, App or integration acceptance.
Both regenerated Core manifests, NXVM documentation governance, Types audit
(1,295 tracked files), the existing 44-owner fixture lifecycle gate and
`git diff --check` pass. Actual production diff review confirms the 27 prior
selector replacements plus the single common task-clearing condition; the
new task repair has no API or persistent-state surface. Shared-six, MyNES,
all artifacts and owner configuration/media remain untouched. The accepted
S7 EXEs therefore still require final S8 rebuild and release qualification.

## GPR Data Movement, LEA, XCHG And INC/DEC

Original PRM forms were extracted and visually cross-checked at printed
26-210/PDF 708 (MOV), 26-190/PDF 688 (LEA), 26-284/PDF 782 (XCHG),
26-163/PDF 661 (INC) and 26-67/PDF 565 (DEC). Rendered pages remain ignored
under `build/t560-s8-research/*-forms.png`. Actual MOV/XCHG/LEA and arithmetic
owners were read; no production change is justified by this passing cohort.
The segment-MOV footnote's isolated 67H wording is not used to redefine the
accepted operand/address prefix contract or the still-open segment-MOV proof.

The existing gpr_mov and moffs tests now select SX in their legacy-valid
matrices, modern operand/address combinations, immediates, segment overrides
and actual UD/unchanged-memory rejection paths. C6/C7 reserved reg fields
remain rejected rather than accepted aliases. A new function inside gpr_mov
uses its existing GPR reader/nonparticipant assertions, not a new fixture.
All 64 source/destination register pairs, both opcode directions and three
operand widths execute across real16, protected CPL0--3 with 16/32-bit code
and VM86: 7,680 executions for 386/SX. This includes high/low-byte overlap,
self copies, preserved upper words and full dword replacement.

Initial MOV/LEA checks pass 3/3 per width in 0.42/0.28 s (`mov-lea` logs).
Subsequent wiring review catches a test-only initialization-order mistake:
GPR seeding overwrote the VM86 flag after the mode helper ran. The order is
corrected before final qualification, and explicit PE/VM preconditions now
reject a mislabeled setup. Final mode-precondition checks pass 1/1 per width,
0.21/0.21 s (`mov-mode-precondition` logs). Initial green results without
that wiring review are not cited as valid VM86 proof.

LEA no longer uses profile-array index 3 as a substitute for modern-family
semantics. Its real/protected size combinations, null-DS no-read result and
LOCK/register-form rejection retain their old assertions and now select SX.
The retained unusable-IDT negatives prove shutdown, not precise original UD
entry. XCHG's modern register/memory, accumulator/NOP and LOCK cases select
SX; four literal AL/AH, reverse AH/AL, AH/CH and AH/AH vectors prove alias
behavior without a second expected-value engine. Existing early-family
default/prefix rejection and S5 actual implicit-memory-LOCK proof remain.
The four data-movement owners pass 4/4 per width in 0.28/0.22 s
(`build/t560-s8-{x64,x86}-data-move-cohort-{build,test}.log`).

INC/DEC changes are confined to first_group's existing register, FE/FF and
address/profile functions. Other retained ALU groups are not mechanically
expanded. Seven literal boundary rows cover zero, one, nibble carry/borrow,
signed maximum/minimum and unsigned wrap. Every primary register and both
input CF values execute all valid word/dword forms on the old families plus
SX: 1,568 executions. Full flags retain CF/IF/DF, nonparticipant GPRs retain
their images and instruction lengths are checked. FE/FF byte/word/dword
register/memory and 32-bit addressing replay SX. Profile predicates use
family values, not positional assumptions. Existing actual SX RMW tests
retain protected/paged/segment/provider failure and nonpublication proof.
The INC/DEC/RMW pair passes 2/2 per width in 0.25/0.18 s (`inc-dec-boundary`
logs). Final actual-diff review also moves the modern address case's failure
return inside its profile loop so a later success cannot hide an earlier
failure.

The resulting six-owner cohort passes 6/6 per width, 0.35/0.27 s
(`build/t560-s8-{x64,x86}-data-inc-final-{build,test}.log`), followed by the
separate mode-precondition recheck above. No full CPU/repository rerun or
new artifact qualification is inferred from these focused checks. This
cohort changes five existing test files by +256/-95 (net +161). Cumulative
S8 test delta is +2086/-942 (net +1144), including the new 192-line integer
case; production remains +29/-28. Segment movement, remaining C15 form
dispositions and final whole/artifact acceptance remain open.

Both Core manifests, NXVM documentation governance, whitespace and the
1,295-file Types audit pass after this cohort. Actual-diff review confirms
only these five existing CPU tests and NXVM evidence/status changed in this
continuation; Shared-six, MyNES and all published artifacts remain unchanged.

## Segment Movement And Defined Unusable-LDT Failure

Existing sreg_mov and les_lds_profile_matrix forms now select SX beside
386, retaining old-family valid/rejection cases and the actual LGDT/LMSW/
far-jump protected bootstrap. Word segment stores retain their existing
upper-GPR policy; this extension does not invent a new dword segment-store
contract. Null data selectors, descriptor Accessed, hidden bases/limits,
unchanged source pointers and target/GPR nonpublication keep their prior
assertions. The existing external interrupt-shadow matrix also includes SX,
using a family predicate rather than profile-array position for LSS admission.
Existing SX debug-shadow tests are retained, not copied into another fixture.
The initial three-owner profile cohort passes 3/3 per width in 0.96/0.26 s
(`build/t560-s8-{x64,x86}-segment-profile-{build,test}.log`). Old protected
unusable-IDT negatives still prove shutdown, not exact GP/NP/SS vectors.

A local precise MOV-segment matrix supplies real protected exception gates.
Five destinations (ES/SS/DS/FS/GS), seven failure recipes with SS-only
qualification and eight register/memory/operand/address-prefix forms execute
on 386/SX: 432 cases. It checks exact GP/NP/SS, RPL-free selector error,
original CS/EIP, handler entry, complete error/EIP/CS/flags stack, unaffected
GPRs and segment caches, unchanged GDT/source bytes and no failed-SS shadow.
GP privilege/type priority precedes not-present SS where applicable. LDTR
and TR also retain their pre-fault images.

The first raw-reset-derived TI recipe returns GP(0), but source/setup review
shows it is not a valid unusable-LDT proof. Reset initialization leaves a
chosen hidden LDTR image; original PRM 10.1, printed 10-1/PDF 251, explicitly
leaves LDTR undefined after power-up. A first !flagValid-only production
attempt therefore does not change that improperly prepared case. Its failed
`sreg-ldt-after` log is retained as investigation, not successful repair proof.
No reset-cache behavior is fitted to that result.

The corrected regression actually executes guest MOV AX,0 / LLDT AX after
protected bootstrap and asserts successful retirement, null selector and
cleared LDTR validity before the tested load. Against the original helper,
both modern profiles still fail with GP(0) instead of GP(0004):
`build/t560-s8-x64-sreg-cleared-ldt-before-test.log`. Original LLDT printed
26-198/PDF 696 explicitly makes that state invalid; MOV printed 26-211/PDF
709 and 9.8 define selector failures and their error encoding. These pages
and reset 10-1 were rendered/visually checked under `t560-s8-research`.
Bochs 2.6 `cpu/segment_ctrl_pro.cc`, `fetch_raw_descriptor`, corroborates
the selector-bearing invalid-LDT rejection; file SHA256 is
`6C77BAF25F6B10A02CD7F42C500EC553EE7B932551B2F998076CB209525E1063`.
That is L2 corroboration, not proof of a fitted chip. No external code was
copied/imported and no sibling tree was modified.

The confirmed owner defect is `_ksa_read_ldt`: it checked only the selected
offset, allowing a known-invalid table to fall into generic logical-memory
GP(0). Its existing rejection condition now includes !LDTR.flagValid and
retains GP(selector & FFFC); trace text identifies both failure reasons.
Production delta is +3/-2, no API/helper/state. Same-class review leaves
GDT's always-owned register alone, LDT write/preflight after a valid descriptor
read unchanged, query `_s_check_selector` returning its existing ZF failure,
and `_s_task_read_selector` retaining its own TS selector classification.
Ordinary effective-address GP(0) is not globally converted into a selector
error. This is one descriptor-owner repair, not a second exception route.

Correctly prepared post-repair eight-owner checks pass 8/8 per width in
1.40/0.63 s (`sreg-cleared-ldt-after` logs). After rebuilding all 93 CPU
executables per width, 99/99 CPU-owner tests/checks pass in 10.05/6.45 s
(`build/t560-s8-{x64,x86}-segment-cpu-owner-{build,test}.log`). The subsequent
extra LDTR/TR nonmutation assertions are rebuilt/rechecked separately,
1/1 per width, 0.22/0.15 s (`sreg-final-state` logs); not another full run.
These remain CPU-owner proof, not full repository or release qualification.

Actual continuation delta is production +3/-2 and three existing tests
+203/-69 (net +134). Cumulative S8 code is source +32/-30 (net +2) and tests
+2289/-1011 (net +1278), excluding manifests/docs/artifacts and including the
new 192-line integer test. Complete remaining C15 disposition and final
whole/artifact qualification remain open; no partial S8 P is published.

Both regenerated Core manifests, documentation governance, whitespace and
the 1,295-file Types gate pass for the segment continuation. The final
production diff remains one instruction owner: prior selector normalization,
task debug clearing and this invalid-table precheck. No Shared-six, MyNES,
INI/media or artifact file is changed.

### Explicit 386 Equality Review

The non-timing CPU C/H query for equality/inequality/switch references to
80386 has eight current matches. Actual bodies, not just names, were read:
reset code-base/name switches include SX explicitly; reset signature has a
separate accepted SX branch; reset-vector fetch and the lexical two-byte
admission list include SX; two FPU-compatibility expressions exclude SX
intentionally under the selected no-FPU contract; LSL's 386-only granularity
record is timing evidence, not semantic admission. No equality in this set
is widened merely because a test now adds SX. Timing-file matches remain
with S9; this limited query is not a claim that every predicate/form in C15
is already accepted.

### Bit, Immediate IMUL, Segment Stack And Debug Preview Continuation

The existing bit-test owner now selects both 386 and SX and checks 640
register-index executions, including destination/index aliasing, width
masking, exact CF and unchanged control flags. Four additional signed dword
memory-index executions cover -1 and -33. Reserved 0F BA extensions 0--3
are rejected for register/memory and plain/66 forms on both modern profiles,
with zero operand-provider effects. Older-family rejection remains intact.
Immediate IMUL now replays its existing modern proofs on SX, including
216 dword signed-product oracle cases across both profiles. Undefined
arithmetic flags are not assigned new requirements.

Legacy segment-stack modern proofs also select SX. The first IRQ run fails
because its local bus lacks the existing special-cycle callback pair needed
by the handler's HLT; CE carries LIB_STATUS_UNSUPPORTED. This is fixture
wiring, not a CPU repair or proof of a special-cycle enum value. Installing
the existing callback pair preserves the original interrupt assertions.
Final bit/IMUL/segment-stack/RMW checks pass 4/4 per width in 0.25/0.21 s:
`build/t560-s8-{x64,x86}-bit-imul-stack-cohort-{build,test}.log`.

An observer check finds a separate real defect: DR4/DR5 aliases accepted by
the S6-selected SX execution contract are unconditionally rejected by the
lexeme scanner, so execution preview reports unavailable. The fail-first
log is `build/t560-s8-x64-dr-alias-preview-before-test.log`. Accepted S6
historical-alias source/model qualifications remain unchanged; this does
not introduce a new architectural capability or later DE behavior.

The existing metadata now assigns these aliases the SX minimum profile,
and the scanner's unconditional alias rejection is deleted. Generic
special-register form validation still rejects memory encodings. Tests
cover direct scan and actual preview alongside all 512 existing alias
execution forms, reject them on 386 and reject memory forms on SX, and
assert byte-for-byte unchanged CPU/instruction/context state and no writes
during observation. No new public interface, state or helper is added.
Debug/control/decode/test-register checks pass 4/4 per width in 1.11/0.38 s:
`build/t560-s8-{x64,x86}-dr-alias-preview-after-{build,test}.log`.

This continuation does not complete the remaining C15 source/form ledger,
whole-repository qualification or eight-EXE publication. Shared-six,
MyNES and owner media/configuration inputs remain outside this batch.

After rebuilding all 93 CPU targets per width, the complete CPU-owner
aggregate passes 99/99 on x64 and 99/99 on x86 (11.03/6.04 s):
`build/t560-s8-{x64,x86}-decode-cpu-owner-{build,test}.log`.
Both Core manifests and `git diff --check` pass after regeneration.

### Reserved CR And Immediate Group-2 Admission

The follow-up same-class review confirms two additional observer admission
defects, not missing instruction execution. `_d_modrm_creg` already rejects
CR1/CR5/CR6/CR7, whereas metadata and scanning accept them. Original PRM
4.1.3 marks CR1 reserved and MOV special-register printed 26-212/PDF 710
lists CR0/CR2/CR3. Selected-SX CR4 remains the accepted S6 contract; no
later control registers are inferred. The complete instruction page is
rendered and visually checked. The existing CR metadata branch now owns
both the register-index validity and CR4's selected-family minimum, without
changing the actual executor or its retained 386 ModR/M policy.

The new existing Control owner case fails first at 386 MOV EAX,CR1:
`build/t560-s8-x64-reserved-cr-before-test.log`. It checks 1,024 reserved
forms across both modern profiles, read/write, four reserved indices, eight
GPRs, both code sizes and four prefix combinations. Metadata and scanner
must reject, then a real IVT must deliver UD with the shared fixture's exact
frame/storage assertions. This is not terminal-shutdown substitution.
Control/debug/VME/decode checks pass 4/4 per width in 1.14/0.44 s
(`reserved-cr-after` logs). Legal CR4/VME and existing ordinary CR cases
remain regression controls.

C0/C1 extension 6 is also already rejected by the execution handlers, but
the immediate-group minimum-186 metadata branch bypasses the later group
validity branch. D0--D3 extension 6 already rejects. The existing immediate
rotate/shift test fails first on 186 C0 /6 (`group2-preview-before` log).
The first condition-only repair still fails because that earlier branch
continues to shadow it: the x64 99-owner aggregate passes 98/99, not all;
x86 is not run for that failed revision. Its `reserved-forms-cpu` logs are
retained. The final design gives the six Group-2 opcode forms one metadata
branch owning both the immediate forms' 186 minimum and extension validity.
No actual shift semantics, public interface, state or new helper is added.

Original PRM revision history H-4/PDF 864 explicitly removes duplicate
TEST/SHL map entries; that whole page is rendered and visually checked.
This explains the published-map distinction but does not qualify the
selected silicon's response to undocumented aliases. Existing rejection
policy is retained pending the additional source reconciliation below.
The same page's deleted 82 map entry does not by itself justify deleting
the inherited 82 execution alias or accepting it as documented SX proof;
that separate applicability question stays on the remaining form ledger.

The rotate test now replays its immediate-extension cases on 386/SX while
retaining 186/286. All 64 opcode/extension/profile cases compare metadata,
scanner availability/length and actual execution or real UD. This preserves
positive controls for the seven legal extensions, not just rejection proof.
Final control/rotate/VME/decode checks pass 4/4 per width in 1.16/0.49 s;
the subsequently added positive lexical controls and loop-layout cleanup
pass the separately rebuilt rotate case on x64/x86 in 0.20/0.14 s
(`group2-all-extensions` logs). These are distinct actual revisions/checks,
not a claim that the earlier 99-owner aggregate became green retroactively.

Cumulative S8 code is production +50/-39 (net +11) and tests +2645/-1130
(net +1515), including the new 192-line integer test, excluding manifests,
documentation and artifacts. Only the existing CPU instruction owner and
Core test corpus change; Shared-six, MyNES and owner configuration/media
remain untouched. Whole C15 and final whole/artifact qualification remain
open; accepted S7 EXEs are not relabeled as containing these repairs.

After the final positive controls, both regenerated Core manifests,
documentation governance, whitespace, the 1,295-file Types audit and the
44-owner declared fixture-lifecycle gate pass. The final actual production
diff is reviewed: the sole instruction owner retains selector errors, task
debug clearing and invalid-LDT repair, and now reconciles CR/DR/Group-2
metadata with execution. The next source-disposition work still includes
the original decimal-adjust invalid-input wording and historical/removed
encoding applicability; neither is silently classified from a green test.

### Defined ASCII Adjust Proof And Reserved-Opcode Source Reconciliation

Original AAA/AAD/AAM/AAS instruction pages, printed 26-18--21/PDF 516--519,
are read completely and rendered/visually checked. The existing ALU owner
adds 1,600 actual ADD/AAA or SUB/AAS cases across 386/SX, all pairs of
unpacked decimal digits and four initial AH values (00/01/7F/FF). Each
case runs both real instructions, independently checks decimal digit/carry,
AH wrap, unaffected upper EAX/ECX and defined CF/AF/control flags. The
undefined arithmetic flags are not converted into requirements. Immutable
guest programs avoid reinitializing the large fixture for every input.

A separate base-ten AAM/AAD cohort checks 131,072 AAD executions (all AX
images on both profiles) and 1,024 AAM executions (every AL and two ignored
AH sentinels on both profiles). Oracles use quotient/remainder or the
published base-ten conversion, byte truncation and independent parity.
SF/ZF/PF and non-arithmetic controls are checked; undefined OF/AF/CF are
not asserted. A 512-byte immutable program is reused per high-byte batch
with explicit prefetch invalidation when rewinding, not a new executor.
These formula tests include non-BCD inputs, but fitted-silicon qualification
outside the original recommended BCD use is not inferred from them.

Both expanded existing ALU tests pass x64/x86 in 0.28/0.24 s:
`build/t560-s8-{x64,x86}-ascii-adjust-{build,test}.log`. This continuation
adds 98 test lines and zero production lines. Cumulative S8 source remains
+50/-39 (net +11); tests are +2743/-1130 (net +1613), including the new
192-line integer owner, excluding manifests/docs/artifacts.

The original source also supplies a decisive qualification limit missing
from the earlier map-only discussion. Printed 9-16/PDF 236, Table 9-3 and
its complete note are rendered/visually read: reserved encodings are not
universally UD. The table explicitly names 82/D6/F1 and 0F07/10--13, and
lists F6/F7/C0/C1/D0--D3 with XX suffixes without specifying every suffix
bit in that table. It cannot justify treating every reserved extension as
documented UD. Therefore the preceding Group-2 test is current execution/
observer consistency proof only, not complete selected-SX alias acceptance.
The production UD handlers and F1/D6 metadata still require source-qualified
disposition; a removed Appendix A entry is not sufficient to close them.
No primary-source conflict is hidden by a passing test or labeled as an
architecturally required UD merely because that is the current implementation.

Read-only Bochs 2.6 `cpu/bcd.cc` research provides a further limited lead:
its comments distinguish early-CPU versus later AX-wide ASCII adjustments
and explicitly describe Pentium/P6 validation and uncertainty in earlier
decimal-adjust documentation. File SHA256:
`643FFCB3A00B2FBADF74997CBDB09BDA9A6BDA9B894DE88492A215732D71CAB7`.
That is not a selected-SX hardware oracle. No external code is copied or
transliterated, no sibling is modified, and no production BCD rewrite is
made from that lead. Valid-decimal proofs remain useful while arbitrary
invalid-input behavior remains explicitly unresolved rather than guessed.

Next finite source dispositions are: reserved/non-UD Table 9-3 forms and
their actual extension bits; removed/historical LOADALL and A-step forms;
invalid decimal/ASCII inputs where old manual prose/pseudocode and later
behavior differ. Required inherited mode/access/exception proof and final
whole dual-width qualification/artifacts remain open. This does not close
F02/F20, C15 or S8, and no partial implementation P is published.

### Selected-SX D6 SALC Model And Actual Execution

The first reserved/non-UD form is now reconciled to a bounded L2 behavior,
not falsely promoted to measured chip behavior. Original Table 9-3 confirms
D6 is not an architectural UD. Two read-only behavior references agree that
it sets AL to FF when CF is set and otherwise to 00, preserving flags and
the other register bits. Bochs 2.6 `cpu/flag_ctrl.cc`, SALC, has SHA256
`2A2C58A0C900C42E0373FAB4D108C591DACF27BE1D5ED3B74997762A7E01F79B`.
86Box revision `4fef696a4eead1d55a28d6ac0e5bd2864e5454da` supplies the
actual selected-family connection, not merely a handler name:

- `src/cpu/cpu.c` selects ops_386/ops_486_0f for Intel 486SX and SL-enhanced
  SX; SHA256 `532176616B6C4D09B3F1D30D71A987D96CF0B6F512DCD120469EF827EEB782BB`.
- `src/cpu/386_ops.h` maps primary D6 to SETALC;
  SHA256 `A4B1A63B67BFC7C96963F92E404989D8525E56FD7F222E2A98E059D4A02D9527`.
- `src/cpu/x86_ops_misc.h`, SETALC, agrees on the CF/AL truth table;
  SHA256 `7CCA59F327985989EF2B3CA5E530DFEA2EE0D012261E6D4023BF9BF69B28E441`.

These are behavior/design references only. No external unit is copied,
translated or imported; no sibling tree changes. The original CPU owner
implements the truth table in its existing traced-handler style, advances
through the existing fetch path, and replaces the D6 undefined dispatch.
Metadata admits it only from the selected SX profile, retaining every older
profile's existing policy. That preservation is a regression constraint,
not a claim that older real silicon lacks SALC. No API/state/alternate
executor or guessed timing value is added; C16/S9 still owns exact cost.

The existing ALU case fails first at SX D6 scanner admission:
`build/t560-s8-x64-salc-before-test.log`. Final positive proof comprises
33,792 actual executions: real/protected CPL0--3/VM86, both valid code sizes
(VM86 kept 16-bit), four 66/67 combinations, absent/F2/F3 repeat prefixes,
64 arithmetic-flag images and four EAX sentinels. It checks exact AL,
upper EAX, unchanged flags/ECX, byte length and no memory writes. Five
older profiles retain exact actual UD/strict-diagnostic proof; selected SX
LOCK D6 must deliver a real UD. The metadata owner is also checked through
the Core primary-opcode inventory rather than leaving its old D6-reserved
expectation behind. ALU/decode/LOCK tests pass 3/3 per width in 1.12/0.43 s.

The expanded x64 CPU-plus-three-board aggregate passes 101/102 in 14.16 s
(`salc-owner-regression` logs). All 99 CPU-owner tests pass, but the old
board preview case still expects every immediate Group-2 extension,
including executor-rejected /6, to be available. Direct executable exit 22
identifies that exact existing case. It now distinguishes current /6
rejection from the seven retained legal-extension length assertions; no
test case is deleted. That is observer/current-policy consistency only,
with the Table 9-3 silicon qualification limit above still explicit.
Final separate preview/UD-inventory/diagnostic checks pass 3/3 per width in
2.08/0.32 s (`salc-board-final` logs). The failed aggregate is not reported
as wholly passing, and it did not proceed to x86 after the failure.

This continuation changes one instruction owner and three existing test
files. Cumulative S8 source is +63/-41 (net +22), tests +2839/-1134
(net +1705), including the new 192-line integer test. No Shared-six,
MyNES, owner INI/media or artifact changes are made. D6 now has an explicit
L2 disposition; F1, the remaining Table 9-3 aliases/internal forms and
invalid-BCD source qualifications are not silently closed with it.

The final x86 CPU-owner rebuild/recheck then passes 99/99 in 5.82 s:
`build/t560-s8-x86-salc-cpu-final-{build,test}.log`. This is distinct from
the earlier three-owner x86 pass and the failed x64 combined aggregate.
The final source/test Core manifests, documentation, whitespace, 1,295-file
Types and 44-owner declared fixture lifecycle checks pass. Actual source
review confirms one new necessary instruction handler, one metadata route,
no public API/persistent state, no Shared-six/MyNES/owner-input/artifact diff.
S8 remains active, with exact timing and final production admission still
mandatory in S9/S10; no complete-P or updated release claim is made.

### Selected-SX Repeated SHL And TEST Encodings

The Table 9-3 class is not closed by asserting UD. The original non-UD
classification and H-4 duplicate TEST/SHL map correction are combined with
the already pinned 86Box 486SX/SL-enhanced SX dispatch connection. Its
`src/cpu/x86_ops_shift.h` maps Group-2 extension 6 to the same operation
as extension 4 at byte/word/dword widths; SHA256 is
`CCA5DD6A357E7F5008B8D8202603E474479FC4C5AC0A414DD34AEADBE9534F89`.
The previously pinned `src/cpu/x86_ops_misc.h` maps F6/F7 extension 1 to
the same immediate TEST operation as extension 0. Relevant full operation
branches and selected-family wiring are read, not inferred from a mnemonic.
These remain L2 behavioral models, not measured fitted-SX silicon results.
No external source unit is copied, translated or imported.

Only the existing CPU instruction owner changes. Metadata assigns the
repeated encodings the SX minimum; the now-redundant F6/F7 scanner rejection
is removed, leaving one lexical family-admission owner. Six existing
Group-2 switches route selected SX /6 to their existing SHL /4 branch, and
two existing F6/F7 switches route selected SX /1 to TEST /0. Raw decoded
extension bits are preserved for evidence. Existing arithmetic, read/write,
preflight and failure paths are unchanged. Legacy-only word branches cannot
be selected by SX and are left alone; every older profile retains its prior
rejection. No new helper, decoder, public interface or persistent state is
introduced, and no shift/TEST algorithm is duplicated. C16/S9 timing still
requires a separate source-qualified disposition, not an invented cost.

Fail-first tests reproduce both omissions at actual selected-SX scanner
admission (`shl-alias-before` and `test-alias-before` x64 logs). Existing
rotate and ALU owners then provide paired canonical/repeated execution:

- SHL: 10,032 pairs, 20,064 actual instructions, across six opcode forms,
  real/protected CPL0--3/VM86, valid 16/32 code sizes, 66/67 combinations,
  memory/register, nine relevant count images and three value images.
  Implicit-one forms run once, not nine duplicate times. A widened unsigned
  shift oracle checks the width-masked result; complete CPU storage and
  write counts must match the qualified canonical operation. Undefined
  flags are compared for model equivalence, not specified as silicon values.
- TEST: 2,816 pairs, 5,632 actual instructions, across the same valid modes
  and attributes, byte/word/dword, memory/register and sixteen value pairs.
  An independent AND/parity oracle checks defined flags; all CPU storage
  must match, source memory/EAX remain unchanged, and TEST produces no
  operand write. Older-profile ordinary aliases and selected-SX LOCK aliases
  retain exact real-UD/strict-diagnostic rejection controls.

Both matrices use existing fixtures and the actual instruction executor;
they are not another decoder or a reference interpreter. VM86 never fakes
a 32-bit code default. Protected source accesses have explicit matching
cached DS privilege rather than accidentally exercising an invalid selector.
The existing RMW owner now includes /6 variants and /1 read-only variants
through its page protection, missing page, split-page, segment permission,
provider read/write failure and effective-zero count scenarios. Read-only
TEST does not acquire a write permission requirement. No parallel failure
fixture or error flag is added. The Core preview owner also checks SX's
new positive lengths while preserving older negative and legal-form cases.

The first four-owner cohort passes 4/4 per width in 1.18/0.42 s. Expanded
failure/preview controls pass 5/5 per width in 2.11/0.47 s. Finally removing
the redundant scanner guard is separately rebuilt/rechecked, 5/5 per width
in 3.03/0.49 s (`group-aliases-owner-clean` logs). These are focused actual
revisions; no unchanged full suite is rerun per encoding, and previous
99/99 CPU results are not relabeled as execution of this latest revision.

Actual continuation code is source +22/-10 (net +12), tests +200/-14
(net +186). Cumulative S8 source is +85/-51 (net +34), tests +3039/-1148
(net +1891), including the new 192-line integer case, excluding manifests,
documentation and artifacts. Shared-six, MyNES, owner configuration/media
and accepted S7 binaries are unchanged. This gives the two repeated-operation
classes explicit L2 dispositions, not whole F20/C15/S8 acceptance.

### F1 Source Lead Before Event-Delivery Repair

The next direct source is Intel SDM 325383-078US, December 2022, local
instruction-reference PDF SHA256
`F117CB21CE1F3B6258F5CE5F786E91665A133BF55E7BC2DC8CAE843B56E4FB8E`.
Complete printed 3-516--517/PDF 626--627 pages are read and visually checked.
They identify F1/INT1/ICEBP as a DB trap with no new DR6 bits, bypassing gate
DPL, VM86 IOPL and VME software-interrupt redirection. That is retrospective
legacy documentation, not authority to add modern processor features.
Bochs 2.6 `cpu/soft_int.cc` and the relevant complete event-kind branch in
`cpu/exception.cc` corroborate a privileged software debug event, not ordinary
INT 1. Their SHA256 values are respectively
`723D6AA8831186BDB297A97A61BFE21209D23F624EB36A29F73122C4816ABD0E`
and `019E3EC05E7F525DBB74BF427E9973C425D36AABB3971F69CC28FAAB341AC771`.
86Box's simpler ordinary software-INT path is not adopted over the explicit
primary-source distinctions.

At this research checkpoint F1 still rejected. Required next proof must cover the
next-instruction return point, unchanged DR6, selected GD handling, gate-DPL
bypass, all VM86 IOPLs/VME, TF interaction, interrupt shadow and entry failure
publication at the existing debug/exception owner. Do not simply alias it
to INT imm8 or create a second trap queue. This is a researched next action,
not an implementation or qualification claim; other internal/historical
encodings, invalid-BCD source reconciliation and whole release remain open.

Final grouped-alias actual-change review confirms the one CPU owner, eight
existing selected-family dispatch selectors, unchanged arithmetic/access
helpers and legacy-only branches, profile-neutral metadata with one lexical
minimum-family decision, and existing test-fixture ownership. Both regenerated
Core manifests, documentation governance, whitespace, the 1,295-file Types
audit and the 44-owner declared fixture lifecycle gate pass. No Shared-six,
MyNES, owner inputs or assets are changed. The final required whole suites,
eight EXEs, complete P push and S8 acceptance are not yet claimed.

### INT1 And VM86 Event Entry

The selected-SX F1 disposition is now implemented at explicit L2 grade:
the original non-UD classification, retrospective Intel legacy instruction
description and independently traced privileged-debug-event reference agree.
The subsequent complete printed 3-518--519/PDF 628--629 operation pages are
also read and visually checked. They distinguish EXT=1 exception entry from
ordinary software INT, real-mode word frames from protected gate-selected
width, and VM86 IOPL/VME bypass. No modern CET/64-bit behavior is imported.

One traced INT1 handler advances EIP and marks the existing debug trap
pending, without adding DR6 cause bits. Metadata admits only selected SX;
older profiles retain their original strict-UD policy. The existing debug
completion owner excludes F1 from the outgoing TF step cause. It remains
the only debug completion/exception path: no second queue, state or API.

Actual tests first expose an adjacent real-mode defect: the preceding
instruction's operand size selected a dword DB frame. The common debug
delivery owner now uses word exception entry, matching the existing generic
exception path; protected gates still choose their own width. Original
1992 PRM printed 23-7/PDF 461 explicitly permits VM86 entry through task,
32-bit interrupt and 32-bit trap gates. Existing admission rejected the
latter two legal routes. Its type check is corrected. The original page's
swapped type-14/type-15 labels do not alter its explicit acceptance of both.

A real F1 task-gate execution then exposes a second guard: task planning
tested PE-and-not-VM instead of PE. Only that entry guard changes. The
existing task owner still saves outgoing VM86 state before loading the new
task; no temporary VM clearing or alternate task transition is introduced.
Direct VM86 far-transfer/return restrictions retain their caller owners.

Actual selected-SX proofs use the existing CPU fixtures:

- 37,376 successful F1 cases span real/protected CPL0--3/VM86, valid code
  sizes, 66/67, interrupt/trap gate widths and all DPL/IOPL/VME combinations,
  TF/GD/RF/IF images. They inspect exact post-instruction return IP, saved
  flags/selector, stack width, live flags, unchanged DR6/GPRs and cleared GD.
- 864 failed-entry cases cover invalid gate type, absent gate and null code
  selector, including VM86. GP/NP use EXT error codes, preserve the retired
  return point, and push the fault RF image. Failed DB entry preserves GD
  under the existing checkpoint rollback, and no pending trap is stranded.
- Six preceding NOP/STI/MOV-SS cases distinguish execution-breakpoint fault
  priority from INT1 completion, RF suppression and the segment shadow;
  none invents a TF BS cause for F1. Older-profile and LOCK rejection controls
  retain actual real exception/strict diagnostic proof.
- The existing task completion matrix adds 512 SX VM86-to-protected task-gate
  flag images (3,152 total retained/new cases). It inspects old TSS saved
  return IP/VM flags, incoming NT/CR3, DR6/GD and existing TLB invalidation.
  Required-fetch, metadata and Core preview owners are rebuilt as well.

Failures are retained, not rewritten as passes. `debug-word-frame-before`
reproduces SP=0x7f4 instead of word-frame 0x7fa. `int1-before` rejects F1.
`int1-entry-first` exposes VM86 trap-gate GP; `int1-task-before` and
`int1-task-diagnostic` expose task-entry shutdown. The first grouped
`int1-entry-after` x64 cohort is 6/7, not a full pass. Correcting the task
guard yields 7/7 per width (`int1-entry-final`). A test-only mistyped refresh
function fails compilation in `int1-boundaries-build`; it is replaced by
the existing void execution-refresh API, not a new helper or interface.

The final stable CPU/Core-owner aggregate passes 102/102 per width in
13.71/5.83 s, after rebuilding all 93 CPU executables plus three affected
board owners in the original reusable graphs. Logs are
`build/t560-s8-{x64,x86}-int1-owner-final-{build,test}.log`. This includes
the new failed-entry/shadow assertions and the preceding grouped aliases.
It is not whole-repository, desktop, artifact or complete-C15 acceptance.

Continuation actual code is source +14/-7 (net +7), tests +269/-16
(net +253). Cumulative S8 source is +99/-58 (net +41), tests +3308/-1164
(net +2144), including the untracked 192-line integer owner and excluding
manifests/docs/artifacts. No public-six, MyNES, owner inputs or deployed
S7 binary changes occur. All new behavior remains in the existing CPU
instruction owner; added private test functions own only their named cases.

The same-class source sweep identifies a distinct incoming-VM86-task issue:
after loading a TSS with VM set, task planning still applies protected-mode
CS/segment descriptor validation. This needs original-source reconciliation
and actual incoming-task proof; the passing outgoing-VM86 case does not
accept it. Internal/historical encodings, invalid-BCD source dispositions,
complete C15 reconciliation and final S8 release remain open. S9/S10 remain
mandatory for timing and production admission.

### Task Entry Into VM86

The preceding open item is now reproduced and repaired, not dismissed as a
protected-task variant. Complete original 1992 PRM printed 23-5--23-6/PDF
459--460 pages are read and visually checked. Section 23.3 requires 8086
base formation for segments loaded from a VM-set 32-bit TSS; 23.3.1 names
task gates, scheduling and NT-IRET transitions and says VM is updated before
interpreting the segment images. This is primary architectural authority,
not an emulator-derived VM86 extension.

The existing task segment validators now select the existing real/VM86
cache constructor when the loaded VM flag is set. Data/stack and code
cache preparation remain their separate existing responsibilities; code
still validates EIP against its real segment limit. Protected selector
checks, LDTR validation, task checkpoints, busy/backlink publication and
the final stack check remain unchanged. No new function, interface, state,
task dispatcher or temporary VM-flag manipulation is introduced.

The existing task-state owner first reproduces shutdown on a valid all-zero
segment-image VM86 task (`task-vm86-entry-before` x64 logs), after the 3,152
earlier completion cases pass. New direct execution checks then cover 768
cases across 386 and selected SX, direct JMP/CALL, GDT task CALL/JMP, IDT
task gate and NT-IRET return; eight zero/nonzero selector images include
every low-bit pattern, TF/RF images and null/valid LDTR. They inspect all
six caches, CPL3/64-KiB/16-bit attributes, saved old TSS IP, incoming GPRs,
busy/backlink state and exact EFLAGS. A following actual VM86 instruction
fetch and DS read prove the constructed addresses, not only copied fields.
TF is explicitly cleared only before that following data-read observation;
the task-entry TF image is first independently checked intact.

Four further cases use the existing task-gate fault-handler wiring:
out-of-range incoming EIP produces GP(0), invalid LDTR produces TS(0x10).
The faults belong to the new task and preserve its VM86 IP/CS in both the
diagnostic and saved TSS, then enter the protected handler task. Thus the
repair does not suppress LDTR or new-task failure boundaries. No new fixture
or test-only production hook is needed.

The initial six-owner dual regression passes 6/6 in 0.92/0.71 s. Adding the
NT-IRET route is separately rebuilt/rechecked at its owner; final failure
assertions pass 1/1 per width in 0.26/0.22 s. Logs are the
`task-vm86-entry-after`, `task-vm86-return` and `task-vm86-faults` cohorts.
These are not relabeled as execution of the earlier complete 102-owner
revision. Actual added code is source +16/-0 and tests +157/-0; the estimate
grows only for explicit return/fault checks, not a new mechanism. Cumulative
S8 source is +115/-58 (net +57), tests +3465/-1164 (net +2301), including
the untracked integer owner and excluding docs/manifests/artifacts.

Same-class review confirms these two private selector validators are called
only by the existing task plan after loading flags. Ordinary MOV/IRET already
have their own VM86 cache paths and are not replaced. A 16-bit incoming TSS
does not load VM and retains protected segment validation. Shared-six, MyNES,
owner inputs and deployed S7 artifacts remain unchanged. Historical/internal
encoding and invalid-BCD dispositions, complete S8 qualification and releases
remain open; this subsection is not complete F16/C15/T560 acceptance.

After that complete task batch, all 93 CPU executables and the three affected
board owners are rebuilt in each original graph. Final combined checks pass
102/102 per width in 14.93/6.52 s; logs are
`build/t560-s8-{x64,x86}-task-vm86-owner-final-{build,test}.log`.
Both widths' 1,295-file Types gates, the 44-owner fixture lifecycle check,
both regenerated Core manifests, documentation governance and whitespace
checks pass. Actual-change review confirms one production instruction file,
two existing private task-selector owners, reused cache construction, no new
production helper/state/interface and no excluded-scope changes. These final
results supersede neither whole-repository qualification nor eight-artifact
publication, which remain required for the completed S8 batch.

### UMOV Aliases And Byte Group-1 Reconciliation

Original Table 9-3's non-UD 0F10--13 class now has an explicit selected-SX
L2 disposition. Robert Collins's first-hand
[UMOV description](https://www.rcollins.org/secrets/opcodes/UMOV.html),
read 2026-10-15, distinguishes ICE target-memory cycles from ordinary
program execution, where the encodings act as MOV aliases. It describes
ordinary ModR/M direction, width and size-prefix behavior. The pinned
86Box `cpu.c` selected-SX route actually selects `ops_486_0f`; all four
size/address quadrants of that exact table use the corresponding ordinary
MOV handlers for these encodings. The neighboring IBM-specific table is
not substituted for the selected Intel route. Previously recorded file
hashes remain unchanged. NASM's older narrower availability description is
not treated as Intel silicon measurement; this remains a bounded L2 model,
not a fitted-SX experiment or an ICE implementation.

No operation algorithm is added. Four existing MOV handlers are selected
by the escaped dispatch table, the existing scanner recognizes ModR/M,
and existing metadata assigns the SX minimum. Thus data access, permissions,
register aliases, failure checkpoints and write publication retain one
owner. Earlier-profile rejection policy is retained; this work does not
retroactively qualify 386 UMOV. No external code is copied, transliterated
or linked, and no new helper, state, API or memory-space mechanism appears.

The existing GPR register matrix first fails at selected-SX byte UMOV in
real mode (`umov-before` x64 logs). It then replays all 3,840 additional
register cases through the four encodings, independently checking source,
destination, high-byte aliases, unaffected GPRs/flags and complete length.
The existing fixture is reused, not mirrored by a reference CPU.

A named private memory/failure case checks 2,640 canonical/UMOV pairs,
5,280 actual instructions. Valid contexts include real, protected CPL0--3
and VM86; only architecturally valid code defaults are used. Byte/word/
dword, address size, direction, all register fields and DS/ES overrides
are checked against an independent value/mask oracle and full canonical
CPU/image/write-count equivalence. Failure cases are not duplicated for
every GPR: register zero supplies limit-GP and actual bus-provider I/O
failure proof, preserving the destination image and restart checkpoint.
The latter is a CE host failure, not mislabeled as a guest exception.
Twelve earlier-profile/LOCK controls use exact actual UD delivery.

Required-fetch cases include all four register and memory forms. The old
ESC-only opcode insertion is removed: existing raw opcode bytes are copied
once, then the existing memory ModR/M/displacement is appended. This keeps
the old ESC case and covers every required UMOV suffix byte without a new
fixture or a second decoder. The local program buffer grows from eight to
nine bytes for escaped memory plus both prefixes. Core metadata's complete
escaped-opcode matrix and preview's register/memory/prefixed length and
nonpublication checks include the new class. Timing allocation remains S9.

Primary 82 already routes to the byte Group-1 owner, exactly as the pinned
selected-SX reference primary table does. It needs no production rewrite.
The existing independent Group-1 oracle adds sixteen SX cases for all eight
operations, register/memory destinations, defined flags and CMP non-write.
This is alias routing proof reusing the qualified arithmetic kernel, not a
claim that Appendix H's removed public-map entry defines silicon UD.

The initial three-owner dual checks pass 3/3 (2.08/0.41 s). The expanded
GPR/decode/ALU/metadata/preview cohort passes 5/5 (2.28/0.37 s). Adding
memory required-fetch proof is separately rebuilt/rechecked, 1/1 per width
in 1.10/0.28 s. Logs use `umov-after`, `umov-preview` and
`umov-required-memory`. A direct shell attempt to archive the public HTML
fails DNS; the successful browser retrieval and URL are the evidence, not
an invented local hash or downloaded source file.

This continuation is source +10/-4 (net +6), tests +148/-11 (net +137).
Cumulative S8 source is +125/-62 (net +63), tests +3613/-1175 (net +2438),
including the new integer owner, excluding manifests/docs/artifacts.
Public-six, MyNES, owner inputs and deployed S7 EXEs remain unchanged.

LOADALL remains a separate source boundary: Table 9-3, Collins's
[LOADALL article](https://www.pcjs.org/documents/manuals/intel/80386/ops/LOADALL/tspec_a3_doc.html)
and the selected reference's unconditional LOADALL dispatch do not establish
the same normal Intel486 execution contract. The article distinguishes
286/386 LOADALL and explicitly excludes ordinary 486 support. No blanket
LOADALL implementation, ICE pin, SMM alias or unproved UD qualification is
invented to resolve that disagreement. Its precise selected-family
disposition, invalid-BCD limits, complete C15 reconciliation and final
whole/artifact qualification stay open.

The final stable UMOV/82 revision rebuilds all 93 CPU and three affected
board executables per width in the original graphs. Its combined regression
passes 102/102 on x64 and x86 in 12.33/6.35 s:
`build/t560-s8-{x64,x86}-umov-owner-final-{build,test}.log`.
Both Types gates, both regenerated Core manifests, the 44-owner fixture
lifecycle gate, documentation governance and whitespace checks pass.
Actual-diff review confirms direct reuse of four existing MOV handlers,
one existing metadata branch, one ModR/M recognition addition and no second
operation/data path. Failure matrices inspect the accepted CPU/memory state,
not only return codes. No excluded-scope changes or new release artifacts
are present. These results qualify this bounded revision, not the remaining
C15/whole-S8 source dispositions, repository suite or artifact publication.

### Historical Internal Forms: Normal SX Admission

The preceding LOADALL source boundary is resolved for the modeled processor
without adding ICE hardware. Collins's January 1997 first-hand
[SMM/ICE comparison](https://www.rcollins.org/ddj/Jan97/Jan97.html), read
2026-10-15, explicitly says 486 LOADALL is disabled outside ICE and generates
UD there. It separately places RSM in SMM. His February 17/28 1997
[contemporaneous clarification](https://groups.google.com/g/comp.lang.asm.x86/c/hBIstu913LQ)
also restricts 486 0F07 to hardware ICE entry. SMM is not ICE, so it cannot
enable that instruction. This is an explicit L2 normal-execution rejection
model, not a new claim of implementing hardware ICE or measuring fitted SX.
The earlier generic Appendix A/Table 9-3 labels alone did not establish
normal execution eligibility. The reference's unconditional LOADALL route
is not adopted over this more specific source distinction.

The original PRM complete printed A-6/PDF 796 map is rendered and inspected:
0FA6/A7 are marked A-step CMPXCHG and earlier XBTS/IBTS, while normal CMPXCHG
is B0/B1. The selected later SX contract and its pinned actual reference
table reject those old slots. This is not an attempt to model the original
386/486 A masks; no new stepping dispatch or CPUID interpretation is added.
Ordinary 286 0F05 LOADALL remains inapplicable to selected SX as described
in the previously cited original LOADALL article. Existing handlers and
metadata already implement the required rejection; production is unchanged.

The existing control-state owner adds 352 selected-SX cases for these four
encodings across real/protected CPL0--3/VM86, valid code defaults, 66/67 and
LOCK. It checks scanner rejection, real UD handlers, original saved EIP,
frame width, preserved GPR/control state and exact frame write count.
The existing SMM owner adds sixteen prefixed/LOCK 0F05/07 cases with actual
SMI entry. UD leaves SMM active, does not read/consume the RSM save image,
preserves its original IP/CS on the word frame and releases exactly once
at finalization. No new fixture or production ICE flag is introduced.
Initial focused checks pass 2/2 per width in 1.09/0.24 s:
`build/t560-s8-{x64,x86}-historical-normal-{build,test}.log`.

### Decimal/ASCII Invalid-Input Model And Radix Proof

Original 1992 packed/unpacked-adjust pages prescribe valid BCD producer
instructions and contain the already recorded pseudocode/prose conflict.
The pinned Intel 325383-078US legacy-operation sections provide a precise
supplement: AAA printed 3-19/PDF 129, AAS 3-25/PDF 135, DAA 3-315/PDF 425
and DAS 3-317/PDF 427. All complete pages are rendered and visually checked.
The selected-SX outside-valid-BCD behavior is explicitly L2 from those
retrospective algorithms, not inferred to be measured 486 silicon from
Bochs's Pentium/P6 validation comments. Original valid-decimal functionality
retains its independent actual arithmetic proof; undefined flags stay
undefined. No unrelated later CPU modes or flags are adopted.

Fail-first `ascii-input-before` x64 proof finds AAA AX=00FA yielding 0100
instead of model 0200. Only selected SX now performs the AX-wide adjustment
in the existing AAA/AAS handlers; earlier-family byte algorithms remain
unchanged. The existing trace/function layout, flags and upper EAX ownership
are preserved. `packed-input-before` then finds DAS AL=9A/AF=CF=0 yielding
94 rather than 34. Only SX's existing DAS owner uses the incoming AL/CF for
the high correction and preserves a low-correction borrow if no high
correction occurs. One local const lib_bool stores incoming CF; it is not
persistent state. DAA already matches the model for every AL/AF/CF image
and its production implementation is not rewritten.

One private case in the existing ALU owner supplies the related input proof:

- AAA/AAS execute 1,079,296 cases: all 65,536 AX images, both initial AF/CF
  bits, both modern profiles, then value-boundary/mode/default-size/prefix
  checks. The retained 386 oracle explicitly preserves its old byte
  implementation; it is not a new 386-silicon qualification claim.
- DAA/DAS execute 17,408 selected-SX cases: all 256 AL images and AF/CF
  combinations, followed by mode/size/prefix/value boundaries. An independent
  additive correction/borrow oracle checks defined AF/CF/SF/ZF/PF, retained
  upper EAX, control/VM flags and absence of operand writes.

Immutable 256-slot instruction programs are reused, with explicit prefetch
invalidation at rewind, not a new executor/cache. Undefined ASCII arithmetic
flags and packed OF are not turned into required outputs. The old raw-DAS
SX fixture expected 9A for AL=00/AF=1/CF=0, an outside-valid-BCD case. After
the production repair it makes the first `bcd-input-after` x64 cohort 3/4;
that failure is preserved. Only its SX expectation changes to FA, while all
old-family expectations and defined flag assertions remain. The final
four-owner cohort passes 4/4 in 0.48/0.65 s (`bcd-input-final` logs).

AAM/AAD's complete original base-ten proof is retained. Intel's complete
generalized-radix pages, printed 3-21/PDF 131 and 3-23/PDF 133, are also
rendered/checked. No production change is needed. The existing result case
now executes 783,872 positive instructions over every immediate base:
all AL and two ignored-AH sentinels for nonzero-base AAM, full AX at AAD base
ten and three AH images/all AL at every other AAD base (including zero).
It checks quotient/remainder or truncated multiplication/addition, defined
SF/ZF/PF and unaffected upper/GPR/control values. This is explicit legacy
radix modeling, not a claim that the original base-ten-only page specified
every undocumented immediate.

Another 176 cases use actual DE/UD handlers across both modern profiles,
real/protected/VM86, valid code defaults and 66/67: zero-base AAM faults
before retirement, while LOCK has UD priority. Programs are placed away
from IVT vector zero. Required-fetch tests include AAM/AAD with ordinary
and zero immediates. The initial radix cohort is 1/2 on x64, not all green:
the zero-base oracle omitted retained 386 descriptor writes. Source review
of `_ksa_write_xdt` and the existing gate-entry owners establishes its
one/two additional protected/VM86 descriptor writes; the test now accounts
for them explicitly instead of weakening the write assertion or modifying
production. `bcd-radix-diagnostic` retains the exact first discrepancy.
Final radix/fetch checks pass 2/2 per width in 0.64/0.77 s, followed by the
test-only LOCK extension's 1/1 checks in 0.63/0.69 s.

The stable production batch rebuilds all 93 CPU and three affected board
executables in both original graphs, then passes 102/102 per width in
14.21/6.42 s (`bcd-historical-owner-final` logs). This aggregate precedes
only the final test-only zero-LOCK assertion; the separate owner rechecks
above qualify that addition, not a falsely relabeled second full suite.

Actual continuation code is source +18/-6 (net +12), tests +265/-1
(net +264). Cumulative S8 source is +143/-68 (net +75), tests +3878/-1176
(net +2702), including the new integer owner and excluding manifests/docs/
artifacts. The three instruction handlers remain the only production
owners changed here; no new function, API, queue, persistent state or Shared
change is introduced. MyNES, owner inputs and S7 artifacts are unchanged.
The remaining step is full finite-family/form reconciliation and whole-S8
qualification/publication, not further unbounded invalid-BCD speculation.
S9 timing and S10 production admission still remain mandatory.

Final actual-change review and both regenerated Core manifests, documentation
governance, whitespace, both 1,295-file Types gates and the 44-owner fixture
lifecycle gate pass. A follow-up exact-386 predicate sweep reads the remaining
semantic exceptions: reset separately initializes SX; reset-vector fetch
explicitly includes SX; external-FPU eligibility intentionally excludes SX;
LSL's 386-only granularity fields are timing evidence, not its result or
selector semantics. They are retained rather than mechanically widened.
The numerous family-specific timing-model comparisons remain S9's named
receiver. No excluded-scope files or release inputs are changed.

## Descriptor Query Type, Presence And Privilege Reconciliation

The final F15/F16 review confirms two defects at the existing query owner:
LAR/LSL/VERR/VERW incorrectly require descriptor Present=1, and LAR admits
interrupt/trap gates (types 6, 7, E, F). Queries inspect visibility/type and
return rights/limit or readability/writability; they do not load the segment.
Their existing Present check therefore conflates a query with an actual load.

Primary authority is the pinned original 1992 PRM: printed 26-188/PDF 686
(LAR's explicit valid/invalid system-type table), 26-207/PDF 705 (LSL),
26-278/PDF 776 (VERR/VERW's complete condition list), and section 6.7.1.
The original LAR and VERR/VERW pages are rendered and visually checked at
`build/t560-s8-research/query-{lar-types,verr-verw}.png`. The previously
pinned Intel 325383-078US December 2022 instruction reference, LAR printed
3-581--582, independently spells out the checks and returns the P bit rather
than requiring it; its protected-mode table agrees with the original gate
exclusions. No IA-32e behavior is imported. Original 26-207's complete table,
not the abbreviated chapter-six table's reserved 16-bit entries, controls
inherited LSL system-type admission.

Before production edits, the plan identifies the existing sole CPU instruction
file and two query tests, with approximately fifteen production lines removed
and 100--160 test lines added. The first test-only correction fails both x64
owners (`query-before` logs, 0/2): their old non-present expectations encoded
the implementation defect. The existing VERR/VERW, LAR and LSL conditions
now omit only the erroneous P precondition. LAR's four invalid system cases
are removed from its existing switch. Actual segment/task/gate loads keep
their mandatory NP/SS/TS checks. No new production helper, function, public
API, persistent state or alternate route is introduced; earlier CPU forms
continue through the same architecturally applicable query owner.

Two private cases extend the existing query tests, with no fixture changes.
Each executes 16,384 combinations on both 386 and SX: all 32 system/user
types, both Present values, all DPL/CPL/RPL triples and four forms. LAR/LSL
cover word-register and dword-memory destinations; VERR/VERW cover register
and operand-prefixed memory selectors. Literal primary-source type masks,
independent privilege/readability equations, rights/byte-granular limits and
unaltered upper-register values are checked. Every case checks instruction
completion, all defined flags, zero writes and unchanged descriptor bytes.
Together these are 65,536 actual query executions, not synthetic validators.
Existing source bounds, LDT, address overrides, mode/LOCK and descriptor-load
failure cases remain. The retained 286 positive/negative tables are corrected
for query presence and the original gate-type table without adding 386 types.

The first complete regression is 101/102 on each width, not all green:
`cpu_segment_selector` retains four more wrong non-present query expectations.
Its diagnostic identifies indices 3/4/11/12, returned LAR rights 00401200,
word LSL result ABCDFFFF and successful VERR/VERW. Only those expected rights,
limit and ZF entries change; its real guest protected bootstrap, actual HLT,
descriptor nonmutation, cached segment and failed-load checks remain intact.
No production rollback or exception relaxation is made to satisfy that test.

After rebuilding this final affected test, both complete 102-case CPU/board
cohorts pass: `build/t560-s8-{x64,x86}-query-final-test.log`, 3.02/3.01 s.
All 93 CPU executable targets and the three retained board owners were built
in both original graphs before the first cohort, and only the subsequently
changed selector test is rebuilt before these final complete reruns. The
initial failures remain in `query-owner-test.log`; x64's earlier focused
query pair also passes 2/2. These are CPU-owner regressions, not complete
repository, desktop, RDP or Windows GUI qualification.

The same-class sweep retains every remaining Present check: actual data/code/
stack, LDTR/TR, interrupt/call gate, task and return loads have real mandatory
presence semantics. Query null/bounds/unusable-LDT and conforming privilege
checks stay with their existing owner. SLDT/STR/SMSW selector write width is
not changed merely because newer CPUs document different upper-register
behavior; those original-family undefined bits are not a new SX requirement.
LSL's existing result and granularity timing evidence remain distinct.

Actual continuation source is +4/-16 (net -12), tests +131/-8 (net +123).
Cumulative S8 source is +147/-84 (net +63), tests +4009/-1184 (net +2825),
including the new integer test and excluding manifests/docs/artifacts.
Shared-six, MyNES, owner inputs and accepted S7 artifacts remain unchanged.
The final finite C15 disposition and whole-S8 qualification/publication are
still open; no partial implementation P or production SX admission is made.

## Conditional Transfer, Counter And SETcc Final Functional Reconciliation

The F11 source/form reconciliation finds a proof gap, not a new production
defect: the retained Jcc and SETcc tests select only one true and one false
flag witness per predicate. LOOP/JCXZ have small count samples rather than
the complete size/counter-edge class. The pre-edit plan extends the existing
branch/SETcc owners, approximately 200 net test lines and no production
rewrite. Required suffix and deferred target-fetch owners are then included
when their actual selected-profile coverage is inspected. The final batch
changes four existing tests, not a new framework or decoder.

Primary authority is the pinned original 1992 PRM: Jcc printed 26-178--180
(PDF 676--678), LOOP printed 26-205--206 (PDF 703--704), and SETcc printed
26-259--260 (PDF 757--758). The operation pages are rendered and visually
checked at `build/t560-s8-research/condition-{jcc,loop,setcc}.png`.
Visual checking matters: extracted text misreads SETPO's 9B as 98 and JNL's
8D as 80; the actual original tables show the correct opcodes. This is not
authority to change production according to OCR errors. Jcc/LOOP operand
size controls the taken destination width, address size controls CX/ECX,
and none of these operations modifies the status flags. SETcc writes exactly
one destination byte according to the same sixteen predicates.

The new direct-execution matrices on both 386 and SX prove:

- 229,376 Jcc executions: all sixteen conditions and all 32 OF/SF/ZF/PF/CF
  images, short/near forms, real/protected CPL0--3/VM86, valid code defaults,
  66/67 combinations and both displacement signs. Additional protected
  high-EIP cases establish that a taken 16-bit branch clears the high half,
  while an untaken branch retains its normal sequential EIP.
- 14,336 LOOP/LOOPZ/LOOPNZ/JCXZ executions: the same applicable mode, code,
  prefix, direction and high-EIP contexts, both initial ZF values and eight
  count images (0, 1, 2, FFFF, 10000, FFFFFFFF, ABCD0002, 12340002).
  The independently computed expected state distinguishes count decrement,
  wrap, retained upper ECX, zero-test without decrement and destination width.
- 81,920 SETcc register executions: every condition/flag image and all eight
  low/high byte-register destinations in the applicable real/protected/VM86
  code defaults. Another 1,024 memory executions cover every condition/flag
  image. Existing operand/address-prefix memory checks remain.

This is 326,656 matrix executions, excluding retained legacy and failure
cases. New register/branch matrices compare the complete CPU snapshot with
an independently constructed expected state, reject exception/shutdown and
assert zero operand writes. They reuse the existing CPU execution and cache
invalidation entry points, not a second runner. A single fixture is reused
only within an immutable context; each operation explicitly rewinds/invalidates
prefetch and restores its inputs. Logical predicate equations are local test
oracles, not callbacks into production condition evaluation.

The former modern near-Jcc witness loop and four address-size samples are
removed because their full behavior is covered by the new matrices. The two
nontrivial upper-ECX seeds move into the counter matrix. Early 8086/186/286
witnesses, independent modern JMP forms, actual GP atomicity, pre-386 UD and
the existing old-family zero-displacement timing assertions remain. SETcc's
two-witness helper is replaced by complete flag-image construction plus one
owner-local expected-value function reused by its register and memory tests.
There is no new production helper, state, API or profile policy.

The existing required-fetch table adds short/near JZ/JNZ, LOOP variants,
near CALL, prefixed dword CALL/JMP, JECXZ and memory SETcc. Every required
suffix byte is injected through the real provider and must fail without
dependent operand reads, writes, port completions or CPU publication.
One private `requires_386` table field keeps modern forms out of early-family
cases; no duplicate fixture or special failure path is introduced. Both
false and true branch recipes still require their displacement bytes.

Actual inspection also finds `cpu_transfer_boundary` hardcoded to 386 with
386-only timing formulas. Its existing semantic recipes now run on SX too:
near/far/conditional/task transfers and returns retire before an unmapped
target's next fetch faults, invalid current references fail first, inner
privilege gates preserve the deferred target boundary, unused endpoints do
not fault, and restored-stack cleanup does not dereference skipped slots.
SX additionally exercises independent caller/gate widths; the old matching-
width 386 timing recipes are preserved rather than weakened. Exact original
286/386 timing assertions remain unchanged. SX explicitly reports its current
unallocated compatibility origin, not borrowed 386 costs; S9 still owns exact
timing and must replace that non-eligibility assertion when it qualifies them.

The first transfer-test build fails on one missed prepare caller and one
lib_size printf argument. Those test-only mistakes are corrected with the
actual profile parameter and existing lib_u32 formatting; the failed build
is retained at `build/t560-s8-x64-transfer-profile-build.log`. No production
error behavior or warning policy changes. The focused transfer rechecks pass,
including the additional independent-width cases. All other development
condition/fetch cohorts pass, not an invented fail-first production defect.

The final stable four-owner batch rebuilds in both original graphs, alongside
the global Types gate. Both complete 102-case CPU/board cohorts pass:
`build/t560-s8-{x64,x86}-condition-complete-{build,test}.log`, 2.89/3.14 s.
Earlier focused and complete snapshots remain in the condition and transfer
logs and are not relabeled as these final tests. This completes F11's
predicate/count/size functional reconciliation together with the retained
actual SX near CALL/RET owner and the original common transfer helpers.
It strengthens F12's deferred-fetch proof but does not accept the entire
far/task source/form universe merely from this boundary test.

Actual continuation test delta is +295/-165 (net +130). Cumulative S8 source
remains +147/-84 (net +63); tests are +4304/-1349 (net +2955), including the
new integer owner and excluding manifests/docs/artifacts. Removed examples
are replaced by stronger same-owner proof; the positive delta is explicit
mode, predicate, alias, failure and boundary qualification, not product code.
Other finite-family dispositions and whole-S8/artifact qualification remain
open. Shared-six, MyNES, owner inputs and accepted S7 artifacts are unchanged.

## String Page-Fault Prefix, Guest Repair And Real Return

F13 still needs exact restart proof beyond the retained tests which deliberately
leave exception receivers unusable and observe shutdown. The pre-edit plan
uses the existing CPU exception owner and REP interrupt owner, approximately
200--260 test lines, without production edits or a new fixture/framework.
Original PRM 3.6.1 and REP printed 26-244--246 specify restart EIP, remaining
count and next uncompleted element. Those are the primary contract for this
batch; the five primitive access/publication owners were already reviewed
above and remain unchanged.

One private case in `cpu_execution_fault_event` now supplies 9,072 page-fault
and complete recovery scenarios on actual 386/SX. It combines five operations,
all seven applicable source/destination fault positions, byte/word/dword,
both repeat encodings, both directions/address sizes, valid protected code
defaults, CPL0--3/VM86 and faults after zero/one/two completed iterations.
Another 9,072 zero-count controls use the same absent operand pages and must
complete without any CPU-state change except normal EIP advancement. Existing
single/compare-result/segment/LOCK tests remain at their five original owners.

Each actual fault must be PF with the correct read/write and user error,
CR2, restart EIP and saved CS/control flags. Prefix counts and indexes are
checked independently before the fault, then must remain at that checkpoint.
Physical source bytes never change; only completed MOVS/STOS destination
elements may change. LODS retains the last completed load and all unrelated
GPRs remain sentinel values. This is not a missing-IDT/shutdown surrogate.

The actual guest handler saves EAX/DS, loads a valid data selector, writes the
missing PTE with an ordinary guest MOV, restores those registers, drops the PF
error and executes dword IRET. Distinct original/handler code descriptors keep
the original code default intact. No test-side CPU-state/page repair is used
for recovery. The test verifies the PTE write, exact return EIP/SP, remaining
count/indexes, then executes every remaining iteration and checks final data,
count, indexes, EAX and untouched GPRs. VM86 uses the real TSS/privilege-entry
frame and returns through actual IRET; it is not simulated with a copied CPU.

The first run fails with a test-process access violation, retained in
`build/t560-s8-x64-string-restart-test.log`. Object/PE inspection rules out
the proposed stack cause (the new frame is about 556 KiB under a 2 MiB stack).
Actual cause: the new backward fixture added an unsigned negative delta to a
host pointer before completing guest-address arithmetic. It now computes the
32-bit address first, then forms the host pointer. All four new occurrences
are corrected; a same-class CPU-test search finds no other matching additions.
No production bounds, compiler warnings or expected architectural fault is
weakened to hide this fixture mistake. The corrected initial and stronger
state proofs pass focused checks.

The existing real REP MOVSB interrupt case now includes SX and verifies one
committed element, remaining count/indexes and saved REP EIP. Its input IRQ
is delivered through the actual bus, not a manufactured callback completion.
This change does not claim that every REP event/debug combination is covered
by that single IRQ case; retained S6/S7 event proof and the following flag
checkpoint question remain separate.

### Explicit Compare-Fault Flags Question

This batch uncovers a source question which is not silently accepted or named
architecturally undefined. The reviewed REP/exception sections of the pinned
1992 PRM do not supply the later explicit whole-instruction EFLAGS sentence.
Extracted-text searches of the original 1990 386 PRM add no positive source;
that is not proof that every original page has been exhaustively excluded.
The 1990 document SHA-256 is
`9A8188F9D2282B113FC421E225CC2A643FCDC349E5C3C43659BD2CF6620F1EA1`;
its ignored extracted text is `build/t560-s8-research/386-prm-1990.txt`.
The read-only 1997 embedded-486 manual adds no resolution of this checkpoint.
The already pinned Intel 325383-078US reference, printed 4-562, and Intel's
[official instruction reference](https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-2b-manual.pdf)
describe restoration of the pre-instruction EFLAGS when repeated compares
fault. That is stronger than the present per-iteration oldcpu checkpoint.

The bounded actual diagnostic deliberately contrasts the two states:
for REPE CMPS/SCAS after one equal comparison, initial arithmetic flags are
08D5, last-success flags 0044 and saved fault flags 0044 on both 386 and SX.
`STRING-COMPARE-CHECKPOINT` records all six direct witnesses in the test log.
The new tests assert nonarithmetic controls and frame identity, and all flags
for the noncompare operations, but deliberately do not assert that 0044 or
08D5 is the correct selected-SX compare-fault arithmetic checkpoint.

The pinned read-only 86Box REP body and Bochs 2.6 repeat_ZF body commit
per-iteration comparisons but do not themselves specify an initial-flags
fault checkpoint. Their exception/front-end machinery requires separate
inspection; these loop bodies are not proof of the full reference outcome.
Their implementation is behavioral research, not authority to override Intel
or a claim of measured 486 silicon.
Researched files SHA-256:
`x86_ops_rep.h` =
`3A6C9D7E73FE804BC42D9393788082115D5C56C318C3B3702332244D57E3D9E0`;
Bochs `cpu/cpu.cc` =
`3C0327B54395A1CB3E64ACD7713BDB3FE659136C5FB688588B6A4CA9C8AC807D`.
No source is copied or transliterated.

This refines, rather than shrinks, F13's existing fault/EFLAGS axis: initial,
first/subsequent iteration, GP/SS/PF/AC, actual interruption/resumption and
reset/debug/SMM lifetime must reconcile at the CPU checkpoint owner before
F13/C15 acceptance. Do not add a guessed second flag owner or accept the
masked assertion as full flag proof. The entire class remains in S8; it is
not transferred out of T560 and does not justify opening production SX.

Before only the final zero-count addition, both complete CPU/board cohorts
pass 102/102 at `build/t560-s8-{x64,x86}-string-complete-{build,test}.log`,
2.92/3.37 s. After that test-only addition the affected owners rebuild and
pass separately, 2/2 in 0.24/0.32 s (`string-zero-final` logs). Both Types
gates pass. This is not another complete final-suite run or whole F13
acceptance. No desktop, RDP, Windows GUI or final artifacts are qualified.

Actual continuation test delta is +209/-1 (net +208). Cumulative S8 source
remains +147/-84 (net +63), tests +4513/-1350 (net +3163), including the new
integer owner and excluding manifests/docs/artifacts. New production helpers,
state and APIs: zero. Shared-six, MyNES, owner inputs and accepted S7 EXEs
remain unchanged. Full C15 and complete-S8 release/publication remain open.

### Compare-Fault Source Decision And Pre-Repair Boundary

The coordinator resolves the recorded source question as a bounded L2 legacy
contract, not a claim that the original 1992 wording explicitly specified the
checkpoint or that particular silicon was measured. The already pinned Intel
325383-078US, printed 4-562, specifies pre-instruction EFLAGS restoration for
faulting repeated CMPS/SCAS without limiting that legacy statement to newer
modes. The original 486 restart/count contract does not contradict this
additional flags detail. Reference loop bodies cannot override that explicit
primary contract. Selected SX will adopt it; retained 386 result policy stays
unchanged. This is an in-scope source-qualified accuracy repair, not permission
to copy later 64-bit or fast-string implementation features.

The complete mechanism class is byte/word/dword CMPS/SCAS with F2/F3, first
and subsequent iterations, real/protected/VM86 and faults versus asynchronous
events. The CPU owns the initial flag image until the repeated instruction
finishes, faults, or is genuinely interrupted/replaced. Hardware wait/HOLD
does not end it. Interrupt/debug/SMM entry and successful external state
replacement do. Required fetch, data-protection failure and preview must use
the same existing instruction/fault owner; count/index and already accepted
memory effects keep their normal iteration checkpoint.

Before production edits, the active packet explicitly includes two transient
CPU-private fields: lib_bool validity and lib_u32 initial EFLAGS. Files are
cpu_instructions.h, cpu.c and cpu_instructions.c, with approximately 35--50
added production lines and focused existing-owner tests. No public ABI, enum,
helper framework, queue, thread, second mutable CPU or duplicate repeat
identity is added. Existing source_repeat_* state belongs to timing and is
not reused as a semantic truth source. Its exact timing redesign remains S9.

### Compare-Fault Repair And Actual Qualification

The three existing production owners now implement that decision. ExecIns
captures initial flags only for selected-SX repeated CMPS/SCAS. ExecFinal
retains normal iteration count/index checkpoints but substitutes the initial
flags on GP/SS/PF/AC. Completion and fault termination clear validity. Actual
interrupt/exception/SMM entry, successful external debug patch, committed entry
and resets clear it; rejected patches, aborted entry, preview, product pause
and qualified physical wait do not. No individual compare handler is rewritten.
The two private fields are payload and validity, not a second mutable CPU,
repeat identity or mirror of timing state. Existing 386 behavior is retained.

The fail-first `compare-flags-before` run exposes saved 0044 instead of
initial 08D5 after one completed SX comparison. After repair the existing
9,072 real PF/guest-repair/IRET scenarios assert the complete saved EFLAGS,
with the appropriate family-specific checkpoint; the 9,072 zero-count
controls remain. Prefix counts, indexes, accepted stores, unrelated registers
and real return assertions are not weakened.

Additional bounded cases use the existing owners and fixtures:

- 44 compare-fault cases: first-iteration CPL3 AC plus subsequent-iteration
  protected GP/SS and real-mode GP, applicable widths and both repeat bytes;
- seven lifecycle cases: preview, pause/read, qualified clock wait, accepted
  versus rejected debug patch, hard/soft reset, aborted versus committed entry
  and ordinary final completion;
- eight actual CMPS/SCAS NMI/TF cases, proving last-completed flags at async
  entry, clearing the image and real NMI return establishing a fresh origin;
- four actual SMI/save-image/RSM cases proving last-completed flags, no hidden
  transient payload in SMRAM and a fresh origin after RSM.

Two existing CMPS/SCAS shutdown witnesses required their selected-SX flag
expectations to follow the new source contract; their retained 386 expectations
stay unchanged. Those tests remain shutdown witnesses, not substitutes for
the new precise vector/frame proofs. One new lifetime fixture initially used
the wrong STPCLK/CLK restart order; it was corrected to the existing qualified
pin contract, without a production workaround. Initial failures are retained
in the ignored `compare-legacy`, `compare-lifetime` and diagnostic logs.

After the production/context-layout change all 93 CPU executables and three
board owners are rebuilt in each original Ninja/ccache graph. Both complete
CPU/board cohorts pass 102/102, 14.58 s / 6.85 s, in
`build/t560-s8-{x64,x86}-compare-complete-{build,test}.log`. Only the final
12 real-mode GP cases follow those aggregate runs; final five affected-owner
rebuild/rechecks pass 5/5 per width, 0.29 s / 0.34 s, in `compare-final` logs.
Both Types checks pass. These are not whole-repository or release results.

This continuation changes production +35/-2 (net +33); cumulative S8
production is +182/-86 (net +96), tests +4776/-1354 (net +3422), including
the untracked integer owner and excluding manifests, documentation and assets.
No public function/type, helper framework, Shared-six, MyNES, owner input or
artifact changes are introduced. Final finite-family reconciliation and full
S8 qualification/publication remain required; S9/S10 are not bypassed.

## F17 Remaining Real/Flags Owner Reconciliation Plan

Actual selection audit finds pushf_popf, iret_state and software_int_state
still stop at 386. Their shared real-mode/attribute, protected IOPL and VM86
assertions therefore do not establish selected SX behavior. Original PRM
printed 26-235/240 supplies flag-stack width, privilege and failure contracts;
existing S6 flag/source qualification supplies the selected later feature
bits and VME distinctions. Existing IRET/INT and real-handler proofs remain
separate from these selection gaps. Extend these three existing owners,
approximately +60--100/-35--65 test lines, preserving early-family rejection,
word/dword selection, stack publication and all old-profile expectations.
No production change is justified yet. In particular a shutdown witness
remains a shutdown witness, not proof of its first fault vector. Do not copy
a second fixture or infer selected timing from the 386 result.

### F17 Actual Extension And Review

The three owners now replay modern forms on both 386 and SX, keeping the
old profiles and their exact expectations. PUSHF/POPF's existing default,
attribute, LOCK, word/dword, protected IOPL, VM86 and stack-failure assertions
all gain actual SX selection. Its privilege matrix executes 2,048 SX cases
across CPL, old IOPL/NT, popped IOPL/NT and old/new IF; retained 286/386
counts are 1,024/2,048. Early-family attribute rejection is split from modern
positive execution so it runs once rather than once per modern profile.
SP=1 remains an explicit shutdown test, not a precise first-vector oracle.
S6's selected AC/ID/VME proofs remain their existing owners, not inferred
from this low-word flag matrix.

Real IRET adds SX default and all operand/address-prefix forms with its
original return target, stack advance, flags and unchanged-register/cache
checks. Modern LOCK rejections include SX while pre-386 attribute rejection
stays pre-386. Software INT3/INT n/INTO, taken versus clear-OF INTO, modern
prefixes and LOCK also gain SX selection. The real transfer test additionally
reads the actual accepted stack frame: next EIP, saved CS, defined flags,
fixed bit 1 and dword upper half. It first checks the expected SP and success
boundary before reading fixture memory, avoiding a host out-of-range read
if a future bad CPU implementation corrupts SP. No exception is manufactured.
The existing terminal-rejection proof is not relabeled as a real UD frame.

Original production PUSHF/POPF/IRET owners and their common flag/stack routes
remain unchanged. No new framework, fixture, handler or public interface is
introduced. Initial selection rechecks pass 3/3 per width (0.42/0.36 s).
After the explicit frame assertions and removal of positional family
selection, final affected-owner rebuild/rechecks pass 3/3 per width,
0.29/0.27 s, in `build/t560-s8-{x64,x86}-flag-entry-final-{build,test}.log`.
Both Types checks, regenerated Core manifests, the 44-owner fixture
lifecycle gate, documentation governance and whitespace check pass.

Actual three-file test delta is +107/-67 (net +40). Cumulative S8 tests are
+4883/-1421 (net +3462), including the new integer owner and excluding
manifests/docs/artifacts. Production remains +182/-86 (net +96). This closes
these selection holes, not all remaining F17 mode/failure reconciliation or
the finite C15 acceptance. Shared-six, MyNES, owner inputs and S7 artifacts
are unchanged; whole-repository/release qualification remains required.

After those last test-only changes, the complete CPU/board cohort passes
102/102 per width again, 2.92 s / 3.17 s, in
`build/t560-s8-{x64,x86}-flag-entry-complete-test.log`. Production objects
were already rebuilt after the context-layout change; all subsequently
changed test owners were rebuilt before this execution. This is not a
full Core, public-six, App, external integration or final EXE qualification.

The remaining no-literal-SX owner audit is an investigation list, not a
claim that all such files are defects. Next reconcile F16 descriptor-table
store/load and task-register owners; then the remaining shared addressing,
segment/prefix and VM86 return owners against their actual setups and the
accepted S2--S7 mechanisms. The old second/final-group arithmetic owners
also need explicit disposition against the expanded ALU matrices. Files
covering only an old-profile distinction need no mechanical SX insertion.
Final F01--F20 disposition and complete-S8 release qualification remain open.

## F16 Descriptor-Table And Task-Register Continuation Plan

Actual review of sgdt_sidt, lgdt_lidt and descriptor_table_register finds
modern execution stopped at 386. The existing descriptor_system owner already
selects SX for layout, privilege/terminal failure and precise selector faults,
but its successful protected table stores only execute CPL0/dword. Reuse all
four existing owners: add selected SX to modern forms, retain 286 compatibility
and early rejection, and expand table stores to word/dword, CPL0--3 and VM86.
Estimated test-only +100--160/-60--110. No production change is justified.

Original PRM 26-261/262 gives six-byte stores, no flag changes, memory-only
forms, segment/PF/AC failures and an explicit compatibility note: word-form
byte six is FF on 286 and zero on 386/486. The neighboring LGDT description
26-193 has conflicting upper-byte wording. Retain the already recorded
instruction-specific disposition, not a new silicon claim or changed behavior.
LGDT/LIDT consume a 24-bit base for word operands and all 32 base bits for
dword operands. Privileged loads and unprivileged stores remain distinct.
Existing exact LLDT/LTR GP/NP vector/frame proof remains authoritative;
old missing-IDT shutdown witnesses are not upgraded by relabeling them.

The first four-owner replay passes both widths. Further source review finds
the existing table-load privilege negatives deliberately terminate without a
valid IDT, so add approximately 100--140 lines to that same descriptor_system
owner for actual inner-privilege delivery, not a new fixture. Cover LLDT/LTR
and LGDT/LIDT at CPL1--3 and VM86 with both operand/address sizes. LLDT/LTR
in VM86 require UD, not a privilege GP. The original LLDT VM86 specification
agrees with the pinned later legacy reference. Original LGDT/LIDT 26-194's
VM86 line omits the privilege prohibition; the explicit later Intel legacy
VM86 GP statement (3-601) qualifies this retained behavior at retrospective
L2, not measured silicon. Protected nonzero-CPL GP is explicit in the original.
Check real vector/error, saved frame, old/new stack, unchanged table/task
registers and unchanged source bytes. No production repair is inferred yet.

### F16 Actual Owner Proof And Results

All three retained specific owners now execute modern forms on selected SX
as well as 386. The old 286 two-form compatibility checks and early-family
rejections are retained and run once, rather than duplicated per modern CPU.
Small local form tables replace repeated call chains; they are immutable test
data, not an alternate decoder. LGDT/LIDT now checks unchanged GPR/flags for
both target registers, instead of returning early for IDTR and skipping those
assertions. DTTR's LLDT/LTR/SLDT/STR and null-LDT/prefix tests gain SX without
changing their old expectations or production implementations.

The existing successful-store owner covers 20 mode/form combinations per
profile: SGDT/SIDT, word/dword and CPL0--3/VM86. It compares all six result
bytes plus both adjacent sentinels, and the complete architectural CPU image
except the expected EIP advance. It verifies unprivileged stores without
relying on the invalid-IDT failure helper.

One new private test function in descriptor_system supplies 128 actual
privileged-load exception scenarios total, 64 on each 386/SX profile. It uses
the same fixture, GDT and actual TSS inner-stack mechanism, not a new mock
entry path. LLDT/LTR/LGDT/LIDT combine CPL1--3/VM86 and both operand/address
sizes. Assertions require the correct GP versus UD, zero error where present,
original instruction point, ring-zero target/stack, full saved return and
VM86 segment frame, unchanged GPRs, tables/task registers and source bytes.
These source-byte assertions prove no mutation, not the absence of a read;
no read counter is invented or claimed. The frame is read only after the
expected actual stack address is verified. Existing real selector GP/NP
proof remains; old shutdown tests are not relabeled as precise exceptions.

Initial four-owner selection/mode checks pass 4/4 per width, 0.43/0.32 s;
the additional vector owner first passes x64 separately. Final four-owner
rebuild/rechecks pass 4/4 per width, 0.18/0.18 s, in
`build/t560-s8-{x64,x86}-table-final-{build,test}.log`. Both final CPU/board
cohorts then pass 102/102, 2.85/2.99 s, in `table-complete-test.log`.
Both Types checks and regenerated Core manifests pass. No production
instruction, state, API, Shared-six, MyNES, owner input or EXE changes follow
from these passing tests. Exact timing is still explicitly outside this proof.

Actual continuation test delta is +242/-80 (net +162); cumulative S8 tests
are +5125/-1501 (net +3624), including the untracked integer owner and
excluding manifests/docs/artifacts. Cumulative production remains +182/-86
(net +96). The new local test function is necessary for real privilege-entry
proof and creates no fixture/framework or production helper. Remaining
address/segment/prefix/VM86 owner reconciliation, final finite C15 disposition
and complete repository/integration/eight-artifact qualification remain open.

## Shared Operand-Span Owner Reconciliation Plan

Actual address_span setup stops at 386, including cases where the bit-index
owner and paired-read instruction owners themselves already select SX.
Extend the existing single span fixture: old-family real wrapping, XLAT sum
width, expand-down inclusive/exclusive limits, signed bit offsets, immediate
bit masking, composite scalar phases, failed first/second provider and VM86
versus retained PE-clear segment limits. Approximately +40--70/-20--40 test
lines; no production change, fixture, bus or public interface is planned.

Original PRM segment-limit discussion and XLAT 26-285 define effective address
width and expansion bounds; existing S3 composite-access source qualification
and its reviewed common preflight owner remain inputs. Provider IO errors
are nonarchitectural CE, not a manufactured guest PF. Invalid protected spans
with absent gates remain first-gate-access witnesses, not precise delivered
GP/SS vectors. Existing real-handler span failures and precise S3/S8 owners
retain their distinct proof. Do not transplant 386 timing assumptions to SX.

The same sweep finds operand_address's four protected execution groups are
also fixed to 386. They use real guest LGDT/LMSW/segment/far-JMP bootstrap,
not copied protected caches: replay their 16/32-bit code, address/operand
prefix, SIB, DS/SS defaults, stack/ENTER/LEAVE and string assertions on SX
through a profile parameter. Expected approximately +40--60/-25--40 lines,
test-only. Its run_gp helper deliberately observes shutdown with no IDT;
do not report that as a precise first-fault GP proof. The exact S3/S8 owners
retain that separate responsibility. No timing expectation is present here.

The remaining prefix_attributes owner has eleven modern groups hardcoded to
386 and one embedded early-family attribute rejection group. Replay the eleven
groups on 386/SX through explicit profile arguments, and extract only that
embedded legacy group so it still executes once. Approximately +70--110/
-60--100 test lines; no production or new fixture change. Preserve segment
selection, operand/address widths, actual LOCK stores and rejection, fixed
string destination, REP counts/data and flags. Existing missing-IDT negatives
remain shutdown witnesses. Repeated same-group prefixes, mixed F2/F3 and F2
MOVS retain the established compatibility policy; these tests alone are not
new silicon evidence or a claim that original legal-prefix text specified all
those combinations. Exact repeat fault/state and LOCK/RMW semantics remain
at their existing precise owners.

### Actual Span, Protected Address And Prefix Proof

The existing span owner now executes 1,025 additional SX scenarios derived
from its finite loops: 480 expand-down limit/offset/width/DS-versus-SS cases,
144 signed bit-index cases, 72 immediate-bit cases, 320 composite operand
phase/failure cases and nine XLAT/real-limit/retained-cache controls. Its old
8086/8088/186/286/386 cases and first/second-provider CE assertions stay intact.
Modern address and width decisions use explicit family comparisons, not the
position of SX in a test array. Physical transfer widths, addresses, accepted
read prefixes and architectural non-publication remain asserted. Protected
missing-gate observations remain first-gate witnesses, not full exception
frame evidence. Initial x64/x86 owner passes are 0.20/0.16 s.

The protected operand_address owner now executes its same four groups on
386/SX using 26 existing setup sites parameterized by profile. Guest code
actually loads GDTR, sets PE with LMSW, loads data/stack selectors and enters
16/32-bit protected code by far JMP before the tested program runs. Retained
SIB, scaled/absolute address, BP/SS/DS defaults, operand/address prefixes,
stack big-bit, ignored POPA slot, ENTER/LEAVE, strings and sequential fetch
limit assertions are not replaced by copied protected caches. Its absent-IDT
negative helper remains a shutdown witness. Both initial owner rechecks pass
in 0.15/0.13 s with no production edits.

All eleven prefix groups now run on both modern profiles through the same
test bodies. The old embedded early-CPU rejection group has its own private
test function and is invoked once, not once per modern CPU. A small immutable
main-case table keeps the original diagnostic names and one invocation path;
no alternate decoder or instruction driver is introduced. The extracted
legacy block is flattened, and a now-redundant final failure branch removed.
Existing byte/register/flag/segment/memory assertions stay intact. Repeated
and historically noncanonical prefix combinations keep their explicit model
qualification; this extension neither adds a new compatibility branch nor
promotes those observations to silicon L1. The final three-owner selection
passes 3/3 per width, 0.21/0.20 s, before that last formatting-only cleanup.

After rebuilding the final changed test in both original cached graphs,
the complete CPU/board cohorts pass 102/102 per width, 2.93/3.13 s, in
`build/t560-s8-{x64,x86}-address-prefix-complete-test.log`. Both Types checks,
regenerated Core manifests, the existing 44-owner fixture lifecycle check and
whitespace check pass. These results are not whole-Core/repository, external
integration, desktop/RDP or release qualification.

Actual three-file test delta is +205/-191 (net +14), including the extracted
legacy block's indentation changes; no test body or assertion is dropped to
reduce lines. Cumulative S8 tests are +5330/-1692 (net +3638), including the
new integer owner and excluding manifests/docs/artifacts. Production stays
+182/-86 (net +96). No public API, production helper/state, Shared-six,
MyNES, owner-input or artifact change is introduced by this continuation.

The no-literal-SX investigation index now contains twelve retained owners:
contract, eflags_local, execution_bus/context/paging, inc_dec_second/final,
les_lds, protected_data_access, segment_selector, vm86_delivery_state and
vm86_iret_state. This is not twelve proven defects. Their actual setups and
assertions must be compared with selected-SX proof and source-specific timing
or older-family duties before adding redundant cases or accepting their
disposition. Final F01--F20/C15 reconciliation and full S8 release remain open.

## VM86 Entry And Return Owner Reconciliation Plan

Actual inspection finds vm86_delivery_state and vm86_iret_state fixed to 386,
including the IRQ context's second explicit bind. Add explicit modern profile
arguments to their existing helpers and invoke both 386/SX from main. Keep
the real GDT/IDT/TSS frame, IRQ bus and paging fixture; do not synthesize
completion or introduce a second interrupt path. Estimated test-only
+100--150/-60--100 lines. The IRQ bus must provide the existing special-cycle
callbacks because its receiver executes HLT on SX; this is fixture wiring,
not a new production service. Existing accepted S6/S7 interrupt/VM86 sources
and original IRET/virtual-mode rules remain the contract, not 386 timings.

Strengthen the partial-frame IRET negative: unchanged CPU alone is too weak
to distinguish the intended no-IDT shutdown from a silent no-op. Require
actual shutdown delivery with no host fault/stop, plus unchanged source frame.
The remaining malformed-entry fixtures intentionally observe terminal rollback;
they are not precise first-vector proof. Exact saved-frame and real return
proof remain the successful entry/return groups and previously accepted owners.

The same review finds protected_data_access's default and three modern groups
also stop at 386. Parameterize the modern attribute, failed-access and
expand-down cases; retain its existing 286 attribute rejection exactly once.
Estimated test-only +35--55/-20--35 lines at that existing owner. Keep its
full register/cache and memory assertions; no production repair is inferred.
Also add the existing SX name/early-semantics pair to cpu_contract's identity
table (+1/-1), which does not construct a running SX or open the Core gate.
The 386 eflags_local duplicate flag-value witnesses need no automatic profile
expansion: selected SX already has all 256 SAHF/LAHF images and direct-flag
proof at the stronger current owners, as recorded above. Its retained old
callback-retirement witness remains separately auditable, not silently removed.

### Actual VM86/Protected Owner Proof And Results

Both VM86 owners now execute their retained 386 cases on selected SX through
explicit profile arguments, including the IRQ context's actual second bind.
The IRQ fixture supplies the existing special-cycle acknowledgement/cancel
callbacks for its real HLT receiver. IRQ/NMI, UD/GP/NM, TF and execution
breakpoint entry retain their real TSS/IDT/GDT, saved VM86 segment frame and
null data-cache assertions. Invalid gate/TSS/SS0/short-stack cases still prove
terminal rollback, not a fabricated first exception vector. The IRET owner
retains full six-cache reconstruction, polluted selector high halves, paging
and word/dword incoming-RF cases; its partial-frame failure now also requires
actual shutdown, no host fault/stop and unchanged source stack bytes.

Private fixture input EFLAGS now includes architecturally fixed bit 1, and
the saved-frame assertions include that same bit instead of relying on an
unphysical raw fixture input. GP's diagnostic and real frame error must both
be zero. An initial strengthened test incorrectly required every diagnostic
exception_code to be zero, causing the x64 four-owner run to pass 3/4.
Existing `_debug_deliver_trap` deliberately records its DB cause in that
diagnostic field; it is not a pushed architectural error word. The corrected
assertion applies the zero-error contract only to GP, retaining all original
DB cause/DR6/frame assertions and requiring the real GP error word. No
production change or weakened GP check hides this test-oracle mistake.
The initial failure is retained in the `vm86-protected-final` log, followed
by passing `vm86-protected-corrected` builds/tests on both widths.

Protected data accesses also replay SX with retained DS/SS/ES defaults,
32-bit attributes, LOCK rejection, invalid/nonwritable/limited/expand-down
segments, unchanged complete register/cache checks and destination sentinels.
The old 286 attribute rejection is separated into one private test function
so it executes once; no fixture or decoder is duplicated. CPU contract adds
only SX's existing identity/early-semantics pair; its executable constructor
scenario stays the original 8086 test, not production SX admission.

Initial VM86 owner replays pass 2/2 in 0.29/0.19 s; initial protected/contract
replays pass 2/2 in 0.26/0.20 s. Final corrected four-owner rebuild/rechecks
pass 4/4 per width, 0.25/0.34 s. After these final test changes, complete
CPU/board cohorts pass 102/102 in 3.09/3.39 s, in
`build/t560-s8-{x64,x86}-vm86-protected-complete-test.log`. Both Types gates,
regenerated Core manifests, the 44-owner fixture lifecycle gate and whitespace
check pass. These are not full Core/repository, external integration, manual
desktop/RDP or updated-artifact acceptance.

Actual four-file test delta is +125/-77 (net +48). Cumulative S8 tests are
+5455/-1769 (net +3686), including the new integer owner and excluding
manifests/docs/artifacts. Production remains +182/-86 (net +96), with no
new source/state/API change in this continuation. Shared-six, MyNES, owner
inputs and S7 artifacts remain untouched. The remaining eight no-literal-SX
owners are still an investigation index, not eight proven defects or an
exhaustive instruction coverage metric. Complete C15/release remains open.

## Remaining Reset/Observer And Old-Model Owner Disposition

Actual execution_paging inspection shows why no-literal-SX is not a defect
metric: its INVLPG negatives intentionally cover CPUs before 486; its CR3
alignment and reserved-control expectations are retained 386 policy, not
the selected SX write mask. Earlier actual S2/S4/S5/S6 owners prove selected
SX control, invalidation, paging and reset behavior. Keep these older-family
tests rather than copying their UD/CR3 expectations into SX.

Execution_context's debug patch/current-versus-entry snapshot and immutable
copy assertions, and execution_bus's actual reset fetch, write observer,
port completion/failure and interrupt acknowledgement checks are useful direct
SX observer coverage. Add SX to their existing tables and allow dword port
widths for >=386 (about +4/-3 lines). Keep the early REP resume-PC test's
8086/186 behavior separate; its selected-SX counterpart is already proved at
the actual repeat/event owners. No source/interface/state change is planned.

Segment_selector's retained modern groups likewise still stop at 386.
Parameterize its nine functional groups and VM86 segment-pop observation,
including their one shared LxS-success helper, and replay 386/SX through the
existing guest bootstrap and diagnostic owner. The 286-specific legal/cache
rejection groups and metadata stay separate and execute once. The existing
mixed-profile rejection table adds SX only to modern-applicable entries;
early FS/GS/prefix rejection must not be generalized. Preserve 386's exact
7-tick VM86 POP assertion; selected SX must explicitly remain unallocated
until S9, not borrow 386 timing or silently discard the observation.
Estimated test-only +90--130/-70--110 lines, no production change or new
fixture. Protected missing-IDT cases remain terminal rollback witnesses,
not precise NP/SS/GP proof. Prior exact selector/mode/fault owners remain
their distinct evidence. Original selector/LxS/query contracts reviewed above
and real guest bootstrap are the basis; no new semantics are inferred.

The older second/final arithmetic group audit exposes a real remaining F01
proof gap, rather than just a no-literal-SX file: legacy_alu's Group-1 helper
and loop only emit 8/16-bit immediates. Its full dword binary matrix does not
prove 81/83's separate immediate decode/sign-extension. Extend that existing
helper to 32 bits on 386/SX, a nine-byte maximum encoding, actual operand
width for memory, and both input CF states. Keep all old widths/profiles and
82's byte-only compatibility alias. Approximately +20--35/-10--20 test lines;
no production repair inferred. The original arithmetic/immediate contract
and already corrected independent lib_u64 flags oracle remain unchanged.
Do not mark the old witnesses redundant before this missing form is proved.

### Actual Observer, Segment And Group-1 Proof

The existing reset/observer and context tables now include SX. Dword ports
are enabled for >=386, retaining old-family width selection and every original
completion, rejected-transfer, interrupt-acknowledgement and immutable snapshot
assertion. The first attempted build used a nonexistent canonical context
alias; inspection locates its actual `core-machine-cpu-context-smoke` target
and separately registered unit name. No alias/registration change is added.
Actual owner rebuild/rechecks pass 2/2 per width, 0.59/0.24 s.

Segment-selector's nine functional groups plus VM86 segment-pop observation
now execute both modern profiles, including their common LxS helper and
actual LGDT/LMSW/segment/far-JMP bootstrap. The 286 groups and static metadata
run once; the mixed rejection table gains only modern-applicable SX entries.
All register/cache, descriptor accessed-byte, query, stack and rollback
assertions remain. 386 keeps its exact seven ticks; SX explicitly retains
unallocated timing pending S9. Its initial owner passes 1/1 per width,
0.19/0.15 s. No production behavior or timing formula is changed.

Group-1's existing ALU helper now emits 81/83 dword forms on 386/SX, with a
nine-byte maximum instruction, correct four-byte memory operands and signed
FF immediate extension to the selected width. Both input CF values execute
all widths/profiles; 82 remains the byte alias only. The finite group now
executes 640 cases rather than 256, including 160 on 386 and 192 on SX.
All full flag-oracle, instruction length, data and CMP non-write assertions
remain; expected flags still use the independent existing lib_u64 oracle.
Old carry-one encodings/expectations are retained. Both expanded ALU owner
checks pass 1/1 in 0.67/0.64 s, with no production repair required.

The remaining no-literal-SX files have these specific retained duties; absence
of that token is not accepted as either proof of correctness or a defect:

- execution_paging: old-family LGDT/control/INVLPG rejection and retained
  386 control policy. Selected SX is proved by S2/S4/S5/S6 actual control,
  paging/TLB and reset owners. Do not copy its older UD expectations.
- eflags_local: retains old callback/flag witnesses. Selected SX has all
  256 SAHF/LAHF images and direct flag/privilege checks at the expanded
  lahf_sahf/direct_flags owners, not an inferred enum range.
- les_lds: retains original early/386 bootstrap and basic rejection witness.
  Selected SX is directly proved by the stronger les_lds_profile_matrix,
  operand-span and newly expanded segment-selector guest-bootstrap paths.
- inc_dec_second_group: retained TEST/ADD/ADC/SBB encoding, boundary and
  earlier attribute witnesses. Selected forms use expanded legacy_alu
  binary/accumulator/TEST/Group-1 and RMW/address/prefix owners; the actual
  missing dword immediate form discovered here is now proved as above.
- inc_dec_final_group: retained OR/AND/SUB/XOR/CMP, old BCD/XLAT and older
  Group-1 witnesses. Selected SX uses the expanded arithmetic/Group-1,
  complete adjust-input/value, span and prefix owners. Old BCD expectations
  are not blindly generalized across the explicitly repaired family boundary.

This closes the no-literal-SX owner investigation, not the final F01--F20
requirement-by-requirement reconciliation. No retained test is removed or
renamed and no whole-family assertion is based only on this file inventory.
After rebuilding all changed owners and Types checks, the expanded CPU/board
selection passes 103/103 per width, 2.87/3.28 s, in
`build/t560-s8-{x64,x86}-owner-complete-test.log`. The extra case is the
separately named context unit; earlier 102-case logs do not retrospectively
include it. Full repository qualification is still separately mandatory.
Both Core manifests, Types, fixture lifecycle and whitespace checks pass.

Actual continuation tests +97/-80 (net +17); cumulative S8 tests are
+5552/-1849 (net +3703), including the new integer owner and excluding
manifests/docs/artifacts. Source remains +182/-86 (net +96), unchanged this
continuation. No new production helper, state, API, Shared-six, MyNES, owner
input or artifact change. Final C15 reconciliation and whole S8 unit,
specialized, integration and eight-artifact publication remain required.

## Final Requirement Audit: Stack Receiver And Decoder Inventory

The first full repository snapshot rebuilds required units and executes all
605 x64 cases. It passes 601/605 in 263.57 s, failing both registrations of
the 80186 and 80386 decoder inventory runners. Their pre-write total-count
assertions fail without diagnostics; their actual admitted masks must be
reconciled against the S8 metadata changes before updating any expected
universe. x86 is not executed by this fail-first aggregate. This is neither
a timeout nor a passing full qualification. No new Shared/Lib failure is
inferred. The ignored `repository-units.log` retains the terminal outcome.

F09 audit confirms protected GPR-stack failure, alias/provider and SP=1
subgroups still hardcode 386. More importantly, multiline source review finds
`_e_push`'s special explicit-PUSH shutdown condition includes only 386, not
SX. The earlier single-line equality query missed that wrapped comparison;
its eight-hit disposition was incomplete and is superseded by the multiline
sweep here. Original PRM 26-237 explicitly requires shutdown for SP/ESP=1 in
real mode and gives VM86 the same rule. PUSHF already has the correct >=286
path. Do not widen all 386 equalities or change CALL/ENTER/PUSHA semantics.

Before production repair, add an existing-owner PUSH regression with a valid
VM86 TSS, SS receiver and ring-zero delivery stack. Every explicit GPR,
immediate, memory and segment PUSH plus PUSHF, word/dword, must shut down
without entering that receiver or changing its stack. This distinguishes
direct shutdown from false success through later nested-delivery failure.
Replay the existing protected/alias/provider and modern real-stack-range
subgroups on SX while retaining 286 and old-frame behavior. Estimated tests
+140--190/-25--45, source +1--3/-1--3 if the reproducer confirms the source
predicate. Keep one `_e_push` owner, no new API/state or exception path.

### Actual Receiver Proof And Inventory Reconciliation

The new existing GPR-stack test first boots the protected GDT through guest
instructions, then supplies a valid VM86 SS gate, busy 32-bit TSS and ring-zero
stack. It covers eighteen explicit PUSH forms (eight GPRs, two immediates,
memory, four legacy segment registers, FS/GS and PUSHF), word/dword. A separate
POP crossing the real/VM86 stack limit proves that this same receiver actually
delivers SS rather than failing into shutdown. Both widths of that control run
for 386 and SX. For PUSH, unchanged CPU data and all forty receiver-stack bytes
must accompany direct shutdown; terminal nested-delivery failure cannot pass.

Before repair, x64 reports exactly 34 failing SX PUSH cases, all reaching
CS=8/EIP=0100 with SS instead of shutdown. The 386 controls, SX PUSHF and both
profiles' receiver controls pass. The log is
`build/t560-s8-stack-receiver-before.log`. The sole production change is the
existing explicit-PUSH family comparison `==80386` to `>=80386`; the >=286
PUSHF rule and CALL/ENTER/PUSHA owners are unchanged. This directly implements
the original PRM printed 26-237 real/VM86 SP=1 requirement, not a guest-BIOS
or board-specific workaround. An initial test compilation used the incorrect
private member spelling `seg.code`; it was corrected to existing `seg.exec`
before the failed behavioral reproduction, without a production layout change.

Existing GPR protected-failure and POP alias/provider matrices now select
both 386 and SX. The latter contributes seventy executions per profile across
real/protected/VM86, operand/stack widths, three aliases and each applicable
provider-failure point. Real stack-range proof retains 286's distinct GP rule
and adds SX's SS behavior. No old profile assertion is removed.

The inventory failure is separately reconciled by comparing every emitted
primary/escaped mask against the preserved T435 historical JSON, not just
changing totals. For both 186 and 386, the only primary-mask differences are
C0 and C1 extension 6: 32 ModR/M values per opcode, sixty-four in total. These
were preview-only candidates rejected by the old executor, as already proved
in this S8's Group-2 fail-first test. Current totals are therefore 61,466/247
and 62,957/253; no primary opcode disappears. The 386 escaped mask also removes
reserved CR indices from 0F20/22, matching the separately source-qualified
actual UD proofs above. There are no other mask differences. The runners
emit their actual inventory and diagnostic counts before testing the expected
current total, so future failures remain inspectable. Historical JSON and
its historical-denominator tools are not rewritten as current qualification.

The final focused build/tests pass 5/5 per width, 1.60/0.50 s, including both
registrations of each inventory owner and the stack test. Logs are
`build/t560-s8-{x64,x86}-stack-inventory-final-{build,test}.log`.
Both Core manifests verify after these changes. Cumulative S8 source is
+183/-87 (net +96); tests +5671/-1874 (net +3797), including the new integer
test, excluding manifests/docs/artifacts. No new production state/helper/API,
Shared-six, MyNES, input or released-artifact change is made here.

The first 103-case owner-complete result above did not rebuild the duplicated
inventory runners and cannot qualify their current metadata. The full build
exposed this omission. The subsequent focused build explicitly rebuilds all
five affected targets on both widths; full repository rebuild/unit execution
is now restarted once for the corrected stable source. Its result remains
separate from the focused pass and from outstanding C15 reconciliation,
specialized/integration and eight-artifact delivery.

### Corrected Full Unit Result And Remaining F03 Review

The corrected complete repository rebuild/unit executions are terminal and
pass 605/605 on x64 and 605/605 on x86, 258.33/81.26 s. The original graphs
and shared persistent ccache are reused, with -j8 build and the existing unit
aggregate. Logs are `build/t560-s8-{x64,x86}-repository-units-final.log`.
This replaces the earlier four-failure unit outcome for this exact snapshot;
it does not retrospectively change that failed run, qualify physical/RDP
interaction or qualify newly edited code in a later continuation. No unit is
skipped or disabled. No other process is still running for this aggregate.

The same snapshot passes both Core manifests, the 44-owner fixture lifecycle,
NXVM documentation governance and whitespace checks. Shared-six, MyNES and
released artifacts have no diff. The existing legacy Group-3 owner still
tests NOT/NEG only at 8/16 bits and a single nonzero value; the new multiply/
divide owner does not prove those unary forms. The actual RMW owner already
covers NOT/NEG byte/word/dword memory permission, PF/GP, cross-page and
provider failure behavior. Do not duplicate that failure mechanism or claim
that its success value proves zero/minimum-negative flags.

Before F03 acceptance, extend the existing arithmetic owner with grouped
NOT/NEG result/flag boundary proof, reusing its mask/parity/SUB flag oracle:
0, 1, minimum negative, adjacent sign boundary, all ones and alternating bits;
8/16/32 widths, memory/register, both incoming arithmetic-flag images and
legal code/address/mode combinations. Original PRM 26-220/222 defines NEG
as zero minus the operand and NOT as inversion with no flag changes. The
1992 NEG flag list omits AF; any explicit AF assertion must name the later
pinned instruction reference's 4-167 clarification as retrospective evidence,
not pretend it is printed in that original list. Estimated test-only change
+80--110/-0--5, no new fixture/framework/API or production repair assumed.
Focused rebuilding follows the added test; another whole run belongs only
after the remaining finite C15 reconciliation reaches a stable snapshot.

## Final F20 Escaped-Slot Reconciliation Plan

Re-reading the complete current dispatch assignments finds 256 primary slots
(including prefix/escape syntax) and 88 non-default escaped handlers. Two of
those, RDMSR/WRMSR, are deliberately unavailable to selected SX. The frozen
ordinary-mode map therefore leaves 170 escaped slots requiring UD, including
the already source-qualified unavailable ICE/LOADALL and A-step forms. The
remaining 86 escaped entries are reconciled by their individual family owners;
do not infer availability from a non-default handler alone.

The existing fault-event test has a real-IVT whole-CPU/frame helper, but only
a few reserved-slot witnesses. Extend that owner, approximately +40--65/-0
test lines, with explicit source-map reserved ranges rather than deriving the
oracle from production metadata. Sweep all 170 slots, both code defaults,
plain/66 forms and register-like/memory-like following bytes; require scanner
rejection and actual UD/frame/unchanged CPU data. Reuse the existing exception
entry and prefix owners for protected/VM86 and overlength behavior instead
of duplicating those mechanisms. This is no new production/API/state change.

The adjacent final group-form review will also qualify each reserved ModR/M
class directly, rather than infer it from LOCK-only tests: 8F/C6/C7, FE/FF,
invalid segment-register indices, 0F00/01/BA, and register encodings of
memory-only BOUND/LEA/LES/LDS/far transfers/LSS/LFS/LGS/table/INVLPG forms.
Existing C6/C7 and source instruction pages remain witnesses; a compact
source-map table at the same fault-event owner supplies all ModR/M and code/
66 combinations, scanner rejection and exact actual UD/frame preservation.
Approximately +55--80/-0 test lines, no new fixture or production assumption.

### Confirmed FE Reserved-Extension Operand Read

The new table fails first on FE /2, ModR/M 10, 32-bit code: its EAX-based
effective address is outside the legal fixture operand range and the old
handler reads it before rejecting the reserved extension, so the required UD
contract fails. That first log identifies the failing form, not a precise
first-fault vector; it must not be cited as a proved CE-versus-GP distinction. A full
table pass retaining one diagnostic per class confirms only FE fails among
the nineteen classes; do not broaden the fix to unrelated handlers.
Logs: `reserved-groups` and `reserved-groups-class`, x64. x86 is intentionally
not run after either failed snapshot.

Original PRM 9.9.6 (9-15/16) specifies invalid opcode/operand forms, and Table
9-2 (9-5) places instruction-decoding faults above memory-operand faults.
The complete relevant paragraphs/table have been read. `_d_modrm` decodes
the effective address but does not fetch the operand; FE then explicitly
calls `_m_read_rm` for extensions it will unconditionally reject. Remove
only that invalid read. Its duplicate >=386/else `_adv` branches are identical
and can collapse to one statement. Estimated production +1/-10, tests above
remain the actual reproducer. Valid INC/DEC still use their single existing
RMW read/write owner; no timing algorithm, API, helper or state is added.

## Final Unary And Reserved-Form Results

The existing arithmetic owner now executes 6,336 NOT/NEG boundary cases across
386/SX, real/protected CPL0--3/VM86, legal code defaults, address sizes,
byte/word/dword, register/memory and both input arithmetic-flag images. Six
width-relative inputs include zero, one, the minimum negative value, its
predecessor, all ones and alternating bits. It checks complete EFLAGS,
destination/upper-half preservation, unaffected registers and memory guards.
Its existing SUB flag oracle supplies NEG; NOT requires all flags unchanged.
AF is explicitly supported by the pinned later reference 4-167's legacy
operation, a retrospective clarification of the original list rather than
a measured SX assertion. Both initial owner tests pass, 0.69/0.67 s in
`build/t560-s8-{x64,x86}-unary-{build,test}.log`. No production unary repair
or second arithmetic oracle is introduced.

The fault-event owner now proves all 170 unadmitted escaped slots in 1,360
real-handler executions. Its nineteen explicit invalid ModR/M classes add
7,136 executions, with all reg/mod/rm values, both code defaults and plain/66
forms. The ranges/masks come from the source-qualified opcode map and form
restrictions, not from querying production metadata for expected results.
Each requires scanner rejection, actual UD, exact word frame and complete
CPU preservation apart from specified exception entry. Illegal memory forms
cannot hide behind a missing exception receiver or a successful stub.

FE's actual repair is +1/-10 production lines. All other eighteen invalid
classes pass before repair and are retained, not modified speculatively.
The existing INC/DEC, RMW permission/PF/provider, prefix/LOCK and required-
fetch owners retain valid and failure controls. The final six-owner cohort
passes 6/6 per width, 2.08/1.55 s, after rebuilding the real CPU library in
each original graph. Logs are `fe-reserved-final-{build,test}` with the usual
`build/t560-s8-{x64,x86}-` prefix. This current snapshot follows, and is not
retroactively qualified by, the earlier 605/605 whole-unit snapshot.

Actual continuation delta is source +1/-10 (net -9), tests +196/-1 (net +195).
Cumulative S8 is source +184/-97 (net +87), tests +5867/-1875 (net +3992),
including the new integer test, excluding manifests/docs/artifacts. The two
new private test functions own finite source-map negative proofs; the unary
function owns result/flag boundaries. They add no fixture framework or
production helper/state/API. Core manifests are regenerated; final full
qualification and release still belong to this S8, not to a partial P.

## C15 Final Functional Disposition Candidate

This is the final source/form reconciliation, not a silicon measurement or a
claim that tests enumerate every possible register/memory bit pattern. The
finite universe remains both dispatch tables and every applicable group/form
at the admitted baseline. The original 1992 PRM instruction entries, exception
chapter and compatibility/map qualifications above govern; explicitly cited
later/reference behavior stays L2. Earlier sections retain failed intermediate
results and do not override this final candidate.

Nonprivileged arithmetic does not acquire independent algorithms per CPL or
mode: the same `_a_*` helper consumes already decoded widths/values. Its value/
flag proofs compose with actual prefix/address, RMW and exception-entry proofs.
That is reuse of an unchanged mechanism, not a missing mode silently omitted.
Privileged, stack, transfer and mode-changing instructions additionally have
their direct context/receiver matrices. Undefined outputs are not upgraded to
exact expected values merely because this emulator returns a stable value.

| Family | Final direct proof and source-qualified disposition |
| --- | --- |
| F01 | Binary/accumulator/eight Group-1 operations, both carries and full 81/83 dword immediates use independent unsigned-wide result/flag oracles. Register/memory, widths, accepted 82 alias, upper halves and shared operand failures are exercised. Reserved/improper LOCK forms stay at the prefix/RMW owners. |
| F02 | DAA/DAS valid-decimal and all-input L2 model, exhaustive AAA/AAS AX carry/borrow, complete AAM/AAD radix/value cases and real divide-zero handlers. XLAT's address/segment/width and single-byte span come from the actual span owner, not a copied host lookup. Invalid decimal data retains its explicit model grade. |
| F03 | New NOT/NEG zero/sign/minimum/upper-half and complete FLAGS matrix plus 36 multiply/divide vectors, actual DE frames, signed minimum/-1 and provider-before-result proofs. IMUL immediate/two-operand aliases and source preservation remain at their direct owners. Undefined division flags/results are not asserted. |
| F04 | Register and FE/FF INC/DEC width/alias/CF preservation and arithmetic boundaries, plus RMW failure/publication. Reserved FE/FF fields now have full ModR/M UD/frame proof; the premature FE operand read is removed at that handler only. |
| F05 | All Group-2 operations, immediate/one/CL, zero/effective-zero, masked and oversized counts; SX /6 uses its source-qualified SHL alias. Legacy immediate /6 rejection and lexical consistency remain distinct. OF/other undefined ranges retain declared qualifications. |
| F06 | SHLD/SHRD left/right, immediate/CL, word/dword, memory/register and zero count preserve source and defined flags. Original 26-263--266 explicitly leaves count >= operand width undefined; capture assertions preserve that distinction, with RMW zero-count/access controls. |
| F07 | Bit-test/change signed memory indices, register modulo, all bit positions and scan aliases/zero input. Operand-span and RMW proofs retain address/permission/provider boundaries; undefined scan destination/flags are not converted to exact silicon requirements. |
| F08 | GPR/immediate/moffs/segment MOV, LEA, sign/zero extension and XCHG aliases/widths/upper halves, including all low/high byte registers. Real selector/load/cache and memory failures stay at their existing owners. Selected UMOV 0F10--13 is explicitly L2 using the existing MOV path, not a second memory executor. |
| F09 | Register/immediate/segment/FS/GS/rm stacks, PUSHA/POPA and ENTER/LEAVE cover operand/stack width, ESP alias, allocation/chain/accepted-write-prefix behavior. The new valid VM86 SS receiver proves explicit PUSH SP=1 direct shutdown rather than accidental nested-fault shutdown. Old wrap/frame rules remain. |
| F10 | CBW/CWDE/CWD/CDQ, all 256 SAHF/LAHF images, individual carry/direction flags, NOP and accumulator XCHG use their actual selected-SX owners, preserved nonparticipants and prefix/LOCK controls. |
| F11 | All sixteen Jcc/SETcc flag predicates and LOOP/JCXZ count/address domains, near direct/indirect CALL/JMP/RET and immediate cleanup. Actual current-reference GP, required-suffix/provider failure and deferred target-fetch boundaries are preserved. |
| F12 | Far direct/indirect transfers, calls/task gates, nested/outer returns and 16/32/cross-width TSS use selected-SX execution, actual descriptor/receiver state, saved frames and busy/access effects. Selector errors now mask RPL correctly; local DR7 clearing follows CPU family, including incoming word TSS. Deferred target fetch is not confused with entry failure. |
| F13 | Five string families cover zero count, both directions, width/address/repeat selection, committed iteration prefixes, IRQ/debug/SMI boundaries and exact guest page-repair/IRET continuation. CMPS/SCAS pre-instruction FLAGS fault restoration is explicitly the approved retrospective L2 contract; its one CPU-owned transient context has lifetime tests. |
| F14 | Scalar and string ports cover IOPL/TSS bitmap, source/destination ordering, successful port prefixes, denied accesses/provider failures and restart. The existing port fixture supplies real accepted-transfer accounting; no replayed prefix or product-specific I/O path is introduced. |
| F15 | BOUND signed range and exact BR frames; ARPL and far data loads; exhaustive LAR/LSL/VERR/VERW type/Present/CPL/RPL/DPL queries. Present does not invalidate a query, unsupported gate types do. Page/descriptor and invalid-form precedence remain separate from source-qualified undefined flag/upper-bit behavior. |
| F16 | Descriptor/task table reads/writes, word/dword pseudo-descriptor layout, LLDT/LTR null/privilege/busy behavior and exact exception frames. VM86 loads retain the explicitly qualified instruction-specific rejection rules. Shared task transition/publication proof is reused with actual selected-SX entry, not inferred from enumeration. |
| F17 | PUSHF/POPF, CLI/STI, INT/INTO/INT3/INT1, IRET and HLT retain direct real/protected/VM86/VME/IOPL tests, correct real frame width, state/cache restoration and privilege/restart behavior. INT1's non-DR6 trap source is distinct from ordinary step/hardware debug; S6/S7 qualified mechanisms remain unchanged. |
| F18 | CR/DR/TR, CLTS, cache/TLB/INVLPG, CPUID and SMM/RSM reuse accepted S2--S7 selected-model proof. S8 adds exact reserved CR and DR4/5 alias admission consistency; ordinary unavailable MSR/ICE/LOADALL forms cannot become available merely from a dispatch assignment. Cache/bus timing still belongs to S9. |
| F19 | Accepted T559/S3/S5 BSWAP/XADD/CMPXCHG code-default, aliases, memory/register, old-family rejection, source/result flags, actual permission/provider and LOCK proofs remain direct selected-SX tests. No corresponding production body changes in S8 invalidate that reuse. |
| F20 | All 170 unadmitted escaped slots and nineteen invalid ModR/M classes now have explicit source-map scanner and real UD/frame proof. Primary prefix/escape syntax, selected aliases, required-fetch failures, 15-byte limit, REP/LOCK rules and absent-FPU NM/WAIT use their direct owners. Hardware ICE and removed A-step encodings retain explicit non-applicability, not success stubs. |

The remaining 86 escaped entries and all primary data/control/group slots map
to these positive or contextual owners; the 170 negative slots complete the
256-entry escaped space. The sixteen primary groups and three escaped groups
are accounted for, including selected aliases rather than pretending those
aliases are architecturally reserved. The multiline profile comparison sweep
has explicit reset/FPU/identity/LSL timing dispositions; no mechanical equality
widening substitutes for that review.

The current complete build refresh and CPU/board cohort passes 103/103 per
width, 23.88/13.75 s, including the changed unary/reserved owners and both
registrations of the current inventories. Logs are
`build/t560-s8-{x64,x86}-final-form-owner-{build,test}.log`; the full build also
passes the global Types gate. These final functional dispositions are candidates
for whole/static/integration/release qualification and actual-diff review.
They are not accepted C15/S8 closure yet. C16 timing, C17 production admission
and fitted-board/PC110 qualification remain their original separate receivers.

## Delivery Qualification: Stale LSL Gate Receiver

The first x64 specialized delivery build exits 1 before running whole units;
x86 is not started by that fail-first pipeline. The actual failure is the
NXVM-only LSL checker looking for `lar_lsl_test_386_attributes_and_memory`
after that existing test was correctly parameterized and renamed to
`lar_lsl_test_attributes_and_memory`. Actual 386 granularity expectations,
both retained timing formulas and the board timing recipes remain unchanged.
The Deferred-ownership error later in the same log belongs to its deliberate
negative self-test, which reports success; it is not another failure.

Synchronize this single existing checker token (+1/-1), preserving every
other check and the original historical evidence. No Shared gate, timing
value, test assertion, public API or source behavior is changed. The active
packet explicitly includes this direct test-receiver correction. The default
x64 artifact is already rebuilt by that partial Ninja attempt, so it is an
intermediate S8 worktree EXE, not accepted S7 or completed S8 delivery.

### Final Whole And Specialized Qualification

After the one-token LSL receiver correction, both complete specialized
aggregates exit 0. Both final whole repository rebuild/unit aggregates also
exit 0: 605/605 per width, x64 251.65 s and x86 67.99 s. Their logs are
`build/t560-s8-{x64,x86}-specialized-delivery-final.log` and
`build/t560-s8-{x64,x86}-whole-delivery-final.log`. These runs include the
final FE production repair and unary/reserved-form tests; earlier 605-case
snapshots above are not substituted for them. Intentional negative checker
self-tests retain their own expected diagnostics and successful outcomes.

Production actual-diff review covers all three CPU files: selector-code
masking, invalid LDTR query guard, VM86 interrupt/task-cache entry, task DR7,
explicit-PUSH shutdown, metadata/scanner eligibility, selected decimal/ASCII
models, aliases, real trap frames, query types/Present, FE admission and REP
compare lifetime. The two new instruction handlers fit the original table
style; aliases reuse original handlers and arithmetic. Two private transient
fields have one semantic purpose and tested lifetime, independent of timing
bookkeeping. No public API, new executor, generic framework or Shared change
is introduced. The NXVM gate receiver is +1/-1 separately from production's
+184/-97. Final test growth supplies explicit missing contexts/failure proof;
it is not an additional product parser, machine model or reference CPU.

Profile release/integration is now running sequentially in those same two
graphs. Default runs all 35 registered integration rows once, including the
twelve unchanged MyNES rows. Other fixed profiles exclude only those same
MyNES rows and run their own complete PC registrations once (3 XT, 5 AT,
5 DeskPro). Thus the planned per-width total is 48 actual executions, not
84 created by rerunning MyNES under every PC selection. Default restoration
also rechecks the deployed process INI/identity. No integration result is
claimed until its process is terminal; artifacts remain intermediate until
all eight paths and required tests are verified.

## Complete Executor Delivery

All eight integration groups are terminal exit 0, with no skipped rows:

| Fixed profile | x64 | x86 |
| --- | --- | --- |
| default-pc-at-80386-1440k-hdd | 35/35, 60.69 s | 35/35, 71.40 s |
| ibm-5160-model-268-360k | 3/3, 30.27 s | 3/3, 41.00 s |
| ibm-5170-model-339-1200k | 5/5, 43.94 s | 5/5, 60.10 s |
| compaq-deskpro-386-model-40-1200k | 5/5, 130.37 s | 5/5, 183.09 s |

Each width executes 48 rows (36 PC and twelve MyNES), without rerunning
MyNES under every fixed PC selection. Both default-restored real-process
INI/identity checks pass 1/1, 0.65/0.61 s. Logs are
`build/t560-s8-<width>-<profile>-integration.log`, with `default` naming the
first group, and `build/t560-s8-<width>-restored-identity.log`. Both original
cached graphs finish configured for default. Profile build logs show zero
recompilations of `cpu_instructions.c`; already built CPU/library objects are
reused rather than copied into new graphs. The ignored local orchestration
script is not a new repository test framework or a committed requirement.

All eight deployed 0.5.0560 stripped Release EXEs pass the existing PE-width
verifier, both during deployment and in the final path/hash check:

| Artifact under assets/ | Bytes | SHA-256 |
| --- | --- | --- |
| my5160/my5160_0_5_0560_x64.exe | 1451113 | `F01B2A966588A2CC5FF76187AC8C311A812A69E1B9880421A3A163D13C3A8E01` |
| my5160/my5160_0_5_0560_x86.exe | 1593585 | `1031A6AF796A9C626D480ECE3618EF72C565ED3F7DC9A32C785EBA3B22007DDC` |
| my5170/my5170_0_5_0560_x64.exe | 1463989 | `A748EE32DD6AA23CC3F24814D2384FF9E007A22BBA6C71455680B38237BB9B5E` |
| my5170/my5170_0_5_0560_x86.exe | 1606840 | `DC448454E66454D392665F569E46D50C596A143B793AA54F57A49C8D01CCD621` |
| mydeskpro386/mydeskpro386_0_5_0560_x64.exe | 1450717 | `705A260602402BB423A162C99B63F31C2E90C207F5EBC779413E1A6A7DB2EA0F` |
| mydeskpro386/mydeskpro386_0_5_0560_x86.exe | 1593063 | `96551AA6181C8E95701570A8F87E1A91CC8995D4E7861102D83B859307D3130F` |
| nxvm/nxvm_0_5_0560_x64.exe | 1462647 | `D22CD7782A619E14F05CB15D3D3C27405CEE064A5269BBD17B2776ABC62CBCB9` |
| nxvm/nxvm_0_5_0560_x86.exe | 1605626 | `821BFD158D90491535F2FDA05DDF43FE8FCA02CE64595E29B0569A00084CCF47` |

Final serialization review removes an accidental manifest-order-only diff:
Core's historical hash-first entry ordering is preserved, with the same path/
hash set and S8 revision header. The actual manifest diffs are source +4/-4,
test +79/-78, not hundreds of unrelated reordered lines. Both manifests pass
again. The changed NXVM gate file is LF-normalized; its only semantic diff
remains the one renamed test token and its direct final check passes.
These serialization-only final checks do not manufacture another full dynamic
run. NXVM documentation governance, fixture lifecycle, Types and whitespace
checks pass. Both complete whole-unit logs contain no skipped/not-run cases.

The implementation comprises three production CPU files (+184/-97), 78
changed/new test/registration paths (+5867/-1875, including the new 192-line
integer test), one NXVM checker (+1/-1), the two Core manifests, NXVM evidence/
status/history/index and eight artifacts. No Shared-six, MyNES production/
tests/released binaries, INI, ROM, disk or snapshot diff is present. Existing
strict-compilation exceptions remain explicit: the matrix reports 510 strict
and three deferred rows, and the CPU retains its existing unused/empty-body
warning suppressions. S8 adds no suppression; it does not claim an entirely
exemption-free source tree.

The frozen C15 functional family/form candidate and all required automatic
S8 qualification are ready for the complete P and coordinator actual-change
review. Original/L2 distinctions, hardware ICE non-applicability, undefined
outputs and fitted-silicon limits above remain qualifications, not hidden
unimplemented successful stubs. C16/S9 timing and C17/S10 production admission
are still mandatory; this is not a complete 486 CPU/PC110 or full Windows 3.1
GUI qualification. Automated desktop-labelled tests pass as listed in the
whole/integration logs; no manual RDP, phone keyboard, desktop gameplay or
complete Windows GUI session has been performed.
