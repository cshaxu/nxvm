# M6 T43 S9 Snapshot Resume

## Mechanism And Repair

Baseline: accepted S8, closure commit 1e86b8e7a. The owner reports Running after
snapshot resume with frozen gameplay. Target is MyNES only; Shared/NXVM and
configuration files are unchanged.

Common stop sets both its own lifecycle stop and the driver's stop signal.
Cold start clears the latter through core_driver_reset. Snapshot restore
intentionally bypasses reset to preserve the captured CPU/device state; Common
clears its own stop but MyNES core_driver_write_state did not clear the driver
latch. Consequently core_driver_run serviced callbacks and returned success
without executing any guest instructions. The state could truthfully describe
the executor as Running while its driver did no guest work.

After successful transactional snapshot decode/commit, MyNES now clears that
previous-run stop latch and invalidates its published-frame revision. The latter
allows the restored image to be published in the new run generation even when
its revision equals the last image from the previous run. Failed loads change
neither latch/cache nor guest state. Audio clearing and pacing rebase remain at
their existing restore boundary. No format, API, thread, queue or run-state
owner is added; Common still exclusively owns lifecycle and execution.

## Regression And Similar-Issue Sweep

The existing product-owned snapshot transaction test is expanded rather than
adding a parallel harness. Its project-owned ROM samples controller A into RAM
and changes its backdrop each vblank. Both graphics and text-output drivers run
through the public Common lifecycle, snapshot, input, frame and paused-debug
interfaces. No game ROM or physical audio signal is needed.

Covered cases: save/resume without stop; stop/load of the immediately saved
image; repeated rewind/load/resume with alternating key make/break; restored
cycle equality before resume; actual increased cycles and consecutive frames
after resume; guest RAM reflecting input; invalid load remains stopped; reset
then resume still runs; orderly stop/shutdown. Existing Core snapshot tests
retain malformed/version/cartridge rejection and state-atomicity checks.

Before the driver repair, the expanded test failed its assertion that resumed
guest cycles increase (14.51 seconds). The first graphics fixture used a static
backdrop; unchanged text frames are correctly coalesced by Common, so the ROM
was changed to vary its palette. An initial full-suite run then exposed the
test's assumption that vblank publication implies guest input sampling already
completed. The fixture now samples input continuously; 10 consecutive x64 runs
pass in 20.18 seconds. No production behavior was changed to satisfy these
fixture corrections.

Sweep query: rg -n 'stop_requested|debug_stop_requested|pacing_origin_cycles|
published_frame_revision' src/app-mynes/core, together with Common reset,
resume, stop, state-write and worker call paths. Cold reset already clears the
stop latch; normal pause/resume never sets it. Media replacement while stopped
still requires cold start/reset; paused replacement retains an active run.
Debug-stop is consumed/cancelled by Common; wake is consumed on the next pace;
input reset is consumed on the executor. Restore already rebases pacing and
clears queued audio. No additional duplicate stop owner is introduced.

## Delivery

Source/test count using git diff --numstat over src/app-mynes and test/app-mynes:
+166/-10, net +156. Production is four added lines (two explanatory comment
lines and two state updates). Test growth replaces minified lifecycle-only
checks with readable end-to-end progress assertions and one two-mode matrix.

Both optimized stripped MyNES 0043 artifacts are rebuilt; PE headers verify
8664/x64 and 014C/x86, and objdump finds no compiler debug sections. Both retain
the owner's MyNes (Running)/(Paused) titles.

- x64: 246798 bytes; SHA-256
  8D3B77FA81B8B3BF679B709EBD932DA287DA6BA73DC76D1790AA699C21A80152.
- x86: 240142 bytes; SHA-256
  6B59B1D73A6CA0D9EEBD92EF50A8B0E76E057692C490E4A6ADA9C1BB5937856B.

Final complete configured MyNES suites pass x64 130/130 (116.09 seconds) and
x86 130/130 (127.71 seconds), including repository-only units, snapshot/App
integration, native presentation and six-corpus governance checks. Commands:
cmake --build build/mynes-gcc-<arch>-release --parallel 8, then ctest --test-dir
that tree --output-on-failure -j 4. Both product targets were relinked/deployed;
later changes affected only the test fixture, not those executable inputs.
Documentation governance and git diff --check pass. Actual-diff review confirms
MyNES-only paths, successful-commit cleanup and no new public API or parallel
execution path. Build trees remain for open T43 and owner verification.
No manual game acceptance or T closure is claimed.
