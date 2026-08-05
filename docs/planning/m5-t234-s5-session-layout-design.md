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

One test-only fixture implementation may include `session.h`. It
exports focused probes grouped by observable need, for example machine borrow,
media state, control state, platform-run state, and selected diagnostics. A
test may request a named observation or test action; it may not borrow the
whole `vm_session`, its raw device structs, or a generic field selector. New
production APIs are not added solely to support white-box tests.

## Migration Order

1. Introduce the opaque public header and private layout header without moving
   ownership or changing session initialization.
2. Move all composition implementation files to the private header; retain the
   existing public lifecycle contracts.
3. Add the test fixture target and migrate tests by capability family: core
   machine, media/device, control/debug, then platform/run-handle observations.
4. Remove direct test layout access and move internal-only declarations out of
   the public header.
5. Add a static gate: no non-composition production source includes
   `session.h`; only the approved fixture implementation may do so;
   `session.h` contains no complete `vm_session` definition.
6. Run GCC current gates and all relevant session, media, debugger, and
   platform smoke tests. No test-only runtime route, mirror state, or selected
   session global is permitted.

## Exit

The public API is opaque, production composition remains the sole owner of the
complete layout, every former direct test field access has a narrow fixture
replacement, and the mechanical gate plus GCC smoke matrix pass.
