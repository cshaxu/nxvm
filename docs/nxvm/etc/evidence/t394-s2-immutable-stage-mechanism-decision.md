# T394 S2 Immutable Stage-Mechanism Decision

## Existing Surface Sweep

| Existing mechanism | Ownership/lifetime | Why it cannot enforce C0 membership |
| --- | --- | --- |
| Retirement contract enum | Core configuration, copied at construction | It has only deterministic/physical and no stage input. |
| Retirement observation | Core-owned copied diagnostic record | Its source form/context fields are not a complete semantic identity; its instruction point includes literal bytes that would incorrectly bind immediates, addresses and dynamic control targets. |
| Entry plan | One stopped-state initialization operation | It establishes initial CPU/RAM state, not retirement membership. |
| Immutable ROM mapping | Core copies/owns image mappings | It establishes backing lifetime, not CPU semantic classification. |
| VM Model-40 construction | Two composition routes | Both intentionally select deterministic mode and may not feed mutable stage state into Core. |

## Decision

No existing immutable Core configuration can describe the complete C0 set
without either collapsing distinct successful forms or overfitting literal
firmware bytes. A configuration descriptor based only on source timing form,
origin, ModR/M and mode is insufficient because the accepted C0 matrix contains
multiple semantic forms owned by the same timing row. A byte-pattern descriptor
would bind dynamic values and violate the stage's semantic, rather than raw
trace, definition.

The earliest feasible receiver is therefore a Core-owned normalized retirement
eligibility key, constructed alongside the existing source timing classifier
from its already available decode/context data. The key must be a finite copied
value with no VM callback, pointer or firmware identity. Only after the full
82-child C0 matrix maps one-to-one to such keys may a fixed descriptor be
validated and copied at machine construction; physical publication must reject
both source-unallocated successes and classified successes absent from that
descriptor before elapsed or device time advances.

This S neither introduces the key nor selects physical mode. It transfers a
bounded shared-Core key/capture design and requires a complete collision audit,
reset/failure behavior, two Model-40 caller proof and prepublication rejection
regressions before implementation.