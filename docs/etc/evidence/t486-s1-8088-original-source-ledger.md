# T486 S1 8088 Original Source Ledger

`M5:T486:S1:8088-ORIGINAL-SOURCE-LEDGER:ACCEPTANCE-CANDIDATE`

## Source identity and review method

The source is Intel, *iAPX 86,88 User's Manual* (1981), document
`AFN-01300C-1`.  The owner-managed archive holds a 803-page PDF scan.  Its
PDF producer identifies an Adobe Paper Capture operation dated 2010, so its
machine text is navigation only and never authority.  Visual page review is
the source check: the cover names the manual, copyright page ii identifies
Intel Copyright 1981 and `AFN-01300C-1`, and the reviewed image pages are
legible without clipped table columns.

## Finite source corpus for T486

| Source region | Role in the later ledger | Visual result |
| --- | --- | --- |
| Printed pages 2-30 through 2-47 | Instruction-family functional descriptions, including data transfer, arithmetic, bit, string, control-transfer and processor-control behavior | Listed by the manual contents; retained as the semantic source region for List 1. |
| Printed page 2-48 and Tables 2-17 through 2-20 on pages 2-49 through 2-51 | Instruction-reference scope; coding, flag, operand, effective-address and timing interpretation | Legible. Page 2-48 says Table 2-21 is detailed operational information; pages 2-49/2-50 define its terms and timing caveats. |
| Table 2-21, printed pages 2-51 through 2-68 | Complete selected instruction and prefix universe: operands, clocks, transfers, bytes and flags | All 18 printed pages were rendered and visually reviewed. The repeated 8088 footnote adds four clocks per 16-bit word transfer. |
| Printed pages 2-15 through 2-30, 2-68 through 2-72 and Chapter 4 encoding/decoding section | CPU-visible bus, lock, wait, interrupt, addressing and encoding context needed when a Table 2-21 row depends on it | In scope for row-level citation only; it does not turn external cycles, queue behavior or board service into instruction timing. |

## Confirmed boundary and uncertainty

Table 2-21 is a valid original-manual source for the selected 8088 instruction
forms, its clock values, operand categories, transfer counts, prefix rows and
its explicit non-instruction entries. It is not a complete physical-time model:
printed page 2-50 says its numbers assume a prefetched instruction and can be
increased by queue and EU/BIU interaction. INTR, NMI and SINGLE STEP are
explicitly marked as timing information rather than instructions. SEGMENT is a
prefix row, but its footnote says ASM-86 incorporates it into the operand
specification rather than treating it as a separate instruction.

No emulator or third-party source was used for this source admission. If a
later row lacks a complete original-manual rule, List 1 must retain that fact
and may only record a lawful non-imported corroboration boundary; it must not
invent a Manual-L3 result.

## Result

This source ledger is sufficient to start T486 S2's complete List-1 inventory.
It authorizes no code change and no List-2 gap conclusion. Retained T485
Table-2-21 evidence is a cross-reference, not a substitute for this
source-level review.
