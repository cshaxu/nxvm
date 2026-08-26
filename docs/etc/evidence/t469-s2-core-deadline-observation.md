# T469 S2: Core Deadline Observation

`M5:T469:S2:CORE-DEADLINE-OBSERVATION:OK`

## Retained Boundary

`core_machine_capture_time_observation()` is the sole public Core-to-VM
guest-time observation introduced by S2. It copies only:

- completed `elapsed_ticks` owned by Core;
- `next_deadline_tick`; and
- `next_deadline_valid`.

The function rejects initialized and running machines, as do the existing
elapsed-time observations. It returns `next_deadline_valid = false` and zero
for the deadline tick on every current profile. This is intentional: the S1
ledger proves that the existing timeline events are recurring arbitration,
readiness and peripheral maintenance callbacks, not source-qualified earliest
guest-observable deadlines.

No Core controller pointer, timeline observation, profile name, mutable state
or callback crosses the boundary. `core_machine_advance_time()` remains the
only explicit composition time-publishing operation; S2 adds no caller and VM
does not consume this observation yet.

## Reset And Cancellation Semantics

A future valid deadline must be cleared or recomputed by its Core owner before
this copied observation may set `next_deadline_valid`. Until then, reset and
cancellation are safe by construction: the observation has no deadline and
reports only the reset Core elapsed tick. It is therefore not a claim that a
scheduled callback is an eligible deadline.

## Verification

`core-machine-time-smoke` proves the observation is unavailable after reset,
remains unavailable after executed guest time, and returns to the reset state.
`vm-session-virtual-time-s6-smoke` and `vm-session-speed-policy-smoke` prove
the unchanged VM path neither consumes the observation nor advances time from
host policy. Documentation governance passes.

## S3 Transfer

S3 must define immutable plan qualification for a future valid deadline and a
verified physical guest timebase. It must reject the present profiles for
host-paced L3 synchronization while the observation remains unavailable. It
must not turn the current false value, a `due_tick + 1` maintenance event, or
host elapsed time into a deadline.
