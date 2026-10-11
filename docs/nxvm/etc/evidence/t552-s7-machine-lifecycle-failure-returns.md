# T552 S7 — Core Machine Lifecycle And Failure Returns

## Reviewed Candidates

| Candidate | Direct evidence | Disposition |
| --- | --- | --- |
| Core run result state | `core-machine-lifecycle-smoke` proves initialized-run rejection, HLT wait, requested stop and reported fault result/lifecycle pairs. `vm_machine_runner_run()` consumes only the successful Core result path and treats a Core fault or an internal run failure as a captured runner failure. | Retained: there is one Core result owner and one bounded runner consumer; no second lifecycle path or status compression was found. |
| FDD/HDD replacement | Each install atomically replaces the current medium before it closes the old lease. The old close result was ignored, incorrectly reporting success after a close failure. | Repaired: retain the installed replacement and return the old lease close result, matching the established eject contract. |
| FDD/HDD removal | `vm_machine_*_remove()` clears media state after Storage consumes the lease and propagates the close result. | Retained: the direct close-failure injector proves an I/O error leaves both devices unloaded. |
| Teardown | Finalization destroys owned media after Core routes are revoked; the `void` destructor contract has no return channel. | Retained: no caller can truthfully consume a close result during object destruction, so adding a recovery channel would be fictitious. |

## Contract

Replacement never reopens or replays an accepted candidate. If the old medium
close reports failure after the atomic replacement, the new medium remains
installed and the caller receives that exact failure status. This avoids a
stale lease while keeping the already-established new media state truthful.

## Verification

`vm-media-close-failure-smoke` injects a post-consumption close failure for
both replacement and removal. It proves that each replacement reports the
failure while the new byte is readable, and each removal reports the failure
while the device is unloaded. `core-machine-lifecycle-smoke`, Core manifest
and corpus gates pass on x64 and x86. No Shared, App, firmware, media asset,
INI or artifact input changed.
