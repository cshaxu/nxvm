# T435 S2 - 80186 Timing Implementation Audit

## Scope and rule

This audit consumes the accepted [80186 ledger](t435-s1-80186-ledger.md).
It records the current successful-retirement selection only; it neither changes
runtime timing nor reopens the S1 evidence/model choice.  Each ledger key is
controlled by the machine-readable
[80186 manifest](t435-s2-80186-timing-manifest.json), not by this narrative.

The observed selection order in `src/core/machine/machine.c` is:

```text
string I/O -> legacy dynamic arithmetic -> primary -> control-stack
-> 80186 fallback -> source-unallocated -> retirement observation
```

The terminal one-tick route sets `source_timing_unallocated`.  A successful
instruction reaching it is nonconforming; it is never an L1 substitute.

## Static reconciliation

| manifest key family | source route observed | current disposition and required close |
| --- | --- | --- |
| adjustments, conversion, selected ADD/SUB/CMP/TEST, `INC`/`DEC`, `NEG`/`NOT`, `XCHG`, immediate `MOV`, `LEA` | primary selector | source values are present, but no key-result proof exists; `missing-test` until one result asserts decoded form, ticks, origin and unallocated=false |
| other ALU mnemonics and forms not recognized by the narrow primary shape | later fallback or terminal | no current source rule is assumed from mnemonic similarity; focused audit must either locate the selector or retain `unallocated` before B0 closes |
| `MOV` memory/moffs/register forms and `Jcc` | 80186 legacy fallback | selected only for the explicit fallback cases; no aggregate family may be called conforming without its key result |
| calls, direct jumps, returns, loops, `INT`, `IRET`, `INTO`, push/pop register/memory/flags, 80186 `PUSHA`/`POPA`/push immediate/`ENTER`/`LEAVE` | control-stack | selected rows exist; five return keys (`RET`, `RET imm16`, `RETF`, `RETF imm16`, `IRET`) have real key-result proof, while every other form distinction remains individually open until its manifest-key result |
| scalar `IN`/`OUT`, string primitives and repeat forms including `INS`/`OUTS` | string/I/O selector and 80186 repeat ledger | all four scalar port keys have real result proof; primitive/repeat first/continue/zero phase, width, termination and source-register contexts still lack complete key results |
| `MUL`, inherited `IMUL`, `IDIV`, immediate `IMUL` | legacy dynamic arithmetic | **wrong-value**: values are endpoints/constrained source-model values, not S1's fixed midpoint L2 values; all 14 keys require replacement by the named midpoint rule |
| `DIV` | legacy dynamic arithmetic | exact source scalar exists but lacks a per-key result; `missing-test`, not conforming |
| Group-2 rotate/shift (`R1`, `RCL`, `M1`, `MCL`, immediate count) | no 80186 Group-2 selector | **unallocated**: all 42 L3 formula keys require B1 allocation; the 80186 low-five-bit count must be an input |
| legal `LOCK`, segment override and repeat contexts | no complete prefix-term program | generated only for their consuming legal instruction form; `LOCK` is limited to memory RMW and no prefix is a standalone retirement key |
| segment override, odd word and repeat phase contexts | inherited helper/string path plus focused runner recipes | all 87 legal single-axis odd-word keys now have complete successful-retirement observations and the required `ODD_WORD` input. Segment, repeat-phase, and all remaining combined contexts retain their manifest-recorded open status until each has equivalent evidence |

The current code applies `core_machine_8086_timing_effective_address()` and
`core_machine_8086_timing_odd_word()` on several 80186 primary paths.  That
is not automatically valid: the 80186 parenthetical Table 1-16 values are
inclusive and do not take the 8086 EA formula.  B3 must make every permitted
term explicit and prove it against the 80186 rule before any affected key can
be conforming.

## Exact initial accounting

`Verify-80186TimingManifest.ps1` expands 279 base keys: 253 L3 and 26
L2:midpoint.  The intentionally strict initial state is 0 conforming, 14
wrong-value, 43 unallocated and 222 missing-test.  The 14 are every accepted
midpoint Group-3/immediate-IMUL key; `BOUND` is the independently retained
Table 2-9 midpoint key; the 42 are the Group-2 formula keys. `XLAT` is an
explicit L3 base key, not an implied member of `MOV`, and its current 80186
route is terminal `source_timing_unallocated`.
`missing-test` is not a pass: it means a
source route may exist, but no focused key result has yet proved the required
value and inputs.

The manifest additionally expands 248 single-axis contexts and 89 legal
combined/phase contexts.  They cover
only legal segment-source uses, odd 16-bit transfers, legal repeat phases and
lockable read-modify-write forms.  The combined set covers legal lock/segment
intersections and every segment/odd-word/repeat-phase intersection that the
ledger authorizes.  It intentionally does not fabricate illegal `LOCK`
combinations or redundant prefix byte streams.

## Complete implementation and closure plan

| batch | required sole outcome | closure blocker |
| --- | --- | --- |
| B0 | one explicit 80186 form/context publisher replaces successful legacy selection | any accepted successful form continues to select independently, reaches unallocated, or lacks a stable key ID |
| B1 | all exact/formula L3 base keys, including Group-2, publish Table 1-16 value/formula inputs | one L3 base key nonconforming or an omitted low-five-bit/branch/outcome partition |
| B2 | all 26 L2 keys use their declared fixed midpoint, with no endpoint or constrained-model substitution | one midpoint form lacks its operand partition result |
| B3 | legal segment, odd-word, lock and repeat-phase axes have one generated key and one explicit term policy | an inherited 8086 EA term is applied without an 80186 source rule, or any legal axis is prose-only |
| B4 | focused test consumes every generated base/context key and emits value, inputs, origin and unallocated bit | any key is not `conforming`, or any obsolete successful selector remains |

Closure is all generated keys `conforming` at their declared L3 or L2:midpoint
level.  There is no L1 fallback and no transfer to another task.

Markers: `M5:T435:S2:80186-IMPLEMENTATION-AUDIT:OK`;
`M5:T435:S2:80186-REPAIR-SET:OK`.
