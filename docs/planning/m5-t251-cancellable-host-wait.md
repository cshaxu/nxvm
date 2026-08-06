# M5 T251: Cancellable Host Wait Boundary

**Status:** S1 active.

## Goal

Add a policy-free, bounded cancellable wait primitive to `core/platform` and
use it where VM composition waits for host lifecycle transitions. It is a
host-thread facility only: it must not observe or advance guest elapsed ticks,
change CPU/PIT scheduling, choose display/exit policy, or expose a raw host
monotonic-clock API.

## Subtasks

### S1: Contract And Wait-Site Inventory

Define a cancellation predicate, completed/cancelled result, maximum bounded
wait semantics, callback reentrancy rule, and platform implementation rule.
Inventory runner pause/HLT waits, session pause wait, execution-flip startup
wait, Win32/Linux display readiness, and debugger polling. Keep injected
`core/utils` wait scopes where they are generic product-tool test seams; do
not mistake that utility for guest timing.

### S2: Implement And Adopt Lifecycle Wait

Implement the neutral core/platform primitive over platform sleep in bounded
poll increments. Adapt only lifecycle waits whose cancellation condition is
already a VM composition fact. Preserve existing execution-flip timeout,
run-handle join, Console/window, and debugger behavior; do not introduce a
new thread, watchdog policy, or timer source.

### S3: Verify The Boundary

Add completed/cancelled/bounded probes and retained run-handle startup/stop
failure coverage. Run current GCC/CTest, Console/window, DOS, graphics,
FDD/HDD, debugger, and artifact verification. Allocate the next developer
artifact if runnable source changes.

## Stop Conditions

Stop for owner direction if any caller needs guest time, host-clock observation,
an unbounded wait, display/exit policy, host-thread guest mutation, or a
second lifecycle path.

**S1 marker:** `M5:T251:S1:CANCELLABLE-WAIT-CONTRACT:OK`.
