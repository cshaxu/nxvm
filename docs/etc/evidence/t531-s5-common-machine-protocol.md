# T531 S5 Common machine protocol proof

## Result

Common Machine now has one copied, run-bound safe-point request protocol for
host input, pause/reset/resume/step/stop, and removable floppy media.  A driver
returns the actual status of a consumed request; Common no longer silently
turns a failed adapter operation into a successful safe-point observation.

`START` is intentionally a Common Session lifecycle request rather than a
Common Machine FIFO item.  Before NXVM has a Core worker or SoftPC has resumed
its CCPU worker, there is no executor safe point at which to consume it.  The
injected adapter owns creating/waking its one distinct executor.  Once that
executor exists, all lifecycle and input work remains on the Common Machine
FIFO.

Floppy insert/eject now follows one route in NXVM:

`product command -> Common Machine copied request -> stopped composition or
paused executor safe point -> VM FDD owner -> copied machine result -> Common
Session monitor fact`.

The stopped composition boundary consumes the same request immediately because
there is no worker and therefore no concurrent guest mutation.  A paused
machine leaves the request for its existing executor; Session and UI never
touch FDD state.  HDD is deliberately absent: it is fixed board topology, not
removable media.

## Two-adapter disposition

| Family | NXVM disposition | Read-only SoftPC crosswalk |
| --- | --- | --- |
| Start | Session lifecycle sink resets and creates the one Core task. | `app_runtime_start()` wakes/starts its retained CCPU worker; it must bind the same Session sink in its own approved integration work. |
| Lifecycle/input | Existing Common FIFO is retained; NXVM driver maps it only at Core runner boundaries. | `app_runtime` already services command/input events at CCPU rendezvous boundaries. |
| Floppy media | New Common removable-media request; FDD mutation occurs only at the stopped or paused adapter safe point and reports a copied result. | `app_runtime_set_floppy()` already requests CCPU-side service and waits for `media_event`; its future Common binding is a receiver, not an NXVM-side duplicate. |

No SoftPC source is changed here.  Therefore this evidence accepts NXVM's
protocol cutover and the common contract, but does not claim the later
two-real-consumer S8/T531 acceptance proof.

## Verification

- Strict C11 source compilation with `-Wall -Wextra -Wpedantic -Werror` covers
  the changed Common machine/session, NXVM adapter/app/product paths and their
  owner tests.
- Directly linked Common machine and session smokes pass.  They cover driver
  result propagation, stale-run rejection, a copied floppy request and monitor
  completion text that cannot be parsed as CLI input.
- The directly rebuilt VM media lifecycle smoke passes.  It creates a temporary
  1.44 MB image, proves stopped-state insert through the Common protocol,
  proves running replacement/eject rejection without path or generation loss,
  then proves stopped eject and cleanup.
- `ctest --test-dir build/mingw-gcc-x64 -L unit -j 8 --output-on-failure`:
  **299/299 passed** in 16.27 seconds.
- Common manifest and Lib-only corpus verification, plus `git diff --check`,
  pass.

The aggregate CMake/Ninja rebuild remains separately blocked by a local hang
before compiler children appear.  The changed sources were instead compiled
and focused tests linked directly; the full unit run is recorded as a complete
regression replay, not as an aggregate rebuild claim.

## Change accounting and retained path

The counted production source/header paths add 192 and remove 37 lines (net
+155); owner tests add 63 and remove 13 lines (net +50).  The added code is
the one copied media request value, explicit driver outcome and one copied
completion fact; it replaces the direct public FDD mutation route.  No second
executor, queue, Core pointer, controller pointer or media cache was added.
