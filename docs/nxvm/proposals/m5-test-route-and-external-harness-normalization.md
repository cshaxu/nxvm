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

## Completion Standard

No registered CTest entry is unlabelled, dual-routed, or ownerless.  No
diagnostic resides in an `integration` directory or normal integration
aggregate.  Shared support has a single Core owner and no App-to-peer-App
test include remains.  The final report separates repository-only unit proof,
desktop proof, diagnostics, and external integration proof.
