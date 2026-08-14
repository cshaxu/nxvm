# Complete Instruction-Timing Corpus

## Purpose

Establish a complete, profile-aware 8086, 80186, 80286, and 80386
instruction-timing corpus. T357's accepted finite source-backed ledger is
existing evidence and a reconciliation baseline, not a boundary on this
corpus. The work begins with a full Intel-manual form inventory and mechanism
ownership map; it must not append isolated clocks when a guest test happens
to need an opcode.

This candidate consumes the preceding legacy dynamic-arithmetic authority and
80286 NOP source-conflict reviews. It applies the shared evidence ladder:
exact Intel value; Intel-range-constrained same-profile external model; named
same-profile external model where Intel has no value; or an explicit
`reference-exhausted` transfer. It must not reopen, average, cross-profile
borrow, or silently override either authority decision. Model-derived timing
is not a physical cycle claim.

## Shared method, profile-local timing accounts

The four profiles share one audit method: each applicable
profile/form/context receives one source disposition, reaches the existing one
successful-retirement tick publisher, has source-labelled regression evidence,
and transfers every non-CPU contribution to one named receiver. This does not
make their timing data or accounting formulas interchangeable.

- **8086** uses its source-defined rows plus separately owned effective-
  address, odd-word, and segment-prefix additions only where the selected
  source accounting calls for them.
- **80186** uses its own Intel domains and same-profile model scalars. An
  eligible scalar outside an Intel closed range is visibly constrained within
  that range; an EA-included scalar never receives 8086 EA/odd-word additions
  again.
- **80286** and **80386** use their distinct Intel rows and their own mode,
  privilege, prefix, and addressing contexts. Neither provides a value for an
  earlier profile or inherits a legacy accounting formula.

For every profile, CPU instruction clocks remain distinct from bus waits,
prefetch/cache effects, DMA/HOLD, device service, and interrupt/fault-delivery
cycles. Those contributions retain their own named receiver. A fallback is a
visible transfer, never a claim of one-cycle accuracy. Closure therefore asks
whether every implemented profile/form/context has one provenance, one CPU
publisher, and one explicit non-CPU boundary, not whether all four processors
share a numerical formula.

## Required scope

For each profile, classify every implemented instruction form and relevant
mode/prefix/addressing variant as one of: a primary-source timing row with an
admitted owner, a documented unsupported/profile-rejected form, an
Intel-required dynamic formula, or an explicit missing source/implementation
receiver. Map each admitted row through the one successful-retirement elapsed
tick publisher, including preflight maximum, fault/nonpublication, repeat and
restart behavior, reset, provider/scheduler visibility, and every timing
consumer.

The admission must group forms by shared timing mechanism, not convenient
opcodes, and create only the bounded implementation subtasks supported by the
inventory. Each group needs source-row provenance, all callers and variants,
focused proof, and a full current-gate result. Existing T357 values remain
authoritative until a source-reconciliation task explicitly changes one.

## Non-goals and stop conditions

Do not infer bus waits, HOLD/DMA ownership, prefetch, cache, pin phases,
device service latency, fault/interrupt delivery cycles, x87 execution, host
time, VME/PVI, or Windows compatibility from instruction tables. Stop and
transfer if a value depends on a physical bus/device contract, an incomplete
primary reference, or a shared mode/state mechanism not yet coherent.

## Evidence standard

Require an Intel source-to-form matrix covering all four profiles; a
classification for every implementation table/handler; source/caller and
validation-to-publication sweep; focused mode/prefix/fault/restart tests;
actual current-gate; and explicit Queue/TODO transfer of non-CPU physical
timing. Completion means the instruction corpus is accounted for and every
model-derived or reference-exhausted row is visible to the later selected
profile audit; it does not make the machine physically cycle exact.

## Ordered mechanism units

1. **S1 -- full form inventory, ownership, and source classification.**
   Enumerate every implemented form and relevant profile, mode, prefix, and
   addressing variant; assign its shared timing mechanism and evidence-ladder
   disposition before allocating any runtime value. Record every unsupported
   or profile-rejected form and give each reference-exhausted or physical-
   timing row one explicit receiver.

1. **S2 -- arithmetic, FLAGS, data and ModRM/EA source matrix.** Primary ALU,
   group forms, TEST/XCHG, conversion, adjustment, selected data forms, and
   shared defined-FLAGS/r-m classification.
2. **S3 -- control and stack source matrix.** Calls/returns/interrupt
   instructions, LOOP/Jcc breadth, stacks, HLT, and source-backed real-mode
   return paths. Protected, cross-privilege, VM86, task, and generic-delivery
   forms remain transfers rather than inferred clocks.
3. **S4 -- string, repeat and ordinary-I/O source matrix.** String primitive
   and restart formulas, repeat/count/direction variants, and the remaining
   ordinary I/O variants share a stateful iteration/provider mechanism.
4. **S5 -- 80386 secondary integer and prefix/width source matrix.** Near Jcc,
   SETcc, bit/double-shift/IMUL/scan/MOVX, 66/67/prefix variants, and their
   shared lexical/operand-address mechanisms.
5. **S6 -- 80286/80386 privileged-form timing and corpus reconciliation.**
   Table/selector/control/debug/task/VM86 forms only after their primary rows
   and delivery assumptions are bounded; transfer delivery cycles rather than
   inventing them.
6. **S7 -- task closure audit.** Verify every receiver has an exact source row
   or a truthful Queue/TODO transfer, including the named physical/device and
   cycle-exact receivers. It does not allocate a new source-conflict task.
