# T531 S8 Closure

## Accepted result

S8 has one Common reuse contract, with no NXVM-specific alternate route:

- NXVM parses and copies the startup `console_control` value, and Common
  Session/UI owns the resulting Console/Window presentation behavior.
- `common_machine` exposes one optional empty-to-nonempty readiness wake after
  its copied FIFO publication. It wakes a product's existing executor only;
  it neither carries a request nor creates a Common worker.
- The paused Debug boundary remains the synchronous typed Common adapter API.
  NXVM uses its paused Core boundary directly; a future SoftPC adapter must
  synchronously relay to its CCPU-affine rendezvous behind the same API.

The implemented source/test surface is 14 tracked paths: 308 added and 43
removed lines (`git diff --numstat 976f75df^..1dc80782 -- src test`). The
retained production owners are Common Session/UI for presentation policy,
Common Machine for copied requests, and each product for its one executor.

## Review and proof

- P1 recorded the parser, copied composition and presentation matrix in
  [the console-control evidence](t531-s8-nxvm-console-control-p1.md).
- P2/P3 recorded the unchanged synchronous Debug boundary in
  [the executor-contract evidence](t531-s8-common-debug-executor-contract.md).
- P4--P7 recorded the readiness-wake ordering plus independent Common x64/x86
  builds in [the readiness-wake evidence](t531-s8-common-machine-readiness-wake.md).
- Coordinator actual-diff review confirmed that the wake is post-copy and
  optional, NXVM binds no duplicate executor hook, and no Common code names a
  product or native platform API.
- The complete repository-only unit suite passed: 299/299, `ctest -L unit -j
  8`, real time 16.14 seconds. The Common manifest verifier and independent
  x64/x86 Common builds also passed.

## Remaining task boundary

T531 stays open at the owner's direction. The real SoftPC CCPU-affine Debug
receiver and its two-product execution evidence are not present in NXVM and
are not represented as complete. This is a future admitted S boundary, not a
fallback path or a claim that SoftPC has been modified.
