# M5 T526 S13 route audit

## Scope and result

S13 audited the retained `vm/events -> vm/machine -> vm/session ->
vm/presentation` route without changing `src/lib` or adding a second execution
queue.  The one discovered final-route defect was the executor's paused wait:
an ingress request could be queued after the runner had consumed the old
control wake and before it waited again.  The executor FIFO now owns the
readiness event together with its ingress data.  Enqueue signals it and the
last dequeue resets it, so runner wakeup cannot be lost independently of the
queue it must drain.

`vm_unified_debug_backend_smoke` also used a private direct thread, bypassing
the public `vm_machine` lifecycle.  It now uses `vm_machine_start()` and
`vm_machine_stop()`; step observes the asynchronous `STEP` pause result rather
than treating the preceding explicit pause as the next completion.

## Owner check

- `vm/machine/executor_fifo` remains the only Core-access request FIFO; the
  readiness event is its state, not another command channel.
- `vm/machine/runtime/runner` waits on the FIFO's readiness plus direct control
  changes; it still consumes requests only at the existing command boundary.
- The integration test no longer creates a raw executor thread or reaches a
  private control lifecycle path.
- No VM source imports a native UI API, and no `src/lib` source changed.

## Simplicity review

The tracked production/test/build delivery is 77 added and 42 removed lines
(net +35; documentation and generated artifacts excluded).  The positive delta
is the one FIFO-owned readiness event and its regression proof; it deletes the
separate lifecycle wake and the test-only raw thread.  The retained production
path is one ingress FIFO, one readiness event, and one runner wait set; there is
no forwarding queue, mirrored request state, or compatibility resume route.

## Verification

- Focused executor FIFO unit and unified-debug integration: pass.
- Full repository unit suite: 299/299 pass.
- Current specialized gates and documentation governance: pass.
- External integration: 42/42 pass (`ctest -L integration -j 8`, 520.15
  process-seconds and 443.38 wall-seconds).  The repaired unified-debug row
  passes.  The external-ROM Model-40 row reaches the installer in 160.99
  seconds; it no longer fails at the 190-second registered limit.
- Diagnostic probes explain the former 60-second observation without inventing
  a controller repair: the Model-40 BIOS is executing its protected-mode RAM
  pattern test (`STOSD`/`LODSD`) after its Compaq EGA option-ROM calibration.
  PIT channel 0 remains a live mode-3 counter (`control=36h`) with changing
  latch/count values.  This is neither a D4 mapping nor a KBC/FDC stall.  The
  resulting throughput is still unsuitable for a 60-second usability target,
  but that is a separate Core-performance/timing task, not a stale or duplicate
  Core/VM/lib ownership route in S13.
- The current stripped Release product targets build for x64 and x86.  CMake
  deploys each to both `build/output` and `assets/sessions`; the pair matches
  byte-for-byte at each location.  SHA-256 is
  `6639BC3D07196647B443D602DCAA2F229F46135D15C0DF2DF56B49AC4BFB2CCC`
  for x64 and
  `EBDACBDD9C9E1FFAB55CEE64AEB7E76D563A030579B37D803BC38531EC5015BB`
  for x86.
