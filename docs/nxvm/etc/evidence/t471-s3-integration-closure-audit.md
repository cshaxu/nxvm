# T471 S3: Integration Closure Audit

`M5:T471:S3:INTEGRATION-CLOSURE:OK`

## Direction Of Time

| Owner | Responsibility |
| --- | --- |
| Core | Publishes copied elapsed ticks/rate and selects every advance/deadline. No VM or host counter writes Core time. |
| Session pacing state | Retains one host-counter/Core-tick origin only for rational comparison; it is cleared at successful reset and invalid host samples. |
| Standard | Waits in bounded host-control slices only while completed verified Core time is ahead of budget. An HLT deadline is not requested until the same budget reaches it. |
| Turbo | Uses the same Core execution and deadline route but omits the host wait. |
| Current profiles | Remain unqualified and retain only the existing non-advancing L2 HLT backoff. |

The exact comparison uses Euclidean rational reduction, so no product path uses
unsafe cross multiplication, nominal MHz, a host tick accumulator or rounded
milliseconds as guest time.

## Proof And Release

- The synthetic Model-339 Core fixture passes Standard paced and Turbo
  no-wait selected-deadline cases without promoting the profile to physical.
- `current.vm-session-speed-policy-smoke` and
  `current.vm-runner-display-cadence-smoke` pass.
- Target/banner: `vm-0-5-0471` / `0.5.0471`.
- Stripped Release: `build/output/nxvm_0_5_0471.exe`.
- SHA-256: `38D78E1C1AE6B4E877B116B3E3EBF92B6F8E83F378CA9D5075C4871955E8D2FD`.

The existing profile physical-timebase TODO remains the sole receiver for
making a real current machine eligible for physical pacing; it cannot be
closed by this host-wait mechanism alone.
