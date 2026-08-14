# Complete Instruction-Timing Corpus

## Purpose

Extend T357's finite source-backed timing corpus into a complete,
profile-aware 8086, 80186, 80286, and 80386 instruction timing program. The
work begins with a full Intel-manual form inventory and mechanism ownership
map; it must not append isolated clocks when a guest test happens to need an
opcode.

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
