# T435 S2 - 80186 L3/L2 Implementation Tracker

## Controlling artifact

The [80186 manifest](t435-s2-80186-timing-manifest.json) is the sole
per-key closure ledger.  It expands 281 base instruction keys from the
accepted [80186 manual ledger](t435-s1-80186-ledger.md), then 263 legal
single-axis and 149 legal combined/phase context keys.  This document is the
human implementation map; it
cannot change a key's source rule, level, status, batch or regression ID.

Run:

```text
powershell -NoProfile -ExecutionPolicy Bypass -File tools/Verify-80186TimingManifest.ps1
```

before every 80186 tracker-changing P.  The verifier rejects duplicate keys,
missing required fields, invalid level/status values, wrong profile and count
drift.  It is a manifest-integrity gate, not runtime evidence.  The later
focused test must consume every generated `key_id` and publish target ticks or
formula inputs, retirement origin and `source_timing_unallocated`.

## Per-key implementation map

| manifest template | level and authoritative rule | present state | close in |
| --- | --- | --- | --- |
| `I186-ADJ-*`, `I186-INC/DEC-*`, `I186-NEG/NOT-*`, `I186-XCHG-*`, `I186-LEA/LDS/LES-*` | L3 Table 1-16 exact | source selection partial/unproven | B0+B1+B4 |
| `I186-ALU-*`, `I186-CMP-*`, `I186-TEST-*` | L3 exact except the explicit AI rows below | narrow primary coverage; no unproven member is conforming | B0+B1+B3+B4 |
| `I186-ALU-*-AI`, `I186-CMP-AI`, `I186-TEST-AI`, `I186-MOV-{RI,MI}` | L2:midpoint, manual range and `ceil` rule in S1 | no midpoint closure result | B0+B2+B4 |
| `I186-MOV-*`, stack, call/jump/return, branches, interrupt/port/ESC/WAIT/HLT | L3 exact/formula | mixed primary/control/fallback routes; per-form proof absent | B0+B1+B3+B4 |
| `I186-{MUL,IMUL,IDIV}-*`, `I186-IMUL-IMM-*` | L2:midpoint accepted S1 values | wrong-value old dynamic/constrained model | B0+B2+B3+B4 |
| `I186-DIV-*` | L3 exact | source scalar exists; proof absent | B0+B1+B3+B4 |
| `I186-{ROL,ROR,RCL,RCR,SHL,SHR,SAR}-*` | L3 formula; count modulo 32 | unallocated | B0+B1+B3+B4 |
| `I186-STRING-*`, `I186-REP-*` | L3 primitive/repeat formula | source table exists; phase/termination proof absent | B0+B1+B3+B4 |
| `I186-{BOUND,ENTER-*,LEAVE}`, `I186-FLAG-*`, `I186-PREFIX-*` | L3 exact/formula | mixed/unallocated as manifest records | B0+B1+B3+B4 |
| generated `-SEGMENT`, `-ODD-WORD`, `-LOCK`, `-REP-PHASE-*` and legal combined suffixes | S1 Table 1-16 context rule | no complete independent input/result stream | B3+B4 |

The template form is deliberately compact, but braces in the manifest expand
one concrete key per opcode/form/outcome/width/context.  A test result for one
member never closes a sibling.

## Non-negotiable final predicate

The profile closes only when every base and context key has one source rule,
one L3/L2:midpoint target, one current route, one batch, one regression ID and
a focused result marking it `conforming`.  No key may retain `wrong-value`,
`unallocated`, `missing-input` or `missing-test`; no successful retirement may
take the anonymous terminal; and no pre-existing timing selector may coexist
with the B0 publisher.

Markers: `M5:T435:S2:80186-IMPLEMENTATION-TRACKER:OK`;
`M5:T435:S2:80186-TRACKER-CLOSURE-PREDICATE:OK`.
