# base

`base` owns blocking mutexes, events, tasks, wait/cancellation, sleep/yield,
monotonic clock composition and current-executable directory discovery. It depends only on header-only `types`; no Console,
KVM, storage or application dependency is permitted. The build selects same-shape
Win32/Linux implementations. Their headers remain component-local.

Use `sync_interface.h`, `clock_interface.h` and `process_interface.h`.
`base_process_executable_directory` copies the actual host executable directory
into a caller buffer; callers append their own product configuration name.
Mutexes require a live object,
same-thread unlock and no recursion; destroy requires no owner or waiters.
Events select manual or automatic reset at creation. Signal/reset report native
failure; auto-reset coalesces signals and wakes one waiter. Manual-reset stays
signaled until reset. Task cancellation retains manual reset and wins wait-any ties.
Task fields are embedded in one platform allocation, with one native entry
trampoline and the existing separate cancellation Event. Task destroy cancels
and joins before disposal. Cancellation, join and destroy return status;
failed destroy retains the task and requires its context to remain alive.
The owner serializes these operations and never joins from the task itself.
These are the existing primitives,
not forwarding aliases for another component. Caller-owned locks retain their
existing lock order and callback non-reentry requirements.
