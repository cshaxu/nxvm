# T526 S10 VM Machine Executor Migration

## Disposition

The former `src/vm/composition/session` execution owner is removed.  The one
remaining Core assembly/execution route is `src/vm/machine`:

- its executor FIFO serializes copied input, pause, reset, resume, step and
  stop requests at an execution boundary;
- its one copied result sink publishes lifecycle, debugger completion, fault
  and display facts;
- `vm/events` contains only bounded copied request/result values and the sink
  signature; it contains no Core, executor, session or native-UI object.

The retained `vm/product` adapter converts that one result route for the old
product control surface only.  It is a temporary consumer explicitly removed
by T526 S11; it is not a second machine executor or a second result owner.

## Sweep And Verification

- All former VM composition source and test paths are deleted or moved under
  `vm/machine`; CMake and every affected static authority gate now name the
  actual owner path.
- `ctest --test-dir build/mingw-gcc-x64 -L unit -j 8 --output-on-failure`:
  303/303 passed (169.57 seconds).
- `verify-current-specialized-gates`: 67/67 passed.  The T345 verifier's
  deliberate negative self-test reports its expected diagnostic and then
  passes.
- Documentation governance, VM machine owner/lifecycle, Core debug, and
  controller-boundary gates passed. `git diff HEAD --check` passed.

## Simplicity Accounting

The complete S10 tracked diff is 4,133 additions and 3,818 removals (net
+315), measured with `git diff HEAD --numstat`.  The positive net is the
bounded request/result ABI, FIFO tests, and migration guards; the former
composition execution route, split machine result callbacks, request bridge,
and request transport are removed rather than retained as compatibility paths.
