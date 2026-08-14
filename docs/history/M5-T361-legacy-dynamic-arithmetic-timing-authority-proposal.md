# M5 Legacy Dynamic Arithmetic Timing Authority

## Purpose

Resolve the timing-authority boundary for the 8086 and 80186 arithmetic forms
whose Intel tables specify a range but no operand-to-clock rule, and close the
retained 80286 `NOP` source conflict. Real-hardware measurement is unavailable
and is excluded. The task therefore establishes a reproducible, explicitly
model-derived allocation policy for the legacy dynamic forms, while the 80286
review establishes a primary-source precedence and consumer decision for its
single already-allocated opcode.

## Scope

The candidate covers only 8086 `F6`/`F7` `MUL`/`IMUL`, and 80186
`F6`/`F7` `MUL`/`IMUL`/`DIV`/`IDIV` plus `69`/`6B` immediate `IMUL`, including
their profile-local operand width, register/memory, prefix, overflow/fault,
rollback, and source-consumer conditions. It inventories each primary table
statement, searches for a contemporary Intel formula, and defines the only
permitted reference-model alternative if the primary record stays incomplete.

It also covers only 80286 `NOP`: establish the edition/page/context of the
Appendix-B three-clock row and conflicting two-clock prose, decide whether they
describe the same condition, and sweep every 80286 timing ledger, classifier,
focused smoke, and scheduler/device consumer. This is a source-precedence
review, not a general 80286 timing expansion.

## Boundaries

Do not choose a range endpoint, midpoint, later-processor formula, host
benchmark, or an undocumented value. Do not implement bus waits, device
timing, prefetch, exception delivery, or a general arithmetic refactor. For
an Intel range, compare every selected formula value with that range. The
reference order is source- and profile-specific: inspect 86Box first for
early-PC model timing, then MAME's i86/i186 family for 80186 timing, then PCjs
only as an 8086 candidate. PCjs model declarations do not prove an independent
80186 timing table; its explicitly benchmark-adjusted or out-of-range values
are ineligible. Bochs is a later-CPU functional/state reference, PC110-EMU is
a machine-specific controller reference, and QEMU `icount` is a deterministic
instruction-count/event mechanism rather than an instruction-cycle reference.
Every non-primary allocation records its exact revision/path, input domain,
comparison result, and `model-derived` provenance. A formula must be captured
before retirement without publishing on fault. The 80286 `NOP` review may
retain Appendix-B `3` or revise it only through a primary-source decision and
focused consumer regression; it may not average prose/table values or use an
emulator or benchmark as authority.

## Completion standard

Every listed legacy arithmetic form has either an exact profile-local authority
and focused consumer proof, an Intel-range-constrained model allocation, or a
precise reference-derived allocation marked as non-physical. The 80286 `NOP`
review must record the chosen primary authority, the other statement's semantic
context, and every changed or unchanged consumer. The task sweeps all timing
consumers and preserves one post-refresh publisher. It is a prerequisite for
the complete instruction timing corpus and contributes to NXVM's documented
model L3 timing, not a physical cycle-exact claim.
