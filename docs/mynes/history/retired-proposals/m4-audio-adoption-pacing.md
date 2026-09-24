# M4 Neutral Audio Adoption And Pacing

## Outcome

Add a generic MyNes Lib audio stream and connect Core PCM to it with one
stop/wake-aware 1x pacing path. SoftPC is not modified by this task; the
component remains self-contained so a later shared-corpus recovery can take it
back without changing its public contract.

## Acceptance

The stream has copied bounded PCM semantics and independent Lib tests. MyNes
proves active/flush/teardown, no stale paused sound, queue backpressure and
guest-time-preserving pacing. This task changes only MyNes; it has no SoftPC
source or delivery action.
