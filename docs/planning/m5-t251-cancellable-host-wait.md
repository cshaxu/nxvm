# M5 T251: Cancellable Host Wait Boundary

**Status:** S3 active.

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

#### S1 Contract

`core/platform/wait_interface.h` will define a cancellation predicate and a
three-way result: completed bounded interval, cancelled, or invalid argument.
The caller supplies a finite millisecond budget and an explicit predicate;
the primitive checks the predicate before and after bounded sleep increments.
It neither reads host wall-clock values nor exposes them, and it never calls
into `core/machine` or changes guest state. Its only host operation is the
existing platform sleep implementation.

The predicate is invoked synchronously on the waiting caller's thread and may
only observe its caller-owned state. It may not invoke a mutable operation on
the waiting run handle/session, block, or recursively wait.

#### S1 Wait-Site Inventory

| Site | Existing owner and condition | T251 disposition |
| --- | --- | --- |
| `vm/composition/session/control.c` pause wait | Composition waits for `paused` or `flagRun` clearing. | Adopt cancellable wait; predicate only observes control atomics. |
| `vm/composition/session/runner.c` paused/HLT waits | Composition runner waits while guest execution is paused or halted. | Adopt cancellable wait so stop/resume/pause state ends the host sleep promptly; guest ticks remain unchanged. |
| `vm/platform/execution.c` execution-flip wait | VM platform waits for a changed flip or stopped execution during startup. | Adopt cancellable wait; keep existing 5-second budget and result semantics. |
| Win32/Linux run-handle display-ready waits | VM platform waits for ready/failed state while creating display resources. | Adopt cancellable wait with the existing finite 5-second budget; retain stop/join/finalize cleanup. |
| Win32/Linux display paint loops | VM platform waits between UI paints while execution is running. | Adopt cancellation predicate only where it observes existing execution state; retain paint cadence as VM policy. |
| `core/product/debug` polling | Core product debug uses an injected `core_utils_wait_scope`. | Keep: this is a generic testable product-tool utility, not a platform lifecycle wait or guest timer. |

No inventoried site needs a raw host-clock API, guest-time mutation, or a
second lifecycle loop. The current finite counters become explicit bounded
wait contracts; their existing cleanup owners remain unchanged.

**S1 marker:** `M5:T251:S1:CANCELLABLE-WAIT-CONTRACT:OK`.

### S2: Implement And Adopt Lifecycle Wait

Implement the neutral core/platform primitive over platform sleep in bounded
poll increments. Adapt only lifecycle waits whose cancellation condition is
already a VM composition fact. Preserve existing execution-flip timeout,
run-handle join, Console/window, and debugger behavior; do not introduce a
new thread, watchdog policy, or timer source.

**S2 result:** `core/platform` now owns bounded copied-predicate waiting.
Win32/Linux run-handle startup, readiness, and display cadence use it. The
execution-flip predicate observes only the flip: treating pre-start
`is_running` as cancellation was a startup race. Guest runner pacing remains
its existing execution concern.

**S2 marker:** `M5:T251:S2:CANCELLABLE-WAIT-MIGRATED:OK`.

### S3: Verify The Boundary

Add completed/cancelled/bounded probes and retained run-handle startup/stop
failure coverage. Run current GCC/CTest, Console/window, DOS, graphics,
FDD/HDD, debugger, and artifact verification. Allocate the next developer
artifact if runnable source changes.

**S3 result:** `current-gates-gcc` passed all 86 current CTest cases with the
cancellable-wait boundary gate. `current-gcc` produced
`build/output/nxvm_0_5_0250.exe`; SHA-256 is
`99985EB76F335178304D444FE0F0A085573D6B650E5445627725E0B9F10D8DA9`.

**S3 marker:** `M5:T251:S3:CANCELLABLE-WAIT-VERIFIED:OK`.

## Stop Conditions

Stop for owner direction if any caller needs guest time, host-clock observation,
an unbounded wait, display/exit policy, host-thread guest mutation, or a
second lifecycle path.
