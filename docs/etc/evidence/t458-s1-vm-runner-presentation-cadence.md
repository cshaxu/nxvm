# T458 S1: VM Runner Presentation Cadence

## Defect And Owner Sweep

The owner reported that default PC/AT, as well as Model 339, is too slow and
that the 256-instruction runner quantum is too dense. The runner needs that
quantum for host-control latency, but it also called
`vm_session_publish_display()` after every quantum. A display snapshot reserves
up to 224,000 pixels; graphics capture expands that frame even when the
display is unchanged.

The production sweep is `rg -n -C 2
"vm_session_publish_display|capture_display_snapshot_from|core_machine_capture_display_snapshot"
src tests CMakeLists.txt`. `src/vm/composition/session/runner.c` is the sole
normal runner caller. The remaining VM callers are forced mode/lifecycle
publication and therefore remain immediate. All other hits are focused tests
that directly observe Core's copied snapshot; none is a product presentation
loop or profile-specific runner. Default PC/AT, Model 339 and Model-40 all use
the same session runner.

## Repair And Regression

VM session state now owns one last-publication host timestamp and last observed
display kind. The runner still visits the display boundary after each control
quantum, but normal snapshot capture is due at most once per 16 host
milliseconds. A failed clock read conservatively preserves the old eager path.
Forced mode and lifecycle calls bypass the cadence. Core remains the sole
display-state and snapshot owner; the mailbox remains the sole copied-frame
consumer boundary.

`vm-runner-display-cadence-smoke` replaces its static invalid-boot observation
with a guest loop that continuously writes text VRAM. After 100 milliseconds,
it requires a nonzero bounded number of publications and emits
`M5:T458:S1:RUNNER-PRESENTATION-CADENCE:OK`. It adds no test-only production
interface.

The current-artifact CMake target now publishes only from a Release tree. A
Debug direct-compilation matrix may link the product entry for coverage, but it
cannot copy a Debug executable into either output directory.

## Verification

- Release focused cadence, DOS prompt, pause-boundary and debugger-recording
  smokes pass.
- Debug focused cadence, DOS prompt, pause-boundary and debugger-recording
  smokes pass.
- The final Debug aggregate has 293 `Test Passed` records and zero failed
  records in `build/mingw-gcc-x64/Testing/Temporary/LastTest.log`; it includes
  the new cadence marker. Documentation governance and every specialized gate
  pass.
- Before the repair, three Release DOS-prompt runs were 1.281, 1.351 and 1.372
  seconds. After the repair they are 1.098, 1.100 and 1.148 seconds. This is a
  15--20 percent end-to-end improvement on the existing DOS startup workload;
  it is not a Windows graphics benchmark.
- The rebuilt stripped Release artifact is
  `build/output/nxvm_0_5_0458.exe`, SHA-256
  `F9A77CDE35A4DEC18CC566800D59205EC43A41C4B30F533E2EF132E0DE14F71D`;
  `objdump -h` finds no `.debug` section.

## Minimalism Review

The implementation adds one small host-clock query, two session fields and one
owner-local due check. It deletes the redundant pre-capture maximum-snapshot
clear. It does not alter the runner quantum, add a profile decision, duplicate
display state, or disable the runtime debugger. A separate optional turbo
policy remains outside this task because it would intentionally change guest
wall-clock pacing.

The CMake/source/test change is 99 added and 70 deleted lines, net +29;
documentation is excluded from that count. The added regression replaces a
larger static-screen probe, rather than introducing a second test harness.
