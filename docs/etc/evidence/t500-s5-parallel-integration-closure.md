# M5 T500 S5 Parallel Integration Closure

The CTest route boundary is exercised with the existing four-job setting.
Only seven tests that create and observe a host thread retain `RUN_SERIAL`:
six external-session scenarios and the repository-only display-cadence smoke.
No global serial setting is used.

On 2026-08-28, the owner-managed integration preset passed all 15 tests with
four workers in 24.37 seconds wall time (50.94 process seconds). Its only
inputs were configured disk-image paths; no asset, hash, path or generated
observation was added to the repository.

The preceding complete repository-only unit run passed 287/287 tests in 14.76
seconds wall time. `git status --short` was empty after both runs.
