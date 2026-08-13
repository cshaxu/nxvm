# 80386DX Width, Prefix, And Integer-Form Closure

## Objective

Close residual 80386DX instruction-form rows whose principal owner is
operand/address width, prefix composition, FS/GS selection, or an integer
primary/`0F` form rather than privileged system state.

## Dependency And Ownership

This candidate consumes closed 8086/80186 and 80286 ledgers. It must preserve
those profile dispositions and start with a complete metadata/form inventory.
For each mechanism, map decoder, prefix, addressing, state writes, validation,
commit, rollback, and fault order before a handler repair.

Its allocation starts from the residual 80386DX form ledger and groups rows by
the construction they share: prefix/decode composition, operand/address
materialization, FS/GS selection, integer operation/FLAGS, and memory
publication. A form may be split only when it crosses one of those mechanism
boundaries. A 16-bit predecessor result is evidence of a baseline disposition,
not proof of its 32-bit variation.

## Required Matrix

Cover all residual `66`/`67` and combined rows, FS/GS behavior, declared
32-bit primary forms, and non-privileged integer `0F` rows. Each row carries
mode, ModRM extension, defined FLAGS, memory selection, valid/invalid `LOCK`,
publication, restart, and appropriate IRQ/fault proof. Intel-required width
differences stay explicit; accidental construction divergence is repaired at
its mechanism owner.

## Non-goals And Stop Conditions

CR/DR/TR privilege, paging translation, VM86 transitions, task state,
breakpoints, VME/PVI, later CPUs, and x87 execution belong elsewhere. Stop if
a reproducer requires any of those state owners rather than a form/prefix
repair.

## Exit Standard

Every allocated 80386DX width/prefix/integer row is proved or transferred once
to the 80386DX system-state candidate; no row remains partial by a smoke-count
claim. The transfer ledger names the receiving state owner, not merely
"80386 system work".
