# M5 Core Platform Primitive Initialization And Outcome Contract

## Purpose

Make Core platform primitives valid under strict C11 initialization rules and
ensure failed display capture/publication is never consumed as a valid frame.

## Required Scope

Initialize every relevant `atomic_flag` according to C11, then trace
presentation mailbox capture/publish failures through Core and VM consumers.
On failure, preserve a defined prior frame or return the status; never render
uninitialized capture storage. Keep host policy outside Core.

## Dependencies And Completion

Completion requires strict compile coverage, focused initialization and
capture-failure regressions, a primitive/caller sweep, and current gates.

## Non-goals And Stop Conditions

Do not execute the larger opaque-interface refactor, add generic callback
wrappers, change copied-frame format, or alter host pacing. Stop if an exposed
ABI change is necessary; transfer it to the existing platform-encapsulation
candidate.
