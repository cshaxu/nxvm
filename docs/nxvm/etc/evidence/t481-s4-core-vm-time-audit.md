# T481 S4 Core/VM Time And Pacing Audit

`M5:T481:S4:TIME-OWNER:OK`

| Boundary | Current owner/path | Evidence and disposition |
| --- | --- | --- |
| Guest-time mutation | `core_machine_publish_elapsed_ticks()` in `machine_scheduler.c` is the sole production writer. CPU retirement and Core-selected deadline advance both reach it; VM/profile code has no caller. | `core-machine-time-smoke`, source sweep, and T474 S1/S6. Core is the only guest clock owner. |
| Time plan and observation | Construction copies immutable `core_machine_time_axis` and controller timing into Core. `core_machine_capture_time_observation()` exports only a copied value. | T474 S2/S3 and T475 S3. No device pointer, live profile setter or VM-owned mirrored counter exists. |
| Deadline composition | Core privately composes only qualified PIT/RTC deadlines; active DMA/KBC/FDC/HDC work blocks fast advance. | T474 S4 and `vm-model-339-clock-contract-smoke`. The VM can request the existing Core operation but cannot select a device/delta. |
| Standard | `waiting.c` reads completed Core observation and, only when a macro rate is available, waits while Core is ahead of the monotonic-host budget. | It never converts host elapsed time into a guest tick. The macro rate is L2 unless a profile proves a physical axis. |
| Turbo | Same run/HLT/deadline path, omitting only Standard's host wait. | Turbo neither changes Core timeline semantics nor grants direct time mutation. |
| Unqualified profile | Observation leaves pacing unavailable; both modes avoid a rate, deadline estimate and fixed HLT fallback. | Honest L1/no-wait result, not an invented L2 clock. |

## Focused Verification

`ctest --test-dir build/mingw-gcc-x64 -R
"core-machine-time-smoke|vm-session-speed-policy-smoke|vm-session-initialization-atomicity-smoke"
--output-on-failure` passed 3/3. The source sweep found all production
`core_machine_publish_elapsed_ticks()` calls in Core and no VM/VDM caller of
`core_machine_advance_time()` or the publisher.

## Recorded Transfer

`src/vm/product/console.c` still describes Standard as retaining an L2 fixed
HLT host-load backoff and Turbo as merely reserved. That is stale user-facing
text: the retained policy is observation-based Standard pacing and no-wait
Turbo, with no fixed HLT backoff. It is a bounded console-help correction for
the next documentation/code-repair receiver; it does not justify another time
owner or a pacing rewrite in this audit.

`M5:T481:S4:PACING-BOUNDARY:OK`

`M5:T481:S4:CORE-VM-AUDIT:OK`
