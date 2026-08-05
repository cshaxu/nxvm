# M5 T217: Core Machine-Time Contract

## S1: Contract And Probe Design

**Status:** active. This subtask designs the contract and focused probes only;
it changes no runtime behavior and produces no executable artifact.

**Source baseline:** `442879d` (M5 T216 S5 artifact record). The retained
baseline is the T216 Console/debugger/FDD/HDD matrix, including unpaced FDD
`EDIT.COM` launch.

T217 establishes the prerequisite for all subsequent time-sensitive PC/AT
device work. `core_machine` will become the only owner of monotonically
increasing guest `elapsed_ticks`. A VM profile will provide frozen clock and
divider parameters; VM composition may bind those parameters and apply host
pacing, but neither composition nor platform may advance guest time.

### Required Contract Decisions

1. Define the tick scalar, origin, overflow behavior, frozen profile parameters,
   and the relationship between an architectural instruction and coarse ticks.
2. Define how bounded `core_machine_run()`, single-step, pause, stop, fault,
   cold reset, and explicit reset observe or advance elapsed time.
3. Define the ownership and call order for the future T219 post-instruction
   scheduler, without implementing that scheduler in T217.
4. Define snapshot/diagnostic observation of elapsed time without exposing a
   mutable clock pointer to platform, profile, Console, or debugger code.

### S1 Probe Plan

The focused contract smoke must demonstrate that equal reset-and-run inputs
produce equal tick results; pause and debugger inspection do not advance time;
the same instruction budget has a deterministic tick result; a cold reset
returns to the defined origin; and host wait/pacing has no guest-clock effect.
The retained FDD/HDD boot, DOS prompt, Console, debugger, and T216 `EDIT.COM`
regressions remain behavior-protection gates for later implementation subtasks.

Expected design/probe markers are `M5:T217:S1:TIME-CONTRACT:OK` and
`M5:T217:S1:TIME-PROBE:OK`; their exact command names and source locations are
part of this S1 design, not pre-committed implementation choices.

### Later Subtasks

S2 may add the one core-owned clock field, frozen profile parameters, and the
documented observation boundary only after S1 fixes the API and probe shape.
S3 must execute the focused clock probes plus the current GCC/CTest matrix and
retained Console/debugger/FDD/HDD regressions, then record the T217 artifact
hash and source commit. No later device scheduler, raster, or timer waveform
work is admitted by T217 itself.

### Non-Goals And Stop Conditions

T217 does not implement a cycle-accurate clock, PIT waveform, VADP raster,
DMA timing, RTC semantics, or 286/386/FPU support. Stop and revise the design
if it requires a VM-side device execution loop, direct platform guest mutation,
a host-wall-clock guest-time shortcut, duplicate time state, or a user-visible
NXVM behavior change.

Applicable rules: `RULES.md` machine/session ownership, wrapper discipline,
and scope/testing/governance; the hardware-device verification template; and
the M5 compatibility gate in `roadmap.md`.
