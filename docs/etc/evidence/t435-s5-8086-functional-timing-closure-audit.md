# T435 S5 - 8086 Functional And Timing Closure Audit

## Scope and proof rule

This is the final 8086 CPU-side reconciliation of the two independent
requirements in the owner request:

1. every successful 8086 instruction form has its documented architectural
   result and fault boundary; and
2. every successful S1/S2 form/context has its selected Intel L3 or named L2
   timing result.

The semantic authority is the Intel *8086 Family User's Manual*, 1979, and
the detailed opcode/form audit is retained in
[T401's concrete form matrix](t401-s77-concrete-form-requirement-matrix.md).
S1's exact/formula/range dispositions remain the timing authority in
[the 8086 ledger](../cpu-timing/t435-s1-8086-ledger.md). Neither test family
is treated as proof of the other: architectural effects are established by
the listed focused Core smoke, while time is established by a real retirement
for every canonical S1/S2 key.

`WAIT` and `ESC` are CPU-side instructions, but 8087 arithmetic, external
READY/HOLD, INTR/NMI delivery and bus-cycle occupancy are external inputs, as
the S1 boundary already states. Their CPU-side consume, provider dispatch and
defined fault behaviour are included; no claim is made to implement an
otherwise unselected external coprocessor or physical bus cycle.

## Complete S1 family crosswalk

Every row below is a complete S1 mnemonic/form family, not an example. The
semantic evidence's all-profile matrix includes the 8086 profile and its
documented rejection/fault forms. The final column is common to every row:
the S5 runner has one real retirement result for each expanded base/context
key applicable to the row.

| S1 successful family | 8086 functional evidence | timing evidence and disposition |
| --- | --- | --- |
| `ADC/ADD/AND/OR/SBB/SUB/XOR`, `CMP`, all register, memory, immediate and accumulator forms | T401 S56; `current.core-machine-legacy-alu-s2-smoke` checks byte/word directions, memory publication, complete defined FLAGS and CMP nonpublication. | `I86-ALU-*`, `I86-CMP-*`: exact/formula L3; S5 result has each EA/segment/odd/LOCK context. |
| `TEST`, including accumulator and Group-3 `/0` immediate forms | T401 S20 and Group-3 semantic matrix; `legacy-alu-s2` executes result/flag nonpublication rules. | `I86-TEST-*`: exact/formula L3; all legal contexts are in the 1,053-key result. |
| `AAA/AAS/DAA/DAS/AAD/AAM`, `CBW/CWD` | T401 S34 and conversion audit; defined AL/AH/AF/CF/SF/ZF/PF effects are asserted and AAM zero-base #DE is retained. | `I86-ADJ-*`, `I86-CONV-*`: exact L3. S5 adds direct real-machine checks for all eight simple forms. |
| `INC/DEC`, `NEG/NOT`, Group-4 `/0,/1` | T401 S10/S11; `current.core-machine-inc-dec-smoke` and primary/Group matrix cover register, r/m, flag-preservation and invalid extensions. | `I86-INCDEC-*`, `I86-NEGNOT-*`: L3 including EA, segment, odd-word and LOCK contexts. |
| `ROL/ROR/RCL/RCR/SHL/SHR/SAR`, one and `CL` count forms | T401 S8; `current.core-machine-rotate-smoke` covers all legal extensions, flags, zero/full 8086 `CL` semantics and rejects `/6`. | `I86-G2-*`: L3 unmasked-8086 count formula plus every legal memory/prefix context. |
| `MUL/IMUL/DIV/IDIV`, register and memory byte/word forms | T401 S9 Group-3 matrix establishes operand result, signed extension, defined CF/OF and decoder/fault boundaries; S5 adds real 8/16-bit signed and unsigned execution checks. | `I86-G3-*`: named `L2-86BOX-8086-G3`, with every result inside S1's manual range and every EA/segment/odd/LOCK context retired. |
| `MOV`, `MOV Sreg`, moffs, immediate r/m, `LEA`, `LDS`, `LES` | T401 S13/S14/S33/S44/S48/S49; `gpr-mov`, `sreg-mov`, `moffs`, `lea`, and `les-lds` smokes cover legal directions, memory-only restrictions, selector loading and fault atomicity. | `I86-MOV-*`, `I86-MOV-SREG-*`, `I86-LOADPTR-*`: L3 base and applicable EA/prefix/odd-word rows. |
| `XCHG`, including `90` NOP alias and r/m forms | T401 S12/S46/S49; `current.core-machine-xchg-smoke` verifies exchange, FLAGS preservation and invalid boundary. | `I86-XCHG-*`, `I86-FLAG-NOP`: L3, with all legal memory contexts. |
| register/segment/memory `PUSH` and `POP`, `PUSHF/POPF`, including 8086 `POP CS` | T401 S39--S41 and S5 `POP CS` correction; stack/selector smokes verify stack images, 8086 PUSH-SP distinction, selector/base load and atomicity. | `I86-PUSH-*`, `I86-POP-*`: exact/formula L3. `POP CS` is a separate 8-clock actual record, not a later-CPU `0F` escape. |
| direct and indirect near/far `CALL/JMP`, `RET/RETF`, `IRET` | T401 S22/S23/S27/S54/S55; `current.core-machine-control-transfer-smoke`, `iret-s51` and delivery smokes prove targets, frames, stack cleanup and failure atomicity. | `I86-CALL-*`, `I86-JMP-*`, `I86-RET-*`, `I86-IRET`: L3 direct/EA values. |
| all 16 `Jcc`, `JCXZ`, `LOOP/LOOPE/LOOPNE` outcomes | T401 S43/S53; control-transfer smoke covers every predicate and both target outcomes, with counter/FLAGS invariants. | `I86-JCC-*`, `I86-LOOP-*`: L3 taken/not-taken result keys. |
| `INT3`, `INT imm8`, `INTO`, `IRET` | T401 S25--S27; software-interrupt and IRET smokes prove vector, saved frame, overflow gate and return restoration. | `I86-INT*`, `I86-INTO-*`, `I86-IRET`: L3 outcome values. |
| `IN/OUT`, immediate and `DX`, byte/word | T401 S52; `current.core-machine-port-io-s55-smoke` proves provider routing, values and error/permission boundaries. | `I86-PORT-*`: exact L3 values. |
| `MOVS/CMPS/STOS/LODS/SCAS`, byte/word, legal `REP/REPE/REPNE` termination | T401 S15--S19; five dedicated string smokes prove source/destination selection, DF, flags, index/count update and repeat stop/restart. | `I86-STRING-*`, `I86-REP-*`: L3 primitive/repeat formula and all phase, segment and odd-word contexts. |
| `CLC/STC/CMC/CLD/STD/CLI/STI`, `LAHF/SAHF`, `HLT` | T401 S36--S38/S42; direct-flags, LAHF/SAHF, CLI/STI and HLT smokes prove defined-bit, inhibit and wake boundaries. | `I86-FLAG-*`: exact L3 values. |
| `XLAT` | T401 S35 plus S5 direct `DS:BX+AL` real-machine execution and segment override check. | `I86-XLAT`, `I86-XLAT-SEGMENT`: exact L3 11/13 clocks and their LOCK crossings. |
| `WAIT`; `ESC` register/memory forms | T401 S6 and `current.core-machine-fpu-escape-smoke` prove CPU-side consume/provider/fault boundary and profile legality. | `I86-WAIT`, `I86-ESC-*`: L3 CPU-side result, including EA/prefix contexts; provider arithmetic is explicitly outside the 8086 CPU instruction boundary. |

The remaining decoded forms are either the legal prefix/context multipliers
above or documented invalid/rejected encodings. T401 S5/S57 and S5's decoder
reconciliation prove the prefix scanner and form rejection set. In particular,
`0F` is valid only as 8086 `POP CS`; Group-2 `/6`, Group-3 `/1`, illegal
segment-MOV extensions and non-8086 opcodes do not enter the successful timing
universe.

## Current executable closure evidence

The runner scans the 233 lexical primary opcode candidates, the accepted
ModR/M forms and all S2 context generation. It executes 1,053 unique successful
8086 keys on a real Core machine and writes the result artifact. It rejects a
missing, duplicate, unallocated or non-S1-backed result in both directions.

Current evidence after the result-record integrity correction:

```text
core-machine-8086-timing-manifest-runner
M5:T435:S5:I86-MANIFEST-PROBE:PASS:1053/1053

Verify-8086TimingResults.ps1
8086 timing results verified: conforming_keys=1053

Verify-8086DecoderLedger.ps1
M5:T435:S5:I86-DECODER-LEDGER-ZERO-DIFFERENCE:PASS:1053

ctest --test-dir build/t435-s4-make --output-on-failure
100% tests passed, 0 tests failed out of 292
```

The generated [result artifact](../cpu-timing/t435-s5-8086-timing-results.json)
contains 989 L3 and 64 named `L2:G3` records. There is no L1 successful 8086
retirement row and no `SOURCE_UNALLOCATED` successful result.

## Audit conclusion

The functional evidence and timing evidence now close the same successful
8086 instruction universe: every S1 family has a manual-form semantic proof,
and every legal S2 form/context has a classified real-retirement timing
record. External coprocessor/bus inputs remain explicit boundaries rather
than unimplemented 8086 instruction forms. No remaining CPU-side 8086
functional or timing gap was found in this reconciliation.

Markers: `M5:T435:S5:I86-FUNCTION-TIMING-CROSSWALK:OK`;
`M5:T435:S5:I86-FUNCTION-TIMING-CLOSURE:OK`.
