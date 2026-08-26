# T472 S2: One Pacing Path

`M5:T472:S2:ONE-PACING-PATH:OK`

## Retained Direction

Core publishes copied elapsed time, rate qualification and earliest valid
deadline. The session records only a host-origin/Core-origin comparison.
Standard waits when completed Core progress is ahead of its selected macro or
physical rate; Turbo omits that wait. On HLT, the same comparison gates Core's
own deadline advance. No host operation creates, compensates or writes guest
time.

`MACRO_PROPORTIONAL` is accepted by the same path as verified physical pacing,
but its physical observation remains false and it cannot change retirement
eligibility. Unqualified profiles return from pacing without a host wait.

## Fixed-Wait Sweep

`rg -n -C 1 "core_platform_sleep_milliseconds(1u)" src/vm src/core` found
three production uses:

| Location | Disposition |
| --- | --- |
| `vm/composition/session/runner.c` HLT path | Removed. No profile receives the former fixed Standard HLT backoff. |
| `vm/composition/session/control.c` pause acknowledgement | Retained: host-control responsiveness, not guest pacing and it cannot advance Core time. |
| `vm/composition/session/runner.c` paused command loop | Retained: host-control responsiveness, not guest pacing and it cannot advance Core time. |
| `vm/composition/session/waiting.c` selected pacing deadline | Retained only when the lead is at least one millisecond. The final sub-millisecond interval yields instead, avoiding the former mandatory one-millisecond oversleep. |

The platform-local yield has one responsibility: permit host scheduling during
the final pacing interval. It owns neither guest state nor time calculation.

## Focused Proof

- `vm-model-339-clock-contract-smoke` proves selected Model-339 Standard
  macro pacing and Turbo's no-wait deadline path on the same macro axis.
- `current.vm-session-speed-policy-smoke` and
  `current.vm-runner-display-cadence-smoke` pass.
- The static sweep above leaves no runner HLT sleep and no VM-to-Core time
  writer.
