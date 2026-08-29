# T499 S2 Core One-Seam Scheduler

## Implemented Boundary

`core_machine_publish_elapsed_ticks()` remains the sole writer of
`machine->elapsed_ticks`.  It now advances to the requested target through one
Core-private loop.  Each iteration selects the earliest eligible copied
deadline. With no eligible deadline, an idle machine settles directly at the
already-published target; exactly one source tick is used only while an active
L1 owner blocks safe fast advance. At every selected point it settles, in
order:

```text
timeline input -> DMA/PIT/PIC -> FDC/HDC/RTC -> KBC/XT keyboard/VADP
```

The old reset-time roots `core_machine_arbitration_tick`,
`core_machine_readiness_tick`, and `core_machine_peripheral_tick` are deleted.
There is no profile, VM, renderer, execution-provider, or controller-private
elapsed-time writer. The VM execution provider retains reset only in the
product; its optional completed-delta callback is null.

## Deadline And Blocker Rule

- PIT, RTC, the timeline, FDC's existing absolute due fields, KBC remaining
  counts and the XT keyboard's existing event query participate as earliest
  candidates at their retained provenance.
- A frozen L2 PIT or RTC ratio remains eligible for Core-local deadline
  composition.  L2 labels the source precision; it does not turn a programmed
  device into an unschedulable poll.  `core-machine-plan-smoke` programs an
  L2-plan PIT and proves its first deadline is published and advanced at Core
  tick one.
- DMA transfer service, HDC active phases, D4 refresh hold, PIC slave
  publication and FDC's pending command/complete phase are not assigned a
  fabricated duration.  They block HLT fast advance and are advanced one
  source tick only on normal Core CPU progression.
- An idle machine has no maintenance event. It cannot reintroduce a hidden
  per-tick loop merely because the caller published a multi-tick Core delta.
- The implementation does not add a host clock path, a VM/profile timing
  setter, a controller pointer outside Core, a second timeline, or a generic
  scheduling framework.

## Runner Control Budget

The VM runner's normal 256-instruction quantum had left its tick budget
unbounded.  An active L1 wait could therefore monopolize one host runner call
despite the intended control-turn boundary.  Normal execution now applies the existing 256-unit runner quantum
to both instructions and Core ticks; the current CPU timing declarations top
out at 106 ticks for one instruction, so this does not split a declared
instruction.  Core preserves the incomplete wait state and resumes it on the
next call.
This does not publish, convert, skip, or otherwise alter guest time.  A
debugger single-step remains unbounded by this host-control ceiling so it can
still complete exactly one instruction.

## Retired Readiness Polls

FDC media generation and ready state are external input, not elapsed controller
time. The scheduler no longer calls `core_machine_fdc_refresh()`; the guest's
existing DIR read is the Core-owned observation boundary. `core_machine_hdc_refresh()`
was an empty maintenance path and is deleted with its unused development-trace
events. FDC command state and HDC active phases remain with their existing
owner-local advance/blocker disposition.

## Peripheral And Provider Disposition

The retained VADP ledger classifies current raster timing as L2/unsupported:
no selected profile supplies a source-qualified raster deadline. VADP remains a
Core-owned delta consumer and cannot wake HLT or manufacture a deadline. The VM
execution provider supplies reset only; its former per-CPU `refresh` route
reached empty FDD/HDD functions and is deleted. Its `advance_time` callback is
null. A future callback receives completed Core delta after settlement and does
not own guest time, so it is not a scheduler blocker or a second clock.

## Static Sweep

```text
rg -n "core_machine_(arbitration_tick|readiness_tick|peripheral_tick)" src tests CMakeLists.txt cmake
```

The query has no production hit. A separate search for `due_tick = ... + 1`
finds only the guarded active-L1 branch in the private Core seam. It runs only
while an active causal owner has no admissible duration; it is not a timeline
token or idle maintenance callback. Remaining timeline scheduling is limited
to the timeline's own API and its focused test; no production caller installs
a periodic maintenance token.

## Focused Verification

The existing Release-probe build compiled, then the following executables all
returned zero:

```text
core-machine-timeline-s2-smoke
core-machine-scheduler-smoke
core-machine-hlt-s49-smoke
core-machine-fdc-smoke
core-machine-kbc-controller-smoke
core-machine-rtc-cmos-s3-smoke
core-machine-pit-waveform-smoke
core-machine-dma-channel-smoke
```

The updated timeline regression proves reset leaves `pending_events == 0` and
`next_sequence == 0`; it also retains same-due insertion order, cancellation
and nested-event behavior. The same target was rebuilt in Debug with runtime
trace enabled: the trace proves CPU publication records before the Core
settlement point, then DMA, PIT, PIC and the run boundary occur at the selected
tick in that order. This is a Core scheduler result only. It does not claim
physical wall-clock pacing or invent service durations for active L1 owners.

Debug S3--S5 order coverage also passed after the poll removal:

```text
core-machine-arbitration-s3-smoke
core-machine-d4-refresh-hold-smoke
core-machine-competition-s3-smoke
core-machine-competition-80386-s1-smoke
core-machine-rtc-storage-s4-smoke
core-machine-input-display-s5-smoke
core-machine-fdc-media-change-port-smoke
core-machine-plan-smoke
```

The same Core scheduler, timeline, HLT and FDC media-observation smokes also
passed in the existing stripped Release probe.  Release records no development
trace; the state-based assertions remain identical.

## Complete Controller-Owner Confirmation

The bounded scheduler verifier now requires the actual advance calls for DMA,
PIT, PIC, FDC, HDC, RTC, KBC, XT keyboard and VADP to remain in
`machine_scheduler.c`; it also rejects the retired VM/media refresh route.
The focused controller and lifecycle set passed after a fresh rebuild:

```text
core-machine-scheduler-smoke
core-machine-timeline-s2-smoke
core-machine-hlt-s49-smoke
core-machine-arbitration-s3-smoke
core-machine-d4-refresh-hold-smoke
core-machine-rtc-storage-s4-smoke
core-machine-input-display-s5-smoke
core-machine-fdc-media-change-port-smoke
core-machine-plan-smoke
vm-pcat-composition-s4-smoke
vm-model40-d4-compatibility-s25-smoke
vm-model40-fdc-s24-smoke
vm-model40-fdd-s18-smoke
vm-display-composition-s5-smoke
vm-ibm-5170-root-resolver-smoke
```

This is S2 scheduler evidence only, not a current-gate closure statement.
S12 supersedes its earlier partial-gate observation with a fresh complete-suite
inventory and requires every production and test failure to be repaired before
T499 may continue toward closure.  No failing check is excluded because its
owner is outside the scheduler-controller receiver.

## Model-40 Matrix Receiver Check

The selected Model-40 construction installs 2 MiB. Its D4 setup readback had
retained the incompatible 4-MiB low-byte value even though the existing
configuration and addressable Core RAM selected 2 MiB. The source-backed D3PE
board decode establishes the 1-MiB extension route; the visible setup-byte
mapping is an External-L2 cross-check against the selected read-only 86Box
model. The reset value is now the matching 2-MiB configuration, and the
existing D4 map smoke proves both that value and A20-enabled access to the
second MiB.

The owner-authorized real-ROM replay remains before its first FDC request
after that correction. This proves the scheduler continuously progresses and
the configuration contradiction is removed; it does not prove a completed
DeskPro boot. The expanded read-only reference sweep distinguishes the D4
control byte's actual relocation/write-protection effect from the setup byte:
PCjs stores the latter as setup readback, while 86Box recalculates its own
internal RAM/cache form. Neither observation proves that setup write is this
ROM's remaining boot cause. The owner has admitted the complete D4
single-Core-owner migration into T499 S8--S11, where List 1/List 2 must settle
that question before any mapping behavior is changed; no profile-side boot
workaround is allowed.

The Debug time, transaction-lifecycle, timeline, HLT and debugger smokes also
pass. They cover Core-only elapsed publication, transactional reset/rollback
boundaries, timeline cancellation/reset, halted-CPU wake progression and the
runtime debugger's current 80386 high-reset-alias observation. The debugger
smoke's obsolete low-alias expectation was replaced; no debugger behavior or
Core reset route changed.
