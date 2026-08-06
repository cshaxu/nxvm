# M5 T234 S7: Fixture Contract Closure

## Objective

Remove raw mutable session and device borrows from the test fixture without
weakening the existing system and device smoke coverage.

## Contract

The fixture may expose only behavior-level operations, immutable snapshots,
and boolean/assertion helpers. It must not return `core_machine`, device,
transport, control, BIOS, platform-run, or other session-owned object pointers.
It must not cast away `const`.

## Migration Order

1. Define typed snapshots and actions for machine execution and observations.
2. Migrate media, FDC, HDC, CMOS, and debug smoke coverage to assertions and
   explicit actions.
3. Migrate platform transport/window and control smoke coverage to actions and
   snapshots.
4. Remove raw-borrow and legacy-storage helpers, then extend the static gate to
   reject raw fixture pointer returns and all casts from const session handles.

## Exit Conditions

- `vm_session_fixture.h` contains no raw session-owned object pointer return.
- No test includes composition `session.h`.
- The fixture implementation contains no const-removing cast.
- The static boundary gate and all current GCC smoke tests pass.
