# T512 S1 Five-CPU Source Cross-validation

`M5:T512:S1:SOURCE-CORPUS:ACCEPTED`

## Method And Evidence Boundary

Every original source below is owner-managed under `assets/manuals`. The
PDF metadata reports Adobe Paper Capture for every selected copy. Extracted
text is therefore navigation only; cover, section-heading and representative
instruction-table pages were rendered and visually checked. The scans are
legible for the cited scope, but OCR is never used to resolve a table value,
reserved-state bit, exception rule or timing formula.

86Box, PCjs and Bochs were inspected read-only. They provide cross-checks of
model gates and state/delivery structure, never a replacement source or code
to import. MAME and QEMU are not present in the approved local research roots;
S1 records that absence rather than fabricating a comparison. Any later need
for their specific behavior must use a separately recorded read-only source
review.

## Original Manual Corpus

| Profile | Original source and SHA-256 | Manual-backed scope | Quality and timing disposition |
| --- | --- | --- | --- |
| 8086 | Intel *iAPX 86,88 User's Manual* (1981), `1981_iAPX_86_88_Users_Manual.pdf`, `3EEA6CA77AD4046AE7ADE731410793206EEBE8EC9A3F8AE75895685D38F4FFE5` | Instruction semantics, encoding, FLAGS, real-mode delivery, Table 2-20 EA and Table 2-21 instruction clocks | 803-page Paper Capture scan; visually checked cover and printed page 2-51. Manual timing values and its stated assumptions are Manual-L3 candidates. |
| 8088 | Same Intel 1981 manual and hash | Same instruction universe, plus Table 2-21's distinct 8088 transfer addition | The shared scan is authoritative for the distinct 8088 row; its four clocks per 16-bit word transfer must not be inferred from 8086. |
| 80186 | Intel *iAPX 86/88, 186/188 User's Manual*, order `210912-001` (1985), `210912-001_iAPX_86_88_186_188_Users_Manual_1985.pdf`, `2516D66CC75076D9AC9EE048E8420C09C35655FB25ED34DDA6351A3EA4E0AFFF` | 80186 forms, model differences, Table 1-16 instruction data and bus context | 353-page Paper Capture scan; visually checked cover and printed page 1-27. Parenthesized 80186 entries require row-by-row visual citation in List 1. |
| 80286 semantics | Intel *80286 and 80287 Programmer's Reference Manual*, order `210498-005` (1987), `210498-005_80286_and_80287_Programmers_Reference_Manual_1987.pdf`, `AD487BA99B48CD9F61B14C0FE912A04C7CDB4C7C14A18419AA9FAF62D8962460` | Base/protected architecture, flags, delivery and Appendix B instruction encoding/semantic forms | 513-page Paper Capture scan; visually checked cover and Appendix B printed page B-1. It is a semantic/encoding authority, not an instruction-clock table. |
| 80286 bus | Intel *80286 Hardware Reference Manual*, order `210760-002` (1987), `210760-002_80286_Hardware_Reference_Manual_1987.pdf`, `D3ECE037A200B17EF32D78A055D0D96C3E47474D755D94569B9576A9A68C6915` | Clock, bus and wait-state electrical relations | 255-page Paper Capture scan. It describes bus/memory timing but supplies no per-instruction clock ledger. It cannot validate the existing 80286 instruction-clock literals. |
| 80386DX | Intel *386 DX Microprocessor Programmer's Reference Manual*, order `230985-003` (1990), `230985-003_386DX_Microprocessor_Programmers_Reference_Manual_1990.pdf`, `9A8188F9D2282B113FC421E225CC2A643FCDC349E5C3C43659BD2CF6620F1EA1` | 32-bit forms, paging/protection/VM86, FLAGS and Chapter 17 instruction forms, exceptions and execution-time columns | 557-page Paper Capture scan; visually checked cover and Chapter 17 printed page 17-1. Its row values and assumptions are Manual-L3 candidates. |

## External Cross-check Boundary

| Reference | Revision / scope inspected | Useful cross-check | Limit |
| --- | --- | --- | --- |
| 86Box | local `4fef696`; `src/cpu/808x.c`, `cpu_table.c`, `x86_ops_flag.h`, `386*.c` | Separates 8086 six-byte and 8088 four-byte prefetch queues; declares 80186, 80286 and 80386 configurations; keeps family-specific FLAGS paths. | Its timing/model literals are Other-L2 only unless the original manual independently supplies the same rule. |
| PCjs | local `c7f21b4`; `machines/pcx86/modules/v2/{x86,x86ops,x86help,cpux86}.js` | Declares all five model constants and branches instruction/delivery behavior by model. Its PUSHF/POPF path is a useful negative warning: it documents its own disagreement/assumption for parts of 80386 FLAGS behavior. | Do not elevate an implementation comment or cycle table over Intel; no PCjs source is imported. |
| Bochs | local `bochs-2.6-compat`; `cpu/flag_ctrl.cc`, `cpu/init.cc` | Provides 80286/80386-era protected/VM86 FLAGS and delivery cross-check structure. | This checkout is configured around later CPU-level compilation and is not evidence for 8086, 8088 or 80186 behavior or timing. |

## Source Correction And S2 Consequence

The prior 80286 timing ledger says `210498-005` Appendix B supplies instruction
clock rows. Visual review shows Appendix B is instead the 80286 instruction-set
encoding dictionary. The companion hardware manual gives bus timing, not a
per-instruction clock table. Therefore existing 80286 instruction-clock
literals are **not Manual-L3 on this source corpus**. S2 must preserve their
current values as unverified rows pending a valid original timing source or
explicitly classify the maintained behavior at the permitted lower level; S3
must not treat the earlier Appendix-B citation as proof.

This correction concerns evidence authority, not a conclusion that current CPU
behavior is wrong. It creates no code change and does not authorize one before
the complete List 1 and List 2 audits.
