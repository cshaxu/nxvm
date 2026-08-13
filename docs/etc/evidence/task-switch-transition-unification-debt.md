# Task-Switch Transition Unification Debt

## Boundary

The current protected task-switch implementation has two internally different
construction styles.  The 80386 32-bit direct-TSS `JMP` path uses an explicit
private plan followed by a commit phase.  Its plan materializes outgoing and
incoming state, validates the required descriptor/TSS/stack spans, and makes
descriptor updates private before the first architectural write.  The older
16-bit-TSS direct `JMP` path performs comparable validation but retains an
integrated function that interleaves transition preparation and the subsequent
field writes.

Both paths operate on the same CPU, descriptor, TSS, and checked-memory
owners.  This report does not claim that the 16-bit path is architecturally
incorrect, nor does it weaken the accepted S1 direct-JMP evidence.  It records
that the two paths are harder to compare, extend, and audit for atomicity.

## Why It Matters

The remaining task-entry and return forms differ only partly by TSS width:

| Transition | Shared semantic difference from direct `JMP` |
| --- | --- |
| Far `CALL` to a TSS | Old TSS remains busy; new TSS receives the backlink; incoming EFLAGS gains NT. |
| Task gate | Gate validation selects a TSS, then applies the same `JMP` or `CALL` transition semantics. |
| Nested task `IRET` | Uses the backlink/NT contract to return and changes busy state under its own fault rules. |
| IDT task gate / double fault | Reuses task-entry mechanics but has event-delivery and containment requirements. |

If each form appends another independent 16-bit and 32-bit write sequence, the
project accumulates divergent busy-bit, backlink, NT, descriptor, and fault
ordering behavior.  Conversely, an indiscriminate generic rewrite could erase
already-proven 16-bit behavior.  The debt therefore requires a bounded
transition unification task, not an opportunistic refactor.

## Required Future Task

Before the task-system package closes, admit a dedicated task-switch
unification slice that:

1. maps every current 16-bit and 32-bit task-transition caller and each TSS
   image/descriptor write;
2. states shared `JMP`, `CALL`, task-gate, and later nested-return semantics,
   including old/new busy state, backlink, NT, TR/cache, and `CR0.TS`;
3. makes the preflight/plan/commit boundary equally reviewable for both TSS
   widths, while retaining width-specific image readers/writers where the Intel
   layouts differ;
4. proves that no ordinary fault remains after the first committed write, or
   documents and tests the Intel-required partial-publication boundary; and
5. reruns the accepted T329 direct-JMP matrix plus new CALL/task-gate and fault
   evidence.

The task must not introduce a second public CPU-state interface, test mirror
state, generic transaction framework, paging/debug expansion, or a speculative
cross-family abstraction.

## Completion Standard

Closure requires a single documented transition vocabulary used by both TSS
width paths or an explicit, tested exception justified by an Intel layout or
fault rule.  Every remaining nested-return, IDT task-gate, LDT, paging, or
debug behavior must remain assigned to a later task-system slice.
