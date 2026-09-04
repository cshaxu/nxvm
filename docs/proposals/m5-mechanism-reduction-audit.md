# M5 NXVM Mechanism-Reduction Audit

## Goal

Reduce accidental NXVM complexity without a rewrite: every retained mutable
fact, lifecycle decision, host/product route, and test side effect has one
owner and one production path.  The result must delete obsolete mechanisms or
duplicate code; it must not add a framework, compatibility layer, or second
state model merely to describe the existing one.

## Why This Candidate Exists

The CPU, bus, controller, ROM and multi-profile emulation domains have
irreducible complexity.  Recent work instead exposed avoidable complexity at
their seams: parallel presentation/action routes, lifecycle observations that
can become mirrors, and tests that write shared fixed paths without declaring
the workspace owner.  Those defects make an otherwise valid local repair
fragile across sessions, profiles, or parallel test runs.

## Scope And Method

Audit the currently runnable product and build/test composition by owner and
data flow, not by cosmetic similarity:

1. **Session and platform mechanism inventory.**  Trace lifecycle,
   presentation, host-input, capture and command routes across Core, VM,
   Console and Window.  Retain separate code only where a real guest, native
   host-capability, lifetime, or failure-boundary distinction exists.  Merge
   equivalent preparation/publication routes at their existing owner, then
   delete the replaced route and its stale tests.
2. **Test and build mechanism inventory.**  Classify every unit/integration
   target by owner, external-asset boundary and filesystem side effects.
   Make side-effect ownership explicit through the existing test/build
   boundary; remove duplicated registration, isolated-workspace or serial
   mechanisms when one declarative route can represent the complete contract.
   Do not weaken the full-unit or task-level integration requirements.
3. **Architecture and documentation reconciliation.**  Remove only prose,
   names and references made obsolete by a converged production mechanism.
   Keep one current authority per topic; historical evidence remains history,
   not a second specification.
4. **Global regression sweep.**  Search all sibling routes of each repaired
   mechanism.  Every hit is either converged, proven to require a distinct
   semantic boundary, or transferred once with a precise admission condition.

## Hard Constraints

- Core remains the sole guest CPU, memory, device, timing and input-state
  owner.  VM/profile composition remains outside Core; platform code remains
  outside guest state.
- A native Console lease, a native Window presenter and a copied guest
  snapshot may remain distinct because their host capability and lifetime
  differ.  They may not become alternate session lifecycle or guest-state
  owners.
- No broad rewrite, global registry, generic plugin/framework layer, test-only
  production interface, compatibility alias or speculative abstraction.
- An abstraction is accepted only if it removes a concrete duplicate route,
  state owner, dependency, or implementation.  Report the deleted and added
  code rather than relying on the size of the local diff.
- External ROM/media integration remains under `test/integration/`; unit tests
  remain repository-only and do not acquire external assets or YAML inputs.

## Planned Subtasks

1. **S1 - evidence-backed mechanism inventory.**  Produce a finite owner,
   route and side-effect inventory for the active source/test/build surfaces;
   classify each candidate as necessary distinction, duplicate mechanism, or
   deferred bounded receiver.  Do not change code before this inventory.
2. **S2 - session/platform convergence.**  Repair every confirmed duplicate
   lifecycle, presentation, action or capture route at its owner, including
   all affected Console/Window/session regressions.  Delete retired paths in
   the same change.
3. **S3 - test/build convergence.**  Consolidate confirmed repeated test
   registration and workspace-isolation mechanisms without reducing coverage
   or serializing independent tests.  Preserve declared external-asset
   integration boundaries.
4. **S4 - closure sweep.**  Re-audit all changed route siblings, verify the
   actual diff and code-size accounting, run complete unit tests after every
   S and the applicable task-level integration suite, then record remaining
   irreducible distinctions or one bounded transfer.

## Completion Standard

The retained mechanisms each have a named owner, one production route and a
real semantic reason to exist.  No known duplicate state, parallel route,
fixed-path test side effect or obsolete compatibility path remains in the
audited surface.  Full unit tests pass for every S; task closure also passes
the applicable integration suite and documentation governance.  The final
review records deleted versus added code and proves that no reduction has
created a new Core/VM/platform reverse dependency or second truth source.
