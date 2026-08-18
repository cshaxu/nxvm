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

T401 S2 repaired the reproducible `0F 25` mismatch: Intel 80386 Appendix A reserves the opcode and dispatch already emitted #UD, while both metadata and lexical ModR/M classification had admitted it through an over-broad `0x20..0x26` range. The shared range now selects only `0F 20..24` and `0F 26`. `core-machine-protected-ud-delivery-s1-smoke` proves invalid metadata, unavailable lexeme and protected #UD delivery for `0F 25`, then confirms all six adjacent CR/DR/TR opcodes still scan. Descriptor-system, debug-MOV and selector regressions pass unchanged. The full current gate then passed 285/285 tests on 2026-08-17. This is a logical decode correction; timing remains nonphysical.

T401 S3 adds the `M5:T401:S3:0F-METADATA-MATRIX:OK` regression: all 256
secondary-opcode slots are checked against the current Intel-derived minimum
profile/reserved classification at the metadata owner, and `0F BA` separately
proves `/0`--`/3` reject while `/4`--`/7` admit. This is deliberately a
metadata/profile inventory proof only; dispatch execution, operand semantics,
fault atomicity and timing remain assigned to their form-level tests and
ledger rows rather than inferred from this matrix.
## S5 Lexical-Coherence Disposition

S5 reconciled the lexical rules for prefix availability/size presence, 8086
`0F` POP CS versus 80186 rejection/80286 escape, primary and secondary
metadata gates, ModR/M/SIB/displacement layout, immediate widths, reserved
primary groups, memory-only forms, standard CR/DR/TR register-only forms and
the selected original-80386 CR-MOV MOD quirk. Each repaired shared rule has a
focused marker and a 285/285 current-gate run. FPU ESC and WAIT remain a
separate external-coprocessor family: their byte layout is retained by the
scanner, but availability and operation result depend on the explicit FPU
profile/provider and CR0 `MP/EM/TS` state. LOCK remains intentionally
unavailable to the timing-preview capability, not reclassified as #UD.

No further source-complete prefix/ModR/M/SIB/displacement/immediate mismatch
was found in this sweep. The next receiver audits FPU/WAIT profile/provider
classification without inventing x87 arithmetic, physical timing or a
DeskPro-L3 claim.
## S5 Lexical Repair: Original 80386 CR-MOV MOD Quirk

The selected DeskPro configuration already records the original 80386 fact
that `MOV r32,CRx` and `MOV CRx,r32` use the `r/m` bits even when `MOD` is not
`11b`. Runtime decoding and timing eligibility already consume that immutable
machine option, but the non-mutating execution preview delegated to the public
generic lexical scanner and consumed an invented SIB/displacement. A private
scanner option now serves only the bound execution-preview context; the public
scanner retains the standard register-only 80386 layout and API. The focused
preview regression proves a generic scanner rejects `0F 20 05 disp32`, as it
would every non-register CR/DR/TR form, while the enabled original-80386
option recognizes the documented CR form as three bytes. This implements
neither a new machine profile nor a physical timing claim.
## S5 Lexical Repair: Memory-Only ModR/M Forms

The runtime decoder rejects register ModR/M forms for `BOUND`, `LEA`, `LES`,
`LDS`, `FF /3` and `/5` far CALL/JMP, `0F 01 /0`--`/3` table-memory forms,
and `0F B2/B4/B5` LSS/LFS/LGS. The preview scanner had decoded their byte
layout but did not retain the memory-only constraint. It now rejects the
register forms before publishing a lexeme, while the regression preserves one
valid memory form for each owner and reports
`M5:T401:S5:LEXEME-MEMORY-FORMS:OK`. This is decoder/preview coherence only;
privilege, descriptor, operand fault and timing evidence remain with the
existing form owners.
## S5 Lexical Repair: Primary ModR/M Groups

Intel's primary group encodings reserve `8F /1`--`/7`, `C6/C7 /1`--`/7`,
`F6/F7 /1`, `FE /2`--`/7` and `FF /7`. Their runtime owners already reach
`#UD`, but the timing-preview scanner had only recognized the opcode byte and
therefore advertised those forms as available. The scanner now owns the same
small group-validity classification before reporting a lexeme. The regression
rejects every listed reserved representative and retains valid POP, TEST/NOT,
INC, PUSH plus `C6/C7 /0` immediate layouts with marker
`M5:T401:S5:LEXEME-PRIMARY-GROUPS:OK`. Memory-only and privilege-specific form
rules remain separate lexical audit rows; this change does not make a timing or
physical-L3 claim.
## S5 Lexical Repair: 8086 `0F` POP CS

The runtime `INS_0F` route retains the historical 8086 one-byte `POP CS`
meaning, while 80186 rejects `0F` and 80286+ uses it as the secondary opcode
escape. The lexical scanner had instead treated every profile's `0F` as that
escape and therefore rejected the valid 8086 form when no second byte was
present. T401 S5 makes only non-8086 profiles enter the secondary-opcode scan.
The focused regression proves a one-byte, one-component, available preview on
8086 and rejection on 80186 with marker `M5:T401:S5:LEXEME-8086-POP-CS:OK`.
This is a shared decoder/preview classification correction; it does not claim
POP CS semantic breadth or physical timing.
## S4 Primary-Map Repair: `F1`

Intel 80386 PRM Appendix A does not assign primary opcode `F1`. Current
execution already routed it to `UndefinedOpcode`, and the retained protected
#UD smoke exercised that route, but the shared primary metadata default and
lexeme scanner had still advertised it as available. T401 S4 makes `F1`
invalid at the metadata owner, which makes the lexical preview unavailable
before it can publish a timing shape. The nearby `82` and `D6` reserved slots
were included in the same primary invalid-opcode branch and remain invalid.
`core-machine-protected-ud-delivery-s1-smoke` now emits
`M5:T401:S4:F1-METADATA:OK` after proving the metadata/lexeme rejection while
retaining existing protected #UD delivery. This is a decode-classification
repair only; instruction timing remains nonphysical.
## S4 Current Primary-Map Reconciliation

`M5:T401:S4:PRIMARY-METADATA-MATRIX:OK` checks every one of the 256 primary
slots. The current metadata classification is: `82`, `D6` and `F1` reserved;
`D8`--`DF` valid only as 8087 escape entries; 15 80186 additions (`60`--`62`,
`68`--`6F`, `C0`, `C1`, `C8`, `C9`); `63` as the 80286 ARPL addition; and
`64`--`67` as the 80386 FS/GS prefix additions. The remaining 225 slots retain
the 8086 baseline classification, including `0F`'s documented pre-80286 POP
CS behavior. This is a metadata/profile inventory check; ModR/M group
validity, prefix legality, execution semantics, faults and timing remain
separately proven or transferred by their form-level rows.

The S4 source sweep confirms that `ExecIns` applies this classification before
the primary dispatch table. The primary matrix therefore guards both the three
reserved slots and future profile-gate drift, while preserving the separately
classified FPU escape route. No additional primary metadata/dispatch mismatch
was found after the `F1` repair.
## S3 Current `0F` Map Reconciliation

The current source matrix contains 66 metadata-valid secondary opcode slots:
five 80286 slots/groups (`00`, `01`, `02`, `03`, `06`) and 61 80386 slots/groups.
The table below is a source/metadata classification, not a claim that every
valid instruction form has one uniform timing model.

| Current slots | Profile / form disposition | Current form-level evidence or transfer |
| --- | --- | --- |
| `00 /0`--`/5`, `01 /0`--`/6`, `02`, `03`, `06` | 80286+; omitted group extensions reject. | Batch A above; DTTR, descriptor-system, LAR/LSL and CLTS focused tests. Nonphysical timing retained. |
| `20`--`24`, `26` | 80386 CR/DR/TR moves; `25` is reserved and rejects. | T401 S2 plus debug-MOV and descriptor-system tests; CR/DR/TR semantics retain their distinct existing evidence boundaries. |
| `80`--`8F` | 80386 near conditional branches. | T303 control-transfer matrix; selected target/fault forms are proven, timing remains at its existing nonphysical owner. |
| `90`--`9F` | 80386 SETcc r/m8 forms. | T310 S3 setcc smoke covers declared condition, register/memory, prefix and pre-fault publication rows. |
| `A0`, `A1`, `A8`, `A9` | 80386 FS/GS push/pop. | T316 S23 FS/GS stack matrix. |
| `A3`, `AB`, `B3`, `BB`, `BA /4`--`/7` | 80386 BT/BTS/BTR/BTC; `BA /0`--`/3` reject. | T310 S5 bit-test matrix and S3 metadata regression. |
| `A4`, `A5`, `AC`, `AD` | 80386 SHLD/SHRD immediate/CL forms. | T310 S6 double-shift matrix. |
| `AF`, `BC`, `BD`, `B6`, `B7`, `BE`, `BF` | 80386 IMUL2, BSF/BSR and MOVZX/MOVSX. | T310 S8/S7/S4 form matrices. |
| `B2`, `B4`, `B5` | 80386 LSS/LFS/LGS, memory-only. | T316 S24 matrix; selected descriptor/fault boundary only. |
| Every other slot, including `09`, `25`, `30`, `32`, `A2`, `AA`, `B0`, `B1`, `B8`, `B9`, `C0`--`FF` | Reserved/later/unsupported for the selected processors; metadata gate rejects before dispatch, even where a later-CPU handler name remains initialized. | S3 256-slot metadata regression plus T401 S2 `25` execution delivery proof. No later-CPU feature is admitted. |

The audit also checks that `ExecIns` applies the metadata/profile gate and
`INS_0F` repeats it before the initialized table route. Consequently, a
later-CPU handler present in the table is not treated as an implemented 80386
instruction. This matrix reconciles the current source graph with the retained
T303/T304/T310/T316 proofs; it exposes no additional metadata/profile mismatch
beyond the repaired `0F 25`. Remaining CPU work is per-form semantic and timing
audit, not an unclassified secondary-opcode hole.
Batch B then covers the remaining `0F` map, beginning with control/debug/test-register,
bit-operation, conditional branch/set and 80386-only forms. FPU escape stays
a separately classified coverage family. Every successful reachable form also
retains an exact row, formula, range model, or explicit nonphysical timing
record; emulator implementations may only cross-check a requirement already
fixed from Intel material.

## S6 WAIT/ESC Profile and Provider Disposition

S6 re-ran the CPU-side external-coprocessor matrix against the retained Intel
8086/80286/80386 coprocessor-interface baseline and current owners.  It found
no shared CPU-interface discrepancy to repair.  The following result is an
explicit interface classification, not a numerical-x87, physical-timing or
DeskPro-L3 claim.

| CPU/profile or condition | `9B` WAIT/FWAIT | ESC `D8`--`DF` | Owner and focused proof |
| --- | --- | --- | --- |
| 8086, 80186, 80286, 80386; FPU `none`; CR0 clear | Consumed with no provider result. | All eight escape families consume their legal byte forms without manufacturing an x87 result. | `WAIT`, `FPU_ESCAPE`, `core-machine-fpu-interface-s65-smoke`. |
| Any selected CPU; `EM` or `TS` for ESC | N/A to the ESC predicate. | CPU raises `#NM` before provider dispatch. | `FPU_ESCAPE`; S65 vector-7/restart delivery matrix. |
| 80386; `TS` and `MP` for WAIT | CPU raises `#NM`. | N/A. | `WAIT`; S65 vector-7/restart delivery matrix. |
| Pending configured provider exception | CPU observes provider pending state and raises `#MF`. | N/A. | `WAIT`, `core_machine_fpu_wait_pending`; S65 vector-16 frame matrix. |
| Configured 8087 subset | N/A unless pending exception. | Existing bounded 8087 operations route to the provider. | `FPU_ESCAPE`, retained `core-machine-fpu-8087-smoke`. |
| Configured 80287/80387 | N/A unless pending exception. | Explicit unsupported-provider fault boundary; no arithmetic is invented. | `core_machine_fpu_escape_dispatch`; S65 unsupported-provider rows. |
| Preview/lexeme | The scanner preserves structural byte layout. | The scanner preserves escape plus ModR/M layout; provider result is intentionally not inferred. | `core_machine_cpu_instruction_lexeme_scan`; S5 disposition. |

The dispatch order is coherent with the matrix: metadata admits `D8`--`DF` as
8086-plus external-coprocessor forms; `FPU_ESCAPE` validates the escape form,
then applies `EM/TS` before provider dispatch; `WAIT` applies `TS && MP`
before pending-provider `#MF`.  This order also explains why a provider-free
ESC is a successful CPU-side consume rather than `#UD`, and why a structural
preview remains available without a provider argument.  Existing 80386
`66h`/`67h` and LOCK/pre-386 prefix rejection coverage remains authoritative;
S6 made no decoder or ABI change.

Focused verification on 2026-08-17 passed:

- `current.core-machine-fpu-escape-smoke` and
  `current.core-machine-fpu-interface-s65-smoke`: 2/2.
- `core-machine-cpu-fpu-profile-smoke`:
  `M5:T154:S1:CPU-FPU-PROFILES:OK`.
- `core-machine-cpu-fpu-profile-closure-smoke`:
  `M5:T158:S1:CPU-FPU-METADATA-CLOSURE:OK`.

No full current-gate rerun is required by this packet because S6 made no
production repair.  Any future numerical x87/provider admission must receive
its own bounded task, source/reference tier and operation matrix; it cannot be
inferred from this CPU-interface audit.
