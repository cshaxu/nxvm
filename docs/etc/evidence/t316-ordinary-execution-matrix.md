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
| Register/memory/immediate data movement: `MOV`, `XCHG`, `LEA`, `LES/LDS`, moffs, `PUSH/POP`, `PUSHA/POPA`, `PUSH imm`, `ENTER/LEAVE`, `CBW/CWD` (including 32-bit attribute variants) | Primary table routes to `MOV_*`, `XCHG_*`, `LEA_R32_M32`/`LDS_R32_M16_32`/`LES_R32_M16_32`, `PUSHA`/`POPA`, stack helpers `_kec_push/_kec_pop`, and `ENTER/LEAVE`; primary defaults do not by themselves certify 32-bit forms. | T302 stack/address focused probe; T301 prefix/profile baseline; `core_machine_les_lds_s41_smoke` (`M5:T316:S41:LES-LDS:OK`), `core_machine_pusha_popa_smoke` (`M5:T316:S42:PUSHA-POPA:OK`), `core_machine_enter_leave_smoke` (`M5:T316:S43:ENTER-LEAVE:OK`), `core_machine_gpr_push_pop_smoke` (`M5:T316:S44:GPR-PUSH-POP:OK`), S26 LEA, S27 XCHG, S29 sign-extend, and S30--S35 movement/string smokes. | **Partial**: S44 closes only general-register PUSH/POP `50`--`5F`, `FF /6`, and `8F /0`; PUSH immediate, PUSHF/POPF, PUSHA/POPA, ENTER/LEAVE, segment-register stacks, stack switching, and wider stack breadth remain partial or outside this slice. Earlier bounded S26--S43 evidence remains indexed below. |
| Primary binary arithmetic/logical/test: `ADD/OR/ADC/SBB/AND/SUB/XOR/CMP/TEST`, accumulator immediates, Groups `80/81/83` | Primary table and `INS_80/81/83`; `_kac_arith2` uses operand read/write and `_kaf_set_flags`. | `core_machine_inc_dec_smoke` (`M5:T316:S4:TEST:OK`, `M5:T316:S7:TEST-RM-REG:OK`, `M5:T316:S8:ADD:OK`, `M5:T316:S9:ADC:OK`, `M5:T316:S10:SBB:OK`, `M5:T316:S11:OR:OK`, `M5:T316:S12:AND:OK`, `M5:T316:S13:SUB:OK`, `M5:T316:S14:XOR:OK`, `M5:T316:S15:CMP:OK`) proves the declared slices. | **Complete for this enumerated primary slice**: TEST forms admitted by S4/S7; `00`--`05` plus `/0` ADD; `08`--`0D` plus `/1` OR; `10`--`15` plus `/2` ADC; `18`--`1D` plus `/3` SBB; `20`--`25` plus `/4` AND; `28`--`2D` plus `/5` SUB; `30`--`35` plus `/6` XOR; and `38`--`3D` plus `/7` CMP, including their declared accumulator/immediate, operand/address, profile, FLAGS, publication, and fault boundaries. This does not claim the wider ordinary arithmetic/FLAGS family complete. |
| Primary unary arithmetic: `INC/DEC` register (`40`--`4F`) and Groups `FE/FF /0,/1`; `NEG/NOT` Groups `F6/F7 /2,/3` | Register handlers `INC_*`/`DEC_*`; `INS_FE/FF`, `INS_F6/F7`; all use `_kac_arith1` and flag masks. | `core_machine_inc_dec_smoke` (`M5:T316:S2:INC-DEC:OK`, `M5:T316:S3:NOT-NEG:OK`) proves both admitted slices. | **Complete** only for T316's named INC/DEC and NOT/NEG forms: 16/32-bit register and 8/16/32-bit r/m forms, their Intel FLAGS contracts, 16/32 operand/address attributes, profile behavior, publication, and protected fault non-publication. The wider unary/arithmetic family remains **Partial**: `F6/F7 /6,/7` division is a separately named slice. |
| Decimal/ASCII adjust and conversion: `DAA/DAS/AAA/AAS/AAM/AAD`, `XLAT` | Primary handlers `DAA`, `DAS`, `AAA`, `AAS`, `AAM`, `AAD`, `XLAT`; DAA/DAS/AAM/AAD call `_kaf_set_flags`. | `core_machine_inc_dec_smoke` (`M5:T316:S16:DECIMAL-ADJUST:OK`, `M5:T316:S17:XLAT:OK`) proves the admitted slices. | **Complete only for T316 S16's six named adjust forms** and T316 S17 XLAT: DS default and ES override, 16-bit BX/AL and `67h` EBX/AL addressing, AL-only publication, legacy/profile behavior, and protected read-limit fault non-publication. The wider data/operand and ordinary arithmetic/FLAGS families remain **Partial**. |
| Shift/rotate Groups `C0/C1/D0`--`D3` | `INS_C0/C1/D0/D1/D2/D3`; rotate paths call `_a_rol/_a_ror/_a_rcl/_a_rcr`; shift paths call `_a_shl/_a_shr/_a_sar`; `/6` remains `UndefinedOpcode`. | `core_machine_rotate_smoke` (`M5:T316:S18:ROTATE:OK`, `M5:T316:S19:SHIFT:OK`) proves the declared slices; T310's double-shift smoke remains only `SHLD/SHRD`. | **Complete only for T316 S18 rotates and S19 SHL/SAL, SHR, SAR**: Group-2 encodings, 8/16/32-bit register and memory operands, count handling, defined/undefined FLAGS boundaries, attributes, profiles, and protected access atomicity. `/6` remains #UD. |
| One-operand multiply/divide Groups `F6/F7 /4`--`/7` | `INS_F6/F7` dispatches `/4` to `_a_mul`, `/5` to `_a_imul`, `/6` to `_a_div`, and `/7` to `_a_idiv`. | `core_machine_inc_dec_smoke` (`M5:T316:S5:MUL-IMUL:OK`, `M5:T316:S6:DIV-IDIV:OK`) covers `/4`--`/7`. | **Complete** only for T316's declared one-operand Group `F6/F7 /4`--`/7` forms at 8/16/32 bits. This does not claim the wider multiply/divide or ordinary arithmetic family complete. |
| EFLAGS transfers, direct flag control, and HLT: `PUSHF/POPF`, `LAHF/SAHF`, `CMC/CLC/STC/CLI/STI/CLD/STD`, `HLT` | `PUSHF`, `POPF`, `SAHF`, `LAHF`, `HLT`, and primary flag handlers; protected-mode masking and halt state are local to those routes. | T302 checks selected PUSHF/POPF; `core_machine_pushf_popf_s47_smoke` (`M5:T316:S47:PUSHF-POPF:OK`), `core_machine_lahf_sahf_smoke` (`M5:T316:S39:LAHF-SAHF:OK`), `core_machine_direct_flags_smoke` (`M5:T316:S40:DIRECT-FLAGS:OK`), `core_machine_cli_sti_s48_smoke` (`M5:T316:S48:CLI-STI:OK`), and `core_machine_hlt_s49_smoke` (`M5:T316:S49:HLT:OK`) prove their named slices. | **Partial**: S47 closes only PUSHF/POPF `9C`/`9D`; S48 closes only CLI/STI `FA`/`FB`; S49 closes only HLT `F4`. IRET, VME/PVI extensions, task switching, NMI, generic interrupt architecture, and other FLAGS/control behavior remain outside these slices. |
| Software interrupt forms: INT3 `CC`, INT imm8 `CD ib`, INTO `CE` | `INT3`, `INT_I8`, `INTO`, `_e_int3`, `_e_int_n`, `_e_into`, and real/protected interrupt serialization. | `core_machine_software_int_s50_smoke` (`M5:T316:S50:SOFTWARE-INT:OK`) proves the admitted S50 routes. | **Complete only for T316 S50**: INT3, immediate INT, and INTO default and declared prefix/profile, IVT/IDT entry, DPL/VM86/fault, LOCK, and pending-PIC boundaries. IRET, hardware IRQ/NMI, task gates/switches, VME/PVI, outer privilege returns, and generic interrupt architecture remain outside this slice. |
| Memory strings `MOVS/CMPS/STOS/LODS/SCAS`, REP/REPE/REPNE, DF, 16/32 operand/address attributes | `MOVSB/W`, `CMPSB/W`, `STOSB/W`, `LODSB/W`, `SCASB/W`, `_kas_move_index`; primary REP loop. | `core_machine_real_mode_386_rep_cmps_smoke` (`M5:T292:S1:REP-STRING:OK`), `core_machine_movs_smoke` (`M5:T316:S33:MOVS:OK`), `core_machine_stos_smoke` (`M5:T316:S34:STOS:OK`), `core_machine_lods_smoke` (`M5:T316:S35:LODS:OK`), `core_machine_scas_smoke` (`M5:T316:S36:SCAS:OK`), and `core_machine_cmps_smoke` (`M5:T316:S37:CMPS:OK`). | **Partial**: S35 closes only LODSB `AC` and LODSW/LODSD `AD`; S36 closes only SCASB `AE` and SCASW/SCASD `AF`; S37 closes only CMPSB `A6` and CMPSW/CMPSD `A7`, including declared attributes, DS source/fixed ES destination, FLAGS, REPE/REPNE condition/restart, #UD/LOCK non-publication, protected DS/ES read-limit #DF, and PIC boundaries. INS/OUTS and broader string behavior remain partial or outside this slice. |
| Ordinary port I/O `IN`/`OUT` immediate and DX forms | `IN_AL_I8`/`IN_EAX_I8`, `OUT_I8_AL`/`OUT_I8_EAX`, `IN_AL_DX`/`IN_EAX_DX`, `OUT_DX_AL`/`OUT_DX_EAX`, `_p_input`, `_p_output`, and `_kpa_test_mode`. | `core_machine_port_io_s55_smoke` (`M5:T316:S55:PORT-IO:OK`). | **Complete only for T316 S55**: immediate-port and DX-port `E4`--`E7`/`EC`--`EF`, their declared widths, profiles, attributes, permission/provider, rejection, and pending-IRQ boundaries. INS/OUTS, generic I/O privilege architecture, port-device semantics, and VME/PVI remain outside this slice. |
| Port strings `INS/OUTS` with REP and size/address attributes | `INSB/INSW/OUTSB/OUTSW`, `_p_input`, `_p_output`, `_kpa_test_mode`, and `_kas_move_index`. | T302 I/O-string portion and `core_machine_port_strings_smoke` (`M5:T316:S38:PORT-STRINGS:OK`). | **Partial**: S38 closes only INSB `6C`, INSW/INSD `6D`, OUTSB `6E`, and OUTSW/OUTSD `6F`, including declared port width, profile, attribute, segment, REP, limit-fault, and interrupt boundaries. Ordinary IN/OUT, general I/O privilege architecture, and broader string behavior remain outside this slice. |
| Short/near conditional control: `Jcc`, `LOOP/LOOPE/LOOPNE`, `JCXZ/JECXZ`, near `CALL/JMP/RET` | Primary `J*_REL8`, `LOOP*`, `JCXZ_REL8`, `CALL_REL32`, `JMP_REL*`, return helpers; `0F 80`--`8F` repeats the profile gate. | `core_machine_control_transfer_smoke` (`M5:T303:CONTROL-TRANSFER:OK`). | **Complete** for T303's declared real/protected 16/32 near, condition, target-limit, and atomic-fault matrix. Other control forms remain separately listed. |
| Far direct/indirect `CALL/JMP`, same-CPL `RETF` and `IRET`; outer returns, gates, task/V86 interactions | `CALL_PTR16_32`, `JMP_PTR16_32`, `_kec_call_far/_kec_jmp_far/_kec_ret_far`, `_e_iret`, and protected return routes. | T303 proves admitted real/protected far and same-CPL return cases; `core_machine_iret_s51_smoke` (`M5:T316:S51:IRET:OK`) and `core_machine_iret_outer_s52_smoke` (`M5:T316:S52:IRET-OUTER:OK`) prove their bounded IRET slices; T305--T308 prove bounded privilege paths. | **Partial** for the ordinary-family inventory: S51 closes only real-mode and protected same-CPL IRET `CF`; S52 closes only protected IRET returns from CPL0 to an outer CPL. Task/gate, VM86/VME/PVI, outer-return variants beyond this bounded route, and broader exception interaction remain separate boundaries. |
| 80386 `0F 90`--`9F` SETcc | `INS_0F` then `SETO_RM8`--`SETG_RM8`; metadata requires 80386. | `core_machine_setcc_smoke` (`M5:T310:S3:SETCC:OK`) covers conditions, register/memory, prefixes, and pre-fault non-publication. | **Complete** for the declared T310 SETcc matrix. |
| 80386 `0F A3/AB/B3/BB`, `0F BA /4`--`/7` BT/BTS/BTR/BTC | `BT_*`, `BTS_*`, `BTR_*`, `BTC_*`, `INS_0F_BA`; metadata/profile gate. | `core_machine_bit_test_smoke` (`M5:T310:S5:BIT:OK`) covers register/memory, immediate/indexed, 16/32, address prefix, rejection, and access-failure publication. | **Complete** for the declared T310 bit-test matrix. |
| 80386 `0F A4/A5/AC/AD` SHLD/SHRD | `SHLD_*`/`SHRD_*`, `_kaf_set_flags`, 80386 metadata gate. | `core_machine_double_shift_smoke` (`M5:T310:S6:DOUBLE-SHIFT:OK`) covers forms, count zero, profile rejection, and access failure. | **Complete** for the declared T310 double-shift matrix. |
| 80386 `0F AF` two-operand IMUL; `0F BC/BD` BSF/BSR; `0F B6/B7/BE/BF` MOVZX/MOVSX | `IMUL_R32_RM32`, `BSF/BSR`, `MOVZX/MOVSX` through `INS_0F` and metadata gate. | `core_machine_imul2_smoke` (`M5:T310:S8:IMUL2:OK`), `core_machine_bit_scan_smoke` (`M5:T310:S7:BIT-SCAN:OK`), `core_machine_movx_smoke` (`M5:T310:S4:MOVX:OK`). | **Complete** for each declared T310 form matrix, including profile rejection and named memory/fault boundaries. |
| 80386 `0F A0/A1/A8/A9` FS/GS push/pop and `0F B2/B4/B5` LSS/LFS/LGS | `INS_0F` table and segment-load routes, with profile checks; `_e_load_far` loads the selector before publishing the destination offset and sets the maskable-IRQ shadow only for SS. | `core_machine_fs_gs_stack_smoke` (`M5:T316:S23:FS-GS-STACK:OK`) and `core_machine_lss_lfs_lgs_smoke` (`M5:T316:S24:LSS-LFS-LGS:OK`). | **Complete only for T316 S23's FS/GS stack forms and S24's LSS/LFS/LGS matrix**: default/`66h` operand size, memory-only form, 80386 gate, selected real/protected publication, bounded source-fault non-publication, and the SS-only IRQ shadow. FS/GS prefix consumers, `LES`/`LDS`, MOV/POP other segment-register families, and broader privilege semantics remain **Partial**. |
| ARPL `63 /r` selector RPL adjustment | `ARPL_RM16_R16` with `_d_modrm`, `_m_read_rm`, and `_m_write_rm`; the handler requires protected mode and profile 80286 or later. | `core_machine_arpl_s53_smoke` (`M5:T316:S53:ARPL:OK`) executes the bounded S53 vectors. | **Complete only for T316 S53**: r/m16,r16 RPL comparison/update and ZF semantics, register and memory destinations, declared segment/address attributes, profile/LOCK rejection, protected operand-access boundary, and PIC delivery. Descriptor validation/loading, MOV/POP Sreg, LAR/LSL/VERR/VERW, and general segment privilege architecture remain outside this slice. |
| BOUND `62 /r` signed range check | `BOUND_R16_M16_16` decodes memory-only ModRM through `_d_modrm` and reads the signed lower/upper pair through `_m_read_rm`; `_SetExcept_BR` now reaches the narrow `#BR` vector-5 delivery route in `ExecFinal`. | `core_machine_bound_s54_smoke` (`M5:T316:S54:BOUND:OK`) executes the bounded S54 vectors. | **Complete only for T316 S54**: BOUND r16,m16&16 and r32,m32&32 with the declared profile, attribute, signed-boundary, segment/EA, IVT/IDT `#BR`, operand-limit, VM86, and pending-PIC boundaries. General exception delivery, descriptor validation, and broader arithmetic/control-transfer behavior remain outside this slice. |
| Immediate three-operand IMUL `69 /r iw/id`, `6B /r ib` | `IMUL_R32_RM32_I32`, `IMUL_R32_RM32_I8`, `_a_imul3`, `_d_modrm`, and r/m access routes. | `core_machine_imul_immediate_s56_smoke` (`M5:T316:S56:IMUL-IMM:OK`). | **Complete only for T316 S56**: 80186--80386 immediate three-operand IMUL, declared 16/32 operand/address attributes, signed product CF/OF boundary, r/m segment/source-limit, VM86, rejection, and PIC boundaries. One-operand IMUL, `0F AF`, MUL/DIV, and general FLAGS behavior remain outside this slice. |
| LAR/LSL `0F 02 /r`, `0F 03 /r` selector inspection | `INS_0F`, `LAR_R32_RM32`, `LSL_R32_RM32`, selector/XDT and r/m read routes. | `core_machine-lar-lsl-s57-smoke` (`M5:T316:S57:LAR-LSL:OK`). | **Complete only for T316 S57's bounded LAR/LSL query forms**: protected GDT/LDT selector inspection, declared attribute/profile/LOCK and memory-source boundaries, VM86 rejection, and PIC delivery. VERR/VERW, selector loads, and broader descriptor architecture remain outside this slice. |
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

## S14 Primary XOR Evidence

`30h`--`35h` route through the six `XOR_*` handlers and Group
`80h/81h/83h /6` routes through `INS_80/81/83` to `_a_xor`. The owner-bound
smoke covers 8/16/32 non-alias register/memory directions, accumulator and
Group immediates, zero-result ZF/PF, and `83h /6, FFh` sign extension. CF/OF
are asserted clear; AF is deliberately unasserted because Intel defines it
as undefined. `67h 66h 31h`, 80186 acceptance, 80286 `66h` #UD
non-publication, and protected source-limit/read-only-destination `#DF`
non-publication are present. `_a_xor`, `_kac_arith2`, and `_kaf_set_flags`
were audited; no runtime/shared-helper change was made.

`M5:T316:S14:XOR:OK`

## S15 Primary CMP Evidence

`38h`--`3Dh` route through the six `CMP_*` handlers and Group
`80h/81h/83h /7` routes through `INS_80/81/83` to `_a_cmp`. The owner-bound
smoke covers 8/16/32 non-alias register and memory directions, accumulator and
Group immediates, and confirms both operands remain unchanged. Borrow and
signed-overflow boundaries assert exact CF/OF/SF/ZF/AF/PF values; `83h /7,
FFh` proves negative-imm8 sign extension. `67h 66h 39h`, 80186 acceptance,
80286 `66h` #UD non-publication, and protected source-limit faults for both
memory-left (`39h`) and memory-right (`3Bh`) directions with `#DF`
non-publication are covered. `_a_cmp`, `_kac_arith2`, and
`_kaf_set_flags` were audited; no runtime/shared-helper change was needed.
Focused diagnosis corrected two fixture expectations: `CMP r/m8, -1` sets
both CF and AF for `00h - FFh`, and CMP's no-write contract means a read-only
memory operand is not a fault condition.

`M5:T316:S15:CMP:OK`

## S18 Group-2 Rotate Closure Evidence

`INS_C0`, `INS_C1`, `INS_D0`, `INS_D1`, `INS_D2`, and `INS_D3` dispatch
Group-2 `/0`--`/3` to `_a_rol`, `_a_ror`, `_a_rcl`, and `_a_rcr`. The
S18 sweep audited every one of those routes and their local CF/OF publication;
no shared operand, address, or flag helper was changed. `/4`--`/7`
`SHL/SAL/SHR/SAR` are explicitly outside this slice and remain partial.

`core_machine_rotate_smoke` proves ROL/ROR/RCL/RCR at 8, 16, and 32 bits for
register and memory r/m operands through immediate-count `C0/C1`, one-count
`D0/D1`, and CL-count `D2/D3` encodings. Its immediate and CL count `21h`
vectors prove 5-bit masking; the four operations, three widths, and both
immediate/CL routes also prove count-zero no-op publication. RCL/RCR cases
start with CF set and apply their width-specific carry-ring count handling. A
separate immediate effective-count-two matrix covers all four rotates and all
three widths, proving multi-step result/CF publication while deliberately not
asserting Intel-undefined OF.
For effective count one the smoke asserts Intel-defined CF and OF; for other
nonzero counts it asserts CF and deliberately does not claim OF, while the
other FLAGS remain unchanged.

The matrix covers destination memory publication and register preservation,
including CL preservation. `67h 66h` memory vectors prove 32-bit
address/operand attributes. An 80186 immediate-count form is accepted; a
80286 `66h` form raises `#UD` without publishing EAX, EFLAGS, or EIP. The
protected fixture proves both out-of-limit read and read-only write failures
retain destination memory, EFLAGS, and EIP at the existing diagnostic fault
boundary.

The initial count-zero vectors exposed a local implementation defect: all four
handlers treated effective zero as an undefined-OF case, and ROR also republished
CF from the unchanged operand. The local 8/16/32 branches now early-exit on
effective zero, preserving all FLAGS as Intel requires. No decoder, ABI,
shared helper, CMake artifact, or preset changed.

The prescribed similar-issue sweep classifies `_a_rol/_a_ror/_a_rcl/_a_rcr`
as **fixed and covered** by the new owner-bound matrix; `INS_C0/C1/D0/D1/D2/D3`
as **covered** by its immediate, one, and CL encodings; and their local
CF/OF publications as **covered** by effective-one and effective-two vectors.
The other Group-2 `/4`--`/7` hits are **deferred/out of scope** to an explicitly
admitted shift slice. `_kaf_set_flags` and the broader arithmetic CF/OF hits
are **out of scope** because rotate handlers do not call that helper; no shared
helper modification was considered. T310 `SHLD/SHRD` evidence remains a
separate covered double-shift route rather than evidence for primary rotates.

`M5:T316:S18:ROTATE:OK`

## S19 Group-2 Shift Evidence

S19 adds owner-bound proof for `SHL/SAL /4`, `SHR /5`, and `SAR /7` through
`C0/C1/D0/D1/D2/D3`. The matrix covers 8/16/32-bit register and memory r/m
forms, immediate-one, imm8, and CL counts, count-zero no-op, `21h` five-bit
masking, and defined CF/OF/SF/ZF/PF results; AF and non-one-count OF are
explicitly unasserted. A separate effective-count-two vector for every
operation and width proves multi-step result, CF, SF/ZF/PF while leaving AF/OF
unasserted. `67h 66h` memory, all-three-operation 80186 legacy acceptance, 80286
`66h #UD` non-publication, and `/6 #UD` are covered. Per-operation protected
read-limit and read-only-destination vectors retain memory, EFLAGS, and EIP.
No shared helper or runtime implementation changed; the sweep classifies the
three local helpers and six dispatch routes as covered, `/6` as retained #UD,
and the remaining Group-2 rotate routes as accepted S18 scope.

`M5:T316:S19:SHIFT:OK`

## S20 Local EFLAGS Evidence

`core_machine_eflags_local_smoke` proves `LAHF`, `SAHF`, `CMC`, `CLC`, `STC`,
`CLD`, and `STD` through their primary decoder entries. SAHF covers both low
flag inputs and preserves all non-SAHF flags; LAHF proves AH low-flag transfer,
bit1 forced to one, EFLAGS preservation, and non-AH EAX preservation. CMC
uses both CF inputs; CLC/STC and CLD/STD assert their sole affected bit while
preserving the rest. All seven forms are independently reachable on both 8086
and 80186 profiles; they have no operand/address-size attribute semantics.
The focused fixture is fault-free and checks EIP publication. A focused failure
found LAHF omitted Intel-mandated AH bit1; the local handler now ORs `02h` into
its AH result. PUSHF/POPF, CLI/STI, IOPL and interrupt shadow remain partial
and out of S20 scope; no shared helper changed.

`M5:T316:S20:EFLAGS-LOCAL:OK`

## S21 PUSHF/POPF Evidence

`core_machine_pushf_popf_smoke` completes only the 80386 `9Ch/9Dh` slice:
16/32 operand forms, ESP deltas, normalized stack images (reserved bits clear
and bit1 set; PUSHFD also clears VM/RF), POPF writable/preserved bits, CPL0
and controlled-CPL3 IOPL/IF gates, VM86 IOPL3 success and IOPL<3 first-#GP
non-publication, plus protected PUSHF write and POPF read stack-fault
atomicity. The VM86 fixture asserts the original #GP diagnostic and source
state non-publication; its later exception-delivery endpoint is not claimed by
S21. CLI/STI, interrupt shadow, and wider FLAGS control remain partial.

`M5:T316:S21:PUSHF-POPF:OK`

### T316 S22 - CLI/STI and maskable IRQ shadow

`core_machine_cli_sti_smoke` proves `FA`/`FB` real-mode reachability on 8086,
80186, and 80386; CLI/STI IF/EIP publication and preservation of the other
tested FLAGS; and deterministic pending-PIC delivery.  The pending IRQ remains
in IRR while STI retires, then is delivered only after one ordinary NOP retires;
the handler frame returns to IP 2 on every profile.  CLI has no shadow and
leaves the IRQ pending.

The protected matrix proves both opcodes at CPL0 and controlled CPL3/IOPL3.
CPL3/IOPL0 rejection uses a delivered `#GP(0)` frame (error code, original
EIP 0, CS 8, and original EFLAGS) rather than `first_fault`; this proves the
instruction did not publish IF or EIP before exception entry.  The VM86 matrix
uses the retained S21 first-fault gate boundary: IOPL3 succeeds and IOPL0
records the original #GP without instruction-state publication.  NMI, POPF and
MOV SS shadow ownership, interrupt-delivery endpoints beyond these observed
boundaries, and wider FLAGS control remain partial.

`M5:T316:S22:CLI-STI:OK`

### T316 S23 - FS/GS stack forms

`core_machine_fs_gs_stack_smoke` proves `0F A0/A1/A8/A9` only: PUSH/POP FS
and PUSH/POP GS on 80386 in default 16-bit and `66h` 32-bit operand-size
forms, with exact stack deltas/images, selector publication, EIP, and retained
general register/EFLAGS state. It also proves 80286 #UD non-publication and
valid protected POP FS/GS selector loads. For the bounded SS-source-limit
fault, POP FS/GS retain EIP, ESP, EFLAGS and destination selector; the existing
fixture observes the resulting first-#DF diagnostic boundary after #SS.
`_e_pop_sreg`'s `flagMaskInt` setter is classified as SS-only and is not
expanded by this slice.

LSS/LFS/LGS, FS/GS prefix consumers, general segment-selector families, and
interrupt-shadow ownership remain partial.

`M5:T316:S23:FS-GS-STACK:OK`

### T316 S24 - LSS/LFS/LGS memory far-pointer forms

`core_machine_lss_lfs_lgs_smoke` proves the bounded `0F B2` LSS, `0F B4`
LFS, and `0F B5` LGS matrix.  Each form uses the required memory ModRM in
real mode and a controlled protected selector load, in both default 16-bit
and `66h` 32-bit operand-size forms.  The vectors assert source offset to
AX/EAX, destination selector, EIP, and retained EFLAGS.  Register-direct
encodings #UD on 80386, and all memory forms #UD on 80286, without GPR,
EFLAGS, or opcode-selected SS/FS/GS publication.

The protected source-boundary probe constrains DS across the far pointer and
observes the retained first-#DF delivery boundary; it proves EIP, EAX,
EFLAGS, and the selected destination selector remain unpublished.  The
deterministic PIC fixture proves LSS defers a pending maskable IRQ through
exactly one following NOP (interrupt frame IP 6), while LFS and LGS take the
same IRQ before that NOP (frame IP 5).  `_e_load_far` caller review classifies
the resulting shadow as SS-only; no shared helper or runtime change was
needed.  FS/GS prefix consumers, `LES`/`LDS`, general segment-register
families, and broader segment privilege semantics remain partial.

`M5:T316:S24:LSS-LFS-LGS:OK`

### T316 S25 - LES/LDS memory far-pointer forms

`core_machine_les_lds_smoke` proves C4 LES and C5 LDS with memory-only
ModRM: default 16-bit legacy execution on 8086/80286/80386, and `66h`
32-bit execution only on 80386. Register-direct encodings and `66h` forms
below 80386 #UD without GPR, EFLAGS, or selected ES/DS publication. The
protected vectors prove valid selector publication, bounded source-fault
non-publication through the observed first-#DF boundary, and deterministic
pending IRQ delivery before the following NOP (frame IP 4), so these forms do
not obtain the SS-only interrupt shadow. No runtime change was required.

`M5:T316:S25:LES-LDS:OK`

### T316 S26 - LEA memory-only form

`core_machine_lea_smoke` proves Intel `LEA 8D /r` as a memory-only form: the
default 16-bit form on 8086, 80186, 80286, and 80386, plus all four independent
operand/address-size combinations on 80386 in real and controlled protected
mode. It checks destination-width publication, EIP, FLAGS, and source-GPR
preservation. Register-direct and LOCK encodings #UD without publication; all
66/67-bearing forms #UD below 80386. A protected null-DS vector proves LEA
does not perform a data-segment or source-memory access. A pending PIC reaches
the handler before the following NOP, proving no interrupt shadow. The
`LEA_R32_M32`, `_d_modrm_ea`, `_kdf_modrm`, prefix route, and table route were
audited; no runtime defect was reproduced, so no runtime source changed.

`M5:T316:S26:LEA:OK`

The coordinator independently reran `current-gates-gcc` after the closure
review: all 52 static/governance checks and all 156 current-gate CTest cases
passed.

### T316 S27 - XCHG r/m forms

`core_machine_xchg_smoke` proves `86` and `87 /r` register and memory forms at
8, 16, and 32 bits, including independent `66`/`67` attributes. Default
16-bit `87` register and memory forms execute on 8086, 80186, and 80286; all
66/67-bearing forms reject below 80386 without publication. It proves memory
LOCK acceptance, LOCK register-direct #UD, FLAGS preservation, protected read
and write fault non-publication, and no IRQ shadow. The apparent early
register-write ordering is transaction-local: the write-fault regression
observes no register or memory publication. The 52 static checks and all 157
current-gate tests passed.

`M5:T316:S27:XCHG:OK`

### T316 S28 - accumulator XCHG forms

`core_machine_xchg_smoke` proves `90` as the architecturally equivalent NOP
and `91`--`97` as the named AX/EAX exchanges with ECX, EDX, EBX, ESP, EBP, ESI,
and EDI. Default 16-bit forms run on 8086, 80186, 80286, and 80386 with
low-word exchange and high-half preservation; all 80386 `66h` forms exchange
the full 32-bit values. The focused vectors assert EIP and EFLAGS preservation,
all nonparticipant GPR preservation, lower-profile `66h` #UD without
publication, and LOCK #UD without publication. A pending-PIC vector proves
the `91` exchange retires and reaches the handler before the following NOP, so
these accumulator handlers establish no interrupt shadow. No runtime change
was required. The coordinator independently passed all 52 static/governance
checks and all 157 current-gate tests.

`M5:T316:S28:XCHG-ACC:OK`

### T316 S29 - sign-extension conversion forms

`core_machine_sign_extend_smoke` proves `98` CBW/CWDE and `99` CWD/CDQ:
default 16-bit positive and negative conversions on 8086, 80186, 80286, and
80386, plus 80386 `66h` 32-bit positive and negative conversions. It asserts
the exact AX/EAX or DX/EDX publication, nonparticipant GPR and EFLAGS
preservation, and 80386 `67h` acceptance without semantic effect. Lower-profile
`66h`/`67h` forms #UD without publication; `F0 98` and `F0 99` #UD without
publication on 80386 only. The separate legacy LOCK-prefix divergence is
recorded as `TODO(Medium)`, not claimed complete here. Pending-PIC vectors for
both handlers prove no interrupt shadow. No runtime change was required.
The coordinator independently passed all 52 static/governance checks and all
158 current-gate tests.

`M5:T316:S29:SIGN-EXTEND:OK`

### T316 S30 - moffs MOV forms

`core_machine_moffs_smoke` proves only MOV moffs `A0`--`A3`: default byte and
word forms on 8086, 80186, 80286, and 80386; 80386 `66h`, `67h`, and combined
operand/address-size forms; default DS and ES/FS/GS override selection; and
exact read/write publication with nonparticipant preservation. Pre-80386
`66h`/`67h` and 80386 LOCK forms #UD without publication. Protected-mode
read and write segment-limit vectors prove GPR/EIP/EFLAGS and target-memory
nonpublication at the no-IDT `#DF` observable boundary. Pending-PIC vectors
for a moffs read and write prove no interrupt shadow. No CPU production defect
was found. This closes the bounded moffs slice only; ModRM MOV, strings, and
stack forms remain outside S30.

`M5:T316:S30:MOFFS:OK`

### T316 S31 - primary non-segment GPR MOV forms

`core_machine_gpr_mov_smoke` closes the bounded primary non-segment GPR MOV
slice only: `88`/`89`/`8A`/`8B`, `C6`/`C7`, and `B0`--`BF`. It covers default
8/16-bit execution on 8086, 80186, 80286, and 80386; `66h`/`67h` attributes;
segment selection; prefix and LOCK rejection; #UD non-publication; protected
limit #DF observability without publication; and PIC no-shadow behavior. The
test asserts full destination-GPR publication so partial-register writes retain
their untouched high bits. This is a test-only closure: no production defect
was found or changed. Segment-register `8C`/`8E`, MOVS, and system MOV remain
outside S31.

`M5:T316:S31:GPR-MOV:OK`

### T316 S32 - segment-register MOV forms

`core_machine_sreg_mov_smoke` closes only `8C` `MOV r/m16,Sreg` and `8E`
`MOV Sreg,r/m16`. It covers real-mode `8C` ES/CS/SS/DS register and memory
forms on all four profiles, `8E` ES/SS/DS register and memory forms on all
four profiles, and `8E` CS register and memory #UD; plus 80386 FS/GS register
and memory forms; fixed 16-bit data under `66h`, `67h` and combined
`66h`/`67h` effective addresses;
and reserved, CS, profile, and LOCK #UD rejection without publication.
Protected vectors prove descriptor/cache loading, null-selector invalidation,
accessed-bit update, type/present/privilege boundaries, and memory read/write
limit #DF-observable nonpublication without an IDT. Pending-PIC vectors prove
the exact one-instruction MOV SS shadow and immediate no-shadow delivery for
MOV DS and MOV FS. No CPU production defect was found or changed. This does
not close PUSH/POP, LxS, far transfer, MOVS, or system MOV families.

`M5:T316:S32:SREG-MOV:OK`

### T316 S33 - MOVS string forms

`core_machine_movs_smoke` closes only MOVSB `A4` and MOVSW/MOVSD `A5`.
It covers default MOVSB/MOVSW single and REP execution on all four profiles,
DF direction, DS and CS/SS/ES/FS/GS source selection with fixed ES destination,
MOVSD only through the 80386 `66h` operand-size form, and 80386 `66h`, `67h`,
and combined forms. Prefix/profile and LOCK rejection assert #UD without
publication. Protected source-read and destination-write limits assert the
no-IDT #DF observable boundary with complete GPR, FLAGS, memory, and DS/ES
hidden-cache nonpublication. Pending IRQ proves single MOVS completes without
shadow and REP is restartable: an IRQ may arrive after one primitive iteration,
with partial progress and frame IP at the REP instruction rather than atomic
completion. No CPU production defect was found or changed. This does not close
CMPS, STOS, LODS, SCAS, INS/OUTS, or wider string behavior.

`M5:T316:S33:MOVS:OK`

### T316 S34 - STOS string forms

`core_machine_stos_smoke` closes only STOSB `AA` and STOSW/STOSD `AB`.
It covers default byte/word profiles, 80386 `66h`, `67h`, and combined forms,
F3 REP count/direction behavior, fixed ES destination under segment overrides,
and accumulator-source publication. Prefix/profile and LOCK rejection assert
#UD without publication. Protected ES write-limit vectors cover single restart
and REP partial progress at the no-IDT #DF boundary. Pending IRQ proves single
no-shadow delivery and restartable REP partial progress. No CPU production
defect was found or changed. LODS, CMPS, SCAS, INS/OUTS, and broader string
behavior remain outside this slice.

`M5:T316:S34:STOS:OK`

### T316 S35 - LODS string forms

`core_machine_lods_smoke` closes only LODSB `AC` and LODSW/LODSD `AD`.
It covers single execution across all four profiles, 80386 `66h`, `67h`, and
combined forms, DF direction, and DS plus CS/SS/ES/FS/GS source selection.
F3 REP covers zero, one, and multiple counts with byte/word/dword accumulator
publication, index/count widths, and source preservation. Prefix/profile and
LOCK rejection assert #UD without publication. Protected DS source-read limits
assert no-IDT #DF observability for single restart and REP partial progress with
DS hidden-cache nonpublication. Pending IRQ proves single LODS has no shadow
and REP is restartable after one primitive iteration. No CPU production defect
was found or changed. CMPS, SCAS, INS/OUTS, and broader string behavior remain
outside this slice.

`M5:T316:S35:LODS:OK`

### T316 S36 - SCAS string forms

`core_machine_scas_smoke` closes only SCASB `AE` and SCASW/SCASD `AF`.
It proves four-profile single byte/word forms, 80386 `66h`, `67h`, and combined
attributes, fixed ES scanning despite CS/FS overrides, DF movement, exact
comparison FLAGS, and accumulator/nonparticipant preservation. F3 REPE and F2
REPNE cover zero and conditional multi-iteration termination. Lower-profile
attributes and 80386 LOCK reject with #UD and no publication. Protected ES
read limits prove no-IDT #DF single restart and REP partial progress; pending
IRQ proves single no-shadow delivery and REP restartable partial progress. No
CPU production defect was found or changed. CMPS, INS/OUTS, and wider strings
remain outside this slice.

`M5:T316:S36:SCAS:OK`

### T316 S37 - CMPS string forms

`core_machine_cmps_smoke` closes only CMPSB `A6` and CMPSW/CMPSD `A7`.
It proves default single and REPE/REPNE execution on all four profiles,
80386 `66h`, `67h`, and combined attributes, DS source with fixed ES
destination under CS/FS overrides, DF movement, and complete CF/PF/AF/ZF/SF/OF
comparison flags with both source and destination memory preserved. Lower-profile
attributes and 80386 LOCK reject with #UD and no publication. Protected DS
source and ES destination read limits prove no-IDT #DF single restart and REP
partial progress while preserving both caches. Pending IRQ proves single
no-shadow delivery and REP restartable one-primitive partial progress. No CPU
production defect was found or changed. INS/OUTS and wider string behavior
remain outside this slice.

`M5:T316:S37:CMPS:OK`

### T316 S38 - INS/OUTS port string forms

`core_machine_port_strings_smoke` closes only INSB `6C`, INSW/INSD `6D`,
OUTSB `6E`, and OUTSW/OUTSD `6F`. It uses a local installed port provider to
prove DX selection, port width, fixed-ES INS destination despite CS/FS
overrides, DS/CS/FS OUTS source selection, DF, and exact index/count/EIP/GPR/
FLAGS and memory or port publication. F3 REP covers zero and multiple counts
at 16- and 32-bit address sizes. Lower profiles and pre-386 attributes, plus
80386 LOCK, reject with #UD and no state, memory, or port publication.
Protected ES write-limit INS and DS read-limit OUTS prove no-IDT #DF single
restart and REP partial progress, including cache nonpublication. Pending IRQ
proves single no-shadow delivery and REP restartable one-primitive progress.
The existing I/O-permission route was not widened or claimed complete. No CPU
production defect was found or changed. Ordinary IN/OUT and broader string
behavior remain outside this slice.

`M5:T316:S38:PORT-STRINGS:OK`

### T316 S39 - LAHF/SAHF FLAGS transfers

`core_machine_lahf_sahf_smoke` closes only LAHF `9F` and SAHF `9E`. It proves
the CF/PF/AF/ZF/SF AH mapping in both clear and set states, forced AH bit 1,
LAHF's AH-only EAX publication, and SAHF preservation of the other EFLAGS and
all GPRs on 8086, 80186, 80286, and 80386. Every `66`/`67` and combined form
rejects below 80386; on 80386 those prefix forms retain their fixed
register-only semantics, while LOCK rejects without publication. A local
protected-mode case proves IOPL remains unchanged, and the fixture-expressible
VM86 case proves VM remains unchanged; these register-only instructions have
no applicable memory or selector fault path, so no-IDT fault atomicity is not
claimed. Pending IRQ0 after either successful instruction delivers immediately
at the following NOP, proving neither creates interrupt shadow. No CPU
production defect was found or changed. PUSHF/POPF and other FLAGS/control
families remain outside this slice.

`M5:T316:S39:LAHF-SAHF:OK`

### T316 S40 - direct FLAGS control forms

`core_machine_direct_flags_smoke` closes only CMC `F5`, CLC `F8`, STC `F9`,
CLD `FC`, and STD `FD`. It proves CMC's two CF inputs and each other form's
sole CF or DF mutation while preserving all other EFLAGS, every GPR, and EIP
on 8086, 80186, 80286, and 80386. All `66`/`67` and combined forms reject
below 80386; on 80386 they retain their fixed register-only semantics, while
LOCK rejects without publication. Local protected-mode and VM86 vectors prove
IOPL/VM preservation; no memory or selector operation exists, so no no-IDT
fault boundary applies. Pending IRQ0 after each successful instruction is
delivered immediately at the following NOP, proving no interrupt shadow. No
CPU production defect was found or changed. PUSHF/POPF and CLI/STI remain
outside this slice.

`M5:T316:S40:DIRECT-FLAGS:OK`

### T316 S41 - LES/LDS far-pointer load forms

`core_machine_les_lds_s41_smoke` closes only LES `C4` and LDS `C5` memory
far-pointer loads. It proves default 16-bit forms on all four profiles;
80386 `66` operand-size, `67` address-size, and combined forms; DS default
plus CS/SS/ES/FS/GS source overrides; memory-only ModRM; exact GPR and ES/DS
selector/cache publication; and unchanged source memory. Lower-profile
attributes, register-direct encodings, and 80386 LOCK reject without
publication. Local protected vectors prove valid and null selectors, accessed
descriptor state, non-present/type/RPL selector fault atomicity, and source
limit #DF at the established no-IDT boundary. Pending IRQ proves successful
LES/LDS takes the interrupt immediately after the instruction, without an SS
style shadow. No CPU production defect was found or changed. LSS/LFS/LGS,
MOV/POP Sreg, and wider far-load or segment families remain outside this slice.

`M5:T316:S41:LES-LDS:OK`

### T316 S42 - PUSHA/POPA stack-register forms

`core_machine_pusha_popa_smoke` closes only PUSHA `60` and POPA `61`. It
proves 8086 rejection and 80186/80286/80386 default 16-bit execution; exact
PUSHA stack order including the original SP, POPA's ignored SP slot, all GPR,
EFLAGS, EIP, and stack-image publication; and 80386 `66` 32-bit operand-size,
`67` address-size, and combined forms. Every legacy operand/address prefix
form and every 80386 LOCK form rejects without CPU or stack-memory
publication. Local protected no-IDT probes establish the stack write/read
limit boundary: PUSHA preserves the already-written stack items before its
fault while CPU state rolls back, whereas POPA's tested read-limit fault
publishes neither registers nor SP. Pending IRQ0 arrives immediately after a
successful PUSHA or POPA, proving no interrupt shadow. The focused profile
vectors exposed and corrected the local PUSHA/POPA minimum-profile guard from
80386 to 80186; no shared stack helper changed. Individual PUSH/POP, PUSH
immediate, PUSHF/POPF, ENTER/LEAVE, segment-register stacks, stack switching,
and wider stack behavior remain outside this slice.

`M5:T316:S42:PUSHA-POPA:OK`

### T316 S43 - ENTER/LEAVE frame construction

`core_machine_enter_leave_smoke` closes only ENTER `C8 iw,ib` and LEAVE `C9`.
It proves 8086 rejection; default 16-bit execution on 80186, 80286, and 80386;
80386 `66h` operand-size, `67h` no-effective-address attribute, and combined
forms; and lower-profile attribute plus 80386 LOCK #UD full nonpublication.
The focused frame vectors cover allocation zero/nonzero, lexical levels zero,
one, multiple, and imm8 modulo-32 masking, with exact old frame pointer,
display, frame temporary, 16-bit high-half retention, stack pointer, EIP,
EFLAGS, and stack images. Protected no-IDT stack-limit probes establish ENTER's
observable earlier display writes with CPU rollback, and LEAVE's pre-read
nonpublication, while preserving the SS cache and untouched stack words. Pending
IRQ0 is delivered immediately after a successful ENTER or LEAVE, before the
following NOP, proving no interrupt shadow. The vectors corrected only the
local C8/C9 80186 profile guard and ENTER frame-chain implementation; no shared
stack helper changed. Individual PUSH/POP, PUSH immediate, PUSHF/POPF,
PUSHA/POPA, segment-register stacks, stack switching, and wider stack behavior
remain partial or outside this slice.

`M5:T316:S43:ENTER-LEAVE:OK`

### T316 S44 - general-register PUSH/POP forms

`core_machine_gpr_push_pop_smoke` closes only general-register PUSH/POP
`50`--`5F`, `FF /6`, and `8F /0`. It proves all four default profiles, including
the 8086 `PUSH SP` decremented-source distinction; 80386 `66h`, `67h`, and
combined forms; register and DS/SS/ESP/EBP-based memory operands; exact stack
images, `POP SP`/`POP ESP`, and post-increment POP r/m effective-address
ordering. It also proves `8F /1`--`/7`, lower-profile attributes, and 80386 LOCK
reject without publication; no-IDT protected stack/source/destination limit
fault boundaries; and immediate pending-IRQ delivery after successful register
and r/m PUSH/POP. The focused vectors corrected only local PUSH SP, POP r/m,
and LOCK route behavior; shared stack and memory helpers were unchanged.
PUSH immediate, PUSHF/POPF, PUSHA/POPA, ENTER/LEAVE, segment stacks, stack
switching, and wider stack behavior remain outside this slice.

`M5:T316:S44:GPR-PUSH-POP:OK`

### T316 S45 - PUSH immediate forms

`core_machine_push_immediate_smoke` closes only PUSH immediate `68` and `6A`:
8086 #UD; 80186--80386 default forms; 80386 `66`, `67`, and combined forms;
and exact `6A` sign extension. It proves lower-profile prefix and 80386 LOCK
rejection without publication, protected no-IDT stack-limit #DF nonpublication
for both handlers, and immediate IRQ0 delivery after each successful form.
Only local gates, sign extension, and LOCK rejection changed; shared immediate
and stack helpers remain unchanged. Other stack forms remain outside this slice.

`M5:T316:S45:PUSH-IMMEDIATE:OK`

### T316 S46 - legacy segment-register stack forms

`core_machine_legacy_sreg_stack_smoke` closes only `PUSH/POP ES`, `PUSH CS`,
`PUSH/POP SS`, and `PUSH/POP DS` (`06`--`07`, `0E`, `16`--`17`, `1E`--`1F`).
It proves default profiles, operand/address prefixes and LOCK rejection,
protected valid/null selector boundaries, controlled SS limit #DF nonpublication,
and POP SS one-instruction IRQ shadow versus ES/DS/PUSH no-shadow. FS/GS,
general and immediate stacks, MOV/POP r/m Sreg, LxS, stack switching, and far
returns remain outside this slice.

`M5:T316:S46:LEGACY-SREG-STACK:OK`

### T316 S47 - PUSHF/POPF FLAGS stack-transfer forms

`core_machine_pushf_popf_s47_smoke` closes only `PUSHF`/`PUSHFD` `9C` and
`POPF`/`POPFD` `9D`. It executes the default word forms on 8086, 80186, 80286,
and 80386; 80386 `66h`, `67h`, and combined operand/address prefixes; and all
pre-386 attribute rejections. The vectors prove the visible FLAGS image,
including forced bit1 and reserved-bit disposition, 16/32 stack width and
SP/ESP publication, and POPF/POPFD RF/VM preservation.

The protected fixture exercises CPL/IOPL IF and IOPL masking, stack #DF
no-publication boundaries, and ordinary VM86 IOPL3 success versus IOPL<3 #GP
behavior. VME/PVI is explicitly excluded: it is not an Intel 80386 feature.
The full `F0` LOCK grid for both opcodes and all accepted attribute forms is
#UD with CPU, cache, and candidate stack-slot nonpublication. Pending PIC IRQ0
is delivered immediately after successful PUSHF and POPF, proving no shadow.

Two local handler fixes normalize legacy PUSHF's visible FLAGS image and retain
RF across POPF/POPFD. No shared helper changed. CLI/STI, IRET, VME/PVI, task
switching, and broader FLAGS or stack behavior remain outside this slice.

`M5:T316:S47:PUSHF-POPF:OK`

### T316 S48 - CLI/STI interrupt-enable forms

`core_machine_cli_sti_s48_smoke` closes only CLI `FA` and STI `FB`. It proves
their default forms on 8086--80386; 80386 `66h`, `67h`, and combined fixed
semantics; pre-386 prefix and 80386 LOCK #UD/full nonpublication; and exact IF,
EIP, GPR, and segment-cache behavior. It executes the protected CPL/IOPL and
ordinary VM86 IOPL3/IOPL<3 boundaries, CLI pending-IRQ inhibition, and STI's
one-next-instruction shadow with frame/IP and PIC ISR/IRR assertions. VME/PVI,
IRET, task switching, and wider interrupt/privilege architecture remain outside
this slice; no production or shared-helper change was needed.

`M5:T316:S48:CLI-STI:OK`

### T316 S49 - HLT halt/wake/privilege form

`core_machine_hlt_s49_smoke` closes only HLT `F4`. It executes default HLT on
8086, 80186, 80286, and 80386; 80386 `66h`, `67h`, and combined fixed prefix
semantics; all pre-386 attribute and 80386 LOCK #UD/full CPU nonpublication;
and exact EIP, halt-state, GPR, EFLAGS, and ES/CS/SS/DS/FS/GS cache effects.

The local 80386 protected fixture proves CPL0 success and CPL3 `#GP(0)` at the
established no-IDT `#DF` observable boundary without publication. A separately
constructed ordinary VM86 state produces `#GP(0)` without halting. Pending
maskable IRQ0 with IF set wakes and delivers after HLT (frame IP after `F4`,
PIC ISR set and IRR clear); with IF clear it leaves the CPU halted with IRQ0
pending. CLI/STI, IRET, NMI, task switching, VME/PVI, generic PIC behavior,
and post-80386 behavior remain outside this slice. No production or shared
helper change was needed.

`M5:T316:S49:HLT:OK`

### T316 S50 - software interrupt forms

`core_machine_software_int_s50_smoke` closes only INT3 `CC`, INT imm8 `CD ib`,
and INTO `CE`. It executes all four real-mode profiles, both INTO OF
dispositions, IVT transfer/frame/IF-and-TF effects, and 80386 `66h`, `67h`,
and combined prefix lengths and stack widths. Pre-386 attribute and 80386 LOCK
forms reject with `#UD` without CPU or stack publication.

The local protected fixtures prove software-gate DPL allow/reject behavior,
selected IDT/target failure observability at the established no-IDT `#DF`
boundary, and the ordinary VM86 route. The pending-IRQ fixture retains the
software interrupt result and PIC ISR/IRR boundary without assigning these
forms an interrupt shadow. No production or shared-helper change was needed.
IRET, hardware IRQ/NMI, task gates/switches, VME/PVI, outer privilege returns,
and generic interrupt architecture remain outside S50.

`M5:T316:S50:SOFTWARE-INT:OK`

### T316 S51 - IRET same-privilege return

`core_machine_iret_s51_smoke` closes only IRET `CF` real-mode and protected
same-CPL return frames. It proves the four default profiles; 80386 `66h`,
`67h`, and combined behavior: `66h` selects the wide return frame while `67h`
is inert for IRET's 16-bit stack-address route and preserves ESP's high half;
the vector keeps distinguishable 16- and 32-bit stack-address candidates.
Pre-386 attributes and
80386 LOCK `#UD` full CPU and source-stack nonpublication; exact return
IP/CS/FLAGS/SP publication; and preservation of unaffected GPR and segment
caches.

The bounded protected fixture executes valid 16- and 32-bit same-CPL frames
and the selector nonpresent/type/DPL, return-IP-limit, and SS stack-read-limit
no-IDT `#DF` observable boundaries. It keeps ordinary VM86, outer-CPL/VM
returns, NT/task returns, VME/PVI, IRET error-code handling, and generic
interrupt architecture outside this slice. Pending IRQ0 delivers after a
successful IRET that restores IF, before its following NOP (frame IP `1`), and
remains pending when IRET does not restore IF. No production or shared-helper
change was needed.

`M5:T316:S51:IRET:OK`

### T316 S52 - IRET outer-privilege return

`core_machine_iret_outer_s52_smoke` closes only protected `IRET` (`CFh`)
returns from CPL0 to an outer CPL. S52-local vectors execute valid 16-bit and
32-bit frames plus `66h`, `67h`, and combined disposition, and assert return
EIP/EFLAGS, all non-target GPR and ES/DS/FS/GS preservation, complete target
CS/SS caches, unchanged old kernel frame, and distinct untouched outer-stack
candidates. The focused rejection grid retains pre-386 attribute and 80386
LOCK `#UD` full nonpublication. The existing bounded protected-return fixture
is run through target-CS/SS type, present, privilege, and limit failures plus
old-stack/new-stack fault paths; those cases assert the established installed
handler boundary rather than claiming unobserved pre-handler full-CPU equality.
A local TSS-backed IRQ0 fixture proves an
IF-restoring outer IRET immediately admits IRQ delivery on its target path and
that an IF-clear return leaves it pending; it verifies the CPL switch, handler
frame IP, and PIC ISR/IRR disposition. No production or shared helper changed.
Same-CPL and real IRET, task/NT and VM86/VME/PVI returns, error-code frames,
and generic IDT, IRQ, and NMI behavior remain outside S52.

`M5:T316:S52:IRET-OUTER:OK`

### T316 S53 - ARPL selector RPL adjustment

`core_machine_arpl_s53_smoke` closes only `ARPL r/m16,r16` (`63 /r`) in
protected mode on 80286/80386. It executes all register-direct encodings and
both no-change and RPL-raise paths, proving that only the destination selector
RPL and ZF are published. Memory vectors cover DS and SS defaults, ES/FS/GS
overrides, and 80386 `67h`/`66h` combined addressing; `66h` leaves ARPL's
word payload unchanged. Real mode and 8086/80186 rejection, pre-386
`66h`/`67h` rejection, and 80386 LOCK rejection retain CPU state and the
candidate memory image.

The protected access-boundary vector records the installed no-IDT diagnostic
handler boundary for an out-of-limit r/m destination: its candidate and
adjacent word, ECX, and all segment caches remain unchanged; EIP and SP are
exception-delivery state, while the saved handler-frame FLAGS preserve the
pre-instruction value. A protected 80386 IRQ0 fixture uses a standard
interrupt gate and the STI one-instruction inhibition window: ARPL executes,
then delivery enters the IRQ handler with return IP `3`, correct ZF/result,
saved pre-gate FLAGS, preserved non-stack GPRs/caches, and PIC ISR/IRR state.
ARPL does not descriptor-validate either selector and does not alter segment
caches. No production or shared helper changed.
Descriptor loading, MOV/POP segment-register forms, LAR/LSL/VERR/VERW, and
broader selector privilege architecture remain outside S53.

`M5:T316:S53:ARPL:OK`

### T316 S54 - BOUND range-check and `#BR` delivery

`core_machine_bound_s54_smoke` closes only memory-form `BOUND 62h /r`:
the 80186--80386 default signed r16/m16&16 form and the 80386 `66h`
r32/m32&32 form.  Its vectors prove signed lower and upper equality,
in-range, and out-of-range decisions; the latter reach real-mode IVT and
protected-mode IDT vector 5 with restart at the BOUND instruction.  The
runtime correction is limited to BOUND's 80186 profile gate, its dword upper
pair at EA+4, and the BOUND-only real-mode delivered-`#BR` branch in final
exception delivery.  `_SetExcept_BR` is the production producer reviewed for
that branch; no other exception class or operand/address helper changed.

The focused matrix includes memory-only ModRM rejection, 8086 `#UD`,
pre-386 `66h`/`67h`/combined rejection, 80386 `66h`, `67h`, combined, and
LOCK rejection with CPU/cache nonpublication.  It covers DS and BP/SS
defaults, CS/ES/FS/GS override selection, and a 32-bit-address SIB
SS-default route.  Successful real, protected, and ordinary VM86 vectors
retain BOUND's GPR, FLAGS, cache, and bounds-memory state.  Protected DS
upper-pair access uses the installed handler boundary; the SS pair's no-IDT
delivery chain terminates as the established `#DF` observable boundary, while
the producer remains the SS access route.  A pending IRQ0 vector proves a
successful BOUND has no interrupt shadow: delivery occurs after BOUND and
before its following NOP.  Descriptor loading/validation, general exception
refactoring, and unrelated arithmetic or control-transfer forms remain outside
S54.

`M5:T316:S54:BOUND:OK`

### T316 S55 - ordinary IN/OUT port forms

`core_machine_port_io_s55_smoke` closes only ordinary `IN` `E4`/`E5`/`EC`/`ED`
and `OUT` `E6`/`E7`/`EE`/`EF`: immediate-port and DX-port byte/word forms on
8086--80386, plus the 80386 `66h`, `67h`, and combined forms.  Its local port
provider records port, width-derived payload, reads, and writes; input vectors
prove AL/AX/EAX publication while output vectors retain EAX and all other
nonparticipants, FLAGS, and segment caches.  The full pre-386 attribute and
80386 LOCK grids reject without CPU, cache, provider, or port-side-effect
publication.

The local protected vectors exercise CPL3/IOPL0 TSS I/O-bitmap permit and deny
routes, including IOPL3 bypass; provider failures retain state and emit the
existing error route.  Ordinary VM86 I/O is recorded through its established
no-IDT terminal diagnostic boundary; VME/PVI and generalized I/O-permission
architecture are explicitly outside S55.  Pending IRQ0 after a successful
immediate input and DX output is delivered after the I/O instruction and before
the following NOP, with the input/output publication and PIC ISR/IRR state
preserved in the evidence.  No production or shared-helper change was needed.
INS/OUTS, port-device semantics, and other I/O architecture remain outside
this slice.

`M5:T316:S55:PORT-IO:OK`

### T316 S56 - immediate three-operand IMUL forms

`core_machine_imul_immediate_s56_smoke` closes only `69 /r iw/id` and `6B /r ib`.
It executes the 8086 rejection and 80186/80286/80386 default word forms, signed
positive, negative, boundary, destination/source-alias, and overflow products,
including `6B` sign extension and defined CF/OF publication without asserting
undefined arithmetic flags.

The 80386 vectors cover `66h` dword products, `67h` effective addresses,
combined attributes, DS/SS defaults and CS/ES/FS/GS overrides, and an SS-default
SIB source. Pre-386 attributes and the 80386 `LOCK` default/`66h`/`67h`/
combined grids for both opcodes reject without CPU or source publication.
Controlled DS and SS source limits use the established no-IDT
`#DF` boundary; ordinary VM86 executes successfully. Pending IRQ0 after both
register and memory IMUL is delivered after IMUL and before NOP, preserving the
product, defined FLAGS in the saved frame, and PIC ISR/IRR contract. `_a_imul3` now widens 32-bit signed operands
before multiplication; its only production callers are the two declared forms.
No decoder, memory, or shared FLAGS helper changed. Other IMUL, multiply/divide,
and general arithmetic families remain outside this slice.

`M5:T316:S56:IMUL-IMM:OK`

### T316 S57 - LAR/LSL selector attribute inspection

`core-machine-lar-lsl-s57-smoke` covers only `0F 02 /r` LAR and `0F 03 /r`
selector queries. Protected default forms cover 80286/80386 register and
memory selectors, null/non-present/RPL-denied ZF=0 retention, GDT and LDT
sources, and granular LSL limits. The 80386 grid includes `66h`, `67h`,
combined forms, DS/BP-SS/CS/ES/FS/GS sources, a 32-bit SIB SS-default
source, and both-form source-limit no-IDT terminal boundaries. `80186` rejects the escape; on 8086 byte `0F` remains the
legacy POP CS encoding and is deliberately outside this query slice. Real mode
and ordinary VM86 reject without LAR/LSL publication; `LOCK` rejects. Pending
IRQ0 after successful LAR, successful LSL, and selector-invalid ZF=0 delivery
uses the protected handler/frame/PIC contract. No production or shared-helper
change was required. VERR/VERW, segment loads, and broader descriptor behavior
remain outside S57.

`M5:T316:S57:LAR-LSL:OK`
