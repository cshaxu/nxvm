# T470 S2: Core Time-Axis Contract

`M5:T470:S2:CORE-TIME-AXIS-CONTRACT:OK`

## Result

The existing `machine->elapsed_ticks` remains the sole mutable Core guest
progression counter. `core_machine_time_axis` is immutable copied plan data
that qualifies that same counter only when a complete physical rate has been
proved. It is not a second counter, host source or profile callback.

`core_machine_time_observation` is now the one value boundary for current
Core time, physical qualification and next guest-observable deadline:

| Observation field | Owner and meaning |
| --- | --- |
| `elapsed_ticks` | The sole Core progression counter. |
| `physical_time_available` / `physical_ticks_per_second` | Copied immutable qualification for that same axis; zero/false when unqualified. |
| `next_deadline_valid` / `next_deadline_tick` | Core-composed, source-qualified guest deadline; independently unavailable when a controller blocks it. |

The obsolete `core_machine_pacing_contract` type and
`core_machine_get_pacing_contract()` getter are deleted. VM waiting captures
the copied observation and asks Core to advance only its own next deadline; it
supplies no tick count. This removes a parallel static value path rather than
wrapping it.

## Qualification And Compatibility

Core creation rejects an unknown axis kind and either a zero verified rate or a
nonzero unqualified rate. The positive Core smoke uses a synthetic 8 MHz axis
solely to prove validation and copy behavior. Default PC/AT, Model-339 and
Model-40 all continue to publish unavailable/zero through the same observation,
so runner behavior remains T469's L2 HLT load backoff.

## Verification

- Focused Debug build: `core-machine-time-smoke`,
  `vm-model-339-clock-contract-smoke`, `vm-model40-byob-s20-smoke`, and
  `vm-session-speed-policy-smoke` built successfully.
- Focused regressions: Core time, Model-40 BYOB and VM speed CTest cases pass;
  the direct Model-339 contract smoke passes and prints its T469 deadline
  marker.
- Static sweep finds no `core_machine_pacing_contract`,
  `core_machine_get_pacing_contract`, `core_machine_guest_timebase`,
  `CORE_MACHINE_GUEST_TIMEBASE` or `guest_timebase` production/test reference.
- Documentation governance and `git diff --check` pass.

## S3 Transfer

S3 must qualify CPU normal, deferred-wait, fault/interrupt/halt and transaction
outcomes against this one axis. It may not set a profile rate, create a second
counter or relax T388's unallocated-success prohibition.
