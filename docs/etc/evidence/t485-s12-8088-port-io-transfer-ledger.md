# T485 S12 8088 Port-I/O Transfer Ledger

`M5:T485:S12:8088-PORT-IO-TRANSFER-LEDGER:READY`

Visual review of the rendered 1981 Intel Table 2-21 source pages `2-55` and
`2-62` resolves the OCR ambiguity: each `IN accumulator,immed8`, `IN
accumulator,DX`, `OUT immed8,accumulator`, and `OUT DX,accumulator` row lists
one transfer.  The page footnote gives the 8088 rule: add four clocks for each
16-bit word transfer.

| Encodings | Base row | Table transfers | 8088 word transfers | Exact result |
| --- | --- | ---: | ---: | --- |
| `E4` / `E5` IN imm8 | `10` | 1 | `0` / `1` | `10` / `14` |
| `EC` / `ED` IN DX | `8` | 1 | `0` / `1` | `8` / `12` |
| `E6` / `E7` OUT imm8 | `10` | 1 | `0` / `1` | `10` / `14` |
| `EE` / `EF` OUT DX | `8` | 1 | `0` / `1` | `8` / `12` |

## Current receiver and boundary

`core_machine_string_io_source_instruction_cost()` is the existing sole
decoder-derived receiver for these eight opcodes.  It already resolves the
four source forms and uses the retained 8086 base ledger.  Its 8088 branch is
absent, so it currently returns no source result and the compatibility path
remains selected.  The decoded opcode supplies the complete local width fact:
the low bit distinguishes byte from word, and no port, device, external-cycle,
wait-state or profile data is required for the Table-2-21 addition.

Therefore the next receiver may consume one immediate private plan with
`word_transfers = opcode & 1`, add `4 * word_transfers` to the existing source
base, and select it before compatibility.  No port-controller service time,
prefetch, ISA cycle count, host pacing or physical-axis claim is admitted.
