# Test Route And External Harness Normalization

## Goal

Make every NXVM and MyNES test belong to exactly one execution route:
`unit`, `integration`, or `diagnostic`.  A route label is not a component
owner: every test also keeps its actual owner label.  This makes the generic
aggregates complete, permits focused owner runs, and prevents diagnostics from
silently disappearing behind an `integration` name.

The result uses the same shared-component test vocabulary as SoftPC where it
is already precise, but closes the unlabelled-static-test gap that exists in
both repositories.  SoftPC can import the resulting Shared test-only changes
without a product dependency.

## Scope And Boundaries

* `unit` is repository-only and has no external firmware, media, INI, native
  desktop, or deployed executable dependency.
* `integration` proves a selected App through its actual configuration and
  external inputs.  It remains under that App unless the test itself is a
  genuine four-PC-family Core contract.
* `diagnostic` is opt-in evidence, probe, admission, or exploratory work.  It
  never claims integration qualification and is not run by the normal
  integration aggregate.
* Shared external-PC harness code belongs in `test/core/setup`; it is test
  support only, never a production App dependency.  App-local cases stay
  beneath their matching `test/app-*/{unit,integration,diagnostic}` owner.
* `test/core/{setup,integration,diagnostic}` is used only when the support or
  test has one real Core/four-PC-family owner.  No App test includes another
  App test tree.

No production behavior, public API, firmware, media, INI, snapshot, or
artifact is changed by this task.  Do not add a second test framework or
replace real external integration with unit mocks.

## Convergence Ledger

The finite universe is every CTest registration produced by each supported
x64/x86 build graph, plus every registered external harness source beneath
`test/`.  Each row has exactly one disposition: `unit`, `integration`,
`diagnostic`, or removed as an exact duplicate/stale registration.  The
registration verifier becomes the durable mechanical proof; the task evidence
records the resulting per-owner counts and every moved diagnostic/support
path.

## Sequential S Plan

| S | Target(s) | Objective and exit proof |
| --- | --- | --- |
| S1 | Shared, NXVM | Establish the exact route-label verifier, repair the Core manifest receiver and the Product/Core duplicate registrations, and normalize misleading unit names.  Every Shared/Core registered test has one route and owner label; x64/x86 static/unit receivers pass. |
| S2 | MyNES | Give all MyNES unit and integration tests the same explicit route/owner labels, establish its `unit` layout contract, and retain product-local runners only as selectors.  MyNES x64/x86 unit and route gates pass. |
| S3 | NXVM | Rehome neutral four-PC external harness support to `test/core/setup`; move opt-in probes to App/Core `diagnostic`; retain actual App integration cases.  Add a boundary gate rejecting peer-App test-support includes. |
| S4 | Shared, NXVM, MyNES | Reconcile all aggregate counts against the ledger, run Shared then Core then every App's complete unit route on x64/x86, and record any external-integration execution plan.  Do not call this an external qualification run. |
| S5 | NXVM, MyNES | Run each selected external integration group once after unit qualification, report diagnostics separately, and close only if every route and retained external input has an explicit result or owner-approved transfer. |
| S6 | Shared, NXVM | Audit the current SoftPC public six-component update for raw import eligibility, component ownership, Types usage, dependency direction, test closure and minimality.  Do not import or modify code without a separate owner-approved implementation scope. |
| S7 | Shared, NXVM | Import SoftPC commit `2f706c37` verbatim for `src/{lib,emulator,product}`, `test/{lib,emulator,product}` and `test/register.cmake`; retain NXVM's established explicit 30-second static-test budgets plus the proven `lib.types-layout-selftest` 180-second and `emulator.verifier-negative` 60-second fixture budgets. Reconcile every affected CTest reference atomically to canonical `lib.*` and source-path identities, then prove shared x64/x86 suites and manifests. |
| S8 | Shared, NXVM, MyNES | Import SoftPC commit `c7b5c3a8` verbatim for the public-six delta since S7: the Win32 Console deactivate flush, its failure-injection regression, and the committed Lib/Emulator test-budget metadata/manifests. Exclude all later SoftPC worktree changes. Prove source/test manifests, boundary gates, and the Shared Lib, Emulator and Product unit routes on x64 and x86; because this changes Shared production input, rebuild, verify and deploy every receiving App's x64/x86 artifact pair. |

## Completion Standard

No registered CTest entry is unlabelled, dual-routed, or ownerless.  No
diagnostic resides in an `integration` directory or normal integration
aggregate.  Shared support has a single Core owner and no App-to-peer-App
test include remains.  The final report separates repository-only unit proof,
desktop proof, diagnostics, and external integration proof.

T550 may close only after one complete reusable x64 build and one complete
reusable x86 build have compiled the whole repository: Lib, Emulator, Product,
Core and every App.  Each App artifact must be rebuilt from those configured
graphs while reusing already-built shared/component targets; an App build must
not independently rebuild a second component graph.  Then every registered
route owned by every component and every App in each of `unit`, `setup` and
`integration` must pass on both widths.  A category with zero registered routes
must be explicitly recorded as zero; it is never silently inferred.  A focused
selection, a compile-only result, diagnostics, or manual desktop evidence
cannot substitute for a required route.
