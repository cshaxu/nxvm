# T401 S1 Four-Profile CPU Audit Ledger

`M5:T401:S1:CPU-AUDIT-LEDGER-FROZEN`

## Coverage Universe

The audit source graph has four finite decoder spaces: 256 primary opcode
slots, 256 `0F` opcode slots, eight FPU escape opcode families, and the
prefix/lexeme path that selects their byte layout.  For each space, every
reachable profile/form row will record CPU/profile/form, Intel primary-manual
requirement, current shared owner, timing disposition, test coverage,
conclusion, and repair or transfer.  This ledger is an inventory and does not
claim complete CPU correctness, physical timing, or DeskPro L3.

Primary authority for the first batch is Intel 80286 Programmer's Reference
Manual order 210498, system-control instruction dictionary and Appendix B,
and Intel 80386 Programmer's Reference Manual order 230985, Chapter 17 and
Appendix A. The 1987 80286 manual and 1986 80386 manual are retained as
read-only reference scans at [80286 PRM](https://bitsavers.org/components/intel/80286/210498-005_80286_and_80287_Programmers_Reference_Manual_1987.pdf)
and [80386 PRM](https://bitsavers.org/components/intel/80386/230985-001_80386_Programmers_Reference_Manual_1986.pdf).

## Batch A: `0F 00/01/02/03/06` System-Control Forms

| Forms | Intel/profile requirement | Shared owner and tests | Conclusion / timing disposition |
| --- | --- | --- | --- |
| `0F 00 /0,/1` SLDT/STR | 80286+; protected-mode fixed `r/m16` visible-selector stores; real mode #UD. | `INS_0F_00`; `core-machine-dttr-s61-smoke`. | Conforms within the established logical contract; timing remains nonphysical with owner-local existing source ledger. |
| `0F 00 /2,/3` LLDT/LTR | 80286+ protected CPL0 selector loads; descriptor validation commits only after success. | `INS_0F_00`, `_s_load_ldtr`, `_s_load_tr`; DTTR and descriptor-system smokes. | Conforms within accepted selector/cache and busy-bit boundary; task switching transfers separately. |
| `0F 00 /4,/5` VERR/VERW | 80286+ protected selector-access query; result is ZF-only and does not load a segment. | `INS_0F_00`; `core-machine-verr-verw-s58-smoke`. | Conforms in audited query context; broader descriptor architecture remains separate. |
| `0F 01 /0,/1` SGDT/SIDT | 80286+ table-memory stores, six-byte pseudo-descriptor image. | `INS_0F_01`, `_m_write_table_pseudo_descriptor`; SGDT/SIDT smoke. | Conforms for fixed layout, memory-only, access-fault and profile/mode paths; nonphysical timing. |
| `0F 01 /2,/3` LGDT/LIDT | 80286+ table-memory loads; protected CPL/VM86 rejection before source access. | `INS_0F_01`, table loaders; LGDT/LIDT and VM86 smokes. | Conforms within current table-state boundary; physical cycle timing remains nonphysical. |
| `0F 01 /4,/6` SMSW/LMSW | 80286+ fixed `r/m16`; SMSW unprivileged, LMSW CPL0 in protected mode and PE-sticky. | `INS_0F_01`, `_s_load_cr0_msw`; `core-machine-msw-s63-smoke`. | Conforms; shared helper checks CPL before CR0 commit. |
| `0F 02/03` LAR/LSL | 80286+ protected selector queries; invalid selector clears ZF rather than loading state. | `LAR_R32_RM32`, `LSL_R32_RM32`; LAR/LSL smoke. | Conforms in established selector-query boundary. |
| `0F 06` CLTS | 80286+; clears only CR0.TS, real/CPL0 protected legal, user/VM86 rejected. | `CLTS`; `core-machine-clts-s62-smoke`. | Conforms; the current metadata correctly admits 80286 and 80386. |

The metadata gate (`core_machine_cpu_instruction_metadata_get`), the `0F`
dispatch table and all listed handlers agree on profile availability. Reserved
`0F 00 /6,/7` and `0F 01 /5,/7` forms remain explicit #UD. No new production
repair is admitted in Batch A. Existing historical evidence T304, T316,
T318, T319, T321 and T341 supplies the focused form/mode/fault regressions;
this batch re-audits their current source ownership rather than silently
reaccepting their task status.

## Transfer

T401 S2 repaired the reproducible `0F 25` mismatch: Intel 80386 Appendix A reserves the opcode and dispatch already emitted #UD, while both metadata and lexical ModR/M classification had admitted it through an over-broad `0x20..0x26` range. The shared range now selects only `0F 20..24` and `0F 26`. `core-machine-protected-ud-delivery-s1-smoke` proves invalid metadata, unavailable lexeme and protected #UD delivery for `0F 25`, then confirms all six adjacent CR/DR/TR opcodes still scan. Descriptor-system, debug-MOV and selector regressions pass unchanged. This is a logical decode correction; timing remains nonphysical.

Batch B then covers the remaining `0F` map, beginning with control/debug/test-register,
bit-operation, conditional branch/set and 80386-only forms. FPU escape stays
a separately classified coverage family. Every successful reachable form also
retains an exact row, formula, range model, or explicit nonphysical timing
record; emulator implementations may only cross-check a requirement already
fixed from Intel material.
