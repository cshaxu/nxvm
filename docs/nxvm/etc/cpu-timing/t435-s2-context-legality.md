# T435 S2 CPU timing context legality contract

This supporting contract gives the four S2 manifests one meaning for a key.
It applies only to a successfully retiring instruction; faults, external bus
ownership, arbitration, READY waits and device completion remain outside this
CPU-instruction timing program.

## Canonical key shape

A base key is one documented instruction form.  A prefix is never a base key
or an independently retiring instruction.  A context key is emitted only when
the prefix or timing condition is legal for a non-empty subset of that base
form and changes a documented CPU timing input.  A combined key is emitted
only for the intersection of its named legal contexts.  A compact base form
may cover byte and word encodings; `ODD-WORD` then denotes its legal word
memory subset, not a separate instruction family.

## Required legality filters

| Context | Generate only for |
| --- | --- |
| `LOCK` | a documented memory read-modify-write form: arithmetic/logical memory destination, `INC`/`DEC`, `NEG`/`NOT`, or memory `XCHG`.  It is never generated for register forms, compare/test, string/repeat, control transfer, or shifts/rotates. |
| `SEGMENT` | a form with an effective memory operand whose segment selection can be overridden.  String keys are limited to source-consuming `MOVS`, `CMPS`, and `LODS`; destination-only string forms do not receive it. |
| `ODD-WORD` | a documented 16-bit memory transfer subset.  Byte-only variants do not receive it. |
| `REP-PHASE` | a documented repeat string or string-I/O form, with first/continuing/zero-iteration outcome where the manual formula distinguishes it. |
| `SIZE16` / `SIZE32` | an 80386 form for which the operand/address attribute is a manual timing input.  The suffix denotes the selected attribute, never an extra instruction byte. |
| `EA-BID`, `NEXT-BYTE`, mode/path, multiplier | precisely the manual condition named by the profile ledger. `NEXT-BYTE` begins at one: a successful next instruction has at least one executed byte, so a zero-byte context is not emitted. |

No selector may use `all` for a prefix context.  A manifest verifier expands
each selector, rejects duplicates, and checks its frozen base/context/
combination cardinality.  Thus removing an invalid key or adding a missing
legal key requires an intentional manifest and expected-count update rather
than silently changing the coverage universe.

The primary evidence remains the profile ledger and its cited Intel manual:
[8086](t435-s1-8086-ledger.md), [80186](t435-s1-80186-ledger.md),
[80286](t435-s1-80286-ledger.md), and [80386DX](t435-s1-80386-ledger.md).

Markers: `M5:T435:S2:FOUR-PROFILE-PREIMPLEMENTATION:OK`;
`M5:T435:S2:CONTEXT-LEGALITY:OK`.
