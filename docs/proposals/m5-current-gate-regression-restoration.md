# M5 Current-Gate Regression Restoration

## Purpose

Restore a truthful passing current gate by classifying and repairing the
currently reproducible T344 fixture-count failure and 20 current-fast smoke
failures at their shared owning mechanism, or by splitting genuinely distinct
owners into separately admitted follow-on tasks.

## Required Scope

Freeze the failing CTest identities, commands, outputs and source revisions.
Determine whether they share one build/test-fixture/runtime mechanism before
changing production behavior. Repair a common owner once when evidence proves
one; otherwise create bounded proposals/TODO transfers for each distinct owner
without weakening a test, loosening an expected result, masking failures, or
rebaselining historical counts without a source-truth justification.

## Correctness And Minimalism Constraints

- The current-gate runner and each smoke keep their existing authority: no
  skip list, expected-failure blanket, environment-only bypass, or test-output
  suppression may turn failure into success.
- A common cause is repaired at its one owner and removes any duplicate setup
  or publication path it replaces. Similar symptoms with different ownership
  remain separate rather than being forced behind a new framework.
- T344's 71-versus-75 constructor invariant is either reconciled to the
  documented fixture universe with proof or remains a named blocking transfer;
  it cannot be silently rebaselined.
- Completion requires the complete current gate to pass, or an owner-approved
  explicit task split that leaves no claimed completed task relying on an
  unrecorded failing gate.

## Non-goals And Stop Conditions

Do not alter CPU timing values, controller behavior, fixture universes, or
task history merely to satisfy a gate. Stop and split work when failures have
independent owners or need source/architecture authority beyond the gate's
bounded restoration scope.
