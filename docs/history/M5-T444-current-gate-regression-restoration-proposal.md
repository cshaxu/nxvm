# M5 Current-Gate Regression Restoration

## Purpose

Restore a truthful passing current gate by repairing the currently reproducible
T344 fixture-count failure and all 20 current-fast smoke failures in the one
admitted task. The failure set is frozen first so a shared mechanism is fixed
once; distinct root causes remain in this task until every named gate passes.

## Required Scope

Freeze the failing CTest identities, commands, outputs and source revisions.
Determine whether they share one build/test-fixture/runtime mechanism before
changing production behavior. Repair each owning mechanism directly, without
weakening a test, loosening an expected result, masking failures, or
rebaselining historical counts without a source-truth justification.

## Correctness And Minimalism Constraints

- The current-gate runner and each smoke keep their existing authority: no
  skip list, expected-failure blanket, environment-only bypass, or test-output
  suppression may turn failure into success.
- A common cause is repaired at its one owner and removes any duplicate setup
  or publication path it replaces. Similar symptoms with different ownership
  remain separate rather than being forced behind a new framework.
- T344's 71-versus-75 constructor invariant is reconciled to the documented
  fixture universe with proof; it cannot be silently rebaselined.
- Completion requires the complete current gate and the complete current-fast
  aggregate to pass. No failure is transferred, skipped, or accepted as
  expected.

## Non-goals And Stop Conditions

Do not alter CPU timing values, controller behavior, fixture universes, or
task history merely to satisfy a gate. Stop only for an unavoidable external
authority or environmental failure after recording the exact evidence.
