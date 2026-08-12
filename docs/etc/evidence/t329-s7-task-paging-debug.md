# T329 S7 Task Paging And Debug-Trap Evidence

## Scope

S7 closes only the Intel 80386 32-bit protected task-transition composition
with the existing page walker and the incoming TSS debug-trap field. It does
not broaden ordinary paging, debug-register, breakpoint, VM86-task, or x87
behavior.

## Transition Contract

The TSS saved dynamic-state span remains `0x1c` through `0x63`. The incoming
task preflight additionally reads the 16-bit TSS debug-trap word at `0x64`;
the outgoing state writer does not overwrite that incoming task-static field.
Before the first task-state write, the plan verifies the old writable dynamic
span, the new readable dynamic span plus debug word, descriptors, selectors,
and target stack under the source task's current CR3. Commit then writes the
outgoing state and descriptor updates, publishes the incoming CR3 with the
remaining target CPU/cache state, and sets `CR0.TS`.

If bit zero of the preflighted incoming debug word is set, the task owner
delivers vector 1 only after that commit. This is a private task-transition
post-commit path: the regular instruction-fault finalizer is not used because
it correctly restores the pre-instruction CPU for faults. The delivered
`#DB` frame therefore names the target task's loaded EIP and the handler sees
the new TR, CR3, caches, and stack.

## Owner Smoke Matrix

`core-machine-task-switch-smoke` retains every accepted S1--S6 vector and
adds these S7 checkpoints:

- A paging-enabled direct 32-bit TSS switch uses source CR3 `0x1000`, writes
  that value to the outgoing TSS image, commits incoming CR3 `0x4000`, and
  fetches the target instruction through a deliberately different page-table
  mapping. The target's `MOV EAX, 0x1234; HLT` proves that the target page,
  rather than the source identity mapping, was consumed after commit.
- A controlled target-TSS page absence under the source CR3 delivers installed
  vector 14 before publication. The handler observes the old TR and null
  LDTR, source CR3 `0x1000`, unchanged old/new busy bytes (`0x8b`/`0x89`), and
  an all-zero outgoing TSS dynamic image.
- A target TSS with the debug-trap bit set commits the target task then enters
  an installed vector-1 interrupt gate. The three-word restart frame contains
  target EIP `0x100`, CS `0x08`, and target EFLAGS; the handler increments AX
  and halts at `0x182`, proving exactly one post-switch trap with target state
  active.

The focused target is current-gate registered and uses target-local strict GCC
options. Full current-gate verification remains the S7 closure gate.

## Transfer

T329 does not claim generic page-walk/TLB policy, ordinary debug-register or
breakpoint semantics, task VM86 breadth, VME/PVI, TSS I/O bitmap behavior, or
x87 execution. Those retain their named closure-map, Queue, or external
coprocessor boundaries.
