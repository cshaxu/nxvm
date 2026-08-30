# T504 S3 Core L1 Compatibility Progression

## Decision

`core_machine_time_progress_disposition` remains the only VM-visible
classification.  A Turbo session may request `core_machine_advance_l1_compatibility`
only after that copied value is `L1_COMPATIBILITY`; the request accepts neither a
controller selection nor elapsed ticks.  Core re-observes its own state before
each bounded normal scheduler transition.

The bound is a host-control quantum, not an emulated duration or timing claim.
It prevents the runner from returning after a single Core tick while still
reconsidering immediate work and source-qualified deadlines at every transition.
The existing scheduler gives immediate work, then a known deadline, precedence
over an L1 classification.  Standard never calls the compatibility entry point.

## Frozen Construction Policy

The profile materializers for `default-pc-at` (and IBM 5170), Model 40, and
IBM 5160 set `CORE_MACHINE_L1_COMPATIBILITY_BOUNDED_PROGRESS` once in their
Core configuration.  Core validates and copies that enum at construction; it
is not a runtime session option, timing source, or mutable board register.
Physical-retirement configurations reject the compatibility action.

## Owner Coverage

The sole Core action covers the four S1 candidates without a VM device branch:

| Owner | Normal progression retained |
| --- | --- |
| DMA | Existing transaction/arbitration advance |
| HDC | Existing readiness advance |
| D4 refresh hold | Existing refresh transaction |
| Slave PIC cascade publication | Existing PIC refresh |

None is promoted above L1 and none gains guessed timing.  A known PIT, RTC,
FDC, KBC, timeline, or other published deadline remains the scheduler result
instead of entering this path.

## Verification

- `unit.core-machine-time-smoke` proves an L1 D4 condition reaches the Core
  action and is settled through its normal owner transition.
- Profile tests cover the immutable policy at IBM 5170, Model 40, and IBM 5160
  construction; `default-pc-at` shares the same materializer as IBM 5170.
- Focused Core/profile tests and the complete repository-only unit suite pass.
- No VM pointer, controller identity, host elapsed time, or tick count enters
  the Core action.

S4 remains responsible for product-mode and lifecycle proof.  S5 remains
responsible for external ROM/DOS integration closure.
