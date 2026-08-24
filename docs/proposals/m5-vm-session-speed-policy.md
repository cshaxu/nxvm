# VM Session Speed Policy

## Purpose

Expose an explicit per-session host-speed choice for the retained NXVM Console:
`standard` or `turbo`. Turbo is the sole unrestricted mode; it shortens guest
idle waits without changing the Core instruction, device, or event-order
contract.

## Scope

The selected VM session owns one speed policy. `standard` remains the default
and preserves its current profile-selected host-time source or idle sleep.
`turbo` is unrestricted: it never guesses a large time batch, and advances a
halted machine one existing Core tick at a time without a host sleep. It also
retains the shared runner control quantum, so command/debug/display latency
does not become mode-specific. The Console exposes `SPEED` to inspect the
selected session and `SPEED STANDARD|TURBO` to select it while that session is
not running.

## Boundary

Core receives only already-selected virtual time through its existing advance
operation. It does not know speed names, host clocks, Console grammar, or
profiles. VM composition owns the policy, the product Console owns its text
grammar, and the existing selected-session adapter is the only product-to-VM
route. No profile, YAML schema, CPU timing ratio, debugger behavior, or guest
event ordering changes.

This is not a host/guest L3 synchronization contract.  A host clock may pace a
VM loop, but it must not manufacture guest time.  Correct paced execution needs
one source-backed profile timebase plus Core-owned, guest-observable deadlines
from every eligible timed controller; the existing per-tick arbitration,
readiness and peripheral callbacks are not such deadlines.  Until that later
contract exists, Standard preserves its existing source behavior and Turbo
uses the same one-tick Core time path without a host wait.

## Acceptance

Both modes are observable on the selected session. Standard retains the current
source behavior; Turbo advances one Core tick only from an interrupt wait and
preserves Core's ordered event processing. Mode changes are rejected while running.
Focused session and Console regressions, all profiles using the shared runner,
the current gate, documentation governance, and a stripped Release `0.5.0459`
artifact pass.
