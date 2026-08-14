# T359 S1: Four-Profile Instruction-Timing Inventory

## Scope and mechanical source

This inventory is an accounting record, not a timing table.  The mechanical
implementation sources are the one primary dispatch, the one `0F` dispatch,
and `core_machine_cpu_instruction_metadata_get()` in
`src/core/machine/cpu_instructions.c`; the sole cost publisher is
`core_machine_instruction_cost()` in `src/core/machine/machine.c`, called only
after successful refresh by `core_machine_run()`.  The verifier named below
requires the exact inventory headings and confirms the publisher still has an
explicit one-tick unallocated result.

The rows use these dispositions:

| Disposition | Meaning |
| --- | --- |
| `selected` | A T357 source row is already applied to the form under its recorded assumptions. |
| `receiver` | The form executes, but current one-tick fallback is a transfer, not an Intel clock claim.  The named T359 mechanism S must allocate an exact primary row or transfer a physical/source boundary. |
| `rejected` | Metadata, decoder, or Intel profile rule rejects the byte/form before successful retirement; it has no instruction-clock row. |
| `external` | The CPU-side boundary is implemented but the numerical/device work is explicitly outside this corpus. |

`66`, `67`, `F0`, segment override, `F2`, and `F3` do not inherit a source
clock merely because their unprefixed opcode is selected.  A selected row
below is selected only for the exact variant stated by the T357 ledger; every
other legal variant remains a receiver.  Real/protected/VM86 distinctions are
also receivers unless T357 S7 explicitly allocated ordinary `IN`/`OUT`.

## Primary-space inventory

Each primary encoding below names every currently dispatchable primary form.
Grouped encodings name their ModRM extension set explicitly; the reserved
extensions are rejected rather than folded into the group.

| Exact encoding or extension | Minimum profile | Current disposition | Timing mechanism receiver / source owner |
| --- | --- | --- | --- |
| `00`--`05`, `08`--`0D`, `10`--`15`, `18`--`1D`, `20`--`25`, `28`--`2D`, `30`--`35`, `38`--`3D` | 8086 | receiver | S2 arithmetic/defined-FLAGS and r/m timing matrix |
| `06/07`, `0E`, `16/17`, `1E/1F` | 8086 | receiver | S3 stack, selector image, and real-control timing |
| `26/2E/36/3E`, `F2/F3` prefix classifiers | 8086 | receiver | S4 string/repeat source timing; physical prefetch transfers later |
| `27/2F/37/3F` | 8086 | receiver | S2 decimal/ASCII adjustment timing |
| `40`--`4F`, `50`--`5F` | 8086 | receiver | S2 register arithmetic; S3 stack timing |
| `60/61`, `62`, `68`--`6F`, `C0/C1`, `C8/C9` | 80186 | receiver; rejected on 8086 | S2 arithmetic/BOUND; S3 stack/control; S4 strings; exact source conflicts transfer to T360 |
| `63` | 80286 | receiver; rejected below 80286 | S6 protected descriptor/privilege timing |
| `64/65`, `66/67` | 80386 | receiver; rejected below 80386 | S4 string/I-O width selection; S5 remaining prefix/width matrix |
| `70`--`7F` short `Jcc` | 8086 | selected for unprefixed taken/not-taken only; every other legal variant receiver | T357 S3--S6; S3 completes branch/lexeme timing |
| `80/81/83 /0`--`/7`; `82`; `84/85`; `86/87` | 8086 | receiver; `82` rejected | S2 immediate-ALU, TEST, XCHG and LOCK-aware memory timing |
| `88`--`8B` | 8086 | selected for the exact T357 register/memory rows; prefix/width/mode variants receiver | T357 S3--S6; S2 completes data/EA variants |
| `8C`, `8D`, `8E`, `8F /0`; `8F /1`--`/7` | 8086 | receiver; nonzero `8F` extensions rejected | S3 selector/LEA/stack timing |
| `90` | 8086 | selected only unprefixed | T357 S3--S6; S2 prefix and exchange variants |
| `91`--`97`, `98/99`, `9A`, `9B`, `9C`--`9F` | 8086 | receiver; `9B` CPU-side external boundary | S2 conversion/FLAGS; S3 control/stack; external x87 timing excluded |
| `A0`--`A3` | 8086 | selected for exact T357 moffs read/write rows; legal attributes receiver | T357 S3--S6; S2 width/prefix variants |
| `A4`--`A7`, `AA`--`AF` | 8086 | selected where the S4 source ledger defines the primitive or repeat row; transferred only for the explicit 80286 `REP LODS` source gap | S4 string/repeat timing |
| `A8/A9` | 8086 | receiver | S2 TEST timing |
| `B0`--`BF` | 8086 | selected only unprefixed immediate-register rows | T357 S3--S6; S2 width/prefix variants |
| `C2/C3`, `C4/C5`, `C6/C7`, `C8/C9`, `CA/CB`, `CC`--`CF` | 8086 or 80186 as metadata states | receiver; `C8/C9` rejected on 8086 | S2 immediate data; S3 call/return/far-load/software-delivery; S3 80186 frame timing |
| `D0`--`D5`, `D7` | 8086 | receiver; `D6` rejected | S2 rotate/decimal/XLAT timing |
| `D8`--`DF` | 8086 with optional FPU | external | CPU interface is retained; numerical x87 timing excluded |
| `E0`--`E3` | 8086 | receiver | S3 loop/condition timing |
| `E4/E5`, `EC/ED`, `E6/E7`, `EE/EF` | 8086 | selected only for T357 exact real/protected/permission ordinary-I/O rows; remaining legal width/prefix/mode variants receiver | T357 S3--S7; S3 ordinary-I/O matrix |
| `E8`--`EB` | 8086 | receiver | S3 near/far transfer timing |
| `F0`, `F4`--`FD` | 8086 | receiver; invalid LOCK successor rejected | S2 FLAGS/group arithmetic; S3 HLT and prefix timing |
| `F1` | 8086 | rejected | No successful-retirement timing row |
| `F6/F7 /0`--`/7`; `FE /0,/1`; `FE /2`--`/7`; `FF /0`--`/6`; `FF /7` | 8086 | receiver; named invalid extensions rejected | S2 group-3 and byte INC/DEC; S3 control/stack group timing |
| `0F` primary escape on 8086; `0F` escape on 80186 | 8086 / 80186 | rejected by the historical POP-CS byte route / ordinary `#UD`, respectively; neither is a secondary-form timing row | No T359 source row; preserve the profile decoder disposition |

## Secondary (`0F`) inventory

The secondary metadata is exhaustive for current valid forms.  Every valid
form is a receiver because T357 allocated no `0F` source clock; invalid or
below-profile forms are rejected.  This avoids treating a completed semantic
form matrix as a timing allocation.

| Exact secondary form | Minimum profile | Current disposition | Timing mechanism receiver |
| --- | --- | --- | --- |
| `0F 00 /0`--`/5` | 80286 | receiver | S6 selector/table system timing |
| `0F 01 /0`--`/6` | 80286 | receiver | S6 table/control and privilege/fault timing |
| `0F 02`, `0F 03`, `0F 06` | 80286 | receiver | S6 descriptor-query/CLTS timing |
| `0F 20`--`26` | 80386 | receiver | S6 control/debug/test-register timing |
| `0F 80`--`8F` | 80386 | receiver | S4 near-branch/lexeme timing |
| `0F 90`--`9F` | 80386 | receiver | S2 condition/FLAGS timing |
| `0F A0/A1/A3/A4/A5/A8/A9/AB/AC/AD/AF` | 80386 | receiver | S4 nonprivileged integer, FS/GS, bit, double-shift, and IMUL timing |
| `0F B2`--`B7`, `0F BB`--`BF` | 80386 | receiver | S4 far-load, bit, scan, and MOVX timing |
| `0F BA /4`--`/7` | 80386 | receiver; `/0`--`/3` rejected | S4 bit-immediate timing |
| all other `0F` bytes or invalid listed extensions | profile-dependent | rejected | No successful-retirement timing row |

## Classifier, mode, and publication inventory

| Mechanism / variants | Disposition | Receiver |
| --- | --- | --- |
| 8086/80186 `F0` bus-prefix legality; 80286 protected LOCK condition; 80386 whitelist and memory-only validation | Receiver when a legal modifying form succeeds; rejected otherwise | S2/S3/S4 retain one `PREFIX_LOCK` owner and allocate only source-backed legal forms. |
| segment override, repeated segment last-wins, operand/address presence, repeated `66/67`, and `F2/F3` last-repeat semantics | Receiver except exact T357 rows | S3/S4 allocate an exact variant or preserve one-tick transfer. |
| real, protected, and ordinary VM86 successful forms | Receiver except T357 S7 ordinary I/O rows | S3/S6 separate state and source assumptions. |
| synchronous fault, delivered exception/NMI/PIC, restart, preflight/budget/overflow, reset and stop | no instruction-clock allocation | Cycle-exact selected-profile receiver; T359 must preserve zero successful-retirement publication. |
| memory/I/O wait, HOLD/DMA, prefetch, cache, alignment not covered by a cited table, pin phase, and device service | physical transfer | Queue candidates Bus-timed PC/AT operation, device service-timing corpus, and cycle-exact selected profile. |

## Ordered mechanism allocation

The inventory creates bounded later S units, rather than allowing a test to
append isolated clocks:

1. **S2 -- arithmetic, FLAGS, data and ModRM/EA source matrix.** Primary ALU,
   group forms, TEST/XCHG, conversion, adjustment, and selected data forms;
   shared defined-FLAGS and r/m classification.
2. **S3 -- control and stack source matrix.** Calls/returns/interrupt
   instructions, LOOP/Jcc breadth, stacks, HLT, and source-backed real-mode
   return paths.
3. **S4 -- string, repeat and ordinary-I/O source matrix.** String primitive
   and restart formulas, repeat/count/direction variants, and remaining
   ordinary I/O variants.
4. **S5 -- 80386 secondary integer and prefix/width source matrix.** Near Jcc,
   SETcc, bit/double-shift/IMUL/scan/MOVX, 66/67/prefix variants, and their
   shared lexical/operand-address mechanisms.
5. **S6 -- 80286/80386 privileged-form timing and corpus reconciliation.**
   Table/selector/control/debug/task/VM86 forms only after their primary rows
   and delivery assumptions are bounded; transfer delivery cycles rather than
   inventing them.
6. **S7 -- task closure audit.** Verify every receiver has an exact source row
   or a truthful Queue/TODO transfer, then hand primary-source conflicts to
   the next Queue candidate, Four-profile Intel timing source reconciliation.

## Primary-source authority and transfers

The exact row/value authority remains the sources already retained by T357:
the Intel 8086 Family User's Manual Tables 2-20/2-21; the Intel 80186 timing
table; the Intel 80286/80287 PRM Appendix B (including the existing Appendix-B
precedence record); and Intel 80386 PRM section 17.2.2.3.  This S1 neither changes a
value nor resolves a table/prose contradiction.  Any new contradiction is a
T360 source-reconciliation row, never an inferred average or a Bochs/PCjs
substitute.

The sole current implementation fallback,
`CORE_MACHINE_SOURCE_UNALLOCATED_TICKS`, remains a visible transfer policy.
It must disappear from a successful form only when a later S adds the exact
source-backed classifier; it is not a valid timing result by itself.

## Verification record

`verify-t359-instruction-timing-inventory` checks the evidence's mandatory
primary, secondary, classifier, receiver, and source-authority rows against
the metadata, dispatch, and source-cost owners.  It intentionally does not
parse C as a full compiler: metadata and dispatch remain the mechanical
inventory, while this checked record owns the semantic timing disposition.

The T359 S4 developer artifact is `build/output/nxvm_0_5_0359.exe`, SHA-256
`D8422AE25FF8608282FAD66E057F76F5BC56143377AAAF4210BF4396A7B9FD89`.
