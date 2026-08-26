# T474 S1 Core Time Convergence Ledger

`M5:T474:S1:CORE-TIME-CONVERGENCE-LEDGER:OK`

## Frozen Universe

This ledger covers every current production route that can publish, derive,
observe, bound, or host-pace Core guest time. Test-only direct structure access
is excluded from production ownership and remains regression instrumentation.

| ID | Current sole owner / route | Disposition and S receiver |
| --- | --- | --- |
| A1 | `core_machine_publish_elapsed_ticks` in `machine_scheduler.c` is the sole production writer of `machine->elapsed_ticks`. | Retain one writer; S2 records its immutable-plan qualifications and removes any duplicate public semantics. |
| A2 | `machine.c` publishes selected successful CPU retirement cost through A1. | Core-owned; S3 reconciles form/context and exception timing inputs. |
| A3 | CPU BUSRDY/external-cycle waits and D4 slowdown publish through A1. | Core-owned L2/L3 selection boundary; S3 classifies every transaction source and removes no live path until its distinct semantics are represented. |
| A4 | `core_machine_advance_time` and `core_machine_advance_to_next_deadline` call A1 only while Core is paused/stopped. | Retain Core mutation boundary; S2 distinguishes deterministic test/control advance from source-qualified deadline advance. |
| A5 | `core_machine_capture_time_observation_private` composes only source-qualified PIT and RTC deadlines. | Retain copied observation; S4 adds eligible owners only from their admitted rule/plan data. |
| A6 | DMA, KBC, FDC, HDC and VADP local counters advance only from Core scheduler callbacks. | No second clock owner; currently deadline-blocking/ineligible until each owner exposes a source-qualified next observable change. S4 receiver. |
| A7 | Timeline arbitration/readiness/peripheral callbacks advance clocks, controllers and traces after A1 publication. | Core-owned deterministic order; S4 verifies deadline composition never bypasses this order. |
| A8 | Public `core_machine_time_observation` is copied from Core and contains elapsed tick, deadline and qualification/rate only. | Retain opaque value boundary; S2 consolidates plan/observation qualifications without device pointers. |
| A9 | VM `waiting.c` reads A8, records host/Core origins, and waits only in Standard mode; it never writes a tick. | Retain one pacing consumer; S5 proves Standard ahead-only waiting and Turbo no-wait on identical Core semantics. |
| A10 | VM HLT handling requests `core_machine_advance_to_next_deadline`; Core selects and publishes the advance. | Retain Core-owned advance; S5 covers reset, cancellation, pause and debugger boundaries. |
| A11 | Profile/session composition supplies copied `core_machine_time_axis` and controller timing rules during construction. | Profile values are inputs, not time owners; S2 unifies their validation/provenance in the immutable plan. |
| A12 | Platform `Sleep(1)` in pause/control routes and bounded Standard pacing is host control only. | Not guest-time advancement; retain distinct from timing model. The former unconditional HLT fallback is absent. |

## Consequences

There is no production VM-to-Core host tick injection, profile callback time
writer, device-private host clock, or parallel scheduler. The current Core
axis already supplies the only mutation route, but its generic plan and
deadline eligibility are incomplete: CPU/transaction rule provenance remains
distributed, and only PIT/RTC can currently form an observable deadline.

S2 owns consolidation of construction-time rule/provenance data and the copied
observation. S3 owns CPU/transaction inputs. S4 owns controller deadline
eligibility. S5 owns the existing Standard/Turbo consumer proof. A profile may
not receive implementation work until those generic receivers expose the
validated neutral contract.
