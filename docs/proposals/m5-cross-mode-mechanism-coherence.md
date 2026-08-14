# Cross-Mode Mechanism Coherence

## Purpose

Audit and reconcile shared CPU mechanisms whose behavior spans real mode,
protected mode, and virtual-8086 mode.  The task prevents a mode classifier
from appearing to select one architectural path while a lower shared helper
silently rejects, bypasses, or changes that path.  It is an architecture
correctness program, not an opcode-count exercise.

## Required scope

Build a decision-to-commit ledger for each admitted shared mechanism.  For
every relevant mode, privilege, and state combination, record the entry
classifier, every validation/early-return branch, state and external side
effects, exception producer/delivery boundary, and final commit or rollback.
The initial priority order is:

1. I/O permission: ordinary `IN`/`OUT` and string `INS`/`OUTS`, `CPL`/`IOPL`,
   TSS I/O permission bit map, and virtual-8086 permit/deny behavior.
2. Exception and IRQ delivery: real/protected/virtual-8086 frames, restart,
   stack selection, and synchronous-fault versus pending-interrupt ordering.
3. Segment, descriptor, table, task, and return transitions: shared selector
   validation, cache materialization, TSS/LDT use, and commit atomicity.
4. Paging and linear memory access: permission, A/D mutation, `#PF`,
   transaction cancellation, and all adopted mode callers.
5. Shared prefix, `LOCK`, width, and string-execution classifiers: prove that
   classification and the executing handler have one compatible disposition.

Each admitted subtask repairs the shared owner when an implementation
divergence is reproduced, sweeps every caller and variant of that owner, and
uses Intel 80386 documentation as architectural authority.  It must retain
real Intel differences in frame width, descriptor/table/TSS layout, and
profile availability; the aim is to eliminate accidental construction
divergence, not width-specific implementations.

## Non-goals and stop conditions

No VME/PVI, 486+ extensions, numerical x87 execution, physical bus timing,
prefetch/cache model, host-time behavior, or Windows compatibility claim.
Do not import Bochs or PCjs code.  They may be read-only differential research
inputs under the source policy, while Intel documentation remains the
architectural authority.

Stop and split when an observed difference is an Intel-required layout or
mode semantic, a missing external corpus, a provider ABI decision, or a
physical-device timing problem rather than a shared-mechanism inconsistency.
Record the exact owner, risk, and admission condition in Queue or TODO; do not
mask it with a local test exception or a second state mirror.

## Evidence standard

For every closed mechanism group require:

- a source/caller/early-return/commit ledger and an Intel-source mapping;
- allow, deny, and fault matrices across every applicable mode and privilege
  state, including state publication, external side effects, exception frame,
  restart, and rollback boundaries;
- focused owner tests plus full current-gate proof;
- a similar-issue sweep over all callers sharing the repaired owner; and
- explicit transfer of intentionally unadmitted variants.

The task follows instruction-timed execution because successful timing rows
must not conceal a mode-correctness defect.  It precedes bus-timed PC/AT
operation: physical timing cannot be made trustworthy while the CPU's shared
execution mechanisms have contradictory mode paths.
