# T435 S6 P2 - 80186 Decoder Reconciliation Contract

## Purpose and denominator discipline

This contract joins three deliberately different universes:

| universe | owner and evidence | count | not permitted to mean |
| --- | --- | ---: | --- |
| lexical decoder candidates | `core-machine-80186-decoder-inventory-runner` | 247 primary opcodes; 61,530 opcode/ModR/M pairs | instruction-form or timing-key denominator |
| manual instruction forms | Table 2-9 primary, Table 1-16 supplemental contexts | represented by the S1 ledger | a decoder implementation claim |
| timing implementation keys | S2 manifest expansion | 279 base keys, then legal contexts | current conformance |

The scanner uses a zero-filled 15-byte window for every primary opcode and
ModR/M byte.  It exercises the actual public lexical decoder at profile 80186,
not a handwritten opcode list.  Its 61,530 pairs deliberately overcount
opcodes without ModR/M and therefore serve only as a drift sentinel.  The
runner fails if either frozen number changes.

## Decoder partition

The generated inventory contains every normal 8086-compatible primary opcode
accepted by the 80186 profile, the 80186 additions `60`--`62`, `68`--`6F`,
`C0`, `C1`, `C8`, and `C9`, FPU escape candidates `D8`--`DF`, and the legal
prefix bytes.  The following partition is mandatory when interpreting it:

| inventory member | disposition |
| --- | --- |
| `26`, `2E`, `36`, `3E`, `F2`, `F3` | prefix only; expanded by the S2 legal-context contract and never a successful retirement key |
| `F0` | semantic-only `LOCK` prefix; deliberately absent from lexical accepted-opcode candidates and covered by S2 legal `LOCK` contexts only |
| `D8`--`DF` | `ESC` form candidates; one S2 key per register/memory source class, with external completion kept outside retirement timing |
| all other inventory members | must map to exactly one S1 family and one or more S2 base keys; ModR/M extension, operand width, condition, branch outcome, immediate width and repeat phase refine the family key |
| rejected primary bytes, including `0F`, `63`--`67`, `F1` | not admitted to the successful-retirement denominator; their architectural fault path is outside it |

The 80186 additions are explicitly represented by S2 keys: `PUSHA`/`POPA`,
`BOUND`, immediate `PUSH`, immediate `IMUL`, immediate-count Group 2,
`INS`/`OUTS`, and `ENTER`/`LEAVE`.  `XLAT (D7)` is explicitly represented as
its own L3 key; this P2 requirement prevents a family-level `MOV` entry from
hiding it again.

## Bidirectional predicates

P2 is accepted only when both predicates hold:

1. Every scanner candidate is classified by the table above and, when it is a
   retirement instruction, has an S1 family and S2 base-key path.
2. Every S2 base key has an S1 source rule and a valid 80186 decoder recipe;
   no key may exist only because a manifest template was convenient.

The first predicate is proven by the generated inventory plus the frozen
candidate count and the partition above. The second is not yet closed by this
P2 scanner: it requires the following P3 recipe/key verifier.  P2 therefore
does not claim `I186-DECODER-LEDGER-ZERO-DIFFERENCE:PASS`.

## Reproduction

```text
cmake -S . -B build/t435-s6-p2 -G "MinGW Makefiles"
cmake --build build/t435-s6-p2 --target core-machine-80186-decoder-inventory-runner -- -j1
build/t435-s6-p2/core-machine-80186-decoder-inventory-runner.exe
```

Markers: `M5:T435:S6:I186-MANUAL-DECODER-PARTITION:OK`;
`M5:T435:S6:I186-DECODER-INVENTORY:247:61530`.
