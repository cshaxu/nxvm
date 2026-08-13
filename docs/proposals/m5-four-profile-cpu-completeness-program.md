# M5 Four-Profile CPU-Completeness Program

## Purpose

Complete the Intel-defined CPU boundary for the project's 8086, 80186, 80286,
and 80386DX profiles before M6 admission. This is an ordered program, not a
claim that a collection of opcode smokes already makes a profile complete.
The Intel manuals remain the architectural authority; the T336 form ledger and
80386 closure map are the project's evidence crosswalks.

## Completion model

Every allocated instruction or architecture-state row has exactly one
disposition for each applicable profile and execution mode:

- implemented and proved at its Intel-defined boundary;
- architecturally rejected, with the precise producer, delivery, and
  nonpublication contract proved;
- a real Intel layout or semantic difference retained deliberately; or
- transferred once to a later named candidate or TODO with an admission
  condition.

An instruction implementation inherited from a later profile is not proof of
an earlier profile. Conversely, a width-specific layout, stack frame, TSS, or
gate rule is retained when Intel requires it. Shared validation, plan,
materialization, commit, rollback, or delivery construction must be reconciled
at its mechanism owner before profile-specific callers duplicate it.

## Ordered dependency and ROI policy

1. Shared state and delivery comes first because exception, interrupt,
   rollback, and frame behavior fan out to every profile and makes later
   instruction evidence trustworthy.
2. 8086/80186 real-mode closure comes next: it has the largest downstream
   surface and establishes the legacy acceptance, prefix, stack, and `LOCK`
   baseline that later profiles must not accidentally rewrite.
3. 80286 protected-mode closure follows, using the stable real-mode and
   delivery contracts for descriptors, gates, 16-bit protected stacks, and
   task/return behavior.
4. 80386DX closure then adds only genuine 32-bit, VM86, paging, debug, and
   extended system-state semantics. It must not use 80386 behavior to fill an
   earlier-profile gap.
5. Cross-profile closure is last. It reconciles evidence; it does not perform
   missing implementation under a verification label.

The Queue realizes this as five candidate packages: 80286 descriptor/transfer;
80386DX form closure; 80386DX state closure; 80386DX audit; and final
four-profile audit. T328 already closes the 8086/80186/80286 legacy `LOCK`
policy, so it is retained evidence rather than a duplicate candidate. This prevents a broad
"80386-complete" task from hiding dependencies between 16-bit protected
construction, width-form behavior, and system-state delivery.

Within a candidate, admit a mechanism owner before isolated opcode symptoms:
first map callers, state writes, validation/preflight, commit, rollback, and
fault ordering; then implement a bounded matrix slice; then sweep equivalent
forms. A reproduced shared defect is repaired at that owner or transferred,
never patched separately in each instruction handler.

## Required evidence for every candidate

- Start from the full assigned form ledger, not an operating-system demand
  subset. Record supported, rejected, transferred, and external rows.
- Audit profile gates, operand/address attributes, valid and invalid `LOCK`
  use, defined FLAGS, state publication, restart point, and applicable
  real/protected/VM86 delivery. Do not assert architecturally undefined FLAGS.
- For 8086, 80186, and 80286, retain T328's explicit legal-versus-illegal
  `LOCK` policy matrix for every allocated memory-capable mechanism. A
  decoder-wide policy is acceptable evidence only after its whitelist, all
  callers, and all profile exceptions are audited.
- Preserve true 16-bit and 32-bit architectural layouts. A common private
  plan may select each real layout independently; it must not flatten the
  layouts or change validation-to-commit/fault ordering.
- Run focused regressions, the applicable static inventories, the complete
  current gate, documentation governance, and diff checks. A candidate may
  close only when its ledger has no in-scope partial, missing, or unclassified
  row.

## Explicit boundaries

VME/PVI, later-CPU instructions, persistent TLB/test-register models, x87
numerical execution, timing fidelity, devices, and Windows compatibility are
not silently part of CPU-profile closure. Each remains external until a Queue
candidate or TODO gives it a bounded owner and admission condition.
