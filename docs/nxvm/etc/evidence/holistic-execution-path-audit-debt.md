# Holistic Execution-Path Construction Debt

## Purpose

This report records a recurring construction risk in the CPU and machine
executor: an implementation can close a single opcode, mode, or smoke failure
while leaving the larger architectural transition undefined.  Intel manuals
remain the authority for each admitted behavior; this report neither activates
work nor reclassifies any completed task.

The risk is referred to in planning as incremental or "patch-on-patch"
construction.  It is a process and architecture debt, not a claim that every
incremental fix is wrong.  A focused repair remains appropriate when its owner,
state transition, caller set, fault boundary, and transfer boundary are already
known.

## Observed Failure Shape

The debt exists when one or more of these conditions is true:

1. a new opcode path duplicates state publication that an adjacent form already
   owns, rather than extending a named common transition;
2. validation, externally visible writes, and CPU/cache publication are
   interleaved without an explicit proof of what can still fault after each
   write;
3. a smoke proves a local success or terminal failure but does not distinguish
   producer state, delivery state, partial publication, and restart boundary;
4. 16-bit, 32-bit, real, protected, VM86, direct, gate, interrupt, or return
   variants acquire divergent implementations without a documented reason; or
5. a narrow repair changes a shared helper, decoder, descriptor, stack, or
   exception route without a caller and affected-form sweep.

The appropriate response is not an automatic repository-wide rewrite.  It is
to establish the complete architectural state model for the bounded family,
then decide whether a common transition, a deliberately local path, or an
explicit transfer is correct.

## Required Admission Method

A future queue admission that addresses this debt must select one bounded
family or subsystem and produce all of the following before changing
production behavior:

| Required artifact | Required content |
| --- | --- |
| Intel form/state matrix | Every in-scope form and mode, its selector/operand/prefix rules, and its complete, partial, missing, or transferred classification. |
| Ownership and caller map | Producer, decoder, memory/descriptor/stack routes, delivery paths, and all callers of any shared helper to be changed. |
| Transition contract | Validation order, private plan state where useful, first externally visible write, commit order, rollback/fault semantics, and post-commit interrupt behavior. |
| Focused evidence | Success, rejected form, pre-commit fault, and post-commit/IRQ or equivalent observations that distinguish the contract. |
| Transfer record | Every excluded mode, architecture extension, or behavior and the named later package that owns it. |

## Completion Standard

The candidate can close only when the selected family has one documented
transition model or an explicit, evidence-backed reason for distinct models;
all changed shared routes have an audited caller set; focused regressions prove
the named commit and fault boundaries; and unresolved breadth has a concrete
queue or TODO transfer.  It must not claim global architectural cleanup.

## Relationship To Current Debt

`docs/states/TODO.md` retains the open ledger entry and its priority/admission
condition.  The queue candidate references this report so that a future task
begins from a concrete audit method rather than another local symptom.
