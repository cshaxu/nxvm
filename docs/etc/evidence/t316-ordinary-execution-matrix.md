# M5 T316 S1: Intel 80386 Ordinary Execution Matrix

## Scope, Authority, And Method

This is an inventory and admission record, not an 80386-completeness claim.
The behavioral authority is the Intel *80386 Programmer's Reference Manual*
(ordinary application instructions, EFLAGS, operand/address-size attributes,
string instructions, and control transfer). The retained
[T309 audit](t309-80386-form-audit.md) is baseline evidence; T310's focused
smokes supersede its former `0F`-integer *unproven* dispositions where listed
below. No external implementation source was imported or used as repository
evidence.

The review traced each group through the single primary decoder `ExecIns`, its
metadata/profile query `core_machine_cpu_instruction_metadata_get` /
`core_machine_cpu_profile_allows_form`, and the `INS_0F` secondary gate in
`src/core/machine/cpu_instructions.c`. Primary metadata defaults to an 8086
minimum profile unless an entry overrides it, so reachability is not proof of
80386 operand/address behavior. `INS_0F` repeats the profile check before its
table dispatch. `cpu_instructions.c` remains the only instruction executor;
the sweep found no second CPU executor.

Classification meaning:

- **Complete** means the stated bounded form set has an initialized route and
  focused proof. It does not certify forms outside that row.
- **Partial** means a route exists but a listed operand, address-size, flag,
  fault/publication, privilege, or breadth proof is absent.
- **Missing** means no 80386 route was found for the reviewed form.
- **Outside-80386** means the Intel form is post-80386 or reserved, and is
  rejected by the profile/metadata route rather than silently admitted.
- **External-coprocessor boundary** means the 80386-side interface is in the
  approved program while 8087/80287/80387 execution is expressly not.

## Form Matrix

Every row is an exhaustive named PRM-form grouping for this S1 ordinary
review. Next placement keeps every partial or missing group visible; it is
not an allocation of later task identifiers.

| PRM form group | Current code route and profile disposition | Focused evidence | Classification and next placement |
| --- | --- | --- | --- |
| Prefixes `26/2E/36/3E`, `64/65`, `66/67`, `F0`, `F2/F3`; repeated-prefix resolution | Prefix loop and `_GetOperandSize`/address decode in `cpu_instructions.c`; FS/GS and `66/67` explicitly gate at 80386. | `core_machine_real_mode_386_address_smoke`, `core_machine_operand_address_smoke` (`M5:T302:OPERAND-ADDRESS-STACK:OK`), T301 baseline. | **Partial**: FS/GS and operand/address prefix paths have bounded proof, but no cross-product proof exists for every primary opcode, LOCK legality, and repeated-prefix combination. Next: ordinary operand/address subfamily. |
| Register/memory/immediate data movement: `MOV`, `XCHG`, `LEA`, `LES/LDS`, moffs, `PUSH/POP`, `PUSHA/POPA`, `PUSH imm`, `ENTER/LEAVE`, `CBW/CWD` (including 32-bit attribute variants) | Primary table routes to `MOV_*`, `XCHG_*`, `LEA_R32_M32`, stack helpers `_kec_push/_kec_pop`, and `ENTER/LEAVE`; primary defaults do not by themselves certify 32-bit forms. | T302 stack/address focused probe; T301 prefix/profile baseline. | **Partial**: direct route and selected 16/32 stack/address cases are proven, but no complete MOV/XCHG/LEA/moffs/segment-load matrix covers all register/memory, size, and fault-publication forms. Next: ordinary data/operand family. |
| Primary binary arithmetic/logical/test: `ADD/OR/ADC/SBB/AND/SUB/XOR/CMP/TEST`, accumulator immediates, Groups `80/81/83` | Primary table and `INS_80/81/83`; `_kac_arith2` uses operand read/write and `_kaf_set_flags`. | `core_machine_inc_dec_smoke` (`M5:T316:S4:TEST:OK`, `M5:T316:S7:TEST-RM-REG:OK`, `M5:T316:S8:ADD:OK`, `M5:T316:S9:ADC:OK`, `M5:T316:S10:SBB:OK`, `M5:T316:S11:OR:OK`, `M5:T316:S12:AND:OK`, `M5:T316:S13:SUB:OK`) proves the declared slices. | **Partial**: T316 S4/S7 complete declared TEST; S8 `/0` ADD; S9 `/2` ADC; S10 `/3` SBB; S11 `/1` OR; S12 `/4` AND; and S13 `28`--`2D` plus `/5` SUB. Only XOR and CMP remain unproven in this row. Next: ordinary arithmetic/FLAGS family. |
| Primary unary arithmetic: `INC/DEC` register (`40`--`4F`) and Groups `FE/FF /0,/1`; `NEG/NOT` Groups `F6/F7 /2,/3` | Register handlers `INC_*`/`DEC_*`; `INS_FE/FF`, `INS_F6/F7`; all use `_kac_arith1` and flag masks. | `core_machine_inc_dec_smoke` (`M5:T316:S2:INC-DEC:OK`, `M5:T316:S3:NOT-NEG:OK`) proves both admitted slices. | **Complete** only for T316's named INC/DEC and NOT/NEG forms: 16/32-bit register and 8/16/32-bit r/m forms, their Intel FLAGS contracts, 16/32 operand/address attributes, profile behavior, publication, and protected fault non-publication. The wider unary/arithmetic family remains **Partial**: `F6/F7 /6,/7` division is a separately named slice. |
| Decimal/ASCII adjust and conversion: `DAA/DAS/AAA/AAS/AAM/AAD`, `XLAT` | Primary handlers `DAA`, `DAS`, `AAA`, `AAS`, `AAM`, `AAD`, `XLAT`; DAA/DAS/AAM/AAD call `_kaf_set_flags`. | No focused architecture probe found. | **Partial**: handlers exist, but documented flag-defined/undefined behavior, base-immediate edge cases, 16/32 addressing for XLAT, and faults are not proven. Next: ordinary arithmetic/FLAGS family. |
| Shift/rotate Groups `C0/C1/D0`--`D3` | `INS_C0/C1/D0/D1/D2/D3`; shift paths call `_kaf_set_flags`. | No focused primary shift/rotate smoke; T310's double-shift smoke is only `SHLD/SHRD`. | **Partial**: primary count masking, count-zero flags, rotate flags, and memory faults remain unproven. Next: ordinary arithmetic/FLAGS family. |
| One-operand multiply/divide Groups `F6/F7 /4`--`/7` | `INS_F6/F7` dispatches `/4` to `_a_mul`, `/5` to `_a_imul`, `/6` to `_a_div`, and `/7` to `_a_idiv`. | `core_machine_inc_dec_smoke` (`M5:T316:S5:MUL-IMUL:OK`, `M5:T316:S6:DIV-IDIV:OK`) covers `/4`--`/7`. | **Complete** only for T316's declared one-operand Group `F6/F7 /4`--`/7` forms at 8/16/32 bits. This does not claim the wider multiply/divide or ordinary arithmetic family complete. |
| EFLAGS transfers and direct flag control: `PUSHF/POPF`, `LAHF/SAHF`, `CMC/CLC/STC/CLI/STI/CLD/STD` | `PUSHF`, `POPF`, `SAHF`, `LAHF`, and primary flag handlers; protected-mode masking is local to those routes. | T302 checks selected PUSHF/POPF; retained string/product tests incidentally use DF/IF paths. | **Partial**: no focused table covers all modifiable/reserved flag bits, CPL/IOPL rules, interrupt shadow, and each direct flag form. Next: ordinary FLAGS/control family. |
| Memory strings `MOVS/CMPS/STOS/LODS/SCAS`, REP/REPE/REPNE, DF, 16/32 operand/address attributes | `MOVSB/W`, `CMPSB/W`, `STOSB/W`, `LODSB/W`, `SCASB/W`, `_kas_move_index`; primary REP loop. | `core_machine_real_mode_386_rep_cmps_smoke` (`M5:T292:S1:REP-STRING:OK`) and T302 memory-string checks. | **Partial**: bounded REP CMPS, index/size, and selected string paths pass; every string opcode, DF direction, segment override, limit/fault restart, and full 32-bit matrix is not proven. Next: ordinary strings/control family. |
| Port strings `INS/OUTS` with REP and size/address attributes | `INSB/INSW/OUTSB/OUTSW` and `_kas_move_index`. | T302 I/O-string portion. | **Partial**: selected I/O-string route is proven; all port-width, REP, protection, and fault/restart behavior is not. Next: ordinary strings/control family. |
| Short/near conditional control: `Jcc`, `LOOP/LOOPE/LOOPNE`, `JCXZ/JECXZ`, near `CALL/JMP/RET` | Primary `J*_REL8`, `LOOP*`, `JCXZ_REL8`, `CALL_REL32`, `JMP_REL*`, return helpers; `0F 80`--`8F` repeats the profile gate. | `core_machine_control_transfer_smoke` (`M5:T303:CONTROL-TRANSFER:OK`). | **Complete** for T303's declared real/protected 16/32 near, condition, target-limit, and atomic-fault matrix. Other control forms remain separately listed. |
| Far direct/indirect `CALL/JMP`, same-CPL `RETF`; outer returns, gates, task/V86 interactions | `CALL_PTR16_32`, `JMP_PTR16_32`, `_kec_call_far/_kec_jmp_far/_kec_ret_far`, IRET/protection routes. | T303 proves admitted real/protected far and same-CPL return cases; T305--T308 prove bounded privilege paths. | **Partial** for the ordinary-family inventory: outer-return, task/gate, V86, and broader exception interaction belong to the Queue control/protection/exception families. |
| 80386 `0F 90`--`9F` SETcc | `INS_0F` then `SETO_RM8`--`SETG_RM8`; metadata requires 80386. | `core_machine_setcc_smoke` (`M5:T310:S3:SETCC:OK`) covers conditions, register/memory, prefixes, and pre-fault non-publication. | **Complete** for the declared T310 SETcc matrix. |
| 80386 `0F A3/AB/B3/BB`, `0F BA /4`--`/7` BT/BTS/BTR/BTC | `BT_*`, `BTS_*`, `BTR_*`, `BTC_*`, `INS_0F_BA`; metadata/profile gate. | `core_machine_bit_test_smoke` (`M5:T310:S5:BIT:OK`) covers register/memory, immediate/indexed, 16/32, address prefix, rejection, and access-failure publication. | **Complete** for the declared T310 bit-test matrix. |
| 80386 `0F A4/A5/AC/AD` SHLD/SHRD | `SHLD_*`/`SHRD_*`, `_kaf_set_flags`, 80386 metadata gate. | `core_machine_double_shift_smoke` (`M5:T310:S6:DOUBLE-SHIFT:OK`) covers forms, count zero, profile rejection, and access failure. | **Complete** for the declared T310 double-shift matrix. |
| 80386 `0F AF` two-operand IMUL; `0F BC/BD` BSF/BSR; `0F B6/B7/BE/BF` MOVZX/MOVSX | `IMUL_R32_RM32`, `BSF/BSR`, `MOVZX/MOVSX` through `INS_0F` and metadata gate. | `core_machine_imul2_smoke` (`M5:T310:S8:IMUL2:OK`), `core_machine_bit_scan_smoke` (`M5:T310:S7:BIT-SCAN:OK`), `core_machine_movx_smoke` (`M5:T310:S4:MOVX:OK`). | **Complete** for each declared T310 form matrix, including profile rejection and named memory/fault boundaries. |
| 80386 `0F A0/A1/A8/A9` FS/GS push/pop and `0F B2/B4/B5` LSS/LFS/LGS | `INS_0F` table and segment-load routes, with profile checks. | T301/T302 prefix/segment baseline. | **Partial**: the bounded segment-load matrix is retained from T301; these exact 0F forms lack a dedicated current focused proof in this ordinary audit. Next: ordinary data/operand family, then protection family if privilege semantics expand. |
| Post-80386 or reserved encodings seen in the tables: `CPUID`, `RSM`, `WBINVD`, `RDMSR/WRMSR`, `CMPXCHG`, `XADD`, `BSWAP`, undefined holes | Metadata/profile gate rejects forms above the active 80386 profile before the named table handler can establish behavior. | `cpu_profile_gate_smoke` and T309 rejection baseline. | **Outside-80386**: retain rejection; no later-IA-32 implementation is admitted by T316. |
| `WAIT/FWAIT`, ESC `D8`--`DF`, CR0 `MP/EM/TS`, `#NM`, external coprocessor fault interface | `WAIT`, FPU escape/profile routes and CPU exception state; optional FPU provider is outside ordinary decoding. | `cpu_fpu_profile_smoke`, `cpu_fpu_profile_closure_smoke`, `fpu_escape_smoke`, `core_machine_fpu_8087_smoke`. | **External-coprocessor boundary**: only 80386-side control/reporting is in the approved program; no 8087/80287/80387 arithmetic, state, or completeness claim is made. |

No reviewed ordinary primary form is classified **Missing**: each named ordinary
group has a primary/`0F` route or the explicit external-coprocessor boundary.
This is not a claim that all routed forms are correct; the partial rows are
the visible remaining work.

## Helper Caller And Test-Coverage Inventory

The candidate S2 does not introduce or change an abstraction. This inventory
records the caller discipline required before any future shared-helper change.

| Candidate helper | Current callers / responsibility | Current focused coverage | S2 consequence |
| --- | --- | --- | --- |
| `_kac_arith1` macro | Four operation families only: `INC`, `DEC`, `NOT`, `NEG` (lines 5321, 5331, 5341, 5352). | T316 S2 covers INC/DEC; T316 S3 covers NOT/NEG. Incidental product bytecode remains excluded. | All current callers now have focused coverage. Neither S2 nor S3 changes this macro; any shared-helper correction still requires a separately admitted caller-impact review. |
| `_kaf_set_flags` | Called by arithmetic macros, primary shifts, string compares, DAA/DAS/AAM/AAD, and T310 SHLD/SHRD; it owns mask-driven CF/PF/AF/ZF/SF/OF publication. | T316 S2--S4 and S7 cover the named INC/DEC, NOT/NEG, and TEST callers; T310 covers SHLD/SHRD and selected string/data paths. Broader primary flag breadth is incomplete. | Do not change it in T316. Each admitted slice asserts its handler-bound flag contract; any shared-helper correction still requires caller-impact review. |
| `_a_mul` / `_a_imul` / `_a_div` / `_a_idiv` | Only `INS_F6 /4,/5,/6,/7` and `INS_F7 /4,/5,/6,/7`; they publish implicit multiply/divide result registers locally. | T316 S5 covers `/4,/5`; T316 S6 covers `/6,/7` at every 8/16/32 register and memory route, including source-fault and `#DE` non-publication. | S5 retains the demonstrated `_a_imul` signed-widening correction. S6 corrects only `_a_idiv`'s local signed quotient range check; no shared flag helper changes. |
| `_kas_move_index` | MOVS, CMPS, STOS, LODS, SCAS, INS, and OUTS size/index combinations. | T292 and T302 cover selected REP/string and I/O-string cases. | Not in S2; strings remain their own matrix slice. |
| `_kec_push/_kec_pop` | Near/far calls/returns, interrupt/IRET frames, PUSH/POP and protected transfer helpers. | T302 stack cases; T303 control transfer; T305--T308 delivery/return matrices. | Not in S2; a helper change would cross ordinary and delivery families. |

## Selected S2 Slice - Primary `INC/DEC` Flags And Operand Forms

Exactly one independently correctable S2 slice is selected:

| Field | S2 boundary |
| --- | --- |
| Forms | `40`--`4F` register INC/DEC; `FE /0,/1` byte r/m INC/DEC; `FF /0,/1` word/dword r/m INC/DEC, including 16/32 operand-size and register/memory forms. |
| Existing gap | Routes exist through `INC_*`, `DEC_*`, `INS_FE`, `INS_FF`, and `_kac_arith1`, but no focused proof establishes result, preserved CF, defined OF/SF/ZF/AF/PF, memory publication, profile/attribute behavior, or fault non-publication. |
| Proposed source/test scope | Keep ownership in `src/core/machine/cpu_instructions.c`; add one focused CPU smoke using the existing CPU fixture. Correct only a demonstrated INC/DEC defect. No helper extraction or `_kaf_set_flags`/`_kac_arith1` refactor is proposed. |
| Verification | Prepared-state 16/32-bit register and 8/16/32-bit r/m vectors for results and edge flags; explicit CF preservation; 16/32 operand/address cases; legacy `FF /0` acceptance and `66h` rejection below the 80386 profile; memory read/write failure and protected-limit non-publication; retained `core_machine_operand_address_smoke`, `core_machine_control_transfer_smoke`, and current gates. |
| Non-goals | `NEG/NOT`, binary arithmetic, BCD/ASCII adjust, rotate/shift, strings, control transfer, decoder ownership, external FPU behavior, CMake graph refactors, public ABI, and artifact/preset changes. |
| Similar-issue scan | Before S2 admission, scan all `_kac_arith1` callers (INC/DEC/NOT/NEG), `INS_FE/FF/F6/F7`, direct EFLAGS writes, and existing flag/operand tests. Any discovered NOT/NEG or shared-helper defect remains a visible separate matrix item unless the coordinator expands the slice. |

## S1 Completion Evidence

- Primary/`0F` dispatch, metadata/profile gates, helper callers, focused tests,
  T309, the Queue, and the 80386DX program were reviewed.
- Every reviewed group above has a source route, test disposition, one of the
  required classifications, and a next placement where incomplete.
- The selected S2 slice has no proposed abstraction and therefore does not
  cross the caller-coverage stop condition.

`M5:T316:S1:80386-ORDINARY-MATRIX:OK`

## S2 INC/DEC Closure Evidence

`core_machine_inc_dec_smoke` exercises all eight `40h`--`47h` INC and
`48h`--`4Fh` DEC direct-register handlers with both 16-bit default and `66h`
32-bit operands. It covers `FE /0,/1` byte and `FF /0,/1` word/dword register
and memory forms, including `67h` plus `66h` 32-bit effective-address access.
The vectors assert defined OF/SF/ZF/AF/PF outcomes and preserved CF, with
16-bit high-half preservation where applicable. It proves pre-80386 acceptance
of the legacy form and `66h` rejection below the 80386 profile.

The initial focused-test construction had three corrected test-only defects:
a two-byte `66h` direct-register encoding was written into a one-byte buffer;
the byte r/m vectors were initialized/read as words; and a provider-write
failure in real mode lacked an architecturally mapped delivery route. The final
fault vectors use the established protected-mode bounded/read-only-memory
fixture from the bit-test pattern. They obtain the existing deliverable `#DF`
diagnostic and prove destination memory, EFLAGS, and EIP are not published on
both source-limit and destination-write failure. No CPU handler, shared helper,
decoder, or ABI change was required.

The S2 sweep finds all `INC_*`/`DEC_*` primary handlers, `INS_FE`, and
`INS_FF` covered by this focused smoke. `_kac_arith1` also serves `NOT` and
`NEG`; those forms are intentionally unchanged and remain the matrix's named
later unary-arithmetic slice.

`M5:T316:S2:INC-DEC:OK`

## S3 NOT/NEG Closure Evidence

The same owner-bound `core_machine_inc_dec_smoke` now covers `F6 /2,/3` and
`F7 /2,/3` in 8/16/32-bit register and memory forms. NOT vectors require the
complete initialized FLAGS value to remain unchanged. NEG vectors use the
signed-minimum operand at each width to prove result, CF, OF, SF, PF, ZF, and
AF; they also prove register and memory publication. The focused 32-bit
operand/address vector uses `67h 66h F7 /3` and the 80186 `F7 /2` route; a
`66h F7 /2` vector rejects with `#UD` below the 80386 profile.

Both NOT and NEG use the established protected-mode bounded/read-only-memory
fixture for source-limit and destination-write faults. Each case proves the
destination, EFLAGS, and EIP remain unpublished and observes the existing
deliverable `#DF` diagnostic. No CPU handler, decoder, shared arithmetic
helper, CMake graph, ABI, artifact target, or preset changed.

The focused test initially omitted the precondition for its `67h 66h F7 1Eh`
vector: at 32-bit address size, r/m `6` names `[ESI]`, not an absolute
displacement. Initializing ESI to the governed test address corrected that
test-only encoding/precondition error; it did not expose a runtime defect.

S3's sweep finds `_kac_arith1` has exactly INC, DEC, NOT, and NEG callers, all
now covered by the T316 focused smoke. `INS_F6` and `INS_F7` also contain
`/0,/1` TEST and `/4`--`/7` multiply/divide routes; those production paths are
outside S3 and remain named partial matrix slices. The F6/F7 primary-table
registrations are covered by this smoke's `/2,/3` executions.

`M5:T316:S3:NOT-NEG:OK`

## S4 TEST Closure Evidence

`core_machine_inc_dec_smoke` covers accumulator-immediate `A8h`/`A9h` and
Group `F6/F7 /0` immediate r/m TEST forms at 8/16/32 bits, including register
and memory r/m operands. Every vector asserts the Intel-defined CF, OF, SF,
ZF, and PF results while deliberately excluding undefined AF. It also proves
that TEST leaves the accumulator/register and memory destination unchanged.

The focused 32-bit memory vector uses `67h 66h F7 /0` with the established
ESI address precondition. An 80186 `A9h` vector remains accepted, while a
`66h A9h` vector rejects with `#UD` under the 80286 profile without changing
EAX, EFLAGS, or EIP. The protected bounded-memory fixture exercises an F7 /0
source-limit fault and proves source memory, EFLAGS, and EIP are unpublished
at the existing deliverable `#DF` boundary.

The only focused-test correction was width-local: reading an unchanged byte
memory operand yields `0x000000ff` into the 32-bit test variable, so the
assertion now compares the width-masked value. No runtime behavior, decoder,
shared flag helper, CMake graph, ABI, artifact target, or preset changed.

S4's `TEST_|INS_F6|INS_F7|_kaf_set_flags` sweep covers the accumulator TEST
handlers, F6/F7 `/0`, and their `_a_test`/flag-helper route. F6/F7 `/1` is
the explicit undefined-opcode path; `/2,/3` are T316 S3 NOT/NEG; `/4`--`/7`
remain the separately named multiply/divide/shift matrix slice. Other
`_kaf_set_flags` callers (binary arithmetic, shifts, strings, adjust, and
T310 double shifts) are outside S4 and remain classified in their own rows.

`M5:T316:S4:TEST:OK`

## S5 One-Operand MUL/IMUL Closure Evidence

The Intel one-operand forms are `F6 /4,/5` for byte sources and `F7 /4,/5`
for word/dword sources. MUL publishes its full unsigned product in AX,
DX:AX, or EDX:EAX; IMUL publishes the corresponding signed full product. For
both, CF and OF are clear only when the high half is zero (MUL) or a sign
extension of the low half (IMUL). SF, ZF, AF, and PF are undefined and are not
asserted by the focused smoke.

`core_machine_inc_dec_smoke` covers every 8/16/32-bit register and memory
source form. The register vectors use non-alias CL/CX/ECX r/m sources, so the
generic ModRM register read is independently proven from the implicit
AL/AX/EAX multiplier. They prove non-overflow CF/OF clear; memory vectors use
boundary operands to prove full implicit results and CF/OF set. It also covers
`67h 66h F7 /5`, an 80186 F7 /4 acceptance case, and `66h F7 /4`
rejection with `#UD` below the 80386 profile. Both F7 /4 and /5 execute the
protected bounded-memory source-fault path and prove AX/DX, EAX/EDX, EFLAGS,
EIP, and source memory are not published.

The S5 reproducer found a real local 32-bit IMUL defect: `_a_imul` multiplied
two signed 32-bit values before widening, so `0x80000000 * 2` wrapped to zero
and incorrectly cleared CF/OF. The repair widens both operands to signed
64-bit before multiplication. No decoder, ABI, shared flag helper, or
unrelated multiply/divide route changed.

S5's `_a_mul|_a_imul|INS_F6|INS_F7|MUL_|IMUL_|EFLAGS_(CF|OF)` sweep finds
the declared `/4,/5` routes covered. `F6/F7 /0` is T316 S4 TEST, `/1` is the
explicit undefined-opcode route, `/2,/3` are T316 S3 NOT/NEG, and `/6,/7`
remain the separately named DIV/IDIV slice. The two-operand `0F AF`, `69`,
and `6B` IMUL routes remain T310 evidence and are outside S5.

S5 established the continuing T316 developer artifact `vm-0-5-0316` at
`build/output/nxvm_0_5_0316.exe`. Its accepted source commit is
`7935bad23704b431893becef11757daee051bd8f`; the verified worktree was based
on admission `bff7908092776e5cb16fbb84fb6428803d4d3989`.

`M5:T316:S5:MUL-IMUL:OK`

## S6 One-Operand DIV/IDIV Closure Evidence

Intel one-operand DIV/IDIV uses the implicit dividend and result pairs: byte
`AX` to quotient `AL` and remainder `AH`; word `DX:AX` to `AX`/`DX`; and
dword `EDX:EAX` to `EAX`/`EDX`. `F6 /6,/7` supplies the byte source and
`F7 /6,/7` supplies the word/dword source. All arithmetic FLAGS are undefined;
the smoke deliberately makes no FLAGS-result assertion for successful division.

`core_machine_inc_dec_smoke` exercises every declared 8/16/32-bit DIV and
IDIV register and memory source. Register vectors use non-alias
`CL`/`CX`/`ECX` sources, independently proving generic ModRM register reads
from the implicit dividend registers. The signed vectors prove negative
quotient/remainder cases; the unsigned vectors prove quotient and remainder
publication. The memory operand is reread unchanged after execution. `67h 66h
F7 /7` proves 32-bit address and operand attributes, an 80186 `F7 /6` form is
accepted, and `66h F7 /6` is rejected with `#UD` under the 80286 profile with
registers, EFLAGS, and EIP unchanged.

For every register form at 8/16/32 bits, divide-by-zero and quotient-overflow
vectors reach the available real-mode `#DE` diagnostic and prove EAX, EDX,
ECX, EFLAGS, and EIP are not published. The protected bounded-memory fixture
executes both `F7 /6` and `/7` with an out-of-limit source, observes the
existing deliverable `#DF` boundary, and proves implicit result registers,
EFLAGS, EIP, and the source memory remain unchanged.

The focused negative-quotient vectors found one local runtime defect in
`_a_idiv`: it masked a signed quotient to an unsigned width before its range
test, causing representable negative results to report `#DE`. The correction
keeps the quotient signed until the explicit signed range check and adds the
host signed-minimum divided by `-1` precheck before C division. `_a_div`, the
decoder, shared helpers, exception-delivery architecture, ABI, and FPU routes
were not changed.

S6's `_a_div|_a_idiv|INS_F6|INS_F7|DIV_|IDIV_|EXCEPT_DE` sweep finds only the
declared `/6,/7` production routes plus the shared Group F6/F7 dispatch. `/0`
is T316 S4 TEST, `/1` is the explicit undefined-opcode path, `/2,/3` are
T316 S3 NOT/NEG, and `/4,/5` are T316 S5 MUL/IMUL. No other production DIV or
IDIV implementation or caller was found. The two-operand IMUL routes remain
T310 evidence and are outside S6.

T316's current developer artifact remains `vm-0-5-0316` at
`build/output/nxvm_0_5_0316.exe`, SHA-256
`5D3405AE5D6C284074BB51EDD4E255A5C3C77F2F030AFFC5A7D3921B42540DD1`.
Its S6 accepted source commit is
`bcfc4fe17cab21a11a5ef896748c3629b2a60e02`; the verified worktree was based
on admission `8485f4ad`.

`M5:T316:S6:DIV-IDIV:OK`

## S7 Primary TEST r/m,reg Closure Evidence

Intel primary `84h` is `TEST r/m8,r8`; `85h` is `TEST r/m16,r16` or, with
the 80386 operand-size attribute, `TEST r/m32,r32`. TEST reads both operands
without publishing either. It clears CF and OF and defines SF, ZF, and PF from
the bitwise-AND result; AF is undefined and is deliberately not asserted.

`core_machine_inc_dec_smoke` covers all declared 8/16/32-bit forms with
non-alias register operands `CL/CX/ECX` and `DL/DX/EDX`, plus direct-memory
r/m destinations and the same source register. Two vectors per form prove the
SF and PF case and the ZF/PF case, with CF/OF clear. Each verifies that both
register operands remain unchanged and, for memory forms, rereads unchanged
memory. `67h 66h 85h` uses `[ESI]` to prove 32-bit address and operand
attributes. An 80186 `85h` form is accepted; `66h 85h` rejects under the 80286
profile with registers, EFLAGS, and EIP unchanged.

The protected bounded-memory fixture executes 84h, 85h, and `66h 85h` with an
out-of-limit r/m source. It observes the existing deliverable `#DF` diagnostic
boundary and proves source memory, the register source, EFLAGS, and EIP are
not published. The initial 16/32-bit SF test expected PF clear for a high-bit
result; its low byte is zero and therefore has even parity. Correcting that
focused-test expectation exposed no runtime defect.

The S7 sweep finds `TEST_RM8_R8` and `TEST_RM32_R32` as the sole primary
84h/85h handlers; both feed `_a_test`. `_a_test` is also called by T316 S4's
accumulator and immediate Group F6/F7 TEST forms. `_kaf_set_flags` has the
reviewed arithmetic, shift, string, adjust, and double-shift callers recorded
above; none changed. No runtime source, decoder, ABI, shared helper, CMake,
or artifact changed, so the existing T316 0316 artifact is retained without a
rebuild.

`M5:T316:S7:TEST-RM-REG:OK`

## S8 Primary ADD Closure Evidence

The declared Intel ADD forms are `00h`--`03h` r/m,reg and reg,r/m at byte and
word/dword widths, `04h/05h` accumulator immediates, and Group
`80h/81h/83h /0`. `83h` sign-extends its byte immediate to the active 16- or
32-bit operand size. ADD publishes its destination and defines CF, OF, SF,
ZF, AF, and PF.

`core_machine_inc_dec_smoke` covers 8/16/32 non-alias register and direct
memory destinations in both r/m,reg and reg,r/m directions, accumulator
immediates, and every Group `/0` immediate form including memory destinations.
Wraparound vectors assert CF/ZF/AF/PF and clear OF/SF; bounded 8/16/32
accumulator `signed-max + 1` vectors independently assert OF/SF/AF and the
width-correct PF result with CF/ZF clear. `83h` negative-immediate vectors
prove sign extension with SF/PF and the remaining defined flags clear.
The smoke checks destination publication and preservation of the non-destination
register or memory source. `67h 66h 01h` proves 32-bit address/operand
attributes; legacy word ADD is accepted on 80186 and `66h` rejects on 80286
without publishing registers, EFLAGS, or EIP.

The protected fixture covers both an out-of-limit r/m source and an in-limit
read-only r/m destination for `01h`, observing the existing deliverable `#DF`
boundary and proving destination memory, the register source, EFLAGS, and EIP
are unpublished. Initial test work corrected two fixture defects: it had
classified default `83h` as 32-bit rather than 16-bit, and configured the
destination-write vector as writable. Neither exposed a runtime defect.

The S8 sweep finds the six `00h`--`05h` handlers and Group `80/81/83 /0` all
feed `_a_add`; `_kac_arith2` also serves ADC, logical operations, SUB/SBB, CMP,
and TEST, while `_kaf_set_flags` has the broader callers recorded above. Neither
shared helper changed. FPU `FADD` is the retained external-coprocessor boundary
and outside S8. No runtime source, decoder, ABI, CMake, or artifact changed;
the existing T316 0316 artifact is retained without rebuild.

`M5:T316:S8:ADD:OK`

## S9 Primary ADC Closure Evidence

`10h`--`15h` and Group `80h/81h/83h /2` route through `_a_adc`; shared
`_kac_arith2` and `_kaf_set_flags` callers were audited and unchanged. The
owner-bound smoke covers 8/16/32 non-alias register/memory directions,
accumulator/group immediates, CF carry-in, 83h sign extension, 66/67,
profiles, and protected source-fault non-publication. Carry-in vectors prove
the exact CF/ZF/AF/PF result with OF/SF clear. No runtime defect was found.

S9 additionally proves carry-in-dependent signed overflow at 8/16/32 bits:
accumulator `signed-max + 0 + CF` yields signed-min, with OF/SF/AF set,
CF/ZF clear, and width-correct PF.

`M5:T316:S9:ADC:OK`

## S10 Primary SBB Closure Evidence

`18h`--`1Dh` and Group `80h/81h/83h /3` route through `_a_sbb`; its shared
`_kac_arith2` and `_kaf_set_flags` callers were audited and unchanged. The
owner-bound smoke covers 8/16/32 non-alias register/memory r/m and reg/rm,
accumulator and Group immediates, CF borrow-in, and `83h` sign-extension.
Borrow-in vectors assert the all-defined CF/OF/SF/ZF/AF/PF contract and
destination publication; the register-destination/memory-source form confirms
the source memory remains unchanged. The initial assertion mistakenly expected
that read-only source memory to publish the result; correcting that test-only
expectation exposed no SBB runtime defect.

Dedicated Group-immediate vectors use `80h /3, FFh` at 8 bits and `83h /3,
FFh` at 16 and 32 bits. They prove the negative `imm8` interpretation by
requiring the r/m destination `0 - (-1)` to become one, with exact CF and AF
set and the remaining defined flags clear.

Each non-alias register-source form also explicitly confirms that its source
register is unchanged at the exercised width; memory-source forms reread the
unchanged source memory.

S10 additionally proves `67h 66h 19h`, 80186 legacy acceptance, and 80286
`66h` #UD non-publication. Protected source-limit and read-only-destination
faults retain memory, register source, EFLAGS, and EIP at the existing `#DF`
boundary. CF-participating signed-minimum subtraction at 8/16/32 bits proves
the exact result and CF/OF/SF/ZF/AF/PF contract, including width-correct PF.

`M5:T316:S10:SBB:OK`

## S11 Primary OR Closure Evidence

Intel 80386 primary OR forms `08h`--`0Dh` route through `OR_RM8_R8`,
`OR_RM32_R32`, `OR_R8_RM8`, `OR_R32_RM32`, `OR_AL_I8`, and `OR_EAX_I32`.
Groups `80h/81h/83h /1` dispatch through `INS_80`, `INS_81`, and `INS_83` to
`_a_or`. `_a_or`, `_kac_arith2`, and `_kaf_set_flags` were audited; no shared
change was needed. The `_kac_arith2` caller set remains ADD, ADC, SBB, AND,
OR, SUB, XOR, CMP, and TEST; it was not refactored.

The owner-bound smoke covers 8/16/32 non-alias register and memory r/m+reg
and reg+r/m forms, accumulator and Group immediate forms, and `83h /1, FFh`
at 16/32 bits to prove negative-imm8 sign extension. It proves destination
publication, source preservation, CF/OF clear, and exact SF/ZF/PF results.
AF is explicitly excluded from assertions because Intel defines it as
undefined for OR. `67h 66h 09h`, 80186 acceptance, and 80286 `66h #UD`
non-publication are covered. The established protected fixture proves both
source-limit and read-only-destination faults retain destination, source,
EFLAGS, and EIP at the existing `#DF` boundary. Two initial failures were
test-only expectation defects: PF is based on the low byte, and byte/word
memory observations require width-local comparison. No runtime defect was
found.

`M5:T316:S11:OR:OK`

## S12 Primary AND Closure Evidence

`20h`--`25h` route through the six `AND_*` handlers; Group `80h/81h/83h /4`
routes through `INS_80/81/83` to `_a_and`. `_a_and`, `_kac_arith2`, and
`_kaf_set_flags` were audited; no shared change was needed. The focused smoke
proves 8/16/32 non-alias register/memory r/m+reg and reg+r/m, accumulator and
Group immediate destination publication, source preservation, `83h /4, FFh`
sign extension, SF/PF and zero-result ZF/PF behavior, with CF/OF clear. AF is
explicitly unasserted because Intel defines it as undefined for AND. It also
proves `67h 66h 21h`, 80186 acceptance, 80286 `66h` #UD non-publication, and
protected source-limit/read-only-destination fault non-publication at `#DF`.
The only correction was a width-local test expectation; no runtime defect was
found.

`M5:T316:S12:AND:OK`

## S13 Primary SUB Closure Evidence

`28h`--`2Dh` route through the six `SUB_*` handlers; Group `80h/81h/83h /5`
routes through `INS_80/81/83` to `_a_sub`. `_a_sub`, `_kac_arith2`, and
`_kaf_set_flags` were audited; no shared change was needed. The focused smoke
proves 8/16/32 non-alias register/memory directions, accumulator and Group
immediates, destination publication and source preservation. Borrow vectors
prove exact CF/SF/AF/PF behavior; signed-minimum subtraction proves exact
OF/AF/PF behavior; and `83h /5, FFh` proves negative-imm8 sign extension.
`67h 66h 29h`, 80186 acceptance, 80286 `66h` #UD non-publication, and
protected source-limit/read-only-destination fault non-publication at `#DF`
are covered. No runtime defect was found.

`M5:T316:S13:SUB:OK`
