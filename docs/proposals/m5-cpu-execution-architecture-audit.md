# M5 CPU Execution Architecture Audit

## Purpose

Re-audit the implemented 8086, 80186, 80286 and 80386DX execution surface from
its authoritative form and state matrices before claiming that the CPU audit is
complete. The work begins with a finite, source-backed inventory of instruction
forms, profile gates, execution owners, validation/preflight, state
publication, rollback, exception delivery and timing dispositions.

## Method

Use the Intel manuals and existing T336/T340/T341 evidence as the authority and
crosswalk. For each assigned row, record one of: proved implementation,
architectural rejection, deliberately distinct Intel layout, or transferred
boundary with a specific later admission condition. Inspect comparable callers
for duplicate paths. Repair a reproduced shared defect at its Core owner; do
not create a per-profile or VM workaround.

## Boundaries

This proposal does not infer undefined flags, invent timing formulas, add later
processor instructions, turn reference emulator behavior into Intel authority,
or claim x87 numerical execution, VME/PVI, persistent TLB/test registers,
physical bus timing or guest-OS compatibility. Each such item remains in its
existing named TODO or candidate.

## Completion Standard

A bounded audit increment closes only with an enumerated form/state universe,
all row dispositions, source and ownership map, focused proof for any repair,
current gate, documentation gate, and explicit transfer of every excluded
receiver. A later cross-profile reconciliation may close the program only after
all its bounded increments are complete.