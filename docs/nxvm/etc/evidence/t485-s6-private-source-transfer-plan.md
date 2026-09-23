# T485 S6 Private 8088 Source-Transfer Plan

`M5:T485:S6:PRIVATE-SOURCE-TRANSFER-PLAN:READY`

## One-use plan boundary

The transfer plan must not become `core_machine` state.  It is a local,
immutable result produced during the same successful-retirement timing
selection that consumes it:

```c
{ source_form, word_transfers, complete }
```

The plan has no lifetime beyond that selection; it has no setter, callback,
snapshot, reset path or public header.  This prevents a second guest-time or
instruction-state owner.

## Input and disposition map

| Source-form family | Existing Core fact | Plan result |
| --- | --- | --- |
| Primary ModRM/moffs forms | decoded form, memory/register shape, operand width, and the existing primary transfer classifier | exact zero, one or two word transfers where that classifier covers the manual row |
| String forms | selected string form, width, source/destination role and repeat phase | exact source/destination word count per completed primitive/repeat phase |
| Control/stack forms | selected source form plus the existing stack/memory role | exact count only for a documented table row whose stack/memory transfers are identified; otherwise incomplete |
| Register/immediate/flag/control forms with `Transfers = -` | selected source form | exact zero |
| Dynamic range, ESC/x87 wait, fault/interrupt delivery, prefetch, I/O/board wait or any form without a complete manual transfer relation | no complete common source fact | incomplete |

The external-cycle trace is explicitly excluded: it observes actual byte
transactions and board behavior, while the plan represents the manual's
instruction-form word-transfer count.

## Implementation rule

The next S introduces this local plan beside the existing private timing
selector and consumes it immediately in the 8088 evaluator.  The plan may use
the existing semantic form classifiers; it must not re-decode opcodes in a
second path.  A plan marked incomplete returns the existing visible unallocated
timing result.  The later IBM 5160 external-cycle receiver remains independent.

## Readiness verification

The owner sweep confirms each required input already resides in the private
CPU timing/execution boundary and none requires VM/profile data.  This S adds
no detached implementation, preserving the no-dead-code requirement.
