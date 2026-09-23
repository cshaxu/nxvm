# T340 S1: 80386DX Form And State Allocation Ledger

## Method

This allocation compares the primary and `0F` metadata/dispatch, prefix and
operand/address mechanisms, existing owner smokes, the T316/T310/T322
ordinary-form reconciliation, T336 four-profile ledger, and T339 descriptor
transfer audit. A historical smoke is accepted only for its declared matrix;
it is not broadened by this table.

## Residual Disposition

| Mechanism boundary | Forms or state rows | Accepted evidence / source owner | T340 disposition |
| --- | --- | --- | --- |
| Prefix classification and composition | `64`--`67`, `F0`, `F2/F3`, repeated prefixes, independent and combined operand/address attributes | `PREFIX_FS`, `PREFIX_GS`, `PREFIX_OprSize`, `PREFIX_AddrSize`, `_GetOperandSize`, address decoder; T316 S64 and T328 | S2 verifies that the declared semantic proof is sufficient for every residual 80386 form row; any missing combination remains at this owner. |
| FS/GS form and selector consumers | `0F A0/A1/A8/A9`, `0F B2/B4/B5`, FS/GS override consumers | `INS_0F`, `_e_load_far`, selector/cache loaders; T316 S23/S24/S31/S64 and T339 S3 | S2 reconciles only 80386 form selection and width. Descriptor privilege/cache or VM86 behavior transfers to T341. |
| Ordinary primary width forms | Existing `MOV`, stack/frame, arithmetic, string, I/O, and control form attributes | Primary dispatch, operand/address and stack helpers; T316 S23--S56 and T322 S1 | Accepted ordinary-form matrices are not reimplemented. S2 records the exact 80386 width/prefix proof or returns a concrete missing row to its shared owner. |
| Non-privileged `0F` integer/control | Near `Jcc`, `SETcc`, bit operations, double shifts, two-operand IMUL, BSF/BSR, MOVZX/MOVSX | `INS_0F` dispatch and arithmetic/FLAGS writers; T303 and T310 S3--S8 | S3 reconciles every metadata/ModRM family and its declared 80386 attribute, `LOCK`, fault, and IRQ proof. No evidence is inferred for privileged forms. |
| Descriptor/table and protected transfer width | 32-bit gates/frames, table forms, selector/table cache state | Protected serializers, `INS_0F_00/01`; T323/T339 | Transfer to T341: these rows require privileged state, frame, or table publication. |
| CR/DR/TR, paging, task, VM86, and debug | Control/debug/test registers, task/VM86 composition, translation and breakpoint delivery | State writers/readers; T320/T321/T325/T329/T337 | Transfer to T341; post-80386 extensions remain explicit outside boundaries. |
| External coprocessor | `9B`, `D8`--`DF`, 8087/80287/80387 execution | ESC/WAIT boundary; T316 S65/T317 | External: CPU interface accepted; numerical execution is not a T340 row. |

## Dependency-Safe T340 Sequence

| Planned S | Mechanism owner and complete result |
| --- | --- |
| S1 | This finite allocation ledger and receiver correction. No runtime claim. |
| S2 | Prefix/operand/address and FS/GS form reconciliation. It either closes every residual form row with actual owner-smoke proof or reports one bounded defect for the earliest shared mechanism owner. |
| S3 | Non-privileged `0F` integer/control reconciliation under the same proof standard. It cannot absorb CR/DR/TR, table, delivery, or debug behavior. |
| S4 | T340 closure audit: reconcile S2/S3 with the source graph and transfer every remaining state row once to T341. It makes no implementation repair. |

The allocation deliberately does not pre-claim that S2 or S3 need runtime
changes. It prevents both duplicated smoke work and a false 80386DX closure.

## Receiver Correction

T339's historical numbered receiver shorthand predates withdrawal of the
duplicate `LOCK` candidate. Under the current Queue, 80386 form rows are
owned by T340, privileged state rows by T341, the DX audit by T342, and the
four-profile audit by T343. These are current receiver names only; the T339
historical proof remains unchanged.
