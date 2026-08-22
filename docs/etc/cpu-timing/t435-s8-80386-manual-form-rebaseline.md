# T435 S8 - 80386DX Manual-Form Rebaseline

## Purpose

This is a corrective, source-first rebaseline inside the active S8 audit. It
records the forms which the original S1 prose named but the old S2 manifest
incorrectly compressed. A compressed family is not a closure key: every
manual-distinguished successful form needs its own key before a later timing
implementation may claim complete coverage.

## Primary evidence and decisions

The owner-authorized `I386DX-PRM-1990` copy, SHA-256
`9A8188F9D2282B113FC421E225CC2A643FCDC349E5C3C43659BD2CF6620F1EA1`, was
read directly. The following Chapter 17 tables are the exact basis for this
revision.

| family | prior S2 shape | required replacement | primary locator | disposition |
| --- | --- | --- | --- | --- |
| `ARPL` | one key | `R` and `M` left operands for `r/m16,r16` | printed p. 17-25 | L3, protected successful path input |
| `BOUND` | one key | `M16` (`r16,m16&16`) and `M32` (`r32,m32&32`) | printed p. 17-27 | L3, memory-only; register ModR/M is a fault and excluded |
| `BSF`, `BSR` | one key each | register/memory source for each 16- and 32-bit operand attribute | printed pp. 17-29, 17-31 | L3, documented `n` formula retained as a required input |
| `BT`, `BTC`, `BTR`, `BTS` | one key each | register/memory base x register/imm8 index x 16/32-bit operand attribute | printed pp. 17-32, 17-34, 17-36, 17-38 | L3, register/memory timing pair and immediate form remain distinct |
| `IN`, `OUT` | port route merged by width | immediate-port and DX-port form for byte, word and dword | printed pp. 17-69, 17-128 | L3, real/PM IOPL/permission successful route is an input |

For memory forms, the legal segment-override context set is also expanded.
That preserves the manual's CPU-only timing boundary: segment selection is a
semantic input, while waiting for a device or an external bus remains outside
this ledger.

## Denominator reconciliation

| metric | before this rebaseline | after this rebaseline | reason |
| --- | ---: | ---: | --- |
| base keys | 368 | 410 | 42 previously collapsed manual forms are now addressable |
| legal context keys | 832 | 935 | 103 new legal size/segment contexts for those forms |
| canonical S2 keys | 1,200 | 1,345 | sum of base and legal contexts; no synthetic cross-product |

All 410 keys remain L3. This document does not mark a key conforming, and it
does not treat an invalid encoding or an exception path as a successful form.
The next S8 reconciliation P must prove every base key against the decoder
inventory and then record the actual current route/status for it.

Markers: `M5:T435:S8:I386-MANUAL-FORM-REBASELINE:OK`;
`M5:T435:S8:I386-MANUAL-DECODER-PARTITION:IN-PROGRESS`.
