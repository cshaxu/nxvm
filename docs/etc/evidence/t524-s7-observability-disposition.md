# T524 S7: Shared Observability Disposition

## Decision

`lib/observability` retains exactly one neutral capability: an owned copied
outcome with `sequence`, `status`, and `valid`.  The owner externally
serializes publish, capture, clear, and destruction.  Capture returns a value
copy; clear invalidates the result and restores `OK` without resetting the
sequence; publish advances the sequence, marks it valid, and returns the
published status for direct propagation.

No synchronization primitive enters this root because that would introduce a
peer-root dependency and none of the current consumers concurrently access an
outcome.  No diagnostic event is admitted: the complete consumer sweep shows
that every library-outcome use needs only the three existing copied fields.
Text, traces, paths, and detailed faults remain with their existing product
owners.

## Consumers And Sweep

Two independent public-header neutral consumers retain the capability:
`observability_outcome_smoke` exercises creation, invalid inputs, publish,
capture, clear, sequence retention, and destruction; the standalone
`library_consumer_conformance` consumer compiles and uses the same public
outcome API beside other independent library roots.

The complete `lib_observability_outcome` sweep has one live NXVM binding:
`vm_session` owns a `start_outcome`, publishes start/reset status, and its
callers capture the copied result after the operation boundary.  The two
startup-failure smokes and the run-handle integration proof consume the same
status-only result.  No caller requires diagnostics or an asynchronous
notification route.

## Verification

- Focused outcome and run-handle checks pass.
- Full repository unit: 311/311 pass in 21.45 seconds. Standalone library
  build/CTest, manifest, strict Linux syntax, observability-root
  vocabulary/dependency sweeps, documentation governance and `git diff --check`
  pass.

## Simplicity Result

The root gains contract comments and invalid-input coverage only.  There is
one outcome state owner, one NXVM binding, and no logging facade, event queue,
or duplicate status state.
