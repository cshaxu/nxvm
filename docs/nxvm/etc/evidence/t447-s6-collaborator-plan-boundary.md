# T447 S6 collaborator and plan boundary

## Result

Core now owns the mutable machine-construction collaborators.  The public
media registry, display-provider slot, and machine plan are opaque handles;
their layouts are private to Core.  A plan carries copied declarative topology
only.  Media, display, FDC terminal observation, memory-device callbacks, and
D4 parity endpoints enter it through bounded Core registration operations.

Default PC/AT, IBM 5170 Model 339, and Model-40 composition create the three
handles, register their declarative and endpoint inputs, create the machine,
and destroy in reverse dependency order.  A failure takes the same rollback
route.  There is no VM-owned Core layout or second initialization path.

## Removed routes

- Public `core_machine_media_registry`, `core_machine_display_provider_slot`,
  and `core_machine_plan` layouts.
- Public plan storage for callback/owner pairs, collaborator pointers, and the
  D4 parity mask pointer.
- Stack-created collaborator and plan fixtures in the affected Core and VM
  tests; they now use the production create/destroy operations.

## Verification

- Full `cmake --build build/mingw-gcc-x64 -j 4` completed.
- `ctest --test-dir build/mingw-gcc-x64 -L current-gate -j 4 --output-on-failure`
  completed successfully.
- Focused media, HDC, Model-40 composition, session-atomicity, and Core plan
  smokes passed.
- `verify-t447-collaborator-plan-boundary` passed and emits
  `M5:T447:S6:COLLABORATOR-PLAN-BOUNDARY:OK`.
- Current `vm-0-5-0447` artifact SHA-256:
  `730D47609C8978838990254D467729977C315CA1ABFF8ECB5885318CCD83A4F3`.

## Minimalism accounting

One Core-owned handle route replaces every exposed mutable layout.  The new
registration functions are capability-specific construction operations, not a
generic callback framework; they delete the former public storage and avoid
parallel VM lifecycle logic.
