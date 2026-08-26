# T470 S1: Physical-Axis Eligibility Ledger

`M5:T470:S1:PHYSICAL-AXIS-ELIGIBILITY:OK`

## Scope And Rule

This is a source-and-owner inventory, not a new timing claim.  A physical
pacing axis needs one unit that covers every Core path that advances or waits
guest time.  Existing controller Manual-L3 facts remain L3 under the project
ledger; this evidence asks the separate question whether their result can be
converted onto one complete Core axis.  It neither implements L4 electrical
timing nor changes current profile eligibility.

## Current Publication And Consumer Ledger

| Row | Current owner/path | Unit or role | Physical-axis disposition and receiver |
| --- | --- | --- | --- |
| 1 | `core_machine_publish_elapsed_ticks()` | Sole `elapsed_ticks` publisher; advances the scheduler after the add. | One required S2 conversion/owner boundary.  The value is mixed today. |
| 2 | `core_machine_run()` successful retirement | Selected CPU form plus external-cycle cost is published after successful retirement. | Blocked by the four profile residual successful routes in T388 S1/S9; S3 receives the complete CPU outcome matrix. |
| 3 | `cpu_retirement_wait_pending` branch | One source tick at a time while an external-cycle wait is pending or BUSRDY holds it. | Source value, wait selection and bus relation must be reconciled together by S3/S4; it cannot inherit the completed instruction's cost. |
| 4 | D4 slowdown branch | One elapsed tick while the selected PIT output holds the CPU. | Model-40-only board input; S4 must establish its source relation before it can qualify a profile. |
| 5 | `core_machine_advance_to_next_deadline()` | Core-selected PIT/RTC deadline delta. | Eligible only when T469's copied deadline is valid; S4 consumes it through the same publisher, never through VM input. |
| 6 | `core_machine_advance_time()` | Explicit Core source-tick advance, used by deterministic Core tests. | Deliberately invalid in T388 physical retirement mode and has no VM production caller.  Retain as deterministic/test-only until S2 gives it a nonphysical disposition or replaces its test role. |
| 7 | Synchronous delivered fault, reset, stop and HLT return paths | Current implementation publishes no elapsed tick on those exits. | Not proof of zero physical duration.  S3 must allocate an Intel/board rule or retain a physical-pacing blocker; the current no-publication behavior stays unchanged in S1. |
| 8 | `core_machine_advance_scheduler()` and timeline | Consumes the sole elapsed input; drives ordered callbacks and provider ratio. | Consumer only, not a clock owner.  S2 must preserve this one route. |
| 9 | DMA/PIT/RTC/KBC/VADP/provider clock domains | Rational conversion from elapsed ticks to local device ticks. | Ratios establish relative device progression only.  They do not establish the elapsed input's seconds-per-unit. |
| 10 | PIT deadline selection | Source-qualified PIT output transition through the copied controller plan. | Candidate S4 contributor whenever no T469 blocker is active; it does not qualify CPU time. |
| 11 | RTC deadline selection | Source-qualified periodic/update IRQ transition through the copied RTC plan. | Candidate S4 contributor under the same condition; it does not qualify CPU time. |
| 12 | DMA, KBC, FDC and HDC active state | T469 blocks a deadline when any such owner has unqualified work. | Explicit blocker, not an estimated deadline.  S4 needs a source-backed owner rule or keeps the profile unavailable. |
| 13 | PIC, VADP, media and provider callbacks | Scheduler-visible state consumers without a currently composed physical deadline. | Their current L3/L2/L1 controller dispositions remain unchanged; S4 must allocate any required physical relation before profile qualification. |
| 14 | `vm_session_waiting_advance()` | Reads copied pacing/deadline values and asks Core to advance only its chosen deadline. | Correct one-way consumer.  Current contracts are unavailable, so it supplies no guest time. |
| 15 | runner `Sleep(1)` after HLT | Host-load backoff after an unavailable Core advance. | L2 host waiting only; it advances no guest state and is not synchronization.  S6 may replace it only for a verified contract. |
| 16 | `vm_platform_host_milliseconds()` | Host display cadence observation. | Presentation-only; it has no Core call and cannot enter the pacing axis. |
| 17 | default PC/AT descriptor | Deterministic one-tick CPU and mostly 1:1/L2 controller ratios. | `UNAVAILABLE`: no selected physical source chain. |
| 18 | IBM 5170 Model-339 descriptor | Sourced PIT/RTC ratios relative to a nominal 8 MHz CPU source. | `UNAVAILABLE`: T388 proves the 80286 successful-retirement input is not fully physical. |
| 19 | Model-40 composition | Deterministic 80386, transaction and 1:1 clock configuration. | `UNAVAILABLE`: no complete CPU/board physical source chain. |
| 20 | `core-machine-time-smoke` synthetic 8 MHz value | Validation-only positive copy test. | Test fixture, not profile evidence and never a product qualification. |

## Profile Disposition

All three current profiles retain `CORE_MACHINE_GUEST_TIMEBASE_UNAVAILABLE`.
The existing positive type accepts only a copied rate; it does not itself prove
that `elapsed_ticks` has that unit.  T388 S9 remains the decisive negative CPU
finding: every current 8086, 80186, 80286 and 80386 profile has a successful
nonphysical route, so no selected profile can map the complete execution axis
to host seconds.

## S2 Boundary

S2 must introduce one Core-owned physical-time representation distinct from
the present mixed `elapsed_ticks` accounting.  It may not add a VM accumulator,
profile callback, controller pointer or parallel scheduler.  It must preserve
the existing single scheduler consumer path and require S3/S4 qualifications
before any profile can publish a nonzero pacing rate.

## Verification

The inventory was checked against the T388 S1/S3/S9 physical-time evidence,
T469 S3--S6 deadline/pacing evidence, current Core publisher/scheduler/run
owners, current VM waiting/runner/display owners, all three session profile
construction paths and the Core time/speed/model contract tests.  Static sweeps
confirm that VM production calls `core_machine_advance_to_next_deadline()` only
through `vm_session_waiting_advance()` and contains no production caller of
`core_machine_advance_time()`.
