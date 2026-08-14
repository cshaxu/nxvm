# Complete Instruction-Timing Corpus

## Purpose

Extend T357's finite source-backed timing corpus into a complete,
profile-aware 8086, 80186, 80286, and 80386 instruction timing program. The
work begins with a full Intel-manual form inventory and mechanism ownership
map; it must not append isolated clocks when a guest test happens to need an
opcode.

This candidate consumes the preceding legacy dynamic-arithmetic authority and
80286 NOP source-conflict reviews. It may retain a precise transfer if either
review proves that a source cannot yet support allocation; it must not reopen,
average, or silently override either authority decision.

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
timing. Completion means the instruction corpus is accounted for, not that
the machine is cycle exact.

## Ordered mechanism units

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
