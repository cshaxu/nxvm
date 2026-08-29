# T500 S11 Parallel Recovery

## Finding

The configured four-job unit route already completes in about 16 seconds.
Raising it to eight jobs reduced one observed run only from 16.57 to 16.08
seconds, so this S retains the supported four-job policy.  The material delay
was instead seven independent integration scenarios marked globally serial.

## Resource Sweep

| Existing test | Mutable resource owner | Disposition |
| --- | --- | --- |
| `vm-timer-firmware-smoke` | Its own session and thread; media inputs are read-only. | Parallel. |
| `vm-debug-pause-boundary-smoke` | Its own session/control/thread. | Parallel after replacing the fixed startup sleep with a bounded owner-state wait. |
| `vm-unified-debug-backend-smoke` | Its own session/debug target/thread. | Parallel after the same owner-state wait repair. |
| `vm-dos-prompt-smoke` | Its own session; disk input is read-only. | Parallel. |
| `vm-dos-keyboard-smoke` | Its own session and host-input path; disk input is read-only. | Parallel. |
| `vm-dos-mem-fault-smoke` | Its own session; disk input is read-only. | Parallel. |
| `vm-windows31-checkpoint` | Its own session and copied/temporary output; ROM and disk inputs are read-only. | Parallel. |
| `vm-runner-display-cadence-smoke` | Its own session, but its assertion deliberately samples a 100-ms host-wall-clock presentation window. | Retain the existing narrow serial mark; it is not part of the integration resource sweep and has no material suite cost. |

The seven former integration marks had no shared mutable file, port, input, or
process resource.  Their common cause was a historical fixed `Sleep(10)`
startup heuristic in two pause tests.  Each test now waits only for its own
published running state, with the existing 2-second bounded failure limit.
No product, runner, timeout, asset, or test-inventory behavior changes.
The scheduling metadata loses 20 lines; the two owner-local waits add seven
net test-source lines, rather than introducing a shared helper or scheduler.

## Verification

- Focused seven-test parallel replay: three consecutive 7/7 passes at `-j 4`;
  representative wall time 11.61 seconds versus 28.15 seconds cumulative
  process time.
- Complete integration route: 20/20 pass at `-j 4`, 17.23 seconds wall time.
- Complete unit route: 312/312 pass at the supported four jobs, 15.58 seconds
  wall time.

This is a scheduling recovery, not coverage expansion: no test was added,
removed, reclassified, or given a weaker success condition.
