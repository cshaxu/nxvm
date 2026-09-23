# T523 S9 Host Clock Owner Ledger

## Disposition

| Former VM mechanism | Sole replacement | Retained policy owner |
| --- | --- | --- |
| `vm/platform/virtual_time.*` | `lib/host/clock.*` | VM session display cadence and Standard pacing comparison. |
| Windows performance counter | `lib/host/clock.c` | No VM or Core native counter reader remains. |
| Linux wall-clock fallback | `CLOCK_MONOTONIC` in `lib/host/clock.c` | Host values remain a limit only, never guest time. |

## Boundary

Core alone advances guest time and publishes completed progress/deadlines. VM
only compares that completed progress with the generic host counter and waits
when Standard runs ahead. Turbo and Core have no host-clock injection route.

## Verification

- Host-clock, display-cadence and speed-policy focused tests pass.
- The production sweep finds no retired VM virtual-time mechanism and no
  direct generic native wall-clock reader outside `lib/host`.
- Repository-only unit: 311/311 pass in 18.87 seconds.
