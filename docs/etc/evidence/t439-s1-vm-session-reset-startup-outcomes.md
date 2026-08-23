# T439 S1: VM Session Reset And Startup Outcomes

## Boundary

The existing VM session lifecycle owns the one VM-visible reset/startup
outcome. The execution callback now returns the Core reset status through the
existing execution context and control path. `vm_session_finish_reset` is the
single completion point: it records a failed outcome, or clears the old outcome
and performs the existing successful-reset display publication. `vm_session_start`
therefore resumes only after a synchronous reset succeeds. The runner sends an
asynchronous reset completion through that same point.

Required FDD backing initialization now returns its existing failure through
the provider initialization path instead of being discarded. Construction keeps
its established reset path, now with the result checked and returned. No public
session-manager contract, second failure object, reset wrapper, or rollback
framework was added.

The default firmware writes its HDD parameter table only when the media query
reports a present hard disk. This makes the existing no-HDD Model 339 reset
truthful: it no longer attempts an unnecessary write into the absent high-memory
backing at `F000:E431`.

## Focused Proof

`vm-session-initialization-atomicity-smoke` proves an injected default-firmware
reset fault reaches `vm_session_start`, leaves the session stopped with the
existing outcome set, and prevents resume. Restoring the firmware context and
resetting succeeds and clears the outcome. It separately drives the runner's
in-flight reset request through the same failure and retry sequence, and proves
invalid FDD geometry aborts initialization without a live Core machine or run
handle. Its output includes:

```
M5:T439:S1:SESSION-RESET-OUTCOME:OK
```

The affected Model 339 composition, CGA topology, firmware/FDC topology and
console lifecycle smokes pass after the no-HDD firmware correction.

## Caller Sweep

`vm_session_execution_context_reset` has one production caller at synchronous
control reset and one at runner completion. Both now return or forward the
status to `vm_session_finish_reset`; the latter is intentionally asynchronous
but has no separate outcome. The direct test-support control-reset callers do
not expose a VM lifecycle result and keep their existing local assertions.

`vm_session_machine_devices_initialize_media` has one production caller,
provider initialization, which returns its status to session initialization.
There is no remaining production void-returning reset callback or discarded FDD
initialization result.

## Artifact

The developer artifact target is `vm-0-5-0439`; copied artifact
`build/output/nxvm_0_5_0439.exe` has SHA-256
`CDBA47D65F97C8BD4764BD00F8CF80D19AEE4C7732AADD3BE5E66B0628BE412D`.
