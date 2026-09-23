# T329 S5 - Nested Task Return and IDT Task Gates

## Bounded Contract

This record closes the S5 slice of the task-transition state machine: nested
`IRET` return through the current TSS backlink, IDT task-gate entry, and a
bounded double-fault task-gate chain. It does not claim non-null LDT task
images, task paging, debug state, VME/PVI, arbitrary nesting depth, or reset
policy after a failed double-fault delivery.

## Shared Transition Boundary

`_ser_task_transition_tss` is now the only 16-bit/32-bit TSS transition
implementation. It receives explicit `nested` and `returning` directions.
The planner admits an available target for entry and a busy target for a
backlink return; its commit keeps the outgoing descriptor busy for nested
entry, clears it for a return, writes only the nested-entry backlink, and
publishes `NT` only for nested entry. `_ser_task_return_tss` reads and checks
the current TSS backlink before entering that same planner.

`_ser_task_gate_descriptor` accepts a previously-read task-gate descriptor and
routes both GDT task-gate transfer and IDT task-gate delivery to the same
transition entry. IDT task gates use their vector error code and software-INT
privilege check; they do not use the far-transfer selector-RPL rule.

## Focused Proof

`core-machine-task-switch-smoke` proves:

- 80286 16-bit and 80386 16-bit/32-bit TSS nested far-CALL entry followed by
  an actual `IRET` return: backlink consumption, source/target busy state,
  `TR`, cleared `NT`, restored outgoing image/cache state, and target-body
  progress;
- an IDT vector-3 task gate on 80286 and 80386: direct target-TSS selection,
  backlink/NT/busy publication and task-body marker;
- a bounded 80386 contributory `#GP` delivery failure followed by IDT vector-8
  task-gate double-fault entry, with the delivered `#DF` diagnostic and the
  same target TSS state proof; and
- retained S1--S4 direct-JMP/CALL/task-gate forms, installed-fault boundaries,
  prefix/LOCK rejects, and pending-IRQ evidence.

The task-return 32-bit fixture deliberately supplies an ESP within the
incoming 16-bit stack segment limit. This makes the preflight check observable
without relaxing that architectural limit.

## Transfer

S6 owns non-null LDTR/LDT task images. S7 owns paging and debug task state.
More-than-one nested return chain, failed `#DF` reset behavior, IDT gates that
are not task gates, and generic exception/IRQ policy remain outside S5.
