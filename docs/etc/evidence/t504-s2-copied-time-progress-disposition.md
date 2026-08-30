# T504 S2 Copied Time-Progress Disposition

## Value boundary

`core_machine_time_observation` now carries one copied enum value:

| Value | Meaning | VM authority |
| --- | --- | --- |
| `IDLE` | No Core-owned progress is currently available; the wait is external or otherwise has no eligible Core path. | Yield only. |
| `DEADLINE` | Core has a source-qualified future deadline. | Existing deadline path only. |
| `IMMEDIATE` | Core has already-due work, including a timeline event or FDC transition. | No direct action in S2; later Core logic must settle it before any compatibility path. |
| `L1_COMPATIBILITY` | No known work is earlier and one of S1's four causal-only owners is pending. | No direct action in S2; later Turbo-only Core path may request its bounded progression. |

The enum carries neither controller identity nor a pointer, tick count, device
state, or request. It is a copied Core result, so VM cannot select elapsed guest
time or mutate an owner.

## Producer precedence

The Core scheduler classifies all known source-qualified work before its old
fast-advance block. Already-due work wins, then a future deadline, then the
finite L1 set from S1 (DMA, HDC, D4 refresh, unpublished slave-PIC cascade),
then idle. The existing `next_deadline_valid` publication remains unchanged in
this S: a causal block still suppresses fast advance. Thus the new value exposes
the correct later decision without altering Standard, Turbo, or scheduler
progression today.

Zero-delay provider results are classified as immediate rather than discarded
as an absent deadline. This applies to clock-converted providers and the XT
keyboard path as well as absolute timeline/FDC due ticks.

## Focused proof

- `machine_time_smoke`: no Core work reports `IDLE`.
- `core_machine_plan_smoke`: a programmed PIT edge reports `DEADLINE`.
- `machine_timeline_s2_smoke`: a due-now timeline event reports `IMMEDIATE`.
- `machine_d4_refresh_hold_smoke`: a blocked D4 causal hold reports
  `L1_COMPATIBILITY` without a published deadline.

The four focused unit tests pass. Full unit and documentation governance are
required before S2 acceptance.
