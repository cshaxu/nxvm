# T479 S1 AT Fixed-Disk Current-Gap Ledger

`M5:T479:S1:HDC-GAP-LEDGER:OK`

| Rows | Current single owner/path | Exact gap or retained disposition | Receiver |
| --- | --- | --- | --- |
| IBM-1 | `pc_at_profile` -> resolved profile -> copied Core plan rejects Model-339 HDD. | Correct selected-machine absence. | None. |
| IBM-2--IBM-6 | No IBM MFM personality exists; `core_machine_hdc` is not renamed to imply one. | Controller-primary source and physical-drive contract are missing; 86Box estimate is not usable timing. | S2 audit, then S3 only if primary source closes identity. |
| CPQ-1--CPQ-5 | `COMPAQ_WD_40MB` in `core_machine_hdc`; `machine_board` validates FDC-before-HDC and one 3F7h wired-OR contribution; Model-40 resolver supplies immutable ports/media. | Functional command/register boundary exists. Long/format/ECC/recovery remain unsupported by RAW-IMG. | Retain; S2 verifies all callers. |
| ATA-R1--R5 | `hdc.c` owns task file, phases, PIO buffer and IRQ; T468 smokes cover selected commands. | Correct ATA-only path; command set intentionally bounded. | Retain; S2 finds no parallel firmware/direct path. |
| ATA-F1--F2 | No Core cable or HDC DMA state. | Correctly absent, not a missing setter. | Future selected board/controller task only. |
| ATA-F3 | `hdc.c` owns SRST/nIEN and reset signature. | Logical Manual-L3 behavior; physical delay lacks input. | Future timing-plan admission. |
| ATA-F4--F5 and ATA-T4 | Core drives causal service through its timeline; no HDC deadline state. | L1 order only, never host time or a scheduler-tick duration. | Future service-time/deadline task. |
| ATA-T1--T3 | One copied plan/topology creates HDC; frozen media registry owns bytes/persistence. | Correct one-owner construction/media split. | Retain; S2 traces declarations. |
| ATA-T5 | Explicit protocol enum rejects ATA LBA/port assumptions in the Compaq form. | Correct semantic separation; enum name is not generic ST-506 evidence. | S3 may rename only as a replacement, never a second path. |
| ESDI-1 | No personality, profile or source. | Unsupported; no placeholder controller object. | Future source-gated candidate. |

S2's finite sweep is `controller_interface.h`, `hdc.[ch]`, `machine_board.c`,
`machine_plan.c`, VM profile declarations, session machine-device assembly,
HDD media ownership, firmware INT 13 code, all HDC tests and every `protocol`
or `lba28_supported` caller. It must decide whether the existing two-personality
state machine already is the required one-owner front end; it may not create a
base class, plug-in layer, second CHS state or media cache.
