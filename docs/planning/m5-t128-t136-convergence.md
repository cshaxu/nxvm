# M5 T128--T136: Invocation And Unified-core Convergence

## Objective

Complete the two open M5 closure items without changing the retained NXVM
Console, debugger grammar, boot sequence, or guest-media behavior:

1. no production command, debugger, or wait operation selects context through
   thread-local state; and
2. VM and VDM create the same executable `core_machine` state. Test fixtures
   do not add a second state representation to `core_machine`.

## T128: Design The Explicit Contracts

`core_product_debug_main` and `vm_product_console_main` keep their public
entry signatures but create stack-owned invocation records. Every internal
command helper receives an explicit pointer to that record. The record carries
the caller-owned context, target, and input/wait operations. It is not stored
in a global, thread-local, session, or target object.

Wait becomes `core_product_wait_milliseconds(const core_product_wait_scope *,
uint32_t)`. A null scope uses the neutral core-platform sleep implementation.
No enter/leave or ambient current provider exists. VM platform backends pass
their session-owned scope explicitly; firmware/debug helpers receive it from
their invocation or profile context.

`core_machine_create` creates the sole executor, RAM, and port state for every
runnable core-machine profile. The core owns initialize/reset/finalize order.
VM and VDM composition borrow that state and install only their respective
providers, profile firmware, and product policy. Contract-only RAM/port/trace
tests move to `tests/support`; `CORE_MACHINE_PROFILE_TEST_MINIMAL` and all
`test_*` fields leave production source.

## Execution Order

| Task | Change | Required evidence |
| --- | --- | --- |
| T129 | Convert debugger and xasm invocation plumbing to explicit state; delete debugger TLS. | debugger target, pause, nested-invocation, retained Console, DOS-prompt gates |
| T130 | Convert NXVM Console parsing/commands to explicit invocation state; delete Console TLS. | retained Console lifecycle and command grammar gates |
| T131 | Replace ambient wait scope with explicit operation propagation. | nested wait, Win32/Linux compile, pause/window/Console gates |
| T132 | Re-audit all production source for TLS/current-object dependencies. | zero production `_Thread_local` selection scan and full focused suite |
| T133 | Define and implement core-owned executor bootstrap and reset/finalize lifecycle. | custom-core lifecycle and VM boot/DOS-prompt gates |
| T134 | Move contract-only test state into test support; migrate tests. | core contract/trace/port/memory gates without test profile |
| T135 | Recompose VDM minimal over the same executor core; it remains non-DOS-runner behavior. | VDM session/presentation plus VM two-session gates |
| T136 | Delete test-minimal profile/state and independently audit one core state model. | no `TEST_MINIMAL`/`test_*` production scan, full GCC and boot suite |

## Invariants

- `core_machine` has no VM, VDM, platform, profile, Console, or DOS policy
  dependency.
- A test fixture may create test data, but it never becomes a
  `core_machine` profile or a product route.
- `vm_session` and `vdm_session` each own one `core_machine *`; neither owns a
  CPU/RAM/port mirror.
- Providers are installed before reset/execution and destroyed before their
  borrowed core state is released.
- A migration task stops on an NXVM Console/debugger/user-visible regression,
  boot failure, second machine allocation, or need for a compatibility global.
- T129--T136 each produce a task-numbered NXVM artifact when they alter the
  runnable VM path. T128 is design-only.
