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
| Primary binary arithmetic/logical/test: `ADD/OR/ADC/SBB/AND/SUB/XOR/CMP/TEST`, accumulator immediates, Groups `80/81/83` | Primary table and `INS_80/81/83`; `_kac_arith2` uses operand read/write and `_kaf_set_flags`. | `core_machine_inc_dec_smoke` (`M5:T316:S4:TEST:OK`) proves only TEST `A8/A9` and `F6/F7 /0`. | **Partial**: T316 S4 completes the named TEST accumulator and immediate r/m forms at 8/16/32 bits. Other primary TEST register forms and the remaining binary arithmetic/logical groups lack a group proof. Next: ordinary arithmetic/FLAGS family. |
| Primary unary arithmetic: `INC/DEC` register (`40`--`4F`) and Groups `FE/FF /0,/1`; `NEG/NOT` Groups `F6/F7 /2,/3` | Register handlers `INC_*`/`DEC_*`; `INS_FE/FF`, `INS_F6/F7`; all use `_kac_arith1` and flag masks. | `core_machine_inc_dec_smoke` (`M5:T316:S2:INC-DEC:OK`, `M5:T316:S3:NOT-NEG:OK`) proves both admitted slices. | **Complete** only for T316's named INC/DEC and NOT/NEG forms: 16/32-bit register and 8/16/32-bit r/m forms, their Intel FLAGS contracts, 16/32 operand/address attributes, profile behavior, publication, and protected fault non-publication. The wider unary/arithmetic family remains **Partial**: `F6/F7 /4`--`/7` belong to separately named multiply/divide/shift slices. |
| Decimal/ASCII adjust and conversion: `DAA/DAS/AAA/AAS/AAM/AAD`, `XLAT` | Primary handlers `DAA`, `DAS`, `AAA`, `AAS`, `AAM`, `AAD`, `XLAT`; DAA/DAS/AAM/AAD call `_kaf_set_flags`. | No focused architecture probe found. | **Partial**: handlers exist, but documented flag-defined/undefined behavior, base-immediate edge cases, 16/32 addressing for XLAT, and faults are not proven. Next: ordinary arithmetic/FLAGS family. |
| Shift/rotate Groups `C0/C1/D0`--`D3`, `F6/F7 /4`--`/7` | `INS_C0/C1/D0/D1/D2/D3/F6/F7`; shift paths call `_kaf_set_flags`. | No focused primary shift/rotate smoke; T310's double-shift smoke is only `SHLD/SHRD`. | **Partial**: primary count masking, count-zero flags, rotate flags, and memory faults remain unproven. Next: ordinary arithmetic/FLAGS family. |
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
| `_kaf_set_flags` | Called by arithmetic macros, primary shifts, string compares, DAA/DAS/AAM/AAD, and T310 SHLD/SHRD; it owns mask-driven CF/PF/AF/ZF/SF/OF publication. | T316 S2--S4 cover the named INC/DEC, NOT/NEG, and TEST callers; T310 covers SHLD/SHRD and selected string/data paths. Broader primary flag breadth is incomplete. | Do not change it in T316. Each admitted slice asserts its handler-bound flag contract; any shared-helper correction still requires caller-impact review. |
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
