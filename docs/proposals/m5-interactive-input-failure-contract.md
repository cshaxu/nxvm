# Interactive Input Failure Contract

Close the P1 availability and memory-safety gap in the retained VM Console and
core debugger: a failed host-stdin read or command-argument allocation must
never enter parsing or command execution.

## Scope

This is one interaction-lifecycle mechanism, not two isolated call-site
patches. Inventory every direct `STD_FGETS` use in the retained debugger and VM
Console, classify its prompt/loop owner, and establish one explicit outcome for
EOF and read error. The selected owner-local helper or narrow shared primitive
must preserve the existing host-input capability boundary and must not create a
second console implementation.

The contract must ensure that:

- an unsuccessful read leaves no input available for parsing;
- the affected prompt or enclosing interaction terminates deterministically,
  without replaying stale command text;
- command-argument storage is successfully established before parsing starts;
- all normal and failed exits release that storage and leave the public context
  reusable, without a dangling arguments pointer; and
- debugger nested prompts apply the same failure rule without indexing an empty
  or uninitialized buffer.

## Non-goals

- Redesigning command syntax, console UX, or the retained debugger command set.
- Replacing the C runtime facade, redesigning xasm, or folding xasm capacity
  debt into this task.
- Introducing a generic input framework, a public callback ABI, or a new
  platform dependency.
- Changing guest execution, session selection, or host cancellation semantics.

## Proposed Subtasks

### S1 - Inventory and owner contract

Identify all debugger and VM Console `STD_FGETS` callers, their prompt nesting,
their argument-storage lifetime, and their current EOF/error behavior. Select
the narrowest existing owner boundary that can express the contract. Record any
excluded input path with its semantic reason and transfer destination.

### S2 - Complete interaction-lifecycle repair

Implement the agreed failure contract across every admitted caller. Ensure
allocation failure, EOF, and read error do not parse or execute a command;
clean up and reset context state on every exit; and retain existing successful
input behavior. Do not solve one prompt with a side path while leaving an
equivalent sibling prompt unsafe.

### S3 - Boundary regression and closure audit

Add deterministic, host-independent tests for VM Console EOF, debugger main
prompt EOF, and every distinct debugger nested-prompt outcome found by S1.
Exercise allocation failure through an existing controlled failure mechanism or
admit a narrowly scoped test seam only if none exists. Verify no command is
replayed after failure, no parser is entered, and returned contexts have no
owned dangling argument storage. Re-run the applicable current gate and inspect
the full changed mechanism against this proposal.

## Acceptance And Exit

The task closes only when the S1 inventory has no unclassified direct input
caller, all admitted callers satisfy the failure invariant, focused regressions
prove the failure outcomes, and the applicable configured gate passes. Any
caller that cannot share the chosen mechanism requires a documented semantic
reason and an explicit Queue or TODO transfer. The existing xasm capacity and
failure-semantics TODO remains open and is not closure evidence for this task.

## Evidence And References

- VM Console command-loop and argument storage:
  `src/vm/product/console.c`.
- Core debugger command-loop, nested prompts, and argument storage:
  `src/core/product/debug/debug.c`.
- Existing xasm capacity/failure-semantics debt:
  `docs/states/TODO.md`.
- Applicable lifecycle and repeated-mechanism rules:
  `docs/rules/EXECUTION.md`, `docs/rules/CODING.md`, and
  `docs/rules/ARCHITECTURE.md`.
