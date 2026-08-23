# M5 VM Session Reset And Startup Outcome Propagation

## Purpose

Make VM session control and startup faithfully propagate the Core reset result
and mandatory device-initialization failures. A session must not report a
successful start after its Core machine failed to reset.

## Required Scope

Consume the Core reset-failure contract. Repair `vm_session` control,
lifecycle, and construction/start paths that discard reset or required device
initialization statuses; publish one truthful stopped/failed outcome through
the existing session boundary. Preserve successful session behavior and the
existing ownership direction.

## Minimal Design Constraints

This repair follows the owner's hard minimalism constraint. The existing
session lifecycle owns the one VM-visible outcome; Core continues to own its
machine lifecycle and reset failure state. Do not add a reset-error manager,
parallel startup/reset route, callback wrapper, generic rollback framework, or
second fault model.

The existing reset call chain must carry `type_status` from its callback to the
session lifecycle. `vm_session_start` may resume only after that reset succeeds.
An in-flight reset request remains an asynchronous request, not a fabricated
success: the existing runner publishes its eventual result through the same
session outcome. Existing error-bearing device initialization, including FDD
backing initialization, returns its existing status through the existing
provider/session initialization route; a `(C_VOID)` discard must be removed.

Configuration continues downward from session/profile into Core. Status and
observations return upward. Any helper introduced must replace and delete a
void-returning truncation or duplicated state path; a forwarding helper that
does not eliminate existing code is out of scope.

## Dependencies And Completion

Depends on Core reset and firmware failure atomicity. Completion requires
focused failures for reset, FDD/device initialization, startup retry and
observable session state, a complete caller sweep, and current gates. The
acceptance review must show one session outcome owner, one production reset and
startup path, no false successful resume after a failed reset, and no retained
status-discarding initialization route.

## Non-goals And Stop Conditions

Do not redesign the public session manager, add a second fault model, change
guest media formats, or modify profile policy. Stop if truthful status requires
a public contract redesign; transfer it to the existing session-boundary task.
