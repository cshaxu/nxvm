# M5 T302 S1: Operand, Address, And Stack Admission Audit

## Scope And Authority

This S1 record freezes the 80386 `66h`/`67h` operand, address, and stack
family before implementation. The authority is the *Intel 80386 Programmer's
Reference Manual* (1986): Sections 3.6, 8.2, 16.1, 17.1, and 17.2, plus the
individual `PUSH`, `POP`, `PUSHA`, `POPA`, `PUSHF`, `POPF`, `ENTER`, `LEAVE`,
`MOVS`, `CMPS`, `STOS`, `LODS`, `SCAS`, `INS`, `OUTS`, and `REP` entries.

Read-only comparison used Bochs 2.6 `cpu/fetchdecode.cc`, `cpu/resolver.cc`,
`cpu/stack16.cc`, `cpu/stack32.cc`, and `cpu/string.cc`, and PCjs 2.00.0
`machines/pcx86/modules/v2/cpux86.js`, `x86mods.js`, `x86ops.js`, and
`x86help.js`. No reference source was copied. The references agree with Intel
on separate operand-, address-, and stack-address-size attributes; SIB/default
segment selection; and per-element string/REP state. No bridge is required.

## Current Matrix

| Family / current path | Classification | T302 contract and focused proof |
| --- | --- | --- |
| Prefix state: `_GetOperandSize`, `_GetAddressSize`, `PREFIX_OprSize`, `PREFIX_AddrSize` | In-scope correction | Keep the single decoder and 80386 profile rejection. Prove default-size XOR override in 16- and 32-bit CS, including repeated prefixes and no prefix-state leakage between instructions. |
| Instruction fetch: `_kdf_code`, `_s_read_cs` | In-scope correction | Prove instruction-byte fetch, immediate/ModRM/SIB fetch, CS limit, and 16/32 code-offset wrapping before decode-side state commits. Control-transfer targets remain T303. |
| ModRM/SIB EA: `_kdf_modrm`, `_d_modrm`, `_d_modrm_ea`, `_d_moffs` | In-scope correction | Cover all 32-bit base/index/scale forms, no-index and no-base SIB encodings, signed `disp8`, `disp32`, address wrap, DS versus SS default selection, and segment override. Existing code is an implementation base, not evidence of all forms. |
| General data accesses: `_m_read_rm`, `_m_write_rm`, `_m_read_logical`, `_m_write_logical` | In-scope correction | Prove operand width independent from address width, data-segment limits and expand-down behavior, and pre-write failure preservation. Paging policy remains deferred even where the shared route is exercised with paging disabled. |
| Single `PUSH`/`POP` and r/m forms: `_kec_push`, `_kec_pop`, opcode handlers | In-scope correction | Cover 16/32 operand width crossed with 16/32 SS stack-address size, wrap and SS limit checks, source-read-before-stack-write, POP destination commit, and the post-pop ESP address rule. |
| `PUSHA`/`POPA`, `PUSHF`/`POPF`, `ENTER`/`LEAVE` | In-scope correction | Cover saved original SP/ESP, ignored POPA/POPAD SP slot, flag masks allowed at CPL0, frame nesting, final allocation probe, and each instruction's defined fault/commit boundary. Privilege and V86 flag policy are deferred. |
| Memory strings: `MOVS`, `CMPS`, `STOS`, `LODS`, `SCAS`, REP helpers | In-scope correction | Cover byte/word/dword element width, 16/32 index and count selected by address size, DF, source override versus fixed ES destination, REP/REPE/REPNE termination, wrap, limit, and one failing iteration after prior committed iterations. |
| I/O strings: `INS`, `OUTS` | In-scope correction limited to size/address/REP | Reuse the T300 single port path. Prove 16/32 index/count and operand-width selection without changing port ownership, DMA, privilege, or controller timing. |
| Existing narrow probes: `core_machine_real_mode_386_address_smoke`, `core_machine_real_mode_386_rep_cmps_smoke` | Retained, insufficient | Retain as regressions. They prove a narrow real-mode address path and `REP CMPS` behavior only; they do not close protected 32-bit defaults, cross widths, stack, fetch, or all string forms. |
| Control transfers and loop-family | Deferred to T303 | Near/far branch targets, return frames, and relative control offsets use related helpers but have their own code-limit and commit contract. |
| Descriptor/system, interrupt/exception, privilege, paging, V86 | Deferred to their Queue families | Do not use T302 to alter table/system state, delivery, privilege rules, paging policy, or virtual-8086 behavior. |

## Frozen Semantics

- `66h` changes operand size only; `67h` changes effective-address, implicit
  string index/count, and moffs address size only. SS stack-address size comes
  from the SS cache and is independent of both prefixes.
- In 32-bit addressing, SIB index value four means no index; base five with
  `mod=00` means no base plus displacement. EBP/ESP-based forms select SS by
  default, all other ordinary forms select DS; an admitted segment override
  replaces that source selection. String destination remains ES.
- Address-size selects SI/DI/CX or ESI/EDI/ECX and their wrapping. Operand-size
  selects word/dword values and string element step. Stack operand width and
  stack-address width are separately crossed in every stack probe.
- Before a non-repeating instruction commits a destination, all required
  source, target, limit, and write checks must have completed. A REP operation
  may retain completed earlier elements; a failing current element must not
  apply that iteration's index/count/destination commit. The exact multi-push
  and `ENTER` fault boundary is tested against the Intel-defined access order,
  not assumed to be whole-instruction rollback.

## S2+ Batches

| Subtask | Bounded implementation | Focused synthetic probe and stop boundary |
| --- | --- | --- |
| S2 | Prefix, instruction-fetch, ModRM/SIB, moffs, and general-data route. | New T302 probe establishes all CS/DS/SS default and override forms, `disp8` sign, SIB exceptions, width crossing, code/data limits, and no destination change on decode/access fault. Stop before stack opcodes. |
| S3 | Stack primitive and frame opcode forms. | The same focused probe adds `PUSH`/`POP`/r/m, `PUSHA`/`POPA`, `PUSHF`/`POPF`, `ENTER`/`LEAVE` across 16/32 SS and operand widths, including source/destination and defined partial-commit failures. Stop before string/REP. |
| S4 | Memory and I/O string/REP address and size forms. | Add byte/word/dword, DF, segment selection, REP termination/count, wrap/limit, and failed-current-iteration cases. Reuse ports only through the existing core registry; stop before timing, privilege, or paging changes. |
| S5 | Family sweep and closure evidence. | Re-run all T302 focused probes plus retained address/REP probes, current gates, artifact cutover, and the one post-family owner-controlled Setup observation. Any first Setup issue becomes a later explicit task, not an S5 expansion. |

## Similar-Issue Sweep And Known Limits

The S1 production sweep used:

```powershell
rg -n "66h|67h|operand.?size|address.?size|PUSHA|POPA|PUSHF|POPF|ENTER|LEAVE|REP|SIB|ModRM|stack" src/core/machine tests/machine CMakeLists.txt
rg -n "^static C_VOID (_kdf_|_d_modrm|_d_moffs|_m_read|_m_write|_kec_push|_kec_pop|PUSH|POP|PUSHA|POPA|PUSHF|POPF|ENTER|LEAVE|MOVS|CMPS|STOS|LODS|SCAS|INS|OUTS)" src/core/machine/cpu_instructions.c
```

All production hits are classified in the matrix. Existing 32-bit decoder,
segment-access, and stack primitives are retained as one core-owned route but
are not yet family-complete. The only known uncertainty is the exact observable
state on a late failing multi-access stack instruction; it is bounded to S3
through Intel access-order probes. It does not require a reference bridge and
does not justify a new executor, session, public interface, or host shortcut.
