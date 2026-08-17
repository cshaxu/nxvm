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

## Corrective P2: Current-Gate Reconciliation

P1 could not be accepted because six registered current smokes failed identically
at accepted T390 S2 `a27caff0`.  P2 traced each failure to its first assertion
and retained or corrected the owning contract; no target was skipped,
de-registered, or weakened.

- The immutable-ROM materialization smoke incorrectly expected a routed ROM
  write and its query to fault.  The established Core ROM contract instead
  accepts the transaction while retaining immutable backing; the repaired
  assertion proves both successful routing and unchanged bytes.
- The Model-40 composition/integration and D4 platform smokes omitted the
  shared PIT counter-1 output bit from port `61h`.  The reset state is `1Bh`,
  not `0Bh`; the tests now preserve the existing NMI/board bits while asserting
  the timer-derived bit.
- The media-provider smoke initialized a one-cylinder HDD, reset it, and then
  incorrectly required two cylinders.  The provider correctly preserves the
  configured geometry; its assertion now matches the fixture's `1 x 32 x 63`
  input.
- P1 adds one registered Core retirement-observation smoke to the T345 owner matrix. The specialized count guard is therefore reconciled from 168 to 169; it retains the test in the owned direct-compilation inventory rather than excluding it.
- The task-switch debug-trap smoke exposed a real Core diagnostic defect.  A
  task-switch `#DB` is delivered outside `ExecFinal()`: its diagnostic copied
  `instruction_state.oldcpu`, which still describes the outgoing task, so its
  point was recorded as EIP `30h` instead of the incoming task EIP `100h`.
  The task-transition owner now passes a local diagnostic instruction snapshot
  whose saved CPU is the incoming trap point.  It preserves the executing
  instruction's original `oldcpu`, actual exception delivery, and all other
  diagnostic paths.  The existing T329 task paging/debug regression now proves
  the corrected point together with its frame and register effects.

Focused repaired smokes and the S3 retirement-observation smoke pass.  After
reconfiguration, the complete configured current gate passes **282/282**
(`144.99 sec*proc`); documentation governance and specialized gates also pass.
The rebuilt developer artifact is `vm-0-5-0390`,
`build/output/nxvm_0_5_0390.exe`, SHA-256
`361360730ADC2E75F01A2FD45D7F07E272D394CF8DBA8E49FB26D02318818BFE`.

This records the complete implementation P2 evidence.  Coordinator
actual-change review and the subsequent governance acceptance remain required;
this evidence does not claim physical qualification, board timing, firmware or
media execution, or L3 closure.
## Transfer

The next T390 receiver needs a contained external BYOB run and a finite
observed form/context matrix.  It must map each physically proposed form to an
Intel-primary timing-row identifier, retain unallocated forms as nonphysical,
and run only after the current-gate corrective receiver has restored a clean
gate.  The six pre-existing failures need their own owner/source/root-cause
inventory and focused repairs; they must not be folded into this CPU observer
slice merely to obtain a green gate.