# T435 S7 - 80286 Manual Rebaseline

## Result

S7 re-read the owner-authorized 80286 manuals against the current Core
lexical decoder. The resulting successful-retirement denominator is 286 base
keys, 459 legal single-axis context keys and 62 legal combined-context keys.
Every base key is L3; this processor has neither a range-derived L2 model nor
an L1 exception.

## Manual disposition

The primary source is `I286-PRM-1987`, Appendix B. Its `Clocks` note supplies
the EA, odd-word, memory-read wait-state, next-instruction-byte and repeat
interpretation rules; external READY, BUSY and arbitration remain outside this
instruction-internal ledger. Each grouped S1 row now carries an explicit
manual locator.

`D7/XLAT` is the only dictionary omission discovered in the 1987 Appendix.
The same-CPU 1985 Intel companion manual, Chapter 8 printed p. 8-109, gives
its exact five-clock rule. S7 therefore adds `I286-XLAT` and its legal segment
context. This is L3 manual evidence, not a fallback or an inferred value.

## Corrected false forms

The prior system template expanded every system instruction into both register
and memory forms. That admitted five non-successful forms: register operands
for `LGDT`, `LIDT`, `SGDT` and `SIDT`, plus a second fictitious ModR/M form of
operand-free `CLTS`. S7 replaces it with the legal partition:

| class | successful forms |
| --- | --- |
| r/m forms | `LAR`, `LSL`, `VERR`, `VERW`, `LLDT`, `LTR`, `LMSW`, `SLDT`, `SMSW`, `STR` |
| memory-only | `LGDT`, `LIDT`, `SGDT`, `SIDT` |
| operand-free | `CLTS` |

No invalid form is retained merely to preserve a historic count.

Markers: `M5:T435:S7:I286-MANUAL-DECODER-PARTITION:OK`.
