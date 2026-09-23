# T485 S4 8088 Table 2-21 Selector Ledger

`M5:T485:S4:8088-TABLE-2-21-SELECTOR:BLOCKED`

## Source boundary

Intel's *iAPX 86 and 88 User's Manual* (1981), Table 2-21, was checked from
the rendered original (printed page 2-56 for its continuing table and
footnote).  It gives instruction costs as clocks and states: the 8088 adds
four clocks for every 16-bit word transfer.  That is a source rule with two
required inputs: the selected instruction form and its complete count of
16-bit transfers.  It is not an instruction-count conversion and does not
define board wait states, prefetch overlap, HOLD, or I/O-cycle selection.

## Current Core selector sweep

`core_machine_cpu_timing_select()` has one exhaustive 8088 branch.  It selects
only `core_machine_compatibility_instruction_cost()` with origin
`COMPATIBILITY`.  T485 S3 already marks that recipe `SOURCE_UNALLOCATED`, so it
cannot publish physical retirement time.

The existing source evaluators deliberately exclude 8088:

| Existing evaluator family | Current source scope | Why it cannot be reused as an 8088 answer |
| --- | --- | --- |
| legacy 8086 Table 2-21 selector | `8086` and `80186` only | It models 8086 effective-address and odd-address additions, not the 8088 rule for every word transfer. |
| primary arithmetic/move selector | `8086`, `80186`, `80286`, `80386` switch | Its 8086 branch has form-specific transfer counts, but no 8088 branch or shared complete transfer contract. |
| control/stack selector | 8086/80186/80286/80386 lookup | It can count selected memory transfers for 8086, but does not expose a general 8088 transfer count to a common owner. |
| string/I/O and dynamic arithmetic selectors | selected legacy profiles only | Their repeat, range and I/O outcomes require explicit 8088 transfer accounting; compatibility is the present fallback. |

Thus the source-table batch has one present disposition: **zero 8088 successful
retirements currently select a Table-2-21 source form**.  This is exhaustive of
the current runtime path, not a claim that Table 2-21 has zero applicable
forms.

## Required earliest receiver

The next S must replace the 8088-only compatibility selection with one
private Core source evaluator that:

1. shares semantic form decoding with the retained 8086 evaluators rather
   than copying a decoder or building a parallel timing path;
2. obtains a complete per-form 16-bit transfer count from the same Core
   instruction/transaction facts used for retirement; and
3. marks any form whose count, prefetch, wait, or external-cycle condition is
   not source-represented as unallocated before physical publication.

That receiver may improve deterministic 8088 instruction costs only when the
manual rule and all required Core inputs are present.  It still cannot qualify
the XT physical axis: S5 must separately classify the IBM normal/five-clock
external cycles and waits, and later device work must close their deadlines.

## Verification

The rendered Table 2-21 source check, the complete 8088 selector-path sweep,
and T485 S3's physical-8088 rejection smoke were reviewed.  No code is changed
in this diagnostic S, so no generic timing value, profile clock or board ratio
is added.
