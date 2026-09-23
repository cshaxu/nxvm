# T522 S10 Observability Closure Evidence

## Ownership

`lib/observability/outcome` is the sole owner of the generic copied startup
outcome: a publication sequence, a validity bit and a `type_status` value.
VM session lifecycle publishes and clears that one object; consumers capture a
copy. The former VM-local `valid/status` start-outcome struct is deleted.

Core machine trace remains Core-owned because its events and ring have machine
semantics. VM fault outcome and debugger state remain VM-owned because they
carry CPU and product diagnostic meaning. Neither is a duplicate generic
outcome, so this S deliberately creates no generic trace framework or second
trace route.

## Verification

- `unit.observability-outcome-smoke` proves create, publish, copy capture and
  clear while retaining monotonic publication sequence.
- `integration.vm-platform-run-handle-smoke` proves normal VM startup, reset
  clear and restart consume the single outcome owner.
- The dedicated fault-injection configuration
  (`PROJECT_VM_PLATFORM_TEST_FAILURE_STAGE=1`) builds and directly executes
  `vm-platform-win32-startup-failure-smoke` and
  `vm-session-startup-failure-smoke` successfully.
- Complete repository-only unit gate: 308/308 pass.
