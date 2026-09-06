# T524 S4 Host Contract

## Contract

`lib/host` remains a peer root that depends only on `base`. It supplies host
clock, manual-reset events, cancellable tasks and multi-source waiting; it
contains no product callback, executor policy, timer tick or machine state.

Events remain signaled until reset. `host_sync_wait_any` accepts one or more
events and an optional task cancellation source. A visible cancellation wins;
otherwise the lowest visible event index wins. A timeout returns no index.
Task destruction requests cancellation and joins; it never terminates a native
thread. Lifecycle calls belong to one owner and destruction cannot run inside
the task entry.

## Platform Parity

Windows puts cancellation first in `WaitForMultipleObjects`, followed by the
provided event order. Linux uses one host-local monotonic condition hub and a
generation counter: state is checked after recording a generation, and a
generation change before blocking restarts the check. This prevents an event
signal between the inspection and wait from being lost. Linux timed waits use
`CLOCK_MONOTONIC`, not wall time.

## Verification

- `host-sync-smoke` proves manual reset, timeout, two-event index selection,
  cancellation priority and destruction-driven cancellation.
- `library-consumer-conformance` is the second public-header neutral host
  consumer.
- UCRT64 `gcc -std=c11 -Wall -Wextra -Werror -fsyntax-only -Isrc` accepts the
  Linux host source, clock source and host smoke.
- Root unit: 312/312 pass in 20.20 seconds.
- Manifest and diff checks pass. No `host` include targets another lib root.

## Remaining Whole-Corpus Work

`host/clock.h` still has one prohibited word in a negative comment. It is a
whole-corpus vocabulary cleanup owned by S8, not a hidden host dependency.
