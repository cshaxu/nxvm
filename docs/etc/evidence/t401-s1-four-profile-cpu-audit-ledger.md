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
before it can publish a timing shape. The nearby `D6` reserved slot
remains invalid. `82` was later reclassified by S7 as the Group-1 byte alias
after the original-8086/manual and reference-implementation reconciliation.
`core-machine-protected-ud-delivery-s1-smoke` now emits
`M5:T401:S4:F1-METADATA:OK` after proving the metadata/lexeme rejection while
retaining existing protected #UD delivery. This is a decode-classification
repair only; instruction timing remains nonphysical.
## S4 Current Primary-Map Reconciliation

`M5:T401:S4:PRIMARY-METADATA-MATRIX:OK` checks every one of the 256 primary
slots. The current metadata classification is: `D6` and `F1` reserved; `82`
is the Group-1 byte alias of `80`; `D8`--`DF` valid only as 8087 escape
entries; 15 80186 additions (`60`--`62`,
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
## S7 Group-1 Immediate Disposition And `82h` Repair

S7 audited every Group-1 selector (`/0` ADD, `/1` OR, `/2` ADC, `/3` SBB,
`/4` AND, `/5` SUB, `/6` XOR and `/7` CMP) for `80h`, `81h`, `82h` and
`83h`.  The existing `80h`/`81h`/`83h` handlers, operand-size branches,
flag/transaction owners and preview layout agree with the retained arithmetic
matrices.  `83h` uses the existing 12-bit/20-bit helper mode solely to
sign-extend `imm8` to the active 16-/32-bit operand.  LOCK register forms are
rejected by `_d_modrm` after prefix classification, while memory-destination
forms retain the existing shared path.

S7 found and repaired one source/reference discrepancy: the shared primary
metadata and dispatch had classified `82h` as `#UD`.  Intel's 1979
[8086 Family User's Manual](https://bitsavers.org/components/intel/8086/9800722-03_The_8086_Family_Users_Manual_Oct79.pdf)
shows the `100000sw` Group-1 encoding, including its byte `82h` combination.
Later material is inconsistent about whether that redundant encoding is
reserved.  Therefore the selected 80386 compatibility result is explicitly
**reference-derived**, cross-checked against the read-only local PCjs x86
implementation, which maps `80h` and `82h` to the same byte-immediate Group-1
handler for all processors.  No third-party source was imported or derived.

The shared repair makes `82h` metadata-valid and dispatches it to `INS_80`.
The lexical scanner already supplied the matching byte-immediate layout, so no
new scanner rule or Core/VM interface was needed.  The S7 matrix executes all
four CPU profiles, all four Group-1 opcode bytes, all eight ModR/M selectors,
and validates `80h`/`82h` byte versus `81h` word and `83h` sign-extended byte
lengths.  The preview smoke additionally proves `82h` is a three-byte,
three-component lexeme across the four profiles.  The previous all-profile
`82h` invalid assertion was removed; `D6h` and `F1h` remain the primary
reserved slots.

Focused proofs on 2026-08-17 passed:

- `current.core-machine-inc-dec-smoke` with
  `M5:T401:S7:GROUP1-PROFILE-MATRIX:OK`.
- `current.core-machine-cpu-timing-preview-smoke`.
- `current.core-machine-protected-ud-delivery-s1-smoke`, including the
  corrected primary metadata matrix.

This is a CPU decode/compatibility repair only.  It makes no physical timing,
board or DeskPro-L3 claim.

The full-gate follow-up also exposed one stale legacy-ALU expectation that still
classified `82h` as reserved.  S7 removed that expectation and reran the
legacy-ALU plus S7 focused set: 4/4 passed on 2026-08-17.  The replacement
full current gate is the acceptance authority for this production repair.

## S8 Group-2 Shift/Rotate Count Reconciliation

S8 audited the `D0h`--`D3h` one/CL-count and `C0h`/`C1h` immediate-count
Group-2 forms across all eight ModR/M extensions.  `/0`--`/5` and `/7`
dispatch to the shared rotate/shift helpers; `/6` remains #UD before a write.
`C0h`/`C1h` remain unavailable on 8086 and available from 80186; the retained
metadata, lexeme and profile-gate paths already match that form boundary.
The owner is the shared Core helper layer, not a machine profile or VM path.

Intel's [1985 iAPX 86/88/186/188 User's Manual](https://www.bitsavers.org/components/intel/8086/210912-001_iAPX_86_88_186_188_Users_Manual_1985.pdf)
establishes the 8086 CL-count form and the 80186 immediate-count addition.
Intel's [80286/80287 Programmer's Reference Manual](https://bitsavers.org/components/intel/80286/210498-005_80286_and_80287_Programmers_Reference_Manual_1987.pdf)
specifies that only the low five CL bits are used.  The retained implementation
had applied that five-bit mask to every profile, incorrectly making 8086
`CL=33` behave as count one.  The distinct 8086 full-eight-bit count is
**reference-derived** where the combined manual is not explicit: the read-only
PCjs CPU test corpus distinguishes 8086/8088 full-CL execution from 80186/188
and later low-five-bit masking.  No external code was imported.

The bounded repair introduces one shared count-normalization helper: 8086
receives the supplied eight-bit count; 80186, 80286 and 80386 receive its low
five bits.  RCL/RCR retain their existing post-mask ring reduction on the
later profiles, while 8086 performs its full count iteratively.  This leaves
all timing nonphysical and changes no Core/VM interface.

`M5:T401:S8:GROUP2-CL-PROFILES:OK` executes `D2h` with every `/0`--`/7`
extension and `CL=33` on all four profiles, proves the divergent count result,
and retains the `/6` atomic #UD proof.  Focused regressions passed on
2026-08-17: rotate, legacy-ALU, CPU-profile-gate, 80286 timing-ledger and CPU
preview (5/5).  The full current gate then executed 285/285 tests with no failure marker on 2026-08-17.
## S9 Group-3 Unary IMUL Sign-Extension Repair

S9 began the `F6h`/`F7h` Group-3 audit and reproduced a shared single-operand
IMUL flag defect.  Intel's 8086/80286/80386 manuals specify CF and OF clear
when the upper half of the product is the sign extension of the lower half.
The existing byte path compared a masked unsigned product with a signed
extension, so `AL=-1; IMUL r/m8(1)` wrongly retained CF/OF; the analogous
word/dword predicates were normalized at the same shared owner.  This is an
original-source semantic rule, not a reference-derived behavior.

The repair compares each full product and its retained low half as the matching
signed width.  `M5:T401:S9:IMUL-SIGN-EXTENSION-PROFILES:OK` covers byte and
word forms on 8086/80186/80286/80386 plus the 80386 dword form, verifies the
result halves and CF/OF clear outcome, and preserves the pre-80386 16-bit
register surface.  Focused inc-dec, preview, CPU-profile-gate and 80286
instruction-timing-ledger tests passed 4/4 on 2026-08-17.  One full-gate run
had an unrelated DOS-keyboard timeout which passed twice in isolation; the
replacement full current gate then executed 285/285 tests with no failure
marker on 2026-08-17.
`M5:T401:S9:GROUP3-PREVIEW-PROFILES:OK` adds the complete decoder/preview
matrix: four profiles, both Group-3 opcode bytes and all eight extensions.  It
proves `/1` unavailable, `/0` immediate-size selection (byte for `F6h`;
active operand size for `F7h`) and the two-byte forms for `/2`--`/7`.  The
existing inc-dec execution, multiply/divide fault and attribute matrices retain
the state, FLAGS, transaction and #DE coverage; no new production repair or
Core/VM boundary is introduced by this test-only completion slice.

## S10 Group-4/5 Form Inventory

S10 began the `FEh`/`FFh` Group-4/5 audit.  The shared dispatch matches the
Intel form partition: `FEh /0`--`/1` are byte INC/DEC and `/2`--`/7` are #UD;
`FFh /0`--`/2`, `/4` and `/6` admit r/m forms; far CALL/JMP `/3` and `/5`
require memory; `/7` is #UD.  The existing control-transfer, GPR-push/pop,
prefix and protected-#UD tests retain the execution, stack, memory-only,
LOCK, transaction and delivery proofs.

`M5:T401:S10:GROUP45-PREVIEW-PROFILES:OK` adds four-profile form inventory for
all extensions and both direct-register and 16-bit-addressed memory encodings.
It verifies the exact availability partition and the two- versus four-byte
preview layouts.  This is source-backed Intel form classification; it makes no
physical timing or DeskPro-L3 claim and introduces no Core/VM interface change.
The execution sweep also adds a real-mode `FF /5` ptr16:16 source boundary
probe at `DS:FFFEh`: the offset is read at `FFFEh`, and the selector is read at
the consecutive `10000h` byte address rather than wrapped to `0000h`. Intel's
form definition supplies the pointer width; the exact cross-boundary access
interpretation is **reference-derived**, corroborated without import by the
read-only local PCjs `fnJMPFdw` handler, which reads the selector at its decoded
linear effective address plus the operand size. NXVM's shared logical-memory
route agrees: `INS_FF` advances the decoded pointer offset by two and submits
that second read to the same checked access helper. The probe executes on the
8086 and 80186 real-mode profiles and requires the resulting `0200:0000h`
target, so a 16-bit wrap reads the deliberately different bytes at `0000h` and
fails. No third-party source was copied; no production code or Core/VM
interface changed.
`M5:T401:S10:GROUP45-INC-DEC-PROFILES:OK` extends the existing FE/FF
increment/decrement execution matrix from an 80386-only run to 8086, 80186,
80286 and 80386. It executes the byte (`FE /0,/1`) and word (`FF /0,/1`)
register and direct-memory forms on every profile; the 80386-only operand-size
prefixed dword forms remain gated to that profile. Each row checks result,
architecturally defined INC/DEC flags and carry preservation. This is an
original-source form/flag rule and test-only coverage expansion; it makes no
physical timing or DeskPro-L3 claim and changes no Core/VM interface.
`M5:T401:S10:GROUP5-PUSH-RM-PROFILES:OK` extends the retained `FF /6`
PUSH-r/m execution matrix to 8086, 80186, 80286 and 80386. It covers register,
DS-default and SS-default 16-bit memory sources, checks the source value,
16-bit stack update, stack image, flags and non-destination registers, and
retains the 80386-only `67h` address-size cases under their existing profile
gate. Existing protected source- and stack-fault cases remain the transaction
proof for this shared stack owner. This is original-source opcode/form coverage,
not a physical timing or DeskPro-L3 claim; no production or Core/VM interface
changed.
`M5:T401:S10:GROUP5-CONTROL-PROFILES:OK` expands the existing unprefixed
16-bit real-mode Group-5 control paths from 8086/80186 to 8086, 80186, 80286
and 80386. It now executes far-indirect `FF /3,/5`, near-indirect `FF /2,/4`
and retains the reserved `FF /7` #UD proof on every profile. The near forms
now include both register and direct-memory targets; `/3` and `/5` register
encodings are also verified as #UD on every profile. The tests verify normal
target/return state, flags and stack preservation, plus the existing memory-only
and atomicity coverage. This is original-source opcode/profile
coverage; protected descriptor/privilege behavior remains with the retained
T303-focused probes, and neither physical timing nor any Core/VM interface is
changed.
## S10 Acceptance

S10 closes the `FEh`/`FFh` Group-4/5 matrix. The four-profile preview sweep
classifies every extension, direct-register versus memory form and memory-only
restriction. The execution regressions cover FE/FF INC/DEC result/FLAGS/carry,
FF /6 push-r/m stack behavior, FF /2,/4 near and FF /3,/5 far control paths,
source-boundary behavior and every invalid/memory-only rejection. Original
Intel form semantics are the primary authority; only the 16-bit far-pointer
cross-boundary interpretation is explicitly reference-derived from the local
read-only PCjs comparison. No third-party source was imported.

No production discrepancy remained after the bounded sweep. Focused INC/DEC,
push/pop and control-transfer probes passed on 2026-08-17; the complete current
gate then ran 285/285 tests with no failure marker. This accepts CPU semantic
and nonphysical timing-baseline evidence only. It does not claim physical
instruction, board, ISA, DMA, firmware, or DeskPro L3 timing; those boundaries
remain with their retained DeskPro receivers.
## S11 Primary Register INC/DEC

S11 audits the distinct primary register INC/DEC opcode family `40h`--`4Fh`.
Intel's 8086/80286/80386 instruction definitions retain these eight INC and
eight DEC register forms on all four selected profiles; 80386 `66h` selects the
dword operand form. The current decoder/dispatch and shared arithmetic owner
match that partition. The prior regression was incomplete evidence: it ran
only INC and only on 80386.

`M5:T401:S11:PRIMARY-INC-DEC-PROFILES:OK` now executes every opcode for 8086,
80186, 80286 and 80386, checks 16-bit result retention for each register,
INC/DEC defined FLAGS and carry preservation, and adds 80386 dword boundary
forms with `66h`. `M5:T401:S11:PRIMARY-INC-DEC-PREVIEW-PROFILES:OK` scans all
sixteen one-byte forms on every profile and both prefixed 80386 representatives.
The result is a test-only evidence correction: no shared Core defect, external
source import, physical timing claim or Core/VM interface change was found.
S11 also closes the unavailable-prefix row: `66h 40h` and `66h 48h` execute as
atomic `#UD` on 8086, 80186 and 80286, preserving EAX, EFLAGS and EIP. This
extends the former single-80286 INC probe to both primary opcode halves and all
pre-80386 selected profiles. The 80386 dword cases are covered by the normal
execution and preview matrix.
## S11 Acceptance

S11 closes the `40h`--`4Fh` primary register INC/DEC matrix: every selected
profile and opcode has execution and preview coverage, 80386 dword prefix
forms are distinguished, and every pre-80386 `66h` representative is proven
atomic #UD. No production discrepancy remained. Focused INC/DEC and preview
probes passed; the current-gate CTest log on 2026-08-17 records all 285/285
tests through `current.vm-ata-pio-dos-smoke` with no failure marker. This is
CPU semantic/nonphysical timing-baseline acceptance only, not a physical or
DeskPro L3 claim.
## S12 Accumulator XCHG

S12 audits `90h`--`97h`: Intel defines `90h` as the accumulator self-exchange
(NOP alias) and the remaining bytes as accumulator exchanges with CX, DX, BX,
SP, BP, SI and DI. The retained XCHG smoke already executes all eight opcodes
on 8086, 80186, 80286 and 80386, including FLAGS preservation, 80386 dword
forms, pre-80386 `66h` #UD atomicity, LOCK rejection and the IRQ boundary.

`M5:T401:S12:ACCUMULATOR-XCHG-PROFILES:OK` names that complete execution
matrix. `M5:T401:S12:ACCUMULATOR-XCHG-PREVIEW-PROFILES:OK` adds the previously
missing decoder-preview sweep for all eight one-byte forms across four profiles
and 80386 `66h` representatives. This is an original-source semantic/form
classification and test-only evidence correction: no production discrepancy,
external-source import, physical timing claim or Core/VM interface change was
found.

## S12 Acceptance

S12 closes 90h--97h accumulator XCHG. The all-profile execution matrix covers the NOP alias, register pair selection, FLAGS preservation, 80386 dword form, pre-80386 66h #UD atomicity, LOCK rejection and IRQ boundary; the new preview matrix covers every byte and 80386 prefixed representatives. No production discrepancy remained. This is CPU semantic/nonphysical timing-baseline acceptance only, not a physical or DeskPro L3 claim.
## S13 Immediate-Register MOV

S13 audits `B0h`--`BFh`. Intel's 80386 Programmer's Reference Manual MOV
entry identifies `B0+rb` as byte-immediate register MOV and `B8+rw/rd` as the
operand-size-selected word/dword register form; the retained
[80386 MOV timing ledger](t357-s3-80386-instruction-timing-ledger.md) links
those manual rows. This is original-source semantic/form evidence. No
secondary emulator source or firmware asset was imported or used.

`M5:T401:S13:IMMEDIATE-REGISTER-MOV-PROFILES:OK` executes all eight byte and
all eight word forms on 8086, 80186, 80286 and 80386, checking high-byte
selection, destination-only publication, FLAGS preservation and exact EIP.
It additionally executes every 80386 `66h B0h`--`B7h` (legal, byte width
unchanged) and `66h B8h`--`BFh` dword form, then proves `66h B0h` and `66h
B8h` are atomic `#UD` on every pre-80386 profile. The retained LOCK and IRQ
checks cover the family boundary.

`M5:T401:S13:IMMEDIATE-REGISTER-MOV-PREVIEW-PROFILES:OK` scans all sixteen
unprefixed encodings for all four profiles, every 80386 `66h` byte form and
every 80386 `66h` dword form. This closes the previously sampled preview
coverage without a production correction. The tests remain CPU
semantic/nonphysical timing-baseline evidence only: they make no physical
cycle, DeskPro L3, or Core/VM-interface claim.

## S13 Acceptance

S13 closes the `B0h`--`BFh` immediate-register MOV family for the selected
four profiles. Focused GPR-MOV and CPU-preview current-gate tests pass after
the full matrix; no production discrepancy was found. The S13 test assertion
was corrected during the audit before acceptance, so it does not represent a
Core defect. Timing remains the existing explicitly nonphysical source-ledger
disposition.
## S14 Moffs MOV

S14 audits `A0h`--`A3h` moffs MOV. Intel's 80386 Programmer's Reference
Manual MOV entry defines the accumulator read/write directions, byte versus
operand-size-selected word/dword payload, address-size-selected moffs field
and segment-override behavior. The retained [80386 MOV timing
ledger](t357-s3-80386-instruction-timing-ledger.md) links the same original
manual rows. No reference-emulator source, ROM or guest asset was imported.

`M5:T401:S14:MOFFS-MOV-PROFILES:OK` retains all-four-profile unprefixed
execution for each read/write opcode, FLAGS and exact EIP, pre-80386 `66h`/
`67h` atomic #UD, LOCK rejection, segment overrides, protected-limit
nonpublication and IRQ boundary. It adds a full 80386 single-`66h`,
single-`67h` and combined attribute matrix across all four opcodes, proving
byte/word/dword accumulator and memory publication with 16- and 32-bit moffs
addresses. `M5:T401:S14:MOFFS-MOV-PREVIEW-PROFILES:OK` supplies the matching
four-profile lexical matrix.

This is original-source semantic/form evidence and a test-only coverage
correction. The timing disposition remains explicitly nonphysical at the
existing Core timing owner; it makes no physical-cycle, DeskPro L3 or Core/VM
interface claim.

## S14 Acceptance

S14 closes the selected `A0h`--`A3h` moffs MOV family. Focused moffs and CPU
preview current-gate tests pass, with no production discrepancy. Temporary CE
observations during test construction were traced to malformed test address
bytes and removed before acceptance.
## S15 MOVS

S15 audits `A4h`/`A5h` MOVS. Intel's 80386 Programmer's Reference Manual
string-instruction/MOV entries define source-to-ES destination routing,
DF-controlled index progression, operand-size payload width, address-size
index/count selection and REP iteration. This is original-source semantic/form
evidence; no emulator implementation, ROM or guest asset was imported.

`M5:T401:S15:MOVS-PROFILES:OK` retains all-four-profile byte/word execution,
REP count zero/one/multiple behavior, 80386 dword and 32-bit address forms,
DF decrement, source segment overrides, pre-80386 prefix atomic #UD, LOCK
rejection and IRQ behavior. `M5:T401:S15:MOVS-PREVIEW-PROFILES:OK` adds the
complete four-profile lexical matrix for both opcodes, REP and every selected
80386 operand/address attribute combination.

No production discrepancy was found. This remains CPU semantic/nonphysical
timing-baseline evidence only and does not establish physical timing, DeskPro
L3 or a Core/VM interface change.

## S15 Acceptance

S15 closes selected MOVS `A4h`/`A5h` form coverage with focused runtime and
preview proof. The existing Core timing disposition remains explicitly
nonphysical.

## S16 CMPS

S16 audits `A6h`/`A7h` CMPS. Intel's 80386 Programmer's Reference Manual
string-instruction/CMPS entries define source-segment to ES comparison,
FLAGS-only subtraction result, DF-controlled index progression, operand-size
payload width, address-size index/count selection and REPE/REPNE stopping.
This is original-source semantic/form evidence; no emulator implementation,
ROM or guest asset was imported.

`M5:T401:S16:CMPS-PROFILES:OK` retains all-four-profile byte/word execution,
FLAGS outcomes, REPE/REPNE count zero/one/multiple behavior, source-segment
routing, DF decrement, 80386 dword and 32-bit address forms, pre-80386 prefix
atomic #UD, LOCK rejection, protected-limit nonpublication and IRQ behavior.
`M5:T401:S16:CMPS-PREVIEW-PROFILES:OK` adds the complete four-profile lexical
matrix for both opcodes, REPE/REPNE and every selected 80386 operand/address
attribute combination.

No production discrepancy was found. This remains CPU semantic/nonphysical
timing-baseline evidence only and does not establish physical timing, DeskPro
L3 or a Core/VM interface change.
## S16 Acceptance

S16 closes selected CMPS `A6h`/`A7h` form coverage with focused runtime and
preview proof across all four profiles. It covers comparison flags, REPE and
REPNE termination, DF/segment routing, 80386 attributes and prefix rejection.
No production discrepancy was found; existing Core timing remains explicitly
nonphysical. S17 owns STOS `AAh`/`ABh`.
## S17 STOS

S17 audits `AAh`/`ABh` STOS. Intel's 80386 Programmer's Reference Manual
string-instruction/STOS entries define AL/AX/EAX storage through ES:DI/EDI,
DF-controlled destination progression, operand-size payload width,
address-size index/count selection and REP iteration. This is original-source
semantic/form evidence; no emulator implementation, ROM or guest asset was
imported.

`M5:T401:S17:STOS-PROFILES:OK` retains all-four-profile byte/word execution,
FLAGS preservation, REP count zero/one/multiple behavior, ES-fixed destination
routing, DF decrement, 80386 dword and 32-bit address forms, pre-80386 prefix
atomic #UD, LOCK rejection, protected-limit nonpublication and IRQ behavior.
`M5:T401:S17:STOS-PREVIEW-PROFILES:OK` adds the complete four-profile lexical
matrix for both opcodes, REP and every selected 80386 operand/address attribute
combination.

No production discrepancy was found. This remains CPU semantic/nonphysical
timing-baseline evidence only and does not establish physical timing, DeskPro
L3 or a Core/VM interface change.
## S17 Acceptance

S17 closes selected STOS `AAh`/`ABh` form coverage with focused runtime and
preview proof across all four profiles. It covers accumulator payload widths,
FLAGS preservation, REP and DF/ES destination routing, 80386 attributes and
prefix rejection. No production discrepancy was found; existing Core timing
remains explicitly nonphysical. S18 owns LODS `ACh`/`ADh`.
## S18 LODS

S18 audits `ACh`/`ADh` LODS. Intel's 80386 Programmer's Reference Manual
string-instruction/LODS entries define DS-selected source loading into
AL/AX/EAX, DF-controlled source progression, operand-size payload width,
address-size index/count selection and REP iteration. This is original-source
semantic/form evidence; no emulator implementation, ROM or guest asset was
imported.

`M5:T401:S18:LODS-PROFILES:OK` retains all-four-profile byte/word execution,
FLAGS preservation, REP count zero/one/multiple behavior, source-segment
overrides, DF decrement, 80386 dword and 32-bit address forms, pre-80386
prefix atomic #UD, LOCK rejection, protected-limit nonpublication and IRQ
behavior. `M5:T401:S18:LODS-PREVIEW-PROFILES:OK` adds the complete
four-profile lexical matrix for both opcodes, REP and every selected 80386
operand/address attribute combination.

No production discrepancy was found. This remains CPU semantic/nonphysical
timing-baseline evidence only and does not establish physical timing, DeskPro
L3 or a Core/VM interface change.
## S18 Acceptance

S18 closes selected LODS `ACh`/`ADh` form coverage with focused runtime and
preview proof across all four profiles. It covers accumulator payload widths,
FLAGS preservation, REP and DF/source-segment routing, 80386 attributes and
prefix rejection. No production discrepancy was found; existing Core timing
remains explicitly nonphysical. S19 owns SCAS `AEh`/`AFh`.