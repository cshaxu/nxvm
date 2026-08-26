# T471 S2: Session Ahead-Only Pacing

`M5:T471:S2:SESSION-AHEAD-ONLY-PACING:OK`

## One Owner And One Direction

`vm_session` owns only the pacing baseline: copied Core tick origin, monotonic
host-counter origin and host counter frequency. `waiting.c` owns comparison and
waiting; `virtual_time.c` exposes the counter without a guest-time conversion.
Core still exclusively publishes and advances its elapsed axis.

The exact `vm_session_pacing_ratio_compare` uses Euclidean fraction comparison,
not cross multiplication, so near-`UINT64_MAX` values cannot overflow. A
counter regression, frequency change or failed sample clears the baseline and
takes no Core action. Successful Core reset clears it at the existing session
reset boundary.

## Product Behavior

| Axis and speed | Result |
| --- | --- |
| Verified + Standard | After a normal Core run, compare completed Core time to host budget. Sleep in bounded 1 ms host-control slices only while Core is ahead. |
| Verified + Turbo | Do not establish or wait on the pacing baseline. |
| Unavailable + Standard/Turbo | No pacing comparison or Core advance occurs; the existing runner HLT L2 fallback remains the sole Standard load backoff. |

No session code converts host elapsed time into ticks, calls a generic Core time
advance from host data, or stores a second guest clock.

## Verification

- `current.vm-session-speed-policy-smoke` passes both unavailable Standard and
  Turbo paths, equality/order cases and near-maximum fraction cases.
- `current.vm-runner-display-cadence-smoke` passes after runner pacing insertion.
- The HLT deadline gate is S3: it uses the same comparison before calling the
  existing Core-selected advance API.
