# T470 S6: Standard And Turbo Policy

`M5:T470:S6:STANDARD-TURBO-POLICY:OK`

## Product Decision

All current product profiles remain `UNQUALIFIED` with a zero physical-axis
rate by the S5 proof.  Consequently, neither mode can honestly use host
elapsed time as guest time or offer wall-clock-accurate pacing.

| Condition | Standard | Turbo |
| --- | --- | --- |
| Normal Core execution | Runs the identical Core execution and device path. | Runs the identical Core execution and device path. |
| HLT with an unavailable physical axis | Retains `core_platform_sleep_milliseconds(1u)` as an explicit L2 host-load backoff. The sleep does not write or advance Core time. | Does not take that sleep. No host clock, accumulator, or batch tick advances Core time. |
| Verified physical axis (no current product profile) | This task does not enable a claim or a host-time conversion. The copied observation remains the only possible receiver until a profile closes its full physical chain. | Same Core ownership and deadline rules; only a future host wait may differ. |

The runner condition is deliberately at the single HLT fallback site:
`!advanced && session->speed == VM_SESSION_SPEED_STANDARD`.  It does not
change the pause/debugger wait path, which is host control rather than guest
time pacing.

## One-Way Boundary Review

- `vm_session_waiting_advance` captures the copied Core observation and returns
  without advancing when `physical_time_available` is false.
- No VM runner, waiting, platform or product path calls a Core time-advance API
  from host elapsed time, a sleep duration, or an accumulator.
- `vm_platform_host_milliseconds` remains display-cadence support only; it has
  no Core time-advance consumer.
- The dormant verified-axis deadline receiver remains Core-selected and is not
  evidence of a current profile's physical pacing qualification.

## Focused Verification

- `current.vm-session-speed-policy-smoke` passes for Standard and Turbo with
  the unavailable Core observation: neither changes elapsed guest time.
- `current.vm-runner-display-cadence-smoke` passes after the runner policy
  change.

The S6 closure audit records the release build, full closure reconciliation and
the required future receiver for physically paced Standard mode.
