# T390 S3: 80386 Retirement Observation Provider

`M5:T390:S3:RETIREMENT-OBSERVATION:OK`

## Implemented Core Contract

S3 adds one optional Core-owned retirement-observation provider.  It may be
installed or removed only while a machine is stopped or paused; a running
callback attempting mutation receives `TYPE_STATUS_INVALID_STATE`.  With no
provider installed, the production path performs no capture or publication.
The public record is copied Core data only: the existing pre-execution CPU
point (PC and fetched byte window), CPU profile, selected execution context,
classifier disposition, source ticks, and pre-publication elapsed/timeline
values.  It does not borrow an executor pointer and does not change the
existing generic trace ABI.

The existing CPU diagnostic record callback remains the sole pre-execution
junction.  It snapshots the observation only when enabled.  After
`core_machine_instruction_cost()` resolves a successful form, one publisher
assigns the exact-or-source-unallocated disposition and invokes the observer
before either the physical-contract rejection or
`core_machine_publish_elapsed_ticks()`.  Reset clears an unconsumed pending
record but keeps the installed provider; lifecycle destruction releases all
Core state conventionally.

`CLASSIFIED` is deliberately not an Intel timing-row identifier.  S3 exposes
only the existing classifier's exact-or-unallocated disposition, so the later
external corpus/matrix receiver must still bind observed form/context to the
Intel-primary row before any form becomes physical-eligible.

## Focused Proof And Sweep

`current.core-machine-retirement-observation-s3-smoke` passes after a clean
rebuild.  It proves all of the following with a production provider, not a
test-only instruction entry point:

- an installed observer survives reset and receives a copied reset-vector NOP
  record before elapsed/timeline publication;
- the running lifecycle rejects provider removal, while paused removal works;
- after removal, another retirement produces no callback; and
- physical 80386 `REP NOP`, currently source-unallocated, is observed before
  it faults, retires zero instructions, and leaves elapsed/timeline at zero.

A source/caller sweep finds one capture call at the existing CPU diagnostic
pre-dispatch callback and one publish call between successful source-timing
classification and the physical rejection/publisher.  The generic
`CPU_RETIRE` trace ABI has no new field or producer.

## Current-Gate Disposition

The full gate cannot currently be accepted for S3.  Six current smoke targets
fail identically in both this source tree and a detached worktree at accepted
T390 S2 commit `a27caff0`; therefore they predate S3:

- `current.vm-model40-private-composition-s7-smoke`;
- `current.vm-model40-integration-s8-smoke`;
- `current.vm-media-provider-smoke`;
- `current.vm-default-pc-at-rom-materialization-smoke`; and
- `current.core-machine-d4-platform-s4-smoke`; and
- `current.core-machine-task-switch-smoke`.

The complete configured gate finishes 276/282 passing; the six failures above are identical in the accepted baseline.  The new T390 S3 smoke passes in the changed tree.  Direct debugger inspection
of the Core-only D4 control establishes its first failed group reads port
`61h` as `0x1b` where its legacy assertion expects `0x0b`; the same failure
repeats across ten direct runs.  This is a reproducible pre-existing
current-gate regression, not an asset-only or flaky result, but it is outside
S3's CPU observation scope.  It must receive a separate, source-complete
current-gate corrective task before S3 can meet its full-gate exit criterion.
No T390 physical qualification, board clock, firmware/ROM/media execution, or
L3 claim is made by this record.

## Transfer

The next T390 receiver needs a contained external BYOB run and a finite
observed form/context matrix.  It must map each physically proposed form to an
Intel-primary timing-row identifier, retain unallocated forms as nonphysical,
and run only after the current-gate corrective receiver has restored a clean
gate.  The six pre-existing failures need their own owner/source/root-cause
inventory and focused repairs; they must not be folded into this CPU observer
slice merely to obtain a green gate.