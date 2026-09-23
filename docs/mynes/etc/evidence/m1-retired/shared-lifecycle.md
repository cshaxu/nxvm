# Shared Executor Lifecycle And Failure Boundaries

## Goal And Dependency

Consume [neutral contracts](shared-neutral-contracts.md) under the
[M1 context](first-emulator-increment-planning.md). Establish finite control,
request and publication behavior using fake machines on the production executor.

## Scope And Proof Batches

Preserve existing neutral mechanisms and unchanged Lib. The following are proof
obligations, not a mandate to redesign already-correct code. Repair a demonstrated
Common contract gap within admission; return Lib defects to the upstream owner
and block affected acceptance pending a reviewed baseline update. Product-owned
debug payloads remain opaque throughout queuing/cancellation. No x86 component
is required by Common and no frame-format negotiation is introduced.

Freeze a transition table for constructed/empty, stopped, running, paused,
faulted and shutdown states, mapping existing enums instead of adding shadow state.
Cover start, pause, resume, stop, reset, replacement and shutdown from every state;
each cell is accepted with expected completion or rejected with no mutation.
Start may reset; resume preserves state; stop is restartable and distinct from
pause/unload/permanent join. Optional replacement capability is exercised with
fake candidates, without interpreting ROMs or adding cartridge policy.

Specify finite control/input/debug queues, overflow rejection, generation handling,
shutdown admission and non-starvable cancellation. Choose capacities and cooperative
safe-boundary budgets in the admitted contract from the workload ledger; test exact
capacity and capacity-plus-one, competing producers and cancellation under saturation.
Do not turn a hung driver into a successful join or free its still-borrowed context.

Prove candidate failure preserves accepted state; reset failure reports faulted;
callbacks and completions are serialized by their owner; old-generation completions
cannot advance session state. Frame publication copies content/sequence/generation
atomically, coalescing only frames. Stop/wake signal hooks do not mutate hardware.
Retire executor and UI producers before releasing sinks and callback contexts.

## Acceptance And Stop Conditions

Every transition cell, queue boundary, stale-completion, construction failure,
replacement failure, reset fault, repeated shutdown and failed-join retention has
direct deterministic evidence or a reasoned capability non-applicability. No timeout
substitutes for a semantic checkpoint. Run the full common completion contract.
Do not implement NES parsing, real pacing, audio or persistence. Stop on a required
change to approved ownership. Hand off lifecycle/generation/reset primitives to
[input and presentation](shared-input-presentation.md).
