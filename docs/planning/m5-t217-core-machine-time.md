# M5 T217: Core Machine-Time Contract

## S1: Contract And Probe Design

**Status:** complete.

**Source baseline:** `442879d` (M5 T216 S5 artifact record). The retained
baseline is the T216 Console/debugger/FDD/HDD matrix, including unpaced FDD
`EDIT.COM` launch.

T217 establishes the prerequisite for all subsequent time-sensitive PC/AT
device work. `core_machine` will become the only owner of monotonically
increasing guest `elapsed_ticks`. A VM profile will provide frozen clock and
divider parameters; VM composition may bind those parameters and apply host
pacing, but neither composition nor platform may advance guest time.

### Contract

1. Define the tick scalar, origin, overflow behavior, frozen profile parameters,
   and the relationship between an architectural instruction and coarse ticks.
2. Define how bounded `core_machine_run()`, single-step, pause, stop, fault,
   cold reset, and explicit reset observe or advance elapsed time.
3. Define the ownership and call order for the future T219 post-instruction
   scheduler, without implementing that scheduler in T217.
4. Define snapshot/diagnostic observation of elapsed time without exposing a
   mutable clock pointer to platform, profile, Console, or debugger code.

### Implementation And Probes

The focused contract smoke must demonstrate that equal reset-and-run inputs
produce equal tick results; pause and debugger inspection do not advance time;
the same instruction budget has a deterministic tick result; a cold reset
returns to the defined origin; and host wait/pacing has no guest-clock effect.
The retained FDD/HDD boot, DOS prompt, Console, debugger, and T216 `EDIT.COM`
regressions remain behavior-protection gates for later implementation subtasks.

S2 adds `elapsed_ticks` and the resolved frozen `ticks_per_instruction` to the
sole `core_machine` instance. `core_machine_run()` now observes both optional
instruction and tick budgets; completed architectural instruction boundaries
advance the core clock; cold reset returns it to zero. `core_machine_run_result`
and `core_machine_observation` return copied clock values; running callers
cannot read a mutable clock reference. The default PC/AT profile supplies the
frozen one-tick parameter at session creation.

`core-machine-time-smoke` records `M5:T217:S2:TIME:OK` for reset origin,
instruction budget, tick budget, and observation behavior. S3 passed the full
current GCC/CTest matrix: 53/53 tests, including Console, debugger, FDD/HDD,
DOS prompt, keyboard, video, and T216 `EDIT.COM` regressions. T217 intentionally
does not alter device refresh order; that move is T219.

### Artifact

Developer artifact: `build/output/nxvm_0_5_0217.exe`.

SHA-256: `C1C704F396EDDFEAA3BA1839142ABF776FA0B33CD3E8AF23DED34D28518EC290`.

Source commit: `87ca1e5` (`M5 T217 S3 P1`). No later device scheduler, raster,
or timer waveform work is admitted by T217 itself.

### Non-Goals And Stop Conditions

T217 does not implement a cycle-accurate clock, PIT waveform, VADP raster,
DMA timing, RTC semantics, or 286/386/FPU support. Stop and revise the design
if it requires a VM-side device execution loop, direct platform guest mutation,
a host-wall-clock guest-time shortcut, duplicate time state, or a user-visible
NXVM behavior change.

Applicable rules: the then-current machine/session ownership and wrapper
discipline rules,
and scope/testing/governance; the hardware-device verification template; and
the M5 compatibility gate in `roadmap.md`.
