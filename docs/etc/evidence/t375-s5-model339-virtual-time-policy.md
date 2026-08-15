# T375 S5: Model-339 Virtual-Time Source Policy

## Selected Policy

The existing VM platform has sleep/wait facilities only. It has no monotonic
clock capability, no elapsed-duration-to-source-tick conversion owner, and no
replay clock. `Sleep(1)` in the runner is a responsiveness wait, not a measured
duration and not evidence for eight thousand Model-339 source ticks. It is
therefore rejected as a time source.

The selected policy is an injected **VM composition virtual-source provider**:
it yields already-converted, nonzero Model-339 source-tick batches. Composition
is its sole consumer and calls S4's `core_machine_advance_time()` only while
the runner is active, halted/non-retiring, and not paused/resetting/stopping.
Core receives no host duration and retains the sole mutation/scheduling path.

| State | Source-batch policy |
| --- | --- |
| ordinary CPU retirement | no provider batch; accepted CPU cost remains the publisher |
| HLT / no delivered interrupt | composition may obtain one bounded provider batch and publish it once |
| explicit paused, single-step pause, stop, display yield | do not obtain or publish a batch |
| reset | discard pending batch and rebase provider before the next run |
| provider zero / unavailable / overflow | publish nothing; preserve guest state and yield according to a bounded later contract |

The provider interface must be injectable: a production platform implementation
may sample a monotonic duration and apply the selected profile's nominal source
rate with documented rounding/clamping, while tests and deterministic replay
provide an ordered fixed batch stream. Those are two implementations of one
composition capability, not a test-only core API. The source record or replay
fixture must capture each published batch exactly; host pacing is not a board
fact and cannot substitute for that record.

## Ownership And Next S

S6 must add the narrow provider contract and session-owned state, bind a fake
batch provider for focused replay, and add a production provider only when a
platform monotonic-clock capability is separately bounded. It must prove:

- HLT advances only batches explicitly supplied by the provider;
- one batch reaches the sole core publication path exactly once;
- pause/reset/stop discard/rebase correctly; and
- the existing default profile does not silently acquire Model-339 timing.

No current replay/log artifact exists, so S6 must make the injected batch
sequence observable in its focused proof; durable recording policy remains a
later product/replay receiver. This is sufficient to prevent the former hidden
host-loop cadence, but it is not yet an elapsed real-time implementation.

No ROM, media, binary, source import, 86Box/MAME/PCjs observation or host-clock
scalar was used. The selection does not claim Model-339 L3.
