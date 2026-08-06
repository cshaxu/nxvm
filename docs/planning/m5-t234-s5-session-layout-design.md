# M5 T234 S5: Session Layout Encapsulation Design

## Scope

The VM session currently exposes its complete aggregate layout to composition
and tests: machine, devices, platform handles, debugger, media paths, and
control state. This design preserves the one session/runtime path while making
the production session an opaque composition handle. It does not implement the
migration or change runtime behavior.

## Target Boundary

`vm/composition/session/session_interface.h` becomes the public composition contract:

- `typedef struct vm_session vm_session;`
- `vm_session_config`, reset-vector data, create/destroy, reconfiguration,
  media-operation, and other deliberately stable operation contracts.
- No complete struct, device type, platform run handle, debugger context, or
  internal construction/control helper.

`vm/composition/session/session.h` owns the full struct and internal
helpers such as storage initialization, request consumption, and direct
provider binding. Only `src/vm/composition/**` may include it in production.

## Test Fixture Contract

Tests may include `session.h` when they test composition implementation state
directly and do not create mirror state or an alternate runtime route. A helper
is optional, not a required facade: it is admitted only when it removes
repeated setup or makes a behavior assertion clearer. New production APIs are
not added solely to support white-box tests.

## Migration Order

1. Introduce the opaque public header and private layout header without moving
   ownership or changing session initialization.
2. Move all composition implementation files to the private header; retain the
   existing public lifecycle contracts.
3. Add a test helper only where it removes repeated setup or clarifies a real
   behavior assertion; direct implementation access remains valid otherwise.
4. Move internal-only declarations out of the public header.
5. Add a static gate: no non-composition production source includes
   `session.h`; `session.h` contains the complete `vm_session` definition.
6. Run GCC current gates and all relevant session, media, debugger, and
   platform smoke tests. No test-only runtime route, mirror state, or selected
   session global is permitted.

## Exit

The public API is opaque, production composition remains the sole owner of the
complete layout, tests use direct implementation access or a justified compact
helper, and the mechanical gate plus GCC smoke matrix pass.
