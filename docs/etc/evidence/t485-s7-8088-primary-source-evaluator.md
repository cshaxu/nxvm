# T485 S7 8088 Primary Source Evaluator

`M5:T485:S7:8088-PRIMARY-SOURCE-EVALUATOR:OK`

## One production path

The existing primary source-form classifier now creates a local transfer plan
and serves the 8088 selector before compatibility.  It reuses the same
decoded form, width, EA and read/modify/write classifier as the retained 8086
path.  For an 8088 primary form, each planned 16-bit word transfer contributes
four clocks; the 8086 keeps its existing odd-address-only term.

No plan is stored on the machine, and no external-cycle callback, VM/profile
input, board wait or prefetch state participates.

## Focused proof

The retirement-observation smoke verifies these rendered Table 2-21 cases:

| Instruction form | Table basis | Expected 8088 clocks |
| --- | --- | ---: |
| `ADD reg,reg` | `3`, no transfer | 3 |
| `ADD reg,mem16` direct | `9 + EA(6)`, one word transfer | 19 |
| `ADD mem16,reg` direct | `16 + EA(6)`, two word transfers | 30 |

The existing repeated-NOP physical-profile case remains source-unallocated:
the evaluator does not broaden to unsupported prefix/string/control forms.

The focused retirement-observation smoke and the complete current gate pass:
all 299 configured current-gate tests passed on 2026-08-26.  The gate's
`LastTestsFailed.log` is a stale runner byproduct for this invocation; the
authoritative current `LastTest.log` records `Test Passed.` for test 290 and
every test through 299.

## Explicit residual

String, control/stack, port/I/O, dynamic range, ESC/x87, fault/interrupt and
external-cycle forms do not enter this evaluator.  They retain the existing
compatibility fallback and unallocated physical disposition until their own
complete source plans are admitted.  IBM 5160 normal/five-clock external
cycles remain a separate receiver.
