# base

`base` owns blocking mutexes, events, tasks, wait/cancellation, sleep/yield and
monotonic clock composition. It depends only on header-only `types`; no Console,
KVM, storage or application dependency is permitted. The build selects same-shape
Win32/Linux implementations. Their headers remain component-local.

Use `sync_interface.h` and `clock_interface.h`. Mutexes require a live object,
same-thread unlock and no recursion; destroy requires no owner or waiters.
Events select manual or automatic reset at creation. Signal/reset report native
failure; auto-reset coalesces signals and wakes one waiter. Manual-reset stays
signaled until reset. Task cancellation retains manual reset and wins wait-any ties.
Task fields are embedded in one platform allocation, with one native entry
trampoline and the existing separate cancellation Event. Task destroy cancels
and joins before disposal. These are the existing primitives,
not forwarding aliases for another component. Caller-owned locks retain their
existing lock order and callback non-reentry requirements.
