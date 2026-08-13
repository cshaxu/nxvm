# 80286 Descriptor-Table And Protected-Transfer Closure

## Objective

Close every 80286-assigned descriptor/table and protected-transfer row that
remains partial after the shared delivery, 8086/80186, and historical bounded
tasks: `0F 00`, `0F 01`, `0F 02/03`, and `0F 06` forms; selector/cache
validation; GDT/LDT/IDT state; gates; and true 16-bit protected stack, frame,
fault, return, and task boundaries.

## Dependency And Ownership

This candidate follows the 8086/80186 baseline. It consumes shared delivery
contracts but owns the 80286-specific selector, descriptor, table, gate, and
16-bit frame materialization. It must map all callers and writes through
validation, preflight, commit, rollback, and exception delivery before fixing
one opcode symptom. Genuine 16-bit layouts remain explicit.

## Required Matrix

The admission ledger expands each opcode/ModRM extension by real and protected
mode, valid and invalid selector/table/gate inputs, privilege, present/type,
memory source or destination, restart, publication, and real `#UD`/protected
fault delivery. It reconciles prior bounded evidence rather than counting it
as complete by name. Rows whose semantics are genuinely 80386-only transfer
once to the 80386DX system-state candidate.

## Non-goals And Stop Conditions

Do not establish the 80286 `LOCK` matrix here; it belongs to the next
candidate. Do not add 32-bit operand/address behavior, VM86, paging, DR/TR
breadth, VME/PVI, or x87 execution. Stop and transfer if correctness requires
a generic delivery redesign outside the assigned 80286 routes or a 80386-only
state contract.

## Exit Standard

Every assigned descriptor/table/transfer row has focused mode/form evidence
or one exact transfer; no assigned row remains partial, missing, or
unclassified. The result must leave a complete input ledger for the 80286
`LOCK` and profile-close candidate.
