# T485 S8 8088 String Transfer Ledger

`M5:T485:S8:8088-STRING-TRANSFER-LEDGER:READY`

## Rendered source rule

The rendered Intel *iAPX 86 and 88 User's Manual* (1981), Table 2-21, gives
the string rows on printed pages 2-53 (CMPS), 2-60 (LODS), 2-61 (MOVS), 2-65
(SCAS), and 2-66 (STOS).  Each page repeats one rule: for the 8088, add four
clocks for each 16-bit word transfer.  The rows provide the base primitive,
repeat setup and repeat-primitive clocks, plus transfer counts.  `MOVSB/MOVSW`
expressly distinguishes byte from word operations; the 8088 surcharge is
therefore zero for byte forms and four clocks per listed transfer for word
forms.  It is not an external byte-cycle count.

## Complete finite ledger

The following is the whole 8088 string family decoded by the existing selector.
For a nonzero repeat, `first` is setup plus the first primitive and `next` is
one continuing primitive.  A zero-count repeat has setup only, therefore no
word transfer and no 8088 surcharge.

| Form/opcodes | Rendered base | Word-transfer plan | Existing Core facts | Disposition |
| --- | --- | --- | --- | --- |
| MOVSB `A4` | `18`; `9 + 17/rep` | 0 | string form, opcode width, repeat phase | exact |
| MOVSW `A5` | `18`; `9 + 17/rep` | 2 per executed primitive | same | exact |
| CMPSB `A6` | `22`; `9 + 22/rep` | 0 | string form, opcode width, repeat prefix/phase | exact |
| CMPSW `A7` | `22`; `9 + 22/rep` | 2 per executed primitive | same | exact |
| STOSB `AA` | `11`; `9 + 10/rep` | 0 | string form, opcode width, repeat phase | exact |
| STOSW `AB` | `11`; `9 + 10/rep` | 1 per executed primitive | same | exact |
| LODSB `AC` | `12`; `9 + 13/rep` | 0 | string form, opcode width, repeat phase | exact |
| LODSW `AD` | `12`; `9 + 13/rep` | 1 per executed primitive | same | exact |
| SCASB `AE` | `15`; `9 + 15/rep` | 0 | string form, opcode width, repeat prefix/phase | exact |
| SCASW `AF` | `15`; `9 + 15/rep` | 1 per executed primitive | same | exact |

CMPS and SCAS accept the existing equal/not-equal repeat variants; their
manual time row and transfer plan are identical.  MOVS, STOS and LODS retain
the existing unconditional REP form.  Segment override remains a separate
two-clock Table-2-21 prefix term for MOVS, CMPS and LODS and must be applied
once by the retained string evaluator, independently of the 8088 surcharge.

## Receiver and hard boundary

`core_machine_source_timing_string_form()` already has the only semantic
opcode-to-form classifier.  The existing repeat publisher owns the observed
primitive, zero-count, first and continuation phase, plus repeat continuation
identity.  The retained 8086 ledger already holds the exact shared Table-2-21
base values.  No execution rerun, memory trace, profile value or board data is
needed.

The next implementation S may make the 8088 selector try the retained string
evaluator before compatibility and add one local, one-use plan from the opcode
width and form.  It must keep the source table as the sole base ledger, factor
the existing mixed 8086 odd-address modifier only if necessary, and add the
8088 `4 * word_transfers` term only for an executed word primitive.  It must
not duplicate the string table, classifier, repeat state, or evaluator.

String port I/O (`INS`/`OUTS`) is not an 8088 instruction family and remains
outside this batch.  Prefetch, board wait, bus-cycle selection and physical XT
axis qualification also remain outside it; source-classified retirement does
not itself publish a physical machine clock.
