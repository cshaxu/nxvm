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

## Dependencies And Completion

Depends on Core reset and firmware failure atomicity. Completion requires
focused failures for reset, FDD/device initialization, startup retry and
observable session state, a complete caller sweep, and current gates.

## Non-goals And Stop Conditions

Do not redesign the public session manager, add a second fault model, change
guest media formats, or modify profile policy. Stop if truthful status requires
a public contract redesign; transfer it to the existing session-boundary task.
