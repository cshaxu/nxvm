# T375 S6: Model-339 Virtual-Time Source Binding

## Decision And Owner

T375 S5 selected a VM-composition-owned source that supplies already-converted
virtual source ticks. This S binds that capability to `vm_session`; the core
continues to own machine-time publication through its S4
`core_machine_advance_time` operation. The session configuration copies the
descriptor, while the configuration caller retains the provider context for
the session lifetime. This is a normal product composition capability, not a
test-only injection path.

## Activation And Lifecycle Contract

`vm_session_runner_run` is the only production caller of
`vm_session_virtual_time_on_waiting`. It invokes it only after
`core_machine_run` returns `CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT`, with the
session control state still running and no single-step pause pending. The
helper rejects any other stop reason or inactive control state before it calls
a provider. A nonzero successful batch makes exactly one
`core_machine_advance_time` call; a zero batch and an absent provider leave
machine time unchanged. A provider failure stops the session instead of
silently converting host sleep into guest time.

The execution-reset callback invokes the optional provider `reset` callback
before resetting the core. This discards or rebases source-side accumulated
time with the core's reset boundary. Host `Sleep(1)` remains solely a
responsiveness wait when no batch was advanced; it is not a guest-clock
source.

## Focused Proof

`vm-session-virtual-time-s6-smoke` configures a normal session source and
proves all of the following through the public core elapsed-time observation:

| Case | Result |
| --- | --- |
| inactive or non-wait result | rejected before the provider is called |
| active wait with a 37-tick source batch | provider called once and `elapsed_ticks` increases by exactly 37 |
| active wait with a zero batch | provider is called, but no time is advanced |
| runner single-step wait branch | source consumption is excluded before the runner requests its step pause |
| explicit session reset | provider reset callback is invoked once at the reset boundary |
| default session with no source | waiting-path handling succeeds without advancing time |

The build and executions passed on 2026-08-15:

```text
M5:T375:S6:VIRTUAL-TIME-SOURCE:OK
M5:T375:S4:EXPLICIT-MACHINE-TIME:OK
M5:T208:S3:DEFAULT-PC-AT-APPLY:OK
```

## Retained Boundary

This binds a deterministic source seam; it does not choose or implement a
production monotonic-host source, source-rate conversion, batching bound,
PIT/RTC phase model, DMA/FDC service duration, or Model-339 L3 conclusion.
Those require subsequent T375 admission and evidence. No ROM or guest medium
is used by this S.
