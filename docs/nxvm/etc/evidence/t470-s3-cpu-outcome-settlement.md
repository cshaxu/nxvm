# T470 S3: CPU Outcome Settlement

`M5:T470:S3:CPU-OUTCOME-SETTLEMENT:OK`

## Result

S3 keeps `core_machine_publish_elapsed_ticks()` as the sole mutable-time
publisher and makes physical publication a closed CPU-only boundary.  A Core
created with `CORE_MACHINE_RETIREMENT_TIME_PHYSICAL` must also carry a copied
`CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL` rate.  The successful-retirement
seam checks that same immutable axis again before it publishes elapsed time,
so a later internal contract mutation cannot turn an unqualified machine into
a physical publisher.

Until S4 supplies an exact transaction/deadline disposition, the sole
publisher rejects every non-retirement publication in physical mode.  This is
a narrow safety boundary, not a second scheduler or a synthetic wait cost.
Deterministic mode keeps its existing CPU, wait, deadline and test behavior.

## Outcome Matrix

| Outcome | Current owner/path | Physical disposition | Receiver |
| --- | --- | --- | --- |
| Successful immediate retirement | CPU selector, eligibility capture, successful-retirement seam, sole publisher | Admitted only with a verified axis, an allocated source form and matching copied qualification key. | S5 can select a profile only after its complete CPU ledger is closed. |
| Successful retirement after an external-cycle wait | Existing deferred-wait state then successful-retirement seam | The CPU part remains subject to the same admitted rule; each interim wait tick is rejected in physical mode. | S4 transaction settlement. |
| CPU BUSRDY-held wait | Existing deferred-wait branch | Rejected before it can publish an inferred physical tick. | S4 board/bus relation. |
| D4 slowdown | Existing PIT-gated non-retirement branch | Rejected before it can publish an inferred physical tick. | S4 Model-40 board relation. |
| Core-selected controller deadline | `core_machine_advance_to_next_deadline()` through the sole publisher | Rejected in physical mode until the selected deadline and every blocker have a source-qualified S4 disposition. | S4 controller settlement. |
| Explicit deterministic advance | `core_machine_advance_time()` | Already invalid for physical retirement and remains test/deterministic-only. | No physical receiver. |
| Synchronous fault/interrupt delivery | Executor delivery exit without successful retirement publication | No zero-duration claim is added; it remains nonpublishing and blocks a profile until its admitted CPU outcome rule exists. | CPU residual ledger / later CPU admission. |
| HLT, reset, stop and cancellation | Lifecycle exits without successful retirement publication | No zero-duration claim is added; lifecycle semantics remain nonpublishing. | S4/S5 only if a selected physical profile needs a source-backed relation. |

The residual successful-form universe remains the finite T388 S4 ledger.
Its prefix, range-only arithmetic, context, x87 and 80386 target-lexeme rows
remain physical blockers; S3 neither estimates nor relabels them.  No current
profile changes from unavailable.

## Code And Focused Proof

The changed production paths are `machine.c` and `machine_scheduler.c`:
creation and the successful-retirement seam bind physical retirement to the
copied axis, while the existing publisher rejects non-retirement physical
publication.  The retained path is therefore CPU outcome -> sole publisher ->
existing scheduler; there is no host input, extra counter, forwarding wrapper
or profile callback.

Focused Debug build and CTest pass for:

- `core-machine-time-smoke` (including invalid physical-retirement/no-axis
  construction);
- `core-machine-instruction-timing-smoke` and
  `core-machine-instruction-timing-ledger-smoke` (qualified and rejected CPU
  retirement); and
- `core-machine-retirement-observation-s3-smoke` and
  `core-machine-prefetch-locality-smoke` (including the deferred external-wait
  physical-publication block).

The tracked source/test diff is 36 added and 2 removed lines by
`git diff --numstat` excluding documentation: 12 added/2 removed production
lines and 24 focused test lines. The positive count is the two required
validation predicates and their direct regression coverage; it replaces no
duplicate path because the pre-existing publisher remains the only production
route.

## Transfer

S4 must classify each non-retirement publisher by source-backed transaction,
board or controller-deadline ownership before it can be permitted on a
physical axis.  S5 remains the only profile-rate selector and must keep all
current profiles unavailable unless this matrix and the T388 residual ledger
are both closed for that profile.
